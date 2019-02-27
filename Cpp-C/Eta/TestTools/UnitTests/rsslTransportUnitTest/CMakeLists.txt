add_executable( rsslTransportUnitTest rsslTransportUnitTest.cpp )
target_link_libraries( rsslTransportUnitTest 
							librssl 
							GTest::Main 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslTransportUnitTest 
							PROPERTIES 
								OUTPUT_NAME rsslTransportUnitTest 
						)

add_executable( rsslTransportUnitTest_shared rsslTransportUnitTest.cpp )
target_link_libraries( rsslTransportUnitTest_shared 
							librssl_shared 
							GTest::Main 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslTransportUnitTest_shared 
							PROPERTIES 
								OUTPUT_NAME rsslTransportUnitTest 
						)

set_target_properties( rsslTransportUnitTest rsslTransportUnitTest_shared
							PROPERTIES 
								OUTPUT_NAME rsslTransportUnitTest 
						)

target_include_directories(rsslTransportUnitTest
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Codec>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Transport>
								$<BUILD_INTERFACE:$<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>>
)

target_include_directories(rsslTransportUnitTest_shared
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Codec>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Transport>
								$<BUILD_INTERFACE:$<TARGET_PROPERTY:CURL::libcurl,INTERFACE_INCLUDE_DIRECTORIES>>
)

if( CMAKE_HOST_UNIX )
							
	set_target_properties( rsslTransportUnitTest_shared 
							PROPERTIES 
								RUNTIME_OUTPUT_DIRECTORY 
									${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared 
							)
else()
	#This definition is required for using google test with VS2012.
	if (MSVC AND MSVC_VERSION EQUAL 1700)
	  add_definitions(/D _VARIADIC_MAX=10)
	endif()
	target_compile_options( rsslTransportUnitTest	 
							PRIVATE 
								${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
								${RCDEV_TYPE_CHECK_FLAG}
								$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)

	set_target_properties( rsslTransportUnitTest_shared 
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
	target_compile_options( rsslTransportUnitTest_shared	 
							PRIVATE 
								${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
								${RCDEV_TYPE_CHECK_FLAG}
								$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
endif()
