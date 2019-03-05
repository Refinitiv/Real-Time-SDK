#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2019. All rights reserved.            --
#]=============================================================================]


include(rcdevExternalUtils)

if(NOT cjson_url)
	set(cjson_url "https://github.com/DaveGamble/cJSON/archive/v1.7.10.tar.gz")
endif()
if(NOT cjson_hash)
	set(cjson_hash "MD5=f7ee1a04b7323440f1d7a58ea2c0c197")
endif()
if(NOT cjson_version)
	set(cjson_version "1.7.10")
endif()
	
# If the option for using the system installed 
#  package is not defined
if( (NOT cjson_USE_INSTALLED) AND 
	(NOT CJSON_FOUND) )
	# An external project for cjson
	set(_EPA_NAME "cjson")

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
	get_filename_component(_dl_filename "${cjson_url}" NAME)
	set( _DL_METHOD "URL           ${cjson_url}" )

	if(cjson_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${cjson_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${cjson_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	# the top CMake entry point is not in the top source_dir location
	# so need to define 'SOURCE_SUBDIR'
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${cjson_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${cjson_install}")

	set(_config_options	"-DENABLE_CUSTOM_COMPILER_FLAGS:BOOL=OFF"
				"-DENABLE_CJSON_TEST:BOOL=OFF"
				)
	
	# CJSON cmake build ignores this flag on UNIX type builds
	# check for any defined flags
	if(cjson_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
		set(_config_options	"${_config_options}" "-DCJSON_BUILD_SHARED_LIBS:BOOL=ON"  
							)
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options	"${_config_options}" "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
					"-DCJSON_BUILD_SHARED_LIBS:BOOL=OFF"
							)
	endif()

	# check for any defined flags
	if(cjson_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${cjson_CONFIG_OPTIONS}")
	else()
		set(cjson_CONFIG_OPTIONS "${_config_options}")
	endif()

	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d"
									"-DCMAKE_C_FLAGS:STRING=/DEBUG:NONE")
	else()
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
	# used and a separate config step does not need to be defined here.
	#  adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	#  list(APPEND _EPA_CONFIGURE_COMMAND  "CONFIGURE_COMMAND  \"\"" )

	# Typically, the build and install steps can be combined.  However, having them as 
	# two separate steps help in the event of having to debug a build
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

	# Add log definitions if selected to be enabled and append them to the
	# additional args variable
	if(cjson_LOG_BUILD)
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
	

	if (EXISTS "${CJSON_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(CJSON_LIBRARY_RELEASE "${CJSON_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	if (EXISTS "${CJSON_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(CJSON_LIBRARY_DEBUG "${CJSON_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

DEBUG_PRINT(::ALL CJSON)
DEBUG_PRINT(CJSON::CJSON)
	# this policy is needed to suppress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 esdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT CJSON_ROOT)
		set(CJSON_ROOT "${cjson_install}" CACHE INTERNAL "")
	endif()
	
	set(CJSON_INCLUDE_DIR "${cjson_install}/include")

	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being corrupted with old values.
	rcdev_reset_ep_add()

	# Since the cjson CMake Find module supports setting CJSON_ROOT
	# there is no need to set any find options here
	# set(cjson_find_options NODEFAULT_PATH HINTS "${cjson_install}")

endif()

# Find the package, for either the system installed version or the one
# just added with the external project template.  Since cjson does not have
# a find_package CMake module, we need to define the target ourselves
if(NOT CJSON_FOUND)
	if (NOT CJSON_INCLUDE_DIR)
		if (EXISTS "${CJSON_ROOT}/include/cjson/cJSON.h")
			set(CJSON_INCLUDE_DIR "${CJSON_ROOT}/include" CACHE PATH "")
		endif()
	endif()

	set(CJSON_INCLUDE_DIRS "${CJSON_INCLUDE_DIR}")

	if (NOT CJSON_LIBRARIES)
		find_library(CJSON_LIBRARY_RELEASE NAMES cjson NAMES_PER_DIR
								 PATHS ${CJSON_ROOT} NO_DEFAULT_PATH 
								 PATH_SUFFIXES lib lib64 )
		if (NOT CJSON_LIBRARY_RELEASE-NOTFOUND)
			list(APPEND CJSON_LIBRARIES Release "${CJSON_LIBRARY_RELEASE}")
			set(CJSON_LIBRARY "${CJSON_LIBRARY_RELEASE}" CACHE FILEPATH "")
		else()
			unset(CJSON_LIBRARY_RELEASE CACHE)
		endif()

		find_library(CJSON_LIBRARY_DEBUG NAMES cjsond NAMES_PER_DIR
								PATHS ${CJSON_ROOT} NO_DEFAULT_PATH 
								PATH_SUFFIXES lib lib64 )

		if (NOT CJSON_LIBRARY_DEBUG-NOTFOUND)
			list(APPEND CJSON_LIBRARIES Debug "${CJSON_LIBRARY_DEBUG}")
			if (NOT CJSON_LIBRARY)
				set(CJSON_LIBRARY "${CJSON_LIBRARY_DEBUG}" CACHE FILEPATH "")
			endif()
		else()
			unset(CJSON_LIBRARY_DEBUG CACHE)
		endif()
	endif()

	DEBUG_PRINT(::ALL CJSON)

	if ((NOT TARGET CJSON::CJSON) AND (DEFINED CJSON_LIBRARY))

		set(APPEND CJSON_LIBRARIES "Release" "${CJSON_LIBRARY_RELEASE}")
		add_library(CJSON::CJSON UNKNOWN IMPORTED)
		set_target_properties(CJSON::CJSON PROPERTIES
										INTERFACE_INCLUDE_DIRECTORIES "${CJSON_INCLUDE_DIRS}")

		set_property(TARGET CJSON::CJSON APPEND PROPERTY IMPORTED_LOCATION "${CJSON_LIBRARY}")
		if (CJSON_LIBRARY_RELEASE)
			set_property(TARGET CJSON::CJSON APPEND PROPERTY 
										 IMPORTED_CONFIGURATIONS RELEASE)
			set_target_properties(CJSON::CJSON PROPERTIES 
											IMPORTED_LOCATION_RELEASE "${CJSON_LIBRARY_RELEASE}")
		endif()

		if (CJSON_LIBRARY_DEBUG)
			set_property(TARGET CJSON::CJSON APPEND PROPERTY 
										 IMPORTED_CONFIGURATIONS DEBUG)
			set_target_properties(CJSON::CJSON PROPERTIES 
											IMPORTED_LOCATION_DEBUG "${CJSON_LIBRARY_DEBUG}")
		endif()

		if ((NOT CJSON_LIBRARY_RELEASE) AND (NOT CJSON_LIBRARY_DEBUG))
			set_property(TARGET CJSON::CJSON APPEND PROPERTY 
										IMPORTED_LOCATION "${CJSON_LIBRARY}")
		endif()

		# Will Map Release => Release_MD, Debug => Debug_Mdd and
		# on UNIX Release => Optimized
		rcdev_map_imported_ep_types(CJSON::CJSON)

		set(CJSON_FOUND true)

	endif()

	rcdev_add_external_target(CJSON::CJSON)

endif()

DEBUG_PRINT(CJSON_ROOT)
DEBUG_PRINT(CJSON::CJSON)
DEBUG_PRINT(CJSON_FOUND)
DEBUG_PRINT(CJSON_LIBRARIES)
DEBUG_PRINT(CJSON_INCLUDE_DIRS)
DEBUG_PRINT(CJSON_VERSION_STRING)
DEBUG_PRINT(CJSON_LIBRARY)
DEBUG_PRINT(CJSON_LIBRARY-NOTFOUND)
