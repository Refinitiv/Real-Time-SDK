#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]


include(rcdevExternalUtils)

if(NOT zlib_url)
	set(zlib_url "https://www.zlib.net/zlib-1.2.11.tar.xz")
endif()
if(NOT zlib_hash)
	set(zlib_hash "SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066")
endif()
if(NOT zlib_version)
	set(zlib_version "1.2.11")
endif()
	
# If the option for using the system installed 
#  package is not defined
if( (NOT zlib_USE_INSTALLED) AND 
	(NOT ZLIB_FOUND) )
	# An external project for zlib
	set(_EPA_NAME "zlib")

	# Initialize the directory variables for the external project
	# default:
	#        external/
	#                dlcache/
	#                  BUILD/_EP_NAME/
	#                               source/
	#                               build/
	#        install/
	rcdev_init_ep_add(${_EPA_NAME})

	# get the file name off the url to ensure it is
	# downloaded with the same name
	get_filename_component(_dl_filename "${zlib_url}" NAME)
	set( _DL_METHOD "URL           ${zlib_url}" )

	if(zlib_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${zlib_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${zlib_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	# the top CMake entry point is not in the top source_dir location
	# so need to define 'SOURCE_SUBDIR'
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${zlib_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${zlib_install}")

	
	# ZLIB cmake build ignores this flag on UNIX type builds
	# check for any defined flags
	if(zlib_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()

	# check for any defined flags
	if(zlib_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${zlib_CONFIG_OPTIONS}")
	else()
		set(zlib_CONFIG_OPTIONS "${_config_options}")
	endif()

	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d")
	else()
		# Since our internal build types are Debug and Optimized, only Debug will translate
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
		else()
			set(_cfg_type "Release")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
		endif()
	endif()	
	# Append the shared args to the CMake arguments to the template variable
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
						"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
						"${_config_options}"
						"${_shared_arg}"
						)

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND can be
	# used and a seperate config step does not need to be defined here.
	#  adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	#  list(APPEND _EPA_CONFIGURE_COMMAND  "CONFIGURE_COMMAND  \"\"" )

	# Typically, the build and install steps can be combined.  However, having them as 
	#  two seperate steps help in the event of having to debug a build
	# Set the <.....>_COMMAND for the build and install template fields
	# However, for this external project it is works out better to combine these next two steps
	# within the INSTALL_COMMAND step.  So, this is skipping the BUILD_COMMAND by 
	# passing "" as the argument for the BUILD_COMMAND
	set( _EPA_BUILD_COMMAND 
				"BUILD_COMMAND        \"\"")

	# Passing the two supported build config types along to the INSTALL_COMMAND for Windows and for 
	# single build type platforms, like Linux, the current config typed is built and installed
	if (WIN32)
		set( _EPA_INSTALL_COMMAND 
					"INSTALL_COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Release "
					"        COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Debug ")
	else()
		set( _EPA_INSTALL_COMMAND 
					"INSTALL_COMMAND    ${CMAKE_COMMAND}   --build .  --target install  --config ${_cfg_type} ")
	endif()	

	# If there isn't a binary directory defined then make sure
	# the option 'BUILD_IN_SOURCE' is enabled
	if (NOT DEFINED _EPA_BINARY_DIR)
		set( _EPA_ADDITIONAL_ARGS "BUILD_IN_SOURCE 1" )
	endif()

	# Add log defiitions if selected to be enabled and append them to the
	# additional args variable
	if(zlib_LOG_BUILD)
		set(_log_args 
						"LOG_CONFIGURE 1"
						"LOG_BUILD 1"
						"LOG_INSTALL 1"
			)
	endif()

	list(APPEND _EPA_ADDITIONAL_ARGS 
						"${_log_args}"
			)

	# Call cmake configure and build on the CMakeLists.txt file
	# written using the previously set template arguments
	rcdev_config_build_ep(${_EPA_NAME})
	
	# If ZLIB_LIBRARY is not set here, then only the shared version will be assigned to the
	#   ZLIB:: tatger library interface
	if(WIN32)
		set(ZLIB_STATIC_NAME "${zlib_install}/lib/zlibstatic")
	else()
		set(ZLIB_STATIC_NAME "${zlib_install}/lib/libz")
	endif()

	set(ZLIB_LIBRARY "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")

	if (EXISTS "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(ZLIB_LIBRARY_RELEASE "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	if (EXISTS "${ZLIB_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(ZLIB_LIBRARY_DEBUG "${ZLIB_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	# this policy is needed to supress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 esdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT ZLIB_ROOT)
		set(ZLIB_ROOT "${zlib_install}" CACHE INTERNAL "")
	endif()

	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

	# Since the Zlib CMake Find module supports setting ZLIB_ROOT
	# there is no need to set any find options here
	# set(zlib_find_options NODEFAULT_PATH HINTS "${zlib_install}")

endif()

if(NOT ZLIB_FOUND)
	# Calling find_package with a required version number will fail if the
	# package does not have a <name>version.cmake in the same location as
	# the <package>config.cmake.  Unfortunately, CMake will not use the version
	# field defiition within a <package>.pc file. Also, the option to search for the
	# newly built version are passed as an argument, in case they have been defined, 
	# in lieu of an installed version
	find_package(ZLIB REQUIRED "${zlib_find_options}")

	# Will Map Release => Release_MD, Debug => Debug_Mdd and
	# on UNIX Release => Optimized
	rcdev_map_imported_ep_types(ZLIB::ZLIB)

	if(ZLIB_VERSION_STRING VERSION_LESS "${zlib_version}")
		message(WARNING
				"  Zlib ver:${ZLIB_VERSION_STRING} found, is older than the supported ver:${zlib_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	endif()
endif()

DEBUG_PRINT(ZLIB_FOUND)
DEBUG_PRINT(ZLIB_LIBRARIES)
DEBUG_PRINT(ZLIB_INCLUDE_DIRS)
DEBUG_PRINT(ZLIB_VERSION_STRING)
DEBUG_PRINT(ZLIB::ZLIB)

