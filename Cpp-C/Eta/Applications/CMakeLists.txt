
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
if ( NOT (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "REDHAT" AND RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 6) )
	    add_subdirectory( Examples/AuthLock )
endif()
		add_subdirectory( Examples/VAProvider )
		add_subdirectory( Examples/VAConsumer )
		add_subdirectory( Examples/VANIProvider )
		add_subdirectory( Examples/WatchlistConsumer )
		add_subdirectory( Examples/MultiCredWLConsumer )
	endif()
else()
    message(STATUS 
            "Skipping Eta Exampls Build: BUILD_ETA_EXAMPLES:${BUILD_ETA_EXAMPLES}")
endif()

if (BUILD_ETA_PERFTOOLS)
    add_subdirectory( PerfTools/ConsPerf )
    add_subdirectory( PerfTools/NIProvPerf )
    add_subdirectory( PerfTools/ProvPerf )
    add_subdirectory( PerfTools/TransportPerf )

	if ( CMAKE_HOST_UNIX )
		set(_output_files	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/WSBConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/WSBConfig.json
						)

		add_custom_command(
					OUTPUT ${_output_files}

					COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
					COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
					COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CLConfig.json
					COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/WSBConfig.json

					COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
					COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml
					COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/CLConfig.json
					COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
							 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/WSBConfig.json

					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					COMMENT "Copying 350k.xml and MsgData.xml for Eta perf tools .... "
					)

	else() # if ( CMAKE_HOST_WIN32 )
		set(_output_files	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/WSBConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/WSBConfig.json

							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/WSBConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/CLConfig.json
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/WSBConfig.json
					)

		add_custom_command(
				OUTPUT	${_output_files}

				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/CLConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/WSBConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/CLConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/WSBConfig.json

				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/CLConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/WSBConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different PerfTools/Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/CLConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/CLConfig.json
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Examples/MultiCredWLConsumer/WSBConfig.json
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/WSBConfig.json

				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "Copying 350k.xml and MsgData.xml for Eta perf tools .... "

				)
	endif()
	add_custom_target(EtaPerftoolsXMLfiles ALL DEPENDS ${_output_files})
	unset(_output_files)

else()
    message(STATUS 
            "Skipping Eta PerfTools Build: BUILD_ETA_PERFTOOLS:${BUILD_ETA_PERFTOOLS}")
endif()
