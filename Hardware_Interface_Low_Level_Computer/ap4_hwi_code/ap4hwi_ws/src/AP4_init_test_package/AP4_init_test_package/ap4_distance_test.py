import math
import rclpy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node
import sys
class DistanceTest(Node):
    def __init__(self):
        super().__init__('distance_test')
        
        # Parameters
        self.declare_parameter('target_distance', 5.0)
        self.declare_parameter('max_speed', 0.45)
        
        self.target_distance = float(self.get_parameter('target_distance').value)
        self.max_speed = float(self.get_parameter('max_speed').value)
        
        # Ramping/Timing Constants
        self.start_speed = 0.25
        self.acceleration = 0.05 
        self.dt = 0.02 # 50Hz
        self.stop_duration = 3.0 # Seconds to wait for drift
        
        self.cmd_vel_pub_ = self.create_publisher(Twist, '/camera_cmd_vel', 10)
        self.odom_sub_ = self.create_subscription(Odometry, '/odom', self.odom_callback, 10)
        
        self.start_pose = None
        self.current_pose = None
        self.current_linear_speed = self.start_speed
        
        # States: "DRIVING", "WAITING", "FINISHED"
        self.state = "DRIVING"
        self.stop_time = None 
        
        self.timer = self.create_timer(self.dt, self.control_loop)
        self.get_logger().info("Validation Test Started. Waiting for Odom origin...")

    def odom_callback(self, msg):
        self.current_pose = msg.pose.pose.position
        if self.start_pose is None:
            self.start_pose = self.current_pose

    def get_distance_moved(self):
        if self.start_pose is None or self.current_pose is None:
            return 0.0
        return math.sqrt(
            (self.current_pose.x - self.start_pose.x)**2 + 
            (self.current_pose.y - self.start_pose.y)**2
        )
        
    def control_loop(self):
        if self.start_pose is None:
            return
        
        dist = self.get_distance_moved()

        if self.state == "DRIVING":
            if dist < self.target_distance:
                # Slew rate acceleration
                if self.current_linear_speed < self.max_speed:
                    self.current_linear_speed += self.acceleration * self.dt
                
                cmd = Twist()
                cmd.linear.x = self.current_linear_speed
                self.cmd_vel_pub_.publish(cmd)
                self.get_logger().info(f"Driving: {dist:.2f}m", throttle_duration_sec=0.5)
            else:
                # Target reached - Switch to waiting state
                self.state = "WAITING"
                self.stop_time = self.get_clock().now()
                self.stop_motors()
                self.get_logger().info("Target distance reached. Measuring coasting drift...")

        elif self.state == "WAITING":
            # Keep stopping to be safe
            self.stop_motors()
            
            # Calculate how long we've been waiting
            elapsed_wait = (self.get_clock().now() - self.stop_time).nanoseconds / 1e9
            self.get_logger().info(f"Coasting... Total Dist: {dist:.3f}m", throttle_duration_sec=0.5)

            if elapsed_wait >= self.stop_duration:
                self.report_and_exit(dist)

    def stop_motors(self):
        cmd = Twist()
        cmd.linear.x = 0.0
        cmd.angular.z = 0.0
        self.cmd_vel_pub_.publish(cmd)

    def report_and_exit(self, final_dist):
        self.get_logger().info("---------------------------------------")
        self.get_logger().info(f"FINAL ODOM POSITION (Including Drift): {final_dist:.3f} m")
        self.get_logger().info(f"DRIFT DISTANCE: {final_dist - self.target_distance:.3f} m")
        self.get_logger().info("---------------------------------------")
        
        sys.exit(0)
def main(args=None):
    rclpy.init(args=args)
    test_node = DistanceTest()
    try:
        rclpy.spin(test_node)
    except (KeyboardInterrupt, SystemExit):
        pass
    finally:
        test_node.stop_motors()
        test_node.destroy_node()
        rclpy.shutdown()
if __name__ == "__main__":

    main()