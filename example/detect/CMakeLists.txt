cmake_minimum_required(VERSION 3.10)

project(detect)

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

include_directories(/opt/tflite-2.4.2/include)
link_directories(/opt/tflite-2.4.2/lib)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../src/)
include_directories(${SRC_DIR})

add_subdirectory(${SRC_DIR}/network network_lib)
add_subdirectory(${SRC_DIR}/yolov4-tiny yolov4-tiny_lib)

message(${OPENCV_STATIC_LDFLAGS})

add_executable(detect detect.cc)
target_link_libraries(detect
    opencv_dnn
    ${OPENCV_STATIC_LDFLAGS}
    yolov4-tiny
    network
    tensorflow-lite)
