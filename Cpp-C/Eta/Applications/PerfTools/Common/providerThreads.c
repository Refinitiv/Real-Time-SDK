/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2020 Refinitiv. All rights reserved.
*/

#include "providerThreads.h"
#include "directoryProvider.h"
#include "testUtils.h"
#include "provPerfConfig.h"
#include "niProvPerfConfig.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

/* Contains the global ProviderThread configuration. */
ProviderThreadConfig providerThreadConfig;

/* Contains the global NIProviderThread configuration */
NIProvPerfConfig niProvPerfConfig;

/* Contains the global ProvPerfConfig configuration */
ProvPerfConfig provPerfConfig;

ProvStats totalStats;

static RsslInt32 defaultThreadCount = 1;
static RsslInt32 defaultThreadBindList[] = { -1 };

static void providerThreadCleanup(ProviderThread *pProvThread);

void clearProviderThreadConfig()
{
	memset(&providerThreadConfig, 0, sizeof(ProviderThreadConfig));
	providerThreadConfig.ticksPerSec = 1000;
	providerThreadConfig.totalBuffersPerPack = 1;
	providerThreadConfig.packingBufferLength = 6000;
	providerThreadConfig.updatesPerSec = 100000;
	providerThreadConfig.genMsgsPerSec = 0;
	providerThreadConfig.refreshBurstSize = 10;
	providerThreadConfig.latencyUpdatesPerSec = 10;
	providerThreadConfig.latencyGenMsgsPerSec = 0;
	providerThreadConfig.writeFlags = 0;
	snprintf(providerThreadConfig.itemFilename, sizeof(providerThreadConfig.itemFilename), "350k.xml");
	snprintf(providerThreadConfig.msgFilename, sizeof(providerThreadConfig.msgFilename), "MsgData.xml");
	providerThreadConfig.threadBindList = defaultThreadBindList;
	providerThreadConfig.threadCount = defaultThreadCount;
	snprintf(providerThreadConfig.statsFilename, sizeof(providerThreadConfig.statsFilename), "ProvStats");
	snprintf(providerThreadConfig.latencyLogFilename, sizeof(providerThreadConfig.latencyLogFilename), "");
	providerThreadConfig.logLatencyToFile = RSSL_FALSE;

	providerThreadConfig.preEncItems = RSSL_FALSE;
	providerThreadConfig.takeMCastStats = RSSL_FALSE;
	providerThreadConfig.nanoTime = RSSL_FALSE;
	providerThreadConfig.measureEncode = RSSL_FALSE;
}

void providerThreadConfigInit()
{
	/* Configuration checks */
	if (providerThreadConfig.latencyUpdatesPerSec > providerThreadConfig.updatesPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total update rate. \n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyGenMsgsPerSec > providerThreadConfig.genMsgsPerSec)
	{
		printf("Config Error: Latency genMsg rate cannot be greater than total genMsg rate. \n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyUpdatesPerSec > providerThreadConfig.ticksPerSec)
	{
		printf("Config Error: Latency update rate cannot be greater than total ticks per second. \n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyGenMsgsPerSec > providerThreadConfig.ticksPerSec)
	{
		printf("Config Error: Latency genMsg rate cannot be greater than total ticks per second. \n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE
		&& providerThreadConfig.preEncItems == RSSL_TRUE)
	{
		printf("Config Error: -preEnc has no effect when always sending latency update, since it must be encoded.\n\n");
		exit(-1);
		}
	if (providerThreadConfig.latencyUpdatesPerSec == 0 && providerThreadConfig.measureEncode)
	{
		printf("Config Error: Measuring message encoding time when latency update rate is zero. Message encoding time is only recorded for latency updates.\n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyGenMsgsPerSec == ALWAYS_SEND_LATENCY_GENMSG
			&& providerThreadConfig.preEncItems == RSSL_TRUE)
	{
		printf("Config Error: -preEnc has no effect when always sending latency genMsg, since it must be encoded.\n\n");
		exit(-1);
	}

	if (providerThreadConfig.latencyGenMsgsPerSec == 0 && providerThreadConfig.measureEncode)
	{
		printf("Config Error: Measuring message encoding time when latency genMsg rate is zero. Message encoding time is only recorded for latency genMsgs.\n\n");
		exit(-1);
	}

	if (providerThreadConfig.updatesPerSec != 0 && providerThreadConfig.updatesPerSec < providerThreadConfig.ticksPerSec)
	{
		printf("Config Error: Update rate cannot be less than total ticks per second(unless it is zero).\n\n");
		exit(-1);
	}

	if (providerThreadConfig.genMsgsPerSec != 0 && providerThreadConfig.genMsgsPerSec < providerThreadConfig.ticksPerSec)
	{
		printf("Config Error: GenMsg rate cannot be less than total ticks per second(unless it is zero).\n\n");
		exit(-1);
	}

	if (providerThreadConfig.totalBuffersPerPack < 1)
	{
		printf("Config Error: Cannot specify less than 1 buffer per pack.\n\n");
		exit(-1);
	}

	if (providerThreadConfig.totalBuffersPerPack > 1 && providerThreadConfig.packingBufferLength == 0)
	{
		printf("Config Error: -maxPackCount set but -packBufSize is zero.\n\n");
		exit(-1);
	}

	/* Determine update rates on per-tick basis */
	providerThreadConfig._updatesPerTick = providerThreadConfig.updatesPerSec / providerThreadConfig.ticksPerSec;
	providerThreadConfig._updatesPerTickRemainder = providerThreadConfig.updatesPerSec % providerThreadConfig.ticksPerSec;

	if (providerThreadConfig.updatesPerSec != 0 && providerThreadConfig.latencyUpdatesPerSec > 0)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = providerThreadConfig.updatesPerSec;
		randomArrayOpts.latencyMsgsPerSec = providerThreadConfig.latencyUpdatesPerSec;
		randomArrayOpts.ticksPerSec = providerThreadConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(&providerThreadConfig._latencyUpdateRandomArray, &randomArrayOpts);
	}


	/* Determine GenMsg rates on per-tick basis */
	providerThreadConfig._genMsgsPerTick = providerThreadConfig.genMsgsPerSec / providerThreadConfig.ticksPerSec;
	providerThreadConfig._genMsgsPerTickRemainder = providerThreadConfig.genMsgsPerSec % providerThreadConfig.ticksPerSec;

	if (providerThreadConfig.genMsgsPerSec != 0 && providerThreadConfig.latencyGenMsgsPerSec > 0)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		clearLatencyRandomArrayOptions(&randomArrayOpts);
		randomArrayOpts.totalMsgsPerSec = providerThreadConfig.genMsgsPerSec;
		randomArrayOpts.latencyMsgsPerSec = providerThreadConfig.latencyGenMsgsPerSec;
		randomArrayOpts.ticksPerSec = providerThreadConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		createLatencyRandomArray(&providerThreadConfig._latencyGenMsgRandomArray, &randomArrayOpts);
	}


	if (xmlMsgDataInit(providerThreadConfig.msgFilename) != RSSL_RET_SUCCESS)
		exit(-1);

	directoryServiceInit();
}

void providerThreadInit(ProviderThread *pProvThread,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg,
		MsgConverterCallback *convertMsg,
		RsslInt32 providerIndex,
		ProviderType providerType)
{
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
	char tmpFilename[sizeof(providerThreadConfig.statsFilename) + 8];

	timeRecordQueueInit(&pProvThread->genMsgLatencyRecords);

	/* Load dictionary from file if possible. */
	pProvThread->pDictionary = NULL;
	pProvThread->pDictionary = (RsslDataDictionary*)malloc(sizeof(RsslDataDictionary));
	rsslClearDataDictionary(pProvThread->pDictionary);
	if (rsslLoadFieldDictionary("RDMFieldDictionary", pProvThread->pDictionary, &errorText) < 0)
	{
		printf("Unable to load field dictionary: %s.\n"
				"Will request dictionaries from provider.\n\n" , errorText.data);
	}
	else if (rsslLoadEnumTypeDictionary("enumtype.def", pProvThread->pDictionary, &errorText) < 0)
	{
		printf("\nUnable to load enum type dictionary: %s\n", errorText.data);
		printf("Unable to load enum type dictionary: %s.\n\n"
				"Will request dictionaries from provider.\n" , errorText.data);
	}
	else
	{
		/* Dictionary successfully loaded. */
		pProvThread->dictionaryStateFlags =
			(DictionaryStateFlags)(DICTIONARY_STATE_HAVE_FIELD_DICT | DICTIONARY_STATE_HAVE_ENUM_DICT);
	}

	provStatsInit(&totalStats);

	initCountStat(&pProvThread->refreshMsgCount);
	initCountStat(&pProvThread->updateMsgCount);
	initCountStat(&pProvThread->itemRequestCount);
	initCountStat(&pProvThread->closeMsgCount);
	initCountStat(&pProvThread->postMsgCount);
	initCountStat(&pProvThread->outOfBuffersCount);
	initCountStat(&pProvThread->msgSentCount);
	initCountStat(&pProvThread->bufferSentCount);
	initCountStat(&pProvThread->stats.genMsgSentCount);
	initCountStat(&pProvThread->stats.genMsgRecvCount);
	initCountStat(&pProvThread->stats.latencyGenMsgSentCount);
	clearValueStatistics(&pProvThread->stats.genMsgLatencyStats);
	clearValueStatistics(&pProvThread->stats.intervalGenMsgLatencyStats);

	pProvThread->stats.inactiveTime = 0;
	pProvThread->stats.firstGenMsgSentTime = 0;
	pProvThread->stats.firstGenMsgRecvTime = 0;

	pProvThread->currentTicks = 0;
	pProvThread->providerIndex = providerIndex;

	if (providerThreadConfig.measureEncode)
		timeRecordQueueInit(&pProvThread->messageEncodeTimeRecords);
	memset(&pProvThread->prevMCastStats, 0, sizeof(pProvThread->prevMCastStats));

	snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
			providerThreadConfig.statsFilename, providerIndex + 1);

	if (!(pProvThread->statsFile = fopen(tmpFilename, "w")))
	{
		printf("Error: Failed to open file '%s'.\n", tmpFilename);
		exit(-1);
	}

	pProvThread->latencyLogFile = NULL;
	if (providerThreadConfig.logLatencyToFile)
	{
		snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
				providerThreadConfig.latencyLogFilename, providerIndex + 1);

		/* Open latency log file. */
		pProvThread->latencyLogFile = fopen(tmpFilename, "w");
		if (!pProvThread->latencyLogFile)
		{
			printf("Failed to open latency log file: %s\n", tmpFilename);
			exit(-1);
		}

		fprintf(pProvThread->latencyLogFile, "Message type, Send time, Receive time, Latency (usec)\n");
	}

	switch(providerType)
	{
		case PROVIDER_INTERACTIVE:
			fprintf(pProvThread->statsFile, "UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsgs received, GenMSg Latencies sent, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");
			break;
			case PROVIDER_NONINTERACTIVE:
			fprintf(pProvThread->statsFile, "UTC, Images sent, Updates sent, CPU usage (%%), Memory (MB)\n");
			break;
	}


	RSSL_MUTEX_INIT(&pProvThread->newClientSessionsLock);
	rsslInitQueue(&pProvThread->newClientSessionsList);
	pProvThread->clientSessionsCount = 0;
	initChannelHandler(&pProvThread->channelHandler, processActiveChannel, processInactiveChannel, processMsg, convertMsg, (void*)pProvThread);

	pProvThread->cpuId = -1;

	latencyRandomArrayIterInit(&pProvThread->randArrayIter);

	pProvThread->pReactor = NULL;

}

static void providerThreadCleanup(ProviderThread *pProvThread)
{
	timeRecordQueueCleanup(&pProvThread->genMsgLatencyRecords);
	if (pProvThread->pDictionary)
	{
		rsslDeleteDataDictionary(pProvThread->pDictionary);
		free(pProvThread->pDictionary);
	}

	channelHandlerCleanup(&pProvThread->channelHandler);

	if(pProvThread->statsFile)
		fclose(pProvThread->statsFile);
	if(pProvThread->latencyLogFile)
		fclose(pProvThread->latencyLogFile);
}

void providerThreadConfigCleanup()
{
	cleanupLatencyRandomArray(&providerThreadConfig._latencyUpdateRandomArray);
	cleanupLatencyRandomArray(&providerThreadConfig._latencyGenMsgRandomArray);
	xmlMsgDataCleanup();
}

ProviderSession *providerSessionCreate(ProviderThread *pProvThread, RsslChannel *pChannel)
{
	ProviderSession *pSession;
	RsslUInt32 maxFragmentSizePad = 0;

	pSession = (ProviderSession*)malloc(sizeof(ProviderSession));

	initRotatingQueue(&pSession->refreshItemList);
	initRotatingQueue(&pSession->updateItemList);

	pSession->preEncMarketPriceMsgs = 0;
	pSession->preEncMarketByOrderMsgs = 0;

	pSession->openItemsCount = 0;
	pSession->pWritingBuffer = 0;
	pSession->packedBufferCount = 0;
	pSession->timeActivated = 0;
	pSession->lastWriteRet = 0;


	hashTableInit(&pSession->itemAttributesTable, 
			10007,
			hashSumItemAttributes,
			hashCompareItemAttributes
			);

	hashTableInit(&pSession->itemStreamIdTable, 
			10007,
			intHashFunction,
			hashCompareStreamId
			);

	if (providerThreadConfig.preEncItems)
	{
		RsslInt32 i;
		RsslInt32 updateCount;
		/* Pre-encode the update into a buffer, using a default item setup.
		 * All updates will copy from this buffer and change the StreamID. */

		MarketPriceItem mpItem;
		MarketByOrderItem mboItem;
		ItemInfo itemInfo;
		RsslRet ret;

		clearMarketPriceItem(&mpItem);
		clearMarketByOrderItem(&mboItem);

		clearItemInfo(&itemInfo);
		itemInfo.attributes.domainType = RSSL_DMT_MARKET_PRICE;
		itemInfo.itemData = (void*)&mpItem;

		updateCount = getMarketPriceUpdateMsgCount();
		pSession->preEncMarketPriceMsgs = (RsslBuffer*)malloc(updateCount * sizeof(RsslBuffer)); assert(pSession->preEncMarketPriceMsgs);

		for(i = 0; i < updateCount; ++i)
		{
			RsslBuffer *pEncMsgBuf = &pSession->preEncMarketPriceMsgs[i];
			pEncMsgBuf->length = estimateItemUpdateBufferLength(&itemInfo);
			pEncMsgBuf->data = malloc(pEncMsgBuf->length); assert(pEncMsgBuf->data);
			if ((ret = encodeItemUpdate(pChannel, &itemInfo, pEncMsgBuf, NULL, 0)) < RSSL_RET_SUCCESS)
			{
				printf("Encoding pre-encoded buf: encodeItemUpdate() failed: %d\n", ret);
				free(pSession->preEncMarketPriceMsgs[i].data);
				free(pSession->preEncMarketPriceMsgs);
				free(pSession);
				return NULL;
			}
		}
		assert(mpItem.iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		itemInfo.attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		itemInfo.itemData = (void*)&mboItem;

		updateCount = getMarketByOrderUpdateMsgCount();
		pSession->preEncMarketByOrderMsgs = (RsslBuffer*)malloc(updateCount * sizeof(RsslBuffer)); assert(pSession->preEncMarketByOrderMsgs);

		for(i = 0; i < updateCount; ++i)
		{
			RsslBuffer *pEncMsgBuf = &pSession->preEncMarketByOrderMsgs[i];
			pEncMsgBuf->length = estimateItemUpdateBufferLength(&itemInfo);
			pEncMsgBuf->data = malloc(pEncMsgBuf->length); assert(pEncMsgBuf->data);
			if ((ret = encodeItemUpdate(pChannel, &itemInfo, pEncMsgBuf, NULL, 0)) < RSSL_RET_SUCCESS)
			{
				printf("Encoding pre-encoded buf: encodeItemUpdate() failed: %d\n", ret);
				free(pSession->preEncMarketByOrderMsgs[i].data);
				free(pSession->preEncMarketByOrderMsgs);
				free(pSession);
				return NULL;
			}
		}
		assert(mboItem.iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */
	}

	if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
	{
		pSession->pChannelInfo = channelHandlerAddChannel(&pProvThread->channelHandler, pChannel, 
				(void*)pSession, RSSL_TRUE );
	}
	else // use UPA VA Reactor
	{
		pSession->pChannelInfo = (ChannelInfo*)malloc(sizeof(ChannelInfo));
		clearChannelInfo(pSession->pChannelInfo);
		pSession->pChannelInfo->pUserSpec = pSession;
		pSession->pChannelInfo->parentQueue = &pProvThread->channelHandler.activeChannelList;
	}

	return pSession;
}

/* Hash table function for comparing Stream ID's. */
static RsslBool hashCompareStreamId(void *key, HashTableLink *pLink)
{
	ItemInfo *itemInfo = HASH_TABLE_LINK_TO_OBJECT(ItemInfo, itemStreamIdTableLink, pLink);
	return ( itemInfo->StreamId == *((RsslInt32*)key) );
}

/* Hash table function for distributing items by their key. */
static RsslUInt32 hashSumItemAttributes(void *key)
{
	RsslUInt32 i;
	RsslUInt32 hashSum = 0;
	RsslBuffer *pName = &((RsslItemAttributes*)key)->pMsgKey->name;

	for(i = 0; i < pName->length; ++i)
	{
		hashSum = (hashSum << 4) + (RsslUInt32)pName->data[i];
		hashSum ^= (hashSum >> 12);
	}

	return hashSum;
}

/* Hash table function for matching items by their key. */
static RsslBool hashCompareItemAttributes(void *key, HashTableLink *pLink)
{
	ItemInfo *itemInfo = HASH_TABLE_LINK_TO_OBJECT(ItemInfo, itemAttributesTableLink, pLink);
	RsslItemAttributes *attribs = (RsslItemAttributes*)key;

	return
		((
		 itemInfo->attributes.domainType == attribs->domainType &&
		 RSSL_RET_SUCCESS == rsslCompareMsgKeys(itemInfo->attributes.pMsgKey, attribs->pMsgKey)) ? RSSL_TRUE : RSSL_FALSE);
}



RsslRet printEstimatedMsgSizes(ProviderThread *pProvThread, ProviderSession *pSession)
{
	MarketPriceItem *mpItem;
	MarketByOrderItem *mboItem;
	RsslBuffer testBuffer;
	RsslRet ret;
	RsslInt32 i;
	RsslMsgKey msgKey;

	ItemInfo *pItemInfo;

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	rsslClearMsgKey(&msgKey);
	msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_SERVICE_ID;
	msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msgKey.name.data = (char*)"RDT0";
	msgKey.name.length = 4;
	msgKey.serviceId = 0;


	/* Market Price */
	if (xmlMsgDataHasMarketPrice)
	{
		mpItem = createMarketPriceItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_PRICE;
		pItemInfo->itemData = (void*)mpItem;

		printf("Approximate message sizes:\n");

		testBuffer.length = estimateItemRefreshBufferLength(pItemInfo);
		testBuffer.data = (char*)malloc(testBuffer.length);
		if ((ret = encodeItemRefresh(pSession->pChannelInfo->pChannel, pItemInfo, &testBuffer, NULL, 0)) < RSSL_RET_SUCCESS)
		{
			printf("printEstimatedMsgSizes: encodeItemRefresh() failed: %s\n", rsslRetCodeToString(ret));
			exit(-1);
		}
		printf(	"  MarketPrice RefreshMsg(without name):\n" 
				"         estimated length: %u bytes\n" 
				"    approx encoded length: %u bytes\n", 
				estimateItemRefreshBufferLength(pItemInfo),
				testBuffer.length);
		free(testBuffer.data);

		/* Update msgs */
		for (i = 0; i < getMarketPriceUpdateMsgCount(); ++i)
		{
			testBuffer.length = estimateItemUpdateBufferLength(pItemInfo);
			testBuffer.data = (char*)malloc(testBuffer.length);
			if ((ret = encodeItemUpdate(pSession->pChannelInfo->pChannel, pItemInfo, &testBuffer, NULL, 0)) < RSSL_RET_SUCCESS)
			{
				printf("printEstimatedMsgSizes: encodeItemUpdate() failed: %s\n", rsslRetCodeToString(ret));
				exit(-1);
			}
			printf(	"  MarketPrice UpdateMsg %d: \n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1,
					estimateItemUpdateBufferLength(pItemInfo),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mpItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketPriceItem(mpItem);
	}

	/* Market By Order */

	if (xmlMsgDataHasMarketByOrder)
	{
		mboItem = createMarketByOrderItem();

		clearItemInfo(pItemInfo);
		pItemInfo->attributes.pMsgKey = &msgKey;
		pItemInfo->attributes.domainType = RSSL_DMT_MARKET_BY_ORDER;
		pItemInfo->itemData = (void*)mboItem;

		testBuffer.length = estimateItemRefreshBufferLength(pItemInfo);
		testBuffer.data = (char*)malloc(testBuffer.length);
		if ((ret = encodeItemRefresh(pSession->pChannelInfo->pChannel, pItemInfo, &testBuffer, NULL, 0)) < RSSL_RET_SUCCESS)
		{
			printf("printEstimatedMsgSizes: encodeItemRefresh() failed: %s\n", rsslRetCodeToString(ret));
			exit(-1);
		}
		printf(	"  MarketByOrder RefreshMsg(without name): \n"
				"         estimated length: %u bytes\n" 
				"    approx encoded length: %u bytes\n", 
				estimateItemRefreshBufferLength(pItemInfo),
				testBuffer.length);
		free(testBuffer.data);

		/* Update msgs */
		for (i = 0; i < getMarketByOrderUpdateMsgCount(); ++i)
		{
			testBuffer.length = estimateItemUpdateBufferLength(pItemInfo);
			testBuffer.data = (char*)malloc(testBuffer.length);
			if ((ret = encodeItemUpdate(pSession->pChannelInfo->pChannel, pItemInfo, &testBuffer, NULL, 0)) < RSSL_RET_SUCCESS)
			{
				printf("printEstimatedMsgSizes: encodeItemUpdate() failed: %s\n", rsslRetCodeToString(ret));
				exit(-1);
			}
			printf(	"  MarketByOrder UpdateMsg %d:\n"
					"         estimated length: %u bytes\n" 
					"    approx encoded length: %u bytes\n", 
					i+1, 
					estimateItemUpdateBufferLength(pItemInfo),
					testBuffer.length);
			free(testBuffer.data);
		}
		assert(mboItem->iMsg == 0); /* encode function increments iMsg. If we've done everything right this should be 0 */

		freeMarketByOrderItem(mboItem);
	}

	free(pItemInfo);

	printf("\n");

	return RSSL_RET_SUCCESS;
}

void providerSessionDestroy(ProviderThread *pProvThread, ProviderSession *pSession)
{
	RsslQueueLink *pLink;

	if (pSession->preEncMarketPriceMsgs)
	{
		int i;
		for (i = 0; i < getMarketPriceUpdateMsgCount(); ++i)
			free(pSession->preEncMarketPriceMsgs[i].data);
		free(pSession->preEncMarketPriceMsgs);
		pSession->preEncMarketPriceMsgs = 0;
	}

	if (pSession->preEncMarketByOrderMsgs)
	{
		int i;
		for (i = 0; i < getMarketByOrderUpdateMsgCount(); ++i)
			free(pSession->preEncMarketByOrderMsgs[i].data);
		free(pSession->preEncMarketByOrderMsgs);
		pSession->preEncMarketByOrderMsgs = 0;
	}

	/* Free any items in the watchlist. */
	while(pLink = rotatingQueuePeekFrontAsList(&pSession->refreshItemList))
		freeItemInfo(pProvThread, pSession, RSSL_QUEUE_LINK_TO_OBJECT(ItemInfo, watchlistLink, pLink));

	while(pLink = rotatingQueuePeekFrontAsList(&pSession->updateItemList))
		freeItemInfo(pProvThread, pSession, RSSL_QUEUE_LINK_TO_OBJECT(ItemInfo, watchlistLink, pLink));

	if (niProvPerfConfig.useReactor || provPerfConfig.useReactor) // Reactor used
		free(pSession->pChannelInfo);

	hashTableCleanup(&pSession->itemAttributesTable);
	hashTableCleanup(&pSession->itemStreamIdTable);
	free(pSession);
}

ItemInfo *createItemInfo(ProviderThread *pProvThread, ProviderSession *pSession, RsslItemAttributes *pAttributes, RsslInt32 streamId)
{
	ItemInfo *pItemInfo;
	RsslMsgKey *pItemKey;
	RsslMsgKey *pMsgKey = pAttributes->pMsgKey;
	RsslRet ret;

	pItemInfo = (ItemInfo*)malloc(sizeof(ItemInfo));

	clearItemInfo(pItemInfo);

	/* Soft-copy attributes. */
	pItemInfo->attributes = *pAttributes;

	/* Allocate key */
	pItemKey = pItemInfo->attributes.pMsgKey = (RsslMsgKey*)malloc(sizeof(RsslMsgKey));

	/* Make space for key name and attrib if present, and copy the key. */
	if (pMsgKey->flags & RSSL_MKF_HAS_NAME)
	{
		pItemKey->name.data = (char *)malloc(pMsgKey->name.length);
		pItemKey->name.length = pMsgKey->name.length;
	}

	if (pMsgKey->flags & RSSL_MKF_HAS_ATTRIB)
	{
		pItemKey->encAttrib.data = (char *)malloc(pMsgKey->encAttrib.length);
		pItemKey->encAttrib.length = pMsgKey->encAttrib.length;
	}

	if ((ret = rsslCopyMsgKey(pItemKey, pMsgKey)) < RSSL_RET_SUCCESS)
	{
		printf("rsslCopyMsgKey() failed: %d\n", ret);
		if (pItemKey->flags & RSSL_MKF_HAS_NAME)
			free(pItemKey->name.data);
		if (pItemKey->flags & RSSL_MKF_HAS_ATTRIB)
			free(pItemKey->encAttrib.data);
		free(pItemKey);
		return NULL;
	}

	/* Copy stream ID. */
	pItemInfo->StreamId = streamId;
	
	/* Get item info data from appropriate pool */
	switch(pAttributes->domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			if (!xmlMsgDataHasMarketPrice)
			{
				printf("createItemInfo: No MarketPrice data present in message data file.\n");
				return NULL;
			}
			pItemInfo->itemData = (void*)createMarketPriceItem();
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			if (!xmlMsgDataHasMarketByOrder)
			{
				printf("createItemInfo: No MarketByOrder data present in message data file.\n");
				return NULL;
			}
			pItemInfo->itemData = (void*)createMarketByOrderItem();
			break;
		default:
			printf("createItemInfo: Unsupported domain %u.\n", pAttributes->domainType);
			if (pItemKey->flags & RSSL_MKF_HAS_NAME)
				free(pItemKey->name.data);
			if (pItemKey->flags & RSSL_MKF_HAS_ATTRIB)
				free(pItemKey->encAttrib.data);
			free(pItemKey);
			free(pItemInfo);
			return NULL;
	}


	/* Add item to watchlist */
	rotatingQueueInsert(&pSession->refreshItemList, &pItemInfo->watchlistLink);
	pItemInfo->myQueue = &pSession->refreshItemList;

	++pSession->openItemsCount;
	return pItemInfo;

}

static void freeItemInfo(ProviderThread *pProvThread, ProviderSession *pSession, ItemInfo* itemInfo)
{
	if (itemInfo)
	{
			switch(itemInfo->attributes.domainType)
			{
			case RSSL_DMT_MARKET_PRICE:
				freeMarketPriceItem((MarketPriceItem*)itemInfo->itemData);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				freeMarketByOrderItem((MarketByOrderItem*)itemInfo->itemData);
				break;
			}

			hashTableRemoveLink(&pSession->itemAttributesTable, &itemInfo->attributes);
			hashTableRemoveLink(&pSession->itemStreamIdTable, &itemInfo->StreamId);

			if(itemInfo->attributes.pMsgKey->flags & RSSL_MKF_HAS_NAME)
				free(itemInfo->attributes.pMsgKey->name.data);
			if(itemInfo->attributes.pMsgKey->flags & RSSL_MKF_HAS_ATTRIB)
				free(itemInfo->attributes.pMsgKey->encAttrib.data);
			free(itemInfo->attributes.pMsgKey);

			rotatingQueueRemove(itemInfo->myQueue, &itemInfo->watchlistLink);
			free(itemInfo);


			--pSession->openItemsCount;
	}
}

void closeItemStream(ProviderThread *pProvThread, ProviderSession *pSession, RsslInt32 streamId)
{
	HashTableLink *pLink = hashTableFind(&pSession->itemStreamIdTable, &streamId);

	/* remove original item request information */
	if (pLink)
	{
		ItemInfo *itemInfo = HASH_TABLE_LINK_TO_OBJECT(ItemInfo, itemStreamIdTableLink, pLink);
		freeItemInfo(pProvThread, pSession, itemInfo);
	}
	else
	{
		/* If we are sending an update for an item to the platform while it is sending
		 * us a close for that same item, it may respond to our update with another close.  
		 * If so, this is okay, so don't close the channel because of it. */
		printf("Received unexpected close on stream %d (this may just be an extra close from the platform).\n", streamId);
	}
}

RsslRet sendRefreshBurst(ProviderThread *pProvThread, ProviderSession *pSession)
{
	RsslInt32 refreshLeft;
	RsslRet ret = RSSL_RET_SUCCESS;
	ItemInfo *item;

	/* Determine refreshes to send out. */
	refreshLeft = rotatingQueueGetCount(&pSession->refreshItemList);
	if (refreshLeft > providerThreadConfig.refreshBurstSize)
		refreshLeft = providerThreadConfig.refreshBurstSize;

	for(; refreshLeft > 0; --refreshLeft)
	{
		RsslQueueLink *pLink;

		pLink = rotatingQueuePeekFrontAsList(&pSession->refreshItemList);

		item = RSSL_QUEUE_LINK_TO_OBJECT(ItemInfo, watchlistLink, pLink);

		/* get a buffer for the response */
		if (rtrUnlikely((ret = getItemMsgBuffer(pProvThread, pSession, estimateItemRefreshBufferLength(item))) 
					< RSSL_RET_SUCCESS))
			return ret;

		/* Encode the message with data appopriate for the domain */

		if (ret = encodeItemRefresh(pSession->pChannelInfo->pChannel, item, pSession->pWritingBuffer, NULL, 0))
			return ret;

		if ((ret =  sendItemMsgBuffer(pProvThread, pSession, refreshLeft > 1)) < RSSL_RET_SUCCESS)
			return ret;

		countStatIncr(&pProvThread->refreshMsgCount);

		/* If it's not a streaming request, don't add it to the update list. */
		if (!(item->itemFlags & ITEM_IS_STREAMING_REQ))
		{
			freeItemInfo(pProvThread, pSession, item);
			continue;
		}

		/* Else move to the update list. */
		rotatingQueueRemove(&pSession->refreshItemList, &item->watchlistLink);
		rotatingQueueInsert(&pSession->updateItemList, &item->watchlistLink);
		item->myQueue = &pSession->updateItemList;
	}

	/* If successful, return the last code from rsslWrite() to indicate any need for flushing. */
	return ret >= RSSL_RET_SUCCESS ? pSession->lastWriteRet : ret;
}

RsslRet sendUpdateBurst(ProviderThread *pProvThread, ProviderSession *pSession)
{
	RsslInt32 updatesLeft;
	RsslInt32 latencyUpdateNumber;
	RsslRet ret = RSSL_RET_SUCCESS;
	ItemInfo *nextItem;

	TimeValue measureEncodeStartTime, measureEncodeEndTime;

	/* Determine updates to send out. Spread the remainder out over the first ticks */
	updatesLeft = providerThreadConfig._updatesPerTick;
	if (providerThreadConfig._updatesPerTickRemainder > pProvThread->currentTicks)
		++updatesLeft;

	latencyUpdateNumber = (providerThreadConfig.latencyUpdatesPerSec > 0) ?
		latencyRandomArrayGetNext(&providerThreadConfig._latencyUpdateRandomArray,
				&pProvThread->randArrayIter) : -1; 

	if (!rotatingQueueGetCount(&pSession->updateItemList))
		return RSSL_RET_SUCCESS;

	for(; updatesLeft > 0; --updatesLeft)
	{
		RsslUInt latencyStartTime = 0;
		RsslQueueLink *pLink = rotatingQueueNext(&pSession->updateItemList);

		/* When appropriate, provide a latency timestamp for the updates. */
		if (providerThreadConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE 
				|| latencyUpdateNumber == (updatesLeft-1))
			latencyStartTime = providerThreadConfig.nanoTime ? getTimeNano() : getTimeMicro();
		else
			latencyStartTime = 0;

		nextItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemInfo, watchlistLink, pLink);

		/* get a buffer for the response */
		if (rtrUnlikely((ret = getItemMsgBuffer(pProvThread, pSession, estimateItemUpdateBufferLength(nextItem))) < RSSL_RET_SUCCESS))
		{
			if (ret == RSSL_RET_BUFFER_NO_BUFFERS)
				countStatAdd(&pProvThread->outOfBuffersCount, updatesLeft);
			return ret;
		}

		if (providerThreadConfig.measureEncode)
			measureEncodeStartTime = getTimeNano();

		if (!providerThreadConfig.preEncItems || latencyStartTime /* Latency item should always be fully encoded so we can send proper time information */)
		{
			if (pSession->pWritingBuffer && 
				(ret = encodeItemUpdate(pSession->pChannelInfo->pChannel, nextItem, pSession->pWritingBuffer, NULL, latencyStartTime) < RSSL_RET_SUCCESS))
				return ret;
		}
		else
		{
			/* Take the buffer we pre-encoded at startup and copy the data.
			 * Change the stream ID to send it for a different item. */

			RsslEncodeIterator eIter;
			RsslBuffer *pEncMsgBuf;

			switch(nextItem->attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pEncMsgBuf = &pSession->preEncMarketPriceMsgs[((MarketPriceItem*)nextItem->itemData)->iMsg];
					getNextMarketPriceUpdate((MarketPriceItem*)nextItem->itemData);
					break;

				case RSSL_DMT_MARKET_BY_ORDER:
					pEncMsgBuf = &pSession->preEncMarketByOrderMsgs[((MarketByOrderItem*)nextItem->itemData)->iMsg];
					getNextMarketByOrderUpdate((MarketByOrderItem*)nextItem->itemData);
					break;
			}

			if (rtrUnlikely(pSession->pWritingBuffer->length < pEncMsgBuf->length))
			{
				printf("Can't copy pre-encoded msg -- buffer is too small\n");
				return RSSL_RET_FAILURE;
			}

			memcpy(pSession->pWritingBuffer->data, pEncMsgBuf->data, pEncMsgBuf->length);
			pSession->pWritingBuffer->length = pEncMsgBuf->length;

			rsslClearEncodeIterator(&eIter);
			rsslSetEncodeIteratorRWFVersion(&eIter, pSession->pChannelInfo->pChannel->majorVersion,
											pSession->pChannelInfo->pChannel->minorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, pSession->pWritingBuffer);
			ret = rsslReplaceStreamId(&eIter, nextItem->StreamId);

			if (rtrUnlikely(ret < RSSL_RET_SUCCESS))
			{
				printf("rsslReplaceStreamId failed: %d\n", ret);
				return ret;
			}
		}

		if (providerThreadConfig.measureEncode)
		{
			measureEncodeEndTime = getTimeNano();
			timeRecordSubmit(&pProvThread->messageEncodeTimeRecords, measureEncodeStartTime, measureEncodeEndTime, 1000);
		}

		if ((ret =  sendItemMsgBuffer(pProvThread, pSession, updatesLeft > 1)) < RSSL_RET_SUCCESS)
			return ret;

		countStatIncr(&pProvThread->updateMsgCount);
	}

	if (rtrUnlikely(++pProvThread->currentTicks == providerThreadConfig.ticksPerSec))
		pProvThread->currentTicks = 0;

	/* If successful, return the last code from rsslWrite() to indicate any need for flushing. */
	return ret >= RSSL_RET_SUCCESS ? pSession->lastWriteRet : ret;
}

RsslRet sendGenMsgBurst(ProviderThread *pProvThread, ProviderSession *pSession)
{
	RsslInt32 genMsgsLeft;
	RsslInt32 latencyGenMsgNumber;
	RsslRet ret = RSSL_RET_SUCCESS;
	ItemInfo *nextItem;

	TimeValue measureEncodeStartTime, measureEncodeEndTime;

	/* Determine generic messages to send out. Spread the remainder out over the first ticks */
	genMsgsLeft = providerThreadConfig._genMsgsPerTick;
	if (providerThreadConfig._genMsgsPerTickRemainder > pProvThread->currentTicks)
		++genMsgsLeft;

	latencyGenMsgNumber = (providerThreadConfig.latencyGenMsgsPerSec > 0) ?
		latencyRandomArrayGetNext(&providerThreadConfig._latencyGenMsgRandomArray,
				&pProvThread->randArrayIter) : -1; 

	if (!rotatingQueueGetCount(&pSession->updateItemList))
	return RSSL_RET_SUCCESS;

	for(; genMsgsLeft > 0; --genMsgsLeft)
	{
		RsslUInt latencyStartTime;
		RsslQueueLink *pLink = rotatingQueueNext(&pSession->updateItemList);

		/* When appropriate, provide a latency timestamp for the genMsgs. */
		if (providerThreadConfig.latencyGenMsgsPerSec == ALWAYS_SEND_LATENCY_GENMSG 
				|| latencyGenMsgNumber == (genMsgsLeft-1))
		{
			countStatIncr(&pProvThread->stats.latencyGenMsgSentCount);
			latencyStartTime = providerThreadConfig.nanoTime ? getTimeNano() : getTimeMicro();
		}
		else
			latencyStartTime = 0;

		nextItem = RSSL_QUEUE_LINK_TO_OBJECT(ItemInfo, watchlistLink, pLink);

		/* get a buffer for the response */
		if (rtrUnlikely((ret = getItemMsgBuffer(pProvThread, pSession, estimateItemGenMsgBufferLength(nextItem))) < RSSL_RET_SUCCESS))
		{
			if (ret == RSSL_RET_BUFFER_NO_BUFFERS)
				countStatAdd(&pProvThread->outOfBuffersCount, genMsgsLeft);
			return ret;
		}

		if (providerThreadConfig.measureEncode)
			measureEncodeStartTime = getTimeNano();

		if (!providerThreadConfig.preEncItems || latencyStartTime /* Latency item should always be fully encoded so we can send proper time information */)
		{
			if (ret = encodeItemGenMsg(pSession->pChannelInfo->pChannel, nextItem, pSession->pWritingBuffer,
										latencyStartTime) < RSSL_RET_SUCCESS)
			return ret;
		}
		else
		{
			/* Take the buffer we pre-encoded at startup and copy the data.
			 * Change the stream ID to send it for a different item. */

			RsslEncodeIterator eIter;
			RsslBuffer *pEncMsgBuf;

			switch(nextItem->attributes.domainType)
			{
				case RSSL_DMT_MARKET_PRICE:
					pEncMsgBuf = &pSession->preEncMarketPriceMsgs[((MarketPriceItem*)nextItem->itemData)->iMsg];
					getNextMarketPriceUpdate((MarketPriceItem*)nextItem->itemData);
					break;

				case RSSL_DMT_MARKET_BY_ORDER:
					pEncMsgBuf = &pSession->preEncMarketByOrderMsgs[((MarketByOrderItem*)nextItem->itemData)->iMsg];
					getNextMarketByOrderUpdate((MarketByOrderItem*)nextItem->itemData);
					break;
			}

			if (rtrUnlikely(pSession->pWritingBuffer->length < pEncMsgBuf->length))
			{
				printf("Can't copy pre-encoded msg -- buffer is too small\n");
				return RSSL_RET_FAILURE;
			}

			memcpy(pSession->pWritingBuffer->data, pEncMsgBuf->data, pEncMsgBuf->length);
			pSession->pWritingBuffer->length = pEncMsgBuf->length;

			rsslClearEncodeIterator(&eIter);
            rsslSetEncodeIteratorRWFVersion(&eIter, pSession->pChannelInfo->pChannel->majorVersion,
											pSession->pChannelInfo->pChannel->minorVersion);
			rsslSetEncodeIteratorBuffer(&eIter, pSession->pWritingBuffer);
			ret = rsslReplaceStreamId(&eIter, nextItem->StreamId);

			if (rtrUnlikely(ret < RSSL_RET_SUCCESS))
			{
				printf("rsslReplaceStreamId failed: %d\n", ret);
				return ret;
			}
		}

		if (providerThreadConfig.measureEncode)
		{
			measureEncodeEndTime = getTimeNano();
			timeRecordSubmit(&pProvThread->messageEncodeTimeRecords, measureEncodeStartTime,
								measureEncodeEndTime, 1000);
		}

		if ((ret =  sendItemMsgBuffer(pProvThread, pSession, genMsgsLeft > 1)) < RSSL_RET_SUCCESS)
			return ret;

		countStatIncr(&pProvThread->stats.genMsgSentCount);
		if (!pProvThread->stats.firstGenMsgSentTime)
			pProvThread->stats.firstGenMsgSentTime = getTimeNano();
	}

	if (rtrUnlikely(++pProvThread->currentTicks == providerThreadConfig.ticksPerSec))
		pProvThread->currentTicks = 0;

	/* If successful, return the last code from rsslWrite() to indicate any need for flushing. */
	return ret >= RSSL_RET_SUCCESS ? pSession->lastWriteRet : ret;
}

static RsslRet writeCurrentBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslError *pError)
{
	RsslChannel *pChannel = pSession->pChannelInfo->pChannel;
	RsslBuffer *pMsgBuffer = 0;
	RsslError	  error;
	RsslUInt32 outBytes;
	RsslUInt32 uncompOutBytes;
	RsslRet ret;
	RsslBool releaseBuffer = RSSL_FALSE;

	assert(pSession->pWritingBuffer);

	pSession->packedBufferCount = 0;

	if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
	{
		pMsgBuffer = 0;

		/* Convert RWF msg to Json only if there is room (i.e. pWritingBuffer->length > 0 */
		if (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE && pSession->pWritingBuffer->length > 0)
		{
			RsslErrorInfo	eInfo;

			do {
					/* convert message to JSON */
				if ((pMsgBuffer = rjcMsgConvertToJson(&(pProvThread->rjcSess), pChannel,
														pSession->pWritingBuffer, &eInfo)) == NULL)
				{
					if (eInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					{
						if ((ret = rsslFlush(pChannel, pError)) < RSSL_RET_SUCCESS)
						{
							printf("rsslFlush() failed with return code %d - <%s>\n", 
									ret, pError->text);
							return ret;
						}
					}
					else
					{
						fprintf(stderr, 
								"writeCurrentBuffer(): Failed to convert RWF > JSON %s\n", 
								eInfo.rsslError.text);
						rsslReleaseBuffer(pSession->pWritingBuffer, &eInfo.rsslError);
						return RSSL_RET_FAILURE;
					}
				}

			} while (eInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS);

			if (pMsgBuffer != NULL)
			{
				memcpy(pSession->pWritingBuffer->data, pMsgBuffer->data, pMsgBuffer->length);
				pSession->pWritingBuffer->length = pMsgBuffer->length;
				rsslReleaseBuffer(pMsgBuffer, &eInfo.rsslError);
				pMsgBuffer = 0;
			}
		}

		pSession->lastWriteRet = ret = rsslWrite(pChannel, 
												 pSession->pWritingBuffer, 
												 RSSL_HIGH_PRIORITY, 
												 providerThreadConfig.writeFlags, &outBytes, 
												 &uncompOutBytes, pError);

	}
	else // use UPA VA Reactor
	{
		RsslErrorInfo errorInfo;
		RsslReactorSubmitOptions submitOpts;
		rsslClearReactorSubmitOptions(&submitOpts);
		submitOpts.priority = RSSL_HIGH_PRIORITY;
		submitOpts.writeFlags = providerThreadConfig.writeFlags;

		pSession->lastWriteRet = ret = rsslReactorSubmit(pProvThread->pReactor, pSession->pChannelInfo->pReactorChannel, pSession->pWritingBuffer, &submitOpts, &errorInfo);
	}

	/* call flush and write again */
	while (rtrUnlikely(ret == RSSL_RET_WRITE_CALL_AGAIN))
	{
		if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
		{
			if (rtrUnlikely((ret = rsslFlush(pChannel, pError)) < RSSL_RET_SUCCESS))
			{
				printf("rsslFlush() failed with return code %d - <%s>\n", ret, pError->text);
				return ret;
			}
			pSession->lastWriteRet = ret = rsslWrite(pChannel, 
													 pSession->pWritingBuffer, 
													 RSSL_HIGH_PRIORITY, 
													 providerThreadConfig.writeFlags, 
													 &outBytes, &uncompOutBytes, pError);
		}
		else // use UPA VA Reactor
		{
			RsslErrorInfo errorInfo;
			RsslReactorSubmitOptions submitOpts;
			rsslClearReactorSubmitOptions(&submitOpts);
			submitOpts.priority = RSSL_HIGH_PRIORITY;
			submitOpts.writeFlags = providerThreadConfig.writeFlags;

			pSession->lastWriteRet = ret = rsslReactorSubmit(pProvThread->pReactor, pSession->pChannelInfo->pReactorChannel, pSession->pWritingBuffer, &submitOpts, &errorInfo);
		}
	}

	countStatIncr(&pProvThread->bufferSentCount);

	if (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
		rsslReleaseBuffer(pSession->pWritingBuffer, &error);

	if (ret >= RSSL_RET_SUCCESS)
	{
		pSession->pWritingBuffer = 0;
		return ret;
	}

	switch(ret)
	{
		case RSSL_RET_WRITE_FLUSH_FAILED:
			/* If FLUSH_FAILED is received, check the channel state.
			 * if it is still active, it's okay, just need to flush. */
			if (pChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				pSession->pWritingBuffer = 0;
				pSession->lastWriteRet = 1;
				return 1;
			}
			/* Otherwise treat as error, fall through to default. */
		default:
			if (pChannel->state == RSSL_CH_STATE_ACTIVE)
			{
				printf("rsslWrite() failed: %s(%s)\n", rsslRetCodeToString(pError->rsslErrorId), 
						pError->text);
			}
			return ret;
	}
}

static RsslRet getNewBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslUInt32 length, RsslError *pError)
{
	assert(!pSession->pWritingBuffer);
	assert(length);

	if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
	{
		pSession->pWritingBuffer = rsslGetBuffer(pSession->pChannelInfo->pChannel, length, providerThreadConfig.totalBuffersPerPack > 1 ? RSSL_TRUE : RSSL_FALSE , pError);
		if (!pSession->pWritingBuffer)
		{
			if (pError->rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
				printf("rsslGetBuffer() failed: (%d) %s \n", pError->rsslErrorId, pError->text);
			return pError->rsslErrorId;
		}
	}
	else // use UPA VA Reactor
	{
		RsslErrorInfo errorInfo;
		pSession->pWritingBuffer = rsslReactorGetBuffer(pSession->pChannelInfo->pReactorChannel, length, providerThreadConfig.totalBuffersPerPack > 1 ? RSSL_TRUE : RSSL_FALSE , &errorInfo);
		if (!pSession->pWritingBuffer)
		{
			if (errorInfo.rsslError.rsslErrorId != RSSL_RET_BUFFER_NO_BUFFERS)
				printf("rsslReactorGetBuffer() failed: (%d) %s \n", errorInfo.rsslError.rsslErrorId, errorInfo.rsslError.text);
			return errorInfo.rsslError.rsslErrorId;
		}
	}

	assert(pSession->pWritingBuffer->length == length);

	return RSSL_RET_SUCCESS;
}

RsslRet getItemMsgBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslUInt32 length)
{
	RsslError error;
	RsslRet ret = RSSL_RET_SUCCESS;

	if (providerThreadConfig.totalBuffersPerPack == 1) /* Not packing. */
	{
		assert(!pSession->pWritingBuffer);
		if ((ret = getNewBuffer(pProvThread, pSession, length, &error)) < RSSL_RET_SUCCESS)
			return ret;
	}
	else
	{
		/* We are packing, and may need to do something different based on the size of
		 * the message and how much room is left in the present buffer. */

		if (length > providerThreadConfig.packingBufferLength) 
		{
			/* Message too large for our packing buffer, write and get a bigger one. */
			if (pSession->pWritingBuffer)
			{
				pSession->pWritingBuffer->length = 0;
				if ((ret = writeCurrentBuffer(pProvThread, pSession, &error)) < RSSL_RET_SUCCESS)
					return ret;
			}

			if ((ret = getNewBuffer(pProvThread, pSession, length, &error)) < RSSL_RET_SUCCESS)
				return ret;
		}
		else if (!pSession->pWritingBuffer)  
		{
			/* Have no buffer currently, so get a new one. */
			if ((ret = getNewBuffer(pProvThread, pSession, providerThreadConfig.packingBufferLength, &error)) < RSSL_RET_SUCCESS)
				return ret;
		}
		else if (length > pSession->pWritingBuffer->length) 
		{
			/* Out of room in current packing buffer. Write the current one and get a new one. */
			pSession->pWritingBuffer->length = 0;
			if ((ret = writeCurrentBuffer(pProvThread, pSession, &error)) < RSSL_RET_SUCCESS)
				return ret;

			if ((ret = getNewBuffer(pProvThread, pSession, providerThreadConfig.packingBufferLength, &error)) < RSSL_RET_SUCCESS)
				return ret;
		}
		else /* Enough room in current packing buffer, don't need a new one. */
			return RSSL_RET_SUCCESS;
	}

	if (pSession->pWritingBuffer) 
		return RSSL_RET_SUCCESS;
	else
	{
		RsslError error;
		pSession->pWritingBuffer = rsslGetBuffer(pSession->pChannelInfo->pChannel, length, providerThreadConfig.totalBuffersPerPack > 1? RSSL_TRUE : RSSL_FALSE , &error);
		return (pSession->pWritingBuffer) ? RSSL_RET_SUCCESS : error.rsslErrorId;
	}

}

RsslRet sendItemMsgBuffer(ProviderThread *pProvThread, ProviderSession *pSession, RsslBool allowPack)
{
	RsslError error;
	RsslRet ret;
	RsslChannel *pChannel = pSession->pChannelInfo->pChannel;

	countStatIncr(&pProvThread->msgSentCount);

	/* Make sure we stop packing at the end of a burst of updates
	 *   in case the next burst is for a different channel. 
	 *   (This will also prevent any latency updates from sitting in the pack for a tick). */
	if (pSession->packedBufferCount == (providerThreadConfig.totalBuffersPerPack - 1) || !allowPack)
	{
		ret = writeCurrentBuffer(pProvThread, pSession, &error);
		return ret;
	}
	else
	{
		/* Pack the buffer and continue using it. */
		++pSession->packedBufferCount;

		if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
		{
			RsslBuffer *pMsgBuffer = 0;

			pMsgBuffer = 0;
			if (pChannel->protocolType == RSSL_JSON_PROTOCOL_TYPE)
			{
				RsslErrorInfo eInfo;
					/* convert message to JSON */
				do {
					if ((pMsgBuffer = rjcMsgConvertToJson(&(pProvThread->rjcSess), pChannel,
															pSession->pWritingBuffer, &eInfo)) == NULL)
					{
						//fprintf(stderr, "sendItemMsgBuffer(): Failed to convert RWF > JSON %s\n", eInfo.rsslError.text);

						if ((ret = rsslFlush(pChannel, &error)) < RSSL_RET_SUCCESS)
						{
							printf("rsslFlush() failed with return code %d - <%s>\n", 
									ret, error.text);
							return ret;
						}
					}

				} while (eInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS);

				memcpy(pSession->pWritingBuffer->data, pMsgBuffer->data, pMsgBuffer->length);
				pSession->pWritingBuffer->length = pMsgBuffer->length;
				rsslReleaseBuffer(pMsgBuffer, &eInfo.rsslError);
				
			}
			pSession->pWritingBuffer = rsslPackBuffer(pChannel, pSession->pWritingBuffer, &error);
			if (!pSession->pWritingBuffer)
			{
				printf("rsslPackBuffer failed: %d <%s>", error.rsslErrorId, error.text);
				return RSSL_RET_FAILURE;
			}
		}
		else // use UPA VA Reactor
		{
			RsslErrorInfo errorInfo;
			pSession->pWritingBuffer = rsslReactorPackBuffer(pSession->pChannelInfo->pReactorChannel, pSession->pWritingBuffer, &errorInfo);
			if (!pSession->pWritingBuffer)
			{
				printf("rsslReactorPackBuffer failed: %d <%s>", errorInfo.rsslError.rsslErrorId, errorInfo.rsslError.text);
				return RSSL_RET_FAILURE;
			}
		}

		return RSSL_RET_SUCCESS;
	}
}

RsslRet providerSessionAddPublishingItems(ProviderThread *pProvThread, ProviderSession *pSession, RsslInt32 commonItemCount, RsslInt32 itemListUniqueIndex, RsslInt32 uniqueItemCount, RsslUInt16 serviceId)
{
	RsslInt32 i, itemListIndex;
	ItemInfo *itemInfo;
	XmlItemInfoList *pXmlItemInfoList;
	RsslItemAttributes attributes;
	RsslMsgKey msgKey;

	assert(!uniqueItemCount || itemListUniqueIndex >= commonItemCount);

	pXmlItemInfoList = createXmlItemList(providerThreadConfig.itemFilename, itemListUniqueIndex + uniqueItemCount);

	if (!pXmlItemInfoList)
	{
		printf("Failed to load item file '%s'.\n", providerThreadConfig.itemFilename);
		return RSSL_RET_FAILURE;
	}


	attributes.pMsgKey = &msgKey;

	itemListIndex = 0;
	for (i = 0; i < commonItemCount + uniqueItemCount; ++i)
	{
		/* If there are multiple NI-provider connections, each provider must build its
		 * list of items from the items that are common to all providers (if any)
		 * and those unique to that provider. */

		/* Once we have filled our list with the common items,
		 * start using the range of items unique to this consumer thread. */
		if (itemListIndex == commonItemCount
				&& itemListIndex < itemListUniqueIndex)
			itemListIndex = itemListUniqueIndex;

		rsslClearMsgKey(&msgKey);
		msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
		msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		msgKey.serviceId = serviceId;
		msgKey.name.data = pXmlItemInfoList->itemInfoList[itemListIndex].name;
		msgKey.name.length = pXmlItemInfoList->itemInfoList[itemListIndex].nameLength;
		attributes.domainType = pXmlItemInfoList->itemInfoList[itemListIndex].domainType;

		itemInfo = createItemInfo(pProvThread, pSession, &attributes, -i-6);

		if (!itemInfo)
		{
			printf("createItemInfo() failed\n");
			destroyXmlItemList(pXmlItemInfoList);
			return RSSL_RET_FAILURE;
		}
		itemInfo->itemFlags = ITEM_IS_STREAMING_REQ;

		hashTableInsertLink(&pSession->itemAttributesTable, &itemInfo->itemAttributesTableLink, &itemInfo->attributes) ;
		hashTableInsertLink(&pSession->itemStreamIdTable, &itemInfo->itemStreamIdTableLink, &itemInfo->StreamId) ;

		++itemListIndex;
	}

	destroyXmlItemList(pXmlItemInfoList);

	return RSSL_RET_SUCCESS;

}

void providerThreadSendNewChannel(ProviderThread *pProvThread, RsslChannel *pChannel)
{
	NewChannel *pNewChannel = (NewChannel*)malloc(sizeof(NewChannel)); assert(pNewChannel);
	pNewChannel->pChannel = pChannel;
	RSSL_MUTEX_LOCK(&pProvThread->newClientSessionsLock);
	++pProvThread->clientSessionsCount;
	rsslQueueAddLinkToBack(&pProvThread->newClientSessionsList, &pNewChannel->queueLink);
	RSSL_MUTEX_UNLOCK(&pProvThread->newClientSessionsLock);
}

RsslInt32 providerThreadGetConnectionCount(ProviderThread *pProvThread)
{
	RsslInt32 connCount;
	RSSL_MUTEX_LOCK(&pProvThread->newClientSessionsLock);
	connCount = pProvThread->clientSessionsCount;
	RSSL_MUTEX_UNLOCK(&pProvThread->newClientSessionsLock);
	return connCount;
}

void providerThreadReceiveNewChannels(ProviderThread *pProvThread)
{
	RsslQueueLink *pLink;

	do
	{
		/* Check if there are any new connections. */

		RSSL_MUTEX_LOCK(&pProvThread->newClientSessionsLock);
		pLink = rsslQueueRemoveFirstLink(&pProvThread->newClientSessionsList);
		RSSL_MUTEX_UNLOCK(&pProvThread->newClientSessionsLock);

		if (pLink)
		{
			NewChannel *pNewChannel = RSSL_QUEUE_LINK_TO_OBJECT(NewChannel, queueLink, pLink);
			ProviderSession *pProvSession;

			if (!(pProvSession = providerSessionCreate(pProvThread, pNewChannel->pChannel)))
			{
				printf("providerSessionCreate() failed\n");
				exit(-1);
			}

			free(pNewChannel);
		}
	} while (pLink);
}

void providerInit(Provider *pProvider, ProviderType providerType,
		ChannelActiveCallback *processActiveChannel,
		ChannelInactiveCallback *processInactiveChannel,
		MsgCallback *processMsg,
		MsgConverterCallback *convertMsg)
{
	RsslInt32 i;

	memset(pProvider, 0, sizeof(pProvider));
	pProvider->providerType = providerType;
	clearValueStatistics(&pProvider->cpuUsageStats);
	clearValueStatistics(&pProvider->memUsageStats);
	initCountStat(&pProvider->refreshCount);
	initCountStat(&pProvider->updateCount);
	initCountStat(&pProvider->requestCount);
	initCountStat(&pProvider->closeCount);
	initCountStat(&pProvider->postCount);
	initCountStat(&pProvider->outOfBuffersCount);
	initCountStat(&pProvider->msgSentCount);
	initCountStat(&pProvider->bufferSentCount);

	initCountStat(&pProvider->mcastPacketSentCount);
	initCountStat(&pProvider->mcastPacketReceivedCount);
	initCountStat(&pProvider->mcastRetransSentCount);
	initCountStat(&pProvider->mcastRetransReceivedCount);

	pProvider->providerThreadList = (ProviderThread*)malloc(providerThreadConfig.threadCount * sizeof(ProviderThread));

	for(i = 0; i < providerThreadConfig.threadCount; ++i)
	{
		providerThreadInit(&pProvider->providerThreadList[i],
				processActiveChannel, processInactiveChannel, processMsg, convertMsg,
				i, providerType);
		pProvider->providerThreadList[i].cpuId = providerThreadConfig.threadBindList[i];
	}

	if (initResourceUsageStats(&pProvider->resourceStats) != RSSL_RET_SUCCESS)
	{
		printf("initResourceUsageStats() failed.\n");
		exit(-1);
	}
}

void startProviderThreads(Provider *pProvider, RSSL_THREAD_DECLARE(threadFunction,pArg))
{
	RsslInt32 i;

	for (i = 0; i < providerThreadConfig.threadCount; ++i)
	{
		ProviderThread *pProvThread = &pProvider->providerThreadList[i];
		if (!CHECK(RSSL_THREAD_START(&pProvThread->threadId, 
						threadFunction, 
						pProvThread) >= 0))
			exit(-1);
	}
}

void providerWaitForThreads(Provider *pProvider)
{
	RsslInt32 i;
	RsslRet ret;

	for (i = 0; i < providerThreadConfig.threadCount; ++i)
		if ((ret = RSSL_THREAD_JOIN(pProvider->providerThreadList[i].threadId)) < 0)
			printf("Failed to join thread %u: %d\n", 
#ifdef WIN32
			pProvider->providerThreadList[i].threadId.threadId, 
#else
			pProvider->providerThreadList[i].threadId,
#endif
			ret);
}

void providerCleanup(Provider *pProvider)
{
	RsslInt32 i;

	for (i = 0; i < providerThreadConfig.threadCount; ++i)
		providerThreadCleanup(&pProvider->providerThreadList[i]);
	
	free(pProvider->providerThreadList);
}

void providerThreadSendMsgBurst(ProviderThread *pProvThread, RsslUInt64 stopTime)
{
	RsslQueueLink *pLink;

	RSSL_QUEUE_FOR_EACH_LINK(&pProvThread->channelHandler.activeChannelList, pLink)
	{
		ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);
		ProviderSession *pSession;
		RsslRet ret = RSSL_RET_SUCCESS;

		pSession = (ProviderSession*)pChannelInfo->pUserSpec;

		/* The application corrects for ticks that don't finish before the time 
		 * that the next update burst should start.  But don't do this correction 
		 * for new channels. */
		if (rtrUnlikely(stopTime < pSession->timeActivated)) continue; 

		/* Send burst of updates */
		if (ret >= RSSL_RET_SUCCESS && providerThreadConfig.updatesPerSec && rotatingQueueGetCount(&pSession->updateItemList) != 0)
			ret = sendUpdateBurst(pProvThread, pSession);

		/* Send burst of genMsgs */
		if (ret >= RSSL_RET_SUCCESS && providerThreadConfig.genMsgsPerSec && rotatingQueueGetCount(&pSession->updateItemList) != 0)
			ret = sendGenMsgBurst(pProvThread, pSession);

		/* Use remaining time in the tick to send refreshes. */
		while (ret >= RSSL_RET_SUCCESS && rotatingQueueGetCount(&pSession->refreshItemList) != 0
				&& getTimeNano() < stopTime)
			ret = sendRefreshBurst(pProvThread, pSession);

		if (rtrUnlikely(ret < RSSL_RET_SUCCESS))
		{
			switch(ret)
			{
				case RSSL_RET_BUFFER_NO_BUFFERS:
					channelHandlerRequestFlush(&pProvThread->channelHandler, pChannelInfo);
					break;
				default:
					printf("Failure while writing message bursts: %s\n",
							rsslRetCodeToString(ret));
					if (niProvPerfConfig.useReactor == RSSL_FALSE && provPerfConfig.useReactor == RSSL_FALSE) // use UPA Channel
					{
						providerThreadCloseChannel(pProvThread, pChannelInfo); /* Failed to send an update. Remove this client */
					}
					break;
			}
		}
		else if (rtrUnlikely(ret > RSSL_RET_SUCCESS))
		{
			/* Need to flush */
			channelHandlerRequestFlush(&pProvThread->channelHandler, pChannelInfo);
		}
	}
}

void providerCollectStats(Provider *pProvider, RsslBool writeStats, RsslBool displayStats, RsslUInt32 currentRuntimeSec,
		RsslUInt32 timePassedSec)
{
	RsslInt32 i;
	RsslUInt64 refreshCount, updateCount, requestCount, closeCount, postCount, genMsgSentCount, genMsgRecvCount,
			   latencyGenMsgSentCount, latencyGenMsgRecvCount, outOfBuffersCount, msgSentCount, bufferSentCount;
	RsslQueue latencyRecords;

	rsslInitQueue(&latencyRecords);

	clearValueStatistics(&pProvider->intervalMsgEncodingStats);

	if (timePassedSec)
	{
		getResourceUsageStats(&pProvider->resourceStats);
		updateValueStatistics(&pProvider->cpuUsageStats, pProvider->resourceStats.cpuUsageFraction);
		updateValueStatistics(&pProvider->memUsageStats, (double)pProvider->resourceStats.memUsageBytes);
	}


	for(i = 0; i < providerThreadConfig.threadCount; ++i)
	{
		RsslQueueLink *pLink;
		ProviderThread *pProviderThread = &pProvider->providerThreadList[i];

		/* Gather latency records for gen msgs. */
		timeRecordQueueGet(&pProviderThread->genMsgLatencyRecords, &latencyRecords);
		RSSL_QUEUE_FOR_EACH_LINK(&latencyRecords, pLink)
		{
			TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
			double latency = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;

			updateValueStatistics(&pProviderThread->stats.intervalGenMsgLatencyStats, latency);
			updateValueStatistics(&pProviderThread->stats.genMsgLatencyStats, latency);

			if (providerThreadConfig.threadCount > 1)
				updateValueStatistics(&totalStats.genMsgLatencyStats, latency);

			if (pProviderThread->latencyLogFile)
				fprintf(pProviderThread->latencyLogFile, "Gen, %llu, %llu, %llu\n", pRecord->startTime, pRecord->endTime, (pRecord->endTime - pRecord->startTime));
		}
		timeRecordQueueRepool(&pProviderThread->genMsgLatencyRecords, &latencyRecords);

		if (pProviderThread->latencyLogFile)
			fflush(pProviderThread->latencyLogFile);
		
		requestCount = countStatGetChange(&pProviderThread->itemRequestCount);
		refreshCount = countStatGetChange(&pProviderThread->refreshMsgCount);
		updateCount = countStatGetChange(&pProviderThread->updateMsgCount);
		genMsgSentCount = countStatGetChange(&pProviderThread->stats.genMsgSentCount);
		genMsgRecvCount = countStatGetChange(&pProviderThread->stats.genMsgRecvCount);
		latencyGenMsgSentCount = countStatGetChange(&pProviderThread->stats.latencyGenMsgSentCount);
		latencyGenMsgRecvCount = pProviderThread->stats.intervalGenMsgLatencyStats.count;
		closeCount = countStatGetChange(&pProviderThread->closeMsgCount);
		postCount = countStatGetChange(&pProviderThread->postMsgCount);
		outOfBuffersCount = countStatGetChange(&pProviderThread->outOfBuffersCount);
	
		if (writeStats)
		{
			/* Write stats to the stats file. */
			printCurrentTimeUTC(pProviderThread->statsFile);
			switch(pProvider->providerType)
			{
				case PROVIDER_INTERACTIVE:
					fprintf(pProviderThread->statsFile, ", %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %.1f, %.1f, %.1f, %.1f, %.2f, %.2f\n", 
							requestCount,
							refreshCount,
							updateCount,
							postCount,
							genMsgSentCount,
							genMsgRecvCount,
							latencyGenMsgSentCount,
							latencyGenMsgRecvCount,
							pProviderThread->stats.intervalGenMsgLatencyStats.average,
							sqrt(pProviderThread->stats.intervalGenMsgLatencyStats.variance),
							latencyGenMsgRecvCount ? pProviderThread->stats.intervalGenMsgLatencyStats.maxValue : 0.0,
							latencyGenMsgRecvCount ? pProviderThread->stats.intervalGenMsgLatencyStats.minValue : 0.0,
							pProvider->resourceStats.cpuUsageFraction * 100.0f, (double)pProvider->resourceStats.memUsageBytes / 1048576.0);
					break;

				case PROVIDER_NONINTERACTIVE:
					fprintf(pProviderThread->statsFile, ", %llu, %llu, %.2f, %.2f\n", 
							refreshCount,
							updateCount,
							pProvider->resourceStats.cpuUsageFraction * 100.0f, (double)pProvider->resourceStats.memUsageBytes / 1048576.0);
					break;
			}

			fflush(pProviderThread->statsFile);
		}

		/* Add the new counts to the provider's total. */
		countStatAdd(&pProvider->refreshCount, refreshCount);
		countStatAdd(&pProvider->updateCount, updateCount);
		countStatAdd(&pProvider->requestCount, requestCount);
		countStatAdd(&pProvider->closeCount, closeCount);
		countStatAdd(&pProvider->postCount, postCount);
		countStatAdd(&totalStats.genMsgRecvCount, genMsgRecvCount);
		countStatAdd(&totalStats.genMsgSentCount, genMsgSentCount);
		countStatAdd(&totalStats.latencyGenMsgSentCount, latencyGenMsgSentCount);
		countStatAdd(&pProvider->outOfBuffersCount, outOfBuffersCount);

		/* Take packing stats, if packing is enabled. */
		if (providerThreadConfig.totalBuffersPerPack > 1)
		{
			countStatAdd(&pProvider->msgSentCount, countStatGetChange(&pProviderThread->msgSentCount));
			countStatAdd(&pProvider->bufferSentCount, countStatGetChange(&pProviderThread->bufferSentCount));
		}

		if (displayStats)
		{
			/* Print screen stats. */

			if (providerThreadConfig.threadCount == 1)
				printf("%03u: ", currentRuntimeSec);
			else
				printf("%03u: Thread %d:\n  ", currentRuntimeSec, i + 1);

			printf("UpdRate: %8llu, CPU: %6.2f%%, Mem: %6.2fMB\n", 
					updateCount/timePassedSec,
					pProvider->resourceStats.cpuUsageFraction * 100.0f, (double)pProvider->resourceStats.memUsageBytes / 1048576.0);

			switch(pProvider->providerType)
			{
				case PROVIDER_INTERACTIVE:
					if (requestCount > 0 || refreshCount > 0)
						printf("  - Received %llu item requests (total: %llu), sent %llu images (total: %llu)\n",
								requestCount, countStatGetTotal(&pProvider->requestCount), refreshCount, countStatGetTotal(&pProvider->refreshCount));
					if (postCount > 0)
						printf("  Posting: received %llu, reflected %llu\n", postCount, postCount);
					if (genMsgRecvCount > 0 || genMsgSentCount > 0)
						printf("  GenMsgs: sent %llu, received %llu, latencies sent %llu, latencies received %llu\n", genMsgSentCount, genMsgRecvCount, latencyGenMsgSentCount, latencyGenMsgRecvCount);
					if (pProviderThread->stats.intervalGenMsgLatencyStats.count > 0)
					{
						printValueStatistics(stdout, "  GenMsgLat(usec)", "Msgs", &pProviderThread->stats.intervalGenMsgLatencyStats, RSSL_FALSE);
						clearValueStatistics(&pProviderThread->stats.intervalGenMsgLatencyStats);
					}
					break;
				case PROVIDER_NONINTERACTIVE:
					if (requestCount > 0 || refreshCount > 0)
						printf("  - Sent %llu images (total: %llu)\n", refreshCount, countStatGetTotal(&pProvider->refreshCount));
					break;
			}


			closeCount = countStatGetChange(&pProvider->closeCount);
			if (closeCount > 0)
				printf("  - Received %llu closes.\n", closeCount);

			outOfBuffersCount = countStatGetChange(&pProvider->outOfBuffersCount);
			if (outOfBuffersCount > 0)
				printf("  - Stopped %llu updates due to lack of output buffers.\n", outOfBuffersCount);

			if (pProvider->intervalMsgEncodingStats.count > 0)
			{
				printValueStatistics(stdout, "Update Encode Time (usec)", "Msgs", &pProvider->intervalMsgEncodingStats, RSSL_TRUE);
				clearValueStatistics(&pProvider->intervalMsgEncodingStats);
			}

			if (providerThreadConfig.takeMCastStats)
			{
				printf("  - Multicast: Pkts Sent: %llu, Pkts Received: %llu, : Retrans sent: %llu, Retrans received: %llu\n",
						countStatGetChange(&pProvider->mcastPacketSentCount),
						countStatGetChange(&pProvider->mcastPacketReceivedCount),
						countStatGetChange(&pProvider->mcastRetransSentCount),
						countStatGetChange(&pProvider->mcastRetransReceivedCount));
			}

			/* Print packing stats, if packing is enabled. */
			msgSentCount = countStatGetChange(&pProvider->msgSentCount);
			bufferSentCount = countStatGetChange(&pProvider->bufferSentCount);
			if (bufferSentCount > 0)
			{
				printf("  - Approx. avg msgs per pack: %.0f\n", (double)msgSentCount/(double)bufferSentCount);
			}
		}

		if(providerThreadConfig.takeMCastStats)
		{
			RsslChannelInfo chnlInfo;
			RsslQueueLink *pLink;

			RSSL_QUEUE_FOR_EACH_LINK(&pProviderThread->channelHandler.activeChannelList, pLink)
			{
				RsslError error;
				ChannelInfo *pChannelInfo = RSSL_QUEUE_LINK_TO_OBJECT(ChannelInfo, queueLink, pLink);

				if (pChannelInfo->pChannel->state != RSSL_CH_STATE_ACTIVE)
					continue;

				if (rsslGetChannelInfo(pChannelInfo->pChannel, &chnlInfo, &error) != RSSL_RET_SUCCESS)
				{
					printf ("rsslGetChannelInfo() failed. errorId = %d (%s)\n", error.rsslErrorId, error.text);
					continue;
				}

				countStatAdd(&pProvider->mcastPacketSentCount, chnlInfo.multicastStats.mcastSent);

				countStatAdd(&pProvider->mcastPacketReceivedCount, chnlInfo.multicastStats.mcastRcvd);

				countStatAdd(&pProvider->mcastRetransSentCount, chnlInfo.multicastStats.retransPktsSent);

				countStatAdd(&pProvider->mcastRetransReceivedCount, chnlInfo.multicastStats.retransPktsRcvd);
			}
		}

		if (providerThreadConfig.measureEncode)
		{
			RsslQueueLink *pLink;
			RsslQueue timeRecords;

			rsslInitQueue(&timeRecords);

			timeRecordQueueGet(&pProviderThread->messageEncodeTimeRecords, &timeRecords);

			RSSL_QUEUE_FOR_EACH_LINK(&timeRecords, pLink)
			{
				TimeRecord *pRecord = RSSL_QUEUE_LINK_TO_OBJECT(TimeRecord, queueLink, pLink);
				double encodingTime = (double)(pRecord->endTime - pRecord->startTime)/(double)pRecord->ticks;

				updateValueStatistics(&pProvider->intervalMsgEncodingStats, encodingTime);
				updateValueStatistics(&pProvider->msgEncodingStats, encodingTime);
			}

			timeRecordQueueRepool(&pProviderThread->messageEncodeTimeRecords, &timeRecords);
		}
	}
}

void providerPrintSummaryStats(Provider *pProvider, FILE *file)
{
	RsslInt32 i;
	TimeValue statsTime;
	TimeValue currentTime = getTimeNano();

	if (providerThreadConfig.threadCount > 1)
	{
		totalStats.inactiveTime = pProvider->providerThreadList[0].stats.inactiveTime;
		totalStats.firstGenMsgSentTime = pProvider->providerThreadList[0].stats.firstGenMsgSentTime;
		totalStats.firstGenMsgRecvTime = pProvider->providerThreadList[0].stats.firstGenMsgRecvTime;
		for(i = 0; i < providerThreadConfig.threadCount; ++i)
		{
			ProviderThread *pProviderThread = &pProvider->providerThreadList[i];
			ProvStats stats = pProviderThread->stats;
			statsTime = (stats.inactiveTime && stats.inactiveTime < currentTime) ? stats.inactiveTime : currentTime;
			if(stats.inactiveTime && stats.inactiveTime < totalStats.inactiveTime)
				totalStats.inactiveTime = stats.inactiveTime;
			if(stats.firstGenMsgSentTime && stats.firstGenMsgSentTime < totalStats.firstGenMsgSentTime) 
				totalStats.firstGenMsgSentTime = stats.firstGenMsgSentTime;
			if(stats.firstGenMsgRecvTime && stats.firstGenMsgRecvTime < totalStats.firstGenMsgRecvTime)
				totalStats.firstGenMsgRecvTime = stats.firstGenMsgRecvTime;
			fprintf( file, "\n--- THREAD %d SUMMARY ---\n\n", i + 1);

			fprintf(file, "Overall Statistics: \n");

			switch (pProvider->providerType)
			{
				case PROVIDER_INTERACTIVE:
					if (pProviderThread->stats.genMsgLatencyStats.count)
					{
						fprintf( file,
								"  GenMsg latency avg (usec): %.1f\n"
								"  GenMsg latency std dev (usec): %.1f\n"
								"  GenMsg latency max (usec): %.1f\n"
								"  GenMsg latency min (usec): %.1f\n",
								pProviderThread->stats.genMsgLatencyStats.average,
								sqrt(pProviderThread->stats.genMsgLatencyStats.variance),
								pProviderThread->stats.genMsgLatencyStats.maxValue,
								pProviderThread->stats.genMsgLatencyStats.minValue);
					}
					else
						fprintf( file, "  No GenMsg latency information was received.\n");
					if (providerThreadConfig.genMsgsPerSec)
						fprintf(file, "  GenMsgs sent: %llu\n", countStatGetTotal(&pProviderThread->stats.genMsgSentCount));
					if (countStatGetTotal(&pProviderThread->stats.genMsgRecvCount))
						fprintf(file, "  GenMsgs received: %llu\n", countStatGetTotal(&pProviderThread->stats.genMsgRecvCount));
					if (providerThreadConfig.latencyGenMsgsPerSec)
						fprintf(file, "  GenMsg latencies sent: %llu\n", countStatGetTotal(&pProviderThread->stats.latencyGenMsgSentCount));
					if (pProviderThread->stats.genMsgLatencyStats.count)
						fprintf(file, "  GenMsg latencies received: %llu\n", pProviderThread->stats.genMsgLatencyStats.count);
					if (providerThreadConfig.genMsgsPerSec)
					{
						fprintf(file, "  Avg GenMsg send rate: %.0f\n", countStatGetTotal(&pProviderThread->stats.genMsgSentCount)/
							(double)((statsTime - pProviderThread->stats.firstGenMsgSentTime)/1000000000.0));
					}
					if (countStatGetTotal(&pProviderThread->stats.genMsgRecvCount))
					{
						fprintf(file, "  Avg GenMsg receive rate: %.0f\n", countStatGetTotal(&pProviderThread->stats.genMsgRecvCount)/
							(double)((statsTime - pProviderThread->stats.firstGenMsgRecvTime)/1000000000.0));
					}
					if (providerThreadConfig.latencyGenMsgsPerSec)
					{
						fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", countStatGetTotal(&pProviderThread->stats.latencyGenMsgSentCount)/
							(double)((statsTime - pProviderThread->stats.firstGenMsgSentTime)/1000000000.0));
					}
					if (pProviderThread->stats.genMsgLatencyStats.count)
					{
						fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", pProviderThread->stats.genMsgLatencyStats.count/
							(double)((statsTime - pProviderThread->stats.firstGenMsgRecvTime)/1000000000.0));
					}
					fprintf(file, "  Image requests received: %llu\n", countStatGetTotal(&pProviderThread->itemRequestCount));
					if (providerThreadConfig.updatesPerSec)
						fprintf(file, "  Updates sent: %llu\n", countStatGetTotal(&pProviderThread->updateMsgCount));
					if (countStatGetTotal(&pProviderThread->postMsgCount))
					{
						fprintf(file, "  Posts received: %llu\n", countStatGetTotal(&pProviderThread->postMsgCount));
						fprintf(file, "  Posts reflected: %llu\n", countStatGetTotal(&pProviderThread->postMsgCount));
					}
					break;
				case PROVIDER_NONINTERACTIVE:
					fprintf(file,
							"  Images sent: %llu\n"
							"  Updates sent: %llu\n\n",
							countStatGetTotal(&pProviderThread->refreshMsgCount),
							countStatGetTotal(&pProviderThread->updateMsgCount));
					break;
			}
		}
	}
	else
	{
		totalStats = pProvider->providerThreadList[0].stats;
		statsTime = (totalStats.inactiveTime && totalStats.inactiveTime < currentTime) ? totalStats.inactiveTime : currentTime;
	}

	fprintf( file, "\n--- OVERALL SUMMARY ---\n\n");

	fprintf(file, "Overall Statistics: \n");

	switch (pProvider->providerType)
	{
		case PROVIDER_INTERACTIVE:
			if (totalStats.genMsgLatencyStats.count)
			{
				fprintf( file,
						"  GenMsg latency avg (usec): %.1f\n"
						"  GenMsg latency std dev (usec): %.1f\n"
						"  GenMsg latency max (usec): %.1f\n"
						"  GenMsg latency min (usec): %.1f\n",
						totalStats.genMsgLatencyStats.average,
						sqrt(totalStats.genMsgLatencyStats.variance),
						totalStats.genMsgLatencyStats.maxValue,
						totalStats.genMsgLatencyStats.minValue);
			}
			else
				fprintf( file, "  No GenMsg latency information was received.\n");
			if (providerThreadConfig.genMsgsPerSec)
				fprintf(file, "  GenMsgs sent: %llu\n", countStatGetTotal(&totalStats.genMsgSentCount));
			if (countStatGetTotal(&totalStats.genMsgRecvCount))
				fprintf(file, "  GenMsgs received: %llu\n", countStatGetTotal(&totalStats.genMsgRecvCount));
			if (providerThreadConfig.latencyGenMsgsPerSec)
				fprintf(file, "  GenMsg latencies sent: %llu\n", countStatGetTotal(&totalStats.latencyGenMsgSentCount));
			if (totalStats.genMsgLatencyStats.count)
				fprintf(file, "  GenMsg latencies received: %llu\n", totalStats.genMsgLatencyStats.count);
			if (providerThreadConfig.genMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg send rate: %.0f\n", countStatGetTotal(&totalStats.genMsgSentCount)/
					(double)((statsTime - totalStats.firstGenMsgSentTime)/1000000000.0));
			}
			if (countStatGetTotal(&totalStats.genMsgRecvCount))
			{
				fprintf(file, "  Avg GenMsg receive rate: %.0f\n", countStatGetTotal(&totalStats.genMsgRecvCount)/
					(double)((statsTime - totalStats.firstGenMsgRecvTime)/1000000000.0));
			}
			if (providerThreadConfig.latencyGenMsgsPerSec)
			{
				fprintf(file, "  Avg GenMsg latency send rate: %.0f\n", countStatGetTotal(&totalStats.latencyGenMsgSentCount)/
					(double)((statsTime - totalStats.firstGenMsgSentTime)/1000000000.0));
			}
			if (totalStats.genMsgLatencyStats.count)
			{
				fprintf(file, "  Avg GenMsg latency receive rate: %.0f\n", totalStats.genMsgLatencyStats.count/
					(double)((statsTime - totalStats.firstGenMsgRecvTime)/1000000000.0));
			}
			fprintf(file, "  Image requests received: %llu\n", countStatGetTotal(&pProvider->requestCount));
			if (providerThreadConfig.updatesPerSec)
				fprintf(file, "  Updates sent: %llu\n", countStatGetTotal(&pProvider->updateCount));
			if (countStatGetTotal(&pProvider->postCount))
			{
				fprintf(file, "  Posts received: %llu\n", countStatGetTotal(&pProvider->postCount));
				fprintf(file, "  Posts reflected: %llu\n", countStatGetTotal(&pProvider->postCount));
			}
			break;
		case PROVIDER_NONINTERACTIVE:
			fprintf(file,
					"  Images sent: %llu\n"
					"  Updates sent: %llu\n",
					countStatGetTotal(&pProvider->refreshCount),
					countStatGetTotal(&pProvider->updateCount));
			break;
	}

	if (pProvider->cpuUsageStats.count)
	{
		assert(pProvider->memUsageStats.count);
		fprintf( file,
				"  CPU/Memory samples: %llu\n"
				"  CPU Usage max (%%): %.2f\n"
				"  CPU Usage min (%%): %.2f\n"
				"  CPU Usage avg (%%): %.2f\n"
				"  Memory Usage max (MB): %.2f\n"
				"  Memory Usage min (MB): %.2f\n"
				"  Memory Usage avg (MB): %.2f\n",
				pProvider->cpuUsageStats.count,
				pProvider->cpuUsageStats.maxValue * 100.0,
				pProvider->cpuUsageStats.minValue * 100.0,
				pProvider->cpuUsageStats.average * 100.0,
				pProvider->memUsageStats.maxValue / 1048576.0,
				pProvider->memUsageStats.minValue / 1048576.0,
				pProvider->memUsageStats.average / 1048576.0
			   );
	}

	printf("\n");
}
