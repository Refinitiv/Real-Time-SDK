/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

class Consumer;
class CoreComponent;
class ConsumerProviderSessionOptions;

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Creates a default service. */
RsslRDMService* defaultService();

/* Copy RsslMsgEvent parts. */
void copyMsgEvent(RsslMsgEvent* srcEvent, RsslMsgEvent* destEvent);

/* Copy RsslRDMLoginMsgEvent parts. */
void copyLoginMsgEvent(RsslRDMLoginMsgEvent* srcEvent, RsslRDMLoginMsgEvent* destEvent);

/* Copy RsslRDMDirectoryMsgEvent parts. */
void copyDirectoryMsgEvent(RsslRDMDirectoryMsgEvent* srcEvent, RsslRDMDirectoryMsgEvent* destEvent);

/* Copy RsslRDMDictionaryMsgEvent parts. */
void copyDictionaryMsgEvent(RsslRDMDictionaryMsgEvent* srcEvent, RsslRDMDictionaryMsgEvent* destEvent);

/* Copy RsslTunnelStreamStatusEvent parts. */
void copyTunnelStreamStatusEvent(RsslTunnelStreamStatusEvent* srcEvent, RsslTunnelStreamStatusEvent* destEvent);

/* Copy RsslTunnelStreamMsgEvent parts. */
void copyTunnelStreamMsgEvent(RsslTunnelStreamMsgEvent* srcEvent, RsslTunnelStreamMsgEvent* destEvent);

/* Copies a LoginMsg. Used for both RDMLoginMsgEvent and TunnelStreamAuthInfo. */
void copyLoginMsg(RsslRDMLoginMsg* srcMsg, RsslRDMLoginMsg* destMsg);

/* Copies a DirectoryMsg. */
void copyDirectoryMsg(RsslRDMDirectoryMsg* srcMsg, RsslRDMDirectoryMsg* destMsg);

/* Copies a DictionaryMsg. */
void copyDictionaryMsg(RsslRDMDictionaryMsg* srcMsg, RsslRDMDictionaryMsg* destMsg);

/* Copies a buffer. */
void copyBuffer(RsslBuffer* srcBuffer, RsslBuffer* destBuffer);

/* Opens a channel between a Reactor-Based pConsumer and a core pProvider. Sets up login and directory streams. */
void openSession(Consumer* pConsumer, CoreComponent* pProvider, ConsumerProviderSessionOptions* pOpts, bool recoveringChannel);

#ifdef __cplusplus
};
#endif

#endif

