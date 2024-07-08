#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.
#]=============================================================================]

if (_rcdevBuildConfigInclude)
    return()
else()
	set(_rcdevBuildConfigInclude TRUE)
endif()

macro(rcdev_map_init_build_types _lang)
	list(APPEND _cmake_dflt Debug Release MinSizeRel RelWithDebInfo)
	if (WIN32)
		list(APPEND _rcdev_dflt Debug_MDd Release_MD NOTDEFINED NOTDEFINED)
	else()
		list(APPEND _rcdev_dflt DEBUG OPTIMIZED NOTDEFINED OPTIMIZEDDEBUG)
	endif()
	list(LENGTH _cmake_dflt _len)
	math(EXPR _len "${_len}-1")
	#get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
	#message("In rcdevBuildConfig :  ENABLED_LANGUAGES:${_languages}")
	foreach(_idx RANGE ${_len})
		list(GET _rcdev_dflt ${_idx} _rc_bld_typ)
		list(GET _cmake_dflt ${_idx} _cm_bld_typ)
		if (NOT (_rc_bld_typ MATCHES "NOTDEFINED"))
			string(TOUPPER "${_cm_bld_typ}" _cm_bld_typ)
			string(TOUPPER "${_rc_bld_typ}" _rc_bld_typ)
			if (NOT DEFINED CMAKE_${_lang}_FLAGS_${_rc_bld_typ}_INIT)
					#message(STATUS "\tNow set CMAKE_${_lang}_FLAGS_${_rc_bld_typ}_INIT = ${CMAKE_${_lang}_FLAGS_${_cm_bld_typ}_INIT}")
				set(CMAKE_${_lang}_FLAGS_${_rc_bld_typ}_INIT 
								${CMAKE_${_lang}_FLAGS_${_cm_bld_typ}_INIT} CACHE INTERNAL "")
			else()
				string(APPEND CMAKE_${_lang}_FLAGS_${_rc_bld_typ}_INIT 
								"${CMAKE_${_lang}_FLAGS_${_cm_bld_typ}_INIT}")
			endif()
		endif()
	endforeach()
	unset(_idx)
	unset(_len)
	unset(_rcdev_dflt)
	unset(_cmake_dflt)
	unset(_rc_bld_typ)
	unset(_cm_bld_typ)
endmacro()
# Write the cache initializer script
function(rcdev_update_initial_cache)
	# Remove from the default config types and initial values from cache 
	#  since they are not supported by this framework
	get_cmake_property(_c_vars CACHE_VARIABLES)

	foreach(_cfg_str DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
		foreach(_m ${_c_vars})
			if (_m MATCHES "^CMAKE_+.*_${_cfg_str}$")
				get_property(_type CACHE "${_m}" PROPERTY TYPE)
				#message("CLEARING Variable ${_m}")
				unset(${_m} CACHE)
			endif()
		endforeach()
		unset(_m)
		unset(_type)
	endforeach()
	unset(_cfg_str)
endfunction()

if (CMAKE_CONFIGURATION_TYPES)
	message("Setting possible configuration types to Debug_MDd and Release_MD")
	set(CMAKE_CONFIGURATION_TYPES "Debug_MDd;Release_MD" CACHE STRING "" FORCE)
else()
	#message("CMAKE_BUILD_TYPE_INIT=${CMAKE_BUILD_TYPE_INIT}")
	if ( NOT CMAKE_BUILD_TYPE )
		#message("Default build type set to Optimized")
		set( CMAKE_BUILD_TYPE Optimized CACHE STRING "" FORCE)
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Select the build configuration type")
	# allowed build types for Linux
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Optimized;Debug;OptimizedDebug")

endif()

#[===============================================[
#]===============================================]
get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(_l "${_languages}")
	rcdev_map_init_build_types(${_l})	
endforeach()
unset(_l)
unset(_languages)

rcdev_update_initial_cache()

# TODO Decide if compileOptions should be included here
# include rcdevCompilerOptions
#[=================================================================[
	CMAKE_<LANG>_FLAGS_<BUILD CONFIG>
	CMAKE_<LANG>_FLAGS_<BUILD CONFIG>_INIT
	CMAKE_EXE_LINKER_FLAGS_<BUILD CONFIG>
	CMAKE_MODULE_LINKER_FLAGS_<BUILD CONFIG>
	CMAKE_SHARED_LINKER_FLAGS_<BUILD CONFIG>
	CMAKE_STATIC_LINKER_FLAGS_<BUILD CONFIG>
#]=================================================================]
