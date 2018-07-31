
set(_ConsPerfSrcFiles
        EmaCppConsPerf.cpp
        ConsPerfConfig.cpp
        ConsumerThread.cpp
        EmaCppConsPerf.h
        ConsPerfConfig.h
        ConsumerThread.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppUtil.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/CtrlBreakHandler.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/GetTime.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Mutex.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfConfig.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Statistics.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadAffinity.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/XmlItemParser.cpp
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppUtil.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/AppVector.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/CtrlBreakHandler.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/GetTime.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Mutex.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/PerfConfig.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/Statistics.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadAffinity.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/ThreadBinding.h
        ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/XmlItemParser.h
    )

add_executable( EmaCppConsPerf ${_ConsPerfSrcFiles})
target_include_directories(EmaCppConsPerf
								PUBLIC
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Examples/PerfTools/Common>
								)
target_link_libraries( EmaCppConsPerf 
								libema 
								${SYSTEM_LIBRARIES} 
						)

add_executable( EmaCppConsPerf_shared ${_ConsPerfSrcFiles})
target_include_directories(EmaCppConsPerf_shared
								PUBLIC
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Examples/PerfTools/Common>
							)
target_link_libraries( EmaCppConsPerf_shared 
								libema_shared 
								${SYSTEM_LIBRARIES} 
						)
set_target_properties( EmaCppConsPerf EmaCppConsPerf_shared
							PROPERTIES 
								OUTPUT_NAME EmaCppConsPerf 
						)

if ( CMAKE_HOST_WIN32 )
    target_link_libraries( EmaCppConsPerf psapi.lib )
	target_compile_options( EmaCppConsPerf	 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	target_link_libraries( EmaCppConsPerf_shared psapi.lib )
	target_compile_options( EmaCppConsPerf_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	
	set_target_properties( EmaCppConsPerf_shared
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
	set_target_properties( EmaCppConsPerf_shared 
                                PROPERTIES 
                                    RUNTIME_OUTPUT_DIRECTORY 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared
                          )
endif()
if ( CMAKE_HOST_UNIX )
	if(TARGET EmaCppConsPerf)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml)
			add_custom_command(
				TARGET EmaCppConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml)
		endif()
	endif()
	if(TARGET EmaCppConsPerf_shared)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml)
			add_custom_command(
				TARGET EmaCppConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml)
		endif()
	endif()
else() # if ( CMAKE_HOST_WIN32 )
	if(TARGET EmaCppConsPerf)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml)
			add_custom_command(
				TARGET EmaCppConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml)
		endif()
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml)
			add_custom_command(
				TARGET EmaCppConsPerf POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml)
		endif()
	endif()
	if(TARGET EmaCppConsPerf_shared)
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml)
			add_custom_command(
				TARGET EmaCppConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml)
		endif()
		if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml)
			add_custom_command(
			TARGET EmaCppConsPerf_shared POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy
						 ${Ema_SOURCE_DIR}/Examples/PerfTools/Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml)
		endif()
	endif()
						 
						 
endif()
