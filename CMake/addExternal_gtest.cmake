#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license 
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.   
 *|           Copyright (C) 2022, 2024, 2025 LSEG. All rights reserved.
#]=============================================================================]


include(rcdevExternalUtils)

set(gtest_version_cpp98 "1.8.1" CACHE STRING "")
set(gtest_version_cpp11 "1.12.1" CACHE STRING "")

if( UNIX AND (RCDEV_HOST_SYSTEM_FLAVOR_REL LESS_EQUAL 7) )
	if(NOT gtest_url)
		set(gtest_url "https://github.com/abseil/googletest/archive/release-${gtest_version_cpp98}.tar.gz")
	endif()
	if(NOT gtest_hash)
		set(gtest_hash "MD5=2e6fbeb6a91310a16efe181886c59596")
	endif()
	if(NOT gtest_version)
		set(gtest_version "${gtest_version_cpp98}")
	endif()
else()
	if(NOT gtest_url)
		set(gtest_url "https://github.com/abseil/googletest/archive/release-${gtest_version_cpp11}.tar.gz")
	endif()
	if(NOT gtest_hash)
		set(gtest_hash "MD5=e82199374acdfda3f425331028eb4e2a")
	endif()
	if(NOT gtest_version)
		set(gtest_version "${gtest_version_cpp11}")
	endif()
endif()

# If the option for using the system installed 
#  package is not defined
if( (NOT gtest_USE_INSTALLED) AND 
	(NOT TARGET GTest::GTest) )
	set(_EPA_NAME "gtest")

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
	get_filename_component(_dl_filename "${gtest_url}" NAME)
	set(_DL_METHOD "URL           ${gtest_url}" )

	if(gtest_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${gtest_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${gtest_download}")
	if (DEFINED _dl_filename)
		if(NOT (${_dl_filename} MATCHES "^gtest"))
			list(APPEND _DL_METHOD "DOWNLOAD_NAME  gtest-${_dl_filename}" )
		else()
			list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
		endif()
	endif()

	# check for any defined flags
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${gtest_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${gtest_install}")

	# check for any defined flags
	if(gtest_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
							"-Dgtest_force_shared_crt:BOOL=ON" )
	endif()

	# check for any defined flags
	if(gtest_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${gtest_CONFIG_OPTIONS}")
	else()
		set(_config_options  "${_config_options}" "-DBUILD_GMOCK:BOOL=OFF")
	endif()

	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d"
									"-DCMAKE_CXX_FLAGS:STRING=/Zc:wchar_t- /DEBUG:NONE"
									)
	else()
		list(APPEND _config_options "-DCMAKE_C_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}"
									"-DCMAKE_CXX_FLAGS:STRING=-m${RCDEV_HOST_SYSTEM_BITS}")
	endif()	

	# Append the config and shared args to the CMake arguments to the template variable
	set( _EPA_CMAKE_ARGS "CMAKE_ARGS"
							"-DCMAKE_INSTALL_PREFIX:STRING=<INSTALL_DIR>"
							"${_config_options}"
							"${_shared_arg}"
							)

	# Since this external project has a CMakeLists.txt, the default CONFIG_COMMAND will be
	# used and one sonce not need to be defined here.
	#  Adding CONFIGURE_COMMAND  "" would skip the default CMake configure step
	# Typically, the build and install steps can be combined.  However, having them as 
	#  two seperate steps help in the event of having to debug a build
	# Set the <.....>_COMMAND for the build and install template fields
	# However, for this external project it is works out better to combine these next two steps
	# within the INSTALL_COMMAND step.  So, this is skipping the BUILD_COMMAND by 
	# passing "" as the argument for the BUILD_COMMAND
	set(_EPA_BUILD_COMMAND "BUILD_COMMAND        \"\" ")

	# Passing the two supported build config types along to the INSTALL_COMMAND for Windows and for 
	# single build type platforms, like Linux, the current config typed is built and installed
	set(_EPA_INSTALL_COMMAND 
				"INSTALL_COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Release "
				"        COMMAND    \"${CMAKE_COMMAND}\"   --build .  --target install  --config Debug ")

	# If there isn't a binary directory defined then make sure
	# the option 'BUILD_IN_SOURCE' is enabled
	if (NOT DEFINED _EPA_BINARY_DIR)
		set( _EPA_ADDITIONAL_ARGS "BUILD_IN_SOURCE 1" )
	endif()

	# Add log defiitions if selected to be enabled and append them to the
	# additional args variable
	if(gtest_LOG_BUILD)
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

	# this policy is needed to supress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074  rtsdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT GTEST_ROOT)
		set(GTEST_ROOT "${gtest_install}" CACHE PATH "")
	endif()

	# In some configurations the gtestConfig cmake modules will return a value '/include'
	# for the INTERFACE_DIRECTORY. So setting GTEST_INCLUDE here in an attempt to avoid this
	if (EXISTS "${gtest_install}/include/gtest/gtest.h")
		set(GTEST_INCLUDE_DIR "${gtest_install}/include")
	endif()

	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

	# Since the Gtest CMake Find module supports setting GTEST_ROOT
	# there is no need to set any find options here
	#set(gtest_find_options HINTS "${gtest_install}")

endif()

# Find the package, for both a system installed version or the one
# just added with the ecternal project template
if(NOT TARGET GTest::GTest)
	# Calling find_package with a required version number will fail if the
	# package does not have a <name>version.cmake in the same location as
	# the <package>config.cmake.  Unfortunately, CMake will not use the version
	# field defiition within a <package>.pc file. Also, the option to search for the
	# newly built version are passed as an argument, in case they have been defined, 
	# in lieu of an installed version.
	# NOTE: As of GTest 1.9.0 and CMake 3.12.4, GTEST OR GOOGLETEST version is set
	#       by the packages find module. 
	find_package(GTest  REQUIRED ${gtest_find_options})

	if (WIN32)
		#[========================================================[
		  The FindGTest CMake module does not add the GTEST_LIBRARY_RELEASE to the 
		  IMPORTED_CONFIGURATIONS.  It only appends the DEBUG config type.  If this
		  id fixed in a future release of CMake, then this module should first query 
		  the tragets to see if the RELEASE build type has been added.  If not then
		  it will be added here using GTEST_LIBRARY and GTEST_MAIN_LIBRARY as the 
		  RELEASE build type.
		#]========================================================]

		get_property(_pval TARGET GTest::gtest PROPERTY IMPORTED_LOCATION_RELEASE SET)
		if (NOT _pval)
			if (EXISTS "${GTEST_LIBRARY}") 
				set(GTEST_LIBRARY_RELEASE "${GTEST_LIBRARY}" CACHE FILEPATH "")
			endif()
			set_target_properties(GTest::gtest PROPERTIES 
												IMPORTED_LOCATION_RELEASE ${GTEST_LIBRARY_RELEASE})
		endif()
		unset(_pval)

		get_property(_pval TARGET GTest::gtest_main PROPERTY IMPORTED_LOCATION_RELEASE SET)
		if (NOT _pval)
			if (EXISTS "${GTEST_MAIN_LIBRARY}") 
				set(GTEST_MAIN_LIBRARY_RELEASE "${GTEST_MAIN_LIBRARY}" CACHE FILEPATH "")
			endif()
			set_target_properties(GTest::gtest_main PROPERTIES 
												IMPORTED_LOCATION_RELEASE ${GTEST_MAIN_LIBRARY_RELEASE})
		endif()
		unset(_pval)

		get_property(_pval TARGET GTest::gtest PROPERTY IMPORTED_CONFIGURATIONS SET)
		if (_pval)
			get_target_property(_pval GTest::gtest IMPORTED_CONFIGURATIONS)
			if (NOT (("Release" IN_LIST _pval) OR ("RELEASE" IN_LIST _pval)) ) 
				set_property(TARGET GTest::gtest APPEND PROPERTY 
										 				IMPORTED_CONFIGURATIONS RELEASE)
			endif()
		endif()
		unset(_pval)

		get_property(_pval TARGET GTest::gtest_main PROPERTY IMPORTED_CONFIGURATIONS SET)
		if (_pval)
			get_target_property(_pval GTest::gtest_main IMPORTED_CONFIGURATIONS)
			if (NOT (("Release" IN_LIST _pval) OR ("RELEASE" IN_LIST _pval)) ) 
				set_property(TARGET GTest::gtest_main APPEND PROPERTY 
										 				IMPORTED_CONFIGURATIONS RELEASE)
			endif()
		endif()
		unset(_pval)

		# Will Map Release => Release_MD, Debug => Debug_Mdd

		rcdev_map_imported_ep_types(GTest::gtest)
		rcdev_map_imported_ep_types(GTest::gtest_main)
	endif()

	# If the gtest CMake find module starts to set a version number
	# it will be validated here
	#[==================================================[
	if(GTEST_VERSION_STRING VERSION_LESS "${gtest_version}")
		message(WARNING
				"  Googletest ver:${GTEST_VERSION_STRING} found, is older than the supported ver:${gtest_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	endif()
	#]==================================================]

if (CMAKE_VERSION VERSION_LESS 3.20)
	rcdev_add_external_target(GTest::GTest GTest::Main)
else()
	rcdev_add_external_target(GTest::gtest GTest::gtest_main)
endif()

endif()
DEBUG_PRINT(GTEST_LIBRARY)
DEBUG_PRINT(GTEST_INCLUDE_DIRS)
DEBUG_PRINT(GTEST_MAIN_LIBRARY)

DEBUG_PRINT(GTest::GTest)
DEBUG_PRINT(GTest::Main)


