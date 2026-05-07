import math
import rclpy
import sys
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry

class SteeringTest(Node):
    def __init__(self):
        super().__init__('steering_distance_test')
        
        # Parameters
        self.declare_parameter('target_1_dist', 2.0)  
        self.declare_parameter('target_angle', math.pi/4) 
        self.declare_parameter('target_2_dist', 1.7)  
        
        # Ramping/Timing Constants
        self.start_speed = 0.25      
        self.target_speed = 0.35     
        self.acceleration = 0.05      
        self.turn_speed = 0.3
        self.stop_duration = 3.0 # Seconds to wait for drift

        # State Variables
        # 0=Straight1, 1=Turn, 2=Straight2, 3=COASTING, 4=FINISHED
        self.state = 0  
        self.origin_pose = None   
        self.start_pose = None    
        self.current_pose = None
        self.yaw = 0.0
        self.start_yaw = None
        self.stop_time = None
        
        self.current_linear_speed = self.start_speed
        self.cmd_vel_pub_ = self.create_publisher(Twist, '/camera_cmd_vel', 10)
        self.odom_sub_ = self.create_subscription(Odometry, '/odom', self.odom_callback, 10)

        self.dt = 0.01 
        self.timer = self.create_timer(self.dt, self.control_loop)
        self.get_logger().info("Steering Test Initialized. Waiting for origin...")

    def get_yaw_from_quat(self, q):
        siny_cosp = 2 * (q.w * q.z + q.x * q.y)
        cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z)
        return math.atan2(siny_cosp, cosy_cosp)

    def odom_callback(self, msg):
        self.current_pose = msg.pose.pose.position
        self.yaw = self.get_yaw_from_quat(msg.pose.pose.orientation)

        if self.origin_pose is None:
            self.origin_pose = self.current_pose
            self.start_pose = self.current_pose
            self.start_yaw = self.yaw
            self.get_logger().info(f"Origin Locked! Start Heading: {math.degrees(self.yaw):.1f}°")

    def get_distance(self, p1, p2):
        return math.sqrt((p1.x - p2.x)**2 + (p1.y - p2.y)**2)
    
    def update_speed(self):
        if self.current_linear_speed < self.target_speed:
            self.current_linear_speed += self.acceleration * self.dt
            if self.current_linear_speed > self.target_speed:
                self.current_linear_speed = self.target_speed
        return self.current_linear_speed

    def control_loop(self):
        if self.origin_pose is None or self.state == 4:
            return

        cmd = Twist()
        
        # Calculate Relative Position for logging
        dx = self.current_pose.x - self.origin_pose.x
        dy = self.current_pose.y - self.origin_pose.y
        rel_x = dx * math.cos(-self.start_yaw) - dy * math.sin(-self.start_yaw)
        rel_y = dx * math.sin(-self.start_yaw) + dy * math.cos(-self.start_yaw)

        if self.state == 0:  # First Straight
            dist = self.get_distance(self.start_pose, self.current_pose)
            cmd.linear.x = self.update_speed()
            
            if dist < self.get_parameter('target_1_dist').value:
                self.get_logger().info(f"P0: {dist:.2f}m", throttle_duration_sec=0.5)
                self.cmd_vel_pub_.publish(cmd)
            else:
                self.state = 1
                self.current_state_start_yaw = self.yaw
                self.get_logger().info("Phase 1: Turning")

        elif self.state == 1:  # Turn
            angle_turned = abs(self.yaw - self.current_state_start_yaw)
            if angle_turned > math.pi:
                angle_turned = abs(angle_turned - 2 * math.pi)

            if angle_turned < self.get_parameter('target_angle').value:
                cmd.linear.x = self.current_linear_speed 
                cmd.angular.z = self.turn_speed
                self.get_logger().info(f"P1: {math.degrees(angle_turned):.1f}°", throttle_duration_sec=0.5)
                self.cmd_vel_pub_.publish(cmd)
            else:
                self.state = 2
                self.start_pose = self.current_pose
                self.get_logger().info("Phase 2: Final Straight")

        elif self.state == 2:  # Second Straight
            dist = self.get_distance(self.start_pose, self.current_pose)
            cmd.linear.x = self.current_linear_speed 
            
            if dist < self.get_parameter('target_2_dist').value:
                self.get_logger().info(f"P2: {dist:.2f}m", throttle_duration_sec=0.5)
                self.cmd_vel_pub_.publish(cmd)
            else:
                # Target Reached - Switch to COASTING
                self.state = 3
                self.stop_time = self.get_clock().now()
                self.stop_motors()
                self.get_logger().info("Target reached. Measuring coasting drift...")

        elif self.state == 3:  # Coasting/Braking Phase
            self.stop_motors()
            elapsed_wait = (self.get_clock().now() - self.stop_time).nanoseconds / 1e9
            self.get_logger().info(
                f"Coasting {elapsed_wait:.1f}s | X: {rel_x:.3f} m | Y: {rel_y:.3f} m",
                throttle_duration_sec=0.25
            )

            if elapsed_wait >= self.stop_duration:
                self.report_final_results()
                self.state = 4

    def stop_motors(self):
        cmd = Twist()
        self.cmd_vel_pub_.publish(cmd)

    def report_final_results(self):
        # Final transformation calculation
        dx = self.current_pose.x - self.origin_pose.x
        dy = self.current_pose.y - self.origin_pose.y
        final_rel_x = dx * math.cos(-self.start_yaw) - dy * math.sin(-self.start_yaw)
        final_rel_y = dx * math.sin(-self.start_yaw) + dy * math.cos(-self.start_yaw)
        
        # Heading calculation
        final_angle_rad = self.yaw - self.start_yaw
        # Normalize angle to -pi to pi
        final_angle_rad = (final_angle_rad + math.pi) % (2 * math.pi) - math.pi

        self.get_logger().info("====================================")
        self.get_logger().info("FINAL VALIDATION REPORT (Post-Drift)")
        self.get_logger().info(f"Final Rel X:  {final_rel_x:.3f} m")
        self.get_logger().info(f"Final Rel Y:  {final_rel_y:.3f} m")
        self.get_logger().info(f"Final Heading: {math.degrees(final_angle_rad):.1f}°")
        self.get_logger().info("====================================")
        
        # Trigger clean exit
        sys.exit(0)

def main(args=None):
    rclpy.init(args=args)
    node = SteeringTest()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, SystemExit):
        pass
    finally:
        node.stop_motors()
        node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    main()