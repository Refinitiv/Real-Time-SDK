/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestReactorEvent.h"
#include "TestUtil.h"
#include "TunnelStreamGetTime.h"
#include "gtest/gtest.h"

using namespace testing;

TestReactorEvent::TestReactorEvent(TestReactorEventTypes type, void* pEvent)
{
	_nanoTime = getTimeNano();
	_type = type;

	_msgBuffer.data = _msgBufferMemory;
	_msgBuffer.length = sizeof(_msgBufferMemory);

	/* Copy event, based on which type it is. */
	switch(type)
	{       
		case CHANNEL_EVENT:
		{
			RsslReactorChannelEvent* channelEvent = (RsslReactorChannelEvent *)pEvent;
			_channelEvent.channelEventType = channelEvent->channelEventType;
			_channelEvent.pReactorChannel = channelEvent->pReactorChannel;
			_channelEvent.pError = &_errorInfo;
			if (channelEvent->pError)
				rsslCopyErrorInfo(_channelEvent.pError, channelEvent->pError);
			break;
		}
			
		case DEFAULT_MSG:
		{
			_msgEvent.pRsslMsgBuffer = &_msgBuffer;
			_msgEvent.pStreamInfo = &_streamInfo;
			copyMsgEvent((RsslMsgEvent *)pEvent, &_msgEvent);
			_msgEvent.pErrorInfo = &_errorInfo;
			if (((RsslMsgEvent *)pEvent)->pErrorInfo)
				rsslCopyErrorInfo(_msgEvent.pErrorInfo, ((RsslMsgEvent *)pEvent)->pErrorInfo);
			break;
		}
		
		case LOGIN_MSG:
		{
			_loginMsgEvent.baseMsgEvent.pRsslMsgBuffer = &_msgBuffer;
			_loginMsgEvent.baseMsgEvent.pStreamInfo = &_streamInfo;
			_loginMsgEvent.pRDMLoginMsg = &_loginMsg;
			copyLoginMsgEvent((RsslRDMLoginMsgEvent *)pEvent, &_loginMsgEvent);
			_loginMsgEvent.baseMsgEvent.pErrorInfo = &_errorInfo;
			if (((RsslMsgEvent *)pEvent)->pErrorInfo)
				rsslCopyErrorInfo(_loginMsgEvent.baseMsgEvent.pErrorInfo, ((RsslMsgEvent *)pEvent)->pErrorInfo);
			break;
		}
		
		case DIRECTORY_MSG:
		{
			_directoryMsgEvent.baseMsgEvent.pRsslMsgBuffer = &_msgBuffer;
			_directoryMsgEvent.baseMsgEvent.pStreamInfo = &_streamInfo;
			_directoryMsgEvent.pRDMDirectoryMsg = &_directoryMsg;
			copyDirectoryMsgEvent((RsslRDMDirectoryMsgEvent *)pEvent, &_directoryMsgEvent);
			_directoryMsgEvent.baseMsgEvent.pErrorInfo = &_errorInfo;
			if (((RsslMsgEvent *)pEvent)->pErrorInfo)
				rsslCopyErrorInfo(_directoryMsgEvent.baseMsgEvent.pErrorInfo, ((RsslMsgEvent *)pEvent)->pErrorInfo);
			break;
		}
		
		case DICTIONARY_MSG:
		{
			_dictionaryMsgEvent.baseMsgEvent.pRsslMsgBuffer = &_msgBuffer;
			_dictionaryMsgEvent.baseMsgEvent.pStreamInfo = &_streamInfo;
			copyDictionaryMsgEvent((RsslRDMDictionaryMsgEvent *)pEvent, &_dictionaryMsgEvent);
			_dictionaryMsgEvent.baseMsgEvent.pErrorInfo = &_errorInfo;
			if (((RsslMsgEvent *)pEvent)->pErrorInfo)
				rsslCopyErrorInfo(_dictionaryMsgEvent.baseMsgEvent.pErrorInfo, ((RsslMsgEvent *)pEvent)->pErrorInfo);
			break;
		}
		 
		default:
			ADD_FAILURE() << "Unknown Msg type: " << type;
			break;
	}
}

TestReactorEvent::TestReactorEvent(TestReactorEventTypes type, RsslTunnelStream* pTunnelStream, void* pEvent)
{
	_type = type;
	_pTunnelStream = pTunnelStream;

	switch (type)
	{
		case TUNNEL_STREAM_STATUS:
		{
			copyTunnelStreamStatusEvent((RsslTunnelStreamStatusEvent *)pEvent, &_tunnelStreamStatusEvent);
			_tunnelStreamStatusEvent.pReactorChannel = ((RsslTunnelStreamStatusEvent *)pEvent)->pReactorChannel;
			break;
		}
		
		case TUNNEL_STREAM_MSG:
		{
			copyTunnelStreamMsgEvent((RsslTunnelStreamMsgEvent *)pEvent, &_tunnelStreamMsgEvent);
			_tunnelStreamMsgEvent.pErrorInfo = &_errorInfo;
			if (((RsslTunnelStreamMsgEvent *)pEvent)->pErrorInfo)
				rsslCopyErrorInfo(_tunnelStreamMsgEvent.pErrorInfo, ((RsslTunnelStreamMsgEvent *)pEvent)->pErrorInfo);
			_tunnelStreamMsgEvent.pReactorChannel = ((RsslTunnelStreamMsgEvent *)pEvent)->pReactorChannel;
			break;
		}

		default:
			ADD_FAILURE() << "Unknown TunnelStream Msg type: " << type;
			break;
	}
}

TestReactorEvent::TestReactorEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamRequestEvent* pTunnelStreamRequestEvent)
{
	_type = TUNNEL_STREAM_REQUEST;
	_pTunnelStream = pTunnelStream;

	/* Copy the event. */
	_tunnelStreamRequestEvent.pReactorChannel = pTunnelStreamRequestEvent->pReactorChannel;
	_tunnelStreamRequestEvent.domainType = pTunnelStreamRequestEvent->domainType;
	_tunnelStreamRequestEvent.streamId = pTunnelStreamRequestEvent->streamId;
	_tunnelStreamRequestEvent.serviceId = pTunnelStreamRequestEvent->serviceId;
	_tunnelStreamRequestEvent.name = _name;
	strncpy(_tunnelStreamRequestEvent.name, pTunnelStreamRequestEvent->name, sizeof(_name));
	_tunnelStreamRequestEvent.classOfServiceFilter = pTunnelStreamRequestEvent->classOfServiceFilter;
}

TestReactorEventTypes TestReactorEvent::type()
{
	return _type;
}

void* TestReactorEvent::reactorEvent()
{
	switch(_type)
	{       
		case CHANNEL_EVENT:
		{
			return &_channelEvent;
		}
			
		case DEFAULT_MSG:
		{
			return &_msgEvent;
		}
		
		case LOGIN_MSG:
		{
			return &_loginMsgEvent;
		}
		
		case DIRECTORY_MSG:
		{
			return &_directoryMsgEvent;
		}
		
		case DICTIONARY_MSG:
		{
			return &_dictionaryMsgEvent;
		}
		
		case TUNNEL_STREAM_STATUS:
		{
			return &_tunnelStreamStatusEvent;
		}
		
		case TUNNEL_STREAM_MSG:
		{
			return &_tunnelStreamMsgEvent;
		}

		default:
			ADD_FAILURE() << "Unknown Msg type: " << _type;
			return NULL;
	}
}

RsslTunnelStreamRequestEvent* TestReactorEvent::tunnelStreamRequestEvent()
{
	return &_tunnelStreamRequestEvent;
}

RsslUInt TestReactorEvent::nanoTime()
{
	return _nanoTime;
}

RsslTunnelStream* TestReactorEvent::tunnelStream()
{
	return _pTunnelStream;
}
