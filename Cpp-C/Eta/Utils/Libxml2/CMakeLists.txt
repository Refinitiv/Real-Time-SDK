if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)

	if(CMAKE_HOST_WIN32)
		set(libxml2_compile_opts "/D LIBXML_STATIC")
	else()
		set(libxml2_compile_opts  
					"-DHAVE_CONFIG_H"
                    "-DLIBXML_THREAD_ENABLED"
                    "-D_POSIX_PTHREAD_SEMANTICS"
			)
	endif()
	
    rcdev_wrap_prebuilt_library(LibXml2
                          LIBNAME libxml2
                          LIBTYPE STATIC
                          LANGUAGE C
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${CMAKE_CURRENT_SOURCE_DIR}
                                     ${CMAKE_CURRENT_SOURCE_DIR}/include/libxml
                                     ${CMAKE_CURRENT_SOURCE_DIR}/include
						  COMPILEOPTS ${libxml2_compile_opts}
                        )

	rcdev_add_target(LibXml2  LibXml2)
	
endif()
