

#ifndef _RTR_RSSL_DIRECTORY_HANDLER_H
#define _RTR_RSSL_DIRECTORY_HANDLER_H

#include "rsslNIProvider.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRCDIR_STREAM_ID -3
#define OPEN_LIMIT 5

void setServiceName(char* servicename);
void setServiceId(RsslUInt64 serviceid);
RsslUInt64 getServiceId();
RsslRet sendSourceDirectoryResponse(RsslChannel* chnl);

#ifdef __cplusplus
};
#endif

#endif
