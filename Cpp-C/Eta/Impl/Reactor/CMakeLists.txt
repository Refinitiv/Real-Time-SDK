project(Reactor)

if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)
    include(prebuiltLibrary)
    wrap_prebuilt_library(librsslVA
                          LIBNAME librsslVA
                          LIBTYPE STATIC
                          LANGUAGE C
                          LINKDEPENDS librssl
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/Reactor
                                     ${Eta_SOURCE_DIR}/Include/Util
                                     ${Eta_SOURCE_DIR}/Include/RDM
                        )

    wrap_prebuilt_library(librsslVA_shared
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
		${Eta_SOURCE_DIR}/Include/rsslVersion.h
    )

    if (CMAKE_HOST_WIN32)
        set(rsslVAWinSrcFiles
            ${Eta_SOURCE_DIR}/Impl/Util/DllVAMain.c
            ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslVAExports.h
            ${Eta_SOURCE_DIR}/Include/rsslVA.rc
            ${Eta_SOURCE_DIR}/Include/rsslResource.h
        )
    endif()

    add_library( librsslVA STATIC 
                ${rsslVASrcFiles} rsslVAVersionStatic.c )
    target_link_libraries(librsslVA librssl)

    target_include_directories(librsslVA
                    PRIVATE
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Watchlist>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Util>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/TunnelStream>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Codec>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Transport>
                    PUBLIC
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Reactor>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Util>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/RDM>
                    )

                    
    add_library( librsslVA_shared SHARED
                    ${rsslVAWinSrcFiles} 
                    ${rsslVASrcFiles}
					rsslVAVersionShared.c
                 )
    target_link_libraries( librsslVA_shared librssl_shared )

    target_include_directories(librsslVA_shared
                    PRIVATE
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Watchlist>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Util>
                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/TunnelStream>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/Util/Include>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Codec>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Transport>
                    PUBLIC
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Reactor>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Util>
                        $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/RDM>
                    )

    set_target_properties( librsslVA librsslVA_shared
                            PROPERTIES 
                                PREFIX "" 
                                OUTPUT_NAME librsslVA
                            )

    if (CMAKE_HOST_WIN32)

        target_compile_options(librsslVA PRIVATE ${RCDEV_DEBUG_TYPE_FLAGS_STATIC})

        target_compile_options( librsslVA_shared 
                                        PRIVATE 
                                            "/D RSSL_VA_EXPORTS" 
                                            ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                            ${RCDEV_TYPE_CHECK_FLAG}
                                            $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>)

        target_link_libraries( librsslVA_shared
                                       wininet.lib
                                       ws2_32.lib
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

        set_target_properties( librsslVA_shared 
                                PROPERTIES 
                                    VERSION ${Eta_VERSION}
                                    SOVERSION ${librsslVA_SO_VERSION}
                            )
    endif()
endif()

