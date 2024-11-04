#!/bin/bash

check_topics() {
    # List of required topics
    required_topics=("/color/image")

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

if [[ $flag == "color" ]]; then
    echo "The input is set to color image and IMU"
elif [[ $flag == "depth" ]]; then 
    echo "The input is set to depth image and IMU"
elif [[ $flag == "orb" ]]; then
    echo "The input is set to color image,IMU and orbs"
fi 

cd ap4hlc_ws
source install/setup.bash

echo "Setting ros domain to 1"
export ROS_DOMAIN_ID=1
sleep 1
#echo "Printing topic list"
#ros2 topic list

echo "Waiting for topic "/color/image" to appear..."
while ! check_topics; do
    sleep 1  # Wait for 1 second before checking again
done
echo "All topics found"
echo "Starting autonomous drive"
sleep 1

ros2 run high_level_control high_level_model.py --param ${flag}