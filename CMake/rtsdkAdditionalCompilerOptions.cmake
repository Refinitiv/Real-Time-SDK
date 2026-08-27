#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
#]=============================================================================]

# Helper function to apply the ENABLE_VERBOSE_JSON_ERROR compile definition to a target if the option is enabled.
# This is used to control verbose error output in the JSON converter.
function(rtsdk_apply_verbose_json_error target_name)
	if(NOT TARGET ${target_name})
		message(FATAL_ERROR "rtsdk_apply_verbose_json_error: target '${target_name}' does not exist")
	endif()

	if(ENABLE_VERBOSE_JSON_ERROR)
		target_compile_definitions(${target_name} PRIVATE ENABLE_VERBOSE_JSON_ERROR)
	endif()
endfunction()

if(CMAKE_HOST_WIN32)
	if(NOT BUILD_ETA_JWT)
		set(_jwtBuildFlags "/D NO_ETA_JWT_BUILD")
		
		set(_rtsdk_flags ${RTSDK_EXTRA_FLAGS} ${_jwtBuildFlags})
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
	
	if(NOT BUILD_ETA_CPU_BIND)
		set(_cpuBindFlags "/D NO_ETA_CPU_BIND")
		
		set(_rtsdk_flags ${RTSDK_EXTRA_FLAGS} ${_cpuBindFlags})
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
else()
	if(NOT BUILD_ETA_JWT)
		set(_jwtBuildFlags "-DNO_ETA_JWT_BUILD")
		
		set(_rtsdk_flags ${RTSDK_EXTRA_FLAGS} ${_jwtBuildFlags})
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
	
	if(NOT BUILD_ETA_CPU_BIND)
		set(_cpuBindFlags "-DNO_ETA_CPU_BIND")
		
		set(_rtsdk_flags ${RTSDK_EXTRA_FLAGS} ${_cpuBindFlags})
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
endif()

unset(_jwtBuildFlags)
unset(_cpuBindFlags)
