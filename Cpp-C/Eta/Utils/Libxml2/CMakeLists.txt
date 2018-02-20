if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)
    include(prebuiltLibrary)
	if(CMAKE_HOST_WIN32)
		set(libxml2_compile_opts "/D LIBXML_STATIC")
	else()
		set(libxml2_compile_opts  
					"-DHAVE_CONFIG_H"
                    "-DLIBXML_THREAD_ENABLED"
                    "-D_POSIX_PTHREAD_SEMANTICS"
			)
	endif()
	
    wrap_prebuilt_library(libxml2
                          LIBNAME libxml2
                          LIBTYPE STATIC
                          LANGUAGE C
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${CMAKE_CURRENT_SOURCE_DIR}
                                     ${CMAKE_CURRENT_SOURCE_DIR}/include/libxml
                                     ${CMAKE_CURRENT_SOURCE_DIR}/include
						  COMPILEOPTS ${libxml2_compile_opts}
                        )
else()

    add_library(libxml2  STATIC 
                DOCBparser.c HTMLparser.c HTMLtree.c SAX.c SAX2.c
                c14n.c catalog.c chvalid.c debugXML.c dict.c
                encoding.c entities.c error.c globals.c hash.c
                legacy.c list.c nanoftp.c nanohttp.c parser.c
                parserInternals.c pattern.c relaxng.c schematron.c threads.c
                tree.c uri.c valid.c xinclude.c xlink.c
                xmlIO.c xmlmemory.c xmlmodule.c xmlreader.c xmlregexp.c
                xmlsave.c xmlschemas.c xmlschemastypes.c xmlstring.c xmlunicode.c
                xmlwriter.c xpath.c xpointer.c
                acconfig.h config.h elfgcchack.h libxml.h trio.h
                triodef.h trionan.h triop.h triostr.h
                )

    target_include_directories(libxml2 PUBLIC
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/libxml>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                              )
                              
    if (CMAKE_HOST_UNIX)
        set_target_properties(libxml2 PROPERTIES PREFIX "" )
        target_compile_options( libxml2  
                                PUBLIC 
                                    "-DHAVE_CONFIG_H"
                                    "-DLIBXML_THREAD_ENABLED"
                                    "-D_POSIX_PTHREAD_SEMANTICS"
                                )
    else()
        target_compile_options( libxml2  
								INTERFACE
									"/D LIBXML_STATIC"
                                PRIVATE 
                                    ${RCDEV_FLAGS_STATIC}
                                )
    endif()

endif()
