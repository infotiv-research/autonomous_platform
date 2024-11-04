#!/bin/bash

docker run -it --net host \
    -v /dev/:/dev/ \
    --privileged \
    -e DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e ROS_DOMAIN_ID=1 \
    depthai-ros ros2 launch depthai_examples stereo_inertial_node.launch.py \
    stereo_fps:=10 \
    rgbResolution:="12MP" \
    monoResolution:="400p" \
    enableRviz:=False \
    enableSpatialDetection:=False \
    syncNN:=False \
    rgbScaleNumerator:=2 \
    rgbScaleDinominator:=39 \

    