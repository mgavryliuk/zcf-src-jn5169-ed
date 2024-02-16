set(CMAKE_BUILD_TYPE Release)

set(JENNIC_CHIP JN5169)
set(JENNIC_CHIP_FAMILY JN516x)
include(cmake/JennicSDK.cmake)

cmake_minimum_required(VERSION 3.28.1)
project(AQARA_D1_ALT C CXX ASM)

# dump_compiler_settings()

add_subdirectory(src)
