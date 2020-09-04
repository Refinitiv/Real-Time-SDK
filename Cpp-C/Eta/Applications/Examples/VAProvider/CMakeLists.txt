if (BUILD_RTSDK-BINARYPACK OR BUILD_INTERNAL_RTSDK)
	set( SOURCE_FILES
  rsslDictionaryProvider.c        rsslDirectoryHandler.c        rsslLoginHandler.c
  rsslItemHandler.c               rsslProvider.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketByOrderItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketByPriceItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASymbolListItems.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAItemEncode.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASendMessage.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVACacheHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/simpleTunnelMsgHandler.c
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/tunnelStreamHandler.c
  
  rsslDictionaryProvider.h        rsslDirectoryHandler.h        rsslLoginHandler.h
  rsslItemHandler.h               rsslProvider.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketPriceItems.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketByOrderItems.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAMarketByPriceItems.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASymbolListItems.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVAItemEncode.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVASendMessage.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/rsslVACacheHandler.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/simpleTunnelMsgHandler.h
  ${EtaExamples_SOURCE_DIR}/Examples/VACommon/tunnelStreamHandler.h
  )

	add_executable( VAProvider_shared ${SOURCE_FILES} )
	target_include_directories(VAProvider_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( VAProvider_shared 
                            librsslVA_shared 
								RTSDK-BinaryPack::rsslVACache_shared 
                            ${SYSTEM_LIBRARIES} 
						)
	add_executable( VAProvider ${SOURCE_FILES} )
	target_include_directories(VAProvider
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/VACommon>
							)
	target_link_libraries( VAProvider 
                            librsslVA  
								RTSDK-BinaryPack::rsslVACache
                            ${SYSTEM_LIBRARIES} 
						)


	if ( CMAKE_HOST_UNIX )
    set_target_properties( VAProvider_shared 
                            PROPERTIES 
                                OUTPUT_NAME VAProvider 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
	else()  #  if ( CMAKE_HOST_WIN32 )

	target_compile_options( VAProvider 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( VAProvider_shared 
                            PROPERTIES 
                                OUTPUT_NAME VAProvider 
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
	target_compile_options( VAProvider_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	endif()
else()
	message(STATUS "Skipping Eta Example VAProvider Build: BUILD_RTSDK-BINARYPACK:${BUILD_RTSDK-BINARYPACK}")
endif()


