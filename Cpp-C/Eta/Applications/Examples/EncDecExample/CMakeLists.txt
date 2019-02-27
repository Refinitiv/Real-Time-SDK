
set( SOURCE_FILES
  elementListEncDec.c        fieldListEncDec.c        filterListEncDec.c
  mapEncDec.c                msgEncDec.c              rsslEncDecExample.c
  seriesEncDec.c             vectorEncDec.c
  )

add_executable( EncDecExample_shared ${SOURCE_FILES} )
target_include_directories(EncDecExample_shared
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
                )

add_executable( EncDecExample ${SOURCE_FILES} )
target_include_directories(EncDecExample
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<BUILD_INTERFACE:${EtaExamples_SOURCE_DIR}/Examples/Common>
                )

target_link_libraries( EncDecExample 
							librssl 
							${SYSTEM_LIBRARIES} 
						)
target_link_libraries( EncDecExample_shared 
							librssl_shared 
							${SYSTEM_LIBRARIES} 
						)

set_target_properties( EncDecExample EncDecExample_shared
                        PROPERTIES 
                            OUTPUT_NAME EncDecExample )

if ( CMAKE_HOST_UNIX )

    set_target_properties( EncDecExample_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shared )

# if ( CMAKE_HOST_WIN32 )
else()
	target_compile_options( EncDecExample 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
	target_compile_options( EncDecExample_shared 
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
    set_target_properties( EncDecExample_shared 
                            PROPERTIES 
                                RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								LIBRARY_OUTPUT_DIRECTORY_RELEASE_MD
                                    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                LIBRARY_OUTPUT_DIRECTORY_DEBUG_MDD
                                    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								PDB_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
								PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                            )
endif()


