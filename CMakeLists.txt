cmake_minimum_required(VERSION 3.19)
project(libplugin LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIXMAN REQUIRED pixman-1)

set(HYPRLAND_SRC_DIR "/usr/include/hyprland")

add_library(plugin MODULE main.cpp)
set_target_properties(plugin PROPERTIES PREFIX "")

target_include_directories(plugin PRIVATE 
    ${HYPRLAND_SRC_DIR}
    ${HYPRLAND_SRC_DIR}/src
    ${HYPRLAND_SRC_DIR}/build
    ${HYPRLAND_SRC_DIR}/protocols
    "/usr/include/libdrm"
    ${PIXMAN_INCLUDE_DIRS}
)
