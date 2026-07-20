cmake_minimum_required(VERSION 4.2)
project(anchor) 

option(BUILD_PRODUCER "Build producer pipeline" ON)
option(BUILD_CONSUMER "Build consumer pipeline" ON)

add_subdirectory(pipeline)
