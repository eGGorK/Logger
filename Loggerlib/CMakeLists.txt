cmake_minimum_required(VERSION 3.10)
project(LoggerLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(logger SHARED src/logger.cpp)
target_include_directories(logger PUBLIC include)
target_compile_options(logger PRIVATE -Wall -Wextra -Wpedantic)

add_custom_target(clean-all

    COMMAND ${CMAKE_COMMAND} -E remove CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove Makefile
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove *.log
    COMMAND ${CMAKE_COMMAND} -E echo "Clean done. Kept: logapp liblogger.so"
)