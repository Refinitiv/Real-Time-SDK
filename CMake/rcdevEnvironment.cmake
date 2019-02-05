#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]

# The following files are intended to be included multiple times.  If the 
# files contain function or macro definitions, they are conditioned to 
# avoid being included more then once.
#

include(rcdevDebug)
include(rcdevBuildConfig)
include(rcdevSystemInfo)
include(rcdevCompilerOptions)
include(rcdevProjectPaths)

set( RCDEV_LIST_NODE_OPTIONS	UPDATE_SOURCE
								)

set( RCDEV_LIST_NODE_ONEVALUE	NAME			# intended project name 
								NODE_NAME       # source node name if different from NAME
								SOURCE_DIR		# 
								BINARY_DIR		# 
								INSTALL_DIR		# 
								REPO_NAME       # Name of source repo
								GIT_REPOSITORY  # Location of Git source repo
								GIT_TAG         # 
								URL             # 
								URL_HASH        #
								VERSION         # 
								IS_SUBNODE      # TRUE|FALSE
                                IS_ROOT_NODE	# Source repo is the root 
                                IS_BINARY_REPO  # Source repo contains and is needed only for pre-built libraries 
                                IS_EXTERNAL_REPO# Source repo is external 
                                IS_CMAKE_ENABLED# Source repo has a CMake entry point defined
								)

set( RCDEV_LIST_NODE_MULTIVALUE	DEPENDS			  # Source repo direct dependencies
								BINARY_DEPENDS    # A repository of pre-built libs/bins which
								                  # which may or may not be a CMake project
								EXTERNAL_DEPENDS  # Dependencies from an external/third-party source
								OPTIONS			  # Options to set for current or sub repos 
								CM_ARGS			  #   (argument prob not needed)
								)

#############################################################
#  Add target object to the projects list of added targets and 
#    will be made available to the wrapped find_package
#  input _t_nspace  - the namespace value for the target object
#                     i.e.  repoA for libutilsA - repoA::libutilsA
#############################################################
macro(rcdev_add_target  _t_nspace)
	set(_rtl "${RCDEV_REPO_TARGET_LIST}")
	foreach(_t ${ARGN})
		if (TARGET ${_t})
			add_library(${_t_nspace}::${_t} ALIAS ${_t})
			list(APPEND _rtl "${_t_nspace}::${_t}" "${_t}")
		endif()
	endforeach()
	set(RCDEV_REPO_TARGET_LIST	"${_rtl}" CACHE INTERNAL "")
	unset(_rtl)
endmacro()

#############################################################
#  This macro makes CMakes find_package a no-op for local targets
#  already created and existing within the build tree 
#    will be made available to the wrapped find_package
#  The check for _find_package_override, prevents this function from being 
#  included/defined multiple times and the result is an infinite loop within the
#  CMake code.
#  input ARGV  - the namespace value for the target object or
#                the target object name or
#                an external package which has a xxxConfig.cmake file or
#                is an installed system package
#  If ARGV is namespace::repoA or repoA the variable repoA_FOUND will be
#  set to 'TRUE'
#############################################################
if (NOT _find_package_override)
	set(_find_package_override TRUE CACHE BOOL INTERNAL "")

	macro(find_package)
		set(_args ${ARGN})
		
		if (${ARGV0} IN_LIST RCDEV_REPO_TARGET_LIST)
			set(_pkg ${ARGV0})
			if (${_pkg} MATCHES "^([a-zA-Z0-9_-]+)::([a-zA-Z0-9_-]+)")
				set(${CMAKE_MATCH_2}_FOUND TRUE)
			else()
				set(${_pkg}_FOUND TRUE)
			endif()
			unset(_pkg)
	DEBUG_PRINT(RCDEV_REPO_TARGET_LIST)
		else()
			if( POLICY CMP0074 )
				#message("Setting CMake policy CMP0074 find_package override ${_pkg}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
				cmake_policy(SET CMP0074 NEW)
			endif()

			_find_package(${_args})
		endif()
	endmacro()

endif()

#############################################################
#  This function makes CMakes add_library a force a pre-check looking
#  for any calls to add_library for IMPORTED targets append the GLOBAL
#    attribute.  This option will be usefule when older Find<xxx> modules
#   are called and do not add the GLOBAL attribute to their IMPORTED library.
#  The check for _add_library_override, prevents this function from being 
#  included/defined multiple times and the result is an infinite loop within the
#  CMake code.
#  input ARGV  - arguments for add_library
#
#  If ARGV has arguments for an IMPORTED library, the GLOBAL attribute
#  will be appended
#  
#############################################################
if (NOT _add_library_override)
	set(_add_library_override TRUE CACHE BOOL INTERNAL "")
	function(add_library)

		set(_args ${ARGN})
		if ("${_args}" MATCHES ";IMPORTED")
			if (NOT("${_args}" MATCHES ";GLOBAL"))
				list(APPEND _args GLOBAL)
			endif()

		endif()

		_add_library(${_args})

	endfunction()

endif()

