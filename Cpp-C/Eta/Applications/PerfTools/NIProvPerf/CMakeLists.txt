
set( SOURCE_FILES
  niProvPerfConfig.c              upacNIProvPerf.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/channelHandler.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/directoryProvider.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/getTime.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/itemEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/latencyRandomArray.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketByOrderEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketPriceEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/providerThreads.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/statistics.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlItemListParser.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlMsgDataParser.c
  )

add_executable( NIProvPerf_shared ${SOURCE_FILES} )
target_include_directories(NIProvPerf_shared
						PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/Common>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/ProvPerf>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/NIProvPerf>
						)
set_target_properties( NIProvPerf_shared 
						PROPERTIES 
							OUTPUT_NAME NIProvPerf 
						)
target_link_libraries( NIProvPerf_shared 
						librsslVA_shared 
                        LibXml2::LibXml2 
                        ${SYSTEM_LIBRARIES} 
						)

add_executable( NIProvPerf ${SOURCE_FILES} )
target_include_directories(NIProvPerf
						PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/Common>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/ProvPerf>
							$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/NIProvPerf>
						)
set_target_properties( NIProvPerf 
						PROPERTIES 
							OUTPUT_NAME NIProvPerf 
						)
target_link_libraries( NIProvPerf 
						librsslVA 
                        LibXml2::LibXml2 
                        ${SYSTEM_LIBRARIES} 
						)

set_target_properties( NIProvPerf
                        PROPERTIES 
                            OUTPUT_NAME NIProvPerf 
						)

if ( CMAKE_HOST_UNIX )
	set_target_properties( NIProvPerf_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )
else() # if ( CMAKE_HOST_WIN32 )
    target_link_libraries( NIProvPerf psapi )
    target_link_libraries( NIProvPerf_shared psapi )
	set_target_properties( NIProvPerf PROPERTIES PROJECT_LABEL "NIProvPerf" )
	target_compile_options( NIProvPerf	 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( NIProvPerf_shared 
                            PROPERTIES 
                                PROJECT_LABEL "NIProvPerf_shared" 
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
	target_compile_options( NIProvPerf_shared	 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)

endif()


