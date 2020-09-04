DEBUG_PRINT(::MESSAGE "
    IN FIND   RTSDK-BinaryPack.cmake
    RTSDK-BinaryPack_DIR : ${RTSDK-BinaryPack_DIR}
   " )
if (NOT RTSDK-BinaryPack_FOUND)
	set(_RTSDK-BinaryPack_REQUIRED "")
	if (RTSDK-BinaryPack_FOUND_REQUIRED)
		set(_RTSDK-BinaryPack_REQUIRED REQUIRED)
    endif ()
	set(_RTSDK-BinaryPack_QUIET "")
	if (RTSDK-BinaryPack_FIND_QUIETLY)
		set(_RTSDK-BinaryPack_QUIET QUIET)
    endif ()

DEBUG_PRINT(::MESSAGE "
	In RTSDK-BinaryPackFind: about to make recursive call
	with RTSDK-BinaryPack_ROOT:${RTSDK-BinaryPack_ROOT}
")
	find_package(RTSDK-BinaryPack 
					${RTSDK-BinaryPack_VERSION}
					${_RTSDK-BinaryPack_REQUIRED}
					${_RTSDK-BinaryPack_QUIET}
					CONFIG
					PATHS 
						${RTSDK-BinaryPack_ROOT}
						${RTSDK-BinaryPack_BINARY_DIR}
					NO_CMAKE_PACKAGE_REGISTRY
					NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)
DEBUG_PRINT(::MESSAGE "
	In RTSDK-BinaryPackFind: returned from recursive call
	RTSDK-BinaryPack_
		dacs_INCLUDE_DIRS           : ${RTSDK-BinaryPack_dacs_INCLUDE_DIRS}
		ansi_INCLUDE_DIRS           : ${RTSDK-BinaryPack_ansi_INCLUDE_DIRS}
		rsslVACache_INCLUDE_DIRS    : ${RTSDK-BinaryPack_rsslVACache_INCLUDE_DIRS}
    ")
endif()

# handle the QUIETLY and REQUIRED arguments and set *_FOUND to TRUE
# if all listed variables are found or TRUE
include (FindPackageHandleStandardArgs)

find_package_handle_standard_args ( RTSDK-BinaryPack
                      REQUIRED_VARS
						RTSDK-BinaryPack_LIBRARIES
                        RTSDK-BinaryPack_dacs_INCLUDE_DIRS
                        RTSDK-BinaryPack_ansi_INCLUDE_DIRS
                        RTSDK-BinaryPack_rsslVACache_INCLUDE_DIRS
                         )

if (NOT DEFINED RTSDK-BinaryPack_FOUND 
    AND DEFINED RTSDK-BINARYPACK_FOUND) 
    set (RTSDK-BinaryPack_FOUND "${RTSDK-BINARYPACK_FOUND}") 
else()
	set(RTSDK-BinaryPack_FOUND TRUE)
endif ()

