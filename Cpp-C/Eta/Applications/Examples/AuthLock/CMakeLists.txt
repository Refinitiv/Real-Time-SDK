
if (BUILD_RTSDK-BINARYPACK OR BUILD_INTERNAL_RTSDK)
	set( SOURCE_FILES rsslAuthLock.c )

	add_executable( authlock ${SOURCE_FILES} )

	if(CMAKE_HOST_WIN32)
	 target_compile_options( authlock 	
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)
	target_link_libraries( authlock RTSDK-BinaryPack::dacsLib )
	#This definition is not required for VS2012 and VS2013.
	if ( MSVC AND (MSVC_VERSION GREATER 1800) )
	  target_link_libraries( authlock legacy_stdio_definitions.lib )
	endif()
	else()
		target_link_libraries( authlock RTSDK-BinaryPack::dacsLib )
	endif()
else()
	message(STATUS "Skipping Eta Example Authlock Build: BUILD_RTSDK-BINARYPACK:${BUILD_RTSDK-BINARYPACK}")
endif()

