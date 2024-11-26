#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved. 
#]=============================================================================]


include(rcdevExternalUtils)

if(NOT l8w8jwt_url)
	set(l8w8jwt_url "https://github.com/GlitchedPolygons/l8w8jwt.git" CACHE STRING "l8w8jwt url")
endif()

if(NOT l8w8jwt_tag)
	set(l8w8jwt_tag "2.4.0" CACHE STRING "l8w8jwt tag")
endif()

if(NOT l8w8jwt_version)
	set(l8w8jwt_version "2.4.0" CACHE STRING "l8w8jwt version")
endif()

unset(_cfg_type)
unset(_l8w8jwt_libname)
# Since our internal build types are Debug and Optimized, only Debug will translate
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(_cfg_type "${CMAKE_BUILD_TYPE}")
	set(_l8w8jwt_libname "libl8w8jwtd.a")
	list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
else()
	set(_cfg_type "Release")
	set(_l8w8jwt_libname "libl8w8jwt.a")
	list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
endif()

set(_libdir "lib")
if (RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
	set(_libdir "lib64")
endif()

# If the option for using the system installed 
#  package is not defined
if((NOT l8w8jwt_USE_INSTALLED) AND 
	(NOT TARGET l8w8jwt::libl8w8jwt) )
	set(_EPA_NAME "l8w8jwt")

	find_package(Git)
	
	find_package(Python3 REQUIRED)

	# Initialize the directory variables for the external project
	# default:
	#        external/
	#                dlcache/
	#                  BUILD/_EP_NAME/
	#                               source/
	#                               build/
	#        install/
	rcdev_init_ep_add(${_EPA_NAME})

	if(WIN32)
		set(_LIB_PATH_NAME "${l8w8jwt_install}/lib/libl8w8jwt.lib")
	else()
		set(_LIB_PATH_NAME "${l8w8jwt_install}/${_libdir}/${_l8w8jwt_libname}")
	endif()

	if(NOT EXISTS ${_LIB_PATH_NAME})

		if (EXISTS "${l8w8jwt_download}/l8w8jwt-${l8w8jwt_tag}.tar.gz")
			# Do the archive
			file(MAKE_DIRECTORY ${l8w8jwt_source})
			execute_process(COMMAND "${CMAKE_COMMAND}" -E tar x "${l8w8jwt_download}/l8w8jwt-${l8w8jwt_tag}.tar.gz"
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_source}
										ERROR_VARIABLE _cmd_out
										)
		else()
			# Clone the GitHub
			file(MAKE_DIRECTORY ${l8w8jwt_source})

			execute_process(COMMAND ${GIT_EXECUTABLE} clone --recursive  ${l8w8jwt_url}
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_source}
										ERROR_VARIABLE _cmd_out
										)

			if(_ret_val)
				message(WARNING 
					"Git pull of l8w8jwt failed!"
						"    gitcmd:${GIT_EXECUTABLE}\n"
						"     dir:${_src}\n"
						"     ret:${_ret_val}\n"
						"     out:${_cmd_out}")
			endif()

			if(GIT_VERSION_STRING VERSION_LESS 2.14.0)
				execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${l8w8jwt_tag}
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt
										ERROR_VARIABLE _cmd_out
										)
				execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --recursive
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt
										ERROR_VARIABLE _cmd_out
										)
			else()
				execute_process(COMMAND ${GIT_EXECUTABLE} checkout --recurse-submodules ${l8w8jwt_tag}
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt
										ERROR_VARIABLE _cmd_out
										)
			endif()
									
		endif()
									
		file(MAKE_DIRECTORY ${l8w8jwt_build})
		
		# check for any defined flags
		# The shared library is the default build
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")

		if(l8w8jwt_CONFIG_OPTIONS)
			set(_config_options "${l8w8jwt_CONFIG_OPTIONS}")
		else()
			# append the standard set of cmake configuration args
			set(_config_options	"-DL8W8JWT_ENABLE_TESTS:BOOL=OFF"
							"-DL8W8JWT_ENABLE_EXAMPLES:BOOL=OFF"
							"-DL8W8JWT_PACKAGE:BOOL=OFF"
							"-DL8W8JWT_ENABLE_EDDSA:BOOL=OFF"
							)
		endif()

	#As of 4/21/2021, VS2019 is showing an error as warnings issue with winbase.h, so we're going to turn off these warnings to build the library.
		if(WIN32)
			list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS} /wd4291 /wd5105 /D__func__=__FUNCTION__ "
										"-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
										"-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH_TMP}"
										"-DCMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}"
										"-DLIBDIR:STRING=${_libdir}"
										)
		else()
			list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
										"-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
										"-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH_TMP}"
										"-DCMAKE_SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}"
										"-DLIBDIR:STRING=${_libdir}"
										"-DGEN_FILES:BOOL=OFF"
										"-DMBEDTLS_FATAL_WARNINGS:BOOL=OFF"
										)
		endif()
										

		set(_config_options "${_config_options}"
						"-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
		if(WIN32)
			set( _cmd
			  "${CMAKE_COMMAND}"
			  "-H${l8w8jwt_source}/l8w8jwt"
			  "-B${l8w8jwt_build}"
			  "-G${CMAKE_GENERATOR}"
			  "${_config_options}"
				)
				
			execute_process(COMMAND ${_cmd}
							WORKING_DIRECTORY "${_source_dir}"
							RESULT_VARIABLE _ret_val
							)
		else()
			set( _cmd
			  "${CMAKE_COMMAND}"
			  "-H${l8w8jwt_source}/l8w8jwt"
			  "-B${l8w8jwt_build}"
			  "-G${CMAKE_GENERATOR}"
			  "${_config_options}"
				)
				
			execute_process(COMMAND ${_cmd}
						WORKING_DIRECTORY "${_source_dir}"
						RESULT_VARIABLE _ret_val
						)
		endif()
		
		if(_ret_val)
			message(FATAL_ERROR "External Project CMake step for l8w8jwt in ${_source_dir} failed: ${_ret_val}")
		endif()
		
		if (WIN32)
			execute_process(COMMAND ${CMAKE_COMMAND} --build .  --config Release
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_build}
										ERROR_VARIABLE _cmd_out
										)
			if(_ret_val)
				message(FATAL_ERROR "l8w8jwt release build in ${_source_dir} failed: ${_ret_val}")
			endif()
										
			execute_process(COMMAND ${CMAKE_COMMAND} --build .  --config Debug
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_build}
										ERROR_VARIABLE _cmd_out
										)
			if(_ret_val)
				message(FATAL_ERROR "l8w8jwt release build in ${_source_dir} failed: ${_ret_val}")
			endif()
			#Now package the output libs from the build into a single library and place it in the install location
			get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)
			message(STATUS "${_vs_bin_path}")
			message(STATUS "${CMAKE_GENERATOR_INSTANCE}")
			
			#Cover the VS2017+ and VS2015 vcvarsall locations
			find_file(_vcvarsall_bin vcvarsall.bat
				"${CMAKE_GENERATOR_INSTANCE}/VC/Auxiliary/Build" "${_vs_bin_path}/../../")
				
			if(${_vcvarsall_bin} EQUAL _vcvarsall_bin-NOTFOUND)
				message(FATAL_ERROR "Unable to find vcvarsall")
			endif()

			configure_file(${rtsdk_SOURCE_DIR}/CMake/l8w8jwtlib.bat.in ${l8w8jwt_build}/l8w8jwtlib.bat @ONLY)
			
			execute_process (COMMAND ${l8w8jwt_build}/l8w8jwtlib.bat
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_build}
										ERROR_VARIABLE _cmd_out
										)

		else()
			execute_process(COMMAND ${CMAKE_COMMAND} --build .
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_build}
										ERROR_VARIABLE _cmd_out
										)
			
			file(MAKE_DIRECTORY ${l8w8jwt_source}/l8w8jwt.arch)
			
			execute_process(COMMAND ${CMAKE_AR} -x ${l8w8jwt_build}/libl8w8jwt.a
							WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt.arch
							)
			# l8w8jwt and mbedtls both have a version.c.o and base64.c.o.  Rename the l8w8jwt ones to avoid collision
			file(RENAME ${l8w8jwt_source}/l8w8jwt.arch/version.c.o ${l8w8jwt_source}/l8w8jwt.arch/l8w8jwtversion.c.o)
			file(RENAME ${l8w8jwt_source}/l8w8jwt.arch/base64.c.o ${l8w8jwt_source}/l8w8jwt.arch/l8w8jwtbase64.c.o)

			execute_process(COMMAND ${CMAKE_AR} -x ${l8w8jwt_build}/mbedtls/library/libmbedcrypto.a
							COMMAND ${CMAKE_AR} -x ${l8w8jwt_build}/mbedtls/library/libmbedtls.a
							COMMAND ${CMAKE_AR} -x ${l8w8jwt_build}/mbedtls/library/libmbedx509.a
							WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt.arch
							)
							
			# Execute_process does not run these commands in a shell, it instead executes the process directly.  So wildcards need to be found through a file GLOB
			file(GLOB temp_obj ${l8w8jwt_source}/l8w8jwt.arch/*.o)
							
			execute_process(COMMAND ${CMAKE_AR} -qcs ${l8w8jwt_install}/${_libdir}/${_l8w8jwt_libname} ${temp_obj}
							WORKING_DIRECTORY ${l8w8jwt_source}/l8w8jwt.arch
							)
				
			if(_ret_val)
				message(FATAL_ERROR "Unable to package l8w8jwt ${_source_dir} failed: ${_ret_val}")
			endif()

		endif()
		
		#Copy the header files to install/include
		file(MAKE_DIRECTORY ${l8w8jwt_install}/include/l8w8jwt)
		
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${l8w8jwt_source}/l8w8jwt/include/l8w8jwt ${l8w8jwt_install}/include/l8w8jwt
										RESULT_VARIABLE _ret_val
										WORKING_DIRECTORY ${l8w8jwt_install}
										ERROR_VARIABLE _cmd_out
										)
										
		
		
	endif()
	
endif()

unset(_shared_arg)
unset(_config_args)
unset(_log_args)
unset(_dl_filename)

# This call will reset all the _EPA_... variables. Because this is a
# macro and if this is not called, the next external project using
# this template will be at risk being currupted with old values.
rcdev_reset_ep_add()

# There is no l8w8jwt find, so we just need to add the library as an imported lib here.
if(NOT TARGET l8w8jwt::libl8w8jwt)
	add_library(libl8w8jwt STATIC IMPORTED GLOBAL)
	set_property(TARGET libl8w8jwt PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${l8w8jwt_install}/include")
	if(WIN32)
		set_property(TARGET libl8w8jwt APPEND PROPERTY IMPORTED_LOCATION_RELEASE_MD "${l8w8jwt_install}/lib/libl8w8jwt.lib")
		set_property(TARGET libl8w8jwt APPEND PROPERTY IMPORTED_LOCATION_DEBUG_MDD "${l8w8jwt_install}/lib/libl8w8jwtd.lib")
	else()
		set_property(TARGET libl8w8jwt APPEND PROPERTY IMPORTED_LOCATION "${l8w8jwt_install}/${_libdir}/${_l8w8jwt_libname}")
	endif()
	
	rcdev_add_target(l8w8jwt libl8w8jwt)
endif()

unset(_libdir)

DEBUG_PRINT(l8w8jwt::libl8w8jwt)
