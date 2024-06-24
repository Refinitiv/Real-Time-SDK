/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "Provider.h"

Provider::Provider(TestReactor* pTestReactor) : TestReactorComponent(pTestReactor)
{
	rsslClearOMMProviderRole(&_providerRole);
	_pReactorRole = (RsslReactorChannelRole *)&_providerRole;
}

RsslReactorCallbackRet Provider::channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleChannelEvent(pEvent);
}

RsslReactorCallbackRet Provider::defaultMsgCallback(RsslReactor*pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleDefaultMsgEvent(pEvent);
}

RsslReactorCallbackRet Provider::loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleLoginMsgEvent(pEvent);
}

RsslReactorCallbackRet Provider::directoryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleDirectoryMsgEvent(pEvent);
}

RsslReactorCallbackRet Provider::dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleDictionaryMsgEvent(pEvent);
}
	
RsslReactorCallbackRet Provider::tunnelStreamDefaultMsgCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pTunnelStream->pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleTunnelStreamMsgEvent(pTunnelStream, pEvent);
}

RsslReactorCallbackRet Provider::tunnelStreamStatusEventCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent)
{
	TestReactor* pTestReactor = ((Provider *)pTunnelStream->pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleTunnelStreamStatusEvent(pTunnelStream, pEvent);
}

RsslReactorCallbackRet Provider::tunnelStreamListenerCallback(RsslTunnelStreamRequestEvent* pEvent, RsslErrorInfo* pErrorInfo)
{
	TestReactor* pTestReactor = ((Provider *)pEvent->pReactorChannel->userSpecPtr)->testReactor();
	return (RsslReactorCallbackRet)pTestReactor->handleTunnelStreamRequestEvent(NULL, pEvent);
}
