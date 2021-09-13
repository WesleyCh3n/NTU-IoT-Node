cmake_minimum_required(VERSION 3.10)

project(cow_monitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_definitions()

# OPENCV
find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

# tflite
add_library(tflite STATIC IMPORTED)
set_target_properties(tflite PROPERTIES
  IMPORTED_LOCATION /opt/tflite-2.4.2/lib/libtensorflow-lite.a
  INTERFACE_INCLUDE_DIRECTORIES /opt/tflite-2.4.2/include
)

# MQTT
add_library(mqttpp3 STATIC IMPORTED)
set_target_properties(mqttpp3 PROPERTIES
  IMPORTED_LOCATION /opt/mqtt/lib/libpaho-mqttpp3.a
  INTERFACE_INCLUDE_DIRECTORIES /opt/mqtt/include
)
add_library(mqtt3a STATIC IMPORTED)
set_target_properties(mqtt3a PROPERTIES
  IMPORTED_LOCATION /opt/mqtt/lib/libpaho-mqtt3a.a
)

# Boost
include_directories(/opt/boost-1.75.0)

# self header
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${SRC_DIR})

# self lib
add_subdirectory(${SRC_DIR}/network network_lib)
add_subdirectory(${SRC_DIR}/mobilenetv2 mobilenetv2_lib)
add_subdirectory(${SRC_DIR}/yolov4-tiny yolov4-tiny_lib)

# compile option
if(NOT DEFINED VERSION)
    set(VERSION beta)
endif()
message(STATUS "NTU IOT NODE:")
message(STATUS "  VERSION = ${VERSION}")

add_library(cow_monitor cow_monitor.cpp)
target_compile_definitions(cow_monitor PUBLIC
    -D NTU_IOT_NODE_VERSION="${VERSION}"
    -D RELEASE)
target_link_libraries(cow_monitor
    ${OPENCV_STATIC_LDFLAGS}
    mobilenetv2
    yolov4-tiny
    network
    tflite
    mqttpp3
    mqtt3a
    stdc++fs)
