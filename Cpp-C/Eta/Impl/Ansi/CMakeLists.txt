
if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)

    rcdev_wrap_prebuilt_library(libansi 
                          LIBNAME libansi 
                          LIBTYPE STATIC
                          LANGUAGE C
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/
									 ${Eta_SOURCE_DIR}/Impl/Util/Include
                        )

else()

    set(AnsiSrcFiles
		ansiVersion.h
		d_cursor.c
		d_ef.c
		d_fe.c
		d_graphic.c
		d_main.c
		d_misc.c
		d_state.c
		e_main.c
		
		${Eta_SOURCE_DIR}/Include/ansi/ansi_int.h
		${Eta_SOURCE_DIR}/Include/ansi/d_table.h
		${Eta_SOURCE_DIR}/Include/ansi/decodeansi.h
		${Eta_SOURCE_DIR}/Include/ansi/platform.h
		${Eta_SOURCE_DIR}/Include/ansi/q_ansi.h
            
    )

    add_library( libansi STATIC ${AnsiSrcFiles})
	set_target_properties(libansi PROPERTIES OUTPUT_NAME "libansi")

    target_include_directories(libansi
									PUBLIC
										${CMAKE_CURRENT_BINARY_DIR}
										${Eta_SOURCE_DIR}/Include/
										${Eta_SOURCE_DIR}/Impl/Util/Include
									)
    if ( CMAKE_HOST_WIN32 )

        target_compile_options( libansi
									PUBLIC
										"/D __STDC__"
									PRIVATE
									    ${RCDEV_DEBUG_TYPE_FLAGS_STATIC}
                                )
	else()
	    set_target_properties( libansi PROPERTIES PREFIX "" )
	endif()

	
	rcdev_add_target(rtsdk libansi)

endif()

