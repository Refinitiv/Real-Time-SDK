
if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)
    include(prebuiltLibrary)
	if(CMAKE_HOST_WIN32)
		set(libema_compile_opts "/D __EMA_STATIC_BUILD__" "/D __EMA_COPY_ON_SET__" )
	else()
		set(libema_compile_opts "-D__EMA_STATIC_BUILD__" "-D__EMA_COPY_ON_SET__" )
	endif()
    wrap_prebuilt_library(libema 
                          LIBNAME libema 
                          LIBTYPE STATIC
                          LANGUAGE CXX
                          LINKDEPENDS librsslVA libxml2 
                          PATHROOT   ${Ema_SOURCE_DIR}/Libs
                          LIBHEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Include
                                     ${Ema_SOURCE_DIR}/Src/Domain/Login/Include
                                     ${Ema_SOURCE_DIR}/Src/Rdm/Include
                                     ${Ema_SOURCE_DIR}/Src/Include
                                     ${Ema_SOURCE_DIR}/Src
						  COMPILEOPTS ${libema_compile_opts}
                        )

    wrap_prebuilt_library(libema_shared 
                          LIBNAME libema 
                          LIBTYPE SHARED
                          LANGUAGE CXX
                          LINKDEPENDS librsslVA_shared libxml2 
                          PATHROOT   ${Ema_SOURCE_DIR}/Libs
                          LIBHEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Include
                                     ${Ema_SOURCE_DIR}/Src/Domain/Login/Include
                                     ${Ema_SOURCE_DIR}/Src/Rdm/Include
                                     ${Ema_SOURCE_DIR}/Src/Include
                                     ${Ema_SOURCE_DIR}/Src
                        )
else()

    set(EmaSrcFiles
            Impl/AckMsg.cpp Impl/Attrib.cpp
            Impl/AckMsgDecoder.cpp Impl/AckMsgDecoder.h
            Impl/AckMsgEncoder.cpp Impl/AckMsgEncoder.h
            Impl/ActiveConfig.cpp Impl/ActiveConfig.h
            # Impl/C....
            Impl/ChannelCallbackClient.cpp Impl/ChannelCallbackClient.h
			Impl/ChannelInformation.cpp
            Impl/ClientSession.cpp Impl/ClientSession.h
            Impl/ComplexType.cpp Impl/ConfigErrorHandling.h
            # Impl/D....
            Impl/Data.cpp Impl/DataType.cpp
            Impl/DateTimeStringFormat.cpp
            Impl/Decoder.cpp Impl/Decoder.h
            Impl/DefaultXML.h
            Impl/DictionaryCallbackClient.cpp Impl/DictionaryCallbackClient.h
            Impl/DictionaryHandler.cpp Impl/DictionaryHandler.h
            Impl/DirectoryCallbackClient.cpp Impl/DirectoryCallbackClient.h
            Impl/DirectoryHandler.cpp Impl/DirectoryHandler.h
            Impl/DirectoryServiceStore.cpp Impl/DirectoryServiceStore.h
            # Impl/E....
            Impl/ElementEntry.cpp Impl/ElementList.cpp
            Impl/ElementListDecoder.cpp Impl/ElementListDecoder.h
            Impl/ElementListEncoder.cpp Impl/ElementListEncoder.h
            Impl/ElementListSetDef.cpp Impl/ElementListSetDef.h
            Impl/EmaBuffer.cpp
            Impl/EmaBufferInt.cpp Impl/EmaBufferInt.h
            Impl/EmaBufferU16.cpp Impl/EmaBufferU16Int.cpp Impl/EmaBufferU16Int.h
            Impl/EmaConfigImpl.cpp Impl/EmaConfigImpl.h
            Impl/EmaList.h
            Impl/EmaPool.cpp Impl/EmaPool.h
            Impl/EmaStaticCombined.cpp
            Impl/EmaString.cpp Impl/EmaStringInt.cpp Impl/EmaStringInt.h
            Impl/EncodeIterator.cpp Impl/EncodeIterator.h
            Impl/Encoder.cpp Impl/Encoder.h
            Impl/ErrorClientHandler.cpp Impl/ErrorClientHandler.h
            Impl/ExceptionTranslator.cpp Impl/ExceptionTranslator.h
            # Impl/F,G...
            Impl/FieldEntry.cpp Impl/FieldList.cpp
            Impl/FieldListDecoder.cpp Impl/FieldListDecoder.h
            Impl/FieldListEncoder.cpp Impl/FieldListEncoder.h
            Impl/FieldListSetDef.cpp Impl/FieldListSetDef.h
            Impl/FilterEntry.cpp Impl/FilterList.cpp
            Impl/FilterListDecoder.cpp Impl/FilterListDecoder.h
            Impl/FilterListEncoder.cpp Impl/FilterListEncoder.h
            Impl/GenericMsg.cpp Impl/GenericMsgDecoder.cpp Impl/GenericMsgDecoder.h
            Impl/GenericMsgEncoder.cpp Impl/GenericMsgEncoder.h
            Impl/GetTime.cpp Impl/GetTime.h
            Impl/GlobalPool.h
            # Impl/H,I,K,L
            Impl/HashTable.cpp Impl/HashTable.h
            Impl/ItemCallbackClient.cpp Impl/ItemCallbackClient.h
            Impl/ItemInfo.cpp Impl/ItemInfo.h
            Impl/Key.cpp
            Impl/LoginCallbackClient.cpp Impl/LoginCallbackClient.h
            Impl/LoginHandler.cpp Impl/LoginHandler.h
            # Impl/M,N...
            Impl/Map.cpp
            Impl/MapDecoder.cpp Impl/MapDecoder.h
            Impl/MapEncoder.cpp Impl/MapEncoder.h
            Impl/MapEntry.cpp
            Impl/MarketItemHandler.cpp Impl/MarketItemHandler.h
            Impl/Msg.cpp
            Impl/MsgDecoder.cpp Impl/MsgDecoder.h
            Impl/MsgEncoder.cpp Impl/MsgEncoder.h
            Impl/Mutex.cpp Impl/Mutex.h
            Impl/NoDataImpl.cpp Impl/NoDataImpl.h
            # Impl/O....
            Impl/OmmAnsiPage.cpp
            Impl/OmmAnsiPageDecoder.cpp Impl/OmmAnsiPageDecoder.h
            Impl/OmmAnsiPageEncoder.cpp Impl/OmmAnsiPageEncoder.h
            Impl/OmmArray.cpp Impl/OmmArrayDecoder.cpp Impl/OmmArrayDecoder.h
            Impl/OmmArrayEncoder.cpp Impl/OmmArrayEncoder.h
            Impl/OmmArrayEntry.cpp
            Impl/OmmAscii.cpp Impl/OmmAsciiDecoder.cpp Impl/OmmAsciiDecoder.h
            Impl/OmmBaseImpl.cpp Impl/OmmBaseImpl.h Impl/OmmBaseImplMap.h
            Impl/OmmBuffer.cpp Impl/OmmBufferDecoder.cpp Impl/OmmBufferDecoder.h
			Impl/OmmCommonImpl.cpp
            Impl/OmmConsumer.cpp Impl/OmmConsumerActiveConfig.cpp Impl/OmmConsumerActiveConfig.h
            Impl/OmmConsumerClient.cpp
            Impl/OmmConsumerConfig.cpp Impl/OmmConsumerConfigImpl.cpp Impl/OmmConsumerConfigImpl.h
			Impl/OmmConsumerErrorClient.cpp
            Impl/OmmConsumerEvent.cpp Impl/OmmConsumerImpl.cpp Impl/OmmConsumerImpl.h
            Impl/OmmDate.cpp Impl/OmmDateDecoder.cpp Impl/OmmDateDecoder.h
            Impl/OmmDateTime.cpp Impl/OmmDateTimeDecoder.cpp Impl/OmmDateTimeDecoder.h
            Impl/OmmDouble.cpp Impl/OmmDoubleDecoder.cpp Impl/OmmDoubleDecoder.h
            Impl/OmmEnum.cpp Impl/OmmEnumDecoder.cpp Impl/OmmEnumDecoder.h
            Impl/OmmError.cpp Impl/OmmErrorDecoder.cpp Impl/OmmErrorDecoder.h
            Impl/OmmException.cpp
            Impl/OmmFloat.cpp Impl/OmmFloatDecoder.cpp Impl/OmmFloatDecoder.h
            Impl/OmmInaccessibleLogFileException.cpp
            Impl/OmmInaccessibleLogFileExceptionImpl.cpp
            Impl/OmmInaccessibleLogFileExceptionImpl.h
            Impl/OmmInt.cpp Impl/OmmIntDecoder.cpp Impl/OmmIntDecoder.h
            Impl/OmmInvalidConfigurationException.cpp
            Impl/OmmInvalidConfigurationExceptionImpl.cpp
            Impl/OmmInvalidConfigurationExceptionImpl.h
            Impl/OmmInvalidHandleException.cpp
            Impl/OmmInvalidHandleExceptionImpl.cpp Impl/OmmInvalidHandleExceptionImpl.h
            Impl/OmmInvalidUsageException.cpp
            Impl/OmmInvalidUsageExceptionImpl.cpp Impl/OmmInvalidUsageExceptionImpl.h
            Impl/OmmIProviderActiveConfig.cpp Impl/OmmIProviderActiveConfig.h
            Impl/OmmIProviderConfig.cpp
            Impl/OmmIProviderConfigImpl.cpp Impl/OmmIProviderConfigImpl.h
            Impl/OmmIProviderImpl.cpp Impl/OmmIProviderImpl.h
            Impl/OmmLoggerClient.cpp Impl/OmmLoggerClient.h
            Impl/OmmMemoryExhaustionException.cpp
            Impl/OmmMemoryExhaustionExceptionImpl.cpp Impl/OmmMemoryExhaustionExceptionImpl.h
            Impl/OmmNiProviderActiveConfig.cpp Impl/OmmNiProviderActiveConfig.h
            Impl/OmmNiProviderConfig.cpp
            Impl/OmmNiProviderConfigImpl.cpp Impl/OmmNiProviderConfigImpl.h
            Impl/OmmNiProviderImpl.cpp Impl/OmmNiProviderImpl.h
            Impl/OmmNonRwfEncoder.cpp Impl/OmmNonRwfEncoder.h
            Impl/OmmOpaque.cpp Impl/OmmOpaqueDecoder.cpp Impl/OmmOpaqueDecoder.h
            Impl/OmmOpaqueEncoder.cpp Impl/OmmOpaqueEncoder.h
            Impl/OmmOutOfRangeException.cpp
            Impl/OmmOutOfRangeExceptionImpl.cpp Impl/OmmOutOfRangeExceptionImpl.h
            Impl/OmmProvider.cpp Impl/OmmProviderClient.cpp
            Impl/OmmProviderConfig.cpp 
			Impl/OmmProviderErrorClient.cpp
			Impl/OmmProviderEvent.cpp
            Impl/OmmProviderImpl.cpp Impl/OmmProviderImpl.h
            Impl/OmmQos.cpp Impl/OmmQosDecoder.cpp Impl/OmmQosDecoder.h
            Impl/OmmReal.cpp Impl/OmmRealDecoder.cpp Impl/OmmRealDecoder.h
            Impl/OmmRmtes.cpp Impl/OmmRmtesDecoder.cpp Impl/OmmRmtesDecoder.h
            Impl/OmmServerBaseImpl.cpp Impl/OmmServerBaseImpl.h
            Impl/OmmState.cpp Impl/OmmStateDecoder.cpp Impl/OmmStateDecoder.h
            Impl/OmmSystemException.cpp
            Impl/OmmSystemExceptionImpl.cpp Impl/OmmSystemExceptionImpl.h
            Impl/OmmTime.cpp Impl/OmmTimeDecoder.cpp Impl/OmmTimeDecoder.h
            Impl/OmmUInt.cpp Impl/OmmUIntDecoder.cpp Impl/OmmUIntDecoder.h
            Impl/OmmUnsupportedDomainTypeException.cpp
            Impl/OmmUnsupportedDomainTypeExceptionImpl.cpp
            Impl/OmmUnsupportedDomainTypeExceptionImpl.h
            Impl/OmmUtf8.cpp Impl/OmmUtf8Decoder.cpp Impl/OmmUtf8Decoder.h
            Impl/OmmXml.cpp Impl/OmmXmlDecoder.cpp Impl/OmmXmlDecoder.h
            Impl/OmmXmlEncoder.cpp Impl/OmmXmlEncoder.h
            # Impl/R,P...
            Impl/Payload.cpp
            Impl/Pipe.cpp Impl/Pipe.h
            Impl/PostMsg.cpp Impl/PostMsgDecoder.cpp Impl/PostMsgDecoder.h
            Impl/PostMsgEncoder.cpp Impl/PostMsgEncoder.h
            Impl/ProgrammaticConfigure.cpp Impl/ProgrammaticConfigure.h
            Impl/RefreshMsg.cpp Impl/RefreshMsgDecoder.cpp Impl/RefreshMsgDecoder.h
            Impl/RefreshMsgEncoder.cpp Impl/RefreshMsgEncoder.h
            Impl/ReqMsg.cpp Impl/ReqMsgDecoder.cpp Impl/ReqMsgDecoder.h
            Impl/ReqMsgEncoder.cpp Impl/ReqMsgEncoder.h
            Impl/RmtesBuffer.cpp Impl/RmtesBufferImpl.cpp Impl/RmtesBufferImpl.h
            # Impl/S....
            Impl/Series.cpp Impl/SeriesDecoder.cpp Impl/SeriesDecoder.h
            Impl/SeriesEncoder.cpp Impl/SeriesEncoder.h
            Impl/SeriesEntry.cpp
            Impl/ServerChannelHandler.cpp Impl/ServerChannelHandler.h
            Impl/StaticDecoder.cpp Impl/StaticDecoder.h
            Impl/StatusMsg.cpp Impl/StatusMsgDecoder.cpp Impl/StatusMsgDecoder.h
            Impl/StatusMsgEncoder.cpp Impl/StatusMsgEncoder.h
            Impl/StreamId.cpp Impl/StreamId.h
            Impl/SummaryData.cpp
            # Impl/T,U,V...
            Impl/Thread.cpp Impl/Thread.h
            Impl/TimeOut.cpp Impl/TimeOut.h
            Impl/TunnelStreamLoginReqMsgImpl.cpp Impl/TunnelStreamLoginReqMsgImpl.h
            Impl/TunnelStreamRequest.cpp
            Impl/UpdateMsg.cpp Impl/UpdateMsgDecoder.cpp Impl/UpdateMsgDecoder.h
            Impl/UpdateMsgEncoder.cpp Impl/UpdateMsgEncoder.h
            Impl/Utilities.cpp Impl/Utilities.h
            Impl/Vector.cpp Impl/VectorDecoder.cpp Impl/VectorDecoder.h
            Impl/VectorEncoder.cpp Impl/VectorEncoder.h
            Impl/VectorEntry.cpp
            # EMA Version Files
            EmaVersion.c EmaVersion.h
            # Include/
            Include/AckMsg.h
            Include/Attrib.h
            Include/Common.h
            Include/ComplexType.h
            Include/Data.h
            Include/DataType.h
            Include/DateTimeStringFormat.h
            Include/ElementEntry.h
            Include/ElementList.h
            Include/EmaBuffer.h
            Include/EmaBufferU16.h
            Include/EmaString.h
            Include/EmaVector.h
            Include/FieldEntry.h
            Include/FieldList.h
            Include/FilterEntry.h
            Include/FilterList.h
            Include/GenericMsg.h
            Include/Key.h
            Include/Map.h
            Include/MapEntry.h
            Include/Msg.h
            Include/OmmAnsiPage.h
            Include/OmmArray.h
            Include/OmmArrayEntry.h
            Include/OmmAscii.h
            Include/OmmBuffer.h
            Include/OmmConsumer.h
            Include/OmmConsumerClient.h
            Include/OmmConsumerConfig.h
            Include/OmmConsumerErrorClient.h
            Include/OmmConsumerEvent.h
            Include/OmmDate.h
            Include/OmmDateTime.h
            Include/OmmDouble.h
            Include/OmmEnum.h
            Include/OmmError.h
            Include/OmmException.h
            Include/OmmFloat.h
            Include/OmmInaccessibleLogFileException.h
            Include/OmmInt.h
            Include/OmmInvalidConfigurationException.h
            Include/OmmInvalidHandleException.h
            Include/OmmInvalidUsageException.h
            Include/OmmIProviderConfig.h
            Include/OmmMemoryExhaustionException.h
            Include/OmmNiProviderConfig.h
            Include/OmmOpaque.h
            Include/OmmOutOfRangeException.h
            Include/OmmProvider.h
            Include/OmmProviderClient.h
            Include/OmmProviderConfig.h
            Include/OmmProviderErrorClient.h
            Include/OmmProviderEvent.h
            Include/OmmQos.h
            Include/OmmReal.h
            Include/OmmRmtes.h
            Include/OmmState.h
            Include/OmmSystemException.h
            Include/OmmTime.h
            Include/OmmUInt.h
            Include/OmmUnsupportedDomainTypeException.h
            Include/OmmUtf8.h
            Include/OmmXml.h
            Include/Payload.h
            Include/PostMsg.h
            Include/RefreshMsg.h
            Include/ReqMsg.h
            Include/RmtesBuffer.h
            Include/Series.h
            Include/SeriesEntry.h
            Include/StatusMsg.h
            Include/SummaryData.h
            Include/TunnelStreamRequest.h
            Include/UpdateMsg.h
            Include/Vector.h
            Include/VectorEntry.h
            #Domain
            ../Domain/Login/Impl/LoginRefresh.cpp
            ../Domain/Login/Impl/LoginRefreshImpl.cpp
            ../Domain/Login/Impl/LoginRefreshImpl.h
            ../Domain/Login/Impl/LoginReq.cpp
            ../Domain/Login/Impl/LoginReqImpl.cpp
            ../Domain/Login/Impl/LoginReqImpl.h
            ../Domain/Login/Impl/LoginStatus.cpp
            ../Domain/Login/Impl/LoginStatusImpl.cpp
            ../Domain/Login/Impl/LoginStatusImpl.h
            ../Domain/Login/Include/Login.h
            #RDM
            ../Rdm/Impl/DataDictionary.cpp
            ../Rdm/Impl/DataDictionaryImpl.cpp
            ../Rdm/Impl/DataDictionaryImpl.h
            ../Rdm/Impl/DictionaryEntry.cpp
            ../Rdm/Impl/DictionaryEntryImpl.cpp
            ../Rdm/Impl/DictionaryEntryImpl.h
            ../Rdm/Impl/DictionaryUtility.cpp
            ../Rdm/Impl/EnumType.cpp
            ../Rdm/Impl/EnumTypeImpl.cpp
            ../Rdm/Impl/EnumTypeImpl.h
            ../Rdm/Impl/EnumTypeTable.cpp
            ../Rdm/Impl/EnumTypeTableImpl.cpp
            ../Rdm/Impl/EnumTypeTableImpl.h
            ../Rdm/Impl/RdmUtilities.cpp
            ../Rdm/Impl/RdmUtilities.h
            ../Rdm/Include/DataDictionary.h
            ../Rdm/Include/DictionaryEntry.h
            ../Rdm/Include/DictionaryUtility.h
            ../Rdm/Include/EmaRdm.h
            ../Rdm/Include/EnumType.h
            ../Rdm/Include/EnumTypeTable.h
            ../Rdm/Include/MfFieldType.h
            #Include
            ../Include/Ema.h
            
    )

    if ( CMAKE_HOST_WIN32 )
        set(EmaWinRcFiles
            Ema.rc
            )
    endif()

    add_library( libema STATIC ${EmaSrcFiles})
    target_include_directories(libema
                                PUBLIC
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Domain/Login/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Rdm/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src>
								PRIVATE
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Impl>
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Rdm/Impl>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Domain/Login/Impl>
                                )
    target_link_libraries( libema 
                                librsslVA 
                                libxml2 
                            )

    add_library( libema_shared SHARED ${EmaSrcFiles} ${EmaWinRcFiles} )
    target_include_directories(libema_shared
                                PUBLIC
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Domain/Login/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Rdm/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Include>
                                    $<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src>
								PRIVATE
									$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Impl>
                                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Rdm/Impl>
									$<BUILD_INTERFACE:${Ema_SOURCE_DIR}/Src/Domain/Login/Impl>
                                )
    target_link_libraries( libema_shared librsslVA_shared libxml2 )

    if ( CMAKE_HOST_WIN32 )

        target_compile_options( libema 
                                    PUBLIC 
                                        "/D __EMA_STATIC_BUILD__" 
                                        "/D __EMA_COPY_ON_SET__" 
                                    PRIVATE
									    ${RCDEV_DEBUG_TYPE_FLAGS_STATIC}
                                )

        target_compile_options( libema_shared 
                                    PRIVATE 
                                        "/D EMA_LIBRARY_EXPORTS" 
                                        "/D __EMA_COPY_ON_SET__"	 
                                        ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                        ${RCDEV_TYPE_CHECK_FLAG}
                                        $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
                                )
        set_target_properties( libema_shared 
                            PROPERTIES 
                                OUTPUT_NAME "libema"
                                RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                                RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD 
                                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared
                                ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD 
                                    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD}/Shared
                                PDB_OUTPUT_DIRECTORY_RELEASE_MD 
                                    ${CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE_MD}/Shared 
                                PDB_OUTPUT_DIRECTORY_DEBUG_MDD 
                                    ${CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG_MDD}/Shared 
                        )
        target_link_libraries(libema_shared wininet.lib ws2_32.lib)

    else()

        set_target_properties ( libema libema_shared
                                PROPERTIES 
                                    PREFIX 		"" 
                                    OUTPUT_NAME "libema" 
                            )
        target_compile_options( libema 
                                PUBLIC 
                                    "-D__EMA_STATIC_BUILD__"
                                    "-D__EMA_COPY_ON_SET__")

        set_target_properties( libema_shared PROPERTIES DEFINE_SYMBOL "" ) # avoid -Dlibema_EXPORTS
        set_target_properties( libema_shared 
                                PROPERTIES 
                                    VERSION 
                                        ${Ema_VERSION} 
                                    SOVERSION 
                                        ${libema_SO_VERSION} 
                            )
        target_compile_options( libema_shared 
                                PUBLIC 
                                    "-D__EMA_COPY_ON_SET__")
    endif()
endif()


