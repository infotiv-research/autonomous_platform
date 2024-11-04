FROM ros:humble-ros-core-jammy

# install bootstrap tools
RUN apt-get update && apt-get install --no-install-recommends -y \
    iputils-ping traceroute netcat iperf3 \
    libcanberra-gtk-module \
    libcanberra-gtk3-module \
    at-spi2-core \
    build-essential \
    ros-humble-twist-mux \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool \
    python3-pandas \
    python3-pip \
	iputils-ping \
    net-tools \
    wget \
    screen \
    nano \
    vim \
    htop \
    alsa-base \
    alsa-utils \
    usbutils \
    && rm -rf /var/lib/apt/lists/* \
    && pip install imitation \
    && pip install stable-baselines3

# bootstrap rosdep
RUN rosdep init && \
  rosdep update --rosdistro $ROS_DISTRO

# setup colcon mixin and metadata
RUN colcon mixin add default \
      https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml && \
    colcon mixin update && \
    colcon metadata add default \
      https://raw.githubusercontent.com/colcon/colcon-metadata-repository/master/index.yaml && \
    colcon metadata update

# install ros2 packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-desktop \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-teleop-twist-keyboard \
    ros-humble-joint-state-publisher-gui \
    ros-humble-xacro \
    ros-humble-ros2-socketcan \
    ros-humble-can-msgs \
    ros-humble-joy \
    ros-humble-teleop-twist-joy \
    ros-humble-usb-cam \
    && rm -rf /var/lib/apt/lists/*

# Create the necessary directories and set permissions
RUN mkdir -p /root/ap4_hlc_docker_dir/ap4hlc_ws/image_output && \
    mkdir -p /root/ap4_hlc_docker_dir/ap4hlc_ws/model && \
    chmod -R 777 /root/ap4_hlc_docker_dir

WORKDIR /root/ap4_hlc_docker_dir

# # Copy twist_mux_example folder into Docker container
# COPY /ap4_hlc_code/ap4hlc_ws/src/twist_mux-rolling /root/ap4_hlc_docker_dir/twist_mux-rolling

# # Install twist_mux_example dependencies and build workspace
# RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
#                   cd twist_mux-rolling && \
#                   rosdep install --from-path src --ignore-src -y"
#                   # -y && \
                 
# RUN /bin/bash -c "source /root/ap4_hlc_docker_dir/twist_mux-rolling/colcon build"
# # Source the setup.bash file of the workspace
# # CMD ["/bin/bash", "-c", "source /root/ap4_hlc_docker_dir/twist_mux-rolling/install/setup.bash && \
# #                          source /root/ap4_hlc_docker_dir/ap4hlc_ws/install/setup.bash && \
# #                          ros2 launch twist_mux twist_mux_launch.py"]
