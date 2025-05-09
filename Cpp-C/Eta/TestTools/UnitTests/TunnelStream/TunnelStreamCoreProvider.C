/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "TunnelStreamCoreProvider.h"
#include "rtr/msgQueueEncDec.h"
#include "gtest/gtest.h"

using namespace testing;

void TunnelStreamCoreProvider::submitTunnelStreamMsg(TunnelStreamData* pTunnelStreamData, RsslMsg* pMsg)
{
	RsslInt ret;
	RsslUInt32 bufferSize = 256;

	ASSERT_TRUE(pMsg != NULL);
		
	do
	{
		RsslBuffer* pBuffer = rsslGetBuffer(_pChannel, bufferSize, RSSL_FALSE, &_error);
		ASSERT_TRUE(pBuffer != NULL);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorRWFVersion(&_eIter, _pChannel->majorVersion, _pChannel->minorVersion));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorBuffer(&_eIter, pBuffer));
		ret = tunnelStreamDataEncodeInit(&_eIter, pTunnelStreamData, COS_CURRENT_STREAM_VERSION);
		if (ret == RSSL_RET_SUCCESS)
			ret = rsslEncodeMsg(&_eIter, pMsg);
		/*if (ret == RSSL_RET_SUCCESS)
			ret = tunnelStreamDataEncodeComplete(&_eIter); // complete function not needed */
			
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

void TunnelStreamCoreProvider::acceptTunnelStreamRequest(RsslRequestMsg* pRequestMsg, RsslClassOfService* pProvClassOfService)
{
	RsslBuffer* pBuffer;
	RsslRefreshMsg refreshMsg;
	RsslClassOfService cos;
	RsslClassOfService* pCos;

	ASSERT_TRUE(rsslRequestMsgCheckStreaming(pRequestMsg));
	ASSERT_TRUE(rsslRequestMsgCheckPrivateStream(pRequestMsg));
	ASSERT_TRUE(rsslRequestMsgCheckQualifiedStream(pRequestMsg));
	ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pRequestMsg->msgBase.msgKey));
	ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pRequestMsg->msgBase.msgKey));
	ASSERT_EQ(RSSL_DT_FILTER_LIST, pRequestMsg->msgBase.containerType);
		
	rsslClearDecodeIterator(&_dIter);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorRWFVersion(&_dIter, _pChannel->majorVersion, _pChannel->minorVersion));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetDecodeIteratorBuffer(&_dIter, &pRequestMsg->msgBase.encDataBody));

	if (pProvClassOfService == NULL)
	{
		/* Echo consumer's class of service. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeClassOfService(&_dIter, &cos, NULL, &_errorInfo));
		pCos = &cos;
	}
	else
		pCos = pProvClassOfService;
		
	pBuffer = rsslGetBuffer(_pChannel, MSG_BUF_LEN, RSSL_FALSE, &_error);
	ASSERT_TRUE(pBuffer != NULL);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorRWFVersion(&_eIter, _pChannel->majorVersion, _pChannel->minorVersion));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorBuffer(&_eIter, pBuffer));
		
	/* Encode tunnel stream refresh. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	rsslRefreshMsgApplyPrivateStream(&refreshMsg);
	rsslRefreshMsgApplyQualifiedStream(&refreshMsg);
	rsslRefreshMsgApplySolicited(&refreshMsg);
	rsslRefreshMsgApplyRefreshComplete(&refreshMsg);
	rsslRefreshMsgApplyDoNotCache(&refreshMsg);
	refreshMsg.msgBase.domainType = pRequestMsg->msgBase.domainType;
	refreshMsg.msgBase.streamId = (pRequestMsg->msgBase.streamId);
	rsslRefreshMsgApplyHasMsgKey(&refreshMsg);
	rsslMsgKeyApplyHasServiceId(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.serviceId = pRequestMsg->msgBase.msgKey.serviceId;
	rsslMsgKeyApplyHasName(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.name = pRequestMsg->msgBase.msgKey.name;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;
	RsslBuffer refreshText;
	char refreshString[255];
	snprintf(refreshString, 255,"Successful open of TunnelStream: %s", pRequestMsg->msgBase.msgKey.name.data);
	refreshText.data = refreshString;
	refreshText.length = (RsslUInt32)strlen(refreshString);
	refreshMsg.state.text = refreshText;
	refreshMsg.msgBase.containerType = RSSL_DT_FILTER_LIST;
	rsslMsgKeyApplyHasFilter(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.filter = pRequestMsg->msgBase.msgKey.filter;
		
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg *)&refreshMsg, 0));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeClassOfService(&_eIter, pCos, pRequestMsg->msgBase.msgKey.filter, true, COS_CURRENT_STREAM_VERSION, &_errorInfo));
		
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, true));
		
	writeBuffer(pBuffer);
}

void TunnelStreamCoreProvider::submitTunnelStreamAck(TunnelStreamAck* pTunnelStreamAck)
{
	submitTunnelStreamAck(pTunnelStreamAck, NULL, NULL, 0);
}
	
void TunnelStreamCoreProvider::submitTunnelStreamAck(TunnelStreamAck* pTunnelStreamAck, AckRangeList* pAckRangeList, AckRangeList* pNakRangeList, RsslInt actionOpCode)
{
	RsslInt ret;
	RsslInt bufferSize = 256;
		
	do
	{
		RsslBuffer* pBuffer = rsslGetBuffer(_pChannel, MSG_BUF_LEN, RSSL_FALSE, &_error);
		ASSERT_TRUE(pBuffer != NULL);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorRWFVersion(&_eIter, _pChannel->majorVersion, _pChannel->minorVersion));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslSetEncodeIteratorBuffer(&_eIter, pBuffer));
		ret = tunnelStreamAckEncode(&_eIter, pTunnelStreamAck, pAckRangeList, pNakRangeList);
			
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
