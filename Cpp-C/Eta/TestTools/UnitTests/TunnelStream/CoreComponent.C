/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "CoreComponent.h"
#include "ReadEvent.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace std;

void CoreComponent::submit(RsslRDMMsgBase* pMsgBase, RsslMsg* pMsg)
{
	RsslInt ret;
	RsslInt32 bufferSize = 256;
	RsslUInt32 bytesWritten;
		
	ASSERT_TRUE((pMsgBase != NULL && pMsg == NULL) || (pMsgBase == NULL && pMsg != NULL));
		
	do
	{
		RsslBuffer* pBuffer = rsslGetBuffer(_pChannel, bufferSize, RSSL_FALSE, &_error);
		ASSERT_TRUE(pBuffer != NULL);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorRWFVersion(&_eIter, _pChannel->majorVersion, _pChannel->minorVersion));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorBuffer(&_eIter, pBuffer));
			
		if (pMsgBase != NULL)
			ret = rsslEncodeRDMMsg(&_eIter, (RsslRDMMsg *)pMsgBase, &bytesWritten, &_errorInfo);
		else
			ret = rsslEncodeMsg(&_eIter, pMsg);

			
		switch (ret)
		{
			case RSSL_RET_BUFFER_TOO_SMALL:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslReleaseBuffer(pBuffer, &_error));
				bufferSize *= 2;
				break;
			default:
				ASSERT_EQ(RSSL_RET_SUCCESS, ret);
				writeBuffer(pBuffer);
				return;
		}
	} while (true);
}

CoreComponent::CoreComponent():
	_pServer(NULL),
	_readInArgs(RSSL_INIT_READ_IN_ARGS),
	_readOutArgs(RSSL_INIT_READ_OUT_ARGS),
	_maxFragmentSize(0),
	_writeInArgs(RSSL_INIT_WRITE_IN_ARGS),
	_writeOutArgs(RSSL_INIT_WRITE_OUT_ARGS),
	_pChannel(NULL),
	_channelInfo(),
	_errorInfo(),
	_error()
{
	_portToBind = 17123;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);
	rsslClearInProgInfo(&_inProg);
	rsslClearEncodeIterator(&_eIter);
	rsslClearDecodeIterator(&_dIter);
}
	
RsslServer* CoreComponent::server()
{
	return _pServer;
}

RsslInt CoreComponent::serverPort()
{
	EXPECT_TRUE(_pServer != NULL);

	return _pServer->portNumber;
}

void CoreComponent::closeChannel()
{
	if (_pChannel != NULL)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslCloseChannel(_pChannel, &_error));
		_pChannel = NULL;
	}
}
	
void CoreComponent::close()
{
	closeChannel();

	if (_pServer != NULL)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslCloseServer(_pServer, &_error));
		_pServer = NULL;
	}

}
	 
void CoreComponent::bind(ConsumerProviderSessionOptions* pOpts)
{
	if (pOpts->connectionType() != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		RsslBindOptions bindOpts;
		rsslClearBindOpts(&bindOpts);
		bindOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
		bindOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
		snprintf(_portToBindString, sizeof (_portToBindString), "%d", (int)_portToBind++);
		bindOpts.serviceName = _portToBindString;
		bindOpts.pingTimeout = pOpts->pingTimeout();
		bindOpts.minPingTimeout = pOpts->pingTimeout();
		_pServer = rsslBind(&bindOpts, &_error);
		ASSERT_TRUE(_pServer != NULL) << "bind failed: " << _error.rsslErrorId << " (" << _error.text << ')' << endl;
	}
}

void CoreComponent::accept(RsslInt connectionType)
{
	if (connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		RsslAcceptOptions acceptOpts;
		RsslInt selRet;
		struct timeval selectTime;
		fd_set readFds;

		// make sure the connect has triggered the bind socket before calling accept();
		ASSERT_TRUE(_pServer != NULL);

		FD_ZERO(&readFds);
		FD_SET(_pServer->socketId, &readFds);

		selectTime.tv_sec = 5;
		selectTime.tv_usec = 0;
		selRet = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);

		ASSERT_TRUE(selRet > 0);

		if (FD_ISSET(_pServer->socketId, &readFds))
		{
			rsslClearAcceptOpts(&acceptOpts);
			_pChannel = rsslAccept(_pServer, &acceptOpts, &_error);
			ASSERT_TRUE(_pChannel != NULL) << "rsslAccept() failed: " << _error.rsslErrorId << " (" << _error.text << ')' << endl;
		}
	}
}
	
void CoreComponent::acceptAndInitChannel(RsslInt connectionType)
{
	struct timeval selectTime;
	fd_set readFds;

	accept(connectionType);

	ASSERT_TRUE(_pChannel != NULL);

	// Initialize channel.
	do
	{
		RsslInt ret;
				
		FD_ZERO(&readFds);
		FD_SET(_pChannel->socketId, &readFds);

		selectTime.tv_sec = 5;
		selectTime.tv_usec = 0;
		ret = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);

		ASSERT_TRUE(ret > 0);
				
		ret = rsslInitChannel(_pChannel, &_inProg, &_error);
		if (ret == RSSL_RET_SUCCESS)
			break;
					
		ASSERT_EQ(RSSL_RET_CHAN_INIT_IN_PROGRESS, ret);
	} while (true);
		
	ASSERT_EQ(_pChannel->state, RSSL_CH_STATE_ACTIVE);
		
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetChannelInfo(_pChannel, &_channelInfo, &_error));
}
	
void CoreComponent::dispatch(RsslUInt expectedEventCount)
{
	dispatch(expectedEventCount, expectedEventCount ? 1000 : 200);
}
	
void CoreComponent::dispatch(RsslUInt expectedEventCount, RsslUInt timeoutMsec)
{
	RsslInt selectRet = 0;
	RsslUInt currentTimeUsec, stopTimeUsec;
	RsslRet lastReadRet = 0;
	struct timeval selectTime;
	fd_set readFds;

	ASSERT_TRUE(_pChannel != NULL);

	FD_ZERO(&readFds);
	FD_SET(_pChannel->socketId, &readFds);

	/* Ensure no events were missed from previous calls to dispatch. */
	ASSERT_EQ(0, _eventQueue.size());

	currentTimeUsec = rsslGetTimeMicro();
		
	stopTimeUsec =  (RsslUInt)(timeoutMsec);
	stopTimeUsec *= 1000;
	stopTimeUsec += currentTimeUsec;

	do
	{
		if (lastReadRet == 0)
		{
			RsslUInt timeToSelect;

			timeToSelect = stopTimeUsec - currentTimeUsec;
					
			selectTime.tv_sec = (long)timeToSelect/1000000;
			selectTime.tv_usec = (long)(timeToSelect - selectTime.tv_sec * 1000000);
			selectRet = select(FD_SETSIZE, &readFds, NULL, NULL, &selectTime);
		}
		else
			selectRet = 1;

		if (selectRet > 0)
		{
			do
			{
				rsslClearReadInArgs(&_readInArgs);
				rsslClearReadOutArgs(&_readOutArgs);
				RsslBuffer* pBuffer = rsslReadEx(_pChannel, &_readInArgs, &_readOutArgs, &lastReadRet, &_error);
				if (lastReadRet == RSSL_RET_READ_PING || lastReadRet == RSSL_RET_READ_WOULD_BLOCK) continue; 
					
				_eventQueue.push_back(new ReadEvent(pBuffer, _pChannel, lastReadRet));
			} while (lastReadRet > 0);
		}

		currentTimeUsec = rsslGetTimeMicro();
	} while(currentTimeUsec < stopTimeUsec && (expectedEventCount == 0 || _eventQueue.size() < expectedEventCount));
		
	ASSERT_EQ(expectedEventCount, _eventQueue.size());
}
	
void CoreComponent::submit(RsslMsg* pMsg)
{
	submit(NULL, pMsg);
}
	
void CoreComponent::submit(RsslRDMMsgBase* pMsgBase)
{
	submit(pMsgBase, NULL);
}
		
void CoreComponent::writeBuffer(RsslBuffer* pBuffer)
{
	RsslInt ret;
		
	ret = rsslWriteEx(_pChannel, pBuffer, &_writeInArgs, &_writeOutArgs, &_error);
	ASSERT_TRUE(ret >= 0 || ret == RSSL_RET_WRITE_FLUSH_FAILED && _pChannel->state == RSSL_CH_STATE_ACTIVE);
		
	while (ret > 0)
	{
		ret = rsslFlush(_pChannel, &_error);
			
		ASSERT_TRUE(ret >= 0);
	}
}
	
ReadEvent* CoreComponent::pollEvent()
{
	ReadEvent* pEvent = NULL;
	
	if (!_eventQueue.empty())
	{
		pEvent = _eventQueue.front();
		_eventQueue.pop_front();
	}

	return pEvent;
}
	
void CoreComponent::defaultSessionDirectoryStreamId(RsslInt defaultSessionDirectoryStreamId)
{
	_defaultSessionDirectoryStreamId = defaultSessionDirectoryStreamId;
	_defaultSessionDirectoryStreamIdIsSet = true;
}

RsslInt CoreComponent::defaultSessionDirectoryStreamId()
{
	EXPECT_TRUE(_defaultSessionDirectoryStreamIdIsSet);
	return _defaultSessionDirectoryStreamId;
}
