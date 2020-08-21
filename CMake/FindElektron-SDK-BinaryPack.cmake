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
	with Elektron-SDK-BinaryPack_ROOT:${Elektron-SDK-BinaryPack_ROOT}
")
	find_package(Elektron-SDK-BinaryPack 
					${Elektron-SDK-BinaryPack_VERSION}
					${_Elektron-SDK-BinaryPack_REQUIRED}
					${_Elektron-SDK-BinaryPack_QUIET}
					CONFIG
					PATHS 
						${Elektron-SDK-BinaryPack_ROOT}
						${Elektron-SDK-BinaryPack_BINARY_DIR}
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
    AND DEFINED RTSDK-BINARYPACK_FOUND) 
    set (Elektron-SDK-BinaryPack_FOUND "${RTSDK-BINARYPACK_FOUND}") 
else()
	set(Elektron-SDK-BinaryPack_FOUND TRUE)
endif ()

