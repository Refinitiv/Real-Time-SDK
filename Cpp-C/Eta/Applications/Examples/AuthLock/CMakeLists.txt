
if (BUILD_RTSDK-BINARYPACK OR BUILD_INTERNAL_RTSDK)
	set( SOURCE_FILES rsslAuthLock.c )

	add_executable( authlock ${SOURCE_FILES} )

	if(CMAKE_HOST_WIN32)
	 target_compile_options( authlock 	
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)
	endif()
	target_link_libraries( authlock RTSDK-BinaryPack::dacsLib )
else()
	message(STATUS "Skipping Eta Example Authlock Build: BUILD_RTSDK-BINARYPACK:${BUILD_RTSDK-BINARYPACK}")
endif()

