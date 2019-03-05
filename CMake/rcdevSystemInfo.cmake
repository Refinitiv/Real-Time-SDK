#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
#]=============================================================================]

if (_rcdevSystemInfoInclude)
    return()
else()
	set(_rcdevSystemInfoInclude TRUE)
endif()

macro(rcdev_set_classic_postprefix_path)
	# Classic path post suffix
	#    x86_64_OracleLinux_6X.64/gnu444-g
	#               or
	#    x86_WindowsNT_5X.x64\ms110-Release-MD
	#
	set(_h_os_v "${RCDEV_HOST_SYSTEM_PROCESSOR}_${RCDEV_HOST_SYSTEM_FLAVOR}")
	set(_h_rel  "_${RCDEV_HOST_SYSTEM_FLAVOR_REL}")
	if (RCDEV_HOST_SYSTEM_BITS STREQUAL "64")
		set(_h_rel  "${_h_rel}X.${RCDEV_HOST_SYSTEM_BITS}")
	endif()
	set(_h_cmp_typ "${RCDEV_HOST_COMPILER_L}${RCDEV_HOST_COMPILER_VER}")

	if (UNIX)

		# This path is for UNIX Builds
		set (RCDEV_POST_PREFIX_CLASSIC_PATH
			"${_h_os_v}${_h_rel}/${_h_cmp_typ}${RCDEV_HOST_BUILD_TYPE_STR_${CMAKE_BUILD_TYPE}}"
			CACHE STRING "The relative path which offsets binaries " FORCE)
		mark_as_advanced(RCDEV_POST_PREFIX_CLASSIC_PATH)
		DEBUG_PRINT(::OUTPATH 
				"Post Prefix CLASSIC Path:"
				"\tRCDEV_POST_PREFIX_CLASSIC_PATH : ${RCDEV_POST_PREFIX_CLASSIC_PATH} ")
	else()
		# Loop through the possible build types if they are defined.  This
		# is neccessary, not just "cool", in case a new generator type is
		# defined in the future
		if (CMAKE_CONFIGURATION_TYPES)
			foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
				string(TOUPPER ${_type} _type_U)
				set (RCDEV_POST_PREFIX_CLASSIC_PATH_${_type_U}
							"${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${_type}"
							CACHE STRING "The relative path which offsets release binaries " FORCE)
				mark_as_advanced(RCDEV_POST_PREFIX_CLASSIC_PATH_${_type_U})
			endforeach()
			unset(_type)
			unset(_type_U)            # This path is for WIN32 Builds
		else()
			set (RCDEV_POST_PREFIX_CLASSIC_PATH_RELEASE_MD
						"${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${RCDEV_HOST_BUILD_RELEASE_MD}"
						CACHE STRING "The relative path which offsets release binaries " FORCE)
			mark_as_advanced(RCDEV_POST_PREFIX_CLASSIC_PATH_RELEASE_MD)

			# This path is for WIN32 Builds
			set (RCDEV_POST_PREFIX_CLASSIC_PATH_DEBUG_MDD
							"${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${RCDEV_HOST_BUILD_DEBUG_MDD}"
							CACHE STRING "The relative path which offsets debug binaries " FORCE)
			mark_as_advanced(RCDEV_POST_PREFIX_CLASSIC_PATH_DEBUG_MDD)
		endif()
		DEBUG_PRINT(::OUTPATH 
				"Post Prefix CLASSIC Paths:"
				"\tRCDEV_POST_PREFIX_CLASSIC_PATH_RELEASE_MD : ${RCDEV_POST_PREFIX_CLASSIC_PATH_RELEASE_MD}"
				"\tRCDEV_POST_PREFIX_CLASSIC_PATH_DEBUG_MDD  : ${RCDEV_POST_PREFIX_CLASSIC_PATH_DEBUG_MDD}"
				)
	endif()
	unset(_h_os_v)
	unset(_h_rel)
	unset(_h_cmp_typ)
endmacro()

macro(rcdev_set_new_postprefix_path)
	# Newer style path post suffix
	#    OL6_64_GNU444/Debug
	#               or
	#    WIN_64_VS140\Release_MD
	#
	set(_h_os_v ${RCDEV_HOST_SYSTEM_NAME_ABREV})
	set(_h_rel  "_${RCDEV_HOST_SYSTEM_BITS}_")
	set(_h_cmp_typ ${RCDEV_HOST_COMPILER}${RCDEV_HOST_COMPILER_VER})

	if (UNIX)
		set(_h_os_v ${_h_os_v}${RCDEV_HOST_SYSTEM_FLAVOR_REL})
		set(_sys_suffix ${_h_os_v}${_h_rel}${_h_cmp_typ})
		# Not a great way to handle this, but legacy static path
		# definitions cause work arounds like this
		# Check to see if need to force GCC instead of system defined GNU
		if (${RCDEV_HOST_COMPILER} STREQUAL "GNU" 
			AND (  ${PROJECT_NAME} STREQUAL "esdk"
				OR ${PROJECT_NAME} MATCHES  "^Elektron" 
				OR ${PROJECT_NAME} STREQUAL "Elektron-SDK-BinaryPack")
			)
			rcdev_get_platform_suffix(_sys_suffix gcc)
		endif()
		# This path is for UNIX Builds
		set (RCDEV_POST_PREFIX_NEW_PATH "${_sys_suffix}/${CMAKE_BUILD_TYPE}"
						CACHE STRING "The relative path which offsets binaries " FORCE)
		mark_as_advanced(RCDEV_POST_PREFIX_NEW_PATH)
		DEBUG_PRINT(::OUTPATH 
				"Post Prefix NEW Path:"
				"\tRCDEV_POST_PREFIX_NEW_PATH : ${RCDEV_POST_PREFIX_NEW_PATH}"
				)
	else()
		set(_sys_suffix ${_h_os_v}${_h_rel}${_h_cmp_typ})

		# Loop through the possible build types if they are defined.  This
		# is neccessary, not just "cool", in case a new generator type is
		# defined in the future
		if (CMAKE_CONFIGURATION_TYPES)
			foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
				string(TOUPPER ${_type} _type_U)
				set (RCDEV_POST_PREFIX_NEW_PATH_${_type_U} "${_sys_suffix}/${_type}"
							CACHE STRING "The relative path which offsets release binaries " FORCE)
				mark_as_advanced(RCDEV_POST_PREFIX_NEW_PATH_${_type_U})
			endforeach()
			unset(_type)
			unset(_type_U)
		else()
			# for some reason the config types are not defined so just define the
			# previous known standard config types
			# This path is for WIN32 Builds
			set (RCDEV_POST_PREFIX_NEW_PATH_RELEASE_MD
							"${_sys_suffix}/${RCDEV_HOST_BUILD_RELEASE_MD}"
							CACHE STRING "The relative path which offsets release binaries " FORCE)
			mark_as_advanced(RCDEV_POST_PREFIX_NEW_PATH_RELEASE_MD)

			# This path is for WIN32 Builds
			set (RCDEV_POST_PREFIX_NEW_PATH_DEBUG_MDD
							"${_sys_suffix}/${RCDEV_HOST_BUILD_DEBUG_MDD}"
							CACHE STRING "The relative path which offsets debug binaries " FORCE)
			mark_as_advanced(RCDEV_POST_PREFIX_NEW_PATH_DEBUG_MDD)
		endif()
		DEBUG_PRINT(::OUTPATH 
				"Post Prefix NEW Paths:"
				"\tRCDEV_POST_PREFIX_NEW_PATH_RELEASE_MD : ${RCDEV_POST_PREFIX_NEW_PATH_RELEASE_MD}"
				"\tRCDEV_POST_PREFIX_NEW_PATH_DEBUG_MDD  : ${RCDEV_POST_PREFIX_NEW_PATH_DEBUG_MDD}"
				)
	endif()

	unset(_h_os_v)
	unset(_h_rel)
	unset(_h_cmp_typ)
	unset(_sys_suffix)
endmacro()


function(get_lsb_release_info _relid _relnum)
	# For now, this will only support identifying Linux flavors via
	# lsb_release.  As new/older platforms arise which do not have lsb_release
	# a condition will be added to check its /etc/???-release for info.
	find_program(_lsb_binary lsb_release)
	if (_lsb_binary)
		set(_lsb_binary "${_lsb_binary}" CACHE INTERNAL "")
		execute_process(COMMAND ${_lsb_binary} -si 
						RESULT_VARIABLE _retval
						OUTPUT_VARIABLE _id
						OUTPUT_STRIP_TRAILING_WHITESPACE
						ERROR_STRIP_TRAILING_WHITESPACE)
		if (_retval EQUAL 0)
			set(${_relid} "${_id}" PARENT_SCOPE)
		endif()
		unset(_retval)
		execute_process(COMMAND ${_lsb_binary} -sr 
						RESULT_VARIABLE _retval
						OUTPUT_VARIABLE _num
						OUTPUT_STRIP_TRAILING_WHITESPACE
						ERROR_STRIP_TRAILING_WHITESPACE)
		if (_retval EQUAL 0)
			set(${_relnum} "${_num}" PARENT_SCOPE)
		endif()

		unset(_lsb_binary CACHE)
	else()
		#[========================================================[
		# if /etc/os-release then freedesktop.org and systemd
			if (EXISTS "/etc/os-release")
		# For some versions of Debian/Ubuntu without lsb_release command
			elseif(EXISTS "/etc/lsb-release")
		# Older Debian/Ubuntu/etc.
			elseif("/etc/debian_version")
		# Older SuSE/etc.
			elseif("/etc/SuSe-release")
		# Older Red Hat, CentOS, etc.
			elseif("/etc/redhat-release")
		# Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
			else()
		endif()
		#]========================================================]
		find_program(_uname_binary uname)
		if (_uname_binary)
			set(_uname_binary "${_uname_binary}" CACHE INTERNAL "")
			execute_process(COMMAND ${_uname_binary} -s 
				RESULT_VARIABLE _retval
				OUTPUT_VARIABLE _id
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_STRIP_TRAILING_WHITESPACE)
			if (_retval EQUAL 0)
				set(${_relid} "${_id}" PARENT_SCOPE)
			endif()
			
			execute_process(COMMAND ${_uname_binary} -r
				RESULT_VARIABLE _retval
				OUTPUT_VARIABLE _num
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_STRIP_TRAILING_WHITESPACE)
			if (_retval EQUAL 0)
				set(${_relnum} "${_num}" PARENT_SCOPE)
			endif()
			
			unset(_uname_binary CACHE)
		endif()
	endif()
	
	unset(_retval)
	unset(_id)
	unset(_num)

endfunction()

# Because of some legacy static path definitions, some paths
# have GCC and/or OL6 in place of GNU and/or RHEL6.  So in order to support
# old past haunts, hints will be added in case past wrongs
# are corrected
function(rcdev_get_normalized_platform_suffix suffix)
	set(_sfx)
	if (WIN32)
		rcdev_get_platform_suffix(_sfx)
	else()
		if ((RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "ORACLE" ) AND 
			(RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 6 ))
			rcdev_get_platform_suffix(_sfx "gcc" "rhel")
		elseif ( ((RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "CENTOS") OR
				 (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "REDHATLINUX")) AND
				 (RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 7 ) )
			rcdev_get_platform_suffix(_sfx "gcc" "centOS")
		else()
			rcdev_get_platform_suffix(_sfx "gcc" )
		endif()
	endif()
	set(${suffix} "${_sfx}" PARENT_SCOPE)		
endfunction()

function(rcdev_get_platform_suffix suffix)

	set(_abbrev ${RCDEV_HOST_SYSTEM_NAME_ABREV})
	set(_bits  ${RCDEV_HOST_SYSTEM_BITS})
	set(_compiler ${RCDEV_HOST_COMPILER}${RCDEV_HOST_COMPILER_VER})

	if (UNIX)
		set(_abbrev ${_abbrev}${RCDEV_HOST_SYSTEM_FLAVOR_REL})
		if (${ARGC} GREATER 1)
			set(_argList "${ARGN}")
			foreach(_arg IN LISTS _argList) 
				string (TOUPPER "${_arg}" _arg_U)
				if (${_arg_U} MATCHES "GCC")
					set(_compiler "GCC${RCDEV_HOST_COMPILER_VER}")
				endif()
				if (${_arg_U} MATCHES "RHEL")
					set(_abbrev  "RHEL${RCDEV_HOST_SYSTEM_FLAVOR_REL}")
				elseif (${_arg_U} MATCHES "CENTOS")
					set(_abbrev  "OL${RCDEV_HOST_SYSTEM_FLAVOR_REL}")
				endif()
			endforeach()
		endif()
	endif()
	set(${suffix} "${_abbrev}_${_bits}_${_compiler}" PARENT_SCOPE)		

	unset(_abbrev)
	unset(_bits)
	unset(_compiler)
	unset(_argList)
	unset(_arg)
	unset(_arg_U)
endfunction()

#[===================================================[
if (PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
    message (FATAL_ERROR 
    "
    In-place or in-source builds are not allowed for ${PROJECT_NAME}.
    Builds should mot be configured and built within the same
    directory. Only out-of-place or out-of-source builds allowed.
    
    Out-of-Place Build: 
    Source root directory = ${PROJECT_NAME}/
        cd ${PROJECT_NAME}/
        mkdir prod-cmake (or build, ...)
        cd prod-build
        cmake ../  
        make
    
    Out-of-Source Build: 
    Source root directory = ${PROJECT_NAME}/
        mkdir ${PROJECT_NAME}-build
        ls
               ${PROJECT_NAME}
               ${PROJECT_NAME}-build
    
        cd ${PROJECT_NAME}-build
        cd ${PROJECT_NAME}/
        mkdir prod-cmake (or build, ...)
        cd prod-build
        cmake ../${PROJECT_NAME}-build
        make
        ")
    
endif ()
#]===================================================]
####################################################################
#   Fields to populate in order to build the binary OUTPUT path
#
#   For Linux platforms some abbreviations have been base off expected
#   results via lsb_release
#   Redhat:
#       
#
#   RCDEV_HOST_SYSTEM_FLAVOR      = [OracleLinux | Linux | WindowsNT]
#   RCDEV_HOST_SYSTEM_FLAVOR_REL  = [0-9+] (i.e. 6X, 7X, ...)
#   RCDEV_HOST_SYSTEM_NAME        = [Linux | Windows]
#   RCDEV_HOST_SYSTEM_NAME_ABREV  = [OL | RHEL | WIN]
#   RCDEV_HOST_COMPILER           = [GNU | VS | ...] *GCC is a legacy static value :(
#   RCDEV_HOST_COMPILER_U         = Compiler ID in Uppercase
#   RCDEV_HOST_COMPILER_L         = Compiler ID in Lowercase
#   RCDEV_HOST_COMPILER_VER       = [0-9][0-9][0-9]
#   RCDEV_HOST_SYSTEM_PROCESSOR   = [x86 | x86_64]
#   RCDEV_HOST_SYSTEM_BITS        = [32 | 64]
#   RCDEV_HOST_BUILD_TYPE_STR_[Optimized|Debug|OptimizedDebug] = [ -[o|g|og] | Debug | Optimized ]
#   RCDEV_HOST_BUILD_[DEBUG_MDD | RELEASE_MD | ... ]   =  [Debug_MDd |  Release_MD | ...]

set(RCDEV_HOST_SYSTEM_FLAVOR )
set(RCDEV_HOST_SYSTEM_FLAVOR_U )
set(RCDEV_HOST_SYSTEM_FLAVOR_REL 0)
set(RCDEV_HOST_SYSTEM_NAME CMAKE_HOST_SYSTEM_NAME)
set(RCDEV_HOST_SYSTEM_NAME_ABREV )
set(RCDEV_HOST_COMPILER )
set(RCDEV_HOST_COMPILER_U )
set(RCDEV_HOST_COMPILER_L )
set(RCDEV_HOST_COMPILER_VER )
set(RCDEV_HOST_SYSTEM_PROCESSOR )
set(RCDEV_HOST_SYSTEM_BITS )

if(CYGWIN)
	message(FATAL_ERROR
			"This build framework does not support a Cygwin version of CMake."
			"Please make sure you are using a Windows installed version oc CMake"
			)
endif()

# RCDEV_HOST_SYSTEM_PROCESSOR should be set by CMAKE_HOST_SYSTEM_PROCESSOR.
# However, when this value is set for # an AMD(AMD64) processor, the x86 
# prefix is still used ..... ???? Dont ask me, this should be investigated
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT BUILD_32_BIT_ETA )
    set(RCDEV_HOST_SYSTEM_BITS  "64")
    set(RCDEV_HOST_SYSTEM_PROCESSOR "x86_64")
else()
    set(RCDEV_HOST_SYSTEM_BITS  "32")
    set(RCDEV_HOST_SYSTEM_PROCESSOR "x86")
endif()

unset(_compilerVer  )

# CMAKE_HOST_UNIX = TRUE if Unix like system
if (UNIX)

    set(_relid )
    set(_relnum)

    get_lsb_release_info(_relid _relnum)

    string(TOUPPER ${_relid} _relid_U)
    if (${_relid_U} MATCHES "ORACLE")
        set(RCDEV_HOST_SYSTEM_FLAVOR "OracleLinux")
        set(RCDEV_HOST_SYSTEM_NAME_ABREV "OL")
    elseif (${_relid_U} MATCHES "REDHAT")
        set(RCDEV_HOST_SYSTEM_FLAVOR "RedHatLinux")
        set(RCDEV_HOST_SYSTEM_NAME_ABREV "RHEL")
    elseif (${_relid_U} MATCHES "CENTOS")
        set(RCDEV_HOST_SYSTEM_FLAVOR "CentOSLinux")
        set(RCDEV_HOST_SYSTEM_NAME_ABREV "CENTOS")
    elseif (${_relid_U} MATCHES "SUSE")
        set(RCDEV_HOST_SYSTEM_FLAVOR "SUSELinux")
        set(RCDEV_HOST_SYSTEM_NAME_ABREV "SUSE")
    else()
        set(RCDEV_HOST_SYSTEM_FLAVOR "${_relid}")
        string(SUBSTRING "${_relid_U}" 0 3 RCDEV_HOST_SYSTEM_NAME_ABREV)
    endif()

    string(TOUPPER "${RCDEV_HOST_SYSTEM_FLAVOR}" RCDEV_HOST_SYSTEM_FLAVOR_U)
    string(REGEX MATCH "^([0-9]+)[.]*" _matchout ${_relnum})
    set(RCDEV_HOST_SYSTEM_FLAVOR_REL ${CMAKE_MATCH_1})

    DEBUG_PRINT(RCDEV_HOST_SYSTEM_FLAVOR)
    DEBUG_PRINT(RCDEV_HOST_SYSTEM_FLAVOR_U)
    DEBUG_PRINT(RCDEV_HOST_SYSTEM_NAME_ABREV)
    DEBUG_PRINT(RCDEV_HOST_SYSTEM_FLAVOR_REL)

	unset(_retval)
	unset(_relnum)
	unset(_relid)
	unset(_relid_U)
	unset(_oracle_rel_file)
	unset(_redhat_rel_file)

	set(RCDEV_HOST_BUILD_TYPE_STR_Optimized "-o-tp")
	set(RCDEV_HOST_BUILD_TYPE_STR_Debug "-g-tp")
	set(RCDEV_HOST_BUILD_TYPE_STR_OptimizedDebug "-og-tp")

	DEBUG_PRINT(CMAKE_HOST_SYSTEM_PROCESSOR)
    string(REGEX MATCH
    "([0-9]+).([0-9]+).([0-9]+)-([0-9]+).([A-Za-z]+)([0-9]+).(${CMAKE_HOST_SYSTEM_PROCESSOR})"
    _matchout ${CMAKE_HOST_SYSTEM_VERSION})
    # Need better id for compiler type and version
    # For now this is just a patch to build the OUTPUT path
    # Also, the version is hard coded and should consider using
    # the actual version found in CMAKE_C_COMPILER_VERSION
    set(RCDEV_HOST_COMPILER ${CMAKE_C_COMPILER_ID})
    string(TOUPPER ${RCDEV_HOST_COMPILER} RCDEV_HOST_COMPILER_U)
    set(RCDEV_HOST_COMPILER_U ${RCDEV_HOST_COMPILER_U})

    string(TOLOWER ${RCDEV_HOST_COMPILER} RCDEV_HOST_COMPILER_L)
    set(RCDEV_HOST_COMPILER_L ${RCDEV_HOST_COMPILER_L})

	if (NOT (DEFINED RCDEV_NORMALIZE_COMPILER_VERERSION))
		set(RCDEV_NORMALIZE_COMPILER_VERERSION OFF CACHE INTERNAL "This will cause the compiler string value for the binary output locations to be limited to only what is supported by internal development")
	endif()

    # For Compiler versions, currently the output path is generalized
    # with two internal linux release versions.  This will be revisited
    # at a later date for better definition with opensource versus internal
    # legacy output paths
	unset(_compilerVer)
	if (RCDEV_NORMALIZE_COMPILER_VERERSION)
		if (   RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "ORACLE"
			OR RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "REDHAT"
			OR RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "CENTOS")
			if (RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 5)
				set(_compilerVer "412")
			elseif (RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 6)
				set(_compilerVer "444")
			elseif(RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 7)
				set(_compilerVer "482")
			endif()
		endif()
	endif()

    if (NOT DEFINED _compilerVer)
        # If there is a better way to identify other possible <LANGUAGE>_COMPILER_VERSION
        # in CMAKE, then this will be improved
        if (DEFINED CMAKE_CXX_COMPILER_VERSION)
            set(_comp "${CMAKE_CXX_COMPILER_VERSION}")
        else()
            set(_comp "${CMAKE_C_COMPILER_VERSION}")
        endif()

        string(REGEX MATCH "^([0-9]+).([0-9]+)[.]*([0-9]*).*" _matchout ${_comp})
        set(_compilerVer "${CMAKE_MATCH_1}${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
        unset(_comp)
    endif()

    set(RCDEV_HOST_COMPILER_VER ${_compilerVer})
    unset(_matchout)

elseif (CMAKE_HOST_WIN32)

    set(RCDEV_HOST_SYSTEM_FLAVOR "WindowsNT")
    set(RCDEV_HOST_SYSTEM_FLAVOR_REL 5)
    set(RCDEV_HOST_SYSTEM_NAME_ABREV "WIN")
    if (CMAKE_CONFIGURATION_TYPES)
        foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
            string(TOUPPER ${_type} _type_U)
            set(RCDEV_HOST_BUILD_${_type_U} ${_type})
        endforeach()
        unset(_type)
        unset(_type_U)
    else()
        # for some reason the config types are not defined so just define the
        # previous known standard config types
        set(RCDEV_HOST_BUILD_DEBUG_MDD "Debug_MDd")
        set(RCDEV_HOST_BUILD_RELEASE_MD "Release_MD")
    endif()
    set(RCDEV_HOST_COMPILER "VS")
    set(RCDEV_HOST_COMPILER_U ${RCDEV_HOST_COMPILER})
    string(TOLOWER ${RCDEV_HOST_COMPILER} RCDEV_HOST_COMPILER_L)
    set(RCDEV_HOST_COMPILER_L ${RCDEV_HOST_COMPILER_L})
    if (MSVC)
		DEBUG_PRINT(MSVC_TOOLSET_VERSION)
        if (MSVC_VERSION GREATER 1910 OR MSVC_VERSION EQUAL 1910)
            set(_compilerVer "150")
            set(_msvcVer "15")
            set(_msvcYear "2017")
        elseif (MSVC_VERSION EQUAL 1900)
            set(_compilerVer "140")
            set(_msvcVer "14")
            set(_msvcYear "2015")
        elseif (MSVC_VERSION EQUAL 1800)
            set(_compilerVer "120")
            set(_msvcVer "12")
            set(_msvcYear "2013")
        elseif (MSVC_VERSION EQUAL 1700)
            set(_compilerVer "110")
            set(_msvcVer "11")
            set(_msvcYear "2012")
        elseif (MSVC_VERSION EQUAL 1600)
            set(_compilerVer "100")
            set(_msvcVer "10")
            set(_msvcYear "2010")
        elseif (MSVC_VERSION EQUAL 1500)
            set(_compilerVer "90")
            set(_msvcVer "9")
            set(_msvcYear "2008")
        elseif (MSVC_VERSION EQUAL 1400)
            set(_compilerVer "80")
            set(_msvcVer "8")
            set(_msvcYear "2005")
        else ()
            set(_compilerVer "00")
        endif ()
    else ()
        if (MSVC14)
            set (_compilerVer "140")
        elseif (MSVC12)
            set (_compilerVer "120")
        elseif (MSVC11)
            set (_compilerVer "110")
        elseif (MSVC10)
            set (_compilerVer "100")
        elseif (MSVC90)
            set (_compilerVer "90")
        elseif (MSVC80)
            set (_compilerVer "80")
        else ()
            set (_compilerVer "00")
        endif ()
    endif ()

    if(RCDEV_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
		set(RCDEV_MSVC_ARCH "amd64")
	endif()

    set(RCDEV_HOST_COMPILER_VER ${_compilerVer})
    set(RCDEV_HOST_MSVC_VERSION ${_msvcVer})
    set(RCDEV_HOST_MSVC_YEAR ${_msvcYear})

DEBUG_PRINT(::OUTPATH 
			"RCDEV_HOST_SYSTEM_FLAVOR                  : ${RCDEV_HOST_SYSTEM_FLAVOR}"
			"RCDEV_HOST_SYSTEM_FLAVOR_REL              : ${RCDEV_HOST_SYSTEM_FLAVOR_REL}"
			"RCDEV_HOST_SYSTEM_NAME                    : ${RCDEV_HOST_SYSTEM_NAME}"
			"RCDEV_HOST_SYSTEM_NAME_ABREV              : ${RCDEV_HOST_SYSTEM_NAME_ABREV}"
			"RCDEV_HOST_COMPILER                       : ${RCDEV_HOST_COMPILER}"
			"RCDEV_HOST_COMPILER_U                     : ${RCDEV_HOST_COMPILER_U}"
			"RCDEV_HOST_COMPILER_L                     : ${RCDEV_HOST_COMPILER_L}"
			"RCDEV_HOST_COMPILER_VER                   : ${RCDEV_HOST_COMPILER_VER}"
			"RCDEV_HOST_MSVC_VERSION                   : ${RCDEV_HOST_MSVC_VERSION}"
			"RCDEV_HOST_MSVC_YEAR                      : ${RCDEV_HOST_MSVC_YEAR}"
			"RCDEV_HOST_SYSTEM_PROCESSOR               : ${RCDEV_HOST_SYSTEM_PROCESSOR}"
			"RCDEV_HOST_SYSTEM_BITS                    : ${RCDEV_HOST_SYSTEM_BITS}"
			"RCDEV_HOST_BUILD_TYPE_STR_Optimized       : ${RCDEV_HOST_BUILD_TYPE_STR_Optimized}"
			"RCDEV_HOST_BUILD_TYPE_STR_Debug           : ${RCDEV_HOST_BUILD_TYPE_STR_Debug}"
			"RCDEV_HOST_BUILD_TYPE_STR_OptimizedDebug  : ${RCDEV_HOST_BUILD_TYPE_STR_OptimizedDebug}"
			"RCDEV_HOST_BUILD_RELEASE_STR              : ${RCDEV_HOST_BUILD_RELEASE_STR}"
			"RCDEV_HOST_BUILD_RELEASE_MD               : ${RCDEV_HOST_BUILD_RELEASE_MD}"
			"RCDEV_HOST_BUILD_DEBUG_MDD                : ${RCDEV_HOST_BUILD_DEBUG_MDD}"
			"RCDEV_MSVC_ARCH                           : ${RCDEV_MSVC_ARCH}"
			)
endif()

unset(_compilerVer  )
unset(_msvcVer  )
unset(_msvcYear  )



