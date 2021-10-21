
set(_NIProvPerfSrcFiles
        EmaCppNIProvPerf.cpp
        EmaCppNIProvPerf.h
        NIProvPerfConfig.cpp
        NIProvPerfConfig.h
        NIProviderPerfClient.cpp
        NIProviderPerfClient.h
        NIProviderThread.cpp
        NIProviderThread.h
        ReadMe.txt

        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppUtil.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/CtrlBreakHandler.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/GetTime.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/LatencyCollection.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/LatencyRandomArray.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MessageDataUtil.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Mutex.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfConfig.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfMessageData.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfUtils.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ProvItemInfo.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Statistics.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadAffinity.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/XmlItemParser.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/xmlMsgDataParser.c
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/xmlPerfMsgDataParser.cpp

        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppUtil.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppVector.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/CtrlBreakHandler.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/GetTime.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/LatencyCollection.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/LatencyRandomArray.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MessageDataUtil.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Mutex.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfConfig.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfMessageData.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfUtils.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ProvItemInfo.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Statistics.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/testUtils.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadAffinity.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadBinding.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/XmlItemParser.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/xmlMsgDataParser.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/xmlPerfMsgDataParser.h
    )

add_executable( EmaCppNIProvPerf ${_NIProvPerfSrcFiles})
target_include_directories(EmaCppNIProvPerf
								PUBLIC
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Examples/PerfTools/Common>
								)
target_link_libraries( EmaCppNIProvPerf 
								libema 
								${SYSTEM_LIBRARIES} 
						)

add_executable( EmaCppNIProvPerf_shared ${_NIProvPerfSrcFiles})
target_include_directories(EmaCppNIProvPerf_shared
								PUBLIC
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Examples/PerfTools/Common>
							)
target_link_libraries( EmaCppNIProvPerf_shared 
								libema_shared 
								${SYSTEM_LIBRARIES} 
						)
set_target_properties( EmaCppNIProvPerf EmaCppNIProvPerf_shared
							PROPERTIES 
								OUTPUT_NAME EmaCppNIProvPerf 
						)

if ( CMAKE_HOST_WIN32 )
    target_link_libraries( EmaCppNIProvPerf psapi.lib )
	target_compile_options( EmaCppNIProvPerf	 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	target_link_libraries( EmaCppNIProvPerf_shared psapi.lib )
	target_compile_options( EmaCppNIProvPerf_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	
	set_target_properties( EmaCppNIProvPerf_shared
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
else()
	set_target_properties( EmaCppNIProvPerf_shared 
                                PROPERTIES 
                                    RUNTIME_OUTPUT_DIRECTORY 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared
                          )
endif()
