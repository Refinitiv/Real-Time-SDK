/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


#ifndef _RTR_RSSL_DIRECTORY_HANDLER_H
#define _RTR_RSSL_DIRECTORY_HANDLER_H

#include "rsslDirectoryEncodeDecode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRCDIR_STREAM_ID 2

/* used to set the serviceName the user is interested */
void setServiceName(char* servicename);
/* used to get the serviceId that should be used for requesting other data */
RsslUInt64 getServiceId();
/*used to determine if a given domain type appers in the source directory response*/
RsslBool getSourceDirectoryCapabilities(RsslUInt64 domainId);
/* sends source directory request message */
RsslRet sendSourceDirectoryRequest(RsslChannel* chnl);
/* processes source directory response messages */
RsslRet processSourceDirectoryResponse(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
/* closes the stream associated with the source directory */
RsslRet closeSourceDirectoryStream(RsslChannel* chnl);
/* returns select attributes from the source directory response - useful to query information about services */
RsslRet getSourceDirectoryResponseInfo(RsslUInt serviceId, RsslSourceDirectoryResponseInfo** rsslSourceDirectoryResponseInfo);

RsslRet serviceNameToIdCallback(RsslBuffer* name, RsslUInt16* Id);

#ifdef __cplusplus
};
#endif

#endif
