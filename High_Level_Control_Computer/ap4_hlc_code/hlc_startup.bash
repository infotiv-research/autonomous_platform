#!/bin/bash

cd ap4hlc_ws
source /opt/ros/humble/setup.bash
colcon build
source install/setup.bash
export ROS_DOMAIN_ID=1
ros2 launch autonomous_platform_robot_description_pkg launch_digital_twin_simulation.launch.py &
ros2 run twist_stamper twist_stamper --ros-args -r cmd_vel_in:=/joystick_cmd_vel -r cmd_vel_out:=cmd_vel_stamped -p frame_id:=my_frame