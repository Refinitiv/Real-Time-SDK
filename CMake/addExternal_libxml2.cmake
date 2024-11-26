#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license 
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
#]=============================================================================]


include(rcdevExternalUtils)

macro(check_libxml2_installed _is_installed)

	unset(_inst)

	if(WIN32)
		set(LIBXML2_STATIC_NAME "libxml2_a${CMAKE_STATIC_LIBRARY_SUFFIX}")
	else()
		set(LIBXML2_STATIC_NAME "libxml2${CMAKE_STATIC_LIBRARY_SUFFIX}")
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

macro(write_libxml2_config)

	if (UNIX AND RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
		set(_cfg_bits "64")
	endif()

	set(_xml2_config_dir "${libxml2_install}/lib${_cfg_bits}/cmake/libxml2")
	string(REGEX MATCHALL "^([0-9]+)\.([0-9]+)\.([0-9]+)" _match "${libxml2_version}")


	if (NOT (EXISTS ${_xml2_config_dir}))
		file(MAKE_DIRECTORY  "${_xml2_config_dir}")
	endif()
	file(WRITE ${_xml2_config_dir}/libxml2-config.cmake "

if(DEFINED LIBXML2_ROOT)
	set(_rootdir \"\${LIBXML2_ROOT}\")
else()
	get_filename_component(_rootdir  \"\${CMAKE_CURRENT_LIST_DIR}/../../../\" ABSOLUTE)
endif()

set(LIBXML2_VERSION_MAJOR	${CMAKE_MATCH_1})
set(LIBXML2_VERSION_MINOR	${CMAKE_MATCH_2})
set(LIBXML2_VERSION_MICRO	${CMAKE_MATCH_3})
set(LIBXML2_VERSION_STRING	\"${libxml2_version}\")
set(LIBXML2_INSTALL_PREFIX	\${_rootdir})
set(LIBXML2_INCLUDE_DIRS	\${_rootdir}/include \${_rootdir}/include/libxml2)
set(LIBXML2_LIBRARY_DIR		\${_rootdir}/lib${_cfg_bits})
set(LIBXML2_LIBRARY_NAME	\"${LIBXML2_STATIC_NAME}\")
set(LIBXML2_LIBRARIES 		-L\${LIBXML2_LIBRARY_DIR})
set(LIBXML2_LIBRARY 		\"\${LIBXML2_LIBRARY_DIR}/\${LIBXML2_LIBRARY_NAME}\")

	")
	unset(_cfg_bits)
	unset(_xml2_config_dir)
endmacro()


if(NOT libxml2_url)
	set(libxml2_url "https://download.gnome.org/sources/libxml2/2.13/libxml2-2.13.5.tar.xz")
endif()
if(NOT libxml2_hash)
	set(libxml2_hash "MD5=0b919be8edff97ade9c946e1a83bdecd")
endif()
if(NOT libxml2_version)
	set(libxml2_version "2.13.5")
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

	# TODO: this should be changed to be defined by a global definition, but
	# for now a non-cmake default value which is a standard location will work
	unset(_bits)
	if (UNIX AND RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
		set(_bits "64")
	endif()
	set(libxml_libdir "${libxml2_install}/lib${_bits}")

	# Because this config/install is not a cmake build, it needs to be cleared, extracted,
	# configured and built multiple times for each build type, especially on WIN32.  So,
	# if this module has already ran once and for whatever reason LibXml2_FOUND was deleted
	# from cache, check the working install location for a previous successful build to 
	# aviod rebuilding if it is not neccessary
	# However, multiple build types are not compatible with the default CMake find_package
	# logic without adding additional logic to the package config file.  This functionality
	# is currently not within the scope of this realease
	check_libxml2_installed(_is_installed)

	if (NOT _is_installed)
		# Configure steps and options are different for win32
		if(WIN32)
			# after the first build type, do not reset all the
			# _EPA variables since there is one more build for the
			# second build type
			#set(_save_epa_vars TRUE)
	
			set(_install_prefix "${libxml2_install}")
			file(TO_NATIVE_PATH "${_install_prefix}" _install_prefix)
	
			# Check for any defined options
			if(libxml2_BUILD_SHARED_LIBS)
				set(_shared_arg "static=no")
			else()
				set(_shared_arg "static=yes")
			endif()
	
			# If the source anb build files exists, remove and extract a new copy
			# This will avoid ithe possibility inheriting any configurations from 
			# a previous build of a different type
			if (EXISTS "${libxml2_source}")
				message("\n\t\tRemoving ${libxml2_projroot} .....\n")
				file(REMOVE_RECURSE "${libxml2_projroot}")
			endif()
	
			message("\n\t\tConfiguring LibXmls ${_cfg_type} build\n")
	
			set(_bld_type "debug=no")
			set(_crt "MD")
			file(TO_NATIVE_PATH "${_libdir}" _libdir)
			set(_libdir "libdir=${_libdir}")
	
			# To avoid dealing with CMake string/list formatting for a Win32 command prompt, it
			# is easier to write the commands to a CMake script file
			# Write the script file for the configure step
			file(WRITE ${libxml2_build}/libxml2_config.cmake 
			"execute_process(
			COMMAND cscript configure.js compiler=msvc ${_shared_arg} cruntime=/${_crt} ")
			file(APPEND ${libxml2_build}/libxml2_config.cmake
						"prefix=${_install_prefix} iconv=no zlib=no ${_bld_type} ftp=no ${_libdir}
			WORKING_DIRECTORY \"${libxml2_source}/win32\" )")
	
			# Typically, the build and install steps can be combined.  However, having them as 
			#  two seperate steps help in the event of having to debug a build
			# Write the script file for the build step
			file(WRITE ${libxml2_build}/libxml2_build.cmake 
						"execute_process(
							COMMAND nmake /f Makefile.msvc 
							WORKING_DIRECTORY \"${libxml2_source}/win32\"
							)")
							#  TRY nmake /f Makefile.msvc libxmla
	
			# Write the script file for the install step
			file(WRITE ${libxml2_build}/libxml2_install.cmake
						"execute_process(
							COMMAND nmake /f Makefile.msvc install-libs
							WORKING_DIRECTORY \"${libxml2_source}/win32\")
					")
	
			# Set the <.....>_COMMAND for the configure, build and install template fields
			set(_EPA_CONFIGURE_COMMAND
								"CONFIGURE_COMMAND \"${CMAKE_COMMAND}\" -P ${libxml2_build}/libxml2_config.cmake")
	
			set(_EPA_BUILD_COMMAND
							"BUILD_COMMAND \"${CMAKE_COMMAND}\" -P ${libxml2_build}/libxml2_build.cmake")
	
			set(_EPA_INSTALL_COMMAND
							"INSTALL_COMMAND \"${CMAKE_COMMAND}\" -P ${libxml2_build}/libxml2_install.cmake")
	
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
	
			# For the first iteration, only reset a few key _EPA variables
			unset(_EPA_CONFIGURE_COMMAND)
			unset(_EPA_UPDATE_COMMAND)
			unset(_EPA_BUILD_COMMAND)
			unset(_EPA_INSTALL_COMMAND)
			unset(_EPA_TEST_COMMAND)
			unset(_EPA_ADDITIONAL_ARGS)
	
			unset(_bld_type)
			unset(_cfg_type)
			unset(_crt)
			unset(_libdir)
	
		else()
	
			# check for any defined flags
			if(libxml2_BUILD_SHARED_LIBS)
				set(_shared_arg "--enable-shared --disable-static")
			else()
				set(_shared_arg "--enable-static --with-pic --disable-shared")
			endif()

			# since this is not a cmake build the config is done with configure 
			# or autogen if configure is not present. The build/install are a
			# simple 'gmake' / 'gmale install'
			set( _EPA_CONFIGURE_COMMAND "CONFIGURE_COMMAND"
										   	"<SOURCE_DIR>/configure "
										   	"--prefix=<INSTALL_DIR> "
										   	"--without-python "
										   	"--without-lzma "
										   	"--without-zlib "
										   	"--libdir=${libxml_libdir}"
										   	"${_shared_arg} "
										   	"--with-threads "
										   	"CFLAGS=-m${RCDEV_HOST_SYSTEM_BITS} "
									)
			
			# set the make/gmake command for the build
			set( _EPA_BUILD_COMMAND "BUILD_COMMAND"
											"${CMAKE_MAKE_PROGRAM}"
							)
			# set the make/gmake command for the install
			set( _EPA_INSTALL_COMMAND "INSTALL_COMMAND"
											"${CMAKE_MAKE_PROGRAM} install"
							)
	
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
		endif()
	endif()


	# Since the CMake FindLibXml2.cmake does not yet support this new standard
	# of using <package>_ROOT, it will be commented out until it provides the support
	#[==============================================================================[
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT LIBXML2_ROOT)
		set(LIBXML2_ROOT "${libxml2_install}")
	endif()
	#]==============================================================================]

	if(WIN32)
		set(LIBXML2_STATIC_NAME "libxml2_a${CMAKE_STATIC_LIBRARY_SUFFIX}")
	else()
		set(LIBXML2_STATIC_NAME "libxml2${CMAKE_STATIC_LIBRARY_SUFFIX}")

	endif()

	if (EXISTS "${libxml_libdir}/${LIBXML2_STATIC_NAME}")
		set(LIBXML2_LIBRARY "${libxml_libdir}/${LIBXML2_STATIC_NAME}" CACHE FILEPATH "")
	endif()

	# These two assignments will ensure the call to find package will locate the package
	# and define the target
	set(LIBXML2_INCLUDE_DIR "${libxml2_install}/include/libxml2" CACHE PATH "")

	write_libxml2_config()

	unset(_bits)
	unset(_shared_arg)
	unset(_cflags)
	unset(_log_args)
	unset(_dl_filename)
	unset(libxml_libdir)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

	set(libxml2_find_options HINTS ${libxml2_install})

endif()


# Find the package, for both a system installed version or the one
# just added with the ecternal project template
if ((NOT LibXml2_FOUND) OR
	(NOT TARGET LibXml2::LibXml2) )

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
