import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu, LaserScan
from std_msgs.msg import Int16, UInt16
import sys

class SensorChecker(Node):
    def __init__(self):
        super().__init__('sensor_checker')
        
        # Track status of each topic
        self.status = {
            "speed_rr": False, "speed_lf": False, 
            "speed_lr": False, "speed_rf": False,
            "imu": False, "scan": False, "steering": False
        }

        # Topic configuration: (Topic Name, Message Type)
        topics_to_check = {
            "speed_rr": ("/GET_0x5dc_SpeedSensorRR_PulseCnt", UInt16),
            "speed_lf": ("/GET_0x5dc_SpeedSensorLF_PulseCnt", UInt16),
            "speed_lr": ("/GET_0x5dc_SpeedSensorLR_PulseCnt", UInt16),
            "speed_rf": ("/GET_0x5dc_SpeedSensorRF_PulseCnt", UInt16),
            "imu": ("/imu", Imu),
            "scan": ("/scan", LaserScan),
            "steering": ("/GET_0x7d0_Get_SteeringAngle", Int16)
        }

        self.subs = []
        qos = 10

        # Create specific subscribers in a loop
        for key, (topic_name, msg_type) in topics_to_check.items():
            # We use a default argument 'k=key' in the lambda to capture the current key 
            # otherwise all subscribers would point to the last 'key' in the loop
            sub = self.create_subscription(
                msg_type,
                topic_name,
                lambda msg, k=key: self.mark(k),
                qos
            )
            self.subs.append(sub)

    def mark(self, key):
        if not self.status[key]:
            self.status[key] = True
            self.get_logger().info(f"Detected: {key}")

    def all_sensors_found(self):
        return all(self.status.values())

    def calculate_error_code(self):
        code = 0
        speed_ok = all([self.status["speed_rr"], self.status["speed_lf"], 
                        self.status["speed_lr"], self.status["speed_rf"]])
        
        if not speed_ok:
            code += 1
            print("FAIL: One or more Speed Sensors missing.")
        if not self.status["imu"]:
            code += 2
            print("FAIL: IMU data missing.")
        if not self.status["scan"]:
            code += 4
            print("FAIL: LiDAR (scan) data missing.")
        if not self.status["steering"]:
            code += 8
            print("FAIL: Steering Angle data missing.")
            
        return code

def main():
    rclpy.init()
    node = SensorChecker()
    
    print("Monitoring topics (Max 30s)...")
    start = node.get_clock().now()
    timeout_ns = 30 * 1e9
    
    while (node.get_clock().now() - start).nanoseconds < timeout_ns:
        rclpy.spin_once(node, timeout_sec=0.1)
        if node.all_sensors_found():
            print("All sensors detected early!")
            break

    exit_code = node.calculate_error_code()
    node.destroy_node()
    rclpy.shutdown()
    sys.exit(exit_code)

if __name__ == "__main__":
    main()