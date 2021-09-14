cmake_minimum_required(VERSION 3.10)

project(ntu-node)
set(bin_name ntu-node)

# CXXFLAGS
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Opencv
find_package(PkgConfig)
pkg_check_modules(OPENCV REQUIRED opencv4)
include_directories(${OPENCV_INCLUDE_DIRS})

# Cxxopts
include_directories(/opt/cxxopts-2.2.1/include)

# self headers
include_directories(${CMAKE_SOURCE_DIR})

# build cow_monitor lib
add_subdirectory(cow_monitor)

# link main
add_executable(${bin_name} main.cc)
target_link_libraries(${bin_name} cow_monitor)

# Installation setting
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release)
install(TARGETS ${bin_name}
    RUNTIME DESTINATION bin)

install(FILES ${CMAKE_SOURCE_DIR}/../tools/upload.sh
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/../cfg/
    DESTINATION cfg)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/../model/
    DESTINATION model)


# release cicd
if(${CMAKE_CXX_COMPILER} MATCHES aarch64)
    set(ARCH arm64v8)
elseif(${CMAKE_CXX_COMPILER} MATCHES gnueabihf)
    set(ARCH arm32v7)
else()
    set(ARCH unknown)
endif()
message(STATUS "FOUND ARCH: ${ARCH}")

set(RELEASETAR ntu-iot-node_${VERSION}_linux_${ARCH}.tar.xz)
add_custom_command(OUTPUT ${RELEASETAR}
    COMMAND tar cJf ${RELEASETAR} --transform s/release/${bin_name}/ release
    VERBATIM)
# TODO
add_custom_command(OUTPUT make_release
    COMMAND ${CMAKE_COMMAND} -E echo "VERSION: ${VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo github-release release
    -u WesleyCh3n
    -r ntu-iot-node
    -t v${VERSION}
    -n v${VERSION}
    -d "$(msg)"
    VERBATIM)
# TODO
add_custom_command(OUTPUT make_upload
    COMMAND ${CMAKE_COMMAND} -E echo github-release upload
    -u WesleyCh3n
    -r ntu-iot-node
    -t v${VERSION}
    -n $(RELEASETAR)
    -f $(RELEASETAR)
    VERBATIM)
add_custom_target(release
    DEPENDS ${RELEASETAR} make_release make_upload)
