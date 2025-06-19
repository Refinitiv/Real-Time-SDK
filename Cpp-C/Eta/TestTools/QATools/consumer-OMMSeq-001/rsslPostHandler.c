/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslPostHandler.h"
#include "rsslDirectoryHandler.h"
#include "rsslDictionaryHandler.h"
#include "rsslLoginConsumer.h"
#include "rsslSendMessage.h"
#include "rsslMarketPriceHandler.h"

// for FIDs and item info
#include "rsslMarketPriceItems.h"

/* system files for time operations */
#ifdef _WIN32
#include <time.h>
#include <process.h>
#include <winsock2.h>
#define getpid _getpid
#else
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>
#endif


/* the frequencey at which post messages are sent */
#define POST_MESSAGE_FREQUENCY 5 /* Post message send frequency in seconds */

static time_t nextPostTime = 0;
static RsslBool postWithMsg = RSSL_TRUE;
static RsslBool	shouldOffstreamPost = RSSL_FALSE, shouldOnstreamPost = RSSL_FALSE;
static RsslBool offstreamPostSent = RSSL_FALSE;
static RsslBool isRefreshComplete = RSSL_FALSE;

static RsslUInt32 nextPostId = 1;
static RsslUInt32 nextSeqNum = 1;
static double itemData = 12.00;

/* enables offstream posting mode */
void enableOffstreamPost()
{
	shouldOffstreamPost = RSSL_TRUE;
}

/* disables offstream posting mode */
void disableOffstreamPost()
{
	shouldOffstreamPost = RSSL_FALSE;
}

/* enables onstream posting mode */
void enableOnstreamPost()
{
	shouldOnstreamPost = RSSL_TRUE;
}

/* disables onstream posting mode */
void disableOnstreamPost()
{
	shouldOnstreamPost = RSSL_FALSE;
}

/* increases the value of the data being posted */
static double getNextItemData()
{
	itemData += 0.01;
	return itemData;
}

/*
 * Initializes timer for Post messages to be sent
 * This function simply gets the current time and 
 * sets up the first time that a post message should
 * be sent based off of that and the post interval 
 */
void initPostHandler()
{
	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	nextPostTime = currentTime + (time_t)POST_MESSAGE_FREQUENCY;

}

/*
 * Uses the current time and the nextPostTime to
 * determine whether a postMsg should be sent. 
 * If a post message should be sent, the time is
 * calculated for the next post after this one.  
 * If the postMsg is sent after a connection recovery
 * it is delayed to give a change to the application
 * to login. 
 * Additionally, the postWithMsg flag is toggled
 * so that posting alternates between posting
 * with a full message as payload or data as payload.  
 */
RsslRet handlePosts(RsslChannel* chnl, RsslBool connectionRecovery)
{

	time_t currentTime = 0;

	/* get current time */
	time(&currentTime);

	/* if connection recovery delay the post message and send a refresh */
	if (connectionRecovery)
	{
		isRefreshComplete = RSSL_FALSE;
		nextPostTime = currentTime + (time_t)POST_MESSAGE_FREQUENCY;
	}

	if (currentTime >= nextPostTime)
	{
		if (shouldOnstreamPost)
		{
			if (sendOnstreamPostMsg(chnl, postWithMsg) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}
		if (shouldOffstreamPost)
		{
			if (sendOffstreamPostMsg(chnl) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;
		}

		nextPostTime = currentTime + (time_t)POST_MESSAGE_FREQUENCY;

		/* iterate between post with msg and post with data */
		if (postWithMsg == RSSL_TRUE)
		{
			postWithMsg = RSSL_FALSE;
		}
		else
		{
			postWithMsg = RSSL_TRUE;
		}
	}

	return RSSL_RET_SUCCESS;
}




/* This function is internally used by sendPostMsg */
/* It encodes a PostMsg, populating the postUserInfo
 * with the IPAddress and process ID of the machine
 * running the application.  
 * The payload of the PostMsg is an UpdateMsg.  The
 * UpdateMsg contains a FieldList containing several fields. 
 * The same message encoding functionality used by the 
 * provider application is leveraged here to encode the 
 * contents.
 */
static RsslRet encodePostWithMsg(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId, RsslBuffer *itemName)
{
	RsslRet ret = 0;
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslItemInfo itemInfo;
	RsslMarketPriceItem mpItemInfo;
	RsslDataDictionary* dictionary = getDictionary();
	RsslBool isSolicited = RSSL_FALSE; // ??? for post
	RsslUInt16 serviceId = (RsslUInt16)getServiceId();
	RsslBuffer payloadMsgBuf = RSSL_INIT_BUFFER;
	RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslBuffer hostName = RSSL_INIT_BUFFER;
	char hostNameBuf[256];

	// First encode message for payload
	

	initMarketPriceItemFields(&mpItemInfo);
	mpItemInfo.TRDPRC_1 = getNextItemData();
	mpItemInfo.BID = getNextItemData();
	mpItemInfo.ASK = getNextItemData();
	strncpy(itemInfo.Itemname, itemName->data, itemName->length);
	itemInfo.Itemname[itemName->length] = '\0';
	if (isRefreshComplete)
	{
		itemInfo.IsRefreshComplete = RSSL_TRUE;
	}
	else
	{
		itemInfo.IsRefreshComplete = RSSL_FALSE;
		isRefreshComplete = RSSL_TRUE;
	}
	itemInfo.domainType = RSSL_DMT_MARKET_PRICE;
	itemInfo.itemData = (void*)&mpItemInfo;
	
	/* set-up message */
	postMsg.msgBase.msgClass = RSSL_MC_POST;
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_MSG;

	// Note: post message key not required for on-stream post
	postMsg.flags = RSSL_PSMF_POST_COMPLETE 
		| RSSL_PSMF_ACK // request ACK
		| RSSL_PSMF_HAS_POST_ID
		| RSSL_PSMF_HAS_SEQ_NUM
		| RSSL_PSMF_HAS_POST_USER_RIGHTS
		| RSSL_PSMF_HAS_MSG_KEY;

	postMsg.postId = nextPostId++;
	postMsg.seqNum = nextSeqNum++;
	postMsg.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
					rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
			return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();

	postMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	postMsg.msgBase.msgKey.name.data = itemName->data;
	postMsg.msgBase.msgKey.name.length = itemName->length;

	postMsg.msgBase.msgKey.nameType = 1;
	postMsg.msgBase.msgKey.serviceId = (RsslUInt16)getServiceId();
	
	// encode message 
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&postMsg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}
	
	/* although we are encoding an RWF type inside this message, 
	   this function is used to extract a buffer to encode into.  
	   This is done because the encodeMarketPriceResponse is shared by 
	   provider applications so it expects to encode the message into 
	   a stand alone buffer.  This could have been pre-encoded as well */
	ret = rsslEncodeNonRWFDataTypeInit(&encodeIter, &payloadMsgBuf);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret =  encodeMarketPriceResponse(chnl, &itemInfo, &payloadMsgBuf, isSolicited, streamId, RSSL_TRUE, RSSL_FALSE, serviceId, dictionary);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: encodeMarketPriceResponse() failed\n");
		return RSSL_RET_FAILURE;
	}

	ret = rsslEncodeNonRWFDataTypeComplete(&encodeIter, &payloadMsgBuf, RSSL_TRUE);
	if (ret != RSSL_RET_SUCCESS)
	{
		printf("encodePostWithMsg: rsslEncodeNonRWFDataTypeInit() failed\n");
		return RSSL_RET_FAILURE;
	}


	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);



	printf ("\n\nSENDING POST WITH MESSAGE:\n");
	printf ("	streamId = %d\n", postMsg.msgBase.streamId);
	printf ("	postId   = %d\n", postMsg.postId);
	printf ("	seqNum   = %d\n", postMsg.seqNum);

	return RSSL_RET_SUCCESS;

}

/* This function is internally used by sendPostMsg */
/* It encodes a PostMsg, populating the postUserInfo
 * with the IPAddress and process ID of the machine
 * running the application.  
 * The payload of the PostMsg is a FieldList containing 
 * several fields. 
 */
static RsslRet encodePostWithData(RsslChannel* chnl, RsslBuffer* msgBuf, RsslInt32 streamId)
{
	RsslRet ret = 0;
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslReal tempReal = RSSL_INIT_REAL;
	RsslDataDictionary* dictionary = getDictionary();
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslMarketPriceItem itemInfo;
	RsslBuffer hostName = RSSL_INIT_BUFFER;
	char hostNameBuf[256];

	/* set-up message */
	postMsg.msgBase.msgClass = RSSL_MC_POST;
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;

	// Note: post message key not required for on-stream post
	postMsg.flags = RSSL_PSMF_POST_COMPLETE 
		| RSSL_PSMF_ACK // request ACK
		| RSSL_PSMF_HAS_POST_ID
		| RSSL_PSMF_HAS_SEQ_NUM;

	postMsg.postId = nextPostId++;
	postMsg.seqNum = nextSeqNum++;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
					rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
			return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();
	
	// encode message 
	if((ret = rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeIteratorBuffer() failed with return code: %d\n", ret);
		return ret;
	}
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&postMsg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	initMarketPriceItemFields(&itemInfo);
	itemInfo.TRDPRC_1 = getNextItemData();
	itemInfo.BID = getNextItemData();
	itemInfo.ASK = getNextItemData();

	/* encode field list */
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* TRDPRC_1 */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[TRDPRC_1_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = TRDPRC_1_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &itemInfo.TRDPRC_1, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* BID */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[BID_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = BID_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &itemInfo.BID, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	/* ASK */
	rsslClearFieldEntry(&fEntry);
	dictionaryEntry = dictionary->entriesArray[ASK_FID];
	if (dictionaryEntry)
	{
		fEntry.fieldId = ASK_FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		rsslClearReal(&tempReal);
		rsslDoubleToReal(&tempReal, &itemInfo.ASK, RSSL_RH_EXPONENT_2);
		if ((ret = rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)&tempReal)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode field list */
	if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	printf ("\n\nSENDING POST WITH DATA:\n");
	printf ("	streamId = %d\n", postMsg.msgBase.streamId);
	printf ("	postId   = %d\n", postMsg.postId);
	printf ("	seqNum   = %d\n", postMsg.seqNum);

	return RSSL_RET_SUCCESS;
}


/* This function encodes and sends an on-stream post message */
/* It will either send a post that contains a full message 
 * or a post that contains only data payload, based on the
 * postWithMsg parameter.  If true, post will contain a message.
 *
 * This function only sends one post message, however it is 
 * called periodically over a time increment by the 
 * handlePosts function 
 */
RsslRet sendOnstreamPostMsg(RsslChannel* chnl, RsslBool postWithMsg)
{
	RsslError error;
	RsslRet ret = 0;
	RsslBuffer* msgBuf = 0;
	RsslInt32 streamId = 0;
	RsslBuffer nameBuf = RSSL_INIT_BUFFER;

	/* get the streamId and name for the market price item to post on */
	streamId = getFirstMarketPriceItem(&nameBuf);

	if (streamId == 0)
	{
		/* no items available to post on */
		printf("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");
		return RSSL_RET_SUCCESS;
	}

	/* get a buffer for the item request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		if (postWithMsg == RSSL_TRUE)
		{
			if (encodePostWithMsg(chnl, msgBuf, streamId, &nameBuf) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error); 
				printf("\nencodePostWithMsg() failed\n");
				return RSSL_RET_FAILURE;
			}
		}
		else
		{
			if (encodePostWithData(chnl, msgBuf, streamId) != RSSL_RET_SUCCESS)
			{
				rsslReleaseBuffer(msgBuf, &error); 
				printf("\nencodePostWithData() failed\n");
				return RSSL_RET_FAILURE;
			}
		}

		/* send item request */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}

/* This function encodes and sends an off-stream post message.
 *
 * This function only sends one post message, however it is 
 * called periodically over a time increment by the 
 * handlePosts function 
 */
RsslRet sendOffstreamPostMsg(RsslChannel* chnl)
{
	RsslError error;
	RsslRet ret = 0;
	RsslBuffer* msgBuf = 0;
	RsslInt32 streamId = 0;
	RsslBuffer nameBuf = RSSL_INIT_BUFFER;
	RsslLoginResponseInfo* loginRespInfo = getLoginResponseInfo();

	/* get the login stream id to off-stream post on */
	streamId = loginRespInfo->StreamId;

	/* get a buffer for the item request */
	msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error);

	if (msgBuf != NULL)
	{
		/* set off-stream post item name */
		nameBuf.data = (char *)"OFFPOST";
		nameBuf.length = (RsslUInt32)strlen("OFFPOST");

		if (encodePostWithMsg(chnl, msgBuf, streamId, &nameBuf) != RSSL_RET_SUCCESS)
		{
			rsslReleaseBuffer(msgBuf, &error); 
			printf("\nencodePostWithMsg() failed\n");
			return RSSL_RET_FAILURE;
		}		

		/* send item request */
		if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;

		offstreamPostSent = RSSL_TRUE;
	}
	else
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}	

	return RSSL_RET_SUCCESS;
}


/* 
 * This function encodes and sends an off-stream post close status message.
 */
RsslRet closeOffstreamPost(RsslChannel* chnl)
{
	RsslError error;
	RsslRet ret = 0;
	RsslBuffer* msgBuf = 0;
	RsslInt32 streamId = 0;
	RsslBuffer nameBuf = RSSL_INIT_BUFFER;
	RsslLoginResponseInfo* loginRespInfo = getLoginResponseInfo();

	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslStatusMsg statusMsg = RSSL_INIT_STATUS_MSG;
	RsslUInt16 serviceId = (RsslUInt16)getServiceId();
	RsslEncodeIterator encodeIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslBuffer hostName = RSSL_INIT_BUFFER;
	char hostNameBuf[256];

	// first check if we have sent offstream posts
	if (!offstreamPostSent)
		return RSSL_RET_SUCCESS;

	/* get the login stream id to off-stream post on */
	streamId = loginRespInfo->StreamId;

	/* get a buffer for the status close */
	if ((msgBuf = rsslGetBuffer(chnl, MAX_MSG_SIZE, RSSL_FALSE, &error)) == NULL)
	{
		printf("rsslGetBuffer(): Failed <%s>\n", error.text);
		return RSSL_RET_FAILURE;
	}

	/* set-up post message */
	postMsg.msgBase.msgClass = RSSL_MC_POST;
	postMsg.msgBase.streamId = streamId;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	postMsg.msgBase.containerType = RSSL_DT_MSG;

	// Note: This example sends a status close when it shuts down. 
	// So don't ask for an ACK (that we will never get)
	postMsg.flags = RSSL_PSMF_POST_COMPLETE 
					| RSSL_PSMF_HAS_POST_ID
					| RSSL_PSMF_HAS_SEQ_NUM
					| RSSL_PSMF_HAS_POST_USER_RIGHTS
					| RSSL_PSMF_HAS_MSG_KEY;

	postMsg.postId = nextPostId++;
	postMsg.seqNum = nextSeqNum++;
	postMsg.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE;

	/* set off-stream post item name */
	postMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	postMsg.msgBase.msgKey.name.data = (char *)"OFFPOST";
	postMsg.msgBase.msgKey.name.length = (RsslUInt32)strlen("OFFPOST");
	postMsg.msgBase.msgKey.nameType = 1;
	postMsg.msgBase.msgKey.serviceId = serviceId;

	/* populate post user info */
	hostName.data = hostNameBuf;
	hostName.length = 256;
	gethostname(hostName.data, hostName.length);
	hostName.length = (RsslUInt32)strlen(hostName.data);
	if ((ret = rsslHostByName(&hostName, &postMsg.postUserInfo.postUserAddr)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("Populating postUserInfo failed. Error %s (%d) with rsslHostByName: %s\n",
					rsslRetCodeToString(ret), ret, rsslRetCodeInfo(ret));
		return ret;
	}
	postMsg.postUserInfo.postUserId = getpid();

	/* set-up status message that will be nested in the post message */
	statusMsg.flags = RSSL_STMF_HAS_STATE;
	statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
	statusMsg.msgBase.streamId = streamId;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	statusMsg.state.streamState = RSSL_STREAM_CLOSED;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;

	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, chnl->majorVersion, chnl->minorVersion);

	// encode post message 
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&postMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() for postMsg failed with return code: %d\n", ret);
		return ret;
	}
	
	// encode nested status message 
	if ((ret = rsslEncodeMsgInit(&encodeIter, (RsslMsg*)&statusMsg, 0)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgInit() for statusMsg failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode status message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() for statusMsg failed with return code: %d\n", ret);
		return ret;
	}

	/* complete encode post message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		rsslReleaseBuffer(msgBuf, &error);
		printf("rsslEncodeMsgComplete() for postMsg failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);


	/* send post */
	if (sendMessage(chnl, msgBuf) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;
	
	rsslFlush(chnl, &error);	// send it immediately because we are exiting

	return RSSL_RET_SUCCESS;
}
