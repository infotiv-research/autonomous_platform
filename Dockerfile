FROM ubuntu:20.04

RUN apt-get update
RUN apt-get install -y --no-install-recommends apt-utils build-essential
RUN apt-get install -y --no-install-recommends python3
RUN apt-get install -y --no-install-recommends gcc-arm-none-eabi  libnewlib-arm-none-eabi

# RUN apt-get install -y \
#     apt-utils \
#     ros-kinetic-desktop-full \
#     python-catkin-tools \
#     ca-certificates \
#     ros-kinetic-pcl-ros \
#     ros-kinetic-pcl-conversions

# RUN apt-get install -y \
#     vim \
#     sudo \
#     ssh \
#     curl \
#     build-essential \
#     expect \
#     wget

RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata 
RUN apt-get install -y --no-install-recommends pandoc texlive-xetex texlive-fonts-recommended lmodern

ENV RUNNING_IN_DOCKER_CONTAINER True