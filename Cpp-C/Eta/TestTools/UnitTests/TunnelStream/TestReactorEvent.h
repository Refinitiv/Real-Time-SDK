/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef _TEST_REACTOR_EVENT_H
#define _TEST_REACTOR_EVENT_H

#include "rtr/rsslTypes.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslReactorChannel.h"
#include "rtr/rsslReactorEvents.h"
#include "rtr/rsslTunnelStream.h"

#ifdef __cplusplus
extern "C" {
#endif

enum TestReactorEventTypes
{
	CHANNEL_EVENT,	           /** ReactorChannelEvent */
	LOGIN_MSG,		           /** RDMLoginMsgEvent */
	DIRECTORY_MSG,	           /** RDMDirectoryMsgEvent */
	DICTIONARY_MSG,	           /** RDMDictionaryMsgEvent */
	DEFAULT_MSG,				/** ReactorMsgEvent */
	TUNNEL_STREAM_STATUS,      /** TunnelStreamStatusEvent */
	TUNNEL_STREAM_MSG,         /** TunnelStreamMsgEvent */
	TUNNEL_STREAM_REQUEST     /** TunnelStreamRequestEvent */
};

class TestReactorEvent
{
private:
	TestReactorEventTypes _type;
	RsslErrorInfo _errorInfo;
	RsslReactorChannelEvent _channelEvent;
	RsslMsgEvent _msgEvent;
	RsslRDMLoginMsgEvent _loginMsgEvent;
	RsslRDMLoginMsg _loginMsg;
	RsslRDMDirectoryMsgEvent _directoryMsgEvent;
	RsslRDMDirectoryMsg _directoryMsg;
	RsslRDMDictionaryMsgEvent _dictionaryMsgEvent;
	RsslRDMDictionaryMsg _dictionaryMsg;
	RsslTunnelStreamStatusEvent _tunnelStreamStatusEvent;
	RsslTunnelStreamMsgEvent _tunnelStreamMsgEvent;
	RsslTunnelStreamRequestEvent _tunnelStreamRequestEvent;
	RsslUInt _nanoTime;
	RsslTunnelStream* _pTunnelStream;
	RsslStreamInfo _streamInfo;
	RsslBuffer _msgBuffer;
	char _msgBufferMemory[1024];
	char _name[256];

public:
	TestReactorEvent(TestReactorEventTypes type, void* pEvent);
	TestReactorEvent(TestReactorEventTypes type, RsslTunnelStream* pTunnelStream, void* pEvent);
	TestReactorEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamRequestEvent* pTunnelStreamRequestEvent);
	TestReactorEventTypes type();
	void* reactorEvent();
	RsslTunnelStreamRequestEvent* tunnelStreamRequestEvent();
	RsslUInt nanoTime();
	RsslTunnelStream* tunnelStream();
};

#ifdef __cplusplus
};
#endif

#endif
