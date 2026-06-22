import os
import rclpy
from rclpy.node import Node
from std_msgs.msg import Empty, Int16, UInt16, UInt8
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Quaternion, Point, TransformStamped, Vector3
import numpy as np
import math
import tf2_ros
from std_msgs.msg import Float32MultiArray
from std_msgs.msg import Float32

class AckermannOdometryRosEKF(Node):
    def __init__(self):
        super().__init__("ackermann_odometry_node")

        self.get_logger().info("Initializing AckermannOdometryRosEKF Node...")
        
        # Initial pose
        self.p_x = 0.0
        self.p_y = 0.0
        self.v_x = 0.0
        self.v_y = 0.0
        self.a_x = 0.0
        self.a_y = 0.0
        self.theta = 0.0
        self.dtheta = 0.0
        self.wheelbase = 0.895  # Adjust based on your robot

        # Wheel speeds (pulse counts per update)
        self.wheel_speeds = {"LF": 0, "LR": 0, "RF": 0, "RR": 0}

        self.dt = 0.01
        self.last_time = self.get_clock().now().nanoseconds

        # sample time
        self.sample_time = 1
        self.countRR = 0
        self.countLR = 0
        self.countLF = 0
        self.countRF = 0

        # Outlier rejection
        self.prev_wheel_speeds = {"LF": 0, "LR": 0, "RF": 0, "RR": 0}
        self.max_pulse_jump = 10   # tune this
        self.max_valid_pulse = 20   # tune this
        
        # Subscribe to steering angle topic
        self.create_subscription(
            Int16, "/GET_0x7d0_Get_SteeringAngle", self.steering_callback, 10
        )

        # Subscribe to wheel speed sensors
        self.create_subscription(
            UInt16, "/GET_0x5dc_SpeedSensorLF_PulseCnt", self.speed_callback_LF, 10
        )
        self.create_subscription(
            UInt16, "/GET_0x5dc_SpeedSensorLR_PulseCnt", self.speed_callback_LR, 10
        )
        self.create_subscription(
            UInt16, "/GET_0x5dc_SpeedSensorRF_PulseCnt", self.speed_callback_RF, 10
        )
        self.create_subscription(
            UInt16, "/GET_0x5dc_SpeedSensorRR_PulseCnt", self.speed_callback_RR, 10
        )
        self.create_subscription(
            UInt16, "/GET_0x5dc_SpeedSensorSampleTime", self.sample_time_callback, 10
        )
        self.create_subscription(
            UInt8, "/controller_reverse_mode", self.reverse_mode_callback, 10
        )
        self.create_subscription(
            Empty, "/reset_odom", self.reset_odom_callback, 10
        )
        
        # Publisher for odometry
        self.odom_pub = self.create_publisher(Odometry, "/odom", 10)

        # Timer to update odometry
        self.timer = self.create_timer(self.dt, self.update_odometry)

        # Sensor values
        self.v = 0.0  # Computed speed
        self.delta = 0.0  # Steering angle
        self.reverse_mode = False

        # Vehicle parameters
        self.front_wheel_radius = 0.1065  # Adjust based on your robot
        self.rear_wheel_radius = 0.1235  # Adjust based on your robot
        self.pulses_per_rev_rear = 120  # Adjust based on sensor
        self.pulses_per_rev_front = 70  # Adjust based on sensor

        # Topics from CAN and ROS
        self.ros_time = 100
        self.can_time = 100
        self.time = 1000

        self.get_logger().info("AckermannOdometryRos Node initialized!")

    def steering_callback(self, msg):
        self.delta = np.radians(msg.data)

    def speed_callback_LF(self, msg):
        val = msg.data
        if val > self.max_valid_pulse or (val - self.prev_wheel_speeds["LF"]) > self.max_pulse_jump:
            self.get_logger().warn(f"LF outlier rejected: {val}")
            val = self.prev_wheel_speeds["LF"]
        self.wheel_speeds["LF"] = val
        self.prev_wheel_speeds["LF"] = val
        self.countLF += msg.data

    def speed_callback_LR(self, msg):
        val = msg.data
        if val > self.max_valid_pulse or (val - self.prev_wheel_speeds["LR"]) > self.max_pulse_jump:
            self.get_logger().warn(f"LR outlier rejected: {val}")
            val = self.prev_wheel_speeds["LR"]
        self.wheel_speeds["LR"] = val
        self.prev_wheel_speeds["LR"] = val
        self.countLR += msg.data
        
    def speed_callback_RF(self, msg):
        val = msg.data
        if val > self.max_valid_pulse or (val - self.prev_wheel_speeds["RF"]) > self.max_pulse_jump:
            self.get_logger().warn(f"RF outlier rejected: {val}")
            val = self.prev_wheel_speeds["RF"]
        self.wheel_speeds["RF"] = val
        self.prev_wheel_speeds["RF"] = val
        self.countRF += msg.data
        
    def speed_callback_RR(self, msg):
        val = msg.data
        if val > self.max_valid_pulse or (val - self.prev_wheel_speeds["RR"]) > self.max_pulse_jump:
            self.get_logger().warn(f"RR outlier rejected: {val}")
            val = self.prev_wheel_speeds["RR"]
        self.wheel_speeds["RR"] = val
        self.prev_wheel_speeds["RR"] = val
        self.countRR += msg.data

    def sample_time_callback(self, msg):
        self.sample_time = msg.data

    def reverse_mode_callback(self, msg):
        self.reverse_mode = msg.data != 0

    def reset_odom_callback(self, _msg):
        self.p_x = 0.0
        self.p_y = 0.0
        self.theta = 0.0
        self.dtheta = 0.0
        self.last_time = self.get_clock().now().nanoseconds
        self.get_logger().info("Reset odometry state from /reset_odom.")

    def variance_callback(self, msg):
        return

    def calculate_speed(self):
        wheel_velocities = {
            "LF": (self.wheel_speeds["LF"] / self.pulses_per_rev_front)
            * (2 * np.pi * self.front_wheel_radius)
            / (self.sample_time / self.time),
            "LR": (self.wheel_speeds["LR"] / self.pulses_per_rev_rear)
            * (2 * np.pi * self.rear_wheel_radius)
            / (self.sample_time / self.time),
            "RF": (self.wheel_speeds["RF"] / self.pulses_per_rev_front)
            * (2 * np.pi * self.front_wheel_radius)
            / (self.sample_time / self.time),
            "RR": (self.wheel_speeds["RR"] / self.pulses_per_rev_rear)
            * (2 * np.pi * self.rear_wheel_radius)
            / (self.sample_time / self.time),
        }

        speed_sign = -1.0 if self.reverse_mode else 1.0

        if abs(self.delta) < 1e-6:
            speed_magnitude = (
                wheel_velocities["LR"]
                + wheel_velocities["LF"]
                + wheel_velocities["RF"]
                + wheel_velocities["RR"]
            ) / 4
        else:
            speed_magnitude = (
                wheel_velocities["LR"]
                + wheel_velocities["RR"]
            ) / 2

        self.v = speed_sign * speed_magnitude


    def update_odometry(self):

        current_time = self.get_clock().now().nanoseconds  # Get time in nanoseconds
        dt = (current_time - self.last_time) / 1e9  # Convert nanoseconds to seconds
        self.last_time = current_time
        self.dt = dt
            
        self.calculate_speed()

        if abs(self.delta) < 1e-6:
            dx = self.v * self.dt * np.cos(self.theta)
            dy = self.v * self.dt * np.sin(self.theta)
            self.dtheta = 0.0
        else:
            R = self.wheelbase / np.tan(self.delta)
            self.dtheta = self.v * self.dt / R
            dx = R * (np.sin(self.theta + self.dtheta) - np.sin(self.theta))
            dy = -R * (np.cos(self.theta + self.dtheta) - np.cos(self.theta))

        self.p_x += dx
        self.p_y += dy
        self.theta += self.dtheta
        self.theta = (self.theta + np.pi) % (2 * np.pi) - np.pi

        self.publish_odometry()

    def publish_odometry(self):
        odom_msg = Odometry()
        odom_msg.header.stamp = self.get_clock().now().to_msg()
        odom_msg.header.frame_id = "odom"
        odom_msg.child_frame_id = "base_footprint"

        odom_msg.pose.pose.position = Point(x=self.p_x, y=self.p_y, z=0.0)
        odom_msg.pose.pose.orientation = Quaternion(
            x=0.0, y=0.0, z=math.sin(self.theta / 2.0), w=math.cos(self.theta / 2.0)
        )
        odom_msg.twist.twist.linear.x = self.v
        odom_msg.twist.twist.angular.z = self.dtheta / self.dt if self.dt > 0 else 0.0

        # Covariance matrices used for EKF
        # Pose cov =  [x, y, z, roll, pitch, yaw]
        odom_msg.pose.covariance = [
            0.25, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0,  0.25, 0.0, 0.0, 0.0, 0.0,
            0.0,  0.0, 1e6, 0.0, 0.0, 0.0,
            0.0,  0.0, 0.0, 1e6, 0.0, 0.0,
            0.0,  0.0, 0.0, 0.0, 1e6, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.25
        ]
        # Twist cov = [v_x, v_y, v_z, \omega_x, \omega_y, \omega_z]
        odom_msg.twist.covariance = [
            0.06, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0,  1e6, 0.0, 0.0, 0.0, 0.0,
            0.0,  0.0, 1e6, 0.0, 0.0, 0.0,
            0.0,  0.0, 0.0, 1e6, 0.0, 0.0,
            0.0,  0.0, 0.0, 0.0, 1e6, 0.0,
            0.0,  0.0, 0.0, 0.0, 0.0, 0.1
        ]

        self.odom_pub.publish(odom_msg)
            
def main(args=None):
    rclpy.init(args=args)
    node = AckermannOdometryRosEKF()
    node.get_logger().info("AckermannOdometryRosEKF node created!")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
