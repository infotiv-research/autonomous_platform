#!/bin/bash

check_topics() {
    # List of required topics
    required_topics=("/cmd_vel_stamped" "/color/image" "/stereo/depth" "/imu")

    # Check if all required topics are present
    for topic in "${required_topics[@]}"; do
        if ! ros2 topic list | grep -q "$topic"; then
            echo "$topic is missing"
            return 1  # Topic not found, return failure
        fi
    done
    return 0  # All topics found, return success
}

flag=${1:-1}


cd ap4hlc_ws
source install/setup.bash

echo "Setting ros domain to 1"
export ROS_DOMAIN_ID=1
#echo "Printing topic list"
#ros2 topic list

echo "Waiting for topics "/cmd_vel_stamped" "/color/image" "/stereo/depth" "/imu" to appear..."
while ! check_topics; do
    sleep 1  # Wait for 1 second before checking again
done
echo "All topics found"
sleep 1

echo "Starting data collection"
ros2 run high_level_control data_collection.py --param ${flag}


