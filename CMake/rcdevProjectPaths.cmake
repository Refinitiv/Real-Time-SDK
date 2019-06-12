#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]


if(RCDEV_POST_PREFIX_TYPE)
	string(TOUPPER "${RCDEV_POST_PREFIX_TYPE}" RCDEV_POST_PREFIX_TYPE)
	DEBUG_PRINT(::MESSAGE "Upper case RCDEV_POST_PREFIX_TYPE=${RCDEV_POST_PREFIX_TYPE}")
	if (RCDEV_POST_PREFIX_TYPE STREQUAL "NEW" OR
		RCDEV_POST_PREFIX_TYPE STREQUAL "CLASSIC" )
		# Update the cache entry for ccmake or cmake-gui to see
		set (RCDEV_POST_PREFIX_TYPE "${RCDEV_POST_PREFIX_TYPE}" CACHE STRING "The type of post prefix path used, \[NEW\|CLASSIC\]" FORCE)
	else()
		message(FATAL_ERROR 
				"Invalid value for RCDEV_POST_PREFIX_TYPE:(${RCDEV_POST_PREFIX_TYPE})\n"
				"\t\tValid values are \"NEW\" or \"CLASSIC\"\n")
	endif()
else()
	set (RCDEV_POST_PREFIX_TYPE "NEW" CACHE STRING 
						"The type of post prefix path used, \[NEW\|CLASSIC\]" FORCE)
endif()
set_property(CACHE RCDEV_POST_PREFIX_TYPE PROPERTY HELPSTRING "The type of post prefix path used, \[NEW\|CLASSIC\]")
set_property(CACHE RCDEV_POST_PREFIX_TYPE PROPERTY STRINGS "NEW;CLASSIC")



rcdev_set_classic_postprefix_path()
rcdev_set_new_postprefix_path()

if(WIN32)
	set (RCDEV_POST_PREFIX_PATH_RELEASE_MD
				"${RCDEV_POST_PREFIX_${RCDEV_POST_PREFIX_TYPE}_PATH_RELEASE_MD}"
				CACHE STRING "" FORCE)
	set (RCDEV_POST_PREFIX_PATH_DEBUG_MDD
				"${RCDEV_POST_PREFIX_${RCDEV_POST_PREFIX_TYPE}_PATH_DEBUG_MDD}"
				CACHE STRING "" FORCE)
	DEBUG_PRINT(::OUTPATH 
				"Set Post Prefix Paths:"
				"\tRCDEV_POST_PREFIX_PATH_RELEASE_MD : ${RCDEV_POST_PREFIX_PATH_RELEASE_MD}"
				"\tRCDEV_POST_PREFIX_PATH_DEBUG_MDD : ${RCDEV_POST_PREFIX_PATH_DEBUG_MDD} "
				)
else()
	set(RCDEV_POST_PREFIX_PATH "${RCDEV_POST_PREFIX_${RCDEV_POST_PREFIX_TYPE}_PATH}" 
							CACHE STRING "" FORCE)
	DEBUG_PRINT(::OUTPATH 
				"Set Post Prefix Path :"
				"\tRCDEV_POST_PREFIX_PATH : ${RCDEV_POST_PREFIX_PATH} ")
endif()

if(NOT INSTALL_LIBRARY_DIR)
  set(INSTALL_LIBRARY_DIR "lib")
endif()
if(NOT INSTALL_RUNTIME_DIR)
  set(INSTALL_RUNTIME_DIR "bin")
endif()
if(NOT INSTALL_ARCHIVE_DIR)
  set(INSTALL_ARCHIVE_DIR "lib")
endif()
if(NOT INSTALL_INCLUDE_DIR)
  set(INSTALL_INCLUDE_DIR "include")
endif()


