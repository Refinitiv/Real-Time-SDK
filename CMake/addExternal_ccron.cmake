#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
#]=============================================================================]

################################################################################################
#[=============================================================================================[
WARNING:  The logic contained within this block is a hack.  This hack is neccessary to account 
for the wrong gcc build falg "-Wconversion" wich drives to the build failure on Linux. 
The following "work-around" will provide the logic for a patch step which will write out 
a modified CMakeLists.txt file without wrong gcc flag. This does not  address any other 
opportunities within the stock ccron CMake build.
#]=============================================================================================]
function(_write_custom_ccronexpr_entrypoint _file_name)

file(WRITE ${_file_name}
"
cmake_minimum_required(VERSION 3.0)
project(ccronexpr)

# Library
add_library(ccronexpr STATIC ccronexpr.c)
target_include_directories(ccronexpr PUBLIC .)

# Supertinycron binary
#add_executable(supertinycron supertinycron.c)
#target_link_libraries(supertinycron ccronexpr)

if (ESP_PLATFORM)
    target_compile_definitions(ccronexpr PRIVATE ESP_PLATFORM=1)
    set(CRON_DISABLE_TESTING ON) # disable tests automatically
endif ()

if (CRON_USE_LOCAL_TIME)
    target_compile_definitions(ccronexpr PUBLIC CRON_USE_LOCAL_TIME=1)
endif ()

if (CRON_COMPILE_AS_CXX)
    target_compile_definitions(ccronexpr PUBLIC CRON_COMPILE_AS_CXX=1)
endif ()

if (MSVC)
    # Strict compilation
    target_compile_options(ccronexpr PRIVATE /W4 /WX)
    # But ignore _s functions
    target_compile_definitions(ccronexpr PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
    # Strict compilation
    target_compile_options(ccronexpr PRIVATE -ansi -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wcast-qual -Wno-unused-parameter -pedantic-errors)
endif ()

# Tests
if (NOT CRON_DISABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif ()
"
)

endfunction()
################################################################################################
#[=============================================================================================[
End of work around for Linux ccron wrong gcc flag issue
#]=============================================================================================]

include(rcdevExternalUtils)

if(NOT ccronexpr_url)
	set(ccronexpr_url "https://github.com/exander77/supertinycron/archive/refs/tags/v2.0.0.tar.gz")
endif()
if(NOT ccronexpr_hash)
	set(ccronexpr_hash "MD5=51182564507d717b9a340f3cf587a982")
endif()
if(NOT ccronexpr_version)
	set(ccronexpr_version "2.0.0")
endif()
	
# If the option for using the system installed 
#  package is not defined
if( (NOT ccronexpr_USE_INSTALLED) AND 
	(NOT TARGET CCRONEXPR::CCRONEXPR) )
	# An external project for ccronexpr
	set(_EPA_NAME "ccronexpr")

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
	get_filename_component(_dl_filename "${ccronexpr_url}" NAME)
	set( _DL_METHOD "URL           ${ccronexpr_url}" )

	if(ccronexpr_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${ccronexpr_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${ccronexpr_download}")

	if (DEFINED _dl_filename)
		if(NOT (${_dl_filename} MATCHES "^ccronexpr"))
			list(APPEND _DL_METHOD "DOWNLOAD_NAME ccronexpr-${_dl_filename}" )
		else()
			list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
		endif()
	endif()

	# the top CMake entry point is not in the top source_dir location
	# so need to define 'SOURCE_SUBDIR'
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${ccronexpr_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${ccronexpr_install}")

	set(_config_options	"-DCRON_DISABLE_TESTING:BOOL=ON"
						"-DCRON_USE_LOCAL_TIME:BOOL=OFF"
						"-DCRON_COMPILE_AS_CXX:BOOL=OFF"
						)

	# CCRONEXPR cmake build ignores this flag on UNIX type builds
	# check for any defined flags
	if(ccronexpr_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options	"${_config_options}" "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()

	# check for any defined flags
	if(ccronexpr_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${ccronexpr_CONFIG_OPTIONS}")
	else()
		set(ccronexpr_CONFIG_OPTIONS "${_config_options}")
	endif()

	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d"
									"-DCMAKE_C_FLAGS:STRING=/DEBUG:NONE")
	else()
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d" 
										"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
		else()
			set(_cfg_type "Release")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
		endif()

		list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
									"-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}")
	endif()	
	# Append the shared args to the CMake arguments to the template variable
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
						"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
						"${_config_options}"
						"${_shared_arg}"
						)
    
	# Hack for Linux to account wrong gcc flag
	if (UNIX)
			set(_patchfile "${ccronexpr_projroot}/patch/CMakeLists.txt")
			_write_custom_ccronexpr_entrypoint(${_patchfile})
			set( _EPA_PATCH_COMMAND 
					"PATCH_COMMAND    \"${CMAKE_COMMAND}\" -E copy ${ccronexpr_source}/CMakeLists.txt ${ccronexpr_source}/CMakeLists_orig.txt
						COMMAND       \"${CMAKE_COMMAND}\" -E copy ${_patchfile} ${ccronexpr_source}
					")
			unset(_patchfile)
	endif()
    
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
	set( _EPA_BUILD_COMMAND "BUILD_COMMAND        \"\"")

	# Passing the two supported build config types along to the INSTALL_COMMAND for Windows and for 
	# single build type platforms, like Linux, the current config typed is built and installed
	if (WIN32)
			set( _EPA_INSTALL_COMMAND 
						"INSTALL_COMMAND    \"${CMAKE_COMMAND}\"   --build . --config Release "
							"        COMMAND    \"${CMAKE_COMMAND}\"   --build . --config Debug ")
	else()
			set( _EPA_INSTALL_COMMAND 
				"INSTALL_COMMAND    ${CMAKE_COMMAND}   --build . --config ${_cfg_type} ")
	endif()	

	# If there isn't a binary directory defined then make sure
	# the option 'BUILD_IN_SOURCE' is enabled
	if (NOT DEFINED _EPA_BINARY_DIR)
		set( _EPA_ADDITIONAL_ARGS "BUILD_IN_SOURCE 1" )
	endif()

	# Add log definitions if selected to be enabled and append them to the
	# additional args variable
	if(ccronexpr_LOG_BUILD)
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
	

	if (EXISTS "${CCRONEXPR_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(CCRONEXPR_LIBRARY_RELEASE "${CCRONEXPR_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()
	if (EXISTS "${CCRONEXPR_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(CCRONEXPR_LIBRARY_DEBUG "${CCRONEXPR_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	# this policy is needed to suppress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 rtsdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT CCRONEXPR_ROOT)
		set(CCRONEXPR_ROOT "${ccronexpr_install}" CACHE INTERNAL "")
	endif()
	
	set(CCRONEXPR_INCLUDE_DIR "${ccronexpr_install}/include/ccronexpr")

	if (WIN32)
		set(CCRONEXPR_LIB_DIR "${ccronexpr_install}/lib")
	else()
		set(CCRONEXPR_LIB_DIR "${ccronexpr_install}/lib${RCDEV_HOST_SYSTEM_BITS}")
	endif()

	#Copy the header files to install/include
	file(MAKE_DIRECTORY ${CCRONEXPR_INCLUDE_DIR})

	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ccronexpr_source}/ccronexpr.h ${CCRONEXPR_INCLUDE_DIR}
																				RESULT_VARIABLE _ret_val
																				WORKING_DIRECTORY ${ccronexpr_install}
																				ERROR_VARIABLE _cmd_out
										)

	if (WIN32)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ccronexpr_source}/Release/ccronexpr.lib ${CCRONEXPR_LIB_DIR}
																				RESULT_VARIABLE _ret_val
																				WORKING_DIRECTORY ${ccronexpr_install}
																				ERROR_VARIABLE _cmd_out
																				)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ccronexpr_source}/Debug/ccronexprd.lib ${CCRONEXPR_LIB_DIR}
																				RESULT_VARIABLE _ret_val
																				WORKING_DIRECTORY ${ccronexpr_install}
																				ERROR_VARIABLE _cmd_out
																				)
	else()
			execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ccronexpr_source}/libccronexpr.a ${CCRONEXPR_LIB_DIR}
																				RESULT_VARIABLE _ret_val
																				WORKING_DIRECTORY ${ccronexpr_install}
																				ERROR_VARIABLE _cmd_out
																				)
			execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ccronexpr_source}/libccronexprd.a ${CCRONEXPR_LIB_DIR}
																				RESULT_VARIABLE _ret_val
																				WORKING_DIRECTORY ${ccronexpr_install}
																				ERROR_VARIABLE _cmd_out
																				)
	endif()

	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)
	if (UNIX)
		unset(_EPA_PATCH_COMMAND)
	endif()

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being corrupted with old values.
	rcdev_reset_ep_add()

	# Since the ccronexpr CMake Find module supports setting CCRONEXPR_ROOT
	# there is no need to set any find options here
	# set(ccronexpr_find_options NODEFAULT_PATH HINTS "${ccronexpr_install}")

endif()

# Find the package, for either the system installed version or the one
# just added with the external project template.  Since ccronexpr does not have
# a find_package CMake module, we need to define the target ourselves
if ((NOT CCRONEXPR_FOUND) OR
	(NOT TARGET CCRONEXPR::CCRONEXPR))
	if (NOT CCRONEXPR_INCLUDE_DIR)
		if (EXISTS "${CCRONEXPR_ROOT}/include/ccronexpr/ccronexpr.h")
			set(CCRONEXPR_INCLUDE_DIR "${CCRONEXPR_ROOT}/include" CACHE PATH "")
		endif()
	endif()

	set(CCRONEXPR_INCLUDE_DIRS "${CCRONEXPR_INCLUDE_DIR}")

	if (NOT CCRONEXPR_LIBRARIES)
		find_library(CCRONEXPR_LIBRARY_RELEASE NAMES ccronexpr NAMES_PER_DIR
								 PATHS ${CCRONEXPR_ROOT} NO_DEFAULT_PATH 
								 PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )
		if (NOT (CCRONEXPR_LIBRARY_RELEASE MATCHES "NOTFOUND"))
			list(APPEND CCRONEXPR_LIBRARIES Release "${CCRONEXPR_LIBRARY_RELEASE}")
			set(CCRONEXPR_LIBRARY "${CCRONEXPR_LIBRARY_RELEASE}" CACHE FILEPATH "")
		else()
			unset(CCRONEXPR_LIBRARY_RELEASE CACHE)
		endif()

		find_library(CCRONEXPR_LIBRARY_DEBUG NAMES ccronexprd ccronexpr NAMES_PER_DIR
								PATHS ${CCRONEXPR_ROOT} NO_DEFAULT_PATH 
								PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )

		if (NOT (CCRONEXPR_LIBRARY_DEBUG MATCHES "NOTFOUND"))
			list(APPEND CCRONEXPR_LIBRARIES Debug "${CCRONEXPR_LIBRARY_DEBUG}")
			if (NOT CCRONEXPR_LIBRARY)
				set(CCRONEXPR_LIBRARY "${CCRONEXPR_LIBRARY_DEBUG}" CACHE FILEPATH "")
			endif()
		else()
			unset(CCRONEXPR_LIBRARY_DEBUG CACHE)
		endif()
	endif()

	if (NOT CCRONEXPR_LIBRARY)
		find_library(CCRONEXPR_LIBRARY NAMES ccronexpr ccronexprd NAMES_PER_DIR
								PATHS ${CCRONEXPR_ROOT} NO_DEFAULT_PATH 
								PATH_SUFFIXES "lib${RCDEV_HOST_SYSTEM_BITS}" lib )
	endif()

	if ((NOT TARGET CCRONEXPR::CCRONEXPR) AND (DEFINED CCRONEXPR_LIBRARY))

		add_library(CCRONEXPR::CCRONEXPR UNKNOWN IMPORTED)
		set_target_properties(CCRONEXPR::CCRONEXPR PROPERTIES
										INTERFACE_INCLUDE_DIRECTORIES "${CCRONEXPR_INCLUDE_DIRS}")

		set_property(TARGET CCRONEXPR::CCRONEXPR APPEND PROPERTY IMPORTED_LOCATION "${CCRONEXPR_LIBRARY}")
		if (WIN32)
			if (CCRONEXPR_LIBRARY_RELEASE)
				#set(APPEND CCRONEXPR_LIBRARIES "Release" "${CCRONEXPR_LIBRARY_RELEASE}")
				set_property(TARGET CCRONEXPR::CCRONEXPR APPEND PROPERTY 
											 IMPORTED_CONFIGURATIONS RELEASE)
				set_target_properties(CCRONEXPR::CCRONEXPR PROPERTIES 
												IMPORTED_LOCATION_RELEASE "${CCRONEXPR_LIBRARY_RELEASE}")
			endif()

			if (CCRONEXPR_LIBRARY_DEBUG)
				set_property(TARGET CCRONEXPR::CCRONEXPR APPEND PROPERTY 
											 IMPORTED_CONFIGURATIONS DEBUG)
				set_target_properties(CCRONEXPR::CCRONEXPR PROPERTIES 
												IMPORTED_LOCATION_DEBUG "${CCRONEXPR_LIBRARY_DEBUG}")
			endif()

			if ((NOT CCRONEXPR_LIBRARY_RELEASE) AND (NOT CCRONEXPR_LIBRARY_DEBUG))
				set_property(TARGET CCRONEXPR::CCRONEXPR APPEND PROPERTY 
											IMPORTED_LOCATION "${CCRONEXPR_LIBRARY}")
			endif()

			# Will Map Release => Release_MD, Debug => Debug_Mdd and
			rcdev_map_imported_ep_types(CCRONEXPR::CCRONEXPR)

		endif()

		set(CCRONEXPR_FOUND true)

	endif()

	rcdev_add_external_target(CCRONEXPR::CCRONEXPR)

endif()

DEBUG_PRINT(CCRONEXPR_ROOT)
DEBUG_PRINT(CCRONEXPR::CCRONEXPR)
DEBUG_PRINT(CCRONEXPR_FOUND)
DEBUG_PRINT(CCRONEXPR_LIBRARIES)
DEBUG_PRINT(CCRONEXPR_INCLUDE_DIRS)
DEBUG_PRINT(CCRONEXPR_VERSION_STRING)
DEBUG_PRINT(CCRONEXPR_LIBRARY)
