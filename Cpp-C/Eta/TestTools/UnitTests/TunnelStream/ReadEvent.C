/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ReadEvent.h"
#include "TestUtil.h"
#include "rtr/msgQueueEncDec.h"
#include "gtest/gtest.h"

using namespace testing;

ReadEvent::ReadEvent(RsslBuffer* pBuffer, RsslChannel* pChannel, RsslInt lastReadRet)
{
	if (pBuffer != NULL)
	{
		pBufferMemory = new char[pBuffer->length];
		_buffer.data = pBufferMemory;
		_buffer.length = pBuffer->length;
		memcpy(_buffer.data, pBuffer->data, pBuffer->length);
	}
	else
	{
		_buffer.data = NULL;
		_buffer.length = 0;
	}
		
	_pChannel = pChannel;
	_lastReadRet = lastReadRet;
	rsslClearDecodeIterator(&_dIter);
	_lastReadRet = 0;
	memset(&_errorInfo, 0, sizeof(RsslErrorInfo));
	pMsg = NULL;
	pLoginMsg = NULL;
	pDirectoryMsg = NULL;
	pTunnelMsg = NULL;
	pMsgDecode = NULL;
}

ReadEvent::~ReadEvent()
{
	if (pBufferMemory)
		delete pBufferMemory;

	if (pMsg)
		delete pMsg;

	if (pLoginMsg)
		delete pLoginMsg;

	if (pDirectoryMsg)
		delete pDirectoryMsg;

	if (pTunnelMsg)
		delete pTunnelMsg;

	if (pMsgDecode)
		delete pMsgDecode;
}

RsslInt ReadEvent::lastReadRet()
{
	return _lastReadRet;
}
	
/* Decodes the buffer of this event to a message and returns it. */
RsslMsg* ReadEvent::msg()
{
	pMsg = new RsslMsg;
	rsslClearDecodeIterator(&_dIter);
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorRWFVersion(&_dIter, _pChannel->majorVersion, _pChannel->minorVersion));
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorBuffer(&_dIter, &_buffer));
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, pMsg));
	return pMsg;
}
	
RsslRDMLoginMsg* ReadEvent::loginMsg()
{
	RsslMsg* pMsg = msg();
	pLoginMsg = new RsslRDMLoginMsg;
				
	EXPECT_EQ(RSSL_DMT_LOGIN, pMsg->msgBase.domainType);
	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_REQUEST;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMLoginMsg(&_dIter, pMsg, pLoginMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_REFRESH:
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_REFRESH;
			EXPECT_EQ(RSSL_RET_SUCCESS,rsslDecodeRDMLoginMsg(&_dIter, pMsg, pLoginMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_STATUS:
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_STATUS;
			EXPECT_EQ(RSSL_RET_SUCCESS,rsslDecodeRDMLoginMsg(&_dIter, pMsg, pLoginMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_GENERIC:
			pLoginMsg->rdmMsgBase.rdmMsgType = RDM_LG_MT_CONSUMER_CONNECTION_STATUS;
			EXPECT_EQ(RSSL_RET_SUCCESS,rsslDecodeRDMLoginMsg(&_dIter, pMsg, pLoginMsg, NULL, &_errorInfo));
			break;
		default:
			ADD_FAILURE() << "Unknown loginMsg type: " << pMsg->msgBase.msgClass;
			break;
	}
		
	return pLoginMsg;
}

RsslRDMDirectoryMsg* ReadEvent::directoryMsg()
{
	pMsg = msg();
	RsslRDMDirectoryMsg* pDirectoryMsg = new RsslRDMDirectoryMsg;
				
	EXPECT_EQ(RSSL_DMT_SOURCE, pMsg->msgBase.domainType);
	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REQUEST:
			pDirectoryMsg->rdmMsgBase.rdmMsgType = RDM_DR_MT_REQUEST;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMDirectoryMsg(&_dIter, pMsg, pDirectoryMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_REFRESH:
			pDirectoryMsg->rdmMsgBase.rdmMsgType = RDM_DR_MT_REFRESH;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMDirectoryMsg(&_dIter, pMsg, pDirectoryMsg, NULL, &_errorInfo));
			break;

		case RSSL_MC_UPDATE:
			pDirectoryMsg->rdmMsgBase.rdmMsgType = RDM_DR_MT_UPDATE;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMDirectoryMsg(&_dIter, pMsg, pDirectoryMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_STATUS:
			pDirectoryMsg->rdmMsgBase.rdmMsgType = RDM_DR_MT_STATUS;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMDirectoryMsg(&_dIter, pMsg, pDirectoryMsg, NULL, &_errorInfo));
			break;
				
		case RSSL_MC_GENERIC:
			pDirectoryMsg->rdmMsgBase.rdmMsgType = RDM_DR_MT_CONSUMER_STATUS;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeRDMDirectoryMsg(&_dIter, pMsg, pDirectoryMsg, NULL, &_errorInfo));
			break;
		default:
			ADD_FAILURE() << "Unknown directoryMsg type: " << pMsg->msgBase.msgClass;
			break;
	}
		
	return pDirectoryMsg;
}

TunnelStreamMsg* ReadEvent::tunnelMsg(RsslMsg* pMsg, AckRangeList* pAckRangeList, AckRangeList* pNakRangeList)
{
	pTunnelMsg = new TunnelStreamMsg;
	EXPECT_EQ(RSSL_MC_GENERIC, pMsg->msgBase.msgClass);
	EXPECT_EQ(RSSL_RET_SUCCESS, tunnelStreamMsgDecode(pMsg, pTunnelMsg, pAckRangeList, pNakRangeList, COS_CURRENT_STREAM_VERSION));
	return pTunnelMsg;
}

RsslMsg* ReadEvent::decodeMsg(RsslBuffer* pBuffer)
{
	pMsgDecode = new RsslMsg;
	rsslClearDecodeIterator(&_dIter);
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorRWFVersion(&_dIter, _pChannel->majorVersion, _pChannel->minorVersion));
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorBuffer(&_dIter, pBuffer));
	EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, pMsgDecode));
	return pMsgDecode;
}
