#/bin/bash

echo "Running Hardware Interface docker startup bash script"
echo "Entering ap4hwi_ws..."
cd ap4hwi_ws
echo "Sourcing ros humbe..."
source /opt/ros/humble/setup.bash
echo "Building ROS packages..-"
colcon build
echo "Sourcing built packages..."
source install/setup.bash
echo "Setting ROS_DOMAIN_ID=1..."
export ROS_DOMAIN_ID=1
echo "Running launch_hwi_software_pkg launch file..."
ros2 launch launch_hwi_software_pkg launch_hwi_ros_software.launch.py &
echo "Starting twist_mux"
ros2 run twist_mux twist_mux --ros-args --params-file ../twist_mux_topics.yaml -r cmd_vel_out:=cmd_vel 
echo "Finnished startup script, exitng..."





