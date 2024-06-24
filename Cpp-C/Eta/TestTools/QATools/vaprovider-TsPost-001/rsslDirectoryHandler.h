/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef _RTR_RSSL_DIRECTORY_HANDLER_H
#define _RTR_RSSL_DIRECTORY_HANDLER_H

#include "rsslProvider.h"
#include "rtr/rsslReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPEN_LIMIT 5
#define MAX_SRCDIR_INFO_STRLEN 256

/* reasons a source directory request is rejected */
typedef enum {
	MAX_SRCDIR_REQUESTS_REACHED	= 0,
	INCORRECT_FILTER_FLAGS		= 1,
	DIRECTORY_RDM_DECODER_FAILED = 2
} RsslSrcDirRejectReason;

/* Stores information about a consumer's directory request. */
typedef struct DirectoryRequestInfo
{
	RsslReactorChannel *chnl;
	RsslBool isInUse;
	RsslRDMDirectoryRequest dirRequest;
} DirectoryRequestInfo;

void initDirectoryHandler();
void setServiceName(char* servicename);
void setServiceId(RsslUInt64 serviceid);
RsslUInt64 getServiceId();
static RsslRDMDirectoryRequest* getDirectoryRequest(RsslReactorChannel* pReactorChannel, RsslRDMDirectoryRequest *pRequest);
static RsslRet sendDirectoryRefresh(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryRequest* srcDirReqInfo);
static RsslRet sendDirectoryRequestReject(RsslReactor *pReactor, RsslReactorChannel* pReactorChannel, RsslInt32 streamId, RsslSrcDirRejectReason reason, RsslErrorInfo *pError);
void closeDirectoryStreamForChannel(RsslReactorChannel* pReactorChannel);
static void closeDirectoryStream(RsslInt32 streamId);
RsslReactorCallbackRet directoryMsgCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslRDMDirectoryMsgEvent *pDirectoryMsgEvent);

/*
 * Clears a DirectoryRequestInfo structure.
 */
RTR_C_INLINE void clearDirectoryRequestInfo(DirectoryRequestInfo *pInfo)
{
	pInfo->chnl = NULL;
	pInfo->isInUse = RSSL_FALSE;
	rsslClearRDMDirectoryRequest(&pInfo->dirRequest);
}

#ifdef __cplusplus
};
#endif

#endif
