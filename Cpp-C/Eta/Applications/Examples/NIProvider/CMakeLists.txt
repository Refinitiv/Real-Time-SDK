set( SOURCE_FILES
  rsslDirectoryHandler.c             rsslItemHandler.c
  rsslNIProvider.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslDictionaryProvider.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslDirectoryEncodeDecode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslItemEncode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslLoginConsumer.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslLoginEncodeDecode.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslMarketByOrderItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslMarketPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/Common/rsslSendMessage.c 
  )

add_executable( NIProvider_shared ${SOURCE_FILES} )
target_include_directories(NIProvider_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
							)
add_executable( NIProvider ${SOURCE_FILES} )
target_link_libraries( NIProvider_shared 
							librssl_shared 
							${SYSTEM_LIBRARIES} )
target_include_directories(NIProvider
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
							)
target_link_libraries( NIProvider 
							librssl 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( NIProvider NIProvider_shared PROPERTIES OUTPUT_NAME NIProvider )

if ( CMAKE_HOST_UNIX )
	set_target_properties( NIProvider_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
else() # if ( CMAKE_HOST_WIN32 )
	target_compile_options( NIProvider 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
    set_target_properties( NIProvider_shared 
                            PROPERTIES 
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
	target_compile_options( NIProvider_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
endif()

