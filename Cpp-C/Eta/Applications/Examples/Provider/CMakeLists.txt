
set( SOURCE_FILES
  rsslDirectoryHandler.c        rsslItemHandler.c
  rsslLoginHandler.c            rsslProvider.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslDirectoryEncodeDecode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslItemEncode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslLoginEncodeDecode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslMarketByOrderItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslMarketByPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslMarketPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslSendMessage.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslSymbolListItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslDictionaryProvider.c
  )

add_executable( Provider_shared ${SOURCE_FILES} )
target_include_directories(Provider_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
							)
target_link_libraries( Provider_shared 
							librssl_shared 
							${SYSTEM_LIBRARIES} 
						)

add_executable( Provider ${SOURCE_FILES} )
target_include_directories(Provider
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
							)
target_link_libraries( Provider
							librssl 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( Provider Provider_shared
							PROPERTIES 
								OUTPUT_NAME Provider 
						)

if ( CMAKE_HOST_UNIX )
    set_target_properties( Provider_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
else ()  # if ( CMAKE_HOST_WIN32 )
    set_target_properties( Provider 
                            PROPERTIES 
                                PROJECT_LABEL "Provider" 
                        )
	target_compile_options( Provider 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( Provider_shared 
                            PROPERTIES 
                                PROJECT_LABEL "Provider_shared" 
                                RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
                                RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								LIBRARY_OUTPUT_DIRECTORY_RELEASE_MD
                                    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                LIBRARY_OUTPUT_DIRECTORY_DEBUG_MDD
                                    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								PDB_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
								PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                        )
	target_compile_options( Provider_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
endif()

