
set(_UnitTestSrcFiles
        AckMsgTests.cpp ArrayTests.cpp
        DataDictionaryTest.cpp
        DataUnitTest.cpp DataUnitTest.h
        DateTimeTests.cpp DomainTypeTest.cpp
        ElementListTests.cpp
        EmaAppClient.cpp EmaAppClient.h
        EmaBufferTest.cpp EmaConfigTest.cpp
        EmaStringTests.cpp EmaVectorTest.cpp
        FieldListTests.cpp FilterListTests.cpp
        GenericMsgTests.cpp LoginHelperTest.cpp
        MapTests.cpp NoDataSizeTest.cpp
        OmmExceptionTests.cpp PollFdMaintenanceTest.cpp
        PostMsgTests.cpp RefreshMsgTests.cpp
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

target_link_libraries( EmaUnitTest libema gtest ${SYSTEM_LIBRARIES} )


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


