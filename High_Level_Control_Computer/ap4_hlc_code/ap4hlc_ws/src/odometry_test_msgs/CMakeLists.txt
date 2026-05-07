cmake_minimum_required(VERSION 3.8)
project(odometry_test_msgs)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Pose2DYaw.msg"
  "msg/OdometryTestData.msg"
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
