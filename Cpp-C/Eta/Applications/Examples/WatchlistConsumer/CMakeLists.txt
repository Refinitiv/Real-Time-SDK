if (BUILD_ELEKTRON-SDK-BINARYPACK OR BUILD_INTERNAL_ESDK)
	set( SOURCE_FILES
  itemDecoder.c        postHandler.c        rsslWatchlistConsumer.c
  watchlistConsumerConfig.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/simpleTunnelMsgHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/tunnelStreamHandler.c
  )

	add_executable( WatchlistConsumer_shared ${SOURCE_FILES} )
	target_include_directories(WatchlistConsumer_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( WatchlistConsumer_shared 
							librssl_shared 
                            librsslVA_shared 
								Elektron-SDK-BinaryPack::rsslVACache_shared 
                            ${SYSTEM_LIBRARIES}
						)

	add_executable( WatchlistConsumer ${SOURCE_FILES} )
	target_include_directories(WatchlistConsumer
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( WatchlistConsumer 
							librssl 
                            librsslVA  
								Elektron-SDK-BinaryPack::rsslVACache
                            ${SYSTEM_LIBRARIES} 
						)

	set_target_properties( WatchlistConsumer_shared WatchlistConsumer
							PROPERTIES 
								OUTPUT_NAME WatchlistConsumer 
						)

	if ( CMAKE_HOST_UNIX )
    set_target_properties( WatchlistConsumer_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
	else()  # if ( CMAKE_HOST_WIN32 )
	target_compile_options( WatchlistConsumer 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
    set_target_properties( WatchlistConsumer_shared 
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
	target_compile_options( WatchlistConsumer_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
	endif()
else()
	message(STATUS "Skipping Eta Example WatchlistConsumer Build: BUILD_ELEKTRON-SDK-BINARYPACK:${BUILD_ELEKTRON-SDK-BINARYPACK}")
endif()

