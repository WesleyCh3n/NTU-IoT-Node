cmake_minimum_required(VERSION 3.10)

project(ntu-iot-node)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

include_directories(/opt/cxxopts-2.2.1/include)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(cow_monitor)

add_executable(ntu-node main.cc)
target_link_libraries(ntu-node cow_monitor)
