/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 LSEG. All rights reserved.     
*/

#ifndef POST_HANDLER_H
#define POST_HANDLER_H

#include "watchlistConsumerConfig.h"
#include "rtr/rsslReactor.h"

#define POST_MESSAGE_FREQUENCY 5 /* Post message send frequency in seconds */

/* market price item data */
typedef struct {
	RsslUInt64		RDNDISPLAY;
	RsslEnum		RDN_EXCHID;
	RsslDate		DIVPAYDATE;
	double			TRDPRC_1;
	double			BID;
	double			ASK;
	double			ACVOL_1;
	double			NETCHNG_1;
	RsslDateTime	ASK_TIME;
	double			PERATIO;
	RsslDateTime	SALTIME;
} MarketPriceItem;

/* Contains posting data. */
typedef struct 
{
	MarketPriceItem	*itemData;			/* Associated data. */
	RsslBool		IsRefreshComplete;	/* Whether a refresh has been sent. */
} PostItemInfo;

/* Maintains information about a posting service. */
typedef struct
{
	/* Service ID to use for posting. */
	RsslUInt16							serviceId;

	/* Whether we have identified the service we will use for posting. */
	RsslBool							isServiceFound;

	/* Whether the service we will use for posting is up. */
	RsslBool							isServiceUp;
} PostServiceInfo;

void clearPostServiceInfo(PostServiceInfo *serviceInfo);

void processPostServiceUpdate(PostServiceInfo *serviceInfo, RsslBuffer *pMatchServiceName, RsslRDMService* pService);

void postHandlerInit();

RsslRet sendOnStreamPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslBool postWithMsg);

RsslRet sendOffStreamPostMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslBool postWithMsg);

static RsslRet encodePostWithMsg(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum, 
		RsslBuffer *itemName, PostItemInfo* itemInfo);

static RsslRet encodePostWithData(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, 
		RsslInt32 streamId, RsslUInt32 postId, RsslUInt32 seqNum, RsslBuffer *itemName, 
		PostItemInfo* itemInfo);

static RsslRet encodeMarketPriceResp(RsslReactorChannel* pReactorChannel, RsslBuffer* msgBuf,
		RsslInt32 streamId, RsslBuffer *itemName, PostItemInfo* itemInfo);

static void initMPItemFields(MarketPriceItem* mpItem);


#endif
