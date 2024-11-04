FROM ros:humble-ros-core-jammy

# install bootstrap tools
RUN apt-get update && apt-get install --no-install-recommends -y \
    build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool \
	iputils-ping \
    net-tools \
    wget \
    screen \
    nano \
    vim \
    htop \
    alsa-base \
    alsa-utils \
    can-utils \
    && rm -rf /var/lib/apt/lists/*


# bootstrap rosdep
RUN rosdep init && \
  rosdep update --rosdistro $ROS_DISTRO

RUN colcon mixin add default \
      https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml && \
    colcon mixin update && \
    colcon metadata add default \
      https://raw.githubusercontent.com/colcon/colcon-metadata-repository/master/index.yaml && \
    colcon metadata update


# install ros2 packages
# Raspberry Pi 4 Hardware interface cannot run GUI applications, hence install ros2 base instead
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-ros-base\
    ros-humble-teleop-twist-keyboard \
    ros-humble-joint-state-publisher-gui \
    ros-humble-xacro \
    ros-humble-twist-mux \
    ros-humble-ros2-socketcan \
    ros-humble-can-msgs \
    ros-humble-rqt-graph \
    ros-humble-can-msgs \
    ros-humble-ros2-socketcan \
    ros-humble-joy \
    ros-humble-teleop-twist-joy \
    && rm -rf /var/lib/apt/lists/*


# Remove, did not solve any audio card issues?    
RUN mkdir -p /root/ap4_hwi_docker_dir
RUN chmod +x /root/ap4_hwi_docker_dir
RUN mkdir -p .local/bin .config .cache
WORKDIR /root/ap4_hwi_docker_dir
