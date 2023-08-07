set(rsslTunnelStreamTestSrcFiles
	Consumer.C
	CoreComponent.C
	Provider.C
	ReadEvent.C
	TestReactor.C
	TestReactorComponent.C
	TestReactorEvent.C
	TestUtil.C
	TunnelStreamCoreProvider.C
	TunnelStreamProvider.C
	TunnelStreamTest.C

	#headers
	Consumer.h
	ConsumerProviderSessionOptions.h
	CoreComponent.h
	Provider.h
	ReadEvent.h
	TestReactor.h
	TestReactorComponent.h
	TestReactorEvent.h
	TestUtil.h
	TunnelStreamCoreProvider.h
	TunnelStreamProvider.h
)



add_executable( rsslTunnelStreamTest ${rsslTunnelStreamTestSrcFiles} )
target_link_libraries( rsslTunnelStreamTest 
							librsslVA 
							GTest::Main 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslTunnelStreamTest 
							PROPERTIES 
								OUTPUT_NAME rsslTunnelStreamTest 
						)
target_include_directories(rsslTunnelStreamTest
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								#Needed for testing of internal functionality
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Reactor/TunnelStream>
							)


if( CMAKE_HOST_UNIX )
	set_target_properties( rsslTunnelStreamTest 
							PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
							)
else()
	#This definition is required for using google test with VS2012.
	if (MSVC AND MSVC_VERSION EQUAL 1700)
		add_definitions(/D _VARIADIC_MAX=10)
	endif()
	target_compile_options( rsslTunnelStreamTest	 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	set_target_properties( rsslTunnelStreamTest 
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

if ( CMAKE_HOST_UNIX )

    add_custom_command(
                    TARGET rsslTunnelStreamTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary_large

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Tunnel Stream Unit Tests .... "
                    )

else()

    add_custom_command(
                    TARGET rsslTunnelStreamTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/RDMFieldDictionary_large

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Tunnel Stream Unit Tests .... "
                    )

endif()
