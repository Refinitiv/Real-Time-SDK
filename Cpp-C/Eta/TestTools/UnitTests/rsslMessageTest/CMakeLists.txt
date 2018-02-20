add_executable( rsslMessageTest rsslMessageTest.C )
target_link_libraries( rsslMessageTest 
							librssl 
							gtest 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslMessageTest 
							PROPERTIES 
								OUTPUT_NAME rsslMessageTest 
						)

add_executable( rsslMessageTest_shared rsslMessageTest.C )
target_link_libraries( rsslMessageTest_shared 
							librssl_shared 
							gtest 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslMessageTest_shared 
							PROPERTIES 
								OUTPUT_NAME rsslMessageTest 
						)

set_target_properties( rsslMessageTest rsslMessageTest_shared
							PROPERTIES 
								OUTPUT_NAME rsslMessageTest 
						)

target_include_directories(rsslMessageTest
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Codec>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
)

target_include_directories(rsslMessageTest_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Codec>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
)

if( CMAKE_HOST_UNIX )
							
	set_target_properties( rsslMessageTest_shared 
							PROPERTIES 
								RUNTIME_OUTPUT_DIRECTORY 
									${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared 
							)
else()
	#This definition is required for using google test with VS2012.
	if (MSVC AND MSVC_VERSION EQUAL 1700)
	  add_definitions(/D _VARIADIC_MAX=10)
	endif()
	target_compile_options( rsslMessageTest	 
							PRIVATE 
								${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
								${RCDEV_TYPE_CHECK_FLAG}
								$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)

	set_target_properties( rsslMessageTest_shared 
							PROPERTIES 
								RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
								RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								PDB_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
								PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
							)
	target_compile_options( rsslMessageTest_shared	 
							PRIVATE 
								${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
								${RCDEV_TYPE_CHECK_FLAG}
								$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
endif()
