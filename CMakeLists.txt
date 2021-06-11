cmake_minimum_required (VERSION 3.10.3)
# The minimum version was chosen from a combination of the need for
# critical bug fixes by CMake, the idea of taking advantage of future
# CMake fixes soon to be released and wanting to fully embrace the
# target oriented CMake build framework.  It may be possible to change
# the minimum required version to an earlier version, but there is no
# guarantee of a successful build if changed.

project (rtsdk
		VERSION 2.0.2.0
		LANGUAGES C CXX)

set(rtsdk_CMAKE_DIR "${rtsdk_SOURCE_DIR}/CMake")
list(APPEND CMAKE_MODULE_PATH ${rtsdk_CMAKE_DIR})

include(rcdevEnvironment)
include(rcdevSupport)

include(rtsdkOptions)
include(rtsdkInfo)


if(BUILD_RTSDK-BINARYPACK)
	include(addExternal_rtsdk-binarypack)
endif()

if (BUILD_UNIT_TESTS)
    # Google Test
	include(addExternal_gtest)
endif()

include(addExternal_zlib)
include(addExternal_lz4)
include(addExternal_libxml2)
include(addExternal_curl)
include(addExternal_cjson)

#
# Set up default build paths if not already defined
if (RCDEV_OUTPUT_DIRECTORY_PREFIX)
    rcdev_update_output_dirs("${RCDEV_OUTPUT_DIRECTORY_PREFIX}")
else()
    rcdev_update_output_dirs("${rtsdk_SOURCE_DIR}/Cpp-C")
endif()

add_subdirectory( Cpp-C )

unset(_output_dir_prefix)

