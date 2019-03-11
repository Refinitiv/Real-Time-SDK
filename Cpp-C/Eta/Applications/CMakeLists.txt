
project(EtaExamples)

if (BUILD_ETA_TRAINING)
    DEBUG_PRINT(BUILD_ETA_TRAINING)
    add_subdirectory(Training/Consumer)
    add_subdirectory(Training/NIProvider)
    add_subdirectory(Training/Provider)
else()
    message(STATUS 
            "Skipping Eta Training Build: BUILD_ETA_TRAINING:${BUILD_ETA_TRAINING}")
endif()

if (BUILD_ETA_EXAMPLES)

    set(CMAKE_ENABLE_EXPORTS OFF)
    add_subdirectory( Examples/Consumer )
    add_subdirectory( Examples/EncDecExample )
    add_subdirectory( Examples/NIProvider )
    add_subdirectory( Examples/Provider )
	if(NOT BUILD_32_BIT_ETA)
	    add_subdirectory( Examples/AuthLock )
		add_subdirectory( Examples/VAProvider )
		add_subdirectory( Examples/VAConsumer )
		add_subdirectory( Examples/VANIProvider )
		add_subdirectory( Examples/WatchlistConsumer )
	endif()
else()
    message(STATUS 
            "Skipping Eta Exampls Build: BUILD_ETA_EXAMPLES:${BUILD_ETA_EXAMPLES}")
endif()

if (BUILD_ETA_PERFTOOLS)
	if ( CMAKE_HOST_UNIX )
			add_custom_command(
					OUTPUT	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml

					COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
					COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
					COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
					COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml

					DEPENDS	 PerfTools/Common/350k.xml PerfTools/Common/MsgData.xml
					)
	else() # if ( CMAKE_HOST_WIN32 )
		add_custom_command(
				OUTPUT	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml

				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml

				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml

				DEPENDS	 PerfTools/Common/350k.xml PerfTools/Common/MsgData.xml
				)
	endif()

    add_subdirectory( PerfTools/ConsPerf )
    add_subdirectory( PerfTools/NIProvPerf )
    add_subdirectory( PerfTools/ProvPerf )
    add_subdirectory( PerfTools/TransportPerf )
else()
    message(STATUS 
            "Skipping Eta PerfTools Build: BUILD_ETA_PERFTOOLS:${BUILD_ETA_PERFTOOLS}")
endif()
