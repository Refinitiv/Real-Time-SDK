
add_subdirectory( EmaCppConsPerf )

if ( CMAKE_HOST_UNIX )
	set(_output_files	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml
					)
	add_custom_command(
				OUTPUT ${_output_files}

				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/MsgData.xml

				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "Copying 350k.xml and MsgData.xml for Ema perf tools .... "
				)

else() # if ( CMAKE_HOST_WIN32 )
	set(_output_files	${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml
					)

	add_custom_command(
				OUTPUT ${_output_files}

				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/MsgData.xml

				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/MsgData.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/350k.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/350k.xml
				COMMAND ${CMAKE_COMMAND} -E copy_if_different Common/MsgData.xml
						${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/MsgData.xml

				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "Copying 350k.xml and MsgData.xml for Ema perf tools .... "
			)
endif()

add_custom_target(EmaPerftoolsXMLfiles ALL DEPENDS ${_output_files})
unset(_output_files)


