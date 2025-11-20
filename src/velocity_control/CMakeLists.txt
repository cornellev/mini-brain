cmake_minimum_required(VERSION 3.8)
project(velocity_control)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS2 dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)

# Pigpio includes
include_directories(/usr/include)

add_executable(velocity_control src/velocity_control.cpp)

target_link_libraries(velocity_control
    pigpiod_if2
    rt
    pthread
)

ament_target_dependencies(velocity_control
    rclcpp std_msgs ackermann_msgs
)

install(TARGETS
  velocity_control
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
