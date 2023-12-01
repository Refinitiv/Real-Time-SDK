set ( rsslTransportUnitTestSrcFiles
	rsslAllocateUnitTest.cpp
	rsslTransportUnitTest.cpp

	TransportUnitTest.h
)

set ( rsslTransportBindThreadSrcFiles
		rsslBindThreadUnitTest.cpp
)


if(${RCDEV_HOST_SYSTEM_BITS} STREQUAL "64")
	add_executable( rsslTransportUnitTest ${rsslTransportUnitTestSrcFiles} ${rsslTransportBindThreadSrcFiles} )
else()
	add_executable( rsslTransportUnitTest ${rsslTransportUnitTestSrcFiles} )
endif()
target_link_libraries( rsslTransportUnitTest 
							librssl 
							GTest::Main 
							${SYSTEM_LIBRARIES} 
						)
set_target_properties( rsslTransportUnitTest 
							PROPERTIES 
								OUTPUT_NAME rsslTransportUnitTest 
						)

if(${RCDEV_HOST_SYSTEM_BITS} STREQUAL "64")
	add_executable( rsslTransportUnitTest_shared ${rsslTransportUnitTestSrcFiles} ${rsslTransportBindThreadSrcFiles} )
else()
	add_executable( rsslTransportUnitTest_shared ${rsslTransportUnitTestSrcFiles} )
endif()

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

if ( CMAKE_HOST_UNIX )

    add_custom_command(
                    TARGET rsslTransportUnitTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/inpbuf_ws_rwf_01.bin
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying inpbuf_01.bin, inpbuf_ws_rwf_01.bin, enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Transport Unit Tests .... "
                    )

    add_custom_command(
                    TARGET rsslTransportUnitTest_shared POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared/inpbuf_ws_rwf_01.bin
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying inpbuf_01.bin, inpbuf_ws_rwf_01.bin, enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Transport Shared Unit Tests .... "
                    )
else()

    add_custom_command(
                    TARGET rsslTransportUnitTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/inpbuf_ws_rwf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/inpbuf_ws_rwf_01.bin
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying inpbuf_01.bin, inpbuf_ws_rwf_01.bin, enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Transport Unit Tests .... "     
                    )
                    
    add_custom_command(
                    TARGET rsslTransportUnitTest_shared POST_BUILD
                    
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared/inpbuf_ws_rwf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/RDMFieldDictionary_large
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/inpbuf_01.bin
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/inpbuf_ws_rwf_01.bin
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared/inpbuf_ws_rwf_01.bin

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying inpbuf_01.bin, inpbuf_ws_rwf_01.bin, enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Transport Shared Unit Tests .... "
                    )

endif()