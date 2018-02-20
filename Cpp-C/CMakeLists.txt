

if( CMAKE_HOST_UNIX )

	set( SYSTEM_LIBRARIES rt dl pthread m stdc++ )

else() #  if ( CMAKE_HOST_WIN32 )

	set( SYSTEM_LIBRARIES wininet.lib ws2_32.lib )
    # all other configurartions considered optimized	
	set_property ( GLOBAL PROPERTY DEBUG_CONFIGURATIONS "Debug_MDd" )

endif()

DEBUG_PRINT(CMAKE_C_FLAGS)
			
#find_package(GTest REQUIRED)

add_subdirectory( Eta )

if (BUILD_EMA_LIBRARY)
    add_subdirectory( Ema )
else()
    message(STATUS 
            "Skipping Ema Library Build: BUILD_EMA_LIBRARY:${BUILD_EMA_LIBRARY}")
endif()

