
project(Ema
		VERSION ${rtsdk_INTERNAL_API_VERSION_MAJOR}.${rtsdk_INTERNAL_API_VERSION_MINOR}.${rtsdk_INTERNAL_API_VERSION_PATCH}.${rtsdk_INTERNAL_API_VERSION_TWEAK}
		LANGUAGES C CXX)
		
#DEBUG_PRINT(Ema_VERSION)

# Configure version information
# relative paths for configure_file are the _CURRENT_SOURCE_DIR for the input file
# and _CURRENT_BINARY_DIR for the output file location
configure_file(Src/Access/EmaVersion.h.in Src/Access/EmaVersion.h @ONLY)
		
if (NOT RCDEV_REL_${PROJECT_NAME}_LIB_DIR)
    set(RCDEV_REL_${PROJECT_NAME}_LIB_DIR ${PROJECT_NAME}/Libs)
endif()

if (NOT RCDEV_REL_${PROJECT_NAME}_BIN_DIR)
    set(RCDEV_REL_${PROJECT_NAME}_BIN_DIR ${PROJECT_NAME}/Executables)
endif()

rcdev_update_output_dirs(SAVE)
# Save off the origional CMAKE_<XXX>_OUTPUT_DIRECTORY prefix.  This  is mainly
# for WIN32 output crossover with DLL going to _RUNTIME_, etc....


if( CMAKE_HOST_UNIX )
	rcdev_update_output_dirs(RUNTIME "${RCDEV_REL_${PROJECT_NAME}_BIN_DIR}/${RCDEV_POST_PREFIX_PATH}")
	rcdev_update_output_dirs(ARCHIVE "${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}/${RCDEV_POST_PREFIX_PATH}" 
							 LIBRARY "${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}/${RCDEV_POST_PREFIX_PATH}/Shared" )
else()
	rcdev_update_output_dirs("${RCDEV_REL_${PROJECT_NAME}_LIB_DIR}")
	rcdev_update_output_dirs(POST_PREFIX)
endif()

# windows. Supported builds include Debug_MDd, Release_MD
# arguments from rtsdk dev build, except no RDEV args
if ( CMAKE_HOST_WIN32 )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D __EMA_COPY_ON_SET__ " )
  set( "CMAKE_CXX_FLAGS_DEBUG_MDD_SHARED" "${CMAKE_CXX_FLAGS_DEBUG_MDD} /D EMA_LIBRARY_EXPORTS" )

  # set ETA library path names
  STRING( REGEX REPLACE "v" "WIN_64_VS" VSSTR "${CMAKE_VS_PLATFORM_TOOLSET}" )
endif()

add_subdirectory( Src/Access )

if( CMAKE_HOST_WIN32 )
	# Reset to the origional CMAKE_<XXX>_OUTPUT_DIRECTORY prefix for executables
	rcdev_update_output_dirs(RESTORE)
	rcdev_update_output_dirs("${RCDEV_REL_${PROJECT_NAME}_BIN_DIR}")
	rcdev_update_output_dirs(POST_PREFIX)
endif()

if (BUILD_EMA_EXAMPLES)
    add_subdirectory( Examples )
endif()

# doxygen
if (BUILD_EMA_DOXYGEN)
    add_subdirectory( Docs/doxygen )
else()
    message(STATUS 
            "Skipping Ema Doxygen Build: BUILD_EMA_DOXYGEN:${BUILD_EMA_DOXYGEN}")
endif()

