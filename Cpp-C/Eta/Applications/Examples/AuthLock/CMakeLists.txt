
if (BUILD_ELEKTRON-SDK-BINARYPACK OR BUILD_INTERNAL_ESDK)
	set( SOURCE_FILES rsslAuthLock.c )

	add_executable( authlock ${SOURCE_FILES} )

	if(CMAKE_HOST_WIN32)
	 target_compile_options( authlock 	
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)
	endif()
	target_link_libraries( authlock Elektron-SDK-BinaryPack::dacsLib )
else()
	message(STATUS "Skipping Eta Example Authlock Build: BUILD_ELEKTRON-SDK-BINARYPACK:${BUILD_ELEKTRON-SDK-BINARYPACK}")
endif()

