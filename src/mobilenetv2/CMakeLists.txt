cmake_minimum_required(VERSION 3.10)

project(mobilenetv2)

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

include_directories(/opt/tflite-2.4.2/include)

add_library(mobilenetv2 mobilenetv2.cc)
