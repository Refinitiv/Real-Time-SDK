/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "msgEncDec.h"
#include "mapEncDec.h"
#include "fieldListEncDec.h"
#include "seriesEncDec.h"

#include "rtr/rsslRDM.h"
#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslDataUtils.h"


/* Begin Simple message example implementation */

/* This function encodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_MAP.  For the payload of the message, 
just have it call the map encoding function.  */
RsslRet exampleEncodeRefreshMsgWithMap(RsslEncodeIterator *encIter)
{
	/* use this to store and check return codes */
	RsslRet retVal;
	RsslBool success = RSSL_TRUE;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;  /* message is a refresh */
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_BY_ORDER;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;

	/* Use the stream Id corresponding to the request, because it is in reply to a request, its solicited */
	refreshMsg.msgBase.streamId = 6; 

	/* Populate flags for refresh message members and behavior ûbecause this in response to a request
	   This should be solicited, msgKey should be present, single part refresh so it is complete,
	   and also want the concrete qos of the stream */
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE |
		RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;

	/* Populate msgKey to specify a serviceId, a name with type of RIC (which is default nameType) */
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME ;
	refreshMsg.msgBase.msgKey.serviceId = 1;
	/* Specify name and length of name.  Because this is a RIC, no nameType is required.  */
	refreshMsg.msgBase.msgKey.name.data = (char *)"TRI";
	refreshMsg.msgBase.msgKey.name.length = 3;

	// set RsslRefreshMsg state information
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;

	/* Qos */
	refreshMsg.qos.dynamic = RSSL_FALSE;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	
	/* Now that everything is set on my message I can encode the message header */

	/* Since I have message payload of a RsslMap that I am going to encode after this,
	  I should use the rsslEncodeMsgInit function */

	/* begin encoding of message - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with
	   buffer and version information, store return value to determine success or failure */
	/* data max encoded size is unknown so 0 is used */
	if ((retVal = rsslEncodeMsgInit(encIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeMsgInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}

	printf("\tMessage Header Encoded for Item %.*s\n", refreshMsg.msgBase.msgKey.name.length, refreshMsg.msgBase.msgKey.name.data);

	/* I am now ready to populate and encode my RsslMap and its contents */
	if ((retVal = exampleEncodeMap(encIter)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		printf("<%s:%d> Error encoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeMap().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}
	
	/* since our RsslMap encoder completed successfully, lets complete encode the message */
	if ((retVal = rsslEncodeMsgComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeMsgComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tMessage Encoding Complete\n");

	return RSSL_RET_SUCCESS;

}


/* This function encodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_SERIES.  For the payload of the message, 
just have it call the series encoding function.  */
RsslRet exampleEncodeRefreshMsgWithSeries(RsslEncodeIterator *encIter)
{
	/* use this to store and check return codes */
	RsslRet retVal;
	RsslBool success = RSSL_TRUE;

	/* Populate and encode a refreshMsg */
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;  /* message is a refresh */
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_BY_ORDER;
	refreshMsg.msgBase.containerType = RSSL_DT_SERIES;

	/* Use the stream Id corresponding to the request, because it is in reply to a request, its solicited */
	refreshMsg.msgBase.streamId = 7; 

	/* Populate flags for refresh message members and behavior  because this in response to a request
	   This should be solicited, msgKey should be present, single part refresh so it is complete,
	   and also want the concrete qos of the stream */
	refreshMsg.flags = RSSL_RFMF_SOLICITED | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE |
		RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;

	/* Populate msgKey to specify a serviceId, a name with type of RIC (which is default nameType) */
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME ;
	refreshMsg.msgBase.msgKey.serviceId = 1;
	/* Specify name and length of name.  Because this is a RIC, no nameType is required.  */
	refreshMsg.msgBase.msgKey.name.data = (char *)"TRI";
	refreshMsg.msgBase.msgKey.name.length = 3;

	// set RsslRefreshMsg state information
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.code = RSSL_SC_NONE;

	/* Qos */
	refreshMsg.qos.dynamic = RSSL_FALSE;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	
	/* Now that everything is set on my message I can encode the message header */

	/* Since I have message payload of a RsslMap that I am going to encode after this,
	  I should use the rsslEncodeMsgInit function */

	/* begin encoding of message - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with
	   buffer and version information, store return value to determine success or failure */
	/* data max encoded size is unknown so 0 is used */
	if ((retVal = rsslEncodeMsgInit(encIter, (RsslMsg*)&refreshMsg, 0)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeMsgInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}

	printf("\tMessage Header Encoded for Item %.*s\n", refreshMsg.msgBase.msgKey.name.length, refreshMsg.msgBase.msgKey.name.data);

	/* I am now ready to populate and encode my RsslMap and its contents */
	if ((retVal = exampleEncodeSeries(encIter)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		printf("<%s:%d> Error encoding series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeSeries().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}
	
	/* since our RsslMap encoder completed successfully, lets complete encode the message */
	if ((retVal = rsslEncodeMsgComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeMsgComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tMessage Encoding Complete\n");

	return RSSL_RET_SUCCESS;

}



/* This function decodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_MAP.  For the payload of the message, 
just have it call the map decoding function.  */
RsslRet exampleDecodeRefreshMsgWithMap(RsslDecodeIterator *decIter)
{
	/* use this to store and check return codes */
	RsslRet retVal;

	/* Create a RsslMsg to decode into */
	RsslMsg msg = RSSL_INIT_MSG; 

	/* decode the message header */

	/* This function will only decode the message header.  Any message 
	   payload will be contained inside of the message structure encData member */
	if ((retVal = rsslDecodeMsg(decIter, &msg)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding message header.\n", __FILE__, __LINE__);
		return retVal;
	}
	
	printf("Message Header Decoded for Item %.*s\n", msg.msgBase.msgKey.name.length, msg.msgBase.msgKey.name.data);

	if ((retVal = exampleDecodeMap(decIter)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		printf("<%s:%d> Error decoding map.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeMap().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}

	printf("\tMessage Decoding Complete\n");
		
	return RSSL_RET_SUCCESS;

}

/* This function decodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_SERIES.  For the payload of the message, 
just have it call the map decoding function.  */
RsslRet exampleDecodeRefreshMsgWithSeries(RsslDecodeIterator *decIter)
{
	/* use this to store and check return codes */
	RsslRet retVal;

	/* Create a RsslMsg to decode into */
	RsslMsg msg = RSSL_INIT_MSG; 

	/* decode the message header */

	/* This function will only decode the message header.  Any message 
	   payload will be contained inside of the message structure encData member */
	if ((retVal = rsslDecodeMsg(decIter, &msg)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error decoding message header.\n", __FILE__, __LINE__);
		return retVal;
	}
		
	printf("Message Header Decoded for Item %.*s\n", msg.msgBase.msgKey.name.length, msg.msgBase.msgKey.name.data);

	if ((retVal = exampleDecodeSeries(decIter)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		printf("<%s:%d> Error decoding series.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleDecodeSeries().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}

	printf("\tMessage Decoding Complete\n");
		
	return RSSL_RET_SUCCESS;

}
