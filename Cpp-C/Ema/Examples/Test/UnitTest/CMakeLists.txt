
set(_UnitTestSrcFiles
        AckMsgTests.cpp ArrayTests.cpp
        DataDictionaryTest.cpp
        DataUnitTest.cpp DataUnitTest.h
        DateTimeTests.cpp DomainTypeTest.cpp
        ElementListTests.cpp
        EmaActiveConfigTest.cpp
        EmaAppClient.cpp EmaAppClient.h
        EmaBufferTest.cpp EmaConfigTest.cpp
        EmaXmlConfigValidationTest.cpp
        EmaStringTests.cpp EmaVectorTest.cpp
        FieldListTests.cpp FilterListTests.cpp
        GenericMsgTests.cpp LoginHelperTest.cpp
        MapTests.cpp NoDataSizeTest.cpp
        OmmExceptionTests.cpp PackedMsgTest.cpp
        PollFdMaintenanceTest.cpp PostMsgTests.cpp
        PreferredHostTest.cpp RefreshMsgTests.cpp
        RequestMsgTests.cpp RmtesBufferTest.cpp
        SeriesTests.cpp StatusMsgTests.cpp
        TestUtilities.cpp TestUtilities.h
        TunnelStreamRequestTests.cpp UpdateMsgTests.cpp
        VectorTests.cpp
        )

add_executable( EmaUnitTest ${_UnitTestSrcFiles} )
target_include_directories(EmaUnitTest
							PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
								$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Access/Impl>
                                $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Access>
								$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Rdm/Impl>
								$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Domain/Login/Impl>
							)

target_link_libraries( EmaUnitTest libema GTest::Main ${SYSTEM_LIBRARIES} )


if (CMAKE_HOST_WIN32)

	#This definition is required for using google test with VS2012.
	if (MSVC AND MSVC_VERSION EQUAL 1700)
	  add_definitions(/D _VARIADIC_MAX=10)
	endif()
	target_compile_options( EmaUnitTest
								PRIVATE
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( EmaUnitTest PROPERTIES PROJECT_LABEL "EmaUnitTest" )

endif()

if ( CMAKE_HOST_UNIX )

    add_custom_command(
                    TARGET EmaUnitTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../EmaConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaConfigDefault.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ../../../EmaConfig.xsd
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaConfigDefault.xsd

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaBlankConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaBlankConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaDefaultConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaDefaultConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaIncompleteConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaIncompleteConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaInvalidConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaInvalidConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaMinimalConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaMinimalConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different Malformed.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Malformed.xml

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaConfigTest.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EmaConfigTest.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different enumtypeTest.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enumtypeTest.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different RDMFieldDictionaryTest
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RDMFieldDictionaryTest

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying EmaConfigTest.xml, enumtypeTest.def and RDMFieldDictionaryTest for Ema Unit Tests .... "
                    )
else()

    add_custom_command(
                    TARGET EmaUnitTest POST_BUILD

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ..\\..\\..\\EmaConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaConfigDefault.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ..\\..\\..\\EmaConfig.xsd

                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaConfigDefault.xsd
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaBlankConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaBlankConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaDefaultConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaDefaultConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaIncompleteConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaIncompleteConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaInvalidConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaInvalidConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaMinimalConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaMinimalConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different Malformed.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Malformed.xml

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ..\\..\\..\\EmaConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaConfigDefault.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ..\\..\\..\\EmaConfig.xsd
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaConfigDefault.xsd

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaBlankConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaBlankConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaDefaultConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaDefaultConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaIncompleteConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaIncompleteConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaInvalidConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaInvalidConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaMinimalConfig.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaMinimalConfig.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different Malformed.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Malformed.xml

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaConfigTest.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/EmaConfigTest.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different enumtypeTest.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/enumtypeTest.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different RDMFieldDictionaryTest
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/RDMFieldDictionaryTest

                    COMMAND ${CMAKE_COMMAND} -E copy_if_different EmaConfigTest.xml
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/EmaConfigTest.xml
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different enumtypeTest.def
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/enumtypeTest.def
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different RDMFieldDictionaryTest
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/RDMFieldDictionaryTest

                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMENT "Copying EmaConfigTest.xml, enumtypeTest.def and RDMFieldDictionaryTest for Ema Unit Tests .... "
                    )

endif()
