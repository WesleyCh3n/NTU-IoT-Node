cmake_minimum_required(VERSION 3.10)

project(cap)

find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

add_executable(cap main.cc)
target_link_libraries(cap ${OPENCV_STATIC_LDFLAGS})
