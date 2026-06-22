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
    use_sim_time = LaunchConfiguration("use_sim_time", default="true")

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

    default_world = os.path.join(
        get_package_share_directory(package_name), "worlds", "gocartcentralen.world"
    )

    world = LaunchConfiguration("world")

    world_arg = DeclareLaunchArgument(
        "world", default_value=default_world, description="World to load"
    )

    rviz_node = launch_ros.actions.Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", LaunchConfiguration("rvizconfig")],
    )

    # Include Gazebo launch file, included in the ros_gz_sim native pkg
    gazebo_node = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [
                os.path.join(
                    get_package_share_directory("ros_gz_sim"),
                    "launch",
                    "gz_sim.launch.py",
                )
            ]
        ),
        launch_arguments={
            "gz_args": ["-r -v4 ", world],
            "on_exit_shutdown": "true",
        }.items(),
    )

    # Run Gazebo entity spawner node
    spawn_entity_node = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-topic",
            "robot_description",
            "-name",
            "AP4_digital_twin",
            "-x",
            "0.0",
            "-y",
            "-4",
            "-z",
            "0.5",
            "-Y",
            "3.1415",
        ],
    )

    bridge_params = os.path.join(
        get_package_share_directory(package_name), "config", "gz_bridge.yaml"
    )
    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "--ros-args",
            "-p",
            f"config_file:={bridge_params}",
        ],
    )

    # Get nav2_bringup package path
    nav2_pkg_path = get_package_share_directory("nav2_bringup")
    nav2_launch_file = os.path.join(nav2_pkg_path, "launch", "navigation_launch.py")
    nav2_params_file = os.path.join(
        get_package_share_directory("autonomous_platform_robot_description_pkg"),
        "config",
        "nav2_config_sim.yaml",
    )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(nav2_launch_file),
        launch_arguments={
            "use_sim_time": "True",
            "params_file": nav2_params_file,
            "slam": "True",  # This disables AMCL and prevents conflict
        }.items(),
    )

    # SLAM Toolbox launch
    slam_toolbox_pkg_path = get_package_share_directory("slam_toolbox")
    slam_config_file = os.path.join(
        pkg_share, "config", "mapper_params_online_async_simulation.yaml"
    )

    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_toolbox_pkg_path, "launch", "online_async_launch.py")
        ),
        launch_arguments={
            "use_sim_time": "True",
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
        parameters=[
            ekf_config,
            {"use_sim_time": use_sim_time},
            {"odom0": "/odom_noisy"},
            {"imu0": "/imu"},
        ],
    )
    odom_noise_node = Node(
        package = "autonomous_platform_robot_description_pkg",
        executable = "odom_noise_injector.py",
        name = "odom_noise_injector",
        output = "screen",
        parameters=[
            {"use_sim_time":use_sim_time},
            {"input_topic": "/odom"},
            {"output_topic": "/odom_noisy"},
            {"position_noise_stddev": 0.0},
            {"yaw_noise_stddev": 0.01},
            {"linear_velocity_noise_stddev": 0.01},
            {"angular_velocity_noise_stddev": 0.007},
            {"position_random_walk_stddev": 0.0},
            {"yaw_random_walk_stddev": 0.0},
            {"covariance_scale": 2.0},
            {"random_seed":35},
        ],
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
                default_value="true",
                description="Use simulation (Gazebo) clock if true",
            ),
            world_arg,
            robot_state_publisher_node,
            rviz_node,
            gazebo_node,
            spawn_entity_node,
            ros_gz_bridge,
            odom_noise_node,
            nav2_launch,
            slam_launch,
            ekf_node,
        ]
    )
