#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
#]=============================================================================]


include(rcdevExternalUtils)

set(lz4_version_10 "1.10.0" CACHE STRING "")

if( UNIX AND (RCDEV_HOST_SYSTEM_FLAVOR_REL LESS_EQUAL 7) )
	if(NOT lz4_url)
		# The 1.9.4 version is needed to support GCC 4.8.2 for OL7 only.
		set(lz4_url "https://github.com/lz4/lz4/archive/refs/tags/v1.9.4.tar.gz")
	endif()
	if(NOT lz4_hash)
		set(lz4_hash "MD5=e9286adb64040071c5e23498bf753261")
	endif()
	if(NOT lz4_version)
		set(lz4_version "1.9.4")
	endif()
else()
	if(NOT lz4_url)
		# The 1.10.0 version is needed to support Visual Studio 2022 version 17.10.5 or higher.
		set(lz4_url "https://github.com/lz4/lz4/archive/refs/tags/v${lz4_version_10}.tar.gz")
	endif()
	if(NOT lz4_hash)
		set(lz4_hash "MD5=dead9f5f1966d9ae56e1e32761e4e675")
	endif()
	if(NOT lz4_version)
		set(lz4_version "${lz4_version_10}")
	endif()
endif()

# If the option for using the system installed package is not defined
# since lz4 does not have a published CMake find module. However,
# if a set of previously built lz4 binaries exist outside this build
# tree, this provides the option to use them
if((NOT lz4_USE_INSTALLED) AND 
	(NOT TARGET LZ4::LZ4) )

	# An external project for liblz4
	set(_EPA_NAME "lz4")

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
	get_filename_component(_dl_filename "${lz4_url}" NAME)
	set( _DL_METHOD "URL           ${lz4_url}")

	if(lz4_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${lz4_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${lz4_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_EPA_NAME}-${_dl_filename}" )
	endif()

	# the top CMake entry point is not in the top source_dir location
	# so need to define 'SOURCE_SUBDIR'
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${lz4_source}"
						"SOURCE_SUBDIR build/cmake")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${lz4_install}")

	# check for any defined flags
	if(lz4_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON"
					   "-DBUILD_STATIC_LIBS:BOOL=OFF")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF"
					   "-DBUILD_STATIC_LIBS:BOOL=ON")
		set(_config_options "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()

	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d"
									"-DCMAKE_C_FLAGS:STRING=/DEBUG:NONE")
	else()
		# Since our internal build types are Debug and Optimized, only Debug will translate
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d" 
										"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
		else()
			set(_cfg_type "Release")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
		endif()
            
		list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
                                    "-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
                                        )
	endif()	

	# Append the shared args to the CMake arguments to the template variable
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
						"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
						"${_config_options}"
						"${_shared_arg}"
						)

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND can be
	# used and a seperate config step does not need to be defined here.
	#  Adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
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
	if(lz4_LOG_BUILD)
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
		#message("Setting CMake policy CMP0074  rtsdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT LZ4_ROOT)
		set(LZ4_ROOT "${lz4_install}")
	endif()

	set(LZ4_INCLUDE_DIR "${lz4_install}/include")

	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

endif()

# Find the package, for either the system installed version or the one
# just added with the external project template.  Since lz4 does not have
# a find_package CMake module, we need to define the target ourselves
if ((NOT LZ4_FOUND) OR
	(NOT TARGET LZ4::LZ4) )
	if (NOT LZ4_INCLUDE_DIR)
		if (EXISTS "${LZ4_ROOT}/include/lz4.h")
			set(LZ4_INCLUDE_DIR "${LZ4_ROOT}/include" CACHE PATH "")
		endif()
	endif()

	set(LZ4_INCLUDE_DIRS "${LZ4_INCLUDE_DIR}")

	if (NOT LZ4_LIBRARIES)
		find_library(LZ4_LIBRARY_RELEASE NAMES lz4 NAMES_PER_DIR
								 PATHS ${LZ4_ROOT} NO_DEFAULT_PATH 
								 PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )
		if (NOT (LZ4_LIBRARY_RELEASE MATCHES "NOTFOUND"))
			list(APPEND LZ4_LIBRARIES Release "${LZ4_LIBRARY_RELEASE}")
			set(LZ4_LIBRARY "${LZ4_LIBRARY_RELEASE}" CACHE FILEPATH "")
		else()
			unset(LZ4_LIBRARY_RELEASE CACHE)
		endif()

		find_library(LZ4_LIBRARY_DEBUG NAMES lz4d lz4 NAMES_PER_DIR
								 PATHS ${LZ4_ROOT} NO_DEFAULT_PATH 
								 PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )

		if (NOT (LZ4_LIBRARY_DEBUG MATCHES "NOTFOUND"))
				list(APPEND LZ4_LIBRARIES Debug "${LZ4_LIBRARY_DEBUG}")
			if (NOT LZ4_LIBRARY)
				set(LZ4_LIBRARY "${LZ4_LIBRARY_DEBUG}" CACHE FILEPATH "")
			endif()
		else()
			unset(LZ4_LIBRARY_DEBUG CACHE)
		endif()
	endif()

	if (NOT LZ4_LIBRARY)
		find_library(LZ4_LIBRARY NAMES lz4 lz4d NAMES_PER_DIR
								 PATHS ${LZ4_ROOT} NO_DEFAULT_PATH 
								 PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )
	endif()

	if ((NOT TARGET LZ4::LZ4) AND (DEFINED LZ4_LIBRARY))

		add_library(LZ4::LZ4 UNKNOWN IMPORTED)
		set_target_properties(LZ4::LZ4 PROPERTIES
										INTERFACE_INCLUDE_DIRECTORIES "${LZ4_INCLUDE_DIRS}")

		set_property(TARGET LZ4::LZ4 APPEND PROPERTY IMPORTED_LOCATION "${LZ4_LIBRARY}")
		if (WIN32)
			if (LZ4_LIBRARY_RELEASE)
				#set(APPEND LZ4_LIBRARIES "Release" "${LZ4_LIBRARY_RELEASE}")
				set_property(TARGET LZ4::LZ4 APPEND PROPERTY 
											 IMPORTED_CONFIGURATIONS RELEASE)
				set_target_properties(LZ4::LZ4 PROPERTIES 
												IMPORTED_LOCATION_RELEASE "${LZ4_LIBRARY_RELEASE}")
			endif()

			if (LZ4_LIBRARY_DEBUG)
				set_property(TARGET LZ4::LZ4 APPEND PROPERTY 
											 IMPORTED_CONFIGURATIONS DEBUG)
				set_target_properties(LZ4::LZ4 PROPERTIES 
												IMPORTED_LOCATION_DEBUG "${LZ4_LIBRARY_DEBUG}")
			endif()

			if ((NOT LZ4_LIBRARY_RELEASE) AND (NOT LZ4_LIBRARY_DEBUG))
				set_property(TARGET LZ4::LZ4 APPEND PROPERTY 
											IMPORTED_LOCATION "${LZ4_LIBRARY}")
			endif()

			# Will Map Release => Release_MD, Debug => Debug_Mdd
			rcdev_map_imported_ep_types(LZ4::LZ4)

		endif()

		set(LZ4_FOUND true)

	endif()

	rcdev_add_external_target(LZ4::LZ4)

endif()

DEBUG_PRINT(LZ4_ROOT)
DEBUG_PRINT(LZ4::LZ4)
DEBUG_PRINT(LZ4_FOUND)
DEBUG_PRINT(LZ4_LIBRARY)
DEBUG_PRINT(LZ4_INCLUDE_DIRS)

