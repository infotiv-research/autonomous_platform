#!/bin/bash

# Initialize startup report
REPORT_FILE="startup_report.txt"
echo "---- Startup Report: $(date) ----" > $REPORT_FILE

# Global timer
START=$SECONDS

task_status() {
    local TASK_NAME=$1
    local EXIT_CODE=$2
    local DURATION=$((SECONDS - START))
    local MSG

    if [ "$EXIT_CODE" -eq 0 ]; then
        MSG="[  OK  ]"
    else
        MSG="[FAILED] (Code: $EXIT_CODE)"
    fi

    # Save to file
    printf "%-30s %-15s %ss\n" "$TASK_NAME" "$MSG" "$DURATION" >> "$REPORT_FILE"

    #Reset timer
    START=$SECONDS
}

# Initialise CAN
sudo ip link set can0 up type can bitrate 1000000
sudo ip link set can0 txqueuelen 1000
task_status "CAN initialisation" $?

# Check Internet
ping -c 1 8.8.8.8 >/dev/null
task_status "Internet connectivity" $?


# Docker initialisation
docker compose down
docker compose build
task_status "Docker build" $?
docker compose up -d
task_status "Docker up" $?

# Check for docker chrash
sleep 2
if [ "$(docker ps -q -f status=running -f name=ap4hwi)" ]; then
    task_status "Docker container running" 0
else
    task_status "Docker container running" 1
fi

# Check CAN IDs transmission
CAN_OUTPUT=$(docker exec ap4hwi bash -lc "timeout 10s candump can0 2>/dev/null")

CAN_FAIL=0
MISSING_IDS=""

for ID in 5DC 7D0; do
    if ! echo "$CAN_OUTPUT" | grep -qw "$ID"; then
        CAN_FAIL=1
        MISSING_IDS="$MISSING_IDS $ID"
    fi
done

task_status "CAN ID transmission" $CAN_FAIL

if [ $CAN_FAIL -ne 0 ]; then
    {
        echo "---- CAN transmission check ----"
        echo "Missing CAN IDs:$MISSING_IDS"
        echo "Expected CAN IDs: 5DC (Speed Sensor frame) 7D0 (SPCU feedback)"
        echo "--------------------------------"
    } >> "$REPORT_FILE"
fi

# Script for checking ROS2 network
docker exec ap4hwi bash -lc "
    cd ap4hwi_ws &&
    source /opt/ros/humble/setup.bash &&
    source install/setup.bash &&
    ros2 run AP4_init_test_package init_check
  "

# Explanation for bitmasking errorcodes only appends uppon failure
ROS_EXIT_CODE=$?
if [ $ROS_EXIT_CODE -ne 0 ]; then
    {
    echo "---- ROS2 Bitmasking error codes guide ----"
    echo "      1-Nodes, 2-Topics, 4-Steering"
    echo "3-Nodes&Topics, 5-Steering&Nodes, 6-Steering&Topics"
    echo "                  7-All"
    echo "-------------------------------------------"
    }>> $REPORT_FILE
fi 
task_status "ROS2 Network" $ROS_EXIT_CODE

# Script for checking Sensors
docker exec ap4hwi bash -lc "
    cd ap4hwi_ws &&
    source /opt/ros/humble/setup.bash &&
    source install/setup.bash &&
    ros2 run AP4_init_test_package sensor_check
  "

# Explanation for bitmasking errorcodes only appends uppon failure
        # Error Bitmask Mapping
        # 1  = Speed Sensors (Any of the 4 pulse counts)
        # 2  = IMU
        # 4  = LiDAR (Scan)
        # 8  = Steering Angle
SENSOR_EXIT_CODE=$?
if [ $SENSOR_EXIT_CODE -ne 0 ]; then
    {
    echo "---- Sensor Bitmasking error codes guide ----"
    echo "          1-Speed sensors, 2-IMU"
    echo "           4-LIDAR,  8-Steering"
    echo "-------------------------------------------"
    }>> $REPORT_FILE
fi 
task_status "Sensors" $SENSOR_EXIT_CODE