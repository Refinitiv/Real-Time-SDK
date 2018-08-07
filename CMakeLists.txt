cmake_minimum_required (VERSION 3.10)
# The minimum version was chosen from a combination of the need for
# critical bug fixes by CMake, the idea of taking advantage of future
# CMake fixes soon to be released and wanting to fully embrace the
# target oriented CMake build framework.  It may be possible to change
# the minimum required version to an earlier version, but there is no
# guarantee of a successful build if changed.

project (esdk
		VERSION 1.2.1.0
		LANGUAGES C CXX)

set(esdk_CMAKE_DIR "${esdk_SOURCE_DIR}/CMake")
list(APPEND CMAKE_MODULE_PATH ${esdk_CMAKE_DIR})
include(rcdevEnvironment)
include(setConfigType)
include(setCompilerOptions)
include(setBinaryEnvironment)
include(setProjectPaths)

#
# Set up default build paths if not already defined
if (RCDEV_OUTPUT_DIRECTORY_PREFIX)
    set(_output_dir_prefix ${RCDEV_OUTPUT_DIRECTORY_PREFIX})
else()
    set(_output_dir_prefix ${esdk_SOURCE_DIR}/Cpp-C)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${_output_dir_prefix})
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_output_dir_prefix})
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${_output_dir_prefix})
endif()

if(WIN32)
    foreach (_type IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER ${_type} _type_U)
        if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_type_U})
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_type_U} ${_output_dir_prefix})
            mark_as_advanced(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_type_U})
        endif()
        DEBUG_PRINT(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_type_U})

        if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_type_U})
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_type_U} ${_output_dir_prefix})
            mark_as_advanced(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_type_U})
        endif()
        DEBUG_PRINT(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_type_U})

        if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_type_U})
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_type_U} ${_output_dir_prefix})
            mark_as_advanced(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_type_U})
        endif()
        DEBUG_PRINT(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_type_U})

        if (NOT CMAKE_PDB_OUTPUT_DIRECTORY_${_type_U})
            set(CMAKE_PDB_OUTPUT_DIRECTORY_${_type_U} ${_output_dir_prefix})
            mark_as_advanced(CMAKE_PDB_OUTPUT_DIRECTORY_${_type_U})
        endif()
        DEBUG_PRINT(CMAKE_PDB_OUTPUT_DIRECTORY_${_type_U})
    endforeach()
    unset(_type_U)
    unset(_type)
endif()

include(esdkOptions)
include(esdkInfo)

if(CMAKE_HOST_UNIX)
    configure_file(${esdk_SOURCE_DIR}/CMake/LinuxSoLink.in ${esdk_SOURCE_DIR}/LinuxSoLink @ONLY)
endif()

# If there is a CMAKE_BUILD_TYPE it is important to ensure it is passed down.
if(CMAKE_BUILD_TYPE)
    list(APPEND _EP_DEFAULT_ARGS "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
endif()

if (RCDEV_DEFAULT_ARGS)
    list(APPEND _EP_DEFAULT_ARGS ${RCDEV_DEFAULT_ARGS})
endif()

if(BUILD_ELEKTRON-SDK-BINARYPACK)
	include(addExternal_Elektron-SDK-BinaryPack)
	
	find_package(Elektron-SDK-BinaryPack 1.2.0.1 MODULE REQUIRED )
				
	set(Elektron-SDK-BinaryPack_NAMESPACE "Elektron-SDK-BinaryPack::")
endif()


if (BUILD_UNIT_TESTS)
    # Google Test
    include(addExternal_googletest)

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${googletest_SOURCE_DIR} 
                     ${googletest_BINARY_DIR}
                     EXCLUDE_FROM_ALL)
endif()

add_subdirectory( Cpp-C )

unset(_output_dir_prefix)

