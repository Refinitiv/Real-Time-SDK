project(Reactor)

if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)

    rcdev_wrap_prebuilt_library(librsslVA
                          LIBNAME librsslVA
                          LIBTYPE STATIC
                          LANGUAGE C
                          LINKDEPENDS librssl
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/Reactor
                                     ${Eta_SOURCE_DIR}/Include/Util
                                     ${Eta_SOURCE_DIR}/Include/RDM
                        )

    rcdev_wrap_prebuilt_library(librsslVA_shared
                          LIBNAME librsslVA
                          LIBTYPE SHARED
                          LANGUAGE C
                          LINKDEPENDS librssl_shared
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/Reactor
                                     ${Eta_SOURCE_DIR}/Include/Util
                                     ${Eta_SOURCE_DIR}/Include/RDM
                        )

else()

    set(rsslVASrcFiles
        TunnelStream/bigBufferPool.c
        TunnelStream/bufferPool.c
        TunnelStream/msgQueueEncDec.c
        TunnelStream/persistFile.c
        TunnelStream/rsslRDMQueueMsgInt.c
        TunnelStream/rsslTunnelStream.c
        TunnelStream/tunnelManager.c
        TunnelStream/tunnelStreamImpl.c
        TunnelStream/tunnelSubstream.c
        Watchlist/rsslWatchlistImpl.c
        Watchlist/wlBase.c
        Watchlist/wlDirectory.c
        Watchlist/wlItem.c
        Watchlist/wlLogin.c
        Watchlist/wlMsgReorderQueue.c
        Watchlist/wlPostIdTable.c
        Watchlist/wlService.c
        Watchlist/wlServiceCache.c
        Watchlist/wlSymbolList.c
        Watchlist/wlView.c
        rsslReactor.c
        rsslReactorWorker.c
        rtr/rsslReactorEventQueue.h
        rtr/rsslReactorEventsImpl.h
        rtr/rsslReactorImpl.h
	rtr/rsslReactorTokenMgntImpl.h
        TunnelStream/rtr/bigBufferPool.h
        TunnelStream/rtr/bufferPool.h
        TunnelStream/rtr/msgQueueEncDec.h
        TunnelStream/rtr/msgQueueHeader.h
        TunnelStream/rtr/msgQueueSubstreamHeader.h
        TunnelStream/rtr/msgQueueTimeoutCodes.h
        TunnelStream/rtr/persistFile.h
        TunnelStream/rtr/rsslRDMQueueMsgInt.h
        TunnelStream/rtr/tunnelManager.h
        TunnelStream/rtr/tunnelManagerImpl.h
        TunnelStream/rtr/tunnelStreamImpl.h
        TunnelStream/rtr/tunnelStreamReturnCodes.h
        TunnelStream/rtr/tunnelSubstream.h
        Util/rtr/rsslReactorUtils.h
        Util/rsslRestClientImpl.c
        Util/rtr/rsslRestClientImpl.h
        Watchlist/rtr/rsslWatchlist.h
        Watchlist/rtr/rsslWatchlistImpl.h
        Watchlist/rtr/wlBase.h
        Watchlist/rtr/wlDirectory.h
        Watchlist/rtr/wlItem.h
        Watchlist/rtr/wlLogin.h
        Watchlist/rtr/wlMsgReorderQueue.h
        Watchlist/rtr/wlPostIdTable.h
        Watchlist/rtr/wlService.h
        Watchlist/rtr/wlServiceCache.h
        Watchlist/rtr/wlStream.h
        Watchlist/rtr/wlSymbolList.h
        Watchlist/rtr/wlView.h
        # Eta root Includes
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslClassOfService.h
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslReactorCallbackReturnCodes.h
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslReactorChannel.h
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslReactorEvents.h
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslReactor.h
        ${Eta_SOURCE_DIR}/Include/Reactor/rtr/rsslTunnelStream.h
        #VA RDM Package files
        ../RDM/rsslRDMDictionaryMsg.c
        ../RDM/rsslRDMDirectoryMsg.c
        ../RDM/rsslRDMLoginMsg.c
        ../RDM/rsslRDMQueueMsg.c
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMDictionaryMsg.h
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMDirectoryMsg.h
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMLoginMsg.h
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMMsg.h
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMMsgBase.h
        ${Eta_SOURCE_DIR}/Include/RDM/rtr/rsslRDMQueueMsg.h
        ${Eta_BINARY_DIR}/Include/rsslVersion.h
    )

    if (CMAKE_HOST_WIN32)
        set(rsslVAWinSrcFiles
            ${Eta_SOURCE_DIR}/Impl/Util/DllVAMain.c
            ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslVAExports.h
            ${Eta_SOURCE_DIR}/Include/rsslVA.rc
            ${Eta_SOURCE_DIR}/Include/rsslResource.h
        )
    endif()

	if (NOT WIN32)
		set(staticVAVersionSrc rsslVAVersionStatic.c)
		set(sharedVAVersionSrc rsslVAVersionShared.c)
	endif()

	if(BUILD_ETA_JWT)
		set(l8w8jwt_lib "l8w8jwt::libl8w8jwt")
		set(l8w8jwt_target "$<TARGET_FILE:l8w8jwt::libl8w8jwt>")
		set(l8w8jwt_command "COMMAND ${CMAKE_AR} -x $<TARGET_FILE:l8w8jwt::libl8w8jwt>")
	endif()
		
	#
	##          -- Build librssl STATIC LIBRARY --

	set(_rsslVA_tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/librsslVA_tmp.arch")
	if (NOT EXISTS "${_rsslVA_tmp_dir}/src")
		file(MAKE_DIRECTORY "${_rsslVA_tmp_dir}/src")
	endif()
	
    add_library( librsslVA_tmp STATIC ${rsslVASrcFiles} ${staticVAVersionSrc} )

    if (CMAKE_HOST_WIN32)
		set_target_properties(librsslVA_tmp PROPERTIES 
											ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD
												"${_rsslVA_tmp_dir}/lib/Debug_MDd"
											ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD
												"${_rsslVA_tmp_dir}/lib/Release_MD"
								)
		target_compile_options(librsslVA_tmp PRIVATE 
                                            ${RCDEV_DEBUG_TYPE_FLAGS_STATIC} 
                                            ${RTSDK_EXTRA_FLAGS}
                                )

	else()
		set_target_properties(librsslVA_tmp PROPERTIES 
											ARCHIVE_OUTPUT_DIRECTORY
												"${_rsslVA_tmp_dir}/lib"
								)
	endif()

    target_include_directories(librsslVA_tmp
										PRIVATE
											Watchlist
											Util
											TunnelStream
											../Codec
											../Util/Include
											../Transport
											.
										PUBLIC
											${Eta_SOURCE_DIR}/Include/Reactor
											$<TARGET_PROPERTY:librssl,INTERFACE_INCLUDE_DIRECTORIES>
                                )
	    target_link_libraries( librsslVA_tmp LibXml2::LibXml2 CJSON::CJSON librssl ${l8w8jwt_lib})

DEBUG_PRINT(librsslVA_tmp)

	file(WRITE ${_rsslVA_tmp_dir}/src/librsslVA_tmp.C "void librsslArchive() {}\n")
    add_library(librsslVA STATIC ${_rsslVA_tmp_dir}/src/librsslVA_tmp.C)

    target_include_directories(librsslVA 
										PUBLIC
											$<TARGET_PROPERTY:librsslVA_tmp,INTERFACE_INCLUDE_DIRECTORIES>
								)

	set_target_properties(librsslVA PROPERTIES OUTPUT_NAME "librsslVA")

	target_link_libraries(librsslVA librsslVA_tmp)

    if (CMAKE_HOST_WIN32)
		add_custom_command(TARGET librsslVA POST_BUILD
							COMMAND ${CMAKE_LINKER} /LIB 
														/OUT:"$<TARGET_FILE:librsslVA>" 
														/LTCG "$<TARGET_FILE:librsslVA_tmp>"
															  "$<TARGET_FILE:LibXml2::LibXml2>"
															  "$<TARGET_FILE:CJSON::CJSON>"
															  ${l8w8jwt_target}
							DEPENDS $<TARGET_FILE:librsslVA_tmp>
									$<TARGET_FILE:LibXml2::LibXml2>
									$<TARGET_FILE:CJSON::CJSON>
									${l8w8jwt_target}
							COMMENT "Linking objects for static librsslVA ..."
							)
	else()
		set_target_properties( librssl PROPERTIES PREFIX "" )

		file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/librsslVA_tmp.arch")
		add_custom_command(TARGET librsslVA POST_BUILD
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:librsslVA_tmp>
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:LibXml2::LibXml2>
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:CJSON::CJSON>
							${l8w8jwt_command}
							COMMAND ${CMAKE_AR} -qcs $<TARGET_FILE:librsslVA> *.o
							WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/librsslVA_tmp.arch
							DEPENDS $<TARGET_FILE:librsslVA_tmp>
									$<TARGET_FILE:LibXml2::LibXml2>
									$<TARGET_FILE:CJSON::CJSON>
									${l8w8jwt_target}
							COMMENT "Archiving objects for librsslVA ..."
							)

	endif()
	unset(_rsslVA_tmp_dir)
                    
	#
	##          -- Build librssl SHARED LIBRARY --

    add_library( librsslVA_shared SHARED
                    ${rsslVAWinSrcFiles} ${rsslVASrcFiles} ${sharedVAVersionSrc})


    target_include_directories(librsslVA_shared
										PRIVATE
											Watchlist
											Util
											TunnelStream
											../Codec
											../Util/Include
											../Transport
											.
										PUBLIC
											${Eta_SOURCE_DIR}/Include/Reactor
											$<TARGET_PROPERTY:librssl_shared,INTERFACE_INCLUDE_DIRECTORIES>
                    )

    set_target_properties( librsslVA librsslVA_shared
                            PROPERTIES 
                                PREFIX "" 
                                OUTPUT_NAME librsslVA
                            )

    if (CMAKE_HOST_WIN32)

        target_compile_options( librsslVA_shared 
                                        PRIVATE 
                                            "/D RSSL_VA_EXPORTS" 
                                            ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                            ${RCDEV_TYPE_CHECK_FLAG}
                                            ${RTSDK_EXTRA_FLAGS}
                                            $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)

		target_link_libraries( librsslVA_shared 
										librssl_shared 
										LibXml2::LibXml2
										CJSON::CJSON
										wininet.lib
										ws2_32.lib
										${l8w8jwt_lib}
							)

         set_target_properties( librsslVA_shared 
                                PROPERTIES 
                                    OUTPUT_NAME librsslVA
                                    RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                                    RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                    PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
                                        ${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                                    PDB_OUTPUT_DIRECTORY_RELEASE_MD 
                                        ${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                    ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD 
                                        ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                                    ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD 
                                        ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
                            )

    else()
		target_compile_options( librsslVA_shared 
                                        PRIVATE 
                                            ${RTSDK_EXTRA_FLAGS}
                           )
        set_target_properties( librsslVA_shared 
                                PROPERTIES 
                                    VERSION ${Eta_VERSION}
                                    SOVERSION ${librsslVA_SO_VERSION}
                            )

		target_link_libraries( librsslVA_shared librssl_shared LibXml2::LibXml2 CJSON::CJSON ${l8w8jwt_lib})
	endif()
DEBUG_PRINT(librsslVA_tmp)
	rcdev_add_target(rtsdk librsslVA librsslVA_shared)

endif()

DEBUG_PRINT(librsslVA)
DEBUG_PRINT(librsslVA_shared)

