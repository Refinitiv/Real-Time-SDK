/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



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

RsslRet serviceNameToIdCallback(RsslBuffer* name, RsslUInt16* Id);

#ifdef __cplusplus
};
#endif

#endif
