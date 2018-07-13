

#ifndef _RTR_RSSL_SENDMSG_H
#define _RTR_RSSL_SENDMSG_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MSG_SIZE 4096
#define NUM_CLIENT_SESSIONS 5

#ifdef _WIN32
#ifdef _WIN64
#define SOCKET_PRINT_TYPE "%llu"	/* WIN64 */
#else
#define SOCKET_PRINT_TYPE "%u"	/* WIN32 */
#endif
#else
#define SOCKET_PRINT_TYPE "%d"  /* Linux */
#endif

extern RsslBool showTransportDetails;

RsslRet sendMessage(RsslChannel* chnl, RsslBuffer* msgBuf);
RsslRet sendMessageEx(RsslChannel* chnl, RsslBuffer* msgBuf, RsslWriteInArgs *writeInArgs);
RsslRet sendPing(RsslChannel* chnl);
RsslRet sendNotSupportedStatus(RsslChannel* chnl, RsslMsg* requestMsg);
RsslRet encodeNotSupportedStatus(RsslChannel* chnl, RsslMsg* requestMsg, RsslBuffer* msgBuf);

#ifdef __cplusplus
};
#endif

#endif
