cmake_minimum_required(VERSION 3.10)
project(LogApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(logapp src/app.cpp)

target_include_directories(logapp PRIVATE Loggerlib/include)
target_link_directories(logapp PRIVATE Loggerlib/build)
target_link_libraries(logapp PRIVATE logger)

find_package(Threads REQUIRED)
target_link_libraries(logapp PRIVATE Threads::Threads)

add_custom_command(TARGET logapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/Loggerlib/build/liblogger.so"
        "$<TARGET_FILE_DIR:logapp>"
    COMMENT "Copying liblogger.so next to executable"
)

add_custom_target(clean-all

    COMMAND ${CMAKE_COMMAND} -E remove CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove Makefile
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove *.log
    COMMAND ${CMAKE_COMMAND} -E echo "Clean done. Kept: logapp liblogger.so"
)