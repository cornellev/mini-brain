cmake_minimum_required(VERSION 3.8)
project(testing)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(testing src/testing.cpp)

ament_target_dependencies(testing rclcpp std_msgs)

install(TARGETS
  testing
  DESTINATION lib/${PROJECT_NAME}
)

# install(DIRECTORY launch/
#   DESTINATION share/${PROJECT_NAME}/launch
# )

ament_package()