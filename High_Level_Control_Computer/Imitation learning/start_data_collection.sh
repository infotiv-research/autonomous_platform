#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CAMERA_SCRIPT="${SCRIPT_DIR}/launch_depthai_ros.sh"

usage() {
    echo "Usage: $0 --param {color|depth|orb|default|validation}"
    exit 1
}

# Initialize param_value with default value
param_value="default"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    key="$1"

    case $key in
        --param)
            param_value="$2"
            shift # past argument
            shift # past value
            ;;
        *)  # unknown option
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

# Validate param_value
if [[ "$param_value" != "color" && "$param_value" != "depth" && "$param_value" != "orb" && "$param_value" != "default" && "$param_value" != "validation" ]]; then
    echo "Invalid parameter: $param_value, use:  --param {color|depth|orb|default|validation}"
    usage
fi

# Launch camera Docker
gnome-terminal --title='Camera Docker' -- /bin/bash -c 'exec "$1"' _ "${CAMERA_SCRIPT}"

# Launch high level model if param_value is "color", "depth" or "orb"
if [[ "$param_value" == "color" || "$param_value" == "depth" || "$param_value" == "orb" ]]; then
    gnome-terminal --title='High level model' -- /bin/bash -c "docker exec -it ap4hlc /bin/bash -c \"source autonomous_drive_startup.bash $param_value\""
fi

# Launch data collection
gnome-terminal --title='Data collection' -- /bin/bash -c "docker exec -it ap4hlc /bin/bash -c \"source data_collection_startup.bash $param_value\""

sleep 3
# Launch ROS bag
gnome-terminal --title='ROS bag' -- /bin/bash -c "docker exec -it ap4hlc /bin/bash -c \"source rosbag_startup.bash $param_value\""

# Launch test script
gnome-terminal --title='Test script' -- /bin/bash -c 'docker exec -it ap4hlc /bin/bash -c "source test_script.bash"'
