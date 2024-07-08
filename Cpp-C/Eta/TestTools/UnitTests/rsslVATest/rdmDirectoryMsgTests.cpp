/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "testFramework.h"
#include "rsslVATestUtil.h"
#include "gtest/gtest.h"
#include "rtr/rsslVAUtils.h"

static TypedMessageStats stats;
static TestWriteAction testWriteAction;

void directoryRequestMsgTests();
void directoryConsStatusMsgTests();
void directoryCloseMsgTests();
void directoryRefreshMsgTests();
void directoryUpdateMsgTests();
void directoryServiceListTests();
void directoryStatusMsgTests();

void initServiceFlags();
void cleanupServiceFlags();

class DirectoryMsgTest : public ::testing::Test {
public:

	static void SetUpTestCase()
	{
		initServiceFlags();
	}

	static void TearDownTestCase()
	{
		cleanupServiceFlags();
	}
};

TEST_F(DirectoryMsgTest, ServiceListTests)
{
	directoryServiceListTests();
}

TEST_F(DirectoryMsgTest, RequestMsgTests)
{
	directoryRequestMsgTests();
}

TEST_F(DirectoryMsgTest, ConsStatusMsgTests)
{
	directoryConsStatusMsgTests();
}

TEST_F(DirectoryMsgTest, CloseMsgTests)
{
	directoryCloseMsgTests();
}

TEST_F(DirectoryMsgTest, RefreshMsgTests)
{
	directoryRefreshMsgTests();
}

TEST_F(DirectoryMsgTest, UpdateMsgTests)
{
	directoryUpdateMsgTests();
}

TEST_F(DirectoryMsgTest, StatusMsgTests)
{
	directoryStatusMsgTests();
}

/* Service Flags */
static RsslUInt32 serviceFlagsBase[] =
{
	RDM_SVCF_HAS_INFO,
	RDM_SVCF_HAS_STATE,
	RDM_SVCF_HAS_LOAD,
	RDM_SVCF_HAS_DATA,
	RDM_SVCF_HAS_LINK,
};
static RsslUInt32 *serviceFlagsList, serviceFlagsListCount;

static RsslUInt32 serviceInfoFlagsBase[] =
{
	RDM_SVC_IFF_HAS_VENDOR,
	RDM_SVC_IFF_HAS_IS_SOURCE,
	RDM_SVC_IFF_HAS_DICTS_PROVIDED,
	RDM_SVC_IFF_HAS_DICTS_USED,
	RDM_SVC_IFF_HAS_QOS,
	RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE,
	RDM_SVC_IFF_HAS_ITEM_LIST,
	RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS,
	RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS,
};
static RsslUInt32 *serviceInfoFlagsList, serviceInfoFlagsListCount;

static RsslUInt32 serviceStateFlagsBase[] =
{
	RDM_SVC_STF_HAS_ACCEPTING_REQS,
	RDM_SVC_STF_HAS_STATUS,
};
static RsslUInt32 *serviceStateFlagsList, serviceStateFlagsListCount;

static RsslUInt32 serviceGroupFlagsBase[] =
{
	RDM_SVC_GRF_HAS_MERGED_TO_GROUP,
	RDM_SVC_GRF_HAS_STATUS,
};
static RsslUInt32 *serviceGroupFlagsList, serviceGroupFlagsListCount;

static RsslUInt32 serviceLoadFlagsBase[] =
{
	RDM_SVC_LDF_HAS_OPEN_LIMIT,
	RDM_SVC_LDF_HAS_OPEN_WINDOW,
	RDM_SVC_LDF_HAS_LOAD_FACTOR,
};
static RsslUInt32 *serviceLoadFlagsList, serviceLoadFlagsListCount;

static RsslUInt32 serviceDataFlagsBase[] =
{
	RDM_SVC_DTF_HAS_DATA,
};
static RsslUInt32 *serviceDataFlagsList, serviceDataFlagsListCount;

static RsslUInt32 serviceLinkFlagsBase[] =
{
	RDM_SVC_LKF_HAS_TYPE,
	RDM_SVC_LKF_HAS_CODE,
	RDM_SVC_LKF_HAS_TEXT,
};
static RsslUInt32 *serviceLinkFlagsList, serviceLinkFlagsListCount;


static char serviceMemoryBlock[16384];
static RsslBuffer serviceMemoryBuffer;

void initServiceFlags()
{
	serviceFlagsListCount = _createFlagCombinations(&serviceFlagsList, serviceFlagsBase, sizeof(serviceFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceInfoFlagsListCount = _createFlagCombinations(&serviceInfoFlagsList, serviceInfoFlagsBase, sizeof(serviceInfoFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceStateFlagsListCount = _createFlagCombinations(&serviceStateFlagsList, serviceStateFlagsBase, sizeof(serviceStateFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceGroupFlagsListCount = _createFlagCombinations(&serviceGroupFlagsList, serviceGroupFlagsBase, sizeof(serviceGroupFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceLoadFlagsListCount = _createFlagCombinations(&serviceLoadFlagsList, serviceLoadFlagsBase, sizeof(serviceLoadFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceDataFlagsListCount = _createFlagCombinations(&serviceDataFlagsList, serviceDataFlagsBase, sizeof(serviceDataFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
	serviceLinkFlagsListCount = _createFlagCombinations(&serviceLinkFlagsList, serviceLinkFlagsBase, sizeof(serviceLinkFlagsBase)/sizeof(RsslUInt32), RSSL_FALSE);
}

void cleanupServiceFlags()
{
	free(serviceFlagsList);
	free(serviceInfoFlagsList);
	free(serviceStateFlagsList);
	free(serviceGroupFlagsList);
	free(serviceLoadFlagsList);
	free(serviceDataFlagsList);
	free(serviceLinkFlagsList);
}

/* Fill in missing parts of service based on flag & count settings. */
/* Uses an RsslBuffer for allocating. This should be considerably faster than malloc'ing and freeing everything. */
void initServiceElements(RsslRDMService *pService, RsslBuffer *pMemoryBuffer, RsslUInt32 groupFlags, RsslUInt32 linkFlags)
{
	RsslUInt32 i;

	RsslUInt serviceId = pService->serviceId;

	/* Info */
	RsslUInt isSource = serviceId + 2;
	RsslUInt supportsQosRange = serviceId + 3;
	RsslUInt supportsOutOfBandSnapshots = serviceId + 4;
	RsslUInt acceptingConsumerStatus = serviceId + 5;

	/* State */
	RsslUInt serviceState = serviceId + 6;
	RsslUInt acceptingRequests = serviceId + 7;

	/* Load */
	RsslUInt openLimit = serviceId + 8;
	RsslUInt openWindow = serviceId + 9;
	RsslUInt loadFactor = serviceId + 10;

	/* Data */
	RsslUInt type = serviceId + 11;

	/* Link */
	RsslUInt linkType = serviceId + 12;
	RsslUInt linkState = serviceId + 13;
	RsslUInt linkCode = serviceId + 14;

	/* Info */
	if (pService->flags & RDM_SVCF_HAS_INFO)
	{
		RsslRDMServiceInfo *pServiceInfo = &pService->info;

		ASSERT_TRUE(pServiceInfo->serviceName.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 16, sizeof(char)));
		pServiceInfo->serviceName.length = snprintf(pServiceInfo->serviceName.data, 16, "Service%llu", serviceId + 1ULL);

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
		{
			ASSERT_TRUE(pServiceInfo->vendor.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 16, sizeof(char)));
			pServiceInfo->vendor.length = snprintf(pServiceInfo->vendor.data, 16, "Vendor%llu", serviceId + 2ULL);
		}

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_IS_SOURCE)
			pServiceInfo->isSource = isSource;

		ASSERT_TRUE(pServiceInfo->capabilitiesList = (RsslUInt*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pServiceInfo->capabilitiesCount, sizeof(RsslUInt)));
		for(i = 0; i < pServiceInfo->capabilitiesCount; ++i)
			pServiceInfo->capabilitiesList[i] = (serviceId + i) % 256;

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
		{
			ASSERT_TRUE(pServiceInfo->dictionariesProvidedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pServiceInfo->dictionariesProvidedCount, sizeof(RsslBuffer)));
			for(i = 0; i < pServiceInfo->dictionariesProvidedCount; ++i)
			{
				ASSERT_TRUE(pServiceInfo->dictionariesProvidedList[i].data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
				pServiceInfo->dictionariesProvidedList[i].length = snprintf(pServiceInfo->dictionariesProvidedList[i].data, 32, "DictionaryProv%llu", serviceId + 2ULL);
			}
		}

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
		{
			ASSERT_TRUE(pServiceInfo->dictionariesUsedList = (RsslBuffer*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pServiceInfo->dictionariesUsedCount, sizeof(RsslBuffer)));
			for(i = 0; i < pServiceInfo->dictionariesUsedCount; ++i)
			{
				ASSERT_TRUE(pServiceInfo->dictionariesUsedList[i].data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
				pServiceInfo->dictionariesUsedList[i].length = snprintf(pServiceInfo->dictionariesUsedList[i].data, 32, "DictionaryProv%llu", serviceId + 2ULL);
			}
		}

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_QOS)
		{
			ASSERT_TRUE(pServiceInfo->qosList = (RsslQos*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pServiceInfo->qosCount, sizeof(RsslQos)));
			for(i = 0; i < pServiceInfo->qosCount; ++i)
			{
				RsslQos *pQos = &pServiceInfo->qosList[i];
				rsslClearQos(pQos);
				pQos->rate = RSSL_QOS_RATE_JIT_CONFLATED;
				pQos->timeliness = RSSL_QOS_TIME_DELAYED;
				pQos->timeInfo = (RsslUInt16)(serviceId + 3);
			}
		}

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST)
		{
			ASSERT_TRUE(pServiceInfo->itemList.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceInfo->itemList.length = snprintf(pServiceInfo->itemList.data, 32, "ItemList%llu", serviceId + 2ULL);
		}

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
			pServiceInfo->supportsQosRange = supportsQosRange;

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS)
			pServiceInfo->supportsOutOfBandSnapshots = supportsOutOfBandSnapshots;

		if (pServiceInfo->flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS)
			pServiceInfo->acceptingConsumerStatus = acceptingConsumerStatus;
	}

	/* State */
	if (pService->flags & RDM_SVCF_HAS_STATE)
	{
		RsslRDMServiceState *pServiceState = &pService->state;

		pServiceState->serviceState = serviceState;

		if (pServiceState->flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
			pServiceState->acceptingRequests = acceptingRequests;

		if (pServiceState->flags & RDM_SVC_STF_HAS_STATUS)
		{
			pServiceState->status.streamState = RSSL_STREAM_OPEN;
			pServiceState->status.dataState = RSSL_DATA_SUSPECT;
			ASSERT_TRUE(pServiceState->status.text.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceState->status.text.length = snprintf(pServiceState->status.text.data, 32, "StateText%llu", serviceId + 2ULL);
		}
	} 

	/* Group */
	ASSERT_TRUE(pService->groupStateList = (RsslRDMServiceGroupState*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pService->groupStateCount, sizeof(RsslRDMServiceGroupState)));
	for (i = 0; i < pService->groupStateCount; ++i)
	{
		RsslRDMServiceGroupState *pServiceGroupState = &pService->groupStateList[i];
		RsslFilterEntryActions groupFilterAction = (i % 2) ? RSSL_FTEA_SET_ENTRY : RSSL_FTEA_CLEAR_ENTRY;

		rsslClearRDMServiceGroupState(pServiceGroupState);

		pServiceGroupState->flags = groupFlags;
		pServiceGroupState->action = groupFilterAction;

		ASSERT_TRUE(pServiceGroupState->group.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
		pServiceGroupState->group.length = snprintf(pServiceGroupState->group.data, 32, "Group%llu", serviceId + i + 2ULL);

		if (groupFlags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
		{
			ASSERT_TRUE(pServiceGroupState->mergedToGroup.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceGroupState->mergedToGroup.length = snprintf(pServiceGroupState->mergedToGroup.data, 32, "MergeGroup%llu", serviceId + i + 2ULL);
		}

		if (groupFlags & RDM_SVC_GRF_HAS_STATUS)
		{
			pServiceGroupState->status.streamState = RSSL_STREAM_OPEN;
			pServiceGroupState->status.dataState = RSSL_DATA_SUSPECT;
			pServiceGroupState->status.code = RSSL_SC_TIMEOUT;
			ASSERT_TRUE(pServiceGroupState->status.text.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceGroupState->status.text.length = snprintf(pServiceGroupState->status.text.data, 32, "GroupStateText%llu", serviceId + i + 2ULL);
		}
	}

	/* Load */
	if (pService->flags & RDM_SVCF_HAS_LOAD)
	{
		RsslRDMServiceLoad *pServiceLoad = &pService->load;

		if (pServiceLoad->flags & RDM_SVC_LDF_HAS_OPEN_LIMIT)
			pServiceLoad->openLimit = openLimit;

		if (pServiceLoad->flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
			pServiceLoad->openWindow = openWindow;

		if (pServiceLoad->flags & RDM_SVC_LDF_HAS_LOAD_FACTOR)
			pServiceLoad->loadFactor = loadFactor;
	}

	/* Data */
	if (pService->flags & RDM_SVCF_HAS_DATA)
	{
		RsslRDMServiceData *pServiceData = &pService->data;

		pServiceData->type = type;

		if (pServiceData->flags & RDM_SVC_DTF_HAS_DATA)
		{
			pServiceData->dataType = RSSL_DT_OPAQUE;
			ASSERT_TRUE(pServiceData->data.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceData->data.length = snprintf(pServiceData->data.data, 32, "SomeData%llu", serviceId + 2ULL);
		}
	}

	/* Link */
	if (pService->flags & RDM_SVCF_HAS_LINK)
	{
		ASSERT_TRUE(pService->linkInfo.linkList = (RsslRDMServiceLink*)rsslReserveAlignedBufferMemory(pMemoryBuffer, pService->linkInfo.linkCount, sizeof(RsslRDMServiceLink)));

		for (i = 0; i < pService->linkInfo.linkCount; ++i)
		{
			RsslRDMServiceLink *pServiceLink = &pService->linkInfo.linkList[i];

			pServiceLink->action = i % 2 ? RSSL_MPEA_DELETE_ENTRY : RSSL_MPEA_UPDATE_ENTRY;
			pServiceLink->flags = linkFlags;

			ASSERT_TRUE(pServiceLink->name.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
			pServiceLink->name.length = snprintf(pServiceLink->name.data, 32, "Link%llu", serviceId + 2ULL);

			if (linkFlags & RDM_SVC_LKF_HAS_TYPE)
				pServiceLink->type = linkType;

			if (linkFlags & RDM_SVC_LKF_HAS_CODE)
				pServiceLink->linkCode = linkCode;

			pServiceLink->linkState = linkState;

			if (linkFlags & RDM_SVC_LKF_HAS_TEXT)
			{
				ASSERT_TRUE(pServiceLink->text.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, 32, sizeof(char)));
				pServiceLink->text.length = snprintf(pServiceLink->text.data, 32, "SomeData%llu", serviceId + 2ULL);
			}
		}
	}
}

void testCompareService(RsslRDMService *pService, RsslRDMService *pCompareService)
{
	RsslUInt32 i;

	ASSERT_TRUE(pService->action == pCompareService->action);
	if (pService->action == RSSL_MPEA_DELETE_ENTRY) return;

	ASSERT_TRUE(pService->flags == pCompareService->flags);

	/* Info */
	if (pService->flags & RDM_SVCF_HAS_INFO)
	{
		RsslRDMServiceInfo *pServiceInfo = &pService->info, *pCompareServiceInfo = &pCompareService->info;

		ASSERT_TRUE(pServiceInfo->action == pCompareServiceInfo->action);

		if (pServiceInfo->action != RSSL_FTEA_CLEAR_ENTRY)
		{

			ASSERT_TRUE(pServiceInfo->flags == pCompareServiceInfo->flags);

			ASSERT_TRUE(rsslBufferIsEqual(&pServiceInfo->serviceName, &pCompareServiceInfo->serviceName));
			ASSERT_TRUE(pServiceInfo->serviceName.data != pCompareServiceInfo->serviceName.data); /* deep-copy check */

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_VENDOR)
			{
				ASSERT_TRUE(rsslBufferIsEqual(&pServiceInfo->vendor, &pCompareServiceInfo->vendor));
				ASSERT_TRUE(pServiceInfo->vendor.data != pCompareServiceInfo->vendor.data); /* deep-copy check */
			}

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_IS_SOURCE)
				ASSERT_TRUE(pServiceInfo->isSource == pCompareServiceInfo->isSource);

			for(i = 0; i < pServiceInfo->capabilitiesCount; ++i)
				ASSERT_TRUE(pServiceInfo->capabilitiesList[i] == pCompareServiceInfo->capabilitiesList[i]);

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED)
			{
				for(i = 0; i < pServiceInfo->dictionariesProvidedCount; ++i)
				{
					ASSERT_TRUE(rsslBufferIsEqual(&pServiceInfo->dictionariesProvidedList[i], &pCompareServiceInfo->dictionariesProvidedList[i]));
					ASSERT_TRUE(pServiceInfo->dictionariesProvidedList[i].data != pCompareServiceInfo->dictionariesProvidedList[i].data); /* deep-copy check */
				}
			}

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_DICTS_USED)
			{
				for(i = 0; i < pServiceInfo->dictionariesUsedCount; ++i)
				{
					ASSERT_TRUE(rsslBufferIsEqual(&pServiceInfo->dictionariesUsedList[i], &pCompareServiceInfo->dictionariesUsedList[i]));
					ASSERT_TRUE(pServiceInfo->dictionariesUsedList[i].data != pCompareServiceInfo->dictionariesUsedList[i].data); /* deep-copy check */
				}
			}

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_QOS)
			{
				for(i = 0; i < pServiceInfo->qosCount; ++i)
				{
					RsslQos *pQos = &pServiceInfo->qosList[i], *pCompareQos = &pCompareServiceInfo->qosList[i];

					ASSERT_TRUE(pQos->rate == pCompareQos->rate);
					ASSERT_TRUE(pQos->timeliness == pCompareQos->timeliness);

					if (pQos->rate == RSSL_QOS_RATE_TIME_CONFLATED)
						ASSERT_TRUE(pQos->rateInfo == pCompareQos->rateInfo);

					if (pQos->timeliness == RSSL_QOS_TIME_DELAYED)
						ASSERT_TRUE(pQos->timeInfo == pCompareQos->timeInfo);
				}
			}

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_ITEM_LIST)
			{
				ASSERT_TRUE(rsslBufferIsEqual(&pServiceInfo->itemList, &pCompareServiceInfo->itemList));
				ASSERT_TRUE(pServiceInfo->itemList.data != pCompareServiceInfo->itemList.data); /* deep-copy check */
			}

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE)
				ASSERT_TRUE(pServiceInfo->supportsQosRange == pCompareServiceInfo->supportsQosRange);

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS)
				ASSERT_TRUE(pServiceInfo->supportsOutOfBandSnapshots == pCompareServiceInfo->supportsOutOfBandSnapshots);

			if (pServiceInfo->flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS)
				ASSERT_TRUE(pServiceInfo->acceptingConsumerStatus == pCompareServiceInfo->acceptingConsumerStatus);
		}
	}

	/* State */
	if (pService->flags & RDM_SVCF_HAS_STATE)
	{
		RsslRDMServiceState *pServiceState = &pService->state, *pCompareServiceState = &pCompareService->state;

		ASSERT_TRUE(pServiceState->action == pCompareServiceState->action);
		if (pServiceState->action != RSSL_FTEA_CLEAR_ENTRY)
		{
			ASSERT_TRUE(pServiceState->flags == pCompareServiceState->flags);

			if (pServiceState->flags & RDM_SVC_STF_HAS_ACCEPTING_REQS)
				ASSERT_TRUE(pServiceState->acceptingRequests == pCompareServiceState->acceptingRequests);

			if (pServiceState->flags & RDM_SVC_STF_HAS_STATUS)
			{
				ASSERT_TRUE(pServiceState->status.streamState == pCompareServiceState->status.streamState);
				ASSERT_TRUE(pServiceState->status.dataState == pCompareServiceState->status.dataState);
				ASSERT_TRUE(pServiceState->status.code == pCompareServiceState->status.code);

				ASSERT_TRUE(rsslBufferIsEqual(&pServiceState->status.text, &pCompareServiceState->status.text));
				ASSERT_TRUE(pServiceState->status.text.data != pCompareServiceState->status.text.data); /* deep-copy check */
			}
		}
	} 

	/* Group */
	ASSERT_TRUE(pService->groupStateCount == pCompareService->groupStateCount);
	for (i = 0; i < pService->groupStateCount; ++i)
	{
		RsslRDMServiceGroupState *pServiceGroupState = &pService->groupStateList[i], *pCompareServiceGroupState = &pCompareService->groupStateList[i];

		ASSERT_TRUE(pServiceGroupState->action == pCompareServiceGroupState->action);
		if (pServiceGroupState->action != RSSL_FTEA_CLEAR_ENTRY)
		{
			ASSERT_TRUE(pServiceGroupState->flags == pCompareServiceGroupState->flags);
			ASSERT_TRUE(rsslBufferIsEqual(&pServiceGroupState->group, &pCompareServiceGroupState->group));
			ASSERT_TRUE(pServiceGroupState->group.data  != pCompareServiceGroupState->group.data); /* deep-copy check */

			if (pServiceGroupState->flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
			{
				ASSERT_TRUE(rsslBufferIsEqual(&pServiceGroupState->mergedToGroup, &pCompareServiceGroupState->mergedToGroup));
				ASSERT_TRUE(pServiceGroupState->mergedToGroup.data  != pCompareServiceGroupState->mergedToGroup.data); /* deep-copy check */
			}

			if (pServiceGroupState->flags & RDM_SVC_GRF_HAS_STATUS)
			{
				ASSERT_TRUE(pServiceGroupState->status.streamState == pCompareServiceGroupState->status.streamState);
				ASSERT_TRUE(pServiceGroupState->status.dataState == pCompareServiceGroupState->status.dataState);
				ASSERT_TRUE(pServiceGroupState->status.code == pCompareServiceGroupState->status.code);

				ASSERT_TRUE(rsslBufferIsEqual(&pServiceGroupState->status.text, &pCompareServiceGroupState->status.text));
				ASSERT_TRUE(pServiceGroupState->status.text.data != pCompareServiceGroupState->status.text.data); /* deep-copy check */
			}
		}
	}

	/* Load */
	if (pService->flags & RDM_SVCF_HAS_LOAD)
	{
		RsslRDMServiceLoad *pServiceLoad = &pService->load, *pCompareServiceLoad = &pCompareService->load;

		ASSERT_TRUE(pServiceLoad->action == pCompareServiceLoad->action);

		if (pServiceLoad->action != RSSL_FTEA_CLEAR_ENTRY)
		{
			ASSERT_TRUE(pServiceLoad->flags == pCompareServiceLoad->flags);

			if (pServiceLoad->flags & RDM_SVC_LDF_HAS_OPEN_LIMIT)
				ASSERT_TRUE(pServiceLoad->openLimit == pCompareServiceLoad->openLimit);

			if (pServiceLoad->flags & RDM_SVC_LDF_HAS_OPEN_WINDOW)
				ASSERT_TRUE(pServiceLoad->openWindow == pCompareServiceLoad->openWindow);

			if (pServiceLoad->flags & RDM_SVC_LDF_HAS_LOAD_FACTOR)
				ASSERT_TRUE(pServiceLoad->loadFactor == pCompareServiceLoad->loadFactor);
		}
	}

	/* Data */
	if (pService->flags & RDM_SVCF_HAS_DATA)
	{
		RsslRDMServiceData *pServiceData = &pService->data, *pCompareServiceData = &pCompareService->data;

		ASSERT_TRUE(pServiceData->action == pCompareServiceData->action);

		if (pServiceData->action != RSSL_FTEA_CLEAR_ENTRY)
		{
			ASSERT_TRUE(pServiceData->flags == pCompareServiceData->flags);

			if (pServiceData->flags & RDM_SVC_DTF_HAS_DATA)
			{
				ASSERT_TRUE(pServiceData->type == pCompareServiceData->type);
				ASSERT_TRUE(pServiceData->dataType == pCompareServiceData->dataType);
				ASSERT_TRUE(rsslBufferIsEqual(&pServiceData->data, &pCompareServiceData->data));
				ASSERT_TRUE(pServiceData->data.data != pCompareServiceData->data.data); /* deep-copy check */
			}
		}
	}

	/* Link */
	if (pService->flags & RDM_SVCF_HAS_LINK)
	{
		ASSERT_TRUE(pService->linkInfo.action == pCompareService->linkInfo.action);

		if (pService->linkInfo.action != RSSL_FTEA_CLEAR_ENTRY)
		{
			ASSERT_TRUE(pService->linkInfo.linkCount == pCompareService->linkInfo.linkCount);

			for (i = 0; i < pService->linkInfo.linkCount; ++i)
			{
				RsslRDMServiceLink *pServiceLink = &pService->linkInfo.linkList[i], *pCompareServiceLink = &pCompareService->linkInfo.linkList[i];

				ASSERT_TRUE(pServiceLink->action == pCompareServiceLink->action);
				if (pServiceLink->action != RSSL_MPEA_DELETE_ENTRY)
				{
					ASSERT_TRUE(pServiceLink->flags == pCompareServiceLink->flags);

					ASSERT_TRUE(rsslBufferIsEqual(&pServiceLink->name, &pCompareServiceLink->name));
					ASSERT_TRUE(pServiceLink->name.data != pCompareServiceLink->name.data); /* deep-copy check */

					if (pServiceLink->flags & RDM_SVC_LKF_HAS_TYPE)
						ASSERT_TRUE(pServiceLink->type == pCompareServiceLink->type);

					if (pServiceLink->flags & RDM_SVC_LKF_HAS_CODE)
						ASSERT_TRUE(pServiceLink->linkCode == pCompareServiceLink->linkCode);

					ASSERT_TRUE(pServiceLink->linkState == pCompareServiceLink->linkState);

					if (pServiceLink->flags & RDM_SVC_LKF_HAS_TEXT)
					{
						ASSERT_TRUE(rsslBufferIsEqual(&pServiceLink->text, &pCompareServiceLink->text));
						ASSERT_TRUE(pServiceLink->text.data != pCompareServiceLink->text.data); /* deep-copy check */
					}
				}
			}
		}
	}
}

void directoryRequestMsgTests()
{
	RsslRDMDirectoryRequest encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryRequest *pDecRDMMsg;
	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DR_RQF_HAS_SERVICE_ID,
		RDM_DR_RQF_STREAMING
	};

	RsslUInt32 *flagsList, flagsListCount;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt16 serviceId = 273;

	clearTypedMessageStats(&stats);

	/* Request */
	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	for (i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslRet ret;

			testWriteAction = testWriteActions[j];


			/*** Encode ***/
			rsslClearRDMDirectoryRequest(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

			encRDMMsg.flags = flagsList[i];

			encRDMMsg.rdmMsgBase.streamId = streamId;

			/* Set parameters based on flags */
			if (encRDMMsg.flags & RDM_DR_RQF_HAS_SERVICE_ID)
				encRDMMsg.serviceId = serviceId;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.directoryMsg.request;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryRequest*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

			ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

			/* Check parameters */
			if (pDecRDMMsg->flags & RDM_DR_RQF_HAS_SERVICE_ID)
				ASSERT_TRUE(pDecRDMMsg->serviceId == serviceId);

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	free(flagsList);

	//printTypedMessageStats(&stats);
}

void directoryConsStatusMsgTests()
{
	RsslRDMDirectoryConsumerStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryConsumerStatus *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslUInt16 serviceId = 273;

	RsslUInt32 j;

	RsslRDMConsumerStatusService serviceStatusList[3];
	RsslInt32 serviceStatusCount = sizeof(serviceStatusList)/sizeof(RsslRDMConsumerStatusService);

	rsslClearRDMConsumerStatusService(&serviceStatusList[0]);
	serviceStatusList[0].serviceId = 2;
	serviceStatusList[0].action = RSSL_MPEA_ADD_ENTRY;
	serviceStatusList[0].sourceMirroringMode = RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY;

	rsslClearRDMConsumerStatusService(&serviceStatusList[1]);
	serviceStatusList[1].serviceId = 4;
	serviceStatusList[1].action = RSSL_MPEA_ADD_ENTRY;
	serviceStatusList[1].sourceMirroringMode = 0;

	rsslClearRDMConsumerStatusService(&serviceStatusList[1]);
	serviceStatusList[1].serviceId = 5;
	serviceStatusList[1].action = RSSL_MPEA_DELETE_ENTRY;
	serviceStatusList[1].sourceMirroringMode = 0;

	rsslClearRDMConsumerStatusService(&serviceStatusList[2]);
	serviceStatusList[2].serviceId = 6;
	serviceStatusList[2].action = RSSL_MPEA_UPDATE_ENTRY;
	serviceStatusList[2].sourceMirroringMode = RDM_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY;

	clearTypedMessageStats(&stats);

	for (j = 0; j < testWriteActionsCount; ++j)
	{
		RsslInt32 k;

		for(k = 0; k <= serviceStatusCount; ++k)
		{
			RsslInt32 l;
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			rsslClearRDMDirectoryConsumerStatus(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);

			encRDMMsg.rdmMsgBase.streamId = streamId;

			/* Encode increasing number of source mirroring info elements */
			encRDMMsg.consumerServiceStatusCount = k;
			if (k >= 1)
				encRDMMsg.consumerServiceStatusList = serviceStatusList;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.directoryMsg.consumerStatus;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryConsumerStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);


			ASSERT_TRUE(pDecRDMMsg->consumerServiceStatusCount == k);

			for(l = 0; l < k; ++l)
			{
				RsslRDMConsumerStatusService *pServiceStatus = &pDecRDMMsg->consumerServiceStatusList[l];

				ASSERT_TRUE(pServiceStatus->serviceId == serviceStatusList[l].serviceId);
				ASSERT_TRUE(pServiceStatus->action == serviceStatusList[l].action);

				if (pServiceStatus->action != RSSL_MPEA_DELETE_ENTRY)
				{
					ASSERT_TRUE(pServiceStatus->sourceMirroringMode == serviceStatusList[l].sourceMirroringMode);
				}
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	//printTypedMessageStats(&stats);
}

void directoryCloseMsgTests()
{
	RsslRDMDirectoryClose encRDMMsg, *pDecRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;

	RsslUInt32 j;

	clearTypedMessageStats(&stats);

	for (j = 0; j < testWriteActionsCount; ++j)
	{
		RsslRet ret;
		testWriteAction = testWriteActions[j];

		/*** Encode ***/
		rsslClearRDMDirectoryClose(&encRDMMsg);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE);

		encRDMMsg.rdmMsgBase.streamId = streamId;

		if (testWriteAction != TEST_EACTION_CREATE_COPY)
		{
			writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
					&rsslMsg, &decRDMMsg,
					&stats);
			pDecRDMMsg = &decRDMMsg.directoryMsg.close;
		}
		else
			ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryClose*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CLOSE);

		if (testWriteAction == TEST_EACTION_CREATE_COPY)
			free(pDecRDMMsg);
	}

	//printTypedMessageStats(&stats);
}

void directoryRefreshMsgTests()
{
	RsslRDMDirectoryRefresh encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryRefresh *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };
	RsslUInt16 serviceId = 64;
	RsslUInt32 seqNum = 11152011;
	RsslUInt32 filter = RDM_DIRECTORY_SERVICE_INFO_FILTER
		| RDM_DIRECTORY_SERVICE_STATE_FILTER
		| RDM_DIRECTORY_SERVICE_GROUP_FILTER
		| RDM_DIRECTORY_SERVICE_LOAD_FILTER
		| RDM_DIRECTORY_SERVICE_DATA_FILTER
		| RDM_DIRECTORY_SERVICE_LINK_FILTER;

	RsslRDMService serviceList[3];
	RsslUInt32 serviceListCount = sizeof(serviceList)/sizeof(RsslRDMService);

	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DR_RFF_HAS_SERVICE_ID,
		RDM_DR_RFF_SOLICITED,
		RDM_DR_RFF_HAS_SEQ_NUM,
		RDM_DR_RFF_CLEAR_CACHE
	};
	RsslUInt32 *flagsList, flagsListCount;

	serviceMemoryBuffer.data = serviceMemoryBlock;
	serviceMemoryBuffer.length = sizeof(serviceMemoryBlock);

	for(i = 0; i < serviceListCount; ++i)
	{
		RsslUInt32 linkFlags =
			RDM_SVC_LKF_HAS_TYPE
			| RDM_SVC_LKF_HAS_CODE
			| RDM_SVC_LKF_HAS_TEXT;

		RsslUInt32 groupFlags =
			RDM_SVC_GRF_HAS_MERGED_TO_GROUP
			| RDM_SVC_GRF_HAS_STATUS;

		rsslClearRDMService(&serviceList[i]);

		serviceList[i].serviceId = i;
		serviceList[i].action = RSSL_MPEA_ADD_ENTRY;
		serviceList[i].flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;

		serviceList[i].info.capabilitiesCount = i;
		serviceList[i].info.dictionariesProvidedCount = i;
		serviceList[i].info.dictionariesUsedCount = i;
		serviceList[i].info.qosCount = i;


		serviceList[i].state.flags = RDM_SVC_STF_HAS_ACCEPTING_REQS
			| RDM_SVC_STF_HAS_STATUS;

		initServiceElements(&serviceList[i], &serviceMemoryBuffer, groupFlags, linkFlags);
	}

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	clearTypedMessageStats(&stats);

	for(i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslUInt32 k;
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			/* Don't try to exhaustively test service objects here. Do it elsewhere.
			 * If we did, this test would never finish. */
			for(k = 0; k <= serviceListCount; ++k)
			{
				RsslUInt32 l;

				rsslClearRDMDirectoryRefresh(&encRDMMsg);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);

				encRDMMsg.flags = flagsList[i];
				encRDMMsg.rdmMsgBase.streamId = streamId;

				encRDMMsg.state = state;
				encRDMMsg.filter = filter;
				encRDMMsg.serviceCount = k;
				encRDMMsg.serviceList = serviceList;
				if (flagsList[i] & RDM_DR_RFF_HAS_SERVICE_ID)
					encRDMMsg.serviceId = serviceId;

				if (flagsList[i] & RDM_DR_RFF_HAS_SEQ_NUM)
					encRDMMsg.sequenceNumber = seqNum;

				if (testWriteAction != TEST_EACTION_CREATE_COPY)
				{
					writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
							&rsslMsg, &decRDMMsg,
							&stats);
					pDecRDMMsg = &decRDMMsg.directoryMsg.refresh;
				}
				else
					ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryRefresh*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);

				ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

				ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
				ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
				ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));

				if (flagsList[i] & RDM_DR_RFF_HAS_SERVICE_ID)
					ASSERT_TRUE(pDecRDMMsg->serviceId == serviceId);

				if (flagsList[i] & RDM_DR_RFF_HAS_SEQ_NUM)
					ASSERT_TRUE(pDecRDMMsg->sequenceNumber == seqNum);

				ASSERT_TRUE(pDecRDMMsg->filter == filter);


				ASSERT_TRUE(pDecRDMMsg->serviceCount == k);
				for(l = 0; l < k; ++l)
					testCompareService(&pDecRDMMsg->serviceList[l], &serviceList[l]);

				if (testWriteAction == TEST_EACTION_CREATE_COPY)
					free(pDecRDMMsg);

			}
		}
	}

	//printTypedMessageStats(&stats);

	free(flagsList);
}

void directoryUpdateMsgTests()
{
	RsslRDMDirectoryUpdate encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryUpdate *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };
	RsslUInt16 serviceId = 64;
	RsslUInt32 seqNum = 11152011;
	RsslUInt32 filter = RDM_DIRECTORY_SERVICE_INFO_FILTER
		| RDM_DIRECTORY_SERVICE_STATE_FILTER
		| RDM_DIRECTORY_SERVICE_GROUP_FILTER
		| RDM_DIRECTORY_SERVICE_LOAD_FILTER
		| RDM_DIRECTORY_SERVICE_DATA_FILTER
		| RDM_DIRECTORY_SERVICE_LINK_FILTER;

	RsslRDMService serviceList[3];
	RsslUInt32 serviceListCount = sizeof(serviceList)/sizeof(RsslRDMService);

	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DR_UPF_HAS_SERVICE_ID,
		RDM_DR_UPF_HAS_FILTER,
		RDM_DR_UPF_HAS_SEQ_NUM
	};
	RsslUInt32 *flagsList, flagsListCount;

	serviceMemoryBuffer.data = serviceMemoryBlock;
	serviceMemoryBuffer.length = sizeof(serviceMemoryBlock);

	for(i = 0; i < serviceListCount; ++i)
	{
		RsslUInt32 linkFlags =
			RDM_SVC_LKF_HAS_TYPE
			| RDM_SVC_LKF_HAS_CODE
			| RDM_SVC_LKF_HAS_TEXT;

		RsslUInt32 groupFlags =
			RDM_SVC_GRF_HAS_MERGED_TO_GROUP
			| RDM_SVC_GRF_HAS_STATUS;


		rsslClearRDMService(&serviceList[i]);

		serviceList[i].serviceId = i;
		serviceList[i].action = RSSL_MPEA_UPDATE_ENTRY;
		serviceList[i].flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE;

		serviceList[i].info.capabilitiesCount = i;
		serviceList[i].info.dictionariesProvidedCount = i;
		serviceList[i].info.dictionariesUsedCount = i;
		serviceList[i].info.qosCount = i;


		serviceList[i].state.flags = RDM_SVC_STF_HAS_ACCEPTING_REQS
			| RDM_SVC_STF_HAS_STATUS;

		initServiceElements(&serviceList[i], &serviceMemoryBuffer, groupFlags, linkFlags);
	}

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	clearTypedMessageStats(&stats);

	for(i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			RsslUInt32 k;
			RsslRet ret;

			testWriteAction = testWriteActions[j];

			/* Don't try to exhaustively test service objects here. Do it elsewhere.
			 * If we did, this test would never finish. */
			for(k = 0; k <= serviceListCount; ++k)
			{
				RsslUInt32 l;

				rsslClearRDMDirectoryUpdate(&encRDMMsg);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
				ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

				encRDMMsg.flags = flagsList[i];
				encRDMMsg.rdmMsgBase.streamId = streamId;

				if (flagsList[i] & RDM_DR_UPF_HAS_FILTER)
					encRDMMsg.filter = filter;
				encRDMMsg.serviceCount = k;
				encRDMMsg.serviceList = serviceList;
				if (flagsList[i] & RDM_DR_UPF_HAS_SERVICE_ID)
					encRDMMsg.serviceId = serviceId;

				if (flagsList[i] & RDM_DR_UPF_HAS_SEQ_NUM)
					encRDMMsg.sequenceNumber = seqNum;

				if (testWriteAction != TEST_EACTION_CREATE_COPY)
				{
					writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
							&rsslMsg, &decRDMMsg,
							&stats);
					pDecRDMMsg = &decRDMMsg.directoryMsg.update;
				}
				else
					ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryUpdate*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret))!= NULL);

				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
				ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

				ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

				if (flagsList[i] & RDM_DR_UPF_HAS_SERVICE_ID)
					ASSERT_TRUE(pDecRDMMsg->serviceId == serviceId); 
				if (flagsList[i] & RDM_DR_UPF_HAS_FILTER)
					ASSERT_TRUE(pDecRDMMsg->filter == filter);
				if (flagsList[i] & RDM_DR_UPF_HAS_SEQ_NUM)
					ASSERT_TRUE(pDecRDMMsg->sequenceNumber == seqNum); 


				ASSERT_TRUE(pDecRDMMsg->serviceCount == k);
				for(l = 0; l < k; ++l)
					testCompareService(&pDecRDMMsg->serviceList[l], &serviceList[l]);

				if (testWriteAction == TEST_EACTION_CREATE_COPY)
					free(pDecRDMMsg);

			}
		}
	}

	//printTypedMessageStats(&stats);

	free(flagsList);
}

void testServiceListInMsg(RsslRDMService *pServiceList, RsslInt32 serviceCount, RsslBuffer *pMemoryBuffer, RsslUInt32 groupFlags, RsslUInt32 linkFlags)
{
	RsslInt32 i;
	RsslRDMDirectoryUpdate encRDMMsg;
	RsslInt32 streamId = -5;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryUpdate *pDecRDMMsg;
	RsslRet ret;

	serviceMemoryBuffer.data = serviceMemoryBlock;
	serviceMemoryBuffer.length = sizeof(serviceMemoryBlock);


	for(i = 0; i < serviceCount; ++i)
		initServiceElements(&pServiceList[i], pMemoryBuffer, groupFlags, linkFlags);

	rsslClearRDMDirectoryUpdate(&encRDMMsg);
	ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

	encRDMMsg.flags = RDM_DR_UPF_NONE;
	encRDMMsg.rdmMsgBase.streamId = streamId;
	encRDMMsg.serviceCount = serviceCount;
	encRDMMsg.serviceList = pServiceList;

	if (testWriteAction != TEST_EACTION_CREATE_COPY)
	{
		writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
				&rsslMsg, &decRDMMsg,
				&stats);
		pDecRDMMsg = &decRDMMsg.directoryMsg.update;
	}
	else
		ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryUpdate*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

	ASSERT_TRUE(pDecRDMMsg->flags == RDM_DR_UPF_NONE);

	ASSERT_TRUE(pDecRDMMsg->serviceCount == serviceCount);
	for(i = 0; i < serviceCount; ++i)
		testCompareService(&pDecRDMMsg->serviceList[i], &pServiceList[i]);

	if (testWriteAction == TEST_EACTION_CREATE_COPY)
		free(pDecRDMMsg);
}

void directoryServiceListTests()
{
	/* Parameters to test with */
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };
	RsslUInt16 serviceId = 64;
	RsslUInt32 filter = RDM_DIRECTORY_SERVICE_INFO_FILTER
		| RDM_DIRECTORY_SERVICE_STATE_FILTER
		| RDM_DIRECTORY_SERVICE_GROUP_FILTER
		| RDM_DIRECTORY_SERVICE_LOAD_FILTER
		| RDM_DIRECTORY_SERVICE_DATA_FILTER
		| RDM_DIRECTORY_SERVICE_LINK_FILTER;

	RsslRDMService serviceList[8];
	RsslInt32 serviceListCount = sizeof(serviceList)/sizeof(RsslRDMService);
	RsslUInt32 j;

	clearTypedMessageStats(&stats);
	

	for (j = 0; j < testWriteActionsCount; ++j)
	{
		RsslInt32 k;

		testWriteAction = testWriteActions[j];

		for(k = 0; k <= serviceListCount; ++k)
		{
			RsslInt32 l;

			RsslUInt32 serviceFilterFlagsIter;

			/* Test each filter by itself (it would take too long to exhaustively test
			 * every combination of service category and each category's members */

			/* Info */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceInfoFlagsListCount; ++serviceFilterFlagsIter)
			{
				for(l = 0; l < k; ++l)
				{
					rsslClearRDMService(&serviceList[l]);
					serviceList[l].serviceId = l;
					serviceList[l].action = RSSL_MPEA_ADD_ENTRY;
					serviceList[l].flags = RDM_SVCF_HAS_INFO;
					serviceList[l].info.flags = serviceInfoFlagsList[serviceFilterFlagsIter];
					serviceList[l].info.action = l % 2 ? RSSL_FTEA_CLEAR_ENTRY : RSSL_FTEA_SET_ENTRY;

					/* Make sure each count is different (serviceListCount should be large enough to accomodate) */
					serviceList[l].info.dictionariesProvidedCount = l % k;
					serviceList[l].info.dictionariesUsedCount = (l+1) % k;
					serviceList[l].info.qosCount = (l+2) % k;
				}

				testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, 0, 0);
			}

			/* State */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceStateFlagsListCount; ++serviceFilterFlagsIter)
			{
				for(l = 0; l < k; ++l)
				{
					rsslClearRDMService(&serviceList[l]);
					serviceList[l].serviceId = l;
					serviceList[l].action = RSSL_MPEA_UPDATE_ENTRY;
					serviceList[l].flags = RDM_SVCF_HAS_STATE;
					serviceList[l].state.flags = serviceStateFlagsList[serviceFilterFlagsIter];
					serviceList[l].state.action = l % 2 ? RSSL_FTEA_CLEAR_ENTRY : RSSL_FTEA_SET_ENTRY;
				}

				testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, 0, 0);
			}

			/* Group */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceGroupFlagsListCount; ++serviceFilterFlagsIter)
			{
				RsslInt32 groupStateCount;

				for(groupStateCount = 1; groupStateCount <= 3; ++groupStateCount)
				{
					for(l = 0; l < k; ++l)
					{
						rsslClearRDMService(&serviceList[l]);
						serviceList[l].serviceId = l;
						serviceList[l].action = RSSL_MPEA_UPDATE_ENTRY;
						serviceList[l].flags = RDM_SVCF_NONE;
						serviceList[l].groupStateCount = (groupStateCount + l) % (groupStateCount + 1);
					}

					testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, serviceGroupFlagsList[serviceFilterFlagsIter], 0);
				}
			}

			/* Load */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceLoadFlagsListCount; ++serviceFilterFlagsIter)
			{
				for(l = 0; l < k; ++l)
				{
					rsslClearRDMService(&serviceList[l]);
					serviceList[l].serviceId = l;
					serviceList[l].action = RSSL_MPEA_ADD_ENTRY;
					serviceList[l].flags = RDM_SVCF_HAS_LOAD;
					serviceList[l].load.flags = serviceLoadFlagsList[serviceFilterFlagsIter];
					serviceList[l].load.action = l % 2 ? RSSL_FTEA_CLEAR_ENTRY : RSSL_FTEA_SET_ENTRY;
				}

				testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, 0, 0);
			}

			/* Data */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceDataFlagsListCount; ++serviceFilterFlagsIter)
			{
				RsslUInt32 dataFlags = serviceDataFlagsList[serviceFilterFlagsIter];

				for(l = 0; l < k; ++l)
				{
					serviceList[l].serviceId = l;
					serviceList[l].action = RSSL_MPEA_ADD_ENTRY;
					serviceList[l].flags = RDM_SVCF_HAS_DATA;
					serviceList[l].data.flags = serviceDataFlagsList[serviceFilterFlagsIter];
					serviceList[l].data.action = l % 2 ? RSSL_FTEA_CLEAR_ENTRY : RSSL_FTEA_SET_ENTRY;
				}

				testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, serviceGroupFlagsList[serviceFilterFlagsIter], 0);

			}

			/* Link */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceLinkFlagsListCount; ++serviceFilterFlagsIter)
			{
				RsslInt32 linkCount;

				for(linkCount = 1; linkCount <= 3; ++linkCount)
				{
					for(l = 0; l < k; ++l)
					{
						rsslClearRDMService(&serviceList[l]);
						serviceList[l].serviceId = l;
						serviceList[l].action = RSSL_MPEA_UPDATE_ENTRY;
						serviceList[l].flags = RDM_SVCF_HAS_LINK;
						serviceList[l].linkInfo.action = l % 2 ? RSSL_FTEA_SET_ENTRY : RSSL_FTEA_CLEAR_ENTRY;
						serviceList[l].linkInfo.linkCount = (linkCount + l) % (linkCount + 1);
					}

					testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, 0, serviceLinkFlagsList[serviceFilterFlagsIter]);
				}
			}

			/* Now test the different filter combinations to make sure they all work together. */
			for (serviceFilterFlagsIter = 0; serviceFilterFlagsIter < serviceFlagsListCount; ++serviceFilterFlagsIter)
			{
				RsslInt32 groupStateCount;
				for(groupStateCount = 0; groupStateCount < 3; ++groupStateCount)
				{
					RsslInt32 linkCount;
					for(linkCount = 0; linkCount < 3; ++linkCount)
					{
						for(l = 0; l < k; ++l)
						{
							RsslUInt32 groupFlags = RDM_SVC_GRF_HAS_MERGED_TO_GROUP
								| RDM_SVC_GRF_HAS_STATUS;

							RsslUInt32 linkFlags = RDM_SVC_LKF_HAS_TYPE
								| RDM_SVC_LKF_HAS_CODE
								| RDM_SVC_LKF_HAS_TEXT;
							rsslClearRDMService(&serviceList[l]);

							serviceList[l].serviceId = l;
							serviceList[l].action = RSSL_MPEA_ADD_ENTRY;
							serviceList[l].flags = serviceFlagsList[serviceFilterFlagsIter];

							if (serviceFlagsList[serviceFilterFlagsIter] & RDM_SVCF_HAS_INFO)
							{
								/* All flags. */
								serviceList[l].info.flags = RDM_SVC_IFF_HAS_VENDOR
									| RDM_SVC_IFF_HAS_IS_SOURCE
									| RDM_SVC_IFF_HAS_DICTS_PROVIDED
									| RDM_SVC_IFF_HAS_DICTS_USED
									| RDM_SVC_IFF_HAS_QOS
									| RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE
									| RDM_SVC_IFF_HAS_ITEM_LIST
									| RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS
									| RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;

								/* Make sure each count is different (serviceListCount should be large enough to accomodate) */
								serviceList[l].info.dictionariesProvidedCount = l % k;
								serviceList[l].info.dictionariesUsedCount = (l+1) % k;
								serviceList[l].info.qosCount = (l+2) % k;
							}

							if (serviceFlagsList[serviceFilterFlagsIter] & RDM_SVCF_HAS_STATE)
							{
								rsslClearRDMService(&serviceList[l]);
								/* All flags. */
								serviceList[l].state.flags = RDM_SVC_STF_HAS_ACCEPTING_REQS
									| RDM_SVC_STF_HAS_STATUS;
							}

							serviceList[l].groupStateCount = groupStateCount;

							if (serviceFlagsList[serviceFilterFlagsIter] & RDM_SVCF_HAS_LOAD)
							{
								/* All flags. */
								serviceList[l].load.flags = RDM_SVC_LDF_HAS_OPEN_LIMIT
									| RDM_SVC_LDF_HAS_OPEN_WINDOW
									| RDM_SVC_LDF_HAS_LOAD_FACTOR;
							}

							if (serviceFlagsList[serviceFilterFlagsIter] & RDM_SVCF_HAS_DATA)
							{
								/* All flags. */
								serviceList[l].data.flags = RDM_SVC_DTF_HAS_DATA;
							}

							serviceList[l].linkInfo.linkCount = linkCount;

							testServiceListInMsg(serviceList, k, &serviceMemoryBuffer, groupFlags, linkFlags);
						}
					}
				}
			}

		}
	}

	//printTypedMessageStats(&stats);
}

void directoryStatusMsgTests()
{
	RsslRDMDirectoryStatus encRDMMsg;

	RsslRDMMsg decRDMMsg;
	RsslMsg rsslMsg;
	RsslRDMDirectoryStatus *pDecRDMMsg;

	/* Parameters to test with */
	RsslInt32 streamId = -5;
	RsslState state = { RSSL_STREAM_OPEN, RSSL_DATA_SUSPECT, RSSL_SC_FAILOVER_COMPLETED, rssl_init_buffer_from_string(const_cast<char*>("In Soviet Russia, state tests you!")) };
	RsslUInt16 serviceId = 64;
	RsslUInt32 filter = RDM_DIRECTORY_SERVICE_INFO_FILTER
		| RDM_DIRECTORY_SERVICE_STATE_FILTER
		| RDM_DIRECTORY_SERVICE_GROUP_FILTER
		| RDM_DIRECTORY_SERVICE_LOAD_FILTER
		| RDM_DIRECTORY_SERVICE_DATA_FILTER
		| RDM_DIRECTORY_SERVICE_LINK_FILTER;

	RsslUInt32 i;

	RsslUInt32 flagsBase[] =
	{
		RDM_DR_STF_NONE,
		RDM_DR_STF_HAS_FILTER,
		RDM_DR_STF_HAS_SERVICE_ID,
		RDM_DR_STF_HAS_STATE,
		RDM_DR_STF_CLEAR_CACHE
	};
	RsslUInt32 *flagsList, flagsListCount;

	flagsListCount = _createFlagCombinations(&flagsList, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), RSSL_FALSE);

	clearTypedMessageStats(&stats);

	for(i = 0; i < flagsListCount; ++i)
	{
		RsslUInt32 j;
		RsslRet ret;

		for (j = 0; j < testWriteActionsCount; ++j)
		{
			testWriteAction = testWriteActions[j];


			rsslClearRDMDirectoryStatus(&encRDMMsg);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(encRDMMsg.rdmMsgBase.rdmMsgType == RDM_DR_MT_STATUS);

			encRDMMsg.flags = flagsList[i];
			encRDMMsg.rdmMsgBase.streamId = streamId;

			if (flagsList[i] & RDM_DR_STF_HAS_FILTER)
				encRDMMsg.filter = filter;
			if (flagsList[i] & RDM_DR_STF_HAS_SERVICE_ID)
				encRDMMsg.serviceId = serviceId;
			if (flagsList[i] & RDM_DR_STF_HAS_STATE)
				encRDMMsg.state = state;

			if (testWriteAction != TEST_EACTION_CREATE_COPY)
			{
				writeRDMMsg((RsslRDMMsg*)&encRDMMsg, testWriteAction,
						&rsslMsg, &decRDMMsg,
						&stats);
				pDecRDMMsg = &decRDMMsg.directoryMsg.status;
			}
			else
				ASSERT_TRUE((pDecRDMMsg = (RsslRDMDirectoryStatus*)rsslCreateRDMMsgCopy((RsslRDMMsg*)&encRDMMsg, 1, &ret)) != NULL);

			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.streamId == streamId);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
			ASSERT_TRUE(pDecRDMMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_STATUS);

			ASSERT_TRUE(pDecRDMMsg->flags == flagsList[i]);

			if (flagsList[i] & RDM_DR_STF_HAS_SERVICE_ID)
				ASSERT_TRUE(pDecRDMMsg->serviceId == serviceId); 
			if (flagsList[i] & RDM_DR_STF_HAS_FILTER)
				ASSERT_TRUE(pDecRDMMsg->filter == filter);
			if (flagsList[i] & RDM_DR_STF_HAS_STATE)
			{
				ASSERT_TRUE(pDecRDMMsg->state.streamState == state.streamState);
				ASSERT_TRUE(pDecRDMMsg->state.dataState == state.dataState);
				ASSERT_TRUE(pDecRDMMsg->state.code == state.code);
				ASSERT_TRUE(rsslBufferIsEqual(&pDecRDMMsg->state.text, &state.text));
			}

			if (testWriteAction == TEST_EACTION_CREATE_COPY)
				free(pDecRDMMsg);
		}
	}

	//printTypedMessageStats(&stats);

	free(flagsList);
}
