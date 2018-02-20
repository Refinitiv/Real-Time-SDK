if (_setProjectPathsInclude)
	if ($ENV{RCDEV_DEBUG_ENABLED})
        message(STATUS "setProjectPaths already included")
    endif()
    return()
endif()
set(_setProjectPathsInclude TRUE)
include(rcdevEnvironment)

if (NOT RCDEV_GLOBAL_POST_PREFIX_ISSET)

    if (RCDEV_USE_CLASSIC_OUTPUT)
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
            set (RCDEV_POST_PREFIX_PATH
                            "${_h_os_v}${_h_rel}/${_h_cmp_typ}${RCDEV_HOST_BUILD_TYPE}"
                            CACHE PATH "The relative path which offsets binaries ")
			mark_as_advanced(RCDEV_POST_PREFIX_PATH)
            DEBUG_PRINT(::MESSAGE " Classic Post Prefix Path:
                    RCDEV_POST_PREFIX_PATH   :   ${RCDEV_POST_PREFIX_PATH} ")
        else()
            # Loop through the possible build types if they are defined.  This
            # is neccessary, not just "cool", in case a new generator type is
            # defined in the future
            if (CMAKE_CONFIGURATION_TYPES)
                foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
                    string(TOUPPER ${_type} _type_U)
                    set (RCDEV_POST_PREFIX_PATH_${_type_U}
                                "${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${_type}"
                                CACHE PATH "The relative path which offsets release binaries ")
					mark_as_advanced(RCDEV_POST_PREFIX_PATH_${_type_U})
                endforeach()
                unset(_type)
                unset(_type_U)            # This path is for WIN32 Builds
            else()
                set (RCDEV_POST_PREFIX_PATH_RELEASE_MD
                            "${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${RCDEV_HOST_BUILD_RELEASE_MD}"
                            CACHE PATH "The relative path which offsets release binaries ")
				mark_as_advanced(RCDEV_POST_PREFIX_PATH_RELEASE_MD)

                # This path is for WIN32 Builds
                set (RCDEV_POST_PREFIX_PATH_DEBUG_MDD
                                "${_h_os_v}${_h_rel}/ms${RCDEV_HOST_COMPILER_VER}-${RCDEV_HOST_BUILD_DEBUG_MDD}"
                                CACHE PATH "The relative path which offsets debug binaries ")
				mark_as_advanced(RCDEV_POST_PREFIX_PATH_DEBUG_MDD)
            endif()
            DEBUG_PRINT(::MESSAGE "
            Classic Post Prefix Paths:
                    RCDEV_POST_PREFIX_PATH_RELEASE_MD   :   ${RCDEV_POST_PREFIX_PATH_RELEASE_MD}
                    RCDEV_POST_PREFIX_PATH_DEBUG_MDD    :   ${RCDEV_POST_PREFIX_PATH_DEBUG_MDD}
                    ")
        endif()
    else()
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
                get_platform_suffix(_sys_suffix gcc)
            endif()
            # This path is for UNIX Builds
            set (RCDEV_POST_PREFIX_PATH "${_sys_suffix}/${CMAKE_BUILD_TYPE}"
                            CACHE PATH "The relative path which offsets binaries ")
			mark_as_advanced(RCDEV_POST_PREFIX_PATH)
            DEBUG_PRINT(::MESSAGE "
            Post Prefix Path:
                    RCDEV_POST_PREFIX_PATH   :   ${RCDEV_POST_PREFIX_PATH}
                    ")
        else()
            set(_sys_suffix ${_h_os_v}${_h_rel}${_h_cmp_typ})

            # Loop through the possible build types if they are defined.  This
            # is neccessary, not just "cool", in case a new generator type is
            # defined in the future
            if (CMAKE_CONFIGURATION_TYPES)
                foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
                    string(TOUPPER ${_type} _type_U)
                    set (RCDEV_POST_PREFIX_PATH_${_type_U} "${_sys_suffix}/${_type}"
                                CACHE PATH "The relative path which offsets release binaries ")
					mark_as_advanced(RCDEV_POST_PREFIX_PATH_${_type_U})
                endforeach()
                unset(_type)
                unset(_type_U)
            else()
                # for some reason the config types are not defined so just define the
                # previous known standard config types
                # This path is for WIN32 Builds
                set (RCDEV_POST_PREFIX_PATH_RELEASE_MD
                                "${_sys_suffix}/${RCDEV_HOST_BUILD_RELEASE_MD}"
                                CACHE PATH "The relative path which offsets release binaries ")
				mark_as_advanced(RCDEV_POST_PREFIX_PATH_RELEASE_MD)

                # This path is for WIN32 Builds
                set (RCDEV_POST_PREFIX_PATH_DEBUG_MDD
                                "${_sys_suffix}/${RCDEV_HOST_BUILD_DEBUG_MDD}"
                                CACHE PATH "The relative path which offsets debug binaries ")
				mark_as_advanced(RCDEV_POST_PREFIX_PATH_DEBUG_MDD)
            endif()
            DEBUG_PRINT(::MESSAGE "
                Post Prefix Paths:
                    RCDEV_POST_PREFIX_PATH_RELEASE_MD   :   ${RCDEV_POST_PREFIX_PATH_RELEASE_MD}
                    RCDEV_POST_PREFIX_PATH_DEBUG_MDD    :   ${RCDEV_POST_PREFIX_PATH_DEBUG_MDD}
                    ")
        endif()
    endif()
    #set(RCDEV_GLOBAL_POST_PREFIX_ISSET TRUE)

	unset(_h_os_v)
	unset(_h_rel)
	unset(_h_cmp_typ)
	unset(_sys_suffix)
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

# Offer the user the choice of overriding the installation directories
mark_as_advanced( CMAKE_RUNTIME_OUTPUT_DIRECTORY
                  CMAKE_LIBRARY_OUTPUT_DIRECTORY
                  CMAKE_ARCHIVE_OUTPUT_DIRECTORY
                )

