

if( CMAKE_HOST_UNIX )

	set( SYSTEM_LIBRARIES rt dl pthread m stdc++ )

else() #  if ( CMAKE_HOST_WIN32 )

	set( SYSTEM_LIBRARIES wininet.lib ws2_32.lib crypt32.lib cryptui.lib bcrypt.lib Iphlpapi.lib )

endif()

DEBUG_PRINT(CMAKE_C_FLAGS)
			
add_subdirectory( Eta )

if (BUILD_EMA_LIBRARY)
    add_subdirectory( Ema )
else()
    message(STATUS 
            "Skipping Ema Library Build: BUILD_EMA_LIBRARY:${BUILD_EMA_LIBRARY}")
endif()

