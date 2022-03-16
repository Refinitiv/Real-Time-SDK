#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2021 Refinitiv. All rights reserved.            --
#]=============================================================================]

include(rcdevExternalUtils)

if(NOT rtsdk-binarypack_url)
	set(rtsdk-binarypack_url "https://github.com/Refinitiv/Real-Time-SDK/releases/download/Real-Time-SDK-2.0.5.L1/RTSDK-BinaryPack-2.0.5.L1.tar.xz")
endif()
if(NOT rtsdk-binarypack_hash)
	# .xz MD5 hash
	# .tar.xz 
	set(rtsdk-binarypack_hash "MD5=e76c0980b1231aaa3f5f027a4e5c71a8")
endif()
if(NOT rtsdk-binarypack_version)
	set(rtsdk-binarypack_version "2.0.5.0")
endif()
	
# If the option for using the system installed 
#  package is not defined
if( (NOT rtsdk-binarypack_USE_INSTALLED) AND 
	(NOT TARGET RTSDK-BinaryPack:rsslVACache) )
	# An external project for RTSDK-BinaryPack
	set(_EPA_NAME "rtsdk-binarypack")

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
	get_filename_component(_dl_filename "${rtsdk-binarypack_url}" NAME)
	set(_DL_METHOD	 "URL           ${rtsdk-binarypack_url}")

	if(rtsdk-binarypack_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${rtsdk-binarypack_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${rtsdk-binarypack_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	# Since the rtsdk-binarypack is a different external beast, the default
	# source and binary directories need to be overridden.  This will need to remain
	# until the Java binaries are removed or an install is created for the rtsdkbp
	set(rtsdk-binarypack_source "${CMAKE_CURRENT_SOURCE_DIR}/RTSDK-BinaryPack")
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${rtsdk-binarypack_source}")
	set(_EPA_BINARY_DIR "BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/RTSDK-BinaryPack")
	
	# There is no install step for the rtsdk-binarypack.  However,
	# if one is ever done, it should be set to this locaion
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${rtsdk-binarypack_install}")
	
	# Append the shared args to the CMake arguments to the template variable
	set(EXTERNAL_MODULE_PATH "${CMAKE_MODULE_PATH}")
	string(REPLACE ";" "|" EXTERNAL_MODULE_PATH "${EXTERNAL_MODULE_PATH}")
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
						"-DEXTERNAL_MODULE_PATH:STRING=${EXTERNAL_MODULE_PATH}"
						"-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
						"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
						"${_shared_arg}"
						)
	if(UNIX)
		list(APPEND _EPA_CMAKE_ARGS "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
	endif()

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND can be
	# used and a seperate config step does not need to be defined here.
	#  adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	#  list(APPEND _EPA_CONFIGURE_COMMAND  "CONFIGURE_COMMAND  \"\"" )

	# Typically, the build and install steps can be combined.  However, having them as 
	#  two seperate steps help in the event of having to debug a build

	# Set the <.....>_COMMAND for the build and install template fields
	# Pass the build config type along to the BUILD_COMMAND.  The CMake generator 
	# expression (<$<CONFIG:Debug>:Release....>) will invoke builds for Debug and release
	# Since this external project simply wraps a set of pre-built libraries for all available generator
	# types, the build command can be skipped as well as the install step
	set(_EPA_BUILD_COMMAND "BUILD_COMMAND  \"\"")

	# Pass the build config type along to the INSTALL_COMMAND  The CMake generator 
	# expression (<$<CONFIG:Debug>:Release....>) will invoke builds for Debug and release
	set(_EPA_INSTALL_COMMAND "INSTALL_COMMAND  \"\"")

	# If there isn't a binary directory defined then make sure
	# the option 'BUILD_IN_SOURCE' is enabled
	if (NOT DEFINED _EPA_BINARY_DIR)
		set(_EPA_ADDITIONAL_ARGS "BUILD_IN_SOURCE 1" )
	endif()

	# Add log defiitions if selected to be enabled and append them to the
	# additional args variable
	DEBUG_PRINT(rtsdk-binarypack_LOG_BUILD)
	if(rtsdk-binarypack_LOG_BUILD)
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

	# this policy is needed to supress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 rtsdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()
	
	set(RTSDK-BinaryPack_ROOT "${rtsdk-binarypack_install}" CACHE PATH "")
	set(RTSDK-BinaryPack_DIR "${RTSDK-BinaryPack_ROOT}" CACHE PATH "")
	set(rtsdk-binarypack_find_options HINTS ${rtsdk-binarypack_install} CACHE INTERNAL "")

	unset(_shared_arg)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

DEBUG_PRINT(RTSDK-BinaryPack_ROOT)
DEBUG_PRINT(RTSDK-BinaryPack_CONFIG)

endif()

if(NOT RTSDK-BinaryPack_FOUND)
	find_package(RTSDK-BinaryPack REQUIRED  "${rtsdk-binarypack_find_options}")
endif()

DEBUG_PRINT(RTSDK-BinaryPack_FOUND)
DEBUG_PRINT(RTSDK-BinaryPack_INCLUDE_DIRS)
DEBUG_PRINT(RTSDK-BinaryPack_VERSION_STRING)
DEBUG_PRINT(RTSDK-BinaryPack::dacsLib)
DEBUG_PRINT(RTSDK-BinaryPack::rsslVACache)


