

set( SOURCE_FILES rsslAuthLock.c )

add_executable( authlock ${SOURCE_FILES} )

if(CMAKE_HOST_WIN32)
	 target_compile_options( authlock 	
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)
endif()
target_link_libraries( authlock ${Elektron-SDK-BinaryPack_NAMESPACE}dacsLib )


