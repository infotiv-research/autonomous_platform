import time
import rclpy
import sys
from rclpy.node import Node

from std_msgs.msg import String
from std_msgs.msg import Int16
from geometry_msgs.msg import Twist


class MinimalPublisher(Node):

    def __init__(self):
        super().__init__("minimal_publisher")
        self.publisher_ = self.create_publisher(String, "topic", 10)

        # Subscribing and publishing topics for steering
        self.set_steering_angle_pub_ = self.create_publisher(Twist, "/camera_cmd_vel", 10)
        self.steering_angle_sub_ = self.create_subscription(
            Int16, "/GET_0x7d0_Get_SteeringAngle", self.steering_angle_callback, 10
        )
        self.last_steering = 0

    # *
    # Method which checks that important nodes are started
    # List of nodes that should be up has to be changed by a developer
    # *#
    def check_ros_node_is_up(self):
        active_nodes = super().get_node_names()
        nodes_should_be_up = [
            "can_ros2_interface_node",
            "socket_can_receiver",
            "socket_can_sender",
            "twist_mux",
            "ackermann_odometry_node",
            "feed_forward_ctrl_node"
        ]

        # returns true if all nodes that should be up are a subset of the list of nodes that should be up
        nodes_up_ok = all(node in active_nodes for node in nodes_should_be_up)

        # Get a list of nodes that are not started, which should be started
        list_nodes_not_started = []
        if not nodes_up_ok:
            for node_not_up in nodes_should_be_up:
                if node_not_up not in active_nodes:
                    list_nodes_not_started.append(node_not_up)

        return (nodes_up_ok, list_nodes_not_started)

    def check_ros_topics_up(self):
        topic_data = super().get_topic_names_and_types()
        active_topics =  [t[0] for t in topic_data]
        topics_should_be_up = [
            "/cmd_vel",
            "/to_can_bus",
            "/from_can_bus",
        ]
        
        topics_up_ok = all(topic in active_topics for topic in topics_should_be_up)


        # Get a list of topics that are not started, which should be started
        list_topics_not_started = []
        if not topics_up_ok:
            for topic_not_up in topics_should_be_up:
                if topic_not_up not in active_topics:
                    list_topics_not_started.append(topic_not_up)

        return (topics_up_ok, list_topics_not_started)

    # *
    # Sets a desired steering angle
    # waits for at most 5 seconds
    # reads steering angle
    # checks if desired steering angle could be reached
    # *#
    def check_steering_ok(self):
        def wait_until(condition, cmd, timeout_sec = 5.0, publish_freq=10.0, hold_sec = 2.0):
            start_time = self.get_clock().now()
            max_wait = rclpy.duration.Duration(seconds=timeout_sec,nanoseconds=0)
            period_sec = 1.0/publish_freq
            
            while(self.get_clock().now() - start_time) < max_wait:
                self.set_steering_angle_pub_.publish(cmd)
                rclpy.spin_once(self,timeout_sec=period_sec)
                if condition():
                    #hold position such that it is visible
                    hold_start = self.get_clock().now()
                    hold_duration = rclpy.duration.Duration(seconds=hold_sec,nanoseconds=0)
                    while(self.get_clock().now()-hold_start) < hold_duration:
                        self.set_steering_angle_pub_.publish(cmd)
                        rclpy.spin_once(self, timeout_sec=period_sec)
                    return True
            return False
        
        left_cmd = Twist()
        left_cmd.angular.z = 1.0
        left_ok = wait_until(lambda: self.last_steering >= 22, left_cmd, hold_sec = 2.0)

        right_cmd =  Twist()
        right_cmd.angular.z = -1.0
        right_ok = wait_until(lambda: self.last_steering <= -22,right_cmd,hold_sec=2.0)
        

        center_cmd = Twist()
        center_cmd.angular.z = 0.0
        center_ok = wait_until(lambda: -5 <= self.last_steering <= 5, center_cmd, timeout_sec = 5.0,hold_sec=2.0)

        return left_ok and right_ok and center_ok

    def steering_angle_callback(self, msg):
        self.last_steering = msg.data
        print("RECIEVED CALLBACK!!!!!!!")

    def run_diagnostics(self):
        start_time = self.get_clock().now()
        timeout = rclpy.duration.Duration(seconds=90.0)
        print("Starting diagnostics... will retry for 90 seconds.")

        while (self.get_clock().now() - start_time) < timeout:
            exitflag = 0

            # Checking nodes
            nodes_ok, nodes_missing  = self.check_ros_node_is_up()
            if nodes_ok:
                print("All important nodes are up")
            else:
                print("Nodes that are missing: ", nodes_missing)
                exitflag = 1

            # Checking topics
            topics_ok, topics_missing = self.check_ros_topics_up()
            if topics_ok:
                print("All important topics are up")
            else:
                print("Topics that are missing: ", topics_missing)
                exitflag += 2

            # Break timout loop if check did not fail
            if exitflag == 0:
                break

            print("Current status:", exitflag, " Retrying in 2 seconds")
            time.sleep(2.0)
        
        # Checking steering
        if exitflag == 0:
            if self.check_steering_ok():
                print("Steering is ok")
            else:
                print("Steering failed")
                exitflag += 4
        return exitflag
    
#-- ROS2 error: 1-Nodes, 2-Topics, 4-Steering --
#-- 3-Nodes&Topics, 5-Steering&Nodes, 6-Steering&Topics --
#-- 7-All--


def main(args=None):
    rclpy.init(args=args)

    # Create node
    minimal_publisher = MinimalPublisher()

    # Catch exit flag
    exit_flag = minimal_publisher.run_diagnostics()

    print("Diagnostics complete, Shutting down node.")
    minimal_publisher.destroy_node()
    rclpy.shutdown()
    sys.exit(exit_flag)


if __name__ == "__main__":
    main()
