/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

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

// API QA
#define DUMP_DEBUG_GEN				-1
#define DUMP_DEBUG_RWF				0
#define DUMP_DEBUG_JSON				1

extern int		debugDumpGeneral;
extern int		debugDumpProtocol[2];

// RsslDebugFlags
extern RsslUInt32 dumpDebugFlags;

RsslRet initDump(void);
RsslRet activateDump(RsslChannel* chnl);

void inDumpIpcIn(const char* functionName, char* buffer, RsslUInt32 length, RsslUInt64 opaque);
void inDumpIpcOut(const char* functionName, char* buffer, RsslUInt32 length, RsslUInt64 opaque);
void inDumpRsslIn(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId);
void inDumpRsslOut(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socketId);

void dumpIpcInRWF(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socket);
void dumpIpcOutRWF(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socket);
void dumpRsslInRWF(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);
void dumpRsslOutRWF(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);

void dumpIpcInJSON(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socket);
void dumpIpcOutJSON(const char* functionName, char* buffer, RsslUInt32 length, RsslSocket socket);
void dumpRsslInJSON(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);
void dumpRsslOutJSON(const char* functionName, char* buffer, RsslUInt32 length, RsslChannel* channel);
// END QA API

#ifdef __cplusplus
};
#endif

#endif
