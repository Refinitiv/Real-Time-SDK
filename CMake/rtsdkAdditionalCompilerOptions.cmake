#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
#]=============================================================================]


if(CMAKE_HOST_WIN32)
	if(NOT BUILD_ETA_JWT)
		set(_jwtBuildFlags "/D NO_ETA_JWT_BUILD ")
		
		set(_rtsdk_flags "${RTSDK_EXTRA_FLAGS} ${_jwtBuildFlags} ")
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
else()
	if(NOT BUILD_ETA_JWT)
		set(_jwtBuildFlags "-DNO_ETA_JWT_BUILD ")
		
		set(_rtsdk_flags "${RTSDK_EXTRA_FLAGS} ${_jwtBuildFlags} ")
		set(RTSDK_EXTRA_FLAGS "${_rtsdk_flags}" CACHE STRING "Internal RTSDK build flags" FORCE)
		unset(_rtsdk_flags)
	endif()
endif()

unset(_jwtBuildFlags)