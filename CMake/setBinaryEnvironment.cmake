if (_setBinaryEnvironmentInclude)
	if ($ENV{RCDEV_DEBUG_ENABLED})
        message(STATUS "setBinaryEnvironment already included")
    endif()
    return()
endif()
set(_setBinaryEnvironmentInclude TRUE)
include(rcdevEnvironment)

function(get_lsb_release_info _relid _relnum)

    # For now, this will only support identifying Linux flavors via
    # lsb_release.  As new/older platforms arise which do not have lsb_release
    # a condition will be added to check its /etc/???-release for info.
    find_program(_lsb_binary lsb_release)
    if (_lsb_binary)
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
		unset(_lsb_binary)
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
            execute_process(COMMAND ${_uname_binary} -s 
                            RESULT_VARIABLE _retval
                            OUTPUT_VARIABLE _id
                            OUTPUT_STRIP_TRAILING_WHITESPACE
                            ERROR_STRIP_TRAILING_WHITESPACE)
            if (_retval EQUAL 0)
                set(${_relid} "${_id}" PARENT_SCOPE)
            endif()
            unset(_retval)
            execute_process(COMMAND ${_uname_binary} -r
                            RESULT_VARIABLE _retval
                            OUTPUT_VARIABLE _num
                            OUTPUT_STRIP_TRAILING_WHITESPACE
                            ERROR_STRIP_TRAILING_WHITESPACE)
            if (_retval EQUAL 0)
                set(${_relnum} "${_num}" PARENT_SCOPE)
            endif()
        endif()
    endif()

unset(_retval)
unset(_id)
unset(_num)

endfunction()

function(get_platform_suffix suffix)

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

if (PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
    message ("")
    message ("In-place or in-source builds are not allowed for ${PROJECT_NAME}.")
    message ("Builds should mot be configured and built within the same")
    message ("directory. Only out-of-place or out-of-source builds allowed.")
    message ("")
    message ("Out-of-Place Build: ")
    message ("Source root directory = ${PROJECT_NAME}/")
    message ("    cd ${PROJECT_NAME}/")
    message ("    mkdir prod-cmake (or build, ...)")
    message ("    cd prod-build")
    message ("    cmake ../  ")
    message ("    make")
    message ("")
    message ("Out-of-Source Build: ")
    message ("Source root directory = ${PROJECT_NAME}/")
    message ("    mkdir ${PROJECT_NAME}-build")
    message ("    ls")
    message ("           ${PROJECT_NAME}")
    message ("           ${PROJECT_NAME}-build")
    message ("")
    message ("    cd ${PROJECT_NAME}-build")
    message ("    cd ${PROJECT_NAME}/")
    message ("    mkdir prod-cmake (or build, ...)")
    message ("    cd prod-build")
    message ("    cmake ../${PROJECT_NAME}-build")
    message ("    make")
    message (SEND_ERROR "")
endif ()
#####################################
###  This section should be an include checking if global values
###  are already set, if not the proceed
#####################################
# CMAKE_HOST_SYSTEM Linux-2.6.32-431.el6.x86_64 
#set(CMAKE_HOST_SYSTEM_NAME Linux )
#CMAKE_HOST_SYSTEM_VERSION 2.6.32-431.el6.x86_64
#CMAKE_HOST_SYSTEM_PROCESSOR x86_64 
#CMAKE_C_COMPILER_VERSION 4.4.7 
#${CMAKE_BUILD_TYPE} STREQUAL Debug )

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
#   RCDEV_HOST_BUILD_TYPE         = [ -g | -o | Debug | Optimized ]
#   RCDEV_HOST_BUILD_DEBUG_STR    = [ g | Debug 
#   RCDEV_HOST_BUILD_RELEASE_STR  = [ o | Optimized 
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
if (CMAKE_HOST_UNIX)

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
	unset(_oracle_rel_file)
	unset(_redhat_rel_file)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(RCDEV_HOST_BUILD_TYPE "-g")
        set(RCDEV_HOST_BUILD_DEBUG_STR ${RCDEV_HOST_BUILD_TYPE})
    else()
        set(RCDEV_HOST_BUILD_TYPE "-o")
        set(RCDEV_HOST_BUILD_RELEASE_STR ${RCDEV_HOST_BUILD_TYPE})
    endif()
DEBUG_PRINT(CMAKE_HOST_SYSTEM_PROCESSOR)
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

    # For Compiler versions, currently the output path is generalized
    # with two internal linux release versions.  This will be revisited
    # at a later date for better definition with opensource versus internal
    # legacy output paths
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
        unset(_matchout)
        unset(_comp)
    endif()
    set(RCDEV_HOST_COMPILER_VER ${_compilerVer})
elseif (CMAKE_HOST_WIN32)
    set(RCDEV_HOST_SYSTEM_FLAVOR "WindowsNT")
    set(RCDEV_HOST_SYSTEM_FLAVOR_REL 5)
    set(RCDEV_HOST_SYSTEM_NAME_ABREV "WIN")
    set(RCDEV_HOST_BUILD_TYPE )
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
        if (MSVC_VERSION GREATER 1910 OR MSVC_VERSION EQUAL 1910)
            set(_compilerVer "150")
        elseif (MSVC_VERSION EQUAL 1900)
            set(_compilerVer "140")
        elseif (MSVC_VERSION EQUAL 1800)
            set(_compilerVer "120")
        elseif (MSVC_VERSION EQUAL 1700)
            set(_compilerVer "110")
        elseif (MSVC_VERSION EQUAL 1600)
            set(_compilerVer "100")
        elseif (MSVC_VERSION EQUAL 1500)
            set(_compilerVer "90")
        elseif (MSVC_VERSION EQUAL 1400)
            set(_compilerVer "80")
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
    set(RCDEV_HOST_COMPILER_VER ${_compilerVer})

DEBUG_PRINT(::MESSAGE "
    RCDEV_HOST_SYSTEM_FLAVOR        : ${RCDEV_HOST_SYSTEM_FLAVOR}
    RCDEV_HOST_SYSTEM_FLAVOR_REL    : ${RCDEV_HOST_SYSTEM_FLAVOR_REL}
    RCDEV_HOST_SYSTEM_NAME          : ${RCDEV_HOST_SYSTEM_NAME}
    RCDEV_HOST_SYSTEM_NAME_ABREV    : ${RCDEV_HOST_SYSTEM_NAME_ABREV}
    RCDEV_HOST_COMPILER             : ${RCDEV_HOST_COMPILER}
    RCDEV_HOST_COMPILER_U           : ${RCDEV_HOST_COMPILER_U}
    RCDEV_HOST_COMPILER_L           : ${RCDEV_HOST_COMPILER_L}
    RCDEV_HOST_COMPILER_VER         : ${RCDEV_HOST_COMPILER_VER}
    RCDEV_HOST_SYSTEM_PROCESSOR     : ${RCDEV_HOST_SYSTEM_PROCESSOR}
    RCDEV_HOST_SYSTEM_BITS          : ${RCDEV_HOST_SYSTEM_BITS}
    RCDEV_HOST_BUILD_TYPE           : ${RCDEV_HOST_BUILD_TYPE}
    RCDEV_HOST_BUILD_DEBUG_STR      : ${RCDEV_HOST_BUILD_DEBUG_STR}
    RCDEV_HOST_BUILD_RELEASE_STR    : ${RCDEV_HOST_BUILD_RELEASE_STR}
    RCDEV_HOST_BUILD_RELEASE_MD     : ${RCDEV_HOST_BUILD_RELEASE_MD}
    RCDEV_HOST_BUILD_DEBUG_MDD      : ${RCDEV_HOST_BUILD_DEBUG_MDD}
")
endif()

unset(_compilerVer  )


