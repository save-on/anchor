find_package( PkgConfig REQUIRED )
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(pipelineProducer)

target_sources(pipelineProducer
    PRIVATE
    pipelineProducer.cpp
)

target_link_libraries(pipelineProducer
    PRIVATE
    PkgConfig::GSTREAMER
)
