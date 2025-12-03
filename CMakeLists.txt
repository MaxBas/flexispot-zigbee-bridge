# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Disable MINIMAL_BUILD so menuconfig shows optional component Kconfig entries like GDB Stub
idf_build_set_property(MINIMAL_BUILD OFF)

project(flexispot-zigbee-bridge)
