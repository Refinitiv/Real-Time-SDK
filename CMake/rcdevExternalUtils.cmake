#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]

# Before returning to avoid multiple includes, unset all the template
# variables within the CMake template config file, CMakeLists.addExternal.txt.in.
# This will prevent values from being carried over from multiple addExternal includes
unset(_DL_METHOD)
unset(_EPA_NAME)
unset(_EPA_SOURCE_DIR)
unset(_EPA_BINARY_DIR)
unset(_EPA_INSTALL_DIR)
unset(_EPA_CACHE_ARGS)
unset(_EPA_CMAKE_ARGS)
unset(_EPA_CONFIGURE_COMMAND)
unset(_EPA_UPDATE_COMMAND)
unset(_EPA_BUILD_COMMAND)
unset(_EPA_INSTALL_COMMAND)
unset(_EPA_TEST_COMMAND)
unset(_EPA_ADDITIONAL_ARGS)

if (_rcdevExternalUtils)
	return()
else()
	set(_rcdevExternalUtils TRUE)
endif()

#[=============================================================[
##  _init_ep_add input: _ep_name :string external project name
##
##   initializes the following directory layout by default, using the 
##     input argument _ep_name
##     out: <_ep_name>_projroot, <_ep_name>_source, <_ep_name>_build,
##          <_ep_name>_download, <_ep_name>_install
##
## The default directory prefix path for all external build/install
## related directories:
##   RCDEV_EXTERNAL_BINARY_PREFIX/  <== if defined else,
##   CMAKE_CURRENT_BINARY_DIR/      <== else, within the build dir of the proj
##                                      including the addExternal module
##                                         'include(addExternal_xxx)'
##                            external/
##
##   RCDEV_EXTERNAL_INSTALL_PREFIX/ <== if defined else,
##   CMAKE_CURRENT_BINARY_DIR/      <== else, within the build dir of the proj
##                                      including the addExternal module
##                                         'include(addExternal_xxx)'
##                            install/
##
##   RCDEV_EXTERNAL_DLCACHE_PREFIX/  <== if defined else,
##                    external/      <== else, within the external directory
##                                       prefix location
##                            dlcache/
##
## Default Layout:
## CMAKE_CURRENT_BINARY_DIR/
##                         external/
##                                 dlcache/   <== <_ep_name>_download, 
##                                 BUILD/<_ep_name>/ <== <_ep_name>_projroot
##                                          source/ <== <_ep_name>_source
##                                          build/  <== <_ep_name>_build
##                         install/   <== <_ep_name>_install,
#]=============================================================]
macro(rcdev_init_ep_add _ep_name)

	if(RCDEV_EXTERNAL_BINARY_PREFIX)
		set(_external_dir "${RCDEV_EXTERNAL_BINARY_PREFIX}/external")
	else()
		set(_external_dir "${CMAKE_CURRENT_BINARY_DIR}/external")
	endif()

	if(RCDEV_EXTERNAL_INSTALL_PREFIX)
		set(_install_dir "${RCDEV_EXTERNAL_INSTALL_PREFIX}")
	else()
		set(_install_dir "${CMAKE_CURRENT_BINARY_DIR}/install")
	endif()

	if(RCDEV_EXTERNAL_DLCACHE_PREFIX)
		set(_download_dir "${RCDEV_EXTERNAL_DLCACHE_PREFIX}/dlcache")
	else()
		set(_download_dir "${_external_dir}/dlcache")
	endif()

	set("${_ep_name}_install" "${_install_dir}")
DEBUG_PRINT("${_ep_name}_install")

	set("${_ep_name}_download" "${_download_dir}")
DEBUG_PRINT("${_ep_name}_download")

	set("${_ep_name}_projroot"	"${_external_dir}/BUILD/${_ep_name}")
DEBUG_PRINT("${_ep_name}_projroot")

	set("${_ep_name}_source"	"${${_ep_name}_projroot}/source")
DEBUG_PRINT("${_ep_name}_source")

	set("${_ep_name}_build"	"${${_ep_name}_projroot}/build")
DEBUG_PRINT("${_ep_name}_build")

	# If this addExternal_ module was included and not selected during pre-config,
	# then turn the option on for correct state in cmake-gui and any addition re-config,
	# attempts
	if(NOT "addExternal_Opt_${_ep_name}")
		set(addExternal_Opt_${_ep_name} ON CACHE BOOL "" FORCE)
	endif()

	unset(_external_dir)
	unset(_install_dir)
	unset(_download_dir)

endmacro()

macro(rcdev_reset_ep_add)

	# Clear out template values
	unset(_DL_METHOD)
	unset(_EPA_SOURCE_DIR)
	unset(_EPA_BINARY_DIR)
	unset(_EPA_INSTALL_DIR)
	unset(_EPA_CACHE_ARGS)
	unset(_EPA_CMAKE_ARGS)
	unset(_EPA_CONFIGURE_COMMAND)
	unset(_EPA_UPDATE_COMMAND)
	unset(_EPA_BUILD_COMMAND)
	unset(_EPA_INSTALL_COMMAND)
	unset(_EPA_TEST_COMMAND)
	unset(_EPA_ADDITIONAL_ARGS)

endmacro()

#[=============================================================[
##  _config_build_ep input: _ep_name :string external project name
##
##  based off of the directory structure set up by the initial call to
##  _init_ep_add(), will run the cmake config, build steps for the cmake
##  entrypoint written for this external project, _ep_name.  Before the cmake
## configure step, a cmake entry point is written in $(${_ep_name}_projroot}
##  
##     input argument _ep_name
##      out: External project _ep_name will be configured and built
##
#]=============================================================]
macro(rcdev_config_build_ep _ep_name)
	set(_flags ADD_SUBDIR)
	cmake_parse_arguments(_arg "${_flags}" "" "" ${ARGN})

	set(_source_dir "${${_ep_name}_projroot}")
	if (_arg_ADD_SUBDIR)
		set(_build_dir ".")
	else()
		set(_build_dir "${${_ep_name}_build}")
	endif()

	configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.addExternal.txt.in
							"${_source_dir}/CMakeLists.txt" @ONLY)

	set( _cmd
		  "${CMAKE_COMMAND}"
		  "-H${_source_dir}"
		  "-B${_build_dir}"
		  "-G${CMAKE_GENERATOR}"
	  )
					
	# Future enhancement to add a cache file to the command line for the
	# external project to use
	#[==================================================================[
	set(_cache_filename "${CMAKE_CURRENT_BINARY_DIR}/${_ep_name}_cache.txt")
	message("\t\t CACHE FILE _cache_filename:${_cache_filename}")
	if(EXISTS "${_cache_filename}")
		 list(APPEND _cmd "-C" "${_cache_filename}")
	endif()
	#]==================================================================]

	execute_process(COMMAND ${_cmd}
							WORKING_DIRECTORY "${_source_dir}"
							RESULT_VARIABLE _ret_val
							)
	if(_ret_val)
		message(FATAL_ERROR "External Project CMake step for ${_ep_name} in ${_source_dir} failed: ${_ret_val}")
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} --build "${_build_dir}" 
							WORKING_DIRECTORY  "${_source_dir}"
							RESULT_VARIABLE _ret_val
							)
	if(_ret_val)
		message(FATAL_ERROR "External Project Build step for ${_ep_name} in ${_source_dir} failed: ${_ret_val}")
	endif()

	unset(_cmd)
	unset(_ret_val)
	unset(_flags)
	unset(_arg_ADD_SUBDIR)
	unset(_source_dir)
	unset(_build_dir)

endmacro()

#[=============================================================[
##  _init_ep_add input: _ep_name :string external project name
##
##   Map the imported build types for Release and Debug to
##                                    Release_MD and Debug_MDd
##                           for UNIX Optimized to Release
##     input argument _ep_name
##     out: mappped target properties
##
#]=============================================================]
macro(rcdev_map_imported_ep_types _ep_target)

	#[========================================================[
	 The CMAKE_MAP_ variables cannot be used without having an effect on
	 other externally added projects which my be a shared library with only one build type.
	 So inteaed of setting and unsetting these multiple times, the target properties for an 
	 imported project will be set when they are needed
	if (WIN32)
		if (NOT CMAKE_MAP_IMPORTED_CONFIG_DEBUG_MDD)
			set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG_MDD Debug)
		endif()
		if (NOT CMAKE_MAP_IMPORTED_CONFIG_RELEASE_MD)
			set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE_MD Release)
		endif()
	else()
		if (NOT CMAKE_MAP_IMPORTED_CONFIG_OPTIMIZED)
			set(CMAKE_MAP_IMPORTED_CONFIG_OPTIMIZED Release)
		endif()
	endif()
	#]========================================================]

	get_target_property(_cfg_list ${_ep_target} IMPORTED_CONFIGURATIONS)
	if (WIN32)
		# If Release is defined and map it to Release_MD if has not been already
		if (("Release" IN_LIST _cfg_list) OR ("RELEASE" IN_LIST _cfg_list)) 
			get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_RELEASE_MD SET)
			if (NOT (_pval))
				set_property(TARGET ${_ep_target} APPEND PROPERTY
			   									MAP_IMPORTED_CONFIG_RELEASE_MD Release)
			endif()
		endif()
		# If Debug is defined and map it to Debug_MDD if has not been already
		if (("Debug" IN_LIST _cfg_list) OR ("DEBUG" IN_LIST _cfg_list)) 
			get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_DEBUG_MDD SET)
			if (NOT (_pval))
				set_property(TARGET ${_ep_target} APPEND PROPERTY
												MAP_IMPORTED_CONFIG_DEBUG_MDD Debug)
			endif()
			# If Debug is not defined, map Release to Debug_MDD if has not been already
		else()
			get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_DEBUG_MDD SET)
			if (NOT (_pval))
				set_property(TARGET ${_ep_target} APPEND PROPERTY
						MAP_IMPORTED_CONFIG_DEBUG_MDD Release)
			endif()
		endif()
	else()
		# If build type is OptimizedDebug, then map Release if it is defined
        if (CMAKE_BUILD_TYPE MATCHES "OptimizedDebug")
            if (("Release" IN_LIST _cfg_list) OR ("RELEASE" IN_LIST _cfg_list))
                    get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_OPTIMIZEDDEBUG SET)
                if (NOT (_pval))
                    set_property(TARGET ${_ep_target} APPEND PROPERTY
                                            MAP_IMPORTED_CONFIG_OPTIMIZEDDEBUG Release)
                endif()
            endif()
		# If build type is Optimized, then map Release if it is defined
        elseif (CMAKE_BUILD_TYPE MATCHES "Optimized")
            if (("Release" IN_LIST _cfg_list) OR ("RELEASE" IN_LIST _cfg_list))
                    get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_OPTIMIZED SET)
                if (NOT (_pval))
                    set_property(TARGET ${_ep_target} APPEND PROPERTY
                                            MAP_IMPORTED_CONFIG_OPTIMIZED Release)
                endif()
            endif()
		# The build type is Debug, and map Release if it is defined
		else()
			if (NOT ("Release" IN_LIST _cfg_list) OR ("RELEASE" IN_LIST _cfg_list)) 
				get_property(_pval TARGET ${_ep_target} PROPERTY MAP_IMPORTED_CONFIG_OPTIMIZED SET)
				if (NOT (_pval))
					set_property(TARGET ${_ep_target} APPEND PROPERTY
											MAP_IMPORTED_CONFIG_DEBUG Release)
				endif()
			endif()
		endif()
	endif()

	unset(_pval)
	unset(_cfg_list)

endmacro()

