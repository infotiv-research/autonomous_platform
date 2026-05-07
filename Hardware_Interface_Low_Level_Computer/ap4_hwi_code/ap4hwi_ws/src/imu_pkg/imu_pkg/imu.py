#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import depthai as dai
import numpy as np
import time
import sys

class ImuPublisher(Node):
    def __init__(self):
        super().__init__("oak_imu_fused_publisher")
        self.get_logger().info("Initializing OAK-D IMU Node...")

        # ROS 2 Publisher
        self.imu_publisher = self.create_publisher(Imu, "imu", 10)

        # Define v3 pipeline
        try:
            self.get_logger().info("Defining DepthAI Pipeline...")
            self.pipeline = dai.Pipeline()
            self.imu = self.pipeline.create(dai.node.IMU)

            # Enable sensors
            self.imu.enableIMUSensor(dai.IMUSensor.GAME_ROTATION_VECTOR, 100)
            self.imu.enableIMUSensor(dai.IMUSensor.LINEAR_ACCELERATION, 100)
            self.imu.enableIMUSensor(dai.IMUSensor.GYROSCOPE_CALIBRATED, 100)
            
            self.imu.setBatchReportThreshold(1)
            self.imu.setMaxBatchReports(10)
            self.get_logger().info("Pipeline defined successfully.")
        except Exception as e:
            self.get_logger().error(f"Failed to define pipeline: {e}")
            raise e

        # Robust Device Connection
        self.live_pipeline = None
        max_retries = 5

                # Create queue
        try:
            self.get_logger().info("Creating IMU Output Queue...")
            self.imuQueue = self.imu.out.createOutputQueue(maxSize=10, blocking=False)
            self.get_logger().info("OAK-D Internal IMU Node Started and Streaming.")
        except Exception as e:
            self.get_logger().error(f"Failed to create output queue: {e}")
            raise e
        
        try:
            self.get_logger().info("Creating pipeline")
            self.live_pipeline = self.pipeline.start()
            self.get_logger().info("OAK-D pipeline created")
        except Exception as e:
            self.get_logger().error(f"Failed to create pipeline {e}")
            raise e

    def run(self):
        self.get_logger().info("Entering main run loop...")
        try:
            while rclpy.ok():
                # Get data from the queue
                imuData = self.imuQueue.get() 
                if imuData is None:
                    # self.get_logger().debug("No IMU data in this tick")
                    continue

                imuPackets = imuData.packets
                for packet in imuPackets:
                    msg = Imu()
                    msg.header.stamp = self.get_clock().now().to_msg()
                    msg.header.frame_id = "imu_link"

                    # Covariance
                    msg.orientation_covariance = [5.8e-4, 0.0, 0.0,
                                                  0.0, 3.6e-5, 0.0, 
                                                  0.0, 0.0, 1e-1]
                    msg.linear_acceleration_covariance = [1.3e-2, 0.0, 0.0,
                                                       0.0, 2.9e-2, 0.0,
                                                       0.0, 0.0, 2.5e-2]
                    msg.angular_velocity_covariance = [1.3e-4, 0.0, 0.0,
                                                          0.0, 1.6e-4, 0.0,
                                                          0.0, 0.0, 5e-4]

                    # Rotation Vector
                    q = packet.rotationVector
                    msg.orientation.x = q.i
                    msg.orientation.y = q.j
                    msg.orientation.z = q.k
                    msg.orientation.w = q.real

                    # Calibrated Gyro
                    gyro = packet.gyroscope
                    msg.angular_velocity.x = gyro.x
                    msg.angular_velocity.y = gyro.y
                    msg.angular_velocity.z = gyro.z

                    # Accelerometer
                    accel = packet.acceleroMeter
                    msg.linear_acceleration.x = accel.x
                    msg.linear_acceleration.y = accel.y
                    msg.linear_acceleration.z = accel.z

                    self.imu_publisher.publish(msg)
                    
        except Exception as e:
            self.get_logger().error(f"Error in run loop: {e}", throttle_duration_sec=1.0)

def main(args=None):
    rclpy.init(args=args)
    node = None
    try:
        node = ImuPublisher()
        node.run()
    except Exception as e:
        # Use basic print if logger failed to initialize
        print(f"CRITICAL: Node initialization failed: {e}", file=sys.stderr)
    finally:
        if node:
            node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    main()