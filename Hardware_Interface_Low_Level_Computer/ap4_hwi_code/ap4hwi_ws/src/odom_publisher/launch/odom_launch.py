from launch import LaunchDescription
from launch_ros.actions import Node
import launch_ros
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python import get_package_share_directory
import os


def generate_launch_description():

    ackermann_odometry_node = launch_ros.actions.Node(
        package="odom_publisher",
        executable="odom_ackermann",
        output="screen",
    )

    return LaunchDescription([ackermann_odometry_node])
