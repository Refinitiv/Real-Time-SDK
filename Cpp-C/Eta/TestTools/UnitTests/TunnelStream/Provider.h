/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_PROVIDER_H
#define TEST_PROVIDER_H

#include "TestReactorComponent.h"

#ifdef __cplusplus
extern "C" {
#endif

class Provider : public TestReactorComponent
{
private:
	RsslReactorOMMProviderRole _providerRole;

public:
	Provider(TestReactor* pTestReactor);
	
	static RsslReactorCallbackRet channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pEvent);

	static RsslReactorCallbackRet defaultMsgCallback(RsslReactor*pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent);

	static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent* pEvent);

	static RsslReactorCallbackRet directoryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pEvent);

	static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pEvent);

	static RsslReactorCallbackRet tunnelStreamDefaultMsgCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent);

	static RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent);

	static RsslReactorCallbackRet tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent* pEvent, RsslErrorInfo* pErrorInfo);
};

#ifdef __cplusplus
};
#endif

#endif
