/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_CONSUMER_H
#define TEST_CONSUMER_H

class TunnelStreamProvider;
class TunnelStreamCoreProvider;

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "rtr/msgQueueHeader.h"
#include "TestReactorComponent.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Information returned from openTunnelStream function. */
class OpenedTunnelStreamInfo
{
private:
	RsslInt _providerTunnelStreamId;
	RsslTunnelStream* _pConsumerTunnelStream;
	RsslTunnelStream* _pProviderTunnelStream;

public:
	OpenedTunnelStreamInfo(RsslTunnelStream* pConsumerTunnelStream, RsslTunnelStream* pProviderTunnelStream, RsslInt providerTunnelStreamId)
	{
		_providerTunnelStreamId = providerTunnelStreamId;
		_pConsumerTunnelStream = pConsumerTunnelStream;
		_pProviderTunnelStream = pProviderTunnelStream;
	}
		
	/* Provider-side tunnel stream ID. This may not match the consumer side when
	 * the watchlist is enabled. */
	RsslInt providerTunnelStreamId() { return _providerTunnelStreamId; }

	/* Consumer's new tunnel stream. */
	RsslTunnelStream* consumerTunnelStream() { return _pConsumerTunnelStream; }

	/* Providers's new tunnel stream (not set on core providers). */
	RsslTunnelStream* providerTunnelStream() { return _pProviderTunnelStream; }
};

class Consumer : public TestReactorComponent
{
private:
	RsslReactorOMMConsumerRole _consumerRole;

	AckRangeList _ackRangeList;
	AckRangeList _nakRangeList;
	
public:
	Consumer(TestReactor* pTestReactor);

	static RsslReactorCallbackRet channelOpenCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorChannelEvent *pEvent);

	static RsslReactorCallbackRet channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pEvent);

	static RsslReactorCallbackRet defaultMsgCallback(RsslReactor*pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pEvent);

	static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMLoginMsgEvent* pEvent);

	static RsslReactorCallbackRet directoryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDirectoryMsgEvent* pEvent);

	static RsslReactorCallbackRet dictionaryMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslRDMDictionaryMsgEvent* pEvent);

	static RsslReactorCallbackRet tunnelStreamDefaultMsgCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent);

	static RsslReactorCallbackRet tunnelStreamStatusEventCallback(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent);
	
	/* Opens a tunnel stream between this consumer and a TunnelStreamProvider. */
	OpenedTunnelStreamInfo* openTunnelStream(TunnelStreamProvider* pProvider, RsslTunnelStreamOpenOptions* pTsOpenOpts);
	
	/* Opens a tunnel stream between this consumer and a TunnelStreamCoreProvider. */
	OpenedTunnelStreamInfo* openTunnelStream(TunnelStreamCoreProvider* pProvider, RsslTunnelStreamOpenOptions* pTsOpenOpts, RsslClassOfService* pProvClassOfService);

	/* Closes a tunnelStream to a provider.
	 * 
	 * @param provider Provider component.
	 * @param consTunnelStream Consumer's tunnel stream.
	 * @param consTunnelStream Provider's tunnel stream.
	 * @param finalStatusEvent Whether the consumer & provider want to get the final TunnelStreamStatusEvent.
	 */
	void closeTunnelStream(TunnelStreamProvider* pProvider, RsslTunnelStream* pConsTunnelStream, RsslTunnelStream* pProvTunnelStream, bool finalStatusEvent);
	
	/* Closes a tunnelStream to a core provider.
	 * 
	 * @param provider Core provider component.
	 * @param consTunnelStream Consumer's tunnel stream.
	 * @param provTunnelStreamId Stream ID on provider side (may not match consumer if watchlist is enabled).
	 * @param provFinSeqNum Sequence number to use as the provider's FIN.
	 * @param finalStatusEvent Whether the consumerw wants to get the final TunnelStreamStatusEvent.
	 */
	void closeTunnelStream(TunnelStreamCoreProvider* pProvider, RsslTunnelStream* pConsTunnelStream, RsslInt provTunnelStreamId, RsslInt provFinSeqNum, bool finalStatusEvent);
};

#ifdef __cplusplus
};
#endif

#endif
