
project(Eta 
		VERSION ${esdk_INTERNAL_API_VERSION_MAJOR}.${esdk_VERSION_MINOR}.${esdk_VERSION_PATCH}.${esdk_VERSION_TWEAK}
		LANGUAGES C CXX)

DEBUG_PRINT(Eta_VERSION)

#Setup version file generation.
configure_file(Include/rsslVersion.h.in Include/rsslVersion.h @ONLY)
configure_file(Impl/Ansi/ansiVersion.h.in Impl/Ansi/ansiVersion.h @ONLY)
configure_file(Impl/Converter/Include/rtr/jsonVersion.h.in Include/jsonVersion.h @ONLY)

if (NOT RCDEV_REL_${PROJECT_NAME}_LIB_DIR)
    set(RCDEV_REL_${PROJECT_NAME}_LIB_DIR ${PROJECT_NAME}/Libs)
endif()

if (NOT RCDEV_REL_${PROJECT_NAME}_BIN_DIR)
    set(RCDEV_REL_${PROJECT_NAME}_BIN_DIR ${PROJECT_NAME}/Executables)
endif()

rcdev_update_output_dirs(SAVE)
# Save off the original CMAKE_<XXX>_OUTPUT_DIRECTORY prefix.  This  is mainly
# for WIN32 output crossover with DLL going to _RUNTIME_, etc....

if( CMAKE_HOST_UNIX )
	rcdev_update_output_dirs(RUNTIME "${RCDEV_REL_${PROJECT_NAME}_BIN_DIR}/${RCDEV_POST_PREFIX_PATH}")
	rcdev_update_output_dirs(ARCHIVE "${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}/${RCDEV_POST_PREFIX_PATH}" 
							 LIBRARY "${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}/${RCDEV_POST_PREFIX_PATH}/Shared" )
else()
	rcdev_update_output_dirs("${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}")
	rcdev_update_output_dirs(POST_PREFIX)
endif()

add_subdirectory( Impl )

if( CMAKE_HOST_WIN32 )
	# Reset to the original CMAKE_<XXX>_OUTPUT_DIRECTORY prefix for executables
	rcdev_update_output_dirs(RESTORE)
	rcdev_update_output_dirs("${RCDEV_REL_${PROJECT_NAME}_BIN_DIR}")
	rcdev_update_output_dirs(POST_PREFIX)
endif()

if (BUILD_ETA_APPLICATIONS)
    DEBUG_PRINT(BUILD_ETA_APPLICATIONS)
    add_subdirectory( Applications )
else()
    message(STATUS 
            "Skipping Eta Applications Build: BUILD_ETA_APPLICATIONS:${BUILD_ETA_APPLICATIONS}")
endif()

# unit tests
if (BUILD_ETA_UNIT_TESTS)
    DEBUG_PRINT(BUILD_ETA_UNIT_TESTS)
    add_subdirectory( TestTools/UnitTests )
else()
    message(STATUS 
            "Skipping Eta Unit Tests Build: BUILD_ETA_UNIT_TESTS:${BUILD_ETA_UNIT_TESTS}")
endif()

# doxygen
if (BUILD_ETA_DOXYGEN)
    add_subdirectory( Docs/doxygen )
else()
    message(STATUS 
            "Skipping Eta Doxygen Build: BUILD_ETA_DOXYGEN:${BUILD_ETA_DOXYGEN}")
endif()

