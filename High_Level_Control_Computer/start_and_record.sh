#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONTAINER_NAME="ap4hlc"
ROS_DOMAIN_ID="${ROS_DOMAIN_ID:-1}"
HOST_UID="$(id -u)"
HOST_GID="$(id -g)"
HOST_BAG_DIR="${SCRIPT_DIR}/ap4_hlc_code/recorded_data/odometry_test_bags"
CONTAINER_BAG_DIR="/root/ap4_hlc_docker_dir/recorded_data/odometry_test_bags"
PID_FILE_CONTAINER="/tmp/odometry_test_rosbag_record.pid"
RECORD_LOG_FILE_CONTAINER="/tmp/odometry_test_rosbag_record.log"
RECORD_TOPIC="/odometry_test"
RESET_TOPIC="/reset_odom"
START_TIMEOUT_SECONDS=20
recording_active=0
current_bag_name=""

usage() {
    cat <<EOF
Usage:
  ./start_and_record.sh [--container ap4hlc]

This script:
  1. Assumes the HLC workspace inside the running Docker is already built.
  2. Resets the low-level odometry publisher over ${RESET_TOPIC}.
  3. Starts launch_robot.launch.py inside the container.
  4. Starts odometry_test_pkg/odometry_test inside the container.
  5. Waits for Enter to start recording /odometry_test.
  6. Waits for Enter again to stop recording and save the rosbag on the host.
EOF
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Required command not found: $1" >&2
        exit 1
    fi
}

run_in_container_terminal() {
    local title="$1"
    local command_text="$2"
    local escaped_command

    printf -v escaped_command '%q' "${command_text}"

    if command -v gnome-terminal >/dev/null 2>&1 && [[ -n "${DISPLAY:-}" ]]; then
        gnome-terminal \
            --title="${title}" \
            -- /bin/bash -lc "docker exec -it ${CONTAINER_NAME} /bin/bash -lc ${escaped_command}"
    else
        echo "gnome-terminal not available. Starting '${title}' detached in ${CONTAINER_NAME}."
        docker exec -d "${CONTAINER_NAME}" /bin/bash -lc "${command_text}"
    fi
}

container_has_record_topic() {
    docker exec "${CONTAINER_NAME}" /bin/bash -lc "
        cd /root/ap4_hlc_docker_dir/ap4hlc_ws
        source /opt/ros/humble/setup.bash
        source install/setup.bash
        export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
        topic_list=\$(ros2 topic list 2>/dev/null || true)
        grep -Fxq ${RECORD_TOPIC@Q} <<< \"\${topic_list}\"
    "
}

container_record_topic_has_publisher_and_subscriber() {
    docker exec "${CONTAINER_NAME}" /bin/bash -lc "
        cd /root/ap4_hlc_docker_dir/ap4hlc_ws
        source /opt/ros/humble/setup.bash
        source install/setup.bash
        export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
        topic_info=\$(ros2 topic info ${RECORD_TOPIC@Q} 2>/dev/null || true)
        [[ \"\${topic_info}\" =~ Publisher\ count:\ [1-9] ]] && [[ \"\${topic_info}\" =~ Subscription\ count:\ [1-9] ]]
    "
}

container_reset_topic_has_subscriber() {
    docker exec "${CONTAINER_NAME}" /bin/bash -lc "
        cd /root/ap4_hlc_docker_dir/ap4hlc_ws
        source /opt/ros/humble/setup.bash
        source install/setup.bash
        export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
        topic_info=\$(ros2 topic info ${RESET_TOPIC@Q} 2>/dev/null || true)
        [[ \"\${topic_info}\" =~ Subscription\ count:\ [1-9] ]]
    "
}

reset_low_level_odom() {
    echo "Waiting for a ${RESET_TOPIC} subscriber..."
    for _ in $(seq 1 "${START_TIMEOUT_SECONDS}"); do
        if container_reset_topic_has_subscriber; then
            break
        fi
        sleep 1
    done

    if ! container_reset_topic_has_subscriber; then
        echo "No subscriber detected on ${RESET_TOPIC}. Refusing to start EKF/SLAM without resetting low-level odom first." >&2
        exit 1
    fi

    echo "Resetting low-level odom before starting launch_robot.launch.py..."
    docker exec "${CONTAINER_NAME}" /bin/bash -lc "
        cd /root/ap4_hlc_docker_dir/ap4hlc_ws
        source /opt/ros/humble/setup.bash
        source install/setup.bash
        export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
        ros2 topic pub --rate 5 --times 5 ${RESET_TOPIC@Q} std_msgs/msg/Empty '{}'
    "
}

print_record_log_tail() {
    docker exec "${CONTAINER_NAME}" /bin/bash -lc \
        "if [[ -f ${RECORD_LOG_FILE_CONTAINER} ]]; then echo 'ros2 bag log:'; tail -n 20 ${RECORD_LOG_FILE_CONTAINER}; fi" \
        2>/dev/null || true
}

start_recording() {
    local record_cmd

    if [[ "${recording_active}" -eq 1 ]]; then
        echo "Rosbag recording is already running."
        return 0
    fi

    mkdir -p "${HOST_BAG_DIR}"
    current_bag_name="odometry_test_$(date +%Y%m%d_%H%M%S)"

    echo "Waiting for ${RECORD_TOPIC} to be available..."
    for _ in $(seq 1 "${START_TIMEOUT_SECONDS}"); do
        if container_has_record_topic; then
            break
        fi
        sleep 1
    done

    if ! container_has_record_topic; then
        echo "Topic ${RECORD_TOPIC} did not appear in time." >&2
        exit 1
    fi

    record_cmd=$(cat <<EOF
cd /root/ap4_hlc_docker_dir/ap4hlc_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
mkdir -p ${CONTAINER_BAG_DIR}
cd ${CONTAINER_BAG_DIR}
rm -f ${RECORD_LOG_FILE_CONTAINER}
ros2 bag record -o ${current_bag_name} ${RECORD_TOPIC} >${RECORD_LOG_FILE_CONTAINER} 2>&1 &
echo \$! > ${PID_FILE_CONTAINER}
wait \$!
rm -f ${PID_FILE_CONTAINER}
EOF
)

    docker exec -d "${CONTAINER_NAME}" /bin/bash -lc "${record_cmd}"

    for _ in $(seq 1 "${START_TIMEOUT_SECONDS}"); do
        if docker exec "${CONTAINER_NAME}" /bin/bash -lc \
            "test -s ${PID_FILE_CONTAINER} && kill -0 \$(cat ${PID_FILE_CONTAINER}) 2>/dev/null"; then
            if container_record_topic_has_publisher_and_subscriber; then
                if docker exec "${CONTAINER_NAME}" /bin/bash -lc \
                    "find ${CONTAINER_BAG_DIR}/${current_bag_name} -type f \\( -name '*.db3' -o -name '*.mcap' \\) -size +0c | grep -q ."; then
                    recording_active=1
                    echo "Recording started."
                    echo "Bag will be saved to ${HOST_BAG_DIR}/${current_bag_name}"
                    return 0
                fi
            fi
        fi
        sleep 1
    done

    echo "Failed to confirm rosbag data recording on ${RECORD_TOPIC}." >&2
    print_record_log_tail >&2
    exit 1
}

stop_recording() {
    if [[ "${recording_active}" -ne 1 ]]; then
        return 0
    fi

    echo "Stopping rosbag recording..."

    docker exec "${CONTAINER_NAME}" /bin/bash -lc \
        "if [[ -f ${PID_FILE_CONTAINER} ]]; then kill -INT \$(cat ${PID_FILE_CONTAINER}); fi" \
        >/dev/null 2>&1 || true

    for _ in $(seq 1 30); do
        if ! docker exec "${CONTAINER_NAME}" /bin/bash -lc \
            "if [[ -f ${PID_FILE_CONTAINER} ]]; then kill -0 \$(cat ${PID_FILE_CONTAINER}) 2>/dev/null; else exit 1; fi" \
            >/dev/null 2>&1; then
            break
        fi
        sleep 1
    done

    docker exec "${CONTAINER_NAME}" /bin/bash -lc "rm -f ${PID_FILE_CONTAINER}" >/dev/null 2>&1 || true
    docker exec "${CONTAINER_NAME}" /bin/bash -lc \
        "if [[ -d ${CONTAINER_BAG_DIR}/${current_bag_name} ]]; then chown -R ${HOST_UID}:${HOST_GID} ${CONTAINER_BAG_DIR}/${current_bag_name}; fi" \
        >/dev/null 2>&1 || true

    recording_active=0
    echo "Rosbag saved to ${HOST_BAG_DIR}/${current_bag_name}"
}

handle_interrupt() {
    echo
    stop_recording
    exit 130
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --container)
            shift
            CONTAINER_NAME="${1:-}"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

require_command docker

trap handle_interrupt INT TERM

if [[ "$(docker inspect -f '{{.State.Running}}' "${CONTAINER_NAME}" 2>/dev/null || true)" != "true" ]]; then
    echo "Container '${CONTAINER_NAME}' is not running." >&2
    echo "Start it first with 'docker compose up -d' from High_Level_Control_Computer." >&2
    exit 1
fi

if ! docker exec "${CONTAINER_NAME}" /bin/bash -lc "test -f /root/ap4_hlc_docker_dir/ap4hlc_ws/install/setup.bash"; then
    echo "Missing /root/ap4_hlc_docker_dir/ap4hlc_ws/install/setup.bash inside '${CONTAINER_NAME}'." >&2
    echo "Build the workspace once before using this launcher." >&2
    exit 1
fi

COMMON_SETUP=$(cat <<EOF
cd /root/ap4_hlc_docker_dir/ap4hlc_ws
source /opt/ros/humble/setup.bash
source install/setup.bash
export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}
EOF
)

ROBOT_LAUNCH_CMD=$(cat <<EOF
${COMMON_SETUP}
ros2 launch autonomous_platform_robot_description_pkg launch_robot.launch.py
EOF
)

ODOMETRY_TEST_CMD=$(cat <<EOF
${COMMON_SETUP}
ros2 run odometry_test_pkg odometry_test
EOF
)

reset_low_level_odom

echo "Starting launch_robot.launch.py..."
run_in_container_terminal "AP4 Robot Launch" "${ROBOT_LAUNCH_CMD}"

sleep 2

echo "Starting odometry_test_pkg/odometry_test..."
run_in_container_terminal "AP4 Odometry Test" "${ODOMETRY_TEST_CMD}"

echo
read -r -p "Press Enter in this terminal to start recording /odometry_test..."
start_recording

echo
read -r -p "Press Enter again to stop recording and save the bag..."
stop_recording
