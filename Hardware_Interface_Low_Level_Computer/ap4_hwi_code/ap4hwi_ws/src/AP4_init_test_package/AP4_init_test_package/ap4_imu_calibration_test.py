#!/usr/bin/env python3
import time
import numpy as np

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu

from scipy.spatial.transform import Rotation as R


class IMUCalibration(Node):
    def __init__(self):
        super().__init__("imu_calibration")

        self.acc_samples = []   # ax, ay, az
        self.gyro_samples = []  # gx, gy, gz
        self.rpy_samples = []   # roll, pitch, yaw (radians)

        self.create_subscription(Imu, "imu", self.imu_callback, 50)

        self.duration_s = 60.0
        self.start_time = time.time()
        self.create_timer(1.0, self.check_done)

        self.get_logger().info(f"Recording IMU for {self.duration_s:.0f}s...")

    def imu_callback(self, msg: Imu):
        ax = msg.linear_acceleration.x
        ay = msg.linear_acceleration.y
        az = msg.linear_acceleration.z

        gx = msg.angular_velocity.x
        gy = msg.angular_velocity.y
        gz = msg.angular_velocity.z

        qx = msg.orientation.x
        qy = msg.orientation.y
        qz = msg.orientation.z
        qw = msg.orientation.w

        roll, pitch, yaw = R.from_quat([qx, qy, qz, qw]).as_euler('xyz', degrees=False)

        self.acc_samples.append([ax, ay, az])
        self.gyro_samples.append([gx, gy, gz])
        self.rpy_samples.append([roll, pitch, yaw])

    def check_done(self):
        elapsed = time.time() - self.start_time
        if elapsed >= self.duration_s:
            self.compute_and_print()
            rclpy.shutdown()
        else:
            self.get_logger().info(f"Time remaining: {int(self.duration_s - elapsed)}s")

    def compute_and_print(self):
        self.get_logger().info("Entered compute_and_print()")
        if not self.acc_samples:
            self.get_logger().info("No IMU samples received.")
            return

        acc = np.asarray(self.acc_samples, dtype=float)
        gyro = np.asarray(self.gyro_samples, dtype=float)
        rpy = np.asarray(self.rpy_samples, dtype=float)
        self.get_logger().info("Done with array building")


        acc_mean = np.mean(acc, axis=0)
        gyro_mean = np.mean(gyro, axis=0)
        rpy_mean = np.mean(rpy, axis=0)
        self.get_logger().info("Done with mean calculations")


        acc_cov = np.cov(acc, rowvar=False, bias=True)
        gyro_cov = np.cov(gyro, rowvar=False, bias=True)
        rpy_cov = np.cov(rpy, rowvar=False, bias=True)
        self.get_logger().info("Done with cov calculations")


        self.get_logger().info("\nIMU means:")
        self.get_logger().info(f"  accel mean  [ax ay az]   = {acc_mean}")
        self.get_logger().info(f"  gyro mean   [gx gy gz]   = {gyro_mean}")
        self.get_logger().info(f"  rpy mean    [r p y] rad  = {rpy_mean}")

        self.get_logger().info("\n3x3 matrices:")
        self.get_logger().info(f"acc_cov:\n{acc_cov}")
        self.get_logger().info(f"gyro_cov:\n{gyro_cov}")
        self.get_logger().info(f"rpy_cov:\n{rpy_cov}")
        self.get_logger().info("Leaving compute_and_print()")


def main(args=None):
    rclpy.init(args=args)
    node = IMUCalibration()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()


if __name__ == "__main__":
    main()