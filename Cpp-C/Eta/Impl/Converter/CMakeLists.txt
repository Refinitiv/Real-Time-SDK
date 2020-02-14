
project(rwfJsonConverter VERSION 1.2.0.0  )

if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)

	if(CMAKE_HOST_WIN32)
		set(librsslJsonConverter_shared_compile_opts "/D RSSL_RJC_IMPORTS")
	endif()

    rcdev_wrap_prebuilt_library(librsslJsonConverter
				LIBNAME librsslJsonConverter
				LIBTYPE STATIC
				LANGUAGE C
				PATHROOT   ${Eta_SOURCE_DIR}/Libs
				LIBHEADERS ${Eta_SOURCE_DIR}/Include/Codec
				           ${Eta_SOURCE_DIR}/Include/Converter
					   ${Eta_SOURCE_DIR}/Include/Util
					   ${Eta_SOURCE_DIR}/Impl/Util/Include
					   ${Eta_SOURCE_DIR}/Impl/Converter/Include
					   ${Eta_BINARY_DIR}/Include
				)

    rcdev_wrap_prebuilt_library(librsslJsonConverter_shared
				LIBNAME librsslJsonConverter
				LIBTYPE SHARED
				LANGUAGE C
				PATHROOT   ${Eta_SOURCE_DIR}/Libs
				LIBHEADERS ${Eta_SOURCE_DIR}/Include/Codec
				           ${Eta_SOURCE_DIR}/Include/Converter
					   ${Eta_SOURCE_DIR}/Include/Util
					   ${Eta_SOURCE_DIR}/Impl/Util/Include
					   ${Eta_SOURCE_DIR}/Impl/Converter/Include
					   ${Eta_BINARY_DIR}/Include
				COMPILEOPTS ${librsslJsonConverter_shared_compile_opts}
				)

else()

set(rsslJCSrcFiles  rsslJsonConverter.C
					rwfToJsonConverter.C 
					rwfToJsonBase.C
					jsonToRwfConverter.C
					jsonToRsslMsgDecoder.C
					jsonToRwfSimple.C
					rwfToJsonSimple.C
					jsonToRwfBase.C
					jsmn.C 
					EnumTableDefinition.C
					Include/rtr/jsmn.h
					Include/rtr/jsonSimpleDefs.h
					Include/rtr/jsonToRsslMsgDecoder.h
					Include/rtr/jsonToRwfBase.h
					Include/rtr/jsonToRwfSimple.h
					Include/rtr/jsonToRwfConverter.h
					Include/rtr/rwfToJsonBase.h
					Include/rtr/rwfToJsonSimple.h
					Include/rtr/rwfToJsonConverter.h
					Include/rtr/rjcstring.h
					Include/rtr/EnumTableDefinition.h
					)

set(rsslJCIncFiles 
					${Eta_SOURCE_DIR}/Include/Converter/rtr/rsslJsonConverter.h
					)

set(rsslJCAdditionalFiles 
					${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataDictionary.h 
					${Eta_SOURCE_DIR}/Include/Util/rtr/rsslTypes.h
					${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/custmem.h
					Include/rtr/rjchexdmp.h
					Include/rtr/rjcbuffer.h
					rjcstring.C
					rjchexdmp.C
					rjcbuffer.C
					)

#[==========================================================[
if (CMAKE_HOST_WIN32)
	set(rjcWinRCFiles
#                ${Eta_SOURCE_DIR}/Include/rjc.rc
#                ${Eta_SOURCE_DIR}/Include/rjcResource.h
		)
else()
#		set(staticVersionSrc	rjcVersionStatic.c)
#		set(sharedVersionSrc	rjcVersionShared.c)
endif()
#]==========================================================]

add_library(librsslJsonConverter STATIC 
									${rsslJCSrcFiles} 
									${rsslJCIncFiles} 
									${rsslJCAdditionalFiles}
									)

set_target_properties( librsslJsonConverter PROPERTIES PREFIX "" )

target_include_directories(librsslJsonConverter
							PRIVATE
								${Eta_SOURCE_DIR}/Include/Codec
								${Eta_SOURCE_DIR}/Include/Transport
								${Eta_SOURCE_DIR}/Include/Util
								${Eta_SOURCE_DIR}/Impl/Util/Include
								Include
							PUBLIC
								${Eta_SOURCE_DIR}/Include/Converter
							)

add_library( librsslJsonConverter_shared SHARED 
									${rsslJCSrcFiles} 
									${rsslJCIncFiles} 
									${rsslJCAdditionalFiles}
								)

target_include_directories(librsslJsonConverter_shared
									PRIVATE
										${Eta_SOURCE_DIR}/Include/Codec
										${Eta_SOURCE_DIR}/Include/Transport
										${Eta_SOURCE_DIR}/Include/Util
										${Eta_SOURCE_DIR}/Impl/Util/Include
										Include
									PUBLIC
										${Eta_SOURCE_DIR}/Include/Converter
										)

target_link_libraries(librsslJsonConverter_shared librssl)

if (CMAKE_HOST_WIN32)
	target_compile_options(librsslJsonConverter PRIVATE ${RCDEV_DEBUG_TYPE_FLAGS_STATIC})

	target_compile_options( librsslJsonConverter_shared 
										INTERFACE
											"/D RSSL_RJC_IMPORTS"
										PRIVATE
											"/D RSSL_RJC_EXPORTS"
											${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
											${RCDEV_TYPE_CHECK_FLAG}
											$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
							)
										   
	set_target_properties( librsslJsonConverter_shared 
							PROPERTIES 
								OUTPUT_NAME librsslJsonConverter
								RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
								ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD 
									${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD 
									${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
								PDB_OUTPUT_DIRECTORY_DEBUG_MDD 		
									${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
								PDB_OUTPUT_DIRECTORY_RELEASE_MD 	
									${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared

						)

else()
	set_target_properties( librsslJsonConverter_shared 
							PROPERTIES 
								PREFIX "" 
								OUTPUT_NAME librsslJsonConverter
								VERSION
									${Eta_VERSION}
								SOVERSION 
									${librsslJsonConverter_SO_VERSION} 
						)

endif()

DEBUG_PRINT(librsslJsonConverter)

if (BUILD_UNIT_TESTS)
	add_subdirectory( Test )
endif()

endif()

#rcdev_update_output_dirs(RESTORE)

