#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
#]=============================================================================]

################################################################################################
#[=============================================================================================[
WARNING:  The logic contained within this block is a hack.  This hack is neccessary to account 
for the inconsistent naming of zlib libraries from Linux to Windows with the ZLIB CMake build, 
as of version zlib-1.2.11.  The following "work-around" will provide the logic for a patch step 
which will write out a modified CMakeLists.txt file which will build zlib static and shared 
libraries the same naming convention expected by the CMake FindZLIB module.  This does not 
address any other opportunities within the stock zlib CMake build.
#]=============================================================================================]
function(_write_custom_zlib_entrypoint _file_name)

		file(WRITE ${_file_name}
"
cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zlib C)

set(VERSION \"1.2.11\")

option(ASM686 \"Enable building i686 assembly implementation\")
option(AMD64 \"Enable building amd64 assembly implementation\")

set(INSTALL_BIN_DIR \"\${CMAKE_INSTALL_PREFIX}/bin\" CACHE PATH \"Installation directory for executables\")
set(INSTALL_LIB_DIR \"\${CMAKE_INSTALL_PREFIX}/lib\" CACHE PATH \"Installation directory for libraries\")
set(INSTALL_INC_DIR \"\${CMAKE_INSTALL_PREFIX}/include\" CACHE PATH \"Installation directory for headers\")
set(INSTALL_MAN_DIR \"\${CMAKE_INSTALL_PREFIX}/share/man\" CACHE PATH \"Installation directory for manual pages\")
set(INSTALL_PKGCONFIG_DIR \"\${CMAKE_INSTALL_PREFIX}/share/pkgconfig\" CACHE PATH \"Installation directory for pkgconfig (.pc) files\")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
enable_testing()

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX \"d\")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(\${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    # If we're doing an out of source build and the user has a zconf.h
    # in their source tree...
    if(EXISTS \${CMAKE_CURRENT_SOURCE_DIR}/zconf.h)
        message(STATUS \"Renaming\")
        message(STATUS \"    \${CMAKE_CURRENT_SOURCE_DIR}/zconf.h\")
        message(STATUS \"to 'zconf.h.included' because this file is included with zlib\")
        message(STATUS \"but CMake generates it automatically in the build directory.\")
        file(RENAME \${CMAKE_CURRENT_SOURCE_DIR}/zconf.h \${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.included)
  endif()
endif()

set(ZLIB_PC \${CMAKE_CURRENT_BINARY_DIR}/zlib.pc)
configure_file( \${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc.cmakein
		\${ZLIB_PC} @ONLY)
configure_file(	\${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
		\${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(\${CMAKE_CURRENT_BINARY_DIR} \${CMAKE_SOURCE_DIR})


#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    \${CMAKE_CURRENT_BINARY_DIR}/zconf.h
    zlib.h
)
set(ZLIB_PRIVATE_HDRS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    trees.h
    zutil.h
)
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
    trees.c
    uncompr.c
    zutil.c
)

if(NOT MINGW)
    set(ZLIB_DLL_SRCS
        win32/zlib1.rc # If present will override custom build rule below.
    )
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS contrib/amd64/amd64-match.S)
    endif ()

	if(ZLIB_ASMS)
		add_definitions(-DASMV)
		set_source_files_properties(\${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
	endif()
endif()

if(MSVC)
    if(ASM686)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx86/inffas32.asm
			contrib/masmx86/match686.asm
		)
    elseif (AMD64)
		ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
			contrib/masmx64/gvmat64.asm
			contrib/masmx64/inffasx64.asm
		)
    endif()

	if(ZLIB_ASMS)
		add_definitions(-DASMV -DASMINF)
	endif()
endif()

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ \${CMAKE_CURRENT_SOURCE_DIR}/zlib.h _zlib_h_contents)
string(REGEX REPLACE \".*#define[ \\t]+ZLIB_VERSION[ \\t]+\\\"([-0-9A-Za-z.]+)\\\".*\"
    \"\\\\1\" ZLIB_FULL_VERSION "\${_zlib_h_contents}")

if(MINGW)
    # This gets us DLL resource information when compiling on MinGW.
    if(NOT CMAKE_RC_COMPILER)
        set(CMAKE_RC_COMPILER windres.exe)
    endif()

    add_custom_command(OUTPUT \${CMAKE_CURRENT_BINARY_DIR}/zlib1rc.obj
                       COMMAND \${CMAKE_RC_COMPILER}
                            -D GCC_WINDRES
                            -I \${CMAKE_CURRENT_SOURCE_DIR}
                            -I \${CMAKE_CURRENT_BINARY_DIR}
                            -o \${CMAKE_CURRENT_BINARY_DIR}/zlib1rc.obj
                            -i \${CMAKE_CURRENT_SOURCE_DIR}/win32/zlib1.rc)
    set(ZLIB_DLL_SRCS \${CMAKE_CURRENT_BINARY_DIR}/zlib1rc.obj)
endif(MINGW)

add_library(zlib SHARED \${ZLIB_SRCS} \${ZLIB_ASMS} \${ZLIB_DLL_SRCS} \${ZLIB_PUBLIC_HDRS} \${ZLIB_PRIVATE_HDRS})
add_library(zlibstatic STATIC \${ZLIB_SRCS} \${ZLIB_ASMS} \${ZLIB_PUBLIC_HDRS} \${ZLIB_PRIVATE_HDRS})
set_target_properties(zlib PROPERTIES DEFINE_SYMBOL ZLIB_DLL)
set_target_properties(zlib PROPERTIES SOVERSION 1)

if(NOT CYGWIN)
    # This property causes shared libraries on Linux to have the full version
    # encoded into their final filename.  We disable this on Cygwin because
    # it causes cygz-\${ZLIB_FULL_VERSION}.dll to be created when cygz.dll
    # seems to be the default.
    #
    # This has no effect with MSVC, on that platform the version info for
    # the DLL comes from the resource file win32/zlib1.rc
    set_target_properties(zlib PROPERTIES VERSION \${ZLIB_FULL_VERSION})
endif()

if(UNIX)
    # On unix-like platforms the library is almost always called libz
   set_target_properties(zlib zlibstatic PROPERTIES OUTPUT_NAME z)
   if(NOT APPLE)
     set_target_properties(zlib PROPERTIES LINK_FLAGS \"-Wl,--version-script,\\\"\${CMAKE_CURRENT_SOURCE_DIR}/zlib.map\\\"\")
   endif()
   #elseif(BUILD_SHARED_LIBS AND WIN32)
elseif(WIN32)
    # Creates zlib1.dll when building shared library version
	set_target_properties(zlib PROPERTIES OUTPUT_NAME zlib1)
	set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME zlib)
endif()

if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS zlib zlibstatic
        RUNTIME DESTINATION \"\${INSTALL_BIN_DIR}\"
        ARCHIVE DESTINATION \"\${INSTALL_LIB_DIR}\"
        LIBRARY DESTINATION \"\${INSTALL_LIB_DIR}\" )
endif()
if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    install(FILES \${ZLIB_PUBLIC_HDRS} DESTINATION \"\${INSTALL_INC_DIR}\")
endif()
if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(FILES zlib.3 DESTINATION \"\${INSTALL_MAN_DIR}/man3\")
endif()
if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(FILES \${ZLIB_PC} DESTINATION \"\${INSTALL_PKGCONFIG_DIR}\")
endif()

#============================================================================
# Example binaries
#============================================================================

if(BUILD_ZLIB_EXZMPLES)
		add_executable(example test/example.c)
		target_link_libraries(example zlib)
		add_test(example example)

		add_executable(minigzip test/minigzip.c)
		target_link_libraries(minigzip zlib)

		if(HAVE_OFF64_T)
			add_executable(example64 test/example.c)
			target_link_libraries(example64 zlib)
			set_target_properties(example64 PROPERTIES COMPILE_FLAGS \"-D_FILE_OFFSET_BITS=64\")
			add_test(example64 example64)

			add_executable(minigzip64 test/minigzip.c)
			target_link_libraries(minigzip64 zlib)
			set_target_properties(minigzip64 PROPERTIES COMPILE_FLAGS \"-D_FILE_OFFSET_BITS=64\")
		endif()
endif()
"
)

endfunction()
################################################################################################
#[=============================================================================================[
End of work around for WIN32 zlib library naming issue
#]=============================================================================================]

include(rcdevExternalUtils)

if(NOT zlib_url)
	set(zlib_url "https://www.zlib.net/zlib-1.2.11.tar.xz")
endif()
if(NOT zlib_hash)
	set(zlib_hash "SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066")
endif()
if(NOT zlib_version)
	set(zlib_version "1.2.11")
endif()
	
# If the option for using the system installed 
#  package is not defined
if( (NOT zlib_USE_INSTALLED) AND 
	(NOT ZLIB_FOUND) )
	# An external project for zlib
	set(_EPA_NAME "zlib")

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
	get_filename_component(_dl_filename "${zlib_url}" NAME)
	set( _DL_METHOD "URL           ${zlib_url}" )

	if(zlib_hash)
		list(APPEND _DL_METHOD "URL_HASH      ${zlib_hash}")
	endif()

	list(APPEND _DL_METHOD "DOWNLOAD_DIR  ${zlib_download}")

	if (DEFINED _dl_filename)
		list(APPEND _DL_METHOD "DOWNLOAD_NAME ${_dl_filename}" )
	endif()

	# the top CMake entry point is not in the top source_dir location
	# so need to define 'SOURCE_SUBDIR'
	set(_EPA_SOURCE_DIR "SOURCE_DIR ${zlib_source}")
	# the BINARY_DIR is not seperate for this type of external project
	set(_EPA_INSTALL_DIR "INSTALL_DIR ${zlib_install}")

	
	# ZLIB cmake build ignores this flag on UNIX type builds
	# check for any defined flags
	if(zlib_BUILD_SHARED_LIBS)
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=ON")
	else()
		set(_shared_arg "-DBUILD_SHARED_LIBS:BOOL=OFF")
		set(_config_options "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON")
	endif()

	# check for any defined flags
	if(zlib_CONFIG_OPTIONS)
		set(_config_options "${_config_options}" "${zlib_CONFIG_OPTIONS}")
	else()
		set(zlib_CONFIG_OPTIONS "${_config_options}")
	endif()

	set(_libdir "lib")
	unset(_cfg_type)
	if (WIN32)
		list(APPEND _config_options "-DCMAKE_DEBUG_POSTFIX:STRING=d"
									"-DCMAKE_C_FLAGS:STRING=/DEBUG:NONE")
	else()
		# Since our internal build types are Debug and Optimized, only Debug will translate
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			set(_cfg_type "${CMAKE_BUILD_TYPE}")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
		else()
			set(_cfg_type "Release")
			list(APPEND _config_options "-DCMAKE_BUILD_TYPE:STRING=Release")
		endif()

		if (RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
			set(_libdir "lib64")
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

	# Hack for win32 to account for ZLIB inconsistent library naming
	if (WIN32)
			set(_patchfile "${zlib_projroot}/patch/CMakeLists.txt")
			_write_custom_zlib_entrypoint(${_patchfile})
			set( _EPA_PATCH_COMMAND 
					"PATCH_COMMAND    \"${CMAKE_COMMAND}\" -E copy ${zlib_source}/CMakeLists.txt ${zlib_source}/CMakeLists_orig.txt
						COMMAND       \"${CMAKE_COMMAND}\" -E copy ${_patchfile} ${zlib_source}
					")
			unset(_patchfile)
	endif()
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
	if(zlib_LOG_BUILD)
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
	
	# If ZLIB_LIBRARY is not set here, then only the shared version will be assigned to the
	#   ZLIB:: tatger library interface
	if(WIN32)
		set(ZLIB_STATIC_NAME "${zlib_install}/lib/zlib")
	else()
		set(ZLIB_STATIC_NAME "${zlib_install}/${_libdir}/libz")
	endif()

	set(ZLIB_LIBRARY "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")

	if (EXISTS "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(ZLIB_LIBRARY_RELEASE "${ZLIB_STATIC_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	if (EXISTS "${ZLIB_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
		set(ZLIB_LIBRARY_DEBUG "${ZLIB_STATIC_NAME}d${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE FILEPATH "")
	endif()

	# this policy is needed to supress a CMake warning about the new
	# standard for using <project>_ROOT variable for find_package()
	if( POLICY CMP0074 )
		#message("Setting CMake policy CMP0074 esdk/${_EPA_NAME}:[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ] ")
		cmake_policy(SET CMP0074 NEW)
	endif()

	if(NOT ZLIB_ROOT)
		set(ZLIB_ROOT "${zlib_install}" CACHE INTERNAL "")
	endif()

	unset(_libdir)
	unset(_shared_arg)
	unset(_config_options)
	unset(_log_args)
	unset(_dl_filename)

	# This call will reset all the _EPA_... variables. Because this is a
	# macro and if this is not called, the next external project using
	# this template will be at risk being currupted with old values.
	rcdev_reset_ep_add()

	# Since the Zlib CMake Find module supports setting ZLIB_ROOT
	# there is no need to set any find options here
	# set(zlib_find_options NODEFAULT_PATH HINTS "${zlib_install}")

endif()

if ((NOT ZLIB_FOUND) OR
		(NOT TARGET ZLIB::ZLIB) )
	
	# Calling find_package with a required version number will fail if the
	# package does not have a <name>version.cmake in the same location as
	# the <package>config.cmake.  Unfortunately, CMake will not use the version
	# field defiition within a <package>.pc file. Also, the option to search for the
	# newly built version are passed as an argument, in case they have been defined, 
	# in lieu of an installed version
	find_package(ZLIB REQUIRED "${zlib_find_options}")

	# Will Map Release => Release_MD, Debug => Debug_Mdd and
	# on UNIX Release => Optimized
	rcdev_map_imported_ep_types(ZLIB::ZLIB)

	if(ZLIB_VERSION_STRING VERSION_LESS "${zlib_version}")
		message(WARNING
				"  Zlib ver:${ZLIB_VERSION_STRING} found, is older than the supported ver:${zlib_version}\n"
				"  This may cause unexpected behavior and/or build results"
				)
	endif()

	rcdev_add_external_target(ZLIB::ZLIB)

endif()

DEBUG_PRINT(ZLIB_FOUND)
DEBUG_PRINT(ZLIB_LIBRARIES)
DEBUG_PRINT(ZLIB_INCLUDE_DIRS)
DEBUG_PRINT(ZLIB_VERSION_STRING)
DEBUG_PRINT(ZLIB::ZLIB)

