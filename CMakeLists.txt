cmake_minimum_required(VERSION 3.8)
project(turtlesim_demo)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)

add_executable(hexagon_turtles src/hexagon_turtles.cpp)
ament_target_dependencies(hexagon_turtles rclcpp geometry_msgs turtlesim)

install(TARGETS hexagon_turtles
  DESTINATION lib/${PROJECT_NAME})

ament_package()
