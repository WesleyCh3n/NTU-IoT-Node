cmake_minimum_required(VERSION 3.10)

project(network)

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR})

include_directories(/opt/tflite-2.4.2/include)

add_library(network network.cc)
