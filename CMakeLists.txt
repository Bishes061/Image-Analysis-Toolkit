cmake_minimum_required(VERSION 3.10)
project(MyProject)
find_package(OpenCV REQUIRED)
add_executable(MyProject clone.cpp)
target_link_libraries(MyProject ${OpenCV_LIBS})