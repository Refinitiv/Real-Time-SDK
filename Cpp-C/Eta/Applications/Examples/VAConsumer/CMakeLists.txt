if (BUILD_RTSDK-BINARYPACK OR BUILD_INTERNAL_RTSDK)
	set( SOURCE_FILES
  rsslConsumer.c            rsslDictionaryHandler.c        rsslDirectoryHandler.c
  rsslLoginConsumer.c       rsslMarketByOrderHandler.c     rsslMarketByPriceHandler.c
  rsslMarketPriceHandler.c  rsslPostHandler.c              rsslSymbolListHandler.c
  rsslYieldCurveHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVACacheHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/simpleTunnelMsgHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/tunnelStreamHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASendMessage.c

  rsslConsumer.h            rsslDictionaryHandler.h        rsslDirectoryHandler.h
  rsslLoginConsumer.h       rsslMarketByOrderHandler.h     rsslMarketByPriceHandler.h
  rsslMarketPriceHandler.h  rsslPostHandler.h              rsslSymbolListHandler.h
  rsslYieldCurveHandler.h   rsslChannelCommand.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVACacheHandler.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketPriceItems.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/simpleTunnelMsgHandler.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/tunnelStreamHandler.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASendMessage.h
  )

	add_executable( VAConsumer_shared ${SOURCE_FILES} )
	target_include_directories(VAConsumer_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( VAConsumer_shared 
							librsslVA_shared 
								Elektron-SDK-BinaryPack::rsslVACache_shared 
                            ${SYSTEM_LIBRARIES} 
						)

	add_executable( VAConsumer ${SOURCE_FILES} )
	target_include_directories(VAConsumer
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( VAConsumer 
							librsslVA 
								Elektron-SDK-BinaryPack::rsslVACache
                            ${SYSTEM_LIBRARIES}
						)

	set_target_properties( VAConsumer VAConsumer_shared PROPERTIES OUTPUT_NAME VAConsumer )

	if ( CMAKE_HOST_UNIX )
    set_target_properties( VAConsumer_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
	else()  # if ( CMAKE_HOST_WIN32 )
	target_compile_options( VAConsumer 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( VAConsumer_shared 
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
	target_compile_options( VAConsumer_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	endif()
else()
	message(STATUS "Skipping Eta Example VAConsumer Build: BUILD_RTSDK-BINARYPACK:${BUILD_RTSDK-BINARYPACK}")
endif()


