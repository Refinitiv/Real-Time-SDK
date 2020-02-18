

#ifndef _RTR_RSSL_DIRECTORY_HANDLER_H
#define _RTR_RSSL_DIRECTORY_HANDLER_H

#include "rsslProvider.h"
#include "rsslDirectoryEncodeDecode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPEN_LIMIT 10

void initDirectoryHandler();
void setServiceName(char* servicename);
void setServiceId(RsslUInt64 serviceid);
RsslUInt64 getServiceId();
static RsslSourceDirectoryRequestInfo* getSourceDirectoryReqInfo(RsslChannel* chnl, RsslMsg* msg, RsslMsgKey* key);
RsslRet processSourceDirectoryRequest(RsslChannel* chnl, RsslMsg* msg, RsslDecodeIterator* dIter);
static RsslRet sendSourceDirectoryResponse(RsslChannel* chnl, RsslSourceDirectoryRequestInfo* srcDirReqInfo);
static RsslRet sendSrcDirectoryRequestReject(RsslChannel* chnl, RsslInt32 streamId, RsslSrcDirRejectReason reason);
static RsslBool keyHasMinFilterFlags(RsslMsgKey* key);
void closeSrcDirectoryChnlStream(RsslChannel* chnl);
static void closeSrcDirectoryStream(RsslInt32 streamId);

RsslRet serviceNameToIdCallback(RsslBuffer* name, RsslUInt16* Id);

#ifdef __cplusplus
};
#endif

#endif
