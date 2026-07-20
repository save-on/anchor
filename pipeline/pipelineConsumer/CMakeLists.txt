find_package( PkgConfig REQUIRED )
pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(pipelineConsumer)

target_sources(pipelineConsumer 
    PRIVATE 
    pipelineConsumer.cpp
)

target_link_libraries(pipelineConsumer
    PRIVATE
    PkgConfig::gstreamer
)
