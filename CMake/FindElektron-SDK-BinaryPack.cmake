DEBUG_PRINT(::MESSAGE "
    IN FIND   Elektron-SDK-BinaryPack.cmake
    Elektron-SDK-BinaryPack_DIR : ${Elektron-SDK-BinaryPack_DIR}
   " )
if (NOT Elektron-SDK-BinaryPack_FOUND)
	set(_Elektron-SDK-BinaryPack_REQUIRED "")
	if (Elektron-SDK-BinaryPack_FOUND_REQUIRED)
		set(_Elektron-SDK-BinaryPack_REQUIRED REQUIRED)
    endif ()
	set(_Elektron-SDK-BinaryPack_QUIET "")
	if (Elektron-SDK-BinaryPack_FIND_QUIETLY)
		set(_Elektron-SDK-BinaryPack_QUIET QUIET)
    endif ()

DEBUG_PRINT(::MESSAGE "
	In Elektron-SDK-BinaryPackFind: about to make recursive call
")
	find_package(Elektron-SDK-BinaryPack 
					${Elektron-SDK-BinaryPack_VERSION}
					${_Elektron-SDK-BinaryPack_REQUIRED}
					${_Elektron-SDK-BinaryPack_QUIET}
					CONFIG
					PATHS ${Elektron-SDK-BinaryPack_BINARY_DIR}
					NO_CMAKE_PACKAGE_REGISTRY
					NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
DEBUG_PRINT(::MESSAGE "
	In Elektron-SDK-BinaryPackFind: returned from recursive call
	Elektron-SDK-BinaryPack_
		dacs_INCLUDE_DIRS           : ${Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS}
		ansi_INCLUDE_DIRS           : ${Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS}
		rsslVACache_INCLUDE_DIRS    : ${Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
    ")
endif()

#[====================================================================================[
# Find Libraries and headers
if (Elektron-SDK-BinaryPack_SOURCE_DIR)
    set(_hint "${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Utils/Dacs/Libs/${_suffix}/Optimized")
    message("Looking for dacs in : ${_hint}")
    find_library(Elektron-SDK-BinaryPack_dacs_LIBRARY 
                    dacs 
                    HINT ${_hint}
                    DOC "DACS library from Elektron-SDK-BinaryPack"
                    NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH
                )

    set(_hint "${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Utils/Ansi/Libs/${_suffix}/Optimized")
    message("Looking for ansi in : ${_hint}")
    find_library(Elektron-SDK-BinaryPack_ansi_LIBRARY 
                    ansi 
                    HINT ${_hint}
                    DOC "Ansi library from Elektron-SDK-BinaryPack"
                    NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH
                )

    set(_hint "${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Libs/${_suffix}/${CMAKE_BUILD_TYPE}")
    message("Looking for rsslVACache_static in : ${_hint}")
    find_library(Elektron-SDK-BinaryPack_rsslVACache_static_LIBRARY 
                    rsslVACache 
                    HINT ${_hint}
                    DOC "Rssl Value Add Cache library from Elektron-SDK-BinaryPack"
                    NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH
                )

    set(_hint "${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Libs/${_suffix}/${CMAKE_BUILD_TYPE}/Shared")
    message("Looking for rsslVACache_shared in : ${_hint}")
    find_library(Elektron-SDK-BinaryPack_rsslVACache_shared_LIBRARY 
                    rsslVACache 
                    HINT ${_hint} 
                    DOC "Rssl Value Add Cache shared library from Elektron-SDK-BinaryPack"
                    NO_SYSTEM_ENVIRONMENT_PATH NO_DEFAULT_PATH
                )
    # 
    #
    find_path(Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS 
                NAMES dacs_lib.h
                PATHS
                ${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Utils/Dacs/Include
                DOC "Dacs library Include path"
            )

    find_path(Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS 
                NAMES ansi/ansi_int.h dev/platform.h
                PATHS
                ${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Utils/Ansi/Include
                DOC "Ansi library Include path"
                #PATH_SUFFIXES ansi dev
            )

    find_path(Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS 
                NAMES rtr/rsslCacheDefs.h
                PATHS
                ${Elektron-SDK-BinaryPack_SOURCE_DIR}/Cpp-C/Eta/Include/Cache
                DOC "Rssl Value Add Cache library Include path"
                #PATH_SUFFIXES rtr
            )
    # Export paths for advanced configurations
    mark_as_advanced (
                     Elektron-SDK-BinaryPack_dacs_LIBRARY
                     Elektron-SDK-BinaryPack_ansi_LIBRARY
                     Elektron-SDK-BinaryPack_rsslVACache_static_LIBRARY
                     Elektron-SDK-BinaryPack_rsslVACache_shared_LIBRARY
                     Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS
                     Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS
                     Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS
                    )

    # import targets
    add_library(Elektron-SDK-BinaryPack::dacsLib UNKNOWN IMPORTED)
    set_target_properties(Elektron-SDK-BinaryPack::dacsLib 
                          PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                            INTERFACE_INCLUDE_DIRECTORIES 
                                ${Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS}
                            IMPORTED_LOCATION                 
                                ${Elektron-SDK-BinaryPack_dacs_LIBRARY}
                        )
    add_library(Elektron-SDK-BinaryPack::ansiLib UNKNOWN IMPORTED)
    set_target_properties(Elektron-SDK-BinaryPack::ansiLib 
                          PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                            INTERFACE_INCLUDE_DIRECTORIES 
                                ${Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS}
                            IMPORTED_LOCATION                 
                                ${Elektron-SDK-BinaryPack_ansi_LIBRARY}
                        )
    add_library (Elektron-SDK-BinaryPack::rsslVACache STATIC IMPORTED)
    set_target_properties(Elektron-SDK-BinaryPack::rsslVACache_static 
                          PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                ${Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
                            IMPORTED_LOCATION                 
                                ${Elektron-SDK-BinaryPack_rsslVACache_static_LIBRARY}
                        )
    add_library (Elektron-SDK-BinaryPack::rsslVACache_shared SHARED IMPORTED)
    set_target_properties(Elektron-SDK-BinaryPack::rsslVACache_shared 
                          PROPERTIES
                            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                            INTERFACE_INCLUDE_DIRECTORIES 
                                ${Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
                            IMPORTED_LOCATION                 
                                ${Elektron-SDK-BinaryPack_rsslVACache_shared_LIBRARY}
                        )
    # aliases / backwards compatibility
    if (NOT Elektron-SDK-BinaryPack_INCLUDE_DIRS)
        set (Elektron-SDK-BinaryPack_INCLUDE_DIRS
                        ${Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
                        ${Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS}
                        ${Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS}
           )
    endif ()

    if (NOT Elektron-SDK-BinaryPack_LIBRARIES)
        set (Elektron-SDK-BinaryPack_LIBRARIES rsslVACache_static rsslVACache_shared ansi dacs)
    endif ()
endif ()
message("

    ABOUT FINISHED with  FIND Elektron-SDK-BinaryPack
	Elektron-SDK-BinaryPack_
        dacs_LIBRARY               : ${Elektron-SDK-BinaryPack_dacs_LIBRARY}
        ansi_LIBRARY               : ${Elektron-SDK-BinaryPack_ansi_LIBRARY}
        rsslVACache_static_LIBRARY : ${Elektron-SDK-BinaryPack_rsslVACache_static_LIBRARY}
        rsslVACache_shared_LIBRARY : ${Elektron-SDK-BinaryPack_rsslVACache_shared_LIBRARY}
		
	Elektron-SDK-BinaryPack_
        dacs_INCLUDE_DIRS           : ${Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS}
        ansi_INCLUDE_DIRS           : ${Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS}
        rsslVACache_INCLUDE_DIRS    : ${Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
")
#]====================================================================================]
# handle the QUIETLY and REQUIRED arguments and set *_FOUND to TRUE
# if all listed variables are found or TRUE
include (FindPackageHandleStandardArgs)

find_package_handle_standard_args ( Elektron-SDK-BinaryPack
                      REQUIRED_VARS
						Elektron-SDK-BinaryPack_LIBRARIES
                        Elektron-SDK-BinaryPack_dacs_INCLUDE_DIRS
                        Elektron-SDK-BinaryPack_ansi_INCLUDE_DIRS
                        Elektron-SDK-BinaryPack_rsslVACache_INCLUDE_DIRS
                         )

if (NOT DEFINED Elektron-SDK-BinaryPack_FOUND 
    AND DEFINED ELEKTRON-SDK-BINARYPACK_FOUND) 
    set (Elektron-SDK-BinaryPack_FOUND "${ELEKTRON-SDK-BINARYPACK_FOUND}") 
else()
	set(Elektron-SDK-BinaryPack_FOUND TRUE)
endif ()

