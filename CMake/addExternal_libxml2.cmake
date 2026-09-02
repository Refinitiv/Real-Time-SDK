#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2021-2026 LSEG. All rights reserved.
#]=============================================================================]

include(rcdevExternalUtils)

# Include this to get a standardized lib directory
if(UNIX)
	include(GNUInstallDirs)
endif()

macro(check_libxml2_installed _is_installed)

	unset(_inst)

	if(WIN32)
		set(LIBXML2_STATIC_NAME "libxml2_a${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	else()
		set(LIBXML2_STATIC_NAME "libxml2${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	endif()

	if (EXISTS "${libxml_libdir}/${LIBXML2_STATIC_NAME}")
		set(LIBXML2_LIBRARY "${libxml_libdir}/${LIBXML2_STATIC_NAME}" CACHE FILEPATH "")
		set(_inst TRUE)
	endif()

	# These two assignments will ensure the call to find package will locate the package
	# and define the target
	if (EXISTS "${libxml2_install}/include/libxml2/libxml/parser.h")
		set(LIBXML2_INCLUDE_DIR "${libxml2_install}/include/libxml2" CACHE PATH "")
	else()
		set(_inst FALSE)
	endif()

	set(${_is_installed} ${_inst})

	unset(_inst)

endmacro()


if(NOT libxml2_url)
	set(libxml2_url "https://download.gnome.org/sources/libxml2/2.15/libxml2-2.15.3.tar.xz")
endif()
if(NOT libxml2_hash)
	set(libxml2_hash "SHA256=78262a6e7ac170d6528ebfe2efccdf220191a5af6a6cd61ea4a9a9a5042c7a07")
endif()
if(NOT libxml2_version)
	set(libxml2_version "2.15.3")
endif()

# If the option for using the system installed 
#  package is not defined
if((NOT libxml2_USE_INSTALLED) AND 
	(NOT TARGET LibXml2::LibXml2) )
	
	set(_EPA_NAME "libxml2")

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
	get_filename_component(_dl_filename "${libxml2_url}" NAME)
	set( _DL_METHOD "URL           ${libxml2_url}" )

	if(libxml2_hash)
		list(APPEND _DL_METHOD 
					"URL_HASH      ${libxml2_hash}")
	endif()

	list(APPEND _DL_METHOD 
						"DOWNLOAD_DIR  ${libxml2_download}")
	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	set(_EPA_SOURCE_DIR "SOURCE_DIR ${libxml2_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${libxml2_install}")

	if (UNIX)
		set(_libdir ${CMAKE_INSTALL_LIBDIR})
	else()
		set(_libdir "lib")
	endif()
	set(libxml_libdir "${libxml2_install}/${_libdir}")

	# LIBXML2 cmake build ignores this flag on UNIX type builds
	# check for any defined flags
	if(libxml2_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()
	
	# LibXML2 sets with_python to OFF by default, and LZMA support has been removed.

	# check for any defined flags
	if(libxml2_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${libxml2_CONFIG_OPTIONS}")
	else()
		set(libxml2_CONFIG_OPTIONS "${_config_options}" "-DLIBXML2_WITH_ZLIB=OFF")
	endif()

	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DLIBXML2_WITH_ICONV=OFF"
									"-DCMAKE_DEBUG_POSTFIX:STRING=d")
	else()
		list(APPEND _config_options "-DLIBXML2_WITH_ICONV=OFF")

		# Since our internal build types are Debug and Optimized, only Debug will translate
		if (CMAKE_BUILD_TYPE STREQUAL "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
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
						"-DINSTALL_LIB_DIR:STRING=<INSTALL_DIR>/${_libdir}"
						"${_config_options}"
						"${_shared_arg}"
						)

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND can be
	# used and a seperate config step does not need to be defined here.
	#  adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	#  list(APPEND _EPA_CONFIGURE_COMMAND  "CONFIGURE_COMMAND  \"\"" )

	# Typically, the build and install steps can be combined.  However, having them as 
	#  two seperate steps help in the event of having to debug a build
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

	# Add log defiitions if selected to be enabled and append them to the
	# additional args variable
	if(libxml2_LOG_BUILD)
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
	
	# this policy is needed to suppress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT LIBXML2_ROOT)
		set(LIBXML2_ROOT "${libxml2_install}")
	endif()
	
	if (EXISTS "${libxml_libdir}/${LIBXML2_STATIC_NAME}")
		set(LIBXML2_LIBRARY "${libxml_libdir}/${LIBXML2_STATIC_NAME}" CACHE FILEPATH "")
	endif()
	
		rcdev_config_build_ep(${_EPA_NAME})
	
	# this policy is needed to suppress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT LIBXML2_ROOT)
		set(LIBXML2_ROOT "${libxml2_install}" CACHE PATH "")
	endif()
	
	if(WIN32)
		set(LIBXML2_STATIC_NAME "libxml2${CMAKE_STATIC_LIBRARY_SUFFIX}")
	else()
		set(LIBXML2_STATIC_NAME "libxml2${CMAKE_STATIC_LIBRARY_SUFFIX}")
	endif()

	if (EXISTS "${libxml_libdir}/${LIBXML2_STATIC_NAME}")
		set(LIBXML2_LIBRARY "${libxml_libdir}/${LIBXML2_STATIC_NAME}" CACHE FILEPATH "")
	endif()

	# These two assignments will ensure the call to find package will locate the package
	# and define the target
	set(LIBXML2_INCLUDE_DIR "${libxml2_install}/include/libxml2" CACHE PATH "")


	unset(_libdir)
	unset(_shared_arg)
	unset(_cflags)
	unset(_log_args)
	unset(_dl_filename)
	unset(libxml_libdir)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()



endif()


# Find the package, for both a system installed version or the one
# just added with the ecternal project template
if ((NOT LibXml2_FOUND) OR
	(NOT TARGET LibXml2::LibXml2) )
	
	set(libxml2_find_options HINTS ${LIBXML2_ROOT})

	# Calling find_package with a required version number will fail if the
	# package does not have a <name>version.cmake in the same location as
	# the <package>config.cmake.  Unfortunately, CMake will not use the version
	# field defiition within a <package>.pc file. Also, the option to search for the
	# newly built version are passed as an argument, in case they have been defined, 
	# in lieu of an installed version
	find_package(LibXml2  REQUIRED ${libxml2_find_options})

	set(libxml2_USE_INSTALLED ON CACHE BOOL "")
	# This condition is here since the FindLibXl2 CMake module for version < Cmake.12.0 does not 
	#  crete an IMPORTED targte object (LibXml2::LibXml2)
	if (NOT TARGET LibXml2::LibXml2)
   		add_library(LibXml2::LibXml2 UNKNOWN IMPORTED)
   		set_target_properties(LibXml2::LibXml2 PROPERTIES 
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				INTERFACE_INCLUDE_DIRECTORIES "${LIBXML2_INCLUDE_DIRS}")
   		set_property(TARGET LibXml2::LibXml2 APPEND PROPERTY IMPORTED_LOCATION "${LIBXML2_LIBRARY}")
	endif()

	if (WIN32)
		set_property(TARGET LibXml2::LibXml2 APPEND PROPERTY 
											 INTERFACE_COMPILE_OPTIONS
											 	"/D LIBXML_STATIC"
							)
		# Will Map Release => Release_MD, Debug => Debug_Mdd
		#rcdev_map_imported_ep_types(LibXml2::LibXml2)

	else()
		set_property(TARGET LibXml2::LibXml2 APPEND PROPERTY 
											 INTERFACE_COMPILE_OPTIONS
											 	"-DHAVE_CONFIG_H"
											 	"-DLIBXML_THREAD_ENABLED"
											 	"-D_POSIX_PTHREAD_SEMANTICS"
							)
	endif()

	if(LIBXML2_VERSION_STRING VERSION_LESS "${libxml2_version}")
		message(WARNING
				"  LibXml2 ver:${LIBXML2_VERSION_STRING} found, is older than the supported ver:${libxml2_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	endif()

	rcdev_add_external_target(LibXml2::LibXml2)

endif()

DEBUG_PRINT(LibXml2_FOUND)
DEBUG_PRINT(LIBXML2_LIBRARIES)
DEBUG_PRINT(LIBXML2_LIBRARY)
DEBUG_PRINT(LIBXML2_INCLUDE_DIRS)
DEBUG_PRINT(LIBXML2_VERSION_STRING)
DEBUG_PRINT(LibXml2::LibXml2)
