cmake_minimum_required(VERSION 4.2)

project(anchor) 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/banner.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE ${GST_INCLUDE_DIRS} include)

target_link_libraries(${PROJECT_NAME} PRIVATE ${GST_LIBRARIES})


