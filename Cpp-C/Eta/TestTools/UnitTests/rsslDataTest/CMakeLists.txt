
add_executable( rsslDataTest rsslDataTest.C dictionaries.h )

target_link_libraries( rsslDataTest librssl GTest::Main ${SYSTEM_LIBRARIES} )
set_target_properties( rsslDataTest PROPERTIES OUTPUT_NAME rsslDataTest )

target_include_directories(rsslDataTest
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Codec>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
)

if( CMAKE_HOST_UNIX )
    set_target_properties( rsslDataTest 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
						)
else()
#This definition is required for using google test with VS2012.
if (MSVC AND MSVC_VERSION EQUAL 1700)
	add_definitions(/D _VARIADIC_MAX=10)
endif()

target_compile_options( rsslDataTest	 
                            PRIVATE 
								${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
								${RCDEV_TYPE_CHECK_FLAG}
								$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
set_target_properties( rsslDataTest 
                        PROPERTIES 
                            RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}
                            RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD
                                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}
                            PDB_OUTPUT_DIRECTORY_RELEASE_MD 
                                ${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}
                            PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
                                ${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}
						)
endif()
