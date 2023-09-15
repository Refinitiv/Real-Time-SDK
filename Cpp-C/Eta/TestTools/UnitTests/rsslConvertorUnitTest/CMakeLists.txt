if( NOT Eta_SOURCE_DIR)
	set(Eta_SOURCE_DIR "${rtsdk_SOURCE_DIR}/Cpp-C/Eta")
endif()

	set(srcFiles_rsslJsonConverterTest
					FidDb/GlobalFidDb.C
					cpp-base64/base64.C 
					rsslJsonConverterTest/arrayTests.C 
					rsslJsonConverterTest/ackMsgTests.C 
					rsslJsonConverterTest/genericMsgTests.C 
					rsslJsonConverterTest/closeMsgTests.C 
					rsslJsonConverterTest/elementListTests.C 
					rsslJsonConverterTest/errorMsgTests.C 
					rsslJsonConverterTest/fieldListTests.C 
					rsslJsonConverterTest/filterListTests.C 
					rsslJsonConverterTest/mapTests.C 
					rsslJsonConverterTest/miscTests.C 
					rsslJsonConverterTest/nestedContainerTests.C 
					rsslJsonConverterTest/seriesTests.C 
					rsslJsonConverterTest/vectorTests.C 
					rsslJsonConverterTest/postMsgTests.C 
					rsslJsonConverterTest/primitiveTests.C 
					rsslJsonConverterTest/refreshMsgTests.C 
					rsslJsonConverterTest/requestMsgTests.C 
					rsslJsonConverterTest/rsslJsonConverterTestBase.C 
					rsslJsonConverterTest/statusMsgTests.C 
					rsslJsonConverterTest/updateMsgTests.C 
					rsslJsonConverterTest/rsslJsonConverterTest.C 
					rsslJsonConverterTest/setDefTests.C 
					rsslJsonConverterTest/bufferOverrunTests.C 
					rsslJsonConverterTest/rsslJsonConverterTestBase.h
					cpp-base64/base64.h
					FidDb/rtr/GlobalFidDb.h
					)
	add_executable( rsslJsonConverterTest ${srcFiles_rsslJsonConverterTest} )

	target_include_directories(rsslJsonConverterTest
							PRIVATE
								${Eta_SOURCE_DIR}/Impl/Converter/Include
								${Eta_SOURCE_DIR}/Impl/Util/Include
								${Eta_SOURCE_DIR}/Include/ansi
								${Eta_SOURCE_DIR}/Include/Converter
							PUBLIC
								${Eta_SOURCE_DIR}/TestTools/UnitTests/rsslConvertorUnitTest/rsslJsonConverterTest
								${Eta_SOURCE_DIR}/TestTools/UnitTests/rsslConvertorUnitTest
							)
	target_link_libraries( rsslJsonConverterTest 
									librssl
									libansi
									GTest::Main
									CJSON::CJSON
									${SYSTEM_LIBRARIES} )

	set_target_properties( rsslJsonConverterTest
							PROPERTIES 
								OUTPUT_NAME rsslJCtest 
						)
	if(CMAKE_HOST_WIN32)

		# This definition is required for using google test with VS2012.
		if (MSVC AND MSVC_VERSION EQUAL 1700)
			add_definitions(/D _VARIADIC_MAX=10) 		
		endif()

	 	target_compile_options( rsslJsonConverterTest 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	endif()

if ( CMAKE_HOST_UNIX )

    add_custom_command(
                    TARGET rsslJsonConverterTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/enumtype.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enumtype.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../../../etc/RDMFieldDictionary_large
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionary_large

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Json Converter Unit Tests .... "
                    )

else()

    add_custom_command(
                    TARGET rsslJsonConverterTest POST_BUILD

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
                    COMMENT "Copying enumtype.def, RDMFieldDictionary and RDMFieldDictionary_large for Eta Json Converter Unit Tests .... "
                    )

endif()

DEBUG_PRINT(rsslJsonConverterTest)
