
set( SOURCE_FILES
  consPerfConfig.c                  consumerThreads.c               marketByOrderDecoder.c
  marketPriceDecoder.c              upacConsPerf.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/channelHandler.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/getTime.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/itemEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/latencyRandomArray.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketByOrderEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/marketPriceEncoder.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/statistics.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlItemListParser.c
  ${EtaExamples_SOURCE_DIR}/PerfTools/Common/xmlMsgDataParser.c
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
target_link_libraries( ConsPerf_shared 
								librsslVA_shared 
								libxml2 
								${SYSTEM_LIBRARIES} )

add_executable( ConsPerf ${SOURCE_FILES} )
target_include_directories(ConsPerf
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/PerfTools/Common>
							)
set_target_properties( ConsPerf 
							PROPERTIES 
								OUTPUT_NAME ConsPerf )
target_link_libraries( ConsPerf 
								librsslVA  
								libxml2 
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
								PDB_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
								PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared 
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

if ( CMAKE_HOST_UNIX )
	if(TARGET ConsPerf)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml)
			add_custom_command(
				TARGET ConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml)
		endif()
	endif()
	if(TARGET ConsPerf_shared)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml)
			add_custom_command(
				TARGET ConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml)
		endif()
	endif()
else() # if ( CMAKE_HOST_WIN32 )
	if(TARGET ConsPerf)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml)
			add_custom_command(
				TARGET ConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml)
		endif()
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml)
			add_custom_command(
				TARGET ConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml)
		endif()
	endif()
	if(TARGET ConsPerf_shared)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml)
			add_custom_command(
				TARGET ConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml)
		endif()
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml)
			add_custom_command(
			TARGET ConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${EtaExamples_SOURCE_DIR}/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml)
		endif()
	endif()
endif()
