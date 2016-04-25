/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "channelHandler.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#include <stdlib.h>

static RsslUInt64 outBytesTotal = 0;
static RsslUInt64 uncompOutBytesTotal = 0;
static RsslReadOutArgs readOutArgs = RSSL_INIT_READ_OUT_ARGS;

static void _flushDone(ChannelHandler *pHandler, ChannelInfo *pChannelInfo)
{
	pChannelInfo->needFlush = RSSL_FALSE;
}

static void _processActiveChannel(ChannelHandler *pHandler, ChannelInfo *pChannelInfo)
{
	TimeValue currentTime;
	RsslUInt64 pingTimeoutNsec;
	RsslError error;

	/* Set the next send/receive ping times. */
	currentTime = getTimeNano();
	pingTimeoutNsec = (RsslInt64)pChannelInfo->pChannel->pingTimeout * 1000000000ULL;
	pChannelInfo->nextSendPingTime = currentTime + pingTimeoutNsec/3;
	pChannelInfo->nextReceivePingTime = currentTime + pingTimeoutNsec;

	_flushDone(pHandler, pChannelInfo);

	if ((pHandler->channelActiveCallback)(pHandler, pChannelInfo) < RSSL_RET_SUCCESS)
	{
		snprintf(error.text, sizeof(error.text), "channelActiveCallback returned failure.");
		error.rsslErrorId = RSSL_RET_FAILURE;
		channelHandlerCloseChannel(pHandler, pChannelInfo, &error);
		return;
	}

	rsslQueueRemoveLink(&pHandler->initializingChannelList, &pChannelInfo->queueLink);
	pChannelInfo->parentQueue = &pHandler->activeChannelList;
	rsslQueueAddLinkToBack(&pHandler->activeChannelList, &pChannelInfo->queueLink);
}

RsslRet channelHandlerWriteChannel(ChannelHandler *pHandler, ChannelInfo *pChannelInfo, RsslBuffer *pBuffer, RsslUInt8 writeFlags)
{
	RsslRet ret;
	RsslError error;
	RsslUInt32 bytes, uncompBytes;
	RsslChannel *pChannel = pChannelInfo->pChannel;

	/* Write buffer */
	ret = rsslWrite(pChannel, pBuffer, RSSL_HIGH_PRIORITY, writeFlags, &bytes, &uncompBytes, &error);
	if (ret >= RSSL_RET_SUCCESS)
		return ret;

	switch (ret)
	{
		case RSSL_RET_WRITE_FLUSH_FAILED:
			if (pChannel->state == RSSL_CH_STATE_ACTIVE)
				return RSSL_RET_SUCCESS + 1; /* Channel is still open, but rsslWrite() tried to flush internally and failed. 
											  * Return positive value so the caller knows there's bytes to flush. */
			return ret;

		default:
			return ret;
	}

}

ChannelInfo *channelHandlerAddChannel(ChannelHandler *pHandler, RsslChannel *pChannel, void *pUserSpec, RsslBool checkPings)
{
	ChannelInfo *pChannelInfo;

	pChannelInfo = (ChannelInfo*)malloc(sizeof(ChannelInfo));


	clearChannelInfo(pChannelInfo);
	pChannelInfo->parentQueue = &pHandler->initializingChannelList;
	pChannelInfo->pChannel = pChannel;
	pChannelInfo->pUserSpec = pUserSpec;
	pChannelInfo->checkPings = checkPings;
	rsslQueueAddLinkToBack(&pHandler->initializingChannelList, &pChannelInfo->queueLink);
	channelHandlerRequestFlush(pHandler, pChannelInfo);

	if (pChannel->state == RSSL_CH_STATE_ACTIVE)
	{
		/* Channel is already active */
		_processActiveChannel(pHandler, pChannelInfo);
	}

	return pChannelInfo;
}

void channelHandlerCloseChannel(ChannelHandler *pHandler, ChannelInfo *pChannelInfo, RsslError *pError)
{
	if (pChannelInfo->pReactorChannel == NULL) // use UPA Channel
	{
		pHandler->channelInactiveCallback(pHandler, pChannelInfo, pError);
		rsslQueueRemoveLink(pChannelInfo->parentQueue, &pChannelInfo->queueLink);
		rsslCloseChannel(pChannelInfo->pChannel, pError);
	}
	else // use UPA VA Reactor
	{
		RsslErrorInfo errorInfo;

		rsslReactorCloseChannel(pChannelInfo->pReactor, pChannelInfo->pReactorChannel, &errorInfo);
		printf("Channel Closed.\n");
	}
	free(pChannelInfo);
}

RsslRet channelHandlerReadChannel(ChannelHandler *pHandler, ChannelInfo *pChannelInfo)
{
	RsslRet ret;
	RsslError error;
	RsslBool channelClosed = RSSL_FALSE;
	RsslReadInArgs readInArgs;

	/* Read until rsslRead() indicates that no more bytes are available in the queue. */
	do
	{
		RsslBuffer *pMsgBuf;
		if (pMsgBuf = rsslReadEx(pChannelInfo->pChannel,&readInArgs,&readOutArgs,&ret,&error))
		{
			/* Mark that we received data for ping timeout handling. */
			pChannelInfo->receivedMsg = RSSL_TRUE;

			/* Received an RsslBuffer, call the application's processing function. */
			if ((pHandler->msgCallback)(pHandler, pChannelInfo, pMsgBuf) < RSSL_RET_SUCCESS)
			{
				snprintf(error.text, sizeof(error.text), "Message callback returned failure.");
				error.rsslErrorId = RSSL_RET_FAILURE;
				channelHandlerCloseChannel(pHandler, pChannelInfo, &error);
				channelClosed = RSSL_TRUE;
				ret = RSSL_RET_SUCCESS;
			}
		}
	} while(ret > RSSL_RET_SUCCESS);

	if (channelClosed)
		return RSSL_RET_FAILURE;

	switch(ret)
	{
		case RSSL_RET_SUCCESS:
		case RSSL_RET_READ_WOULD_BLOCK:
		case RSSL_RET_READ_FD_CHANGE:
		case RSSL_RET_READ_IN_PROGRESS:
			return RSSL_RET_SUCCESS;
		case RSSL_RET_READ_PING:
			pChannelInfo->receivedMsg = RSSL_TRUE; /* Mark that we received data for ping timeout handling. */
			return RSSL_RET_SUCCESS;
		default:
			channelHandlerCloseChannel(pHandler, pChannelInfo, &error);
			return ret;
	}
}

RsslRet channelHandlerInitializeChannel(ChannelHandler *pHandler, ChannelInfo *pChannelInfo)
{
	RsslChannel *pChannel = pChannelInfo->pChannel;
	RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;
	RsslRet ret;
	RsslError error;

	_flushDone(pHandler, pChannelInfo);
	ret = rsslInitChannel(pChannel, &inProg, &error);
	switch (ret)
	{
		case RSSL_RET_CHAN_INIT_IN_PROGRESS:
			if (inProg.flags & RSSL_IP_FD_CHANGE) /* Set write fd on next select call */
				channelHandlerRequestFlush(pHandler, pChannelInfo); 
			break;

		case RSSL_RET_SUCCESS:
			/* Channel is now active! */
			_processActiveChannel(pHandler, pChannelInfo);
			break;

		default:
			channelHandlerCloseChannel(pHandler, pChannelInfo, &error);
			break;

	}

	return ret;
}

RsslRet channelHandlerWaitForChannelInit(ChannelHandler *pHandler, ChannelInfo *pChannelInfo,
		RsslUInt32 waitTimeUsec)
{
	int selRet;
	fd_set useReadFds;
	fd_set useExceptFds;
	fd_set useWriteFds;
	struct timeval time_interval;

	FD_ZERO(&useReadFds);
	FD_ZERO(&useExceptFds);
	FD_ZERO(&useWriteFds);

	FD_SET(pChannelInfo->pChannel->socketId, &useReadFds);
	FD_SET(pChannelInfo->pChannel->socketId, &useExceptFds);

	/* needFlush indicates if we want to call rsslInitChannel() immediately. */ 
	if (pChannelInfo->needFlush)
		FD_SET(pChannelInfo->pChannel->socketId, &useWriteFds);

	time_interval.tv_sec = 0;
	time_interval.tv_usec = waitTimeUsec;

	selRet = select(pChannelInfo->pChannel->socketId+1, &useReadFds, &useWriteFds, 
			&useExceptFds, &time_interval);


	if (selRet > 0)
		return channelHandlerInitializeChannel(pHandler, pChannelInfo);
	else if (selRet == 0)
		return RSSL_RET_CHAN_INIT_IN_PROGRESS;
#ifdef WIN32
	else if (WSAGetLastError() == WSAEINTR)
#else 
	else if (errno == EINTR)
#endif
		return RSSL_RET_CHAN_INIT_IN_PROGRESS;

	perror("select");
	exit(-1);
	return RSSL_RET_FAILURE;
}


void channelHandlerReadChannels(ChannelHandler *pHandler, TimeValue stopTimeNsec)
{
	int selRet;
	RsslQueueLink *pLink;

	TimeValue currentTime;

	fd_set useReadFds;
	fd_set useExceptFds;
	fd_set useWriteFds;

	/* Loop on select(), looking for channels with available data, until stopTimeNsec is reached. */
	do
	{


#ifdef WIN32
		/* Windows does not allow select() to be called with empty file descriptor sets. */
		if (rsslQueueGetElementCount(&pHandler->initializingChannelList) + rsslQueueGetElementCount(&pHandler->activeChannelList) == 0)
		{
			currentTime = getTimeNano();
			selRet = 0;
			Sleep( (DWORD)((currentTime < stopTimeNsec) ? (stopTimeNsec - currentTime)/1000000 : 0));
		}
		else
#endif
		{
			struct timeval time_interval;
			RsslSocket maxFd = 0;

			/* Clear descriptor sets. */
			FD_ZERO(&useReadFds);
			FD_ZERO(&useExceptFds);
			FD_ZERO(&useWriteFds);

			/* Add active channels to the descriptor sets. */
			RSSL_QUEUE_FOR_EACH_LINK(&pHandler->activeChannelList, pLink)
			{
				ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);

				FD_SET(pChannelInfo->pChannel->socketId, &useReadFds);
				FD_SET(pChannelInfo->pChannel->socketId, &useExceptFds);

				if (pChannelInfo->pChannel->socketId > maxFd)
					maxFd = pChannelInfo->pChannel->socketId;

				if (pChannelInfo->needFlush)
					FD_SET(pChannelInfo->pChannel->socketId, &useWriteFds);
			}

			/* Add initializing channels to the descriptor sets. */
			RSSL_QUEUE_FOR_EACH_LINK(&pHandler->initializingChannelList, pLink)
			{
				ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);

				FD_SET(pChannelInfo->pChannel->socketId, &useReadFds);
				FD_SET(pChannelInfo->pChannel->socketId, &useExceptFds);

				if (pChannelInfo->pChannel->socketId > maxFd)
					maxFd = pChannelInfo->pChannel->socketId;

				if (pChannelInfo->needFlush)
					FD_SET(pChannelInfo->pChannel->socketId, &useWriteFds);
			}

			currentTime = getTimeNano();
			time_interval.tv_usec = (long)((currentTime < stopTimeNsec) ? (stopTimeNsec - currentTime)/1000 : 0);
			time_interval.tv_sec = 0;

			selRet = select(maxFd+1, &useReadFds, &useWriteFds, &useExceptFds, &time_interval);
		}

		if (selRet == 0)
			return;
		else if (selRet > 0)
		{

			RsslRet		ret = RSSL_RET_SUCCESS;
			RsslError	error;

			RSSL_QUEUE_FOR_EACH_LINK(&pHandler->activeChannelList, pLink)
			{
				ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
				RsslChannel *pChannel = pChannelInfo->pChannel;
				RsslBool channelClosed = RSSL_FALSE;

				/* Check if each channel has something to read. */
				if ( FD_ISSET(pChannel->socketId, &useReadFds) || FD_ISSET(pChannel->socketId, &useExceptFds))
				{
					if (channelHandlerReadChannel(pHandler, pChannelInfo) < RSSL_RET_SUCCESS)
						continue;
				}

				/* Check if we can flush data from any channels. */
				if ( FD_ISSET(pChannel->socketId, &useWriteFds))
				{
					if ((ret = rsslFlush(pChannel, &error)) < RSSL_RET_SUCCESS)
					{
						channelHandlerCloseChannel(pHandler, pChannelInfo, &error);
						continue;
					}
					else if (ret == RSSL_RET_SUCCESS)
					{
						/* rsslFlush() returned 0 instead of a higher value, so there's no more data to flush. */
						_flushDone(pHandler, pChannelInfo);
					}
				}
			}

			/* Try to initialize channels. */
			RSSL_QUEUE_FOR_EACH_LINK(&pHandler->initializingChannelList, pLink)
			{
				ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
				RsslChannel *pChannel = pChannelInfo->pChannel;

				if ((FD_ISSET(pChannel->socketId, &useReadFds)) ||
						(FD_ISSET(pChannel->socketId, &useExceptFds)) ||
						(FD_ISSET(pChannel->socketId, &useWriteFds)))
				{
					RsslInProgInfo inProg = RSSL_INIT_IN_PROG_INFO;

					_flushDone(pHandler, pChannelInfo);
					channelHandlerInitializeChannel(pHandler, pChannelInfo);
				}
			}
		}
#ifdef WIN32
		else if (WSAGetLastError() != WSAEINTR)
#else 
		else if (errno != EINTR)
#endif
		{
			perror("select");
			exit(-1);
		}
	} while (currentTime < stopTimeNsec);

}

void channelHandlerCheckPings(ChannelHandler *pHandler)
{
	TimeValue currentTime;
	RsslRet ret;
	RsslQueueLink *pLink;
	RsslError rsslError;

	/* get current time */
	currentTime = getTimeNano();

	RSSL_QUEUE_FOR_EACH_LINK(&pHandler->activeChannelList, pLink)
	{
		ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
		TimeValue pingTimeoutNsec = (RsslInt64)pChannelInfo->pChannel->pingTimeout * 1000000000LL;

		if (!pChannelInfo->checkPings)
			continue;

		/* handle sending pings to clients */
		if (currentTime >= pChannelInfo->nextSendPingTime)
		{
			ret = rsslPing(pChannelInfo->pChannel, &rsslError);

			if (ret > RSSL_RET_SUCCESS) 
				channelHandlerRequestFlush(pHandler, pChannelInfo);
			else if (ret < RSSL_RET_SUCCESS)
			{
				channelHandlerCloseChannel(pHandler, pChannelInfo, &rsslError); /* Remove client if sending the message failed */
				continue;
			}


			/* set time to send next ping to client */
			pChannelInfo->nextSendPingTime = currentTime + pingTimeoutNsec/3;
		}

		/* handle receiving pings from client */
		if ( currentTime >= pChannelInfo->nextReceivePingTime)
		{
			/* check if server received message from client since last time */
			if (pChannelInfo->receivedMsg)
			{
				/* reset flag for client message received */
				pChannelInfo->receivedMsg = RSSL_FALSE;

				/* set time server should receive next message/ping from client */
				pChannelInfo->nextReceivePingTime = currentTime + pingTimeoutNsec;
			}
			else /* lost contact with client */
			{
				snprintf(rsslError.text, sizeof(rsslError.text), "Ping timed out.");
				rsslError.rsslErrorId = RSSL_RET_FAILURE;
				channelHandlerCloseChannel(pHandler, pChannelInfo, &rsslError);
			}
		}
	}
}

void channelHandlerCleanup(ChannelHandler *pHandler)
{
	RsslQueueLink *pLink;
	RSSL_QUEUE_FOR_EACH_LINK(&pHandler->activeChannelList, pLink)
	{
		ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
		channelHandlerCloseChannel(pHandler, pChannelInfo, NULL);
	}
}
