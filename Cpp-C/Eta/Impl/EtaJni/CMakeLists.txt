# |-----------------------------------------------------------------------------
# |            This source code is provided under the Apache 2.0 license      --
# |  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
# |                See the project's LICENSE.md for details.                  --
# |           Copyright (C) 2019 Refinitiv. All rights reserved.            --
# |-----------------------------------------------------------------------------
#
project(EtaJNI 
	VERSION ${rtsdk_INTERNAL_API_VERSION_MAJOR}.${rtsdk_INTERNAL_API_VERSION_MINOR}.${rtsdk_INTERNAL_API_VERSION_PATCH}.${rtsdk_INTERNAL_API_VERSION_TWEAK}
        LANGUAGES C CXX)

DEBUG_PRINT(Eta_VERSION)

#Setup version file generation.
# relative paths for configure_file are the _CURRENT_SOURCE_DIR for the input file
# and _CURRENT_BINARY_DIR for the output file location
configure_file(Include/rsslEtaJNIVersion.h.in Include/rsslEtaJNIVersion.h @ONLY)

if (CMAKE_HOST_WIN32)
    set(rsslEtaJNIWinRCFiles
        ${Eta_SOURCE_DIR}/Impl/EtaJni/Include/rsslEtaJNI.rc
        ${Eta_SOURCE_DIR}/Impl/EtaJni/Include/rsslEtaJNIResource.h
        )
endif()

set ( EtaJniSrcFiles
  Impl/JniTransport.c
  )
  
set ( EtaJniHeaderFiles
	Include/JNIChannel.h
	Include/JNIProtocol.h
	Include/JNIServer.h
	Include/jni.h
	${CMAKE_CURRENT_BINARY_DIR}/Include/rsslEtaJNIVersion.h
)

set(EtaJniLinuxHeaderFiles
	Include/linux/jni_md.h
	)
	
set(EtaJniWinHeaderFiles
	Include/win32/jni_md.h
	)

if ( CMAKE_HOST_WIN32 )
  add_library( libEtaJni_shared SHARED ${EtaJniSrcFiles} ${EtaJniHeaderFiles}
      ${EtaJniWinHeaderFiles} ${rsslEtaJNIWinRCFiles} )
  target_compile_options( libEtaJni_shared
								PRIVATE 
									${RCDEV_DEBUG_TYPE_FLAGS_NONSTATIC}
									${RCDEV_TYPE_CHECK_FLAG}
									$<$<CONFIG:Release_MD>:${RCDEV_FLAGS_NONSTATIC_RELEASE}>
						)
  set_target_properties( libEtaJni_shared PROPERTIES OUTPUT_NAME "rsslEtaJNI" )
  
  target_include_directories(libEtaJni_shared
							PUBLIC
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/EtaJni/Include>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/EtaJni/Include/win32>
								$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Include>
							)
  
  target_link_libraries( libEtaJni_shared 
							librssl 
							${SYSTEM_LIBRARIES} 
						)
  
  	set_target_properties( libEtaJni_shared 
                            PROPERTIES 
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

else()
    add_library( libEtaJni_shared SHARED ${EtaJniSrcFiles}
        ${EtaJniHeaderFiles} ${EtaJniLinuxHeaderFiles} Impl/rsslEtaJNIVersionShared.c )
	
    target_link_libraries( libEtaJni_shared 
							librssl 
							${SYSTEM_LIBRARIES} 
						)
    set_target_properties( libEtaJni_shared PROPERTIES OUTPUT_NAME "rsslEtaJNI" )

    target_include_directories(libEtaJni_shared
							PUBLIC
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/EtaJni/Include>
								$<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Impl/EtaJni/Include/linux>
								$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Include>
							)

endif()

rcdev_add_target(rtsdk libEtaJni_shared)

