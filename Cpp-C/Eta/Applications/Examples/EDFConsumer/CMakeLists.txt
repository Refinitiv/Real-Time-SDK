
set( SOURCE_FILES
  edfExampleConfig.c         itemList.c                 msgQueue.c
  realTimeSession.c          rsslMarketByOrderHandler.c rsslMarketPriceHandler.c
  rsslMarketByPriceHandler.c rsslEDFConsumer.c          gapRequestSession.c
  gapFillSession.c           refDataSession.c           snapshotSession.c
  )

add_executable( EDFConsumer_shared ${SOURCE_FILES} )
target_include_directories(EDFConsumer_shared
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
                )

add_executable( EDFConsumer_static ${SOURCE_FILES} )
target_include_directories(EDFConsumer_static
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
                )


if ( CMAKE_HOST_UNIX )

    target_link_libraries( EDFConsumer_static  ${EXAMPLE_LIBRARIES_STATIC} 
                                                ${SYSTEM_LIBRARIES} 
                                                ${EXAMPLE_LIBRARIES} 
                        )
    set_target_properties( EDFConsumer_static 
                            PROPERTIES 
                                OUTPUT_NAME EDFConsumer 
                        )
    target_link_libraries( EDFConsumer_shared ${EXAMPLE_LIBRARIES_SHARED} ${SYSTEM_LIBRARIES})

    set_target_properties( EDFConsumer_shared 
                            PROPERTIES 
                                OUTPUT_NAME EDFConsumer 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared 
                        )

#   if ( CMAKE_HOST_WIN32 )
else()
    set_target_properties( EDFConsumer_static 
                            PROPERTIES 
                                PROJECT_LABEL "EDFConsumer_static" )
    set_target_properties( EDFConsumer_shared 
                            PROPERTIES 
                                PROJECT_LABEL "EDFConsumer_shared" )

    target_link_libraries( EDFConsumer_static ${EXAMPLE_STATIC_LIBRARIES} )

    # TODO: Add config gen syntax for debug libs
    target_link_libraries( EDFConsumer_shared librsslVA )

endif()
