/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_READ_EVENT_H
#define TEST_READ_EVENT_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "rtr/msgQueueHeader.h"

#ifdef __cplusplus
extern "C" {
#endif

class ReadEvent
{
private:
	RsslBuffer _buffer;
	char* pBufferMemory;
	RsslChannel* _pChannel;
	RsslDecodeIterator _dIter;
	RsslInt _lastReadRet;
	RsslErrorInfo _errorInfo;
	RsslMsg* pMsg;
	RsslRDMLoginMsg* pLoginMsg;
	RsslRDMDirectoryMsg* pDirectoryMsg;
	TunnelStreamMsg* pTunnelMsg;
	RsslMsg* pMsgDecode;

public:
	ReadEvent(RsslBuffer* pBuffer, RsslChannel* pChannel, RsslInt lastReadRet);

	~ReadEvent();
	
	/* The readRetVal of the last call to channel.read. */
	RsslInt lastReadRet();
	
	/* Decodes the buffer of this event to a message and returns it. */
	RsslMsg* msg();
	
	/* Decodes the buffer of this event to a RsslRDMLoginMsg and returns it. */
	RsslRDMLoginMsg* loginMsg();

	/* Decodes the buffer of this event to a RsslRDMDirectoryMsg and returns it. */
	RsslRDMDirectoryMsg* directoryMsg();

	/* Decodes the given message to a TunnelStream msg and returns it. */
	TunnelStreamMsg* tunnelMsg(RsslMsg* pMsg, AckRangeList* pAckRangeList, AckRangeList* pNakRangeList);
	
	/* Decodes a buffer to a msg. */
	RsslMsg* decodeMsg(RsslBuffer* pBuffer);
};

#ifdef __cplusplus
};
#endif

#endif
