
if( CMAKE_HOST_UNIX )
    ENABLE_LANGUAGE(ASM-ATT)
endif()

if ( CMAKE_HOST_WIN32 )
    ENABLE_LANGUAGE(ASM_MASM)
endif()

if (BUILD_WITH_PREBUILT_ETA_EMA_LIBRARIES)
	
	if(CMAKE_HOST_WIN32)
		set(librssl_shared_compile_opts "/D RSSL_IMPORTS")
	endif()
	
    rcdev_wrap_prebuilt_library(librssl 
                          LIBNAME librssl 
                          LIBTYPE STATIC
                          LANGUAGE C
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/Codec
                                     ${Eta_SOURCE_DIR}/Include/Util
                                     ${Eta_SOURCE_DIR}/Include/Transport
									 ${Eta_SOURCE_DIR}/Impl/Util/Include
									 ${Eta_BINARY_DIR}/Include
                        )

    rcdev_wrap_prebuilt_library(librssl_shared 
                          LIBNAME librssl 
                          LIBTYPE SHARED
                          LANGUAGE C
                          PATHROOT   ${Eta_SOURCE_DIR}/Libs
                          LIBHEADERS ${Eta_SOURCE_DIR}/Include/Codec
                                     ${Eta_SOURCE_DIR}/Include/Util
                                     ${Eta_SOURCE_DIR}/Include/Transport
									 ${Eta_SOURCE_DIR}/Impl/Util/Include
									 ${Eta_BINARY_DIR}/Include
						  COMPILEOPTS ${librssl_shared_compile_opts}
                        )
else()

    set(rsslSrcFiles 
                #Codec source files
                arrayDecoder.c arrayEncoder.c codes.c
                dataDictionary.c dataTypes.c dataUtils.c
                dtime.c elemListDecoder.c elemListEncoder.c
                fieldListDecoder.c fieldListEncoder.c filterListDecoder.c
                filterListEncoder.c mapDecoder.c mapEncoder.c
                numeric.c primitiveDecoders.c primitiveEncoders.c
                rsslCharSet.c rsslcnvtab.c rsslRmtes.c
                rwfConvert.c seriesDecoder.c seriesEncoder.c
                setDictionary.c state.c vectorDecoder.c
                vectorEncoder.c messageDecoders.c messageEncoders.c
                msgs.c decodeRoutines.c dictionary.c
                fieldListDefs.c xmlDomainDump.c xmlDump.c xmlMsgDump.c
				fileUtil.c
                
                #Codec internal headers
                rtr/decoderTools.h
                rtr/encoderTools.h
                rtr/intDataTypes.h
                rtr/retmacros.h
                rtr/rsslCharSet.h
                rtr/rsslcnvtab.h
                rtr/rsslIteratorUtilsInt.h
                rtr/rsslMessageInt.h
                rtr/rwfConvert.h

                #Transport source files
                ${Eta_SOURCE_DIR}/Impl/Transport/ripccomp.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rwsutils.c
                ${Eta_SOURCE_DIR}/Impl/Transport/ripchttp.c
                ${Eta_SOURCE_DIR}/Impl/Transport/ripcssldh.c
                ${Eta_SOURCE_DIR}/Impl/Transport/ripcsslutils.c
                ${Eta_SOURCE_DIR}/Impl/Transport/ripcutils.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rsslImpl.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rsslSeqMcastTransportImpl.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rsslSocketTransportImpl.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rsslWebSocketTransportImpl.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rsslUniShMemTransportImpl.c
                ${Eta_SOURCE_DIR}/Impl/Transport/shmemtrans.c
				${Eta_SOURCE_DIR}/Impl/Util/rsslCurlJIT.c

               
                
                #Transport internal headers
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripc_int.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripc_internal.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcdatadef.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcflip.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripch.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripchttp.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rwsutils.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcinetutils.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcplat.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcssljit.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcsslutils.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcutils.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslAlloc.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslChanManagement.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslErrors.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslLoadInitTransport.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslSeqMcastTransport.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslSeqMcastTransportImpl.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslSocketTransport.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslSocketTransportImpl.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslUniShMemTransport.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslUniShMemTransportImpl.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/shmemtrans.h
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/rsslpipe.h

                #Utils source files
                ${Eta_SOURCE_DIR}/Impl/Util/cutildfltcbuffer.c
                ${Eta_SOURCE_DIR}/Impl/Util/cutilsmplcbuffer.c
                ${Eta_SOURCE_DIR}/Impl/Util/rsslHashFuncs.c
                ${Eta_SOURCE_DIR}/Impl/Util/rsslHashTable.c
                ${Eta_SOURCE_DIR}/Impl/Util/rsslMemoryBuffer.c
                ${Eta_SOURCE_DIR}/Impl/Util/rsslNotifier.c
                ${Eta_SOURCE_DIR}/Impl/Util/shmem.c
                ${Eta_SOURCE_DIR}/Impl/Util/shmemcirbuf.c
                ${Eta_SOURCE_DIR}/Impl/Util/tr_sha_1.c
                ${Eta_SOURCE_DIR}/Impl/Util/tr_sl1_64.c
				${Eta_SOURCE_DIR}/Impl/Util/rsslGetTime.c

                #Utils header files
                ${Eta_SOURCE_DIR}/Include/Util/rtr/os.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslErrorInfo.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslEventSignal.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslHashFuncs.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslHashTable.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslHeapBuffer.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslMemoryBuffer.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslMemoryPool.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslNotifier.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslQueue.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslThread.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslTypes.h
                ${Eta_SOURCE_DIR}/Include/Util/rtr/rsslVAUtils.h
				${Eta_SOURCE_DIR}/Include/Util/rtr/rsslCurlJIT.h
				${Eta_SOURCE_DIR}/Include/Util/rtr/rsslGetTime.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/application_signing.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/byteswap.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/custmem.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/cutilcbuffer.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/cutildfltcbuffer.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/cutilsmplcbuffer.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/intcustmem.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/platform.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rtdataty.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rtratoi.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rtrdefs.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rtratomic.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rtrdiv10.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rwfNet.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/rwfNetwork.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/shmemcirbuf.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/shmem.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/smplcbuffer.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/socket.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/spinlock.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/textFileReader.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/tr_sha_1.h
                ${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/tr_sl1_64.h
                

                # Etac external Includes
                ${Eta_SOURCE_DIR}/Include/Codec/decodeRoutines.h
                ${Eta_SOURCE_DIR}/Include/Codec/dictionary.h
                ${Eta_SOURCE_DIR}/Include/Codec/fieldListDefs.h
				${Eta_SOURCE_DIR}/Include/Codec/fileUtil.h
                ${Eta_SOURCE_DIR}/Include/Codec/xmlDomainDump.h
                ${Eta_SOURCE_DIR}/Include/Codec/xmlDump.h
                ${Eta_SOURCE_DIR}/Include/Codec/xmlMsgDump.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslAckMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslArray.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslCloseMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataDictionary.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataPackage.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataTypeEnums.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDateTime.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslElementList.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslFieldList.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslFilterList.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslGenericMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslIterators.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMap.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMessagePackage.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMsgBase.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMsgDecoders.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMsgEncoders.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslMsgKey.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslPostMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslPrimitiveDecoders.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslPrimitiveEncoders.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslQos.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslRDM.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslReal.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslRefreshMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslRequestMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslRmtes.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslSeries.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslSetData.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslState.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslStatusMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataUtils.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslRetCodes.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslUpdateMsg.h
                ${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslVector.h
                ${Eta_SOURCE_DIR}/Include/Transport/rtr/rsslTransport.h
                ${Eta_BINARY_DIR}/Include/rsslVersion.h
                )

	set(rsslJsonConverterFiles
		${Eta_SOURCE_DIR}/Impl/Converter/rsslJsonConverter.C
		${Eta_SOURCE_DIR}/Impl/Converter/rwfToJsonConverter.C 
		${Eta_SOURCE_DIR}/Impl/Converter/rwfToJsonBase.C
		${Eta_SOURCE_DIR}/Impl/Converter/jsonToRwfConverter.C
		${Eta_SOURCE_DIR}/Impl/Converter/jsonToRsslMsgDecoder.C
		${Eta_SOURCE_DIR}/Impl/Converter/jsonToRwfSimple.C
		${Eta_SOURCE_DIR}/Impl/Converter/rwfToJsonSimple.C
		${Eta_SOURCE_DIR}/Impl/Converter/jsonToRwfBase.C
		${Eta_SOURCE_DIR}/Impl/Converter/jsmn.C 
		${Eta_SOURCE_DIR}/Impl/Converter/EnumTableDefinition.C

		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsmn.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsonSimpleDefs.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsonToRsslMsgDecoder.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsonToRwfBase.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsonToRwfSimple.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/jsonToRwfConverter.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rwfToJsonBase.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rwfToJsonSimple.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rwfToJsonConverter.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rjcstring.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/EnumTableDefinition.h

		${Eta_SOURCE_DIR}/Include/Converter/rtr/rsslJsonConverter.h

		${Eta_SOURCE_DIR}/Include/Codec/rtr/rsslDataDictionary.h 
		${Eta_SOURCE_DIR}/Include/Util/rtr/rsslTypes.h
		${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/custmem.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rjchexdmp.h
		${Eta_SOURCE_DIR}/Impl/Converter/Include/rtr/rjcbuffer.h
		${Eta_SOURCE_DIR}/Impl/Converter/rjcstring.C
		${Eta_SOURCE_DIR}/Impl/Converter/rjchexdmp.C
		${Eta_SOURCE_DIR}/Impl/Converter/rjcbuffer.C
	)

    if (CMAKE_HOST_WIN32)
        set(rsslWinInetSrcFiles 
                ${Eta_SOURCE_DIR}/Impl/Transport/ripcinetutils.c
                ${Eta_SOURCE_DIR}/Impl/Transport/rtr/ripcinetutils.h
            )
        set(rsslWinRCFiles
                ${Eta_SOURCE_DIR}/Include/rssl.rc
                ${Eta_SOURCE_DIR}/Include/rsslResource.h
            )
		if(BUILD_ETA_CPU_BIND)
			set(rsslWinBindThread
	                ${Eta_SOURCE_DIR}/Impl/Util/rsslBindThread.c
					${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/bindthread.h
					${Eta_SOURCE_DIR}/Include/Util/rtr/rsslBindThread.h

					${Eta_SOURCE_DIR}/Impl/Util/cpu_topo.c
	                ${Eta_SOURCE_DIR}/Impl/Util/Include/eta/cputopology.h
					${Eta_SOURCE_DIR}/Impl/Util/util_os.c


					${Eta_SOURCE_DIR}/Impl/Util/get_cpuid.asm
				)
		endif()

    else()
		set(staticVersionSrc	rsslVersionStatic.c)
		set(sharedVersionSrc	rsslVersionShared.c)

		if(BUILD_ETA_CPU_BIND)
			set(rsslLinuxBindThread
	                ${Eta_SOURCE_DIR}/Impl/Util/rsslBindThread.c
					${Eta_SOURCE_DIR}/Impl/Util/Include/rtr/bindthread.h
					${Eta_SOURCE_DIR}/Include/Util/rtr/rsslBindThread.h

					${Eta_SOURCE_DIR}/Impl/Util/cpu_topo.c
	                ${Eta_SOURCE_DIR}/Impl/Util/Include/eta/cputopology.h
					${Eta_SOURCE_DIR}/Impl/Util/util_os.c
				)
			set(rsslLnx64Asm
					${Eta_SOURCE_DIR}/Impl/Util/get_cpuid_lix64.s
				)
			add_library(rssl_asm OBJECT ${rsslLnx64Asm})
		endif()
    endif()

	#
	##          -- Build librssl STATIC LIBRARY --

	set(_rssl_tmp_dir "${CMAKE_CURRENT_BINARY_DIR}/librssl_tmp.arch")
	if (NOT EXISTS "${_rssl_tmp_dir}/src")
		file(MAKE_DIRECTORY "${_rssl_tmp_dir}/src")
	endif()

    if (CMAKE_HOST_WIN32)
		if(BUILD_ETA_CPU_BIND)
			add_library( librssl_tmp STATIC 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${staticVersionSrc} ${rsslJsonConverterFiles} ${rsslWinBindThread} )
		else()
			add_library( librssl_tmp STATIC 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${staticVersionSrc} ${rsslJsonConverterFiles} )
		endif()
    else()
		if(BUILD_ETA_CPU_BIND)
			add_library( librssl_tmp STATIC 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${staticVersionSrc} ${rsslJsonConverterFiles} ${rsslLinuxBindThread} $<TARGET_OBJECTS:rssl_asm> )
		else()
			add_library( librssl_tmp STATIC 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${staticVersionSrc} ${rsslJsonConverterFiles} )
		endif()
    endif()

    if (CMAKE_HOST_WIN32)
		set_target_properties(librssl_tmp PROPERTIES 
											ARCHIVE_OUTPUT_DIRECTORY_DEBUG_MDD
												"${_rssl_tmp_dir}/lib/Debug_MDd"
											ARCHIVE_OUTPUT_DIRECTORY_RELEASE_MD
												"${_rssl_tmp_dir}/lib/Release_MD"
								)
		target_compile_options(librssl_tmp 
						PRIVATE 
							${RCDEV_DEBUG_TYPE_FLAGS_STATIC} 
                            ${RTSDK_EXTRA_FLAGS}
							"/D REAL_SHA1_INIT")

	else()
		set_target_properties(librssl_tmp PROPERTIES 
											ARCHIVE_OUTPUT_DIRECTORY
												"${_rssl_tmp_dir}/lib"
								)
		target_compile_options(librssl_tmp 
						PRIVATE
                            ${RTSDK_EXTRA_FLAGS}
							"-DREAL_SHA1_INIT")
	endif()

    target_include_directories(librssl_tmp
										PRIVATE
											../Transport
											${Eta_SOURCE_DIR}/Impl/Util/Include
											${Eta_SOURCE_DIR}/Impl/Converter/Include

										PUBLIC
											../Util/Include
											${Eta_SOURCE_DIR}/Include/RDM
											${Eta_SOURCE_DIR}/Include/Codec
											${Eta_SOURCE_DIR}/Include/Util
											${Eta_SOURCE_DIR}/Include/Transport
											${Eta_SOURCE_DIR}/Include/Converter
											${Eta_SOURCE_DIR}/Include
											${Eta_BINARY_DIR}/Include
											.
								)

    if (CMAKE_HOST_WIN32)
		target_link_libraries( librssl_tmp wininet.lib ws2_32.lib crypt32.lib cryptui.lib ZLIB::ZLIB LZ4::LZ4 Iphlpapi.lib )
	else()
		target_link_libraries( librssl_tmp ZLIB::ZLIB LZ4::LZ4 )
	endif()


	file(WRITE ${_rssl_tmp_dir}/src/librssl_tmp.C "void librsslArchive() {}\n")
    add_library(librssl STATIC ${_rssl_tmp_dir}/src/librssl_tmp.C)

    target_include_directories(librssl PUBLIC
										$<TARGET_PROPERTY:librssl_tmp,INTERFACE_INCLUDE_DIRECTORIES>
									)

	set_target_properties(librssl PROPERTIES OUTPUT_NAME "librssl")

	target_link_libraries(librssl librssl_tmp)

    if (CMAKE_HOST_WIN32)
		add_custom_command(TARGET librssl POST_BUILD
							COMMAND ${CMAKE_LINKER} /LIB 
													/OUT:"$<TARGET_FILE:librssl>" 
													/LTCG "$<TARGET_FILE:librssl_tmp>"
														  "$<TARGET_FILE:ZLIB::ZLIB>"
														  "$<TARGET_FILE:LZ4::LZ4>"
							DEPENDS $<TARGET_FILE:librssl_tmp> 
									$<TARGET_FILE:ZLIB::ZLIB> 
									$<TARGET_FILE:LZ4::LZ4>
							COMMENT "Linking objects for static librssl ..."
							)
	else()
        set_target_properties( librssl PROPERTIES PREFIX "" )

		file(MAKE_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/librssl_tmp.arch")
		add_custom_command(TARGET librssl POST_BUILD
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:librssl_tmp>
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:ZLIB::ZLIB>
							COMMAND ${CMAKE_AR} -x $<TARGET_FILE:LZ4::LZ4>
							COMMAND ${CMAKE_AR} -qcs $<TARGET_FILE:librssl> *.o
							WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/librssl_tmp.arch
							DEPENDS $<TARGET_FILE:librssl_tmp> 
									$<TARGET_FILE:ZLIB::ZLIB> 
									$<TARGET_FILE:LZ4::LZ4>
							COMMENT "Archiving objects for librssl ..."
							)

	endif()
	unset(_rssl_tmp_dir)

	#
	##          -- Build librssl SHARED LIBRARY --

    if (CMAKE_HOST_WIN32)
		if(BUILD_ETA_CPU_BIND)			
			add_library( librssl_shared SHARED 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${rsslWinRCFiles} ${sharedVersionSrc} ${rsslJsonConverterFiles} ${rsslWinBindThread} )
		else()
			add_library( librssl_shared SHARED 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${rsslWinRCFiles} ${sharedVersionSrc} ${rsslJsonConverterFiles} )
		endif()
    else()
		if(BUILD_ETA_CPU_BIND)			
			add_library( librssl_shared SHARED 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${rsslWinRCFiles} ${sharedVersionSrc} ${rsslJsonConverterFiles} ${rsslLinuxBindThread} $<TARGET_OBJECTS:rssl_asm> )
		else()
			add_library( librssl_shared SHARED 
							${rsslSrcFiles} ${rsslWinInetSrcFiles} ${rsslWinRCFiles} ${sharedVersionSrc} ${rsslJsonConverterFiles} )
		endif()
    endif()

    target_include_directories(librssl_shared
										PRIVATE
											../Transport
											${Eta_SOURCE_DIR}/Impl/Util/Include
											${Eta_SOURCE_DIR}/Impl/Converter/Include
										PUBLIC
											../Util/Include
											${Eta_SOURCE_DIR}/Include/RDM
											${Eta_SOURCE_DIR}/Include/Codec
											${Eta_SOURCE_DIR}/Include/Util
											${Eta_SOURCE_DIR}/Include/Transport
											${Eta_SOURCE_DIR}/Include/Converter
											${Eta_SOURCE_DIR}/Include
											${Eta_BINARY_DIR}/Include
											.
                                )
								
    if (CMAKE_HOST_WIN32)
        target_link_libraries( librssl_shared 
                                    wininet.lib 
                                    ws2_32.lib
									crypt32.lib
									cryptui.lib
									Iphlpapi.lib
									ZLIB::ZLIB 
									LZ4::LZ4
								)
        target_compile_options( librssl_shared 
                                    INTERFACE
                                        "/D RSSL_IMPORTS"
                                    PRIVATE
                                        "/D RSSL_EXPORTS" 
					                    "/D REAL_SHA1_INIT"
                                        ${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
                                        ${RCDEV_TYPE_CHECK_FLAG}
                                        ${RTSDK_EXTRA_FLAGS}
                                        $<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
                                )
                                               
        set_target_properties( librssl_shared 
                                PROPERTIES 
                                    OUTPUT_NAME librssl
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
		
	target_compile_options(librssl_shared 
					PRIVATE
						"-DREAL_SHA1_INIT"
                        ${RTSDK_EXTRA_FLAGS} )

        set_target_properties( librssl_shared 
                                PROPERTIES 
                                    PREFIX "" 
                                    OUTPUT_NAME librssl 
                                    VERSION 
                                        ${Eta_VERSION} 
                                    SOVERSION 
                                        ${librssl_SO_VERSION} 
                            )

		target_link_libraries( librssl_shared ZLIB::ZLIB LZ4::LZ4 )

    endif()

	DEBUG_PRINT(librssl_tmp)

	rcdev_add_target(rtsdk librssl librssl_shared)


endif()

DEBUG_PRINT(librssl)
DEBUG_PRINT(librssl_shared)
