#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license 
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details. 
 *|             Copyright (C) 2024 LSEG. All rights reserved.
#]=============================================================================]


include(rcdevExternalUtils)

if(NOT curl_url)
	set(curl_url "https://github.com/curl/curl/releases/download/curl-8_11_0/curl-8.11.0.tar.xz")
endif()
if(NOT curl_hash)
	set(curl_hash "MD5=49dd886ac84ed3de693464f78f1ee926")
endif()
if(NOT curl_version)
	set(curl_version "8.11.0")
endif()

# If the option for using the system installed 
#  package is not defined
if((NOT curl_USE_INSTALLED) AND 
	(NOT TARGET CURL::libcurl) )
	set(_EPA_NAME "curl")

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
	get_filename_component(_dl_filename "${curl_url}" NAME)
	set(_DL_METHOD	"URL           ${curl_url}" )

	if(curl_hash)
		list(APPEND _DL_METHOD "URL_HASH       ${curl_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${curl_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	set(_EPA_SOURCE_DIR "SOURCE_DIR ${curl_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${curl_install}")

	# check for any defined flags
	# The shared library is the default build
	if(curl_BUILD_STATIC_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	endif()

	# The curl distribution has a autoconf configure method a well as a CMake.
	# These _CONFIG_OPTIONS would be used if this external add was changed to build curl
	# using the autouconf method
	#[=============================================================[
	if(curl_CONFIG_OPTIONS)
		set(_config_options "${curl_CONFIG_OPTIONS}")
	else()
		set(_config_options "no-asm no-tests no-comp no-srp no-hw no-dso ")
	endif()
	#]=============================================================]

	if(curl_CONFIG_OPTIONS)
		set(_config_options "${curl_CONFIG_OPTIONS}")
	else()
		# append the standard set of cmake configuration args
		set(_config_options	"-DBUILD_CURL_EXE:BOOL=OFF"
						"-DCMAKE_USE_LIBSSH2:BOOL=OFF"
						"-DHTTP_ONLY:BOOL=ON"
						"-DBUILD_TESTING:BOOL=OFF"
						"-DENABLE_IPV6:BOOL=OFF"
						"-DENABLE_MANUAL:BOOL=OFF"
						"-DCURL_CA_PATH=none"
						"-DBUILD_LIBCURL_DOCS=OFF"
						"-DBUILD_MISC_DOCS=OFF"
						"-DENABLE_CURL_MANUAL=OFF"
						)
	endif()

	unset(_cfg_type)
	if(WIN32)
		set(_config_options "${_config_options}"
						"-DCURL_USE_SCHANNEL:BOOL=ON")
#						"-DCMAKE_USE_WINSSL:BOOL=ON")
	else()
		set(_config_options "${_config_options}" 
							"-DCURL_USE_OPENSSL:BOOL=ON")

		# Need to search for the correct OpenSsl version incase there are 64 and 32 bit 
		# versions installed, since the curl build and install will not protect itself from 
		# linking with the incorrect arch version
		find_package(PkgConfig QUIET)
		if (NOT CMAKE_PREFIX_PATH)
			set(CMAKE_PREFIX_PATH "/usr" "/usr/local")
		endif()
		string(REPLACE ";" "|" CMAKE_PREFIX_PATH_TMP "${CMAKE_PREFIX_PATH}")
		PKG_CHECK_MODULES(_OPENSSL QUIET openssl)

		DEBUG_PRINT(::ALL _OPENSSL)

		if (NOT ("x${_OPENSSL_LIBDIR}x" STREQUAL "xx"))
			list(APPEND _config_options "-DOpenSSL_DIR:PATH=${_OPENSSL_LIBDIR}")
		endif()

		# Since our internal build types are Debug and Optimized, only Debug will translate
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
										"-DCMAKE_DEBUG_POSTFIX:STRING= ")
		else()
			set(_cfg_type "Release")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
		endif()

		set(_libdir "lib")
		if (RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
			set(_libdir "lib64")
		endif()

		list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
									"-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
									"-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH_TMP}"
									"-DCMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}"
									"-DLIBDIR:STRING=${_libdir}"
									)

	endif()

	if(curl_BUILD_STATIC_LIBS)
		set(_config_options "${_config_options}"
						"-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()
	# Append the config and shared args to the CMake arguments to the template variable
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
						"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
						"${_config_options}"
						"${_shared_arg}"
						"LIST_SEPARATOR |"
						)

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND will be
	# used and one sonce not need to be defined here.
	#  Adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	#  list(APPEND _EPA_CONFIGURE_COMMAND  "CONFIGURE_COMMAND  \"\"" )

	# Typically, the build and install steps can be combined.  However, having them as 
	#  two seperate steps help in the event of having to debug a build
	# Set the <.....>_COMMAND for the build and install template fields
	# However, for this external project it is works out better to combine these next two steps
	# within the INSTALL_COMMAND step.  So, this is skipping the BUILD_COMMAND by 
	# passing "" as the argument for the BUILD_COMMAND
	if (WIN32)
		set( _EPA_BUILD_COMMAND 
					"BUILD_COMMAND     \"${CMAKE_COMMAND}\"   --build .  --config Release "  
					"COMMAND    \"${CMAKE_COMMAND}\"   --build .  --config Debug ")
	else()
		set( _EPA_BUILD_COMMAND 
					"BUILD_COMMAND    ${CMAKE_COMMAND}   --build .  --config ${_cfg_type} ")
	endif()

	# Passing the two supported build config types along to the INSTALL_COMMAND for Windows and for 
	# single build type platforms, like Linux, the current config typed is built and installed
	if (WIN32)
		set( _EPA_INSTALL_COMMAND 
					"INSTALL_COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Release "
  					"	COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Debug ")
	else()
		set( _EPA_INSTALL_COMMAND 
					"INSTALL_COMMAND    ${CMAKE_COMMAND}   --build .  --target install  --config ${_cfg_type} ")
	endif()	

	set(_EPA_TEST_COMMAND "TEST_COMMAND \"\"")

	# If there isn't a binary directory defined then make sure
	# the option 'BUILD_IN_SOURCE' is enabled
	if (NOT DEFINED _EPA_BINARY_DIR)
		set(_EPA_ADDITIONAL_ARGS "BUILD_IN_SOURCE 1" )
	endif()

	# Add log defiitions if selected to be enabled and append them to the
	# additional args variable
	if(curl_LOG_BUILD)
		set(_log_args 
						"LOG_CONFIGURE 1"
						"LOG_BUILD 1"
						"LOG_INSTALL 1"
			)
	endif()

	list(APPEND _EPA_ADDITIONAL_ARGS "${_log_args}")

	# Call cmake configure and build on the CMakeLists.txt file
	# written using the previously set template arguments
	rcdev_config_build_ep(${_EPA_NAME})

	# this policy is needed to supress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 ${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	# Not yet supported by the Curl Find module, but set it for good practice
	if(NOT CURL_ROOT)
		set(CURL_ROOT "${curl_install}" CACHE INTERNAL "")
	endif()

	unset(_libdir)
	unset(_shared_arg)
	unset(_config_args)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()
	
	set(curl_find_options PATHS ${curl_install} NO_DEFAULT_PATH)

endif()

# Find the package, for both a system installed version or the one
# just added with the ecternal project template
if ((NOT CURL_FOUND) OR
	(NOT TARGET CURL::libcurl))
	# Calling find_package with a required version number will fail if the
	# package does not have a <name>version.cmake in the same location as
	# the <package>config.cmake.  Unfortunately, CMake will not use the version
	# field defiition within a <package>.pc file. Also, the option to search for the
	# newly built version are passed as an argument, in case they have been defined, 
	# in lieu of an installed version
	find_package(CURL REQUIRED ${curl_find_options})

	# This condition is here since the FindCURL CMake module for version < Cmake.12.0 does not 
	#  crete an IMPORTED targte object (CURL::libcurl)
	if(NOT TARGET CURL::libcurl)
		add_library(CURL::libcurl UNKNOWN IMPORTED)
		set_target_properties(CURL::libcurl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
		set_property(TARGET CURL::libcurl APPEND PROPERTY IMPORTED_LOCATION "${CURL_LIBRARY}")
	endif()

	get_property(aliased_target TARGET CURL::libcurl PROPERTY ALIASED_TARGET)
	if("${aliased_target}" STREQUAL "")
		# is not an alias
		rcdev_map_imported_ep_types(CURL::libcurl)
	else()
		# is an alias
		rcdev_map_imported_ep_types(${aliased_target})
	endif()

	if( (DEFINED CURL_VERSION_STRING) AND
		(CURL_VERSION_STRING VERSION_LESS "${curl_version}") )
		message(WARNING
				"  libcurl ver:${CURL_VERSION_STRING} found, is older than the supported ver:${curl_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	# In case the version string from the package config is defined
	elseif( (DEFINED CVF_VERSION_MAJOR) AND
		(CVF_VERSION_MAJOR VERSION_LESS "${curl_version}") )
		message(WARNING
				"  libcurl ver:${CVF_VERSION_MAJOR } found, is older than the supported ver:${curl_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	endif()

	rcdev_add_external_target(CURL::libcurl)

endif()

DEBUG_PRINT(OpenSSL::SSL)
DEBUG_PRINT(OpenSSL::Crypto)
DEBUG_PRINT(CURL_FOUND)
DEBUG_PRINT(CURL_LIBRARY)
DEBUG_PRINT(CURL_VERSION_STRING)
DEBUG_PRINT(CURL::libcurl)


