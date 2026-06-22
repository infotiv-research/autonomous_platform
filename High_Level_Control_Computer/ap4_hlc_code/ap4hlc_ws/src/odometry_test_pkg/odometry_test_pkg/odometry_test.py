#!/usr/bin/env python3

import math

import rclpy
from rclpy.node import Node

from nav_msgs.msg import Odometry
from odometry_test_msgs.msg import OdometryTestData, Pose2DYaw
from transforms3d.euler import quat2euler


class OdometryTestNode(Node):
    def __init__(self):
        super().__init__('odometry_test_node')

        self.odom_msg = None
        self.filtered_msg = None
        self.odom_start_pose = None

        self.odom_sub = self.create_subscription(
            Odometry,
            '/odom',
            self.odom_callback,
            10
        )

        self.filtered_sub = self.create_subscription(
            Odometry,
            '/odometry/filtered',
            self.filtered_callback,
            10
        )

        self.comparison_pub = self.create_publisher(
            OdometryTestData,
            '/odometry_test',
            10
        )

        self.get_logger().info('Odometry test node started.')

    def odometry_to_test_pose(self, msg: Odometry):
        pose = Pose2DYaw()
        pose.x = msg.pose.pose.position.x
        pose.y = msg.pose.pose.position.y
        pose.yaw = quat2euler((
            msg.pose.pose.orientation.w,
            msg.pose.pose.orientation.x,
            msg.pose.pose.orientation.y,
            msg.pose.pose.orientation.z,
        ))[2]
        return pose

    @staticmethod
    def normalize_angle(angle):
        return math.atan2(math.sin(angle), math.cos(angle))

    def make_relative_odom_pose(self, pose: Pose2DYaw):
        relative_pose = Pose2DYaw()
        relative_pose.x = pose.x - self.odom_start_pose.x
        relative_pose.y = pose.y - self.odom_start_pose.y
        relative_pose.yaw = self.normalize_angle(pose.yaw - self.odom_start_pose.yaw)
        return relative_pose

    def odom_callback(self, msg: Odometry):
        if self.odom_start_pose is None:
            self.odom_start_pose = self.odometry_to_test_pose(msg)
        self.odom_msg = msg
        self.publish_comparison()

    def filtered_callback(self, msg: Odometry):
        self.filtered_msg = msg
        self.publish_comparison()

    def publish_comparison(self):
        if self.odom_msg is None or self.filtered_msg is None:
            return

        odom_pose = self.make_relative_odom_pose(
            self.odometry_to_test_pose(self.odom_msg)
        )
        filtered_pose = self.odometry_to_test_pose(self.filtered_msg)

        comparison_msg = OdometryTestData()
        comparison_msg.odom = odom_pose
        comparison_msg.filtered = filtered_pose
        comparison_msg.dx = odom_pose.x - filtered_pose.x
        comparison_msg.dy = odom_pose.y - filtered_pose.y
        comparison_msg.dyaw = self.normalize_angle(odom_pose.yaw - filtered_pose.yaw)

        self.comparison_pub.publish(comparison_msg)

        self.get_logger().info(
            f'/odom: ({odom_pose.x:.3f}, {odom_pose.y:.3f}, '
            f'{math.degrees(odom_pose.yaw):.2f} deg) | '
            f'/odometry/filtered: ({filtered_pose.x:.3f}, {filtered_pose.y:.3f}, '
            f'{math.degrees(filtered_pose.yaw):.2f} deg) | '
            f'diff: ({comparison_msg.dx:.3f}, {comparison_msg.dy:.3f}, '
            f'{math.degrees(comparison_msg.dyaw):.2f} deg)'
        )


def main(args=None):
    rclpy.init(args=args)
    node = OdometryTestNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
