import rclpy
from rclpy.node import Node

from geometry_msgs.msg import Twist  # /cmd_vel topic
from sensor_msgs.msg import Joy
from std_msgs.msg import Int8
from std_msgs.msg import UInt8
from std_msgs.msg import UInt16


class MinimalPublisher(Node):

    def __init__(self):
        super().__init__("feed_forward_ctrl_node")

        # Create publishers with topics out from feed forward controller
        self.publisher_throttle_ = self.create_publisher(
            UInt16, "/SET_0x3e8_Act_ThrottleVoltage", 10
        )
        self.publisher_break_ = self.create_publisher(
            UInt16, "/SET_0x3e8_Act_BreakVoltage", 10
        )
        self.publisher_steering_position_ = self.create_publisher(
            Int8, "/SET_0x3e8_Act_SteeringPosition", 10
        )
        self.publisher_reverse_mode_ = self.create_publisher(
            UInt8, "/controller_reverse_mode", 10
        )

        # create timed publish rate of commands
        timer_period = 1 / 20
        self.timer_ = self.create_timer(timer_period, self.TimerCallback)

        # constants, maximum and minimum millivolts
        self.max = 4350
        self.min = 850
        self.max_cmd_speed_ = 0.35  # speed cap between 0 and 0.5
        self.min_forward_linear_speed_mps_ = (
            self.declare_parameter("min_forward_linear_speed_mps", 0.14)
            .get_parameter_value()
            .double_value
        )

        # internal storage of variables to publish
        self.saved_throttlevoltage_ = self.min
        self.saved_brakevoltage_ = self.min
        self.saved_steeringangle_ = 0
        self.requested_linear_speed_ = 0.0

        # Skeleton reverse state driven directly from Xbox A button.
        self.saved_reverse_mode_ = 0
        self.reverse_button_index_ = 0  # Xbox A button
        self.reverse_button_prev_pressed_ = False
        self.brake_pulse_sequence_ = [3100, 850, 3100]
        self.brake_pulse_index_ = 0
        self.brake_pulse_active_ = False

        # create subscribers
        # Listen to /cmd_vel topic, once cmd_vel message recieved, perform
        # controller action
        self.subscriber_cmd_vel_ = self.create_subscription(
            Twist, "/cmd_vel", self.Callback_cmd_vel, 10
        )
        self.subscriber_joy_ = self.create_subscription(
            Joy, "/joy", self.Callback_joy, 10
        )

        # TODO listen to wheel speed sensor messages
        # to perform feedback control
        # self.subscriber_vehicle_speed_ = ...
        self.get_logger().info(
            "Feed-forward speed cap %.2f m/s, min forward speed %.2f m/s"
            % (
                self.max_cmd_speed_,
                self.min_forward_linear_speed_mps_,
            )
        )

    def ThrottleVoltageFeedForward(self, linear_speed):
        x_dot = self.ClampLinearSpeed(linear_speed)
        kp = self.max / 0.7
        x1_dot = x_dot * kp

        # limit range of output voltage between 850 and 4350 mV
        if x1_dot < self.min:
            x1_dot = self.min
        elif x1_dot > self.max:
            x1_dot = self.max

        self.saved_throttlevoltage_ = int(x1_dot)

    def BreakVoltageFeedForward(self, linear_speed):
        x_dot = self.ClampLinearSpeed(linear_speed)
        kp = -self.max / 0.7
        x1_dot = x_dot * kp

        # limit range of output voltage between 850 and 4350 mV
        if x1_dot < self.min:
            x1_dot = self.min
        elif x1_dot > self.max:
            x1_dot = self.max

        self.saved_brakevoltage_ = int(x1_dot)

    def SteeringFeedForward(self, msg):
        max_steering = 22.0
        min_steering = -22.0

        rz = msg.angular.z
        # Match the standard ROS/HLC convention: left is positive, right is
        # negative.
        kp = 90

        y = rz * kp

        # constrain steering value between min and max
        if y > max_steering:
            y = max_steering
        elif y < min_steering:
            y = min_steering

        # remove any rounding errors close to zero
        dead_space = 3  # degrees
        if y > 0 and y < dead_space:
            y = 0
        elif y < 0 and y > -dead_space:
            y = 0

        self.saved_steeringangle_ = y

    def ClampLinearSpeed(self, linear_x):
        min_forward_speed = min(
            self.max_cmd_speed_,
            max(0.0, self.min_forward_linear_speed_mps_),
        )

        if linear_x > self.max_cmd_speed_:
            return self.max_cmd_speed_
        if linear_x < -self.max_cmd_speed_:
            return -self.max_cmd_speed_
        if 0.0 < linear_x < min_forward_speed:
            return min_forward_speed
        return linear_x

    def UpdatePropulsionFeedForward(self):
        self.ThrottleVoltageFeedForward(self.requested_linear_speed_)
        self.BreakVoltageFeedForward(self.requested_linear_speed_)

    # *
    # Callback function
    # msg is of type geometry_msgs/msg/Twist
    # *#
    def Callback_cmd_vel(self, msg):
        self.requested_linear_speed_ = self.ClampLinearSpeed(msg.linear.x)
        self.SteeringFeedForward(msg)

    def Callback_joy(self, msg):
        button_pressed = (
            self.reverse_button_index_ < len(msg.buttons)
            and msg.buttons[self.reverse_button_index_] == 1
        )
        if button_pressed and not self.reverse_button_prev_pressed_:
            self.saved_reverse_mode_ = 0 if self.saved_reverse_mode_ else 1
            self.brake_pulse_index_ = 0
            self.brake_pulse_active_ = True
        self.reverse_button_prev_pressed_ = button_pressed

    def TimerCallback(self):
        self.UpdatePropulsionFeedForward()

        # throttle
        out_msg = UInt16()
        out_msg.data = self.saved_throttlevoltage_
        self.publisher_throttle_.publish(out_msg)

        # break
        brake_voltage = self.saved_brakevoltage_
        if self.brake_pulse_active_:
            brake_voltage = self.brake_pulse_sequence_[self.brake_pulse_index_]
            self.brake_pulse_index_ += 1
            if self.brake_pulse_index_ >= len(self.brake_pulse_sequence_):
                self.brake_pulse_active_ = False
                self.brake_pulse_index_ = 0

        out_msg = UInt16()
        out_msg.data = int(brake_voltage)
        self.publisher_break_.publish(out_msg)

        # steering
        out_msg = Int8()
        out_msg.data = int(self.saved_steeringangle_)
        self.publisher_steering_position_.publish(out_msg)

        # Skeleton reverse-mode publisher: toggles on each A press.
        out_msg = UInt8()
        out_msg.data = int(self.saved_reverse_mode_)
        self.publisher_reverse_mode_.publish(out_msg)


def main(args=None):
    rclpy.init(args=args)

    minimal_publisher = MinimalPublisher()

    rclpy.spin(minimal_publisher)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    minimal_publisher.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
