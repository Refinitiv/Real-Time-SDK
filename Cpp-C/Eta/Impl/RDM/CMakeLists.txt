
project(RDM)

set( RDMSrcFiles
			rsslRDMDictionaryMsg.c
			rsslRDMDirectoryMsg.c
			rsslRDMLoginMsg.c
			rsslRDMQueueMsg.c 
			)

add_library(librsslRDM STATIC ${RDMSrcFiles})
target_include_directories(librsslRDM PUBLIC
                            $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/RDM>
                            $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Util>
                            $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Codec>
                            $<BUILD_INTERFACE:${Eta_SOURCE_DIR}/Include/Transport>
						  )
if (CMAKE_HOST_UNIX)
    set_target_properties(librsslRDM PROPERTIES PREFIX "" )
endif()


