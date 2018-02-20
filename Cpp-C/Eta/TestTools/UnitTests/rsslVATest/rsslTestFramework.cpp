/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#if defined(_WIN32)
#include <windows.h>
#include <winsock2.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif
#include "rsslTestFramework.h"


/* timeMultiplier multiples all timeouts by the specified value. Handy when
 * running in slower environments (debugger, valgrind). This is set differently on
 * each OS as some may be slower than others (this may just be the machines I am using
 * to test. If time allows this should probably be made a command-line option).*/
static RsslUInt32 timeMultiplier;
#ifdef WIN32
static RsslUInt32 baseTimeMultiplier = 3;
#elif defined(Linux)
static RsslUInt32 baseTimeMultiplier = 1;
#else
static RsslUInt32 baseTimeMultiplier = 2;
#endif


static RsslConnectionTypes rsslTestConnectionType;
static RsslBool rsslTestInitialized = RSSL_FALSE;
static RsslServer *pServer = NULL;

static RsslBuffer applicationName = { 16, (char*)"rsslVATest" } ;
static RsslBuffer applicationId = { 3, (char*)"256" };
static RsslBuffer loginConfigPosition = {9, (char*)"localhost"};

void rsslTestInitialize(rsslTestInitOpts *pOpts)
{
	RsslError rsslError;
#ifdef WIN32
	Sleep(200);	/* Server might have been closed recently. Give windows a chance to release it. */
#endif
	rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);
	rsslTestConnectionType = pOpts->connectionType;
	timeMultiplier = baseTimeMultiplier;

	// multicast connections do not have a server.
	if (rsslTestConnectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		RsslBindOptions bindOpts;
		RsslError error;

		rsslClearBindOpts(&bindOpts); 
		bindOpts.serviceName = const_cast<char*>("14011");
		ASSERT_TRUE((pServer = rsslBind(&bindOpts, &error)));
	}
	else
	{
		timeMultiplier *= 3;	// multicast connections are slower
	}

	rsslTestInitialized = RSSL_TRUE;
}

void rsslTestUninitialize()
{
	RsslError error;

	if (rsslTestConnectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
		rsslCloseServer(pServer, &error);

	pServer = NULL;
	rsslTestInitialized = RSSL_FALSE;
	rsslUninitialize();
}

RsslChannel *rsslTestCreateConsumerChannel()
{
	RsslConnectOptions connectOpts;
	RsslChannel *pChannel;
	RsslError rsslError;

	rsslClearConnectOpts(&connectOpts);
	connectOpts.connectionType = rsslTestConnectionType;
	if (rsslTestConnectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		connectOpts.connectionInfo.unified.address = const_cast<char*>("localhost");
		connectOpts.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	}
	else
	{
		connectOpts.connectionInfo.segmented.recvAddress = const_cast<char*>("235.1.1.1");
		connectOpts.connectionInfo.segmented.recvServiceName = const_cast<char*>("15011");
		connectOpts.connectionInfo.segmented.sendAddress = const_cast<char*>("235.1.1.1");
		connectOpts.connectionInfo.segmented.sendServiceName = const_cast<char*>("15011");
		connectOpts.connectionInfo.segmented.unicastServiceName = const_cast<char*>("16010");
		connectOpts.connectionInfo.segmented.interfaceName = const_cast<char*>("localhost");
	}

	EXPECT_TRUE((pChannel = rsslConnect(&connectOpts, &rsslError))) << 
		printf("rsslTestCreateConsumerChannel: rsslConnect() failed: %d(%s)\n", rsslError.rsslErrorId, rsslError.text);
	
	return pChannel;
}

RsslChannel *rsslTestCreateProviderChannel()
{
	RsslChannel *pChannel;

	if (rsslTestConnectionType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		RsslAcceptOptions acceptOpts;
		RsslError rsslError;
		struct timeval time_interval;
		fd_set	readfds;
		fd_set	exceptfds;
		fd_set	wrtfds;
		int selRet;

		// make sure the rsslConnect has triggered the bind socket before calling rsslAccept();
		FD_ZERO(&wrtfds);
		FD_ZERO(&readfds);
		FD_ZERO(&exceptfds);
		FD_SET(pServer->socketId,&readfds);
		FD_SET(pServer->socketId,&exceptfds);
		time_interval.tv_sec = 10;
		time_interval.tv_usec = 0;
		selRet = select(FD_SETSIZE, &readfds, &wrtfds, &exceptfds, &time_interval);

		rsslClearAcceptOpts(&acceptOpts);
		EXPECT_TRUE((pChannel = rsslAccept(pServer, &acceptOpts, &rsslError))) <<
			printf("rsslTestCreateProviderChannel: rsslAccept() failed: %d(%s)\n", rsslError.rsslErrorId, rsslError.text);
	}
	else
	{
		RsslConnectOptions rsslConnectOptions;
		RsslError rsslError;

		rsslClearConnectOpts(&rsslConnectOptions);
		rsslConnectOptions.connectionInfo.segmented.recvAddress = const_cast<char*>("235.1.1.1");
		rsslConnectOptions.connectionInfo.segmented.recvServiceName = const_cast<char*>("15011");
		rsslConnectOptions.connectionInfo.segmented.sendAddress = const_cast<char*>("235.1.1.1");
		rsslConnectOptions.connectionInfo.segmented.sendServiceName = const_cast<char*>("15011");
		rsslConnectOptions.connectionInfo.segmented.unicastServiceName = const_cast<char*>("16011");
		rsslConnectOptions.connectionInfo.segmented.interfaceName = const_cast<char*>("localhost");
		rsslConnectOptions.connectionType = RSSL_CONN_TYPE_RELIABLE_MCAST;
		rsslConnectOptions.multicastOpts.flags = RSSL_MCAST_FILTERING_ON;
	
		EXPECT_TRUE(pChannel = rsslConnect(&rsslConnectOptions, &rsslError)) <<
			printf("rsslTestCreateProviderChannel: rsslConnect() failed: %d(%s)\n", rsslError.rsslErrorId, rsslError.text);
	}

	return pChannel;
}

void rsslTestInitChannels(RsslChannel *provChannel, RsslChannel *consChannel)
{
    RsslInProgInfo inProgInfo = RSSL_INIT_IN_PROG_INFO;
	RsslError error;
	RsslRet ret;
			
	while (provChannel->state != RSSL_CH_STATE_ACTIVE || consChannel->state != RSSL_CH_STATE_ACTIVE)
	{
		if (provChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ret = rsslInitChannel(provChannel, &inProgInfo, &error);
			ASSERT_FALSE (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_CHAN_INIT_IN_PROGRESS);
		}

		if (consChannel->state != RSSL_CH_STATE_ACTIVE)
		{
			ret = rsslInitChannel(consChannel, &inProgInfo, &error);
			ASSERT_FALSE (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_CHAN_INIT_IN_PROGRESS);
		}
	}
}


RsslRet sendDirectoryRequest(RsslChannel *pChannel, RsslInt32 streamId)
{
	RsslRDMDirectoryRequest directoryRequest;
	RsslRet ret;
	RsslEncodeIterator eIter;
	RsslBuffer *msgBuf;
	RsslErrorInfo	threadErrorInfo;

	/* Send Directory Request */
	if ((ret = rsslInitDefaultRDMDirectoryRequest(&directoryRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMDirectoryRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (!(msgBuf = rsslGetBuffer(pChannel, 128, RSSL_FALSE, &(threadErrorInfo.rsslError))))
	{
		rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
		return threadErrorInfo.rsslError.rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if ((ret = rsslEncodeRDMDirectoryMsg(&eIter, (RsslRDMDirectoryMsg*)&directoryRequest, &msgBuf->length, &threadErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	if ( sendMessage(pChannel, msgBuf) != RSSL_RET_SUCCESS)
		return ret;

	return RSSL_RET_SUCCESS;
}

RsslRet sendDictionaryRequests(RsslChannel *pChannel, RsslUInt16 serviceId, 
		RsslInt32 fieldDictionaryStreamId, RsslInt32 enumTypeDictionaryStreamId)
{
	RsslRDMDictionaryRequest dictionaryRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;
	RsslErrorInfo	threadErrorInfo;

	rsslClearRDMDictionaryRequest(&dictionaryRequest);
	dictionaryRequest.flags = RDM_DC_RQF_STREAMING;
	dictionaryRequest.serviceId = serviceId;
	
	/* Field dictionary request. */
	msgBuf = rsslGetBuffer(pChannel, 128, RSSL_FALSE, &(threadErrorInfo.rsslError));
	EXPECT_FALSE_RET_IF_FAILED(msgBuf == NULL, threadErrorInfo.rsslError.text, threadErrorInfo.rsslError.rsslErrorId);

	dictionaryRequest.dictionaryName.data = (char*)"RWFFld";
	dictionaryRequest.dictionaryName.length = 6;
	dictionaryRequest.rdmMsgBase.streamId = fieldDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, ret, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &threadErrorInfo);
	EXPECT_FALSE_RET_IF_FAILED( ret != RSSL_RET_SUCCESS, threadErrorInfo.rsslError.text, ret);

	if ( sendMessage(pChannel, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	/* Enumerated types dictionary request. */
	if (!(msgBuf = rsslGetBuffer(pChannel, 128, RSSL_FALSE, &(threadErrorInfo.rsslError))))
	{
		rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
		return (threadErrorInfo.rsslError).rsslErrorId;
	}

	dictionaryRequest.dictionaryName.data = (char*)"RWFEnum";
	dictionaryRequest.dictionaryName.length = 7;
	dictionaryRequest.rdmMsgBase.streamId = enumTypeDictionaryStreamId;
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__, 
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	ret = rsslEncodeRDMDictionaryMsg(&eIter, (RsslRDMDictionaryMsg*)&dictionaryRequest, &msgBuf->length, &threadErrorInfo);
	EXPECT_FALSE_RET_IF_FAILED( ret != RSSL_RET_SUCCESS, threadErrorInfo.rsslError.text, ret);

	if ( sendMessage(pChannel, msgBuf) != RSSL_RET_SUCCESS)
		 return ret;

	return RSSL_RET_SUCCESS;
}


RsslRet sendLoginRequest(RsslChannel *pChannel, RsslInt32 streamId)
{
	RsslRDMLoginRequest loginRequest;
	RsslRet ret;
	RsslBuffer *msgBuf;
	RsslEncodeIterator eIter;
	RsslErrorInfo	threadErrorInfo;

	/* Send Login Request */
	if ((ret = rsslInitDefaultRDMLoginRequest(&loginRequest, streamId)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslInitDefaultRDMLoginRequest() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}


	loginRequest.flags |= RDM_LG_RQF_HAS_ROLE | RDM_LG_RQF_HAS_APPLICATION_NAME;
	loginRequest.applicationName = applicationName;

	if (!(msgBuf = rsslGetBuffer(pChannel, 256, RSSL_FALSE, &(threadErrorInfo.rsslError))))
	{
		rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
		return (threadErrorInfo.rsslError).rsslErrorId;
	}

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
	if ( (ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) != RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfo(&threadErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_FAILURE, __FILE__, __LINE__,
				(char*)"rsslSetEncodeIteratorBuffer() failed: %d(%s)", ret, rsslRetCodeToString(ret));
		return ret;
	}

	ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRequest, &msgBuf->length, &threadErrorInfo);
	EXPECT_FALSE_RET_IF_FAILED(ret != RSSL_RET_SUCCESS, threadErrorInfo.rsslError.text, ret);

	ret = sendMessage(pChannel, msgBuf);
	EXPECT_FALSE_RET_IF_FAILED(ret != RSSL_RET_SUCCESS, "sendMessage() failed", ret);

	return RSSL_RET_SUCCESS;
}

RsslRet sendMessage(RsslChannel *pChannel, RsslBuffer *msgBuf)
{
	RsslUInt32 bytesWritten, uncompBytesWritten;
	RsslRet ret;
	RsslErrorInfo	threadErrorInfo;

	ret = rsslWrite(pChannel, msgBuf, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompBytesWritten, &(threadErrorInfo.rsslError));

	/* call flush and write again */
	while (ret == RSSL_RET_WRITE_CALL_AGAIN)
	{
		if ((ret = rsslFlush(pChannel, &(threadErrorInfo.rsslError))) < RSSL_RET_SUCCESS)
		{
			rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
			return ret;
		}
		ret = rsslWrite(pChannel, msgBuf, RSSL_HIGH_PRIORITY, 0, &bytesWritten, &uncompBytesWritten, &(threadErrorInfo.rsslError));
	}

	if (ret < RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
		return ret;
	}

	/* Call rsslFlush() to ensure that the submitted message buffer is sent to network properly.*/
	if ((ret = rsslFlush(pChannel, &(threadErrorInfo.rsslError))) < RSSL_RET_SUCCESS)
	{
		rsslSetErrorInfoLocation(&threadErrorInfo, __FILE__, __LINE__);
		return ret;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet processLoginRequest(RsslChannel *pChannel, RsslMsg* msg, RsslDecodeIterator* dIter)
{
	RsslRet ret;
	RsslRDMLoginMsg loginMsg;
	char loginMsgChar[4000];
	RsslBuffer memoryBuffer = { 4000, loginMsgChar };
	RsslErrorInfo threadErrorInfo;
	
	ret = rsslDecodeRDMLoginMsg(dIter, msg, &loginMsg, &memoryBuffer, &threadErrorInfo);
	EXPECT_FALSE_RET_IF_FAILED(ret != RSSL_RET_SUCCESS, threadErrorInfo.rsslError.text, ret);

	switch(loginMsg.rdmMsgBase.rdmMsgType)
	{
	case RDM_LG_MT_REQUEST:
	{
		RsslError error;
		RsslBuffer* msgBuf = 0;
		RsslRet ret;
		RsslEncodeIterator eIter;
		RsslRDMLoginRefresh loginRefresh;

		/* get a buffer for the login response */
		msgBuf = rsslGetBuffer(pChannel, 512, RSSL_FALSE, &error);
		EXPECT_FALSE_RET_IF_FAILED( msgBuf == NULL, error.text, error.rsslErrorId);
		
		rsslClearRDMLoginRefresh(&loginRefresh);

		/* provide login response information */
		/* StreamId */
		loginRefresh.rdmMsgBase.streamId = loginMsg.rdmMsgBase.streamId;

		/* Username */
		loginRefresh.userName = loginMsg.request.userName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;

		/* ApplicationId */
		loginRefresh.applicationId = applicationId;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;

		/* ApplicationName */
		loginRefresh.applicationName = applicationName;
		loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;

		/* Position */
		loginRefresh.position = loginConfigPosition;
		loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;

		loginRefresh.singleOpen = 0; /* this provider does not support SingleOpen behavior */
		loginRefresh.flags |= RDM_LG_RFF_HAS_SINGLE_OPEN;

		loginRefresh.supportOMMPost = 1;
		loginRefresh.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;

		loginRefresh.flags |= RDM_LG_RFF_SOLICITED;

		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
		rsslSetEncodeIteratorBuffer(&eIter, msgBuf);

		ret = rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &threadErrorInfo);
		EXPECT_FALSE_RET_IF_FAILED( ret != RSSL_RET_SUCCESS, threadErrorInfo.rsslError.text, ret);

		/* send login response */
		ret = sendMessage(pChannel, msgBuf);
		EXPECT_FALSE(ret != RSSL_RET_SUCCESS);

		return ret;
	}

	case RDM_LG_MT_CLOSE:
		return RSSL_RET_SUCCESS;

	default:
		printf("\nReceived Unhandled Login Msg Class: %d\n", msg->msgBase.msgClass);
    	return RSSL_RET_FAILURE;
	}
}

