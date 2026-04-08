cmake_minimum_required(VERSION 4.2)

project(anchor) 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/banner.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
