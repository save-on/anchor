if(BUILD_PRODUCER)
    add_subdirectory(pipelineProducer)
endif()

if(BUILD_CONSUMER)
    add_subdirectory(pipelineConsumer)
endif()
