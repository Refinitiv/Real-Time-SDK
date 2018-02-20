
macro(wrap_prebuilt_library TARGET_NAME )
    set(options)
    set(oneValueArgs LIBNAME LIBTYPE LANGUAGE PATHROOT)
    set(multiValueArgs LIBHEADERS LINKDEPENDS PATHS COMPILEOPTS)

    cmake_parse_arguments(_WPL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    DEBUG_PRINT(::MESSAGE "TARGET_NAME=${TARGET_NAME}")
    if ((_WPL_LIBNAME MATCHES "^(lib)(.*)$") AND (CMAKE_FIND_LIBRARY_PREFIXES MATCHES lib ))
        set(_origional_CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES}) 
        set(CMAKE_FIND_LIBRARY_PREFIXES "")
    endif()

    set(_plat_suffix)
    set(_plat_suffix2)

    if (${_WPL_LIBTYPE} STREQUAL SHARED)
        set(_shared_lib "TRUE")
    else()
        set(_shared_lib )
    endif()

    set(_origional_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES}) 
    if ( CMAKE_HOST_UNIX )
        # Because of some legacy static path definitions, some paths
        # have GCC and/or OL6 in place of GNU and/or RHEL6.  So in order to support
        # old past haunts, hints will be added in case past wrongs
        # are corrected
        get_platform_suffix(_plat_suffix)
        if (    RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "ORACLE" 
			AND RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 6 )
            get_platform_suffix(_plat_suffix2 "gcc" "rhel")
        elseif (    RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "CENTOS" 
				AND RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 7 )
            get_platform_suffix(_plat_suffix2 "gcc" "centOS")
        else()
            get_platform_suffix(_plat_suffix2 "gcc" )
        endif()

        if (_shared_lib)
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        endif()

        # For UNIX, when not crosscompiling, we only need to handle the current
        # build config type and not all possible config types
        set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${CMAKE_BUILD_TYPE}"
                   "${_WPL_PATHROOT}/${_plat_suffix2}/${CMAKE_BUILD_TYPE}"
                   )
        string(TOUPPER ${CMAKE_BUILD_TYPE} _type_U)
        DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hints}")
        unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
        find_library(${TARGET_NAME}_LIBRARY_${_type_U}
                        NAMES ${_WPL_LIBNAME}
                        PATHS ${_hints}
                        PATH_SUFFIXES   Shared
                        DOC "Library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                        NO_DEFAULT_PATH)
        set(_search_lib ${${TARGET_NAME}_LIBRARY_${_type_U}})
        DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
        mark_as_advanced(${TARGET_NAME}_LIBRARY_${_type_U})

    else() # if (WIN32)

        get_platform_suffix(_plat_suffix)
        foreach(_type IN LISTS CMAKE_CONFIGURATION_TYPES)
            string(TOUPPER ${_type} _type_U)
            #if (${_WPL_LIBTYPE} STREQUAL SHARED)
            if (_shared_lib)
                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}/Shared")
                DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hint}")
                unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_IMP_LIBRARY_${_type_U}
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Shared import library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_lib ${${TARGET_NAME}_IMP_LIBRARY_${_type_U}})
                DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
                mark_as_advanced(${TARGET_NAME}_IMP_LIBRARY_${_type_U})

                set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}/Shared")
                DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME}_DLL in : ${_hint}")
                unset(${TARGET_NAME}_DLL_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_DLL_LIBRARY_${_type_U}
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Shared DLL library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_dlllib ${${TARGET_NAME}_DLL_LIBRARY_${_type_U}})
                DEBUG_PRINT(::MESSAGE "Result:${_search_dlllib}\n")
                mark_as_advanced(${TARGET_NAME}_DLL_LIBRARY_${_type_U})

                set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

            else()
                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}")
                DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hint}")
                unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_LIBRARY_${_type_U} 
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Static library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_lib ${${TARGET_NAME}_LIBRARY_${_type_U}})
                DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
                mark_as_advanced(${TARGET_NAME}_LIBRARY_${_type_U})

            endif()
        endforeach()
    endif()

    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_origional_CMAKE_FIND_LIBRARY_SUFFIXES}) 
    if (_origional_CMAKE_FIND_LIBRARY_PREFIXES)
        set(CMAKE_FIND_LIBRARY_PREFIXES ${_origional_CMAKE_FIND_LIBRARY_PREFIXES})
    endif()

    if (_search_lib MATCHES NOTFOUND OR
        _search_dlllib MATCHES NOTFOUND )
        message("Prebuilt libraries not found: ${_search_lib},${_search_dlllib} in \n\t${_hints}")
    else()
        # TODO: add all targets with correct namespace
        #       i.e esdk::
        # set(TARGET_NAME "esdk::${TARGET_NAME}")
        add_library(${TARGET_NAME} ${_WPL_LIBTYPE} IMPORTED GLOBAL)
        set_target_properties(${TARGET_NAME} PROPERTIES
                                    INTERFACE_INCLUDE_DIRECTORIES
                                        "${_WPL_LIBHEADERS}"
                                    IMPORTED_LINK_INTERFACE_LANGUAGES
                                        ${_WPL_LANGUAGE}
                            )
		if(_WPL_COMPILEOPTS)
			set_target_properties(${TARGET_NAME} PROPERTIES
									INTERFACE_COMPILE_OPTIONS
										"${_WPL_COMPILEOPTS}"
                            )
		endif()
        if (_shared_lib)
            set_target_properties(${TARGET_NAME} PROPERTIES
                                        IMPORTED_NO_SONAME  TRUE
                                )
        endif()
        if (_WPL_LINKDEPENDS)
            set_target_properties(${TARGET_NAME} PROPERTIES
                                    INTERFACE_LINK_LIBRARIES "${_WPL_LINKDEPENDS}"
                                )
        endif()

        if (CMAKE_HOST_UNIX)
            string(TOUPPER ${CMAKE_BUILD_TYPE} _type_U)
            set_property(TARGET ${TARGET_NAME} 
                                APPEND PROPERTY 
                                    IMPORTED_CONFIGURATIONS ${CMAKE_BUILD_TYPE} )
            set_target_properties(${TARGET_NAME} 
                                    PROPERTIES
                                        IMPORTED_LOCATION_${_type_U} 
                                            "${${TARGET_NAME}_LIBRARY_${_type_U}}"
              )

        else() # if (WIN32)
            foreach(_type IN LISTS CMAKE_CONFIGURATION_TYPES)
                string(TOUPPER ${_type} _type_U)

                set_property(TARGET ${TARGET_NAME} 
                                    APPEND PROPERTY 
                                        IMPORTED_CONFIGURATIONS ${_type_U})

                if (_shared_lib)
                    set_target_properties(${TARGET_NAME} 
                                            PROPERTIES
                                                IMPORTED_LOCATION_${_type_U}
                                                    "${${TARGET_NAME}_DLL_LIBRARY_${_type_U}}"
                                                IMPORTED_IMPLIB_${_type_U} 
                                                    "${${TARGET_NAME}_IMP_LIBRARY_${_type_U}}"
                                        )
                else()
                    set_target_properties(${TARGET_NAME} 
                                          PROPERTIES
                                            IMPORTED_LOCATION_${_type_U}
                                              "${${TARGET_NAME}_LIBRARY_${_type_U}}"
                                        )
                endif()
            endforeach()
        endif()
    endif()

    unset(_plat_suffix)
    unset(_plat_suffix2)
    unset(_hints)
    unset(_shared_lib)
    unset(_search_lib)
    unset(_search_dlllib)
    unset(_type)
    unset(_type_U)
    unset(_origional_CMAKE_FIND_LIBRARY_SUFFIXES)
    unset(_origional_CMAKE_FIND_LIBRARY_PREFIXES)
endmacro()

