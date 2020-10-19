
set( SOURCE_FILES
  consPerfConfig.c                  consumerThreads.c               marketByOrderDecoder.c
  marketPriceDecoder.c              ConsPerf.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/rjConverter.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/channelHandler.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/itemEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/latencyRandomArray.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketByOrderEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketPriceEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/perfTunnelMsgHandler.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/statistics.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/tunnelStreamHandler.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlItemListParser.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlMsgDataParser.c

    consPerfConfig.h                  consumerThreads.h               marketByOrderDecoder.h
  marketPriceDecoder.h              ConsPerf.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/rjConverter.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/channelHandler.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/itemEncoder.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/latencyRandomArray.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketByOrderEncoder.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketPriceEncoder.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/perfTunnelMsgHandler.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/statistics.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/tunnelStreamHandler.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlItemListParser.h
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlMsgDataParser.h
  )

add_executable( ConsPerf_shared ${SOURCE_FILES} )
target_include_directories(ConsPerf_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/Common>
							)
set_target_properties( ConsPerf_shared 
							PROPERTIES 
								OUTPUT_NAME ConsPerf )
target_link_libraries( ConsPerf_shared librsslVA_shared librsslJsonConverter_shared 
								LibXml2::LibXml2 
								${SYSTEM_LIBRARIES} )

add_executable( ConsPerf ${SOURCE_FILES} )
target_include_directories(ConsPerf
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/Common>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Converter>
							)
set_target_properties( ConsPerf 
							PROPERTIES 
								OUTPUT_NAME ConsPerf )
target_link_libraries( ConsPerf 
								librsslVA  LibXml2::LibXml2 
								${SYSTEM_LIBRARIES} )

if ( CMAKE_HOST_UNIX )
	set_target_properties( ConsPerf_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )

else() # if ( CMAKE_HOST_WIN32 )
    set_target_properties( ConsPerf PROPERTIES PROJECT_LABEL "ConsPerf" )
	target_compile_options( ConsPerf 
                                PRIVATE 
                                    ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                    ${RCDEV_TYPE_CHECK_FLAG}
                                    $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( ConsPerf_shared 
                            PROPERTIES 
                                PROJECT_LABEL "ConsPerf_shared" 
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
	target_compile_options( ConsPerf_shared
								PRIVATE 
                                    ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                    ${RCDEV_TYPE_CHECK_FLAG}
                                    $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    target_link_libraries( ConsPerf psapi )
    target_link_libraries( ConsPerf_shared psapi )
endif()


