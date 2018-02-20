#
## External Project Name
set(_ep_name "Elektron-SDK-BinaryPack")

#[=============================================================[
list(APPEND esdk_DEPENDS_LIST Elektron-SDK-BinaryPack)

set(CONF_PROJ_NAME Elektron-SDK-BinaryPack)
set(CONF_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CONF_SOURCE_DIR "Elektron-SDK-BinaryPack")
set(CONF_BINARY_DIR "Elektron-SDK-BinaryPack-build")
set(CONF_CONFIGURE_COMMAND)
set(CONF_DOWNLOAD_DIR "")

if (NOT esdk_DEFAULT_ARGS)
    list(APPEND esdk_DEFAULT_ARGS "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
                                "-DCMAKE_PREFIX_PATH:PATH=${CONF_BINARY_DIR};${CMAKE_PREFIX_PATH}"
    )
endif()
#]=============================================================]

set(_EP_PROJ_NAME ${_ep_name})
DEBUG_PRINT(_EP_PROJ_NAME)

if (${_ep_name}_EP_PREFIX_DIR)
    set(_EP_PREFIX_DIR "${${_ep_name}_EP_PREFIX_DIR}")
else()
    set(_EP_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
DEBUG_PRINT(_EP_PREFIX_DIR)

if (${_ep_name}_EP_SOURCE_DIR)
    set(_EP_SOURCE_DIR "${${_ep_name}_EP_SOURCE_DIR}")
else()
    set(_EP_SOURCE_DIR "${_EP_PREFIX_DIR}/${_ep_name}")
endif()
DEBUG_PRINT(_EP_SOURCE_DIR)
set(${_ep_name}_SOURCE_DIR "${_EP_SOURCE_DIR}")

if (${_ep_name}_EP_BINARY_DIR)
    set(_EP_BINARY_DIR "${${_ep_name}_EP_BINARY_DIR}")
else()
    set(_EP_BINARY_DIR "${_EP_PREFIX_DIR}/${_ep_name}-build")
endif()
DEBUG_PRINT(_EP_BINARY_DIR)
set(${_ep_name}_BINARY_DIR "${_EP_BINARY_DIR}")

set(_EP_DOWNLOAD_DIR "${_EP_BINARY_DIR}/${_ep_name}-download")
set(_EP_STAMP_DIR "${_EP_BINARY_DIR}/stamp")
set(_EP_TMP_DIR   "${_EP_BINARY_DIR}/tmp")
set(_EP_INSTALL_DIR "${_EP_BINARY_DIR}/install")
set(_EP_CMAKE_ARGS ${_EP_DEFAULT_ARGS})


if (NOT (IS_DIRECTORY "${${_ep_name}_BINARY_DIR}" )
    OR NOT DEFINED "${${_ep_name}_CONFIGURED_PREFIX_DIR}"
    OR NOT "${${_ep_name}_CONFIGURED_PREFIX_DIR}" STREQUAL "${_EP_PREFIX_DIR}")

    if(NOT EXISTS "${_EP_SOURCE_DIR}/CMakeLists.txt")

		find_package(Git)

        if (${_ep_name}_CMAKE_ARGS)
            list(APPEND _EP_CMAKE_ARGS "${${_ep_name}_CMAKE_ARGS}")
        endif()

        set(_EP_GIT_REPO "${${_ep_name}_repo}")
        set(_EP_GIT_TAG "${${_ep_name}_tag}")

        set(_output_quite "OUTPUT_QUIET")
        if ($ENV{RCDEV_DEBUG_ENABLED})
            list(APPEND _EP_GIT_ARGS "GIT_SHALLOW   0"
                                     "GIT_PROGRESS  1")
            set(_output_quite "")
        endif()

		DEBUG_PRINT(_EP_CMAKE_ARGS)
		DEBUG_PRINT(_EP_GIT_REPO)
		DEBUG_PRINT(_EP_GIT_TAG)
		DEBUG_PRINT(_EP_GIT_ARGS)

        if(EXISTS "${_EP_DOWNLOAD_DIR}/CMakeCache.txt")
            file(REMOVE "${_EP_DOWNLOAD_DIR}/CMakeCache.txt")
        endif()

        configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.externalProject.txt.in
                        "${_EP_DOWNLOAD_DIR}/CMakeLists.txt")

        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                        RESULT_VARIABLE _ret_val
                        WORKING_DIRECTORY "${_EP_DOWNLOAD_DIR}"
                        ${_output_quite}
                        )
        if(_ret_val)
            message(FATAL_ERROR "CMake step for ${_ep_name} failed: ${_ret_val}")
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                        RESULT_VARIABLE _ret_val
                        WORKING_DIRECTORY "${_EP_DOWNLOAD_DIR}"
                        ${_output_quite}
                        )
        if(_ret_val)
            message(FATAL_ERROR "Build step for ${_ep_name} failed: ${_ret_val}")
        endif()
    endif()

    set (${_ep_name}_CONFIGURED_PREFIX_DIR "${_EP_PREFIX_DIR}" CACHE INTERNAL "" FORCE)

endif()

if (EXISTS "${CMAKE_BINARY_DIR}/${_ep_name}")
	set(${_ep_name}_DIR "${CMAKE_BINARY_DIR}/${_ep_name}")
endif()

unset(_ep_name)
unset(_EP_PROJ_NAME)
unset(_EP_PREFIX_DIR)
unset(_EP_SOURCE_DIR)
unset(_EP_BINARY_DIR)
unset(_EP_STAMP_DIR)
unset(_EP_TMP_DIR)
unset(_EP_DOWNLOAD_DIR)
unset(_EP_INSTALL_DIR)
unset(_EP_CMAKE_ARGS)
unset(_EP_GIT_REPO)
unset(_EP_GIT_TAG)
unset(_output_quite)
unset(_ret_val)
