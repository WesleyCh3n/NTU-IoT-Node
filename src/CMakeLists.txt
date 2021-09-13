cmake_minimum_required(VERSION 3.10)

project(ntu-iot-node)
set(bin_name ntu-node)

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

add_executable(${bin_name} main.cc)
target_link_libraries(${bin_name} cow_monitor)

# Install
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
set(RELEASETAR ntu-iot-node_${VERSION}_linux_arm32v7.tar.xz)
add_custom_command(OUTPUT ${RELEASETAR}
    COMMAND tar cJf ${RELEASETAR} --transform s/release/ntu-iot-node/ release
    VERBATIM)
add_custom_command(OUTPUT make_release
    COMMAND ${CMAKE_COMMAND} -E echo "VERSION: ${VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo github-release release
    -u WesleyCh3n
    -r ntu-iot-node
    -t v${VERSION}
    -n v${VERSION}
    -d "$(msg)"
    VERBATIM)
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
