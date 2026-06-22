import launch
from launch.substitutions import Command, LaunchConfiguration
import launch_ros
import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node


def generate_launch_description():
    package_name = "autonomous_platform_robot_description_pkg"

    pkg_share = launch_ros.substitutions.FindPackageShare(
        package="autonomous_platform_robot_description_pkg"
    ).find("autonomous_platform_robot_description_pkg")

    default_model_path = os.path.join(
        pkg_share, "src/description/ap4_robot_description.urdf.xacro"
    )

    default_rviz_config_path = os.path.join(pkg_share, "rviz/rviz_nav2.rviz")
    use_sim_time = LaunchConfiguration("use_sim_time", default="false")

    robot_state_publisher_node = launch_ros.actions.Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[
            {
                "robot_description": Command(["xacro ", LaunchConfiguration("model")]),
                "use_sim_time": use_sim_time,
            }
        ],
    )

    joint_state_publisher_node = launch_ros.actions.Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
        name="joint_state_publisher",
        output="screen",
        parameters=[{"use_sim_time": use_sim_time}],
    )

    rviz_node = launch_ros.actions.Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", LaunchConfiguration("rvizconfig")],
    )

    # Get nav2_bringup package path
    nav2_pkg_path = get_package_share_directory("nav2_bringup")
    nav2_launch_file = os.path.join(nav2_pkg_path, "launch", "navigation_launch.py")

    nav2_params_file = os.path.join(
        get_package_share_directory("autonomous_platform_robot_description_pkg"),
        "config",
        "nav2_config.yaml",
    )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(nav2_launch_file),
        launch_arguments={
            "use_sim_time": "false",
            "params_file": nav2_params_file,
            "slam": "True",  # This disables AMCL and prevents conflict
        }.items(),
    )

    # SLAM Toolbox launch
    slam_toolbox_pkg_path = get_package_share_directory("slam_toolbox")
    slam_config_file = os.path.join(
        pkg_share, "config", "mapper_params_online_async.yaml"
    )

    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_toolbox_pkg_path, "launch", "online_async_launch.py")
        ),
        launch_arguments={
            "use_sim_time": "false",
            "slam_params_file": slam_config_file,
        }.items(),
    )

    ekf_config = os.path.join(
        get_package_share_directory("autonomous_platform_robot_description_pkg"),
        "config",
        "ekf_imu.yaml",
    )

    ekf_node = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[ekf_config, {"odom0": "/odom"}, {"imu0": "/imu"}],
    )

    return LaunchDescription(
        [
            launch.actions.DeclareLaunchArgument(
                name="model",
                default_value=default_model_path,
                description="Absolute path to robot urdf file",
            ),
            launch.actions.DeclareLaunchArgument(
                name="rvizconfig",
                default_value=default_rviz_config_path,
                description="Absolute path to rviz config file",
            ),
            launch.actions.DeclareLaunchArgument(
                name="use_sim_time",
                default_value="false",
                description="Use simulation (Gazebo) clock if true",
            ),
            robot_state_publisher_node,
            joint_state_publisher_node,
            rviz_node,
            nav2_launch,
            slam_launch,
            ekf_node,
        ]
    )
