#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
#]=============================================================================]

if (_rcdevCommonUtilsInclude)
    return()
else()
	set(_rcdevCommonUtilsInclude TRUE)
endif()


function(__getDTNumberFields dtString wd mn dy yr hr min sec)
    #DEBUG_PRINT(dtString)
    string(REGEX MATCH "^([0-9]+),([0-9]+)-([0-9]+)-([0-9]+)T([0-9]+):([0-9]+):([0-9]+)" _matchout "${dtString}")
    set(${wd} ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(${mn} ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(${dy} ${CMAKE_MATCH_3} PARENT_SCOPE)
    set(${yr} ${CMAKE_MATCH_4} PARENT_SCOPE)
    set(${hr} ${CMAKE_MATCH_5} PARENT_SCOPE)
    set(${min} ${CMAKE_MATCH_6} PARENT_SCOPE)
    set(${sec} ${CMAKE_MATCH_7} PARENT_SCOPE)
endfunction()

function(__getDTAlphaNumFields dtString wd mn dy yr hr min sec)
    #DEBUG_PRINT(dtString)
    string(REGEX MATCH "^([A-Za-z]+) ([A-Za-z]+) ([0-9]+),([0-9]+)T([0-9]+):([0-9]+):([0-9]+)" _matchout "${dtString}")
    set(${wd} ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(${mn} ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(${dy} ${CMAKE_MATCH_3} PARENT_SCOPE)
    set(${yr} ${CMAKE_MATCH_4} PARENT_SCOPE)
    set(${hr} ${CMAKE_MATCH_5} PARENT_SCOPE)
    set(${min} ${CMAKE_MATCH_6} PARENT_SCOPE)
    set(${sec} ${CMAKE_MATCH_7} PARENT_SCOPE)
endfunction()

function(__getTSNumberString tsString)
    string(TIMESTAMP TS "%w,%m-%d-%YT%H:%M:%S") 
	set(${tsString} ${TS} PARENT_SCOPE)
endfunction()

function(__getTSNameString tsString)
    string(TIMESTAMP TS "%a %b %d,%YT%H:%M:%S") 
	set(${tsString} ${TS} PARENT_SCOPE)
endfunction()

function(__getMonthName mValue mVar)
    set(L_Months Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec)
    #DEBUG_PRINT(mValue)
    if (${mValue} STRGREATER "00" AND ${mValue} STRLESS "13")
        math(EXPR M_IDX "(${mValue}*1) - 1")
        list(GET L_Months ${M_IDX} Month_Name)
	    set(${mVar} ${Month_Name} PARENT_SCOPE)
    else()
        message(STATUS "__getMonthName: invalid arg value")
        #DEBUG_PRINT(mValue)
    endif()
endfunction()

function(__getWeekdayName dValue dVar)
    set(L_Days Sun Mon Tue Wed Thr Fri Sat)
    #DEBUG_PRINT(dValue)
    if ((${dValue} STRGREATER "0" OR ${dValue} STREQUAL "0") AND
        (${dValue} STRLESS "7"))
        math(EXPR D_IDX "${dValue}*1")
        list(GET L_Days ${D_IDX} Weekday_Name)
	    set(${dVar} ${Weekday_Name} PARENT_SCOPE)
    else()
        message(STATUS "__getWeekdayName: invalid arg value")
        #DEBUG_PRINT(dValue)
    endif()
endfunction()

function(Get_Build_Time bTIME)
	__getTSNumberString(FULL_TS)
	__getDTNumberFields(${FULL_TS} _wd _month _day _year _hour _min _sec)
	set(${bTIME} "${_hour}:${_min}:${_sec}" PARENT_SCOPE)
endfunction()

function(Get_Build_Date bDATE)
	__getTSNameString(FULL_TS)
	__getDTAlphaNumFields(${FULL_TS} _wd _month _day _year _hour _min _sec)
    #__getWeekdayName(${_wd} _wName)
    #__getMonthName(${_month} _mName)
	set(${bDATE} "${_wd} ${_month} ${_day}, ${_year}" PARENT_SCOPE)
endfunction()

function(Get_Timestamp_Fields wDAY MONTH DAY YEAR HOUR MINUTE SECOND)
	__getTSNameString(FULL_TS)
	__getDTAlphaNumFields(${FULL_TS} _wd _month _day _year _hour _min _sec)
    #___getWeekdayName(${_wd} _wName)
    #__getMonthName(${_month} _mName)
    set(${wDAY} ${_wd} PARENT_SCOPE)
    set(${MONTH} ${_month} PARENT_SCOPE)
    set(${DAY} ${_day} PARENT_SCOPE)
    set(${YEAR} ${_year} PARENT_SCOPE)
    set(${HOUR} ${_hour} PARENT_SCOPE)
    set(${MINUTE} ${_min} PARENT_SCOPE)
    set(${SECOND} ${_sec} PARENT_SCOPE)
endfunction()

function(Get_Build_Timestamp bTS)
	__getTSNameString(FULL_TS)
	__getDTAlphaNumFields(${FULL_TS} _wd _month _day _year _hour _min _sec)
    #__getWeekdayName(${_wd} _wName)
    #__getMonthName(${_month} _mName)
    set(${bTS} "${_wd} ${_month} ${_day}, ${_year} at ${_hour}: ${_min} ${_sec}" PARENT_SCOPE)
endfunction()

#[=============================================================[
##  _is_directory_populated input: _dir :path absolute path to a directory
##                          output: _populated :bool will be set to TRUE if 
##                                  condition is true, else set to FALSE
##                          optional args:
##                             CMAKE :flag will check directory if cmake entry point (CMakeLists.txt)
##                             GIT   :flag will check directory is a git repo
##
##  Given a path to a directory, the second argument will be set to true if
##  any files or directories are found.  WIll also set flag if either or both
##  optional flags are psssed in and found to be true
##
#]=============================================================]
function(rcdev_is_directory_populated _populated _dir)

	set(flags	CMAKE GIT)
	cmake_parse_arguments(_flag "${flags}" "" "" ${ARGN})

	if ((NOT _flag_CMAKE) AND (NOT _flag_GIT))
		set(_check_populated TRUE)
	endif()

	set(_is_pop FALSE)
	file(GLOB _fout "${_dir}/*")

	# If the directory is not empty, need to see if the
	# CMAKE and/OR GIT conditions should also be ckecked
	if(NOT ("${_fout}xx" STREQUAL "xx"))
		# if neither option was passed in,
		# set populated totrue and return
		if(_check_populated)
			set(_is_pop TRUE)
		else()
			# CMAKE option: if dir is a cmake entrypoint
			if(_flag_CMAKE AND EXISTS "${_dir}/CMakeLists.txt")
				set(_is_pop TRUE)
			endif()
			# GIT option: if dir is a git repository
			if(_flag_GIT AND EXISTS "${_dir}/.git")
				# Only should set the flag if the
				# CMAKE flag was not enabled
				# e.g. 'is a CMAKE build and GIT repo'
				if(NOT _flag_CMAKE)
					set(_is_pop TRUE)
				endif()
			endif()
		endif()
	endif()

	set(${_populated} "${_is_pop}" PARENT_SCOPE)
	unset(_is_pop)
	unset(_check_populated)

endfunction()


macro(rcdev_update_output_dirs)
	set(options SAVE RESTORE SET_AS_RUNTIME POST_PREFIX SET_ALL)
	set(oneValueArgs RUNTIME ARCHIVE LIBRARY PDB)
	set(multiValueArgs)
	cmake_parse_arguments(_OD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	list(APPEND _bin_types "RUNTIME" "ARCHIVE" "LIBRARY")
	if(WIN32)
		list(APPEND _bin_types "PDB")
	endif()

	# Check for request to restore XXX_OUTPUT_DIRECTORY from
	#  the _ORIG saved values
	if (_OD_RESTORE)
		foreach(_bt ${_bin_types})
			if(WIN32)
				if(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD)
					DEBUG_PRINT(::OUTPATH "Restoring backup of ${_bt}_OUTPUT_DIRECTORY :"
								"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
								"\t    =>  CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
								)
					set(CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD "${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}")
					unset(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD)
				endif()
				if(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD)
					DEBUG_PRINT(::OUTPATH "Restoring backup of ${_bt}_OUTPUT_DIRECTORY :"
								"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
								"\t    =>  CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
								)
					set(CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD "${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}")
					unset(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD)
				endif()
			else()
				if(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY)
					DEBUG_PRINT(::OUTPATH "Restoring backup of ${_bt}_OUTPUT_DIRECTORY :"
								"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY}"
								"\t    =>  CMAKE_${_bt}_OUTPUT_DIRECTORY : ${CMAKE_${_bt}_OUTPUT_DIRECTORY}"
								)
					set(CMAKE_${_bt}_OUTPUT_DIRECTORY "${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY}")
					unset(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY)
				endif()
			endif()
		endforeach()
	endif()
	# Check for request to save the current XXX_OUTPUT_DIRECTORY values
	#  to _ORIG_XXX_OUTPUT_DIRECTORY
	if (_OD_SAVE)
		foreach(_bt ${_bin_types})
			if(WIN32)
				set(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD "${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}")
				set(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD "${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}")
				DEBUG_PRINT(::OUTPATH "Saving backup of ${_bt}_OUTPUT_DIRECTORY to :"
							"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
							"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
							)
			else()
				set(_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY "${CMAKE_${_bt}_OUTPUT_DIRECTORY}")
				DEBUG_PRINT(::OUTPATH "Saving backup of ${_bt}_OUTPUT_DIRECTORY to :"
							"\t_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY : ${_ORIG_CMAKE_${_bt}_OUTPUT_DIRECTORY}"
							)
			endif()
		endforeach()
	endif()

	# Any _UNPARSED_ARGUMENTS are considerd to be directories intended to be
	#  appended to the end of all XXX_OUTPUT_DIRECTORY paths or the keywords passed in
	set(_path )
	foreach(_upa ${_OD_UNPARSED_ARGUMENTS})
		string(APPEND _path "${_upa}/")
	endforeach()
	unset(_upa)
	# If _UNPARSED_ARGUMENTS are passed in and no keywords for _OUTPUT_DIRECTORY exist,
	#  then the flag for set all is enabled
	if(_path AND
		(NOT _OD_RUNTIME AND 
		 NOT _OD_ARCHIVE  AND 
		 NOT _OD_LIBRARY AND 
		 NOT _OD_PDB)
		)
		 set(_OD_SET_ALL ON)
	endif()

	# If the POST_PREFIX flag is passed in and it is not currently within the
	# defined XXX_OUTPUT_DIRECTORY path, it will be appended
	if(_OD_POST_PREFIX)
		foreach(_bt ${_bin_types})
			if(WIN32)
				if(NOT (CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD MATCHES 
						"${RCDEV_POST_PREFIX_PATH_RELEASE_MD}"))
					string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD
								"${RCDEV_POST_PREFIX_PATH_RELEASE_MD}/")
					DEBUG_PRINT(::OUTPATH "Appending ${_bt}_OUTPUT_DIRECTORY with : ${RCDEV_POST_PREFIX_PATH_RELEASE_MD}"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
								)
				else()
					DEBUG_PRINT(::OUTPATH "NOT Appending ${_bt}_OUTPUT_DIRECTORY : POST_PREFIX is already set"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
								)
				endif()
				if(NOT (CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD MATCHES 
						"${RCDEV_POST_PREFIX_PATH_DEBUG_MDD}"))
					string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD
								"${RCDEV_POST_PREFIX_PATH_DEBUG_MDD}/")
					DEBUG_PRINT(::OUTPATH "Appending ${_bt}_OUTPUT_DIRECTORY with : ${RCDEV_POST_PREFIX_PATH_DEBUG_MDD}"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
								)
				else()
					DEBUG_PRINT(::OUTPATH "NOT Appending ${_bt}_OUTPUT_DIRECTORY : POST_PREFIX is already set"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
								)
				endif()
			else()
				if(NOT (CMAKE_${_bt}_OUTPUT_DIRECTORY MATCHES "${RCDEV_POST_PREFIX_PATH}"))

					string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY "${RCDEV_POST_PREFIX_PATH}/")
					DEBUG_PRINT(::OUTPATH "Appending ${_bt}_OUTPUT_DIRECTORY with : ${RCDEV_POST_PREFIX_PATH}"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY : ${CMAKE_${_bt}_OUTPUT_DIRECTORY}"
								)
				else()
					DEBUG_PRINT(::OUTPATH "NOT Appending ${_bt}_OUTPUT_DIRECTORY : POST_PREFIX is already set"
								"\tCMAKE_${_bt}_OUTPUT_DIRECTORY : ${CMAKE_${_bt}_OUTPUT_DIRECTORY}"
								)
				endif()
			endif()
		endforeach()
	endif()

	# For all XXX_OUTPUT_DIRECTORY paths, they will have their corresponding argument appended
	#  or the current _UNPARSED_ARGUMENT appended if no keywords were passed in
	foreach(_bt ${_bin_types})
		if (_OD_${_bt} OR
			_OD_SET_ALL )
			if(NOT _OD_SET_ALL)
				set(_path "${_OD_${_bt}}/")
			endif()
			if(WIN32)
				string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD "${_path}")
				string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD "${_path}")
				DEBUG_PRINT(::OUTPATH "Appending _OUTPUT_DIRECTORY with : ${_path}"
							"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
							"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}"
							)
			else()
				string(APPEND CMAKE_${_bt}_OUTPUT_DIRECTORY "${_path}")
				DEBUG_PRINT(::OUTPATH "Appending _OUTPUT_DIRECTORY with : ${_path}"
							"\tCMAKE_${_bt}_OUTPUT_DIRECTORY : ${CMAKE_${_bt}_OUTPUT_DIRECTORY}"
							)
			endif()
		endif()
	endforeach()

	# Specifically for WIN32, the keyword SET_AS_RUNTIME will assign all other
	#  XXX_OUTPUT_DIRECTORY paths to equal the RUNTIME_OUTPUT_DIRECTORY path
	if(WIN32 AND _OD_SET_AS_RUNTIME)
		foreach(_bt ${_bin_types})
			if(NOT (_bt STREQUAL "RUNTIME"))
				set(CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}")
				set(CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}")
				DEBUG_PRINT(::OUTPATH "Setting _OUTPUT_DIRECTORY with : _RUNTIME_OUTPUT_DIRECTORY : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}"
							"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}"
							"\tCMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}"
							)
			endif()
		endforeach()
	endif()
	unset(_path)
	unset(_bt)
#[==============================[
#]==============================]
	unset(_bin_types)
endmacro()

macro(rcdev_print_current_out_dirs)
	list(APPEND _bin_types "RUNTIME" "ARCHIVE" "LIBRARY")
	if(WIN32)
		list(APPEND _bin_types "PDB")
	endif()
	DEBUG_PRINT(::OUTPATH "CMAKE_CURRENT_SOURCE_DIR:${CMAKE_CURRENT_SOURCE_DIR}")
	foreach(_bt ${_bin_types})
		if(WIN32)
			DEBUG_PRINT(::OUTPATH "\tCMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD=${CMAKE_${_bt}_OUTPUT_DIRECTORY_RELEASE_MD}"
			                      "\tCMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD=${CMAKE_${_bt}_OUTPUT_DIRECTORY_DEBUG_MDD}")
		else()
			DEBUG_PRINT(::OUTPATH "\tCMAKE_${_bt}_OUTPUT_DIRECTORY=${CMAKE_${_bt}_OUTPUT_DIRECTORY}")
		endif()
	endforeach()
	unset(_bt)
	unset(_bin_types)
endmacro()

macro(rcdev_make_lib_archive TARGET_NAME )
    set(options)
    set(oneValueArgs LIBNAME LIBTYPE LANGUAGE PATHROOT)
    set(multiValueArgs LIBHEADERS LINKDEPENDS PATHS COMPILEOPTS)

    cmake_parse_arguments(_MLA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

DEBUG_PRINT(::MESSAGE "TARGET_NAME=${TARGET_NAME}")
    if ((_WPL_LIBNAME MATCHES "^(lib)(.*)$") AND (CMAKE_FIND_LIBRARY_PREFIXES MATCHES lib ))
        set(_origional_CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES}) 
        set(CMAKE_FIND_LIBRARY_PREFIXES "")
    endif()
endmacro()

macro(rcdev_wrap_prebuilt_library TARGET_NAME )
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
        rcdev_get_platform_suffix(_plat_suffix)
        if (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "ORACLE" 
                AND RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 6 )
            rcdev_get_platform_suffix(_plat_suffix2 "gcc" "rhel")
        elseif ( ((RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "CENTOS") OR
                (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "REDHATLINUX")) AND
                (RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 7 ) )
            rcdev_get_platform_suffix(_plat_suffix2 "gcc" "centOS")
        elseif ( ((RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "CENTOS") OR
                (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "REDHATLINUX")) AND 
                 ((RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 8 ) OR (RCDEV_HOST_SYSTEM_FLAVOR_REL EQUAL 9 )) )
             rcdev_get_platform_suffix(_plat_suffix2 "gcc" "rhel")
		elseif (RCDEV_HOST_SYSTEM_FLAVOR_U MATCHES "UBUNTU")
			rcdev_get_platform_suffix(_plat_suffix2 "ubuntu" "compiler_rhel8")
        else()
            rcdev_get_platform_suffix(_plat_suffix2 "gcc" )
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
        #DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hints}")
        unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
        find_library(${TARGET_NAME}_LIBRARY_${_type_U}
                        NAMES ${_WPL_LIBNAME}
                        PATHS ${_hints}
                        PATH_SUFFIXES   Shared
                        DOC "Library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                        NO_DEFAULT_PATH)
        set(_search_lib ${${TARGET_NAME}_LIBRARY_${_type_U}})
        #DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
        mark_as_advanced(${TARGET_NAME}_LIBRARY_${_type_U})

    else() # if (WIN32)

        rcdev_get_platform_suffix(_plat_suffix)
        foreach(_type IN LISTS CMAKE_CONFIGURATION_TYPES)
            string(TOUPPER ${_type} _type_U)
            #if (${_WPL_LIBTYPE} STREQUAL SHARED)
            if (_shared_lib)
                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}/Shared")
                #DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hint}")
                unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_IMP_LIBRARY_${_type_U}
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Shared import library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_lib ${${TARGET_NAME}_IMP_LIBRARY_${_type_U}})
                #DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
                mark_as_advanced(${TARGET_NAME}_IMP_LIBRARY_${_type_U})

                set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}/Shared")
                #DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME}_DLL in : ${_hint}")
                unset(${TARGET_NAME}_DLL_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_DLL_LIBRARY_${_type_U}
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Shared DLL library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_dlllib ${${TARGET_NAME}_DLL_LIBRARY_${_type_U}})
                #DEBUG_PRINT(::MESSAGE "Result:${_search_dlllib}\n")
                mark_as_advanced(${TARGET_NAME}_DLL_LIBRARY_${_type_U})

                set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

            else()
                set(_hints "${_WPL_PATHROOT}/${_plat_suffix}/${_type}")
                #DEBUG_PRINT(::MESSAGE "Looking for ${_WPL_LIBNAME} in : ${_hint}")
                unset(${TARGET_NAME}_LIBRARY_${_type_U} CACHE)
                find_library(${TARGET_NAME}_LIBRARY_${_type_U} 
                                NAMES ${_WPL_LIBNAME}
                                PATHS ${_hints}
                                DOC "Static library ${_WPL_LIBNAME} as target ${TARGET_NAME}"
                                NO_DEFAULT_PATH )
                set(_search_lib ${${TARGET_NAME}_LIBRARY_${_type_U}})
                #DEBUG_PRINT(::MESSAGE "Result:${_search_lib}\n")
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
        #       i.e rtsdk::
        # set(TARGET_NAME "rtsdk::${TARGET_NAME}")
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

