if (_setConfigType)
	if ($ENV{RCDEV_DEBUG_ENABLED})
        message(STATUS "setConfigType already included")
    endif()
    return()
endif()
set(_setConfigType TRUE)

# set allowed build types for windows and linux; also select default for Linux
if ( CMAKE_HOST_UNIX )
	# default for Linux if build type not specified by user
	if ( NOT CMAKE_BUILD_TYPE )
		set( CMAKE_BUILD_TYPE Optimized )
	endif()
	message( STATUS "incoming CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}" )

	# allowed build types for Linux
	set (CMAKE_CONFIGURATION_TYPES Optimized Debug)
	if ( NOT ${CMAKE_BUILD_TYPE} IN_LIST CMAKE_CONFIGURATION_TYPES )
	  message( FATAL_ERROR " invalid build type [${CMAKE_BUILD_TYPE}]; allowed build types are ${CMAKE_CONFIGURATION_TYPES}" )
	endif()
# windows build files are generated and will support the following config type
elseif( CMAKE_HOST_WIN32 )
	set(CMAKE_CONFIGURATION_TYPES Debug_MDd Release_MD )
else()
	message( FATAL_ERROR "unsupported CMAKE_SYSTEM_NAME [${CMAKE_SYSTEM_NAME}]" )
endif()


