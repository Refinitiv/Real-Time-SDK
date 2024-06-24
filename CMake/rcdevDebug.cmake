#[=============================================================================[
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.
#]=============================================================================]

option(RCDEV_TRACE_ENABLED "Will be enabled if any of the previous are ON" "$ENV{RCDEV_TRACE_ENABLED}")
mark_as_advanced(RCDEV_TRACE_ENABLED)

if (_rcdevDebugInclude)
	return()
else()
	set(_rcdevDebugInclude TRUE)
endif()

include(CMakeDependentOption)
option(RCDEV_TRACE_ENABLED "Will be enabled if any of the previous are ON" "$ENV{RCDEV_TRACE_ENABLED}")
mark_as_advanced(RCDEV_TRACE_ENABLED)
	# The default value of RCDEV_TRACE_ALL ON 
	#      if RCDEV_TRACE_ENABLED is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_ALL 
							"Print all Types of DEBUG messages" ON
							"RCDEV_TRACE_ENABLED" OFF)
							mark_as_advanced(RCDEV_TRACE_ALL)
	# The default value of RCDEV_TRACE_TARGET OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_TARGETS 
							"Print TARGET type related DEBUG messages" ON
							"RCDEV_TRACE_ENABLED" OFF)
							mark_as_advanced(RCDEV_TRACE_TARGETS)
	# The default value of RCDEV_TRACE_VARIABLE OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_VARIABLES 
							"Print VARIABLE type related DEBUG messages" ON 
							"RCDEV_TRACE_ENABLED" OFF) 
							mark_as_advanced(RCDEV_TRACE_VARIABLES)
	# The default value of RCDEV_TRACE_MESSAGE OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_MESSAGES 
							"Print MESSAGE type related DEBUG messages" 
							ON "RCDEV_TRACE_ENABLED" OFF) 
							mark_as_advanced(RCDEV_TRACE_MESSAGES)
	# The default value of RCDEV_TRACE_SRCREPO OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_SRCREPOS 
							"Print SRCREPO type related DEBUG messages" 
							ON "RCDEV_TRACE_ENABLED" OFF) 
							mark_as_advanced(RCDEV_TRACE_SRCREPOS)
	# The default value of RCDEV_TRACE_FLAGOPTS OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_FLAGOPTS 
							"Print FLAGOPT type related DEBUG messages" 
							ON "RCDEV_TRACE_ENABLED" OFF) 
							mark_as_advanced(RCDEV_TRACE_FLAGOPTS)
	# The default value of RCDEV_TRACE_OUTPATHS OFF 
	#      if RCDEV_TRACE_ALL is TRUE then ON 
	#		else it will not appear as an option
CMAKE_DEPENDENT_OPTION(RCDEV_TRACE_OUTPATHS 
							"Print OUTPATH type related DEBUG messages" 
							ON "RCDEV_TRACE_ENABLED" OFF) 
							mark_as_advanced(RCDEV_TRACE_OUTPATHS)

macro(rcd_cmake_debug _typ _dmsg)
    message("DEBUG ${_typ}: ${_dmsg}")
endmacro()

macro(rcd_cmake_mult_debug _typ)
	set(_prefix "DEBUG ${_typ}")
	set(_mout "")
	foreach(_m ${ARGN})
		set(_mout "${_mout}${_prefix}: ${_m}")
		set(_prefix "\nDEBUG         ")
	endforeach()
	message("${_mout}")
	unset(_prefix)
	unset(_mout)
	unset(_msg)
endmacro()

macro(print_all_variables _match_str)
    message(STATUS "\tDEBUG_PRINT ALL ${_match_str} -----------")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_v_name ${_variableNames})

#[===================================[
		if (_v_name MATCHES "[a-zA-Z0-9]+(::)?")
			message("\n\tFOUND ::  ${_v_name}\n")
			#string(REGEX REPLACE ":" "\\:" _v_clean "${_v_name}")
			string(REGEX MATCHALL "^([a-zA-Z0-9])+(::)?([a-zA-Z0-9])+$" _v_clean "${_v_name}")
			message("\n\tFOUND ::  ${_v_name} CM0:${CMAKE_MATCH_0} CM1:${CMAKE_MATCH_0} CM2:${CMAKE_MATCH_2}\n")
			message("\n\tCLEANED? ::  _v_name:${_v_name}  _v_clean:${_v_clean}\n")
            rcd_cmake_debug("Variable TEST " "${_v_clean}=\${${_v_clean}}")
        endif()
#]===================================]
        if ("x${_match_str}x" STREQUAL "xx")
            message(STATUS "DEBUG Variable: ${_v_name}=${${_v_name}}")
		elseif (_v_name MATCHES "[a-zA-Z0-9]+(::)+")
			rcd_cmake_debug("Variable" "${_v_name}=")
		elseif (_v_name MATCHES "^(${_match_str})+")
            rcd_cmake_debug("Variable" "${_v_name}=\${${_v_name}}")
		elseif (_v_name MATCHES "[_ ]+(${_match_str}(([_]+)|$))+") 
            rcd_cmake_debug("Variable" "${_v_name}=\${${_v_name}}")
        endif()
    endforeach()
    message("")
	unset(_variableNames)
endmacro()

function(_load_cmake_properties cp_list)
# Based off logic found : 
# https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake/51987470#51987470

    execute_process(COMMAND ${CMAKE_COMMAND} --help-property-list 
					RESULT_VARIABLE _ret
					ERROR_VARIABLE _err
					OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

	if(CMAKE_PROPERTY_LIST)
		# Convert command output into a CMake list
		string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
		string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
		list(FILTER CMAKE_PROPERTY_LIST EXCLUDE REGEX "^LOCATION$|^LOCATION_|_LOCATION$")
		list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
		set(${cp_list} ${CMAKE_PROPERTY_LIST} PARENT_SCOPE)
	endif()

	unset(_ret)
	unset(_err)

endfunction(_load_cmake_properties)

function(print_properties)
    message ("CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction(print_properties)

function(print_target_properties _tgt)
# Based off logic found for getting all properties for a target object: 
# https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake/51987470#51987470
    if(NOT TARGET ${_tgt})
		message (STATUS "Target  : ${_tgt} => UNDEFINED\n")
        return()
    endif()

    _load_cmake_properties(CMAKE_PROPERTY_LIST)

	unset(_whitelisted_property_list)
	foreach(_prop ${CMAKE_PROPERTY_LIST})
		if(_prop MATCHES "^(INTERFACE|[_a-z]|IMPORTED_LIBNAME_|MAP_IMPORTED_CONFIG_)|^(COMPATIBLE_INTERFACE_(BOOL|NUMBER_MAX|NUMBER_MIN|STRING)|EXPORT_NAME|IMPORTED(_GLOBAL|_CONFIGURATIONS|_LIBNAME)?|NAME|TYPE|NO_SYSTEM_FROM_IMPORTED)$")
			list(APPEND _whitelisted_property_list ${_prop})
		endif()
	endforeach(_prop)
	get_target_property(_target_t ${_tgt} TYPE)
	message ("\t-- Target : ${_tgt} --")

	if(_target_t STREQUAL "INTERFACE_LIBRARY")
		set(PROP_LIST ${_whitelisted_property_list})
	else()
		set(PROP_LIST "${CMAKE_PROPERTY_LIST}")
	endif()

	list(APPEND PROP_LIST	COMPILE_FLAGS
							COMPILE_OPTIONS 
							COMPILE_DEFINITIONS 
							COMPILE_FEATURES 
							INTERFACE_COMPILE_OPTIONS 
							INTERFACE_COMPILE_DEFINITIONS 
							INTERFACE_COMPILE_FEATURES
							)
	unset(_target_t)
    
	foreach (_prop ${PROP_LIST})
		if (UNIX)
			string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" _prop "${_prop}")
		endif()
		get_property(_propval TARGET ${_tgt} PROPERTY ${_prop} SET)
		if (_propval)
			get_target_property(_propval ${_tgt} ${_prop})
			rcd_cmake_debug("Target  " "${_tgt} ${_prop} = ${_propval}")
		endif()
		unset(_propval)
	endforeach()
	foreach (_build  "" _OPTIMIZED _RELEASE _DEBUG _DEBUG_MDD _RELEASE_MD )
		foreach ( _itm MAP_IMPORTED_CONFIG IMPORTED_LOCATION LINK_FLAGS PDB_NAME COMPILE_PDB_NAME STATIC_LIBRARY_FLAGS)
			set(_prop "${_itm}${_build}")
			get_property(_propval TARGET ${_tgt} PROPERTY ${_prop} SET)
			if (_propval)
				get_target_property(_propval ${_tgt} ${_prop})
				rcd_cmake_debug("Target  " "${_tgt} ${_prop} = ${_propval}")
			endif()
			unset(_propval)
		endforeach()
	endforeach()
	unset(_propval)
	unset(_prop)
endfunction(print_target_properties)

function(DEBUG_PRINT vName)
	if (RCDEV_TRACE_ENABLED)
		set(_mlist ${ARGN})
        string(TOUPPER ${vName} _vName_U)
		if (${ARGC} GREATER 1 AND ${_vName_U} MATCHES "^[ ]*::[ ]*MESSAGE")
			if(NOT (RCDEV_TRACE_MESSAGES OR RCDEV_TRACE_ALL))
				return()
			endif()
			if (${ARGC} GREATER 2)
				rcd_cmake_mult_debug("Message " ${ARGN})
			else()
				rcd_cmake_debug("Message " ${ARGN})
			endif()
		elseif (${ARGC} GREATER 1 AND ${_vName_U} MATCHES "^[ ]*::[ ]*SRCREPO")
			if(NOT (RCDEV_TRACE_SRCREPOS OR RCDEV_TRACE_ALL))
				return()
			endif()
			if (${ARGC} GREATER 2)
				rcd_cmake_mult_debug("SrcRepo " ${ARGN})
			else()
				rcd_cmake_debug("SrcRepo " "${ARGV1}")
			endif()
		elseif (${ARGC} GREATER 1 AND ${_vName_U} MATCHES "^[ ]*::[ ]*OUTPATH")
			if(NOT (RCDEV_TRACE_OUTPATHS OR RCDEV_TRACE_ALL))
				return()
			endif()
			if (${ARGC} GREATER 2)
				rcd_cmake_mult_debug("OutPath " ${ARGN})
			else()
				rcd_cmake_debug("OutPath " "${ARGV1}")
			endif()
		elseif (${ARGC} GREATER 1 AND ${_vName_U} MATCHES "^[ ]*::[ ]*FLAGOPT")
			if(NOT (RCDEV_TRACE_FLAGOPTS OR RCDEV_TRACE_ALL))
				return()
			endif()
			if (${ARGC} GREATER 2)
				rcd_cmake_mult_debug("FlagOpt " ${ARGN})
			else()
				rcd_cmake_debug("FlagOpt " "${ARGV1}")
			endif()
		elseif (${_vName_U} MATCHES "^[ ]*::ALL")
			if (NOT (RCDEV_TRACE_VARIABLES OR RCDEV_TRACE_ALL))
				return()
			endif()
            if (${ARGC} GREATER 1)
			    print_all_variables(${ARGV1})
            else()
			    print_all_variables("")
            endif()
		elseif (TARGET ${vName})
			if (NOT (RCDEV_TRACE_TARGETS OR RCDEV_TRACE_ALL))
				return()
			else()
				print_target_properties(${vName})
			endif()
		else()
			if (NOT (RCDEV_TRACE_VARIABLES OR RCDEV_TRACE_ALL))
				return()
			elseif (${vName} MATCHES "^::.*")
				message(WARNING "Poorly formatted DEBUG_PRINT message:\n"
								"\tthe prefix \'::\', is a reserved character sequence \(argv[1]\'${vName}\'\):\n"
								"\t\tDEBUG_PRINT\(::MESSAGE \'<text>\'\)\n"
								"\t\t\t or \n"
								"\t\tDEBUG_PRINT\(::PRINT_ALL\)-print all CMAKE vars \(the world\)")
			elseif (NOT (${vName} MATCHES "::"))	
				if (NOT(${vName}))
					rcd_cmake_debug("Variable" "${vName}=<UNDEFINED>")
				else()
					rcd_cmake_debug("Variable" "${vName}=\${${vName}}")
				endif()
			else()
				rcd_cmake_debug("Message " "${vName}= \'\'")
			endif ()

		endif(${ARGC} GREATER 1 AND ${_vName_U} MATCHES "^[ ]*::[ ]*MESSAGE")

        unset(_vName_U)
	endif()
endfunction()


