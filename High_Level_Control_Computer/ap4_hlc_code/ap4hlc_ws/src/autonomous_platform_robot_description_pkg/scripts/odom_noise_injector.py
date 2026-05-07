#!/usr/bin/env python3

import math
import random

import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node


def yaw_from_quaternion(z, w):
    return 2.0 * math.atan2(z, w)


def quaternion_from_yaw(yaw):
    return math.sin(yaw / 2.0), math.cos(yaw / 2.0)


class OdomNoiseInjector(Node):
    def __init__(self):
        super().__init__("odom_noise_injector")

        self.declare_parameter("input_topic", "/odom")
        self.declare_parameter("output_topic", "/odom_noisy")
        self.declare_parameter("position_noise_stddev", 0.05)
        self.declare_parameter("yaw_noise_stddev", 0.02)
        self.declare_parameter("linear_velocity_noise_stddev", 0.05)
        self.declare_parameter("angular_velocity_noise_stddev", 0.02)
        self.declare_parameter("position_random_walk_stddev", 0.01)
        self.declare_parameter("yaw_random_walk_stddev", 0.005)
        self.declare_parameter("covariance_scale", 4.0)
        self.declare_parameter("random_seed", 42)

        input_topic = str(self.get_parameter("input_topic").value)
        output_topic = str(self.get_parameter("output_topic").value)

        self.position_noise_stddev = float(
            self.get_parameter("position_noise_stddev").value
        )
        self.yaw_noise_stddev = float(self.get_parameter("yaw_noise_stddev").value)
        self.linear_velocity_noise_stddev = float(
            self.get_parameter("linear_velocity_noise_stddev").value
        )
        self.angular_velocity_noise_stddev = float(
            self.get_parameter("angular_velocity_noise_stddev").value
        )
        self.position_random_walk_stddev = float(
            self.get_parameter("position_random_walk_stddev").value
        )
        self.yaw_random_walk_stddev = float(
            self.get_parameter("yaw_random_walk_stddev").value
        )
        self.covariance_scale = float(self.get_parameter("covariance_scale").value)
        random_seed = int(self.get_parameter("random_seed").value)

        random.seed(random_seed)

        self.random_walk_x = 0.0
        self.random_walk_y = 0.0
        self.random_walk_yaw = 0.0
        self.last_stamp = None

        self.publisher = self.create_publisher(Odometry, output_topic, 20)
        self.subscription = self.create_subscription(
            Odometry, input_topic, self.odom_callback, 20
        )

        self.get_logger().info(
            f"Injecting odom noise: '{input_topic}' -> '{output_topic}', seed={random_seed}"
        )

    def _compute_dt(self, msg):
        stamp = msg.header.stamp
        now = stamp.sec + stamp.nanosec * 1e-9
        if self.last_stamp is None:
            self.last_stamp = now
            return 1.0 / 50.0
        dt = max(1e-3, now - self.last_stamp)
        self.last_stamp = now
        return dt

    def _inflate_covariances(self, odom_msg, dt):
        pose_var_xy = (self.position_noise_stddev**2) + (
            self.position_random_walk_stddev**2
        ) * dt
        pose_var_yaw = (self.yaw_noise_stddev**2) + (
            self.yaw_random_walk_stddev**2
        ) * dt
        twist_var_vx = self.linear_velocity_noise_stddev**2
        twist_var_wz = self.angular_velocity_noise_stddev**2

        odom_msg.pose.covariance[0] = max(
            odom_msg.pose.covariance[0], pose_var_xy * self.covariance_scale
        )  # x
        odom_msg.pose.covariance[7] = max(
            odom_msg.pose.covariance[7], pose_var_xy * self.covariance_scale
        )  # y
        odom_msg.pose.covariance[35] = max(
            odom_msg.pose.covariance[35], pose_var_yaw * self.covariance_scale
        )  # yaw

        odom_msg.twist.covariance[0] = max(
            odom_msg.twist.covariance[0], twist_var_vx * self.covariance_scale
        )  # vx
        odom_msg.twist.covariance[35] = max(
            odom_msg.twist.covariance[35], twist_var_wz * self.covariance_scale
        )  # wz

    def odom_callback(self, msg):
        dt = self._compute_dt(msg)

        # Slowly drifting bias to mimic wheel slip / model mismatch.
        self.random_walk_x += random.gauss(
            0.0, self.position_random_walk_stddev * math.sqrt(dt)
        )
        self.random_walk_y += random.gauss(
            0.0, self.position_random_walk_stddev * math.sqrt(dt)
        )
        self.random_walk_yaw += random.gauss(
            0.0, self.yaw_random_walk_stddev * math.sqrt(dt)
        )

        out = Odometry()
        out.header = msg.header
        out.child_frame_id = msg.child_frame_id

        out.pose.pose.position.x = (
            msg.pose.pose.position.x
            + self.random_walk_x
            + random.gauss(0.0, self.position_noise_stddev)
        )
        out.pose.pose.position.y = (
            msg.pose.pose.position.y
            + self.random_walk_y
            + random.gauss(0.0, self.position_noise_stddev)
        )
        out.pose.pose.position.z = msg.pose.pose.position.z

        yaw = yaw_from_quaternion(
            msg.pose.pose.orientation.z, msg.pose.pose.orientation.w
        )
        noisy_yaw = yaw + self.random_walk_yaw + random.gauss(0.0, self.yaw_noise_stddev)
        out.pose.pose.orientation.x = 0.0
        out.pose.pose.orientation.y = 0.0
        out.pose.pose.orientation.z, out.pose.pose.orientation.w = quaternion_from_yaw(
            noisy_yaw
        )

        out.twist.twist = msg.twist.twist
        out.twist.twist.linear.x += random.gauss(0.0, self.linear_velocity_noise_stddev)
        out.twist.twist.angular.z += random.gauss(
            0.0, self.angular_velocity_noise_stddev
        )

        out.pose.covariance = msg.pose.covariance
        out.twist.covariance = msg.twist.covariance
        self._inflate_covariances(out, dt)

        self.publisher.publish(out)


def main(args=None):
    rclpy.init(args=args)
    node = OdomNoiseInjector()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
