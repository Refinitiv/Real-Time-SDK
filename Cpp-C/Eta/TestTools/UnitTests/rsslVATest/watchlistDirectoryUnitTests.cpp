/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

void watchlistDirectoryTest_NoRdmCallbacks(WtfCallbackAction action, RsslConnectionTypes connectionType);
void watchlistDirectoryTest_OneService(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_ServiceUpdate(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_ServiceUpdateLinkFilterOnly(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_ServiceUpdatePartialFilter(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_RsslMsgClose(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_MultipleRequests(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_DirectoryStatus(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTest_OneService(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTestDataFilterOnly_OneService(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTest_Links(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTestLinkFilterOnly_Links(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTest_Groups(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTestGroupFilterOnly_Groups(RsslConnectionTypes connectionType);
void watchlistDirectoryDataTestDataFilterOnly_Data(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_DeleteService(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_BigDirectory(RsslConnectionTypes connectionType);
void watchlistDirectoryTest_DuplicateServiceName(RsslConnectionTypes connectionType);

class WatchlistDirectoryUnitTest : public ::testing::TestWithParam<RsslConnectionTypes> {
public:

	static void SetUpTestCase()
	{
		wtfInit(NULL);
	}

	virtual void SetUp()
	{
		wtfBindServer(GetParam());
	}

	static void TearDownTestCase()
	{
		wtfCleanup();
	}

	virtual void TearDown()
	{
		wtfCloseServer();
	}
};

TEST_P(WatchlistDirectoryUnitTest, OneService)
{
	watchlistDirectoryTest_OneService(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, ServiceUpdate)
{
	watchlistDirectoryTest_ServiceUpdate(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, ServiceUpdatePartialFilter)
{
	watchlistDirectoryTest_ServiceUpdatePartialFilter(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, ServiceUpdateLinkFilterOnly)
{
	watchlistDirectoryTest_ServiceUpdateLinkFilterOnly(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, RsslMsgClose)
{
	watchlistDirectoryTest_RsslMsgClose(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, MultipleRequests)
{
	watchlistDirectoryTest_MultipleRequests(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, NoRdmCallbacks_WTF_CB_NONE)
{
	watchlistDirectoryTest_NoRdmCallbacks(WTF_CB_NONE, GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, NoRdmCallbacks_WTF_CB_RAISE_TO_DEFAULT_CB)
{
	watchlistDirectoryTest_NoRdmCallbacks(WTF_CB_RAISE_TO_DEFAULT_CB, GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DirectoryStatus)
{
	watchlistDirectoryTest_DirectoryStatus(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTest_OneService)
{
	watchlistDirectoryDataTest_OneService(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataFilterOnly_OneService)
{
	watchlistDirectoryDataTestDataFilterOnly_OneService(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTest_Links)
{
	watchlistDirectoryDataTest_Links(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTestLinkFilterOnly_Links)
{
	watchlistDirectoryDataTestLinkFilterOnly_Links(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTest_Groups)
{
	watchlistDirectoryDataTest_Groups(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTestGroupFilterOnly_Groups)
{
	watchlistDirectoryDataTestGroupFilterOnly_Groups(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DataTestDataFilterOnly_Data)
{
	watchlistDirectoryDataTestDataFilterOnly_Data(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DeleteService)
{
	watchlistDirectoryTest_DeleteService(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, BigDirectory)
{
	watchlistDirectoryTest_BigDirectory(GetParam());
}

TEST_P(WatchlistDirectoryUnitTest, DuplicateServiceName)
{
	watchlistDirectoryTest_DuplicateServiceName(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
	TestingWatchlistDirectoryUnitTests,
	WatchlistDirectoryUnitTest,
	::testing::Values(
		RSSL_CONN_TYPE_SOCKET, RSSL_CONN_TYPE_WEBSOCKET
	));

void watchlistDirectoryTest_OneService(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService service1;
	RsslRDMDirectoryClose	directoryClose;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;
	wtfSetupConnection(&connOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= RDM_DIRECTORY_SERVICE_LOAD_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests directory (non-streaming). */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 3);
	directoryRequest.flags &= ~RDM_DR_RQF_STREAMING;
	directoryRequest.filter |= RDM_DIRECTORY_SERVICE_LOAD_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags == (RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE | RDM_SVCF_HAS_LOAD));
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.flags == (RDM_SVC_LDF_HAS_OPEN_LIMIT | RDM_SVC_LDF_HAS_OPEN_WINDOW | RDM_SVC_LDF_HAS_LOAD_FACTOR));
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.openLimit == 0xffffffffffffffffULL);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.openWindow == 0xffffffffffffffffULL);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.loadFactor == 65535);


	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 3);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_NON_STREAMING);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags == (RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_STATE | RDM_SVCF_HAS_LOAD));
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.flags == (RDM_SVC_LDF_HAS_OPEN_LIMIT | RDM_SVC_LDF_HAS_OPEN_WINDOW | RDM_SVC_LDF_HAS_LOAD_FACTOR));
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.openLimit == 0xffffffffffffffffULL);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.openWindow == 0xffffffffffffffffULL);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.loadFactor == 65535);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer re-requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer closes directory request. */
	rsslClearRDMDirectoryClose(&directoryClose);
	directoryClose.rdmMsgBase.streamId = 2;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryClose;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.flags = RDM_SVCF_HAS_STATE; 
	service1.state.serviceState = 0;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer received no messages (since request is closed). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryTest_ServiceUpdate(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service1;
	RsslReactorSubmitMsgOptions opts;
	RsslBuffer newLinkName = { 4, const_cast<char*>("fish") };

	RsslUInt newCapabilitiesList[] = { RSSL_DMT_HEADLINE, RSSL_DMT_REPLAYHEADLINE };
	RsslUInt32 newCapabilitiesCount = 2;

	RsslRDMServiceLink newLink, *pLink;
	WtfSetupConnectionOpts connOpts;

	rsslClearRDMServiceLink(&newLink);
	newLink.flags = RDM_SVC_LKF_NONE;
	newLink.name = newLinkName;
	newLink.linkState = 1;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDictionaryUsedAndProvided = RSSL_TRUE;
	wtfSetupConnection(&connOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.dictionariesProvidedCount = 2);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.flags & RDM_SVC_IFF_HAS_DICTS_USED);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.dictionariesUsedCount = 2);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with filter changes. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	/* Change info filter's capabilities. */
	service1.flags |= RDM_SVCF_HAS_INFO;
	service1.info.serviceName = service1Name;
	service1.info.capabilitiesList = newCapabilitiesList;
	service1.info.capabilitiesCount = newCapabilitiesCount;

	/* Send the entire dictionary provided and used with directory update */
	service1.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
	service1.info.dictionariesProvidedCount = service1DictionariesProvidedCount;
	service1.info.dictionariesProvidedList = service1DictionariesProvidedList;

	service1.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
	service1.info.dictionariesUsedCount = service1DictionariesUsedCount;
	service1.info.dictionariesUsedList = service1DictionariesUsedList;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE; 
	service1.state.serviceState = 0;

	/* Update the load filter. */
	service1.flags |= RDM_SVCF_HAS_LOAD;
	service1.load.flags = RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service1.load.openWindow = 55555;

	/* Add a link. */
	service1.flags |= RDM_SVCF_HAS_LINK;
	service1.linkInfo.linkCount = 1;
	service1.linkInfo.linkList = &newLink;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));

	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);

	/* Check info update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesCount == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[0]
			== newCapabilitiesList[0]);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[1]
			== newCapabilitiesList[1]); 
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.dictionariesProvidedCount = 4);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.flags& RDM_SVC_IFF_HAS_DICTS_USED);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.dictionariesUsedCount = 4);

	/* Check state update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.action == RSSL_FTEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.serviceState == 0);

	/* Check load filter update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_LOAD);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].load.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].load.openWindow == 55555);

	/* Check link update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].linkInfo.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].linkInfo.linkCount == 1);

	pLink = &pDirectoryUpdate->serviceList[0].linkInfo.linkList[0];
	ASSERT_TRUE(pLink->action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pLink->flags == RDM_SVC_LKF_NONE);
	ASSERT_TRUE(rsslBufferIsEqual(&pLink->name, &newLinkName));
	ASSERT_TRUE(pLink->linkState == 1);

	/* Consumer requests directory again. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);

	/* Check info refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesCount == 2);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesList[0]
			== newCapabilitiesList[0]);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesList[1]
			== newCapabilitiesList[1]);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.flags& RDM_SVC_IFF_HAS_DICTS_PROVIDED);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.dictionariesProvidedCount = 4);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.flags& RDM_SVC_IFF_HAS_DICTS_USED);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.dictionariesUsedCount = 4);

	/* Check state refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.serviceState == 0);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.serviceState == 0);

	/* Check load filter refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_LOAD);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].load.openWindow == 55555);

	/* Check link refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].linkInfo.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].linkInfo.linkCount == 1);

	pLink = &pDirectoryRefresh->serviceList[0].linkInfo.linkList[0];
	ASSERT_TRUE(pLink->action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pLink->flags == RDM_SVC_LKF_NONE);
	ASSERT_TRUE(rsslBufferIsEqual(&pLink->name, &newLinkName));
	ASSERT_TRUE(pLink->linkState == 1);

	free(service1DictionariesProvidedList);
	service1DictionariesProvidedList = 0;

	free(service1DictionariesUsedList);
	service1DictionariesUsedList = 0;

	wtfFinishTest();
}

void watchlistDirectoryTest_ServiceUpdatePartialFilter(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service1;
	RsslReactorSubmitMsgOptions opts;
	RsslBuffer newLinkName = { 4, const_cast<char*>("fish") };

	RsslUInt newCapabilitiesList[] = { RSSL_DMT_HEADLINE, RSSL_DMT_REPLAYHEADLINE };
	RsslUInt32 newCapabilitiesCount = 2;

	RsslRDMServiceLink newLink;

	RsslRDMServiceGroupState groupStateList[2];

	rsslClearRDMServiceLink(&newLink);
	newLink.flags = RDM_SVC_LKF_NONE;
	newLink.name = newLinkName;
	newLink.linkState = 1;

	char someRandomString[] = "some random String";
	char one[2] = { 0, 1 };
	char zero[2] = { 0, 0 };
	RsslBuffer group1 = { 2, (char *)one };
	RsslBuffer group0 = { 2, (char *)zero };
	RsslBuffer someRandomText = { sizeof(someRandomString), const_cast<char*>(someRandomString) };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2); // default filter is INFO | STATE | GROUP
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with filter changes. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	/* Change info filter's capabilities. */
	service1.flags |= RDM_SVCF_HAS_INFO;
	service1.info.serviceName = service1Name;
	service1.info.capabilitiesList = newCapabilitiesList;
	service1.info.capabilitiesCount = newCapabilitiesCount;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE;
	service1.state.serviceState = 0;

	/* Update the load filter. */
	service1.flags |= RDM_SVCF_HAS_LOAD;
	service1.load.flags = RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service1.load.openWindow = 55555;

	/* Add a link. */
	service1.flags |= RDM_SVCF_HAS_LINK;
	service1.linkInfo.linkCount = 1;
	service1.linkInfo.linkList = &newLink;

	/* Add a group. */
	service1.groupStateCount = 2;
	service1.groupStateList = groupStateList;

	rsslClearRDMServiceGroupState(&groupStateList[0]);
	groupStateList[0].flags = RDM_SVC_GRF_HAS_STATUS;
	groupStateList[0].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[0].group = group1;
	groupStateList[0].status.dataState = RSSL_DATA_SUSPECT;
	groupStateList[0].status.streamState = RSSL_STREAM_OPEN;
	groupStateList[0].status.code = RSSL_SC_NO_RESOURCES;
	groupStateList[0].status.text = someRandomText;

	rsslClearRDMServiceGroupState(&groupStateList[1]);
	groupStateList[1].flags = RDM_SVC_GRF_HAS_MERGED_TO_GROUP;
	groupStateList[1].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[1].group = group1;
	groupStateList[1].mergedToGroup = group0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));

	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);

	// check filter for INFO | STATE | GROUP
	ASSERT_TRUE(pDirectoryUpdate->filter == (RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER));

	/* Check info update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesCount == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[0]
		== newCapabilitiesList[0]);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[1]
		== newCapabilitiesList[1]);

	/* Check state update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.action == RSSL_FTEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.serviceState == 0);

	/* Check group update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateCount == 2);

	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[0].flags == RDM_SVC_GRF_HAS_STATUS);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[0].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pDirectoryUpdate->serviceList[0].groupStateList[0].group, &group1));
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[0].status.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[0].status.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[0].status.code == RSSL_SC_NO_RESOURCES);
	ASSERT_TRUE(memcmp(pDirectoryUpdate->serviceList[0].groupStateList[0].status.text.data, someRandomText.data, someRandomText.length -1) == 0);

	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[1].flags == RDM_SVC_GRF_HAS_MERGED_TO_GROUP);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].groupStateList[1].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pDirectoryUpdate->serviceList[0].groupStateList[1].group, &group1));
	ASSERT_TRUE(rsslBufferIsEqual(&pDirectoryUpdate->serviceList[0].groupStateList[1].mergedToGroup, &group0));

	/* Consumer requests directory again. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2); // default filter is INFO | STATE | GROUP
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);

	// check filter for INFO | STATE | GROUP
	ASSERT_TRUE(pDirectoryRefresh->filter == (RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER));

	/* Check info refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesCount == 2);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesList[0]
		== newCapabilitiesList[0]);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].info.capabilitiesList[1]
		== newCapabilitiesList[1]);

	/* Check state refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.serviceState == 0);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].state.serviceState == 0);

	/* Check group refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].groupStateCount == 0);

	wtfFinishTest();
}

void watchlistDirectoryTest_ServiceUpdateLinkFilterOnly(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service1;
	RsslReactorSubmitMsgOptions opts;
	RsslBuffer newLinkName = { 4, const_cast<char*>("fish") };

	RsslUInt newCapabilitiesList[] = { RSSL_DMT_HEADLINE, RSSL_DMT_REPLAYHEADLINE };
	RsslUInt32 newCapabilitiesCount = 2;

	RsslRDMServiceLink newLink, *pLink;

	rsslClearRDMServiceLink(&newLink);
	newLink.flags = RDM_SVC_LKF_NONE;
	newLink.name = newLinkName;
	newLink.linkState = 1;

	char someRandomString[] = "some random String";
	char one[2] = { 0, 1 };
	char zero[2] = { 0, 0 };
	RsslBuffer group1 = { 2, (char *)one };
	RsslBuffer group0 = { 2, (char *)zero };
	RsslBuffer someRandomText = { sizeof(someRandomString), const_cast<char*>(someRandomString) };

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with filter changes. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;
	service1.action = RSSL_MPEA_UPDATE_ENTRY;

	/* Change info filter's capabilities. */
	service1.flags |= RDM_SVCF_HAS_INFO;
	service1.info.serviceName = service1Name;
	service1.info.capabilitiesList = newCapabilitiesList;
	service1.info.capabilitiesCount = newCapabilitiesCount;

	/* Change state filter's service state. */
	service1.flags |= RDM_SVCF_HAS_STATE;
	service1.state.serviceState = 0;

	/* Update the load filter. */
	service1.flags |= RDM_SVCF_HAS_LOAD;
	service1.load.flags = RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service1.load.openWindow = 55555;

	/* Add a link. */
	service1.flags |= RDM_SVCF_HAS_LINK;
	service1.linkInfo.linkCount = 1;
	service1.linkInfo.linkList = &newLink;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));

	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);

	/* Check info update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesCount == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[0]
		== newCapabilitiesList[0]);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].info.capabilitiesList[1]
		== newCapabilitiesList[1]);

	/* Check state update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_STATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.action == RSSL_FTEA_UPDATE_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].state.serviceState == 0);

	/* Check load filter update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_LOAD);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].load.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].load.openWindow == 55555);

	/* Check link update. */
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].linkInfo.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].linkInfo.linkCount == 1);

	pLink = &pDirectoryUpdate->serviceList[0].linkInfo.linkList[0];
	ASSERT_TRUE(pLink->action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pLink->flags == RDM_SVC_LKF_NONE);
	ASSERT_TRUE(rsslBufferIsEqual(&pLink->name, &newLinkName));
	ASSERT_TRUE(pLink->linkState == 1);

	/* Consumer requests directory again. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_LINK_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);

	// check filter for only LINK
	ASSERT_TRUE(pDirectoryRefresh->filter == RDM_DIRECTORY_SERVICE_LINK_FILTER);

	/* Check link refresh. */
	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].linkInfo.action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(pDirectoryRefresh->serviceList[0].linkInfo.linkCount == 1);

	pLink = &pDirectoryRefresh->serviceList[0].linkInfo.linkList[0];
	ASSERT_TRUE(pLink->action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pLink->flags == RDM_SVC_LKF_NONE);
	ASSERT_TRUE(rsslBufferIsEqual(&pLink->name, &newLinkName));
	ASSERT_TRUE(pLink->linkState == 1);

	wtfFinishTest();
}

void watchlistDirectoryTest_RsslMsgClose(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslRDMService service1;
	RsslCloseMsg closeMsg;
	RsslReactorSubmitMsgOptions opts;

	/* Tests that watchlist handles directory close
	 * via RsslMsg. */

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer closes directory request using RsslMsg. */
	rsslClearCloseMsg(&closeMsg);
	closeMsg.msgBase.streamId = 2;
	closeMsg.msgBase.domainType = RSSL_DMT_SOURCE;
	closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	opts.pRsslMsg = (RsslMsg*)&closeMsg;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.flags = RDM_SVCF_HAS_STATE; 
	service1.state.serviceState = 0;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer received no messages (since request is closed). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryTest_MultipleRequests(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	WtfSetupConnectionOpts csOpts;
	RsslReactorSubmitMsgOptions opts;

	RsslRDMDirectoryRefresh directoryRefresh, *pDirectoryRefresh;
	RsslRDMDirectoryUpdate directoryUpdate, *pDirectoryUpdate;

	RsslRDMService	services[3];
	RsslUInt32		serviceCount = 2;
	RsslBuffer		serviceNames[3] = { { 9, const_cast<char*>("DUCK_FEED") }, { 11, const_cast<char*>("WABBIT_FEED") }, {10, const_cast<char*>("WRONG_NAME")} };
	RsslUInt16		serviceIds[3] = { 2 , 3 , 100};

	RsslBool		stream2ReceivedMsg = RSSL_FALSE;
	RsslBool		stream3ReceivedMsg = RSSL_FALSE;
	RsslBool		stream4ReceivedMsg = RSSL_FALSE;
	RsslBool		stream5ReceivedMsg = RSSL_FALSE;
	RsslBool		stream6ReceivedMsg = RSSL_FALSE;
	RsslBool		stream7ReceivedMsg = RSSL_FALSE;
	RsslBool		stream8ReceivedMsg = RSSL_FALSE;

	ASSERT_TRUE(wtfStartTest());

	/* Make directory requests for all services, and by specific name/ID. */

	wtfClearSetupConnectionOpts(&csOpts);
	csOpts.provideDefaultDirectory = RSSL_FALSE;
	wtfSetupConnection(&csOpts, connectionType);

	/* Provider sends directory refresh. */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = services;
	directoryRefresh.serviceCount = serviceCount;

	/* Start with some default service info. */
	wtfSetService1Info(&services[0]);
	wtfSetService1Info(&services[1]);

	/* Change service names. */
	services[0].info.serviceName = serviceNames[0];
	services[1].info.serviceName = serviceNames[1];

	/* Change service ID's. */
	services[0].serviceId = serviceIds[0];
	services[1].serviceId = serviceIds[1];

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Watchlist process directory(but no messages go to consumer). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer requests full directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by name. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 3);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.pServiceName = &serviceNames[0];
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by ID. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 4);
	directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
	directoryRequest.serviceId = serviceIds[0];
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests second service by name. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.pServiceName = &serviceNames[1];
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests second service by ID. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 6);
	directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
	directoryRequest.serviceId = serviceIds[1];
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by wrong ID. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 7);
	directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
	directoryRequest.serviceId = serviceIds[2]; // wrong servise ID
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by wrong name. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 8);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.pServiceName = &serviceNames[2];
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	while((pEvent = wtfGetEvent()))
	{
		RsslRDMService *serviceList;

		ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
		serviceList = pDirectoryRefresh->serviceList;

		switch(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId)
		{
			case 2: /* Full */
				ASSERT_TRUE(!stream2ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 2);
				stream2ReceivedMsg = RSSL_TRUE;
				break;
			case 3: /* First service by name */
				ASSERT_TRUE(!stream3ReceivedMsg);
				stream3ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				ASSERT_TRUE(rsslBufferIsEqual(&serviceNames[0], &serviceList[0].info.serviceName));
				break;
			case 4: /* First service by ID */
				ASSERT_TRUE(!stream4ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				ASSERT_TRUE(serviceList[0].serviceId == serviceIds[0]);
				stream4ReceivedMsg = RSSL_TRUE;
				break;
			case 5: /* Second service by name */
				ASSERT_TRUE(!stream5ReceivedMsg);
				stream5ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				ASSERT_TRUE(rsslBufferIsEqual(&serviceNames[1], &serviceList[0].info.serviceName));
				break;
			case 6: /* Second service by ID */
				ASSERT_TRUE(!stream6ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				stream6ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(serviceList[0].serviceId == serviceIds[1]);
				break;
			case 7: /* Third service by wrong ID */
				ASSERT_TRUE(!stream7ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 0);
				stream7ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(serviceList == NULL);
				break;
			case 8: /* Third service by wrong name */
				ASSERT_TRUE(!stream8ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 0);
				stream8ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(serviceList == NULL);
				break;
			default:
				ASSERT_TRUE(0);
				break;
		}
	}

	ASSERT_TRUE(stream2ReceivedMsg && stream3ReceivedMsg && stream4ReceivedMsg
			&& stream5ReceivedMsg && stream6ReceivedMsg && stream7ReceivedMsg && stream8ReceivedMsg);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());


	/* Provider sends directory update. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();

	directoryUpdate.serviceList = services;
	directoryUpdate.serviceCount = serviceCount + 1; /*Added last service*/

	wtfSetService1Info(&services[2]);

	/* Add service name. */
	services[2].info.serviceName = serviceNames[2];

	/* Add service ID. */
	services[2].serviceId = serviceIds[2];

	services[0].flags = RDM_SVCF_HAS_STATE;
	rsslClearRDMServiceState(&services[0].state);
	services[0].state.serviceState = 0;

	services[1].flags = RDM_SVCF_HAS_STATE;
	rsslClearRDMServiceState(&services[1].state);
	services[1].state.serviceState = 0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	stream2ReceivedMsg = RSSL_FALSE;
	stream3ReceivedMsg = RSSL_FALSE;
	stream4ReceivedMsg = RSSL_FALSE;
	stream5ReceivedMsg = RSSL_FALSE;
	stream6ReceivedMsg = RSSL_FALSE;
	stream7ReceivedMsg = RSSL_FALSE;
	stream8ReceivedMsg = RSSL_FALSE;

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	while((pEvent = wtfGetEvent()))
	{
		RsslRDMService *serviceList;

		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
		serviceList = pDirectoryUpdate->serviceList;

		switch(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId)
		{
			case 2: /* Full */
				ASSERT_TRUE(!stream2ReceivedMsg);
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 3);
				stream2ReceivedMsg = RSSL_TRUE;
				break;
			case 3: /* First service by name */
				ASSERT_TRUE(!stream3ReceivedMsg);
				stream3ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				break;
			case 4: /* First service by ID */
				ASSERT_TRUE(!stream4ReceivedMsg);
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				ASSERT_TRUE(serviceList[0].serviceId == serviceIds[0]);
				stream4ReceivedMsg = RSSL_TRUE;
				break;
			case 5: /* Second service by name */
				ASSERT_TRUE(!stream5ReceivedMsg);
				stream5ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				break;
			case 6: /* Second service by ID */
				ASSERT_TRUE(!stream6ReceivedMsg);
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				stream6ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(serviceList[0].serviceId == serviceIds[1]);
				break;
			case 7: /* Third service by wrong ID */
				ASSERT_TRUE(!stream7ReceivedMsg);
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				ASSERT_TRUE(serviceList[0].serviceId == serviceIds[2]);
				stream7ReceivedMsg = RSSL_TRUE;
				break;
			case 8: /* Third service by wrong name */
				ASSERT_TRUE(!stream8ReceivedMsg);
				ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
				stream8ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(rsslBufferIsEqual(&serviceNames[2], &serviceList[0].info.serviceName));
				break;
			default:
				ASSERT_TRUE(0);
				break;
		}
	}

	ASSERT_TRUE(stream2ReceivedMsg);
	ASSERT_TRUE(stream3ReceivedMsg);
	ASSERT_TRUE(stream4ReceivedMsg);
	ASSERT_TRUE(stream5ReceivedMsg);
	ASSERT_TRUE(stream6ReceivedMsg);
	ASSERT_TRUE(stream7ReceivedMsg);
	ASSERT_TRUE(stream8ReceivedMsg);

	wtfFinishTest();
}

void watchlistDirectoryTest_DeleteService(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	*pDirectoryUpdate;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMService service1;
	RsslRDMService serviceList[2];
	RsslUInt serviceId1 = 555;
	RsslBuffer serviceName1 = { 10, const_cast<char*>("ZUREK_SOUP") };

	RsslUInt serviceId2 = 666;
	RsslBuffer serviceName2 = { 13, const_cast<char*>("CZARNINA_SOUP") };

	RsslBuffer vendor = { 6, const_cast<char*>("Poland") };

	RsslUInt capabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE, RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_SYMBOL_LIST };
	RsslUInt32 capabilitiesCount = sizeof(capabilitiesList) / sizeof(RsslUInt);

	RsslBuffer dictionariesProvidedList[] = { { 6, const_cast<char*>("RWFFld") }, { 6, const_cast<char*>("Polish") }, { 7, const_cast<char*>("Klingon") }, { 12, const_cast<char*>("StrongBadian") } };
	RsslUInt32 dictionariesProvidedCount = sizeof(dictionariesProvidedList)/sizeof(RsslBuffer);

	RsslBuffer dictionariesUsedList[] = { { 5, const_cast<char*>("Spock") }, { 6, const_cast<char*>("Yelled") }, { 8, const_cast<char*>("KHAAAAAN") }, { 3, const_cast<char*>("And") }, { 1, const_cast<char*>("I") }, { 10, const_cast<char*>("Facepalmed") }};
	RsslUInt32 dictionariesUsedCount = sizeof(dictionariesUsedList)/sizeof(RsslBuffer);
	int s1, s2;

	ASSERT_TRUE(wtfStartTest());

	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with delete. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = service1Id;
	service1.action = RSSL_MPEA_DELETE_ENTRY;
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with two services. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.serviceList = serviceList;
	rsslClearRDMService(&serviceList[0]);
	rsslClearRDMService(&serviceList[1]);
	directoryUpdate.serviceCount = 2;
	serviceList[0].serviceId = serviceId1;
	serviceList[1].serviceId = serviceId2;
	serviceList[0].action = RSSL_MPEA_ADD_ENTRY;
	serviceList[1].action = RSSL_MPEA_ADD_ENTRY;
	serviceList[0].flags = RDM_SVCF_HAS_INFO;
	serviceList[1].flags = RDM_SVCF_HAS_INFO;
	serviceList[0].info.flags = (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED | RDM_SVC_IFF_HAS_VENDOR);
	serviceList[1].info.flags = (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED);
	serviceList[0].info.serviceName = serviceName1;
	serviceList[1].info.serviceName = serviceName2;
	serviceList[0].info.capabilitiesCount = capabilitiesCount;
	serviceList[1].info.capabilitiesCount = capabilitiesCount;
	serviceList[0].info.capabilitiesList = capabilitiesList;
	serviceList[1].info.capabilitiesList = capabilitiesList;
	serviceList[0].info.dictionariesProvidedCount = dictionariesProvidedCount;
	serviceList[1].info.dictionariesProvidedCount = dictionariesProvidedCount;
	serviceList[0].info.dictionariesProvidedList = dictionariesProvidedList;
	serviceList[1].info.dictionariesProvidedList = dictionariesProvidedList;
	serviceList[0].info.dictionariesUsedCount = dictionariesUsedCount;
	serviceList[1].info.dictionariesUsedCount = dictionariesUsedCount;
	serviceList[0].info.dictionariesUsedList = dictionariesUsedList;
	serviceList[1].info.dictionariesUsedList = dictionariesUsedList;
	serviceList[0].info.vendor = vendor;
	serviceList[1].info.vendor = vendor;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 2);
	// the services can arrive in any order. Check to see which is which
	if (pDirectoryUpdate->serviceList[0].serviceId == serviceId1)
	{
		s1 = 0;
		s2 = 1;
	}
	else if (pDirectoryUpdate->serviceList[0].serviceId == serviceId2)
	{
		s1 = 1;
		s2 = 0;
	}

	ASSERT_TRUE(pDirectoryUpdate->serviceList[s1].info.flags == (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED | RDM_SVC_IFF_HAS_VENDOR | RDM_SVC_IFF_HAS_QOS));
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s2].info.flags == (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED | RDM_SVC_IFF_HAS_QOS));
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s1].serviceId == serviceId1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s1].action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s1].info.serviceName.length == serviceName1.length);
	ASSERT_TRUE(memcmp(pDirectoryUpdate->serviceList[s1].info.serviceName.data, serviceName1.data, serviceName1.length) == 0);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s1].info.capabilitiesCount == capabilitiesCount);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s2].serviceId == serviceId2);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s2].action == RSSL_MPEA_ADD_ENTRY);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s2].info.serviceName.length == serviceName2.length);
	ASSERT_TRUE(memcmp(pDirectoryUpdate->serviceList[s2].info.serviceName.data, serviceName2.data, serviceName2.length) == 0);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[s2].info.capabilitiesCount == capabilitiesCount);

	// provider deletes one of the two services just added
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	service1.serviceId = serviceId1;
	service1.action = RSSL_MPEA_DELETE_ENTRY;
	directoryUpdate.serviceList = &service1;
	directoryUpdate.serviceCount = 1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);
	/* Consumer receives update */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].serviceId == serviceId1);
	ASSERT_TRUE(pDirectoryUpdate->serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);
	wtfFinishTest();
}
//
// this should generate a directory message of several kilobytes
//
void watchlistDirectoryTest_BigDirectory(RsslConnectionTypes connectionType)
{
	int i;
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	*pDirectoryUpdate;
	RsslRDMDirectoryUpdate	directoryUpdate;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMService serviceList[20];
	char		serviceNames[20][50];

	RsslBuffer vendor = { 108, const_cast<char*>("Ferengi, Bajoran, Tamarian, Orion, Andorii, Borg, Vulcan, Rihannsu, Federation, Denobula, Rigelian, Talaxian") };

	RsslUInt capabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE, RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_SYMBOL_LIST };
	RsslUInt32 capabilitiesCount = sizeof(capabilitiesList) / sizeof(RsslUInt);

	RsslBuffer dictionariesProvidedList[] = { 
		{22,const_cast<char*>("British Virgin Islands")},{10,const_cast<char*>("Cardassian")},{7,const_cast<char*>("Klingon")},{12,const_cast<char*>("StrongBadian")},{12,const_cast<char*>("Turkmenistan")},{13,const_cast<char*>("Liechtenstein")}, {24,const_cast<char*>("Negara Brunei Darussalam")}
	};
	RsslUInt32 dictionariesProvidedCount = sizeof(dictionariesProvidedList)/sizeof(RsslBuffer);

	RsslBuffer dictionariesUsedList[] = { 
		{22,const_cast<char*>("British Virgin Islands")},{7,const_cast<char*>("Romulan")},{7,const_cast<char*>("Klingon")},{12,const_cast<char*>("StrongBadian")},{12,const_cast<char*>("Turkmenistan")},{13,const_cast<char*>("Liechtenstein")}, {24,const_cast<char*>("Negara Brunei Darussalam")}
	};
	RsslUInt32 dictionariesUsedCount = sizeof(dictionariesUsedList)/sizeof(RsslBuffer);

	RsslBuffer itemList = { 131, const_cast<char*>("phasers, photon topedoes, communicator, replicator, holodeck, datapad, android, visor, Nacelle, anti-matter, tribble, Earl Grey Tea")};

	ASSERT_TRUE(wtfStartTest());
	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update with lots services. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.serviceList = serviceList;
	directoryUpdate.serviceCount = 20;
	for (i=0; i<20; i++)
	{
		rsslClearRDMService(&serviceList[i]);
		sprintf(serviceNames[i], "Star Trek Next Generation Episode %3d",i);
		serviceList[i].info.serviceName.length = 37;
		serviceList[i].info.serviceName.data = serviceNames[i];
		serviceList[i].serviceId = i + 20;
		serviceList[i].action = RSSL_MPEA_ADD_ENTRY;
		serviceList[i].flags = RDM_SVCF_HAS_INFO | RDM_SVCF_HAS_LOAD;
		serviceList[i].info.flags = (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED | RDM_SVC_IFF_HAS_VENDOR | RDM_SVC_IFF_HAS_QOS | RDM_SVC_IFF_HAS_ITEM_LIST);
		serviceList[i].info.capabilitiesCount = capabilitiesCount;
		serviceList[i].info.capabilitiesList = capabilitiesList;
		serviceList[i].info.dictionariesProvidedCount = dictionariesProvidedCount;
		serviceList[i].info.dictionariesProvidedList = dictionariesProvidedList;
		serviceList[i].info.dictionariesUsedCount = dictionariesUsedCount;
		serviceList[i].info.dictionariesUsedList = dictionariesUsedList;
		serviceList[i].info.vendor = vendor;
		serviceList[i].info.itemList = itemList;
		serviceList[i].load.flags = RDM_SVC_LDF_HAS_LOAD_FACTOR;
		serviceList[i].load.loadFactor = 0x7fffffffff;
	}

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives update */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 20);
	for (i=0; i<20; i++)
	{
		ASSERT_TRUE(pDirectoryUpdate->serviceList[i].info.flags == (RDM_SVC_IFF_HAS_DICTS_PROVIDED | RDM_SVC_IFF_HAS_DICTS_USED | RDM_SVC_IFF_HAS_VENDOR | RDM_SVC_IFF_HAS_QOS | RDM_SVC_IFF_HAS_ITEM_LIST));
		ASSERT_TRUE(pDirectoryUpdate->serviceList[i].action == RSSL_MPEA_ADD_ENTRY);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[i].info.serviceName.length == 37);
		ASSERT_TRUE(pDirectoryUpdate->serviceList[i].info.capabilitiesCount == capabilitiesCount);
	}

	wtfFinishTest();
}

void watchlistDirectoryTest_NoRdmCallbacks(WtfCallbackAction action, RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRefreshMsg	*pRefreshMsg;
	WtfSetupConnectionOpts wtfOpts;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	/* Since the watchlist provides administrative messages in their respective RDMMsg format,
	 * test that those messages can be reencoded as RsslMsgs when the consumer only has a 
	 * default Msg callback. */

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.consumerLoginCallback = action;
	wtfOpts.consumerDirectoryCallback = action;
	wtfOpts.consumerDictionaryCallback = action;
	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.msgClass == RSSL_MC_REFRESH);
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == 2);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryTest_DirectoryStatus(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMDirectoryRefresh *pDirectoryRefresh;
	RsslRDMDirectoryStatus directoryStatus;
	RsslRDMDirectoryUpdate *pDirectoryUpdate;

	RsslBool		stream2ReceivedMsg = RSSL_FALSE;
	RsslBool		stream3ReceivedMsg = RSSL_FALSE;
	RsslBool		stream4ReceivedMsg = RSSL_FALSE;

	ASSERT_TRUE(wtfStartTest());

	/* Make directory requests for all services, and by specific name/ID. */
	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests full directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by name. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 3);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests first service by ID. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 4);
	directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
	directoryRequest.serviceId = (RsslUInt16)service1Id;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory refresh. */
	while((pEvent = wtfGetEvent()))
	{
		RsslRDMService *serviceList;

		ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
		serviceList = pDirectoryRefresh->serviceList;

		switch(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId)
		{
			case 2: /* Full */
				ASSERT_TRUE(!stream2ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				stream2ReceivedMsg = RSSL_TRUE;
				break;
			case 3: /* Service by name */
				ASSERT_TRUE(!stream3ReceivedMsg);
				stream3ReceivedMsg = RSSL_TRUE;
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				ASSERT_TRUE(rsslBufferIsEqual(&service1Name, &serviceList[0].info.serviceName));
				break;
			case 4: /* Service by ID */
				ASSERT_TRUE(!stream4ReceivedMsg);
				ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
				ASSERT_TRUE(serviceList[0].serviceId == service1Id);
				stream4ReceivedMsg = RSSL_TRUE;
				break;
			default:
				ASSERT_TRUE(0);
				break;
		}
	}

	ASSERT_TRUE(stream2ReceivedMsg && stream3ReceivedMsg && stream4ReceivedMsg);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory suspect status. */
	rsslClearRDMDirectoryStatus(&directoryStatus);
	directoryStatus.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryStatus.flags = RDM_DR_STF_HAS_STATE;
	directoryStatus.state.streamState = RSSL_STREAM_OPEN;
	directoryStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Status messages are not forwarded (any changes are served out of cache). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());



	/* Provider sends directory closed recover status. */
	rsslClearRDMDirectoryStatus(&directoryStatus);
	directoryStatus.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryStatus.flags = RDM_DR_STF_HAS_STATE;
	directoryStatus.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
	directoryStatus.state.dataState = RSSL_DATA_SUSPECT;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryStatus;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	stream2ReceivedMsg = RSSL_FALSE;
	stream3ReceivedMsg = RSSL_FALSE;
	stream4ReceivedMsg = RSSL_FALSE;

	/* Should get updates, since status cleared cache. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Each directory request receives the state. */
	while((pEvent = wtfGetEvent()))
	{
		ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);

		switch(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId)
		{
			case 2: /* Full */
				ASSERT_TRUE(!stream2ReceivedMsg);
				stream2ReceivedMsg = RSSL_TRUE;
				break;
			case 3: /* Service by name */
				ASSERT_TRUE(!stream3ReceivedMsg);
				stream3ReceivedMsg = RSSL_TRUE;
				break;
			case 4: /* Service by ID */
				ASSERT_TRUE(!stream4ReceivedMsg);
				stream4ReceivedMsg = RSSL_TRUE;
				break;
			default:
				ASSERT_TRUE(0);
				break;
		}
	}

	wtfFinishTest();
}

void watchlistDirectoryDataTest_OneService(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts wtfOpts;
	RsslUInt32 ui, uh;

	RsslUInt serviceId = 555;
	RsslBuffer serviceName = { 9, const_cast<char*>("DUCK_SOUP") };
	RsslBuffer vendor = { 9, const_cast<char*>("Freedonia") };

	RsslUInt32 capabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE,
		RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_SYMBOL_LIST };
	RsslUInt32 capabilitiesCount = sizeof(capabilitiesList) / sizeof(RsslUInt32);

	RsslBuffer dictionariesProvidedList[] = { { 6, const_cast<char*>("RWFFld") }, { 7, const_cast<char*>("RWFEnum") }, { 7, const_cast<char*>("Klingon") },
    { 12, const_cast<char*>("StrongBadian") } };
	RsslUInt32 dictionariesProvidedCount = sizeof(dictionariesProvidedList)/sizeof(RsslBuffer);

	RsslBuffer dictionariesUsedList[] = { { 5, const_cast<char*>("Spock") }, { 6, const_cast<char*>("Yelled") }, { 8, const_cast<char*>("KHAAAAAN") },
		{ 3, const_cast<char*>("And") }, { 1, const_cast<char*>("I") }, { 10, const_cast<char*>("Facepalmed") }};
	RsslUInt32 dictionariesUsedCount = sizeof(dictionariesUsedList)/sizeof(RsslBuffer);

	RsslQos qosList[3];
	RsslUInt32 qosCount = 3;

	RsslBuffer itemList = { 13, const_cast<char*>("80S_CASH_LIST") };

	RsslBuffer dataBuffer = { 64, const_cast<char*>("I'd like to see an example of how to use this one of these days.") };

	RsslRDMServiceLink linkList[3];
	RsslBuffer linkNames[3] = { { 5, const_cast<char*>("Mario") }, { 5, const_cast<char*>("Luigi") }, {6, const_cast<char*>("Edison") } };
	RsslBuffer linkTexts[3] = { { 10, const_cast<char*>("Can't help") }, { 16, const_cast<char*>("but be hooked on") }, {12, const_cast<char*>("the brothers") } };
	RsslUInt32 linkCount = 3;

	/* Setup Qos list(watchlist will sort them but these should already be in the proper order). */
	rsslClearQos(&qosList[0]);
	qosList[0].timeliness = RSSL_QOS_TIME_REALTIME;
	qosList[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&qosList[1]);
	qosList[1].timeliness = RSSL_QOS_TIME_DELAYED;
	qosList[1].timeInfo = 7777;
	qosList[1].rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qosList[1].rateInfo = 9999;

	rsslClearQos(&qosList[2]);
	qosList[2].timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	qosList[2].rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qosList[2].rateInfo = 9999;

	/* Setup links. */
	for(ui = 0; ui < linkCount; ++ui)
	{
		rsslClearRDMServiceLink(&linkList[ui]);
		linkList[ui].flags = RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT;
		linkList[ui].name = linkNames[ui];
		linkList[ui].type = 1;
		linkList[ui].linkState = 1;
		linkList[ui].linkCode = 1;
		linkList[ui].text = linkTexts[ui];
	}


	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	service.info.flags |= RDM_SVC_IFF_HAS_VENDOR;
	service.info.vendor = vendor;
	
	service.info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
	service.info.isSource = 1;

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
	service.info.dictionariesProvidedList = dictionariesProvidedList;
	service.info.dictionariesProvidedCount = dictionariesProvidedCount;

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
	service.info.dictionariesUsedList = dictionariesUsedList;
	service.info.dictionariesUsedCount = dictionariesUsedCount;

	service.info.flags |= RDM_SVC_IFF_HAS_QOS;
	service.info.qosList = qosList;
	service.info.qosCount = qosCount;

	service.info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
	service.info.itemList = itemList;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
	service.info.supportsQosRange = 1;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
	service.info.supportsOutOfBandSnapshots = 0;

	service.info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
	service.info.acceptingConsumerStatus = 0;

	/* Setup state filter. */
	service.flags |= RDM_SVCF_HAS_STATE;

	service.state.serviceState = 1;

	service.state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
	service.state.acceptingRequests = 0;

	/* Setup load filter. */
	service.flags |= RDM_SVCF_HAS_LOAD;

	service.load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	service.load.openLimit = 100000;

	service.load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service.load.openWindow = 64;

	service.load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	service.load.loadFactor = 11111;

	/* Setup data filter. */
	service.flags |= RDM_SVCF_HAS_DATA;

	service.data.flags |= RDM_SVC_DTF_HAS_DATA;
	service.data.data = dataBuffer;
	service.data.dataType = RSSL_DT_ASCII_STRING;
	service.data.type = 8;

	/* Setup link filter. */
	service.flags |= RDM_SVCF_HAS_LINK;
	service.linkInfo.linkList = linkList;
	service.linkInfo.linkCount = linkCount;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* At the end of the loop, reissue the request to test if the cache can
	 * correctly reproduce the refresh (sans transient data such as the Data filter). */
	for(uh = 0; uh < 2; ++uh)
	{
		/* Consumer receives directory refresh. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
		ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
		ASSERT_TRUE(!wtfGetEvent());

		ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
		pService = &pDirectoryRefresh->serviceList[0];

		ASSERT_TRUE(pService->serviceId == serviceId);

		ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_INFO);

		ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_VENDOR);
		ASSERT_TRUE(rsslBufferIsEqual(&pService->info.vendor, &vendor));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_IS_SOURCE);
		ASSERT_TRUE(pService->info.isSource == 1);

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED);
		ASSERT_TRUE(pService->info.dictionariesProvidedCount == dictionariesProvidedCount);
		for (ui = 0; ui < dictionariesProvidedCount; ++ui)
			ASSERT_TRUE(rsslBufferIsEqual(&pService->info.dictionariesProvidedList[ui],
						&dictionariesProvidedList[ui]));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_DICTS_USED);
		ASSERT_TRUE(pService->info.dictionariesUsedCount == dictionariesUsedCount);
		for (ui = 0; ui < dictionariesUsedCount; ++ui)
			ASSERT_TRUE(rsslBufferIsEqual(&pService->info.dictionariesUsedList[ui],
						&dictionariesUsedList[ui]));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_QOS);
		ASSERT_TRUE(pService->info.qosCount == qosCount);
		for (ui = 0; ui < qosCount; ++ui)
			ASSERT_TRUE(rsslQosIsEqual(&pService->info.qosList[ui],
						&qosList[ui]));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_ITEM_LIST);
		ASSERT_TRUE(rsslBufferIsEqual(&pService->info.itemList, &itemList));

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE);
		ASSERT_TRUE(pService->info.supportsQosRange == 1);

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS);
		ASSERT_TRUE(pService->info.supportsOutOfBandSnapshots == 0);

		ASSERT_TRUE(pService->info.flags & RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS);
		ASSERT_TRUE(pService->info.acceptingConsumerStatus == 0);

		/* Test state filter. */
		ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_STATE);
		ASSERT_TRUE(pService->state.serviceState == 1);

		ASSERT_TRUE(pService->state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS);
		ASSERT_TRUE(pService->state.acceptingRequests == 0);

		/* Test load filter. */
		ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LOAD);

		ASSERT_TRUE(pService->load.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT);
		ASSERT_TRUE(pService->load.openLimit == 100000);

		ASSERT_TRUE(pService->load.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW);
		ASSERT_TRUE(pService->load.openWindow == 64);

		ASSERT_TRUE(pService->load.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR);
		ASSERT_TRUE(pService->load.loadFactor == 11111);

		/* Test data filter. */
		if (uh == 0)
		{
			ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_DATA);

			ASSERT_TRUE(pService->data.flags & RDM_SVC_DTF_HAS_DATA);
			ASSERT_TRUE(pService->data.dataType == RSSL_DT_ASCII_STRING);
			ASSERT_TRUE(pService->data.type == 8);
			ASSERT_TRUE(rsslBufferIsEqual(&pService->data.data, &dataBuffer));
		}
		else
		{
			ASSERT_TRUE(!(pService->flags & RDM_SVCF_HAS_DATA));
		}


		/* Test link filter. */
		ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
		ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

		for(ui = 0; ui < linkCount; ++ui)
		{
			RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
			/* RDM_SVC_LKF_HAS_TYPE is not present since the default value was set. */
			ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		}

		if (uh == 0)
		{
			rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
			directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
			rsslClearReactorSubmitMsgOptions(&opts);
			opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
			wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);
		}
	}

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTestDataFilterOnly_OneService(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts wtfOpts;
	RsslUInt32 ui, uh;

	RsslUInt serviceId = 555;
	RsslBuffer serviceName = { 9, const_cast<char*>("DUCK_SOUP") };
	RsslBuffer vendor = { 9, const_cast<char*>("Freedonia") };

	RsslUInt32 capabilitiesList[] = { RSSL_DMT_DICTIONARY, RSSL_DMT_MARKET_PRICE,
		RSSL_DMT_MARKET_BY_ORDER, RSSL_DMT_SYMBOL_LIST };
	RsslUInt32 capabilitiesCount = sizeof(capabilitiesList) / sizeof(RsslUInt32);

	RsslBuffer dictionariesProvidedList[] = { { 6, const_cast<char*>("RWFFld") },{ 7, const_cast<char*>("RWFEnum") },{ 7, const_cast<char*>("Klingon") },
	{ 12, const_cast<char*>("StrongBadian") } };
	RsslUInt32 dictionariesProvidedCount = sizeof(dictionariesProvidedList) / sizeof(RsslBuffer);

	RsslBuffer dictionariesUsedList[] = { { 5, const_cast<char*>("Spock") },{ 6, const_cast<char*>("Yelled") },{ 8, const_cast<char*>("KHAAAAAN") },
	{ 3, const_cast<char*>("And") },{ 1, const_cast<char*>("I") },{ 10, const_cast<char*>("Facepalmed") } };
	RsslUInt32 dictionariesUsedCount = sizeof(dictionariesUsedList) / sizeof(RsslBuffer);

	RsslQos qosList[3];
	RsslUInt32 qosCount = 3;

	RsslBuffer itemList = { 13, const_cast<char*>("80S_CASH_LIST") };

	RsslBuffer dataBuffer = { 64, const_cast<char*>("I'd like to see an example of how to use this one of these days.") };

	RsslRDMServiceLink linkList[3];
	RsslBuffer linkNames[3] = { { 5, const_cast<char*>("Mario") },{ 5, const_cast<char*>("Luigi") },{ 6, const_cast<char*>("Edison") } };
	RsslBuffer linkTexts[3] = { { 10, const_cast<char*>("Can't help") },{ 16, const_cast<char*>("but be hooked on") },{ 12, const_cast<char*>("the brothers") } };
	RsslUInt32 linkCount = 3;

	/* Setup Qos list(watchlist will sort them but these should already be in the proper order). */
	rsslClearQos(&qosList[0]);
	qosList[0].timeliness = RSSL_QOS_TIME_REALTIME;
	qosList[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&qosList[1]);
	qosList[1].timeliness = RSSL_QOS_TIME_DELAYED;
	qosList[1].timeInfo = 7777;
	qosList[1].rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qosList[1].rateInfo = 9999;

	rsslClearQos(&qosList[2]);
	qosList[2].timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	qosList[2].rate = RSSL_QOS_RATE_TIME_CONFLATED;
	qosList[2].rateInfo = 9999;

	/* Setup links. */
	for (ui = 0; ui < linkCount; ++ui)
	{
		rsslClearRDMServiceLink(&linkList[ui]);
		linkList[ui].flags = RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT;
		linkList[ui].name = linkNames[ui];
		linkList[ui].type = 1;
		linkList[ui].linkState = 1;
		linkList[ui].linkCode = 1;
		linkList[ui].text = linkTexts[ui];
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_DATA_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	opts.pServiceName = &serviceName;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	service.info.flags |= RDM_SVC_IFF_HAS_VENDOR;
	service.info.vendor = vendor;

	service.info.flags |= RDM_SVC_IFF_HAS_IS_SOURCE;
	service.info.isSource = 1;

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
	service.info.dictionariesProvidedList = dictionariesProvidedList;
	service.info.dictionariesProvidedCount = dictionariesProvidedCount;

	service.info.flags |= RDM_SVC_IFF_HAS_DICTS_USED;
	service.info.dictionariesUsedList = dictionariesUsedList;
	service.info.dictionariesUsedCount = dictionariesUsedCount;

	service.info.flags |= RDM_SVC_IFF_HAS_QOS;
	service.info.qosList = qosList;
	service.info.qosCount = qosCount;

	service.info.flags |= RDM_SVC_IFF_HAS_ITEM_LIST;
	service.info.itemList = itemList;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE;
	service.info.supportsQosRange = 1;

	service.info.flags |= RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS;
	service.info.supportsOutOfBandSnapshots = 0;

	service.info.flags |= RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS;
	service.info.acceptingConsumerStatus = 0;

	/* Setup state filter. */
	service.flags |= RDM_SVCF_HAS_STATE;

	service.state.serviceState = 1;

	service.state.flags |= RDM_SVC_STF_HAS_ACCEPTING_REQS;
	service.state.acceptingRequests = 0;

	/* Setup load filter. */
	service.flags |= RDM_SVCF_HAS_LOAD;

	service.load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	service.load.openLimit = 100000;

	service.load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	service.load.openWindow = 64;

	service.load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	service.load.loadFactor = 11111;

	/* Setup data filter. */
	service.flags |= RDM_SVCF_HAS_DATA;

	service.data.flags |= RDM_SVC_DTF_HAS_DATA;
	service.data.data = dataBuffer;
	service.data.dataType = RSSL_DT_ASCII_STRING;
	service.data.type = 8;

	/* Setup link filter. */
	service.flags |= RDM_SVCF_HAS_LINK;
	service.linkInfo.linkList = linkList;
	service.linkInfo.linkCount = linkCount;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* At the end of the loop, reissue the request to test if the cache can
	* correctly reproduce the refresh (sans transient data such as the Data filter). */
	for (uh = 0; uh < 2; ++uh)
	{
		/* Consumer receives directory refresh. */
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
		ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
		ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
		ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
		ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
		ASSERT_TRUE(!wtfGetEvent());

		ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
		pService = &pDirectoryRefresh->serviceList[0];

		ASSERT_TRUE(pService->serviceId == serviceId);

		ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_INFO);

		/* Test state filter. */
		ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_STATE);

		/* Test load filter. */
		ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_LOAD);

		/* Test data filter. */
		if (uh == 0)
		{
			ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_DATA);

			ASSERT_TRUE(pService->data.flags & RDM_SVC_DTF_HAS_DATA);
			ASSERT_TRUE(pService->data.dataType == RSSL_DT_ASCII_STRING);
			ASSERT_TRUE(pService->data.type == 8);
			ASSERT_TRUE(rsslBufferIsEqual(&pService->data.data, &dataBuffer));
		}
		else
		{
			ASSERT_TRUE(!(pService->flags & RDM_SVCF_HAS_DATA));
		}


		/* Test link filter. */
		ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_LINK);

		if (uh == 0)
		{
			rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
			directoryRequest.filter = RDM_DIRECTORY_SERVICE_DATA_FILTER;
			rsslClearReactorSubmitMsgOptions(&opts);
			opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
			opts.pServiceName = &serviceName;
			wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);
		}
	}

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTest_Links(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts wtfOpts;
	RsslUInt32 ui;

	RsslUInt serviceId = 555;
	RsslBuffer serviceName = { 9, const_cast<char*>("DUCK_SOUP") };

	RsslRDMServiceLink linkList[3];
	RsslBuffer linkNames[3] = { { 5, const_cast<char*>("Mario") }, { 5, const_cast<char*>("Luigi") }, {6, const_cast<char*>("Edison") } };
	RsslBuffer linkTexts[3] = { { 10, const_cast<char*>("Can't help") }, { 16, const_cast<char*>("but be hooked on") }, {12, const_cast<char*>("the brothers") } };
	RsslBuffer linkTexts2[3] = { { 6, const_cast<char*>("Galaga") }, { 8, const_cast<char*>("Galaxian") }, { 9, const_cast<char*>("Millipede") } };
	RsslUInt32 linkCount = 3;

	/* Setup links. */
	for(ui = 0; ui < linkCount; ++ui)
	{
		rsslClearRDMServiceLink(&linkList[ui]);
		linkList[ui].flags = RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT;
		linkList[ui].name = linkNames[ui];
		linkList[ui].type = 1;
		linkList[ui].linkState = 1;
		linkList[ui].linkCode = 1;
		linkList[ui].text = linkTexts[ui];
	}


	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	/* Setup link filter. */
	service.flags |= RDM_SVCF_HAS_LINK;
	service.linkInfo.linkList = linkList;
	service.linkInfo.linkCount = linkCount;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing (directory not yet requested). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_INFO);

	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Test link filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

	for(ui = 0; ui < linkCount; ++ui)
	{
		RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
		RsslBool found_it = RSSL_FALSE;
		int j;

		ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		ASSERT_TRUE(pLink->type == 1);
		ASSERT_TRUE(pLink->linkState == 1);
		ASSERT_TRUE(pLink->linkCode == 1);
		for (j=0; j<3; j++)
		{
			if (pLink->name.length == linkList[j].name.length && rsslBufferIsEqual(&pLink->name, &linkList[j].name))
			{
				found_it = RSSL_TRUE;
				break;
			}
		}
		ASSERT_TRUE(found_it == RSSL_TRUE);
		ASSERT_TRUE(rsslBufferIsEqual(&pLink->text,&linkList[j].text));
	}

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider updates links. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.flags = RDM_DR_UPF_HAS_FILTER;

	directoryUpdate.serviceList = &service;
	directoryUpdate.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	service.flags |= RDM_SVCF_HAS_LINK;
	service.linkInfo.linkList = linkList;
	service.linkInfo.linkCount = linkCount;

	for(ui = 0; ui < linkCount; ++ui)
	{
		rsslClearRDMServiceLink(&linkList[ui]);
		linkList[ui].flags = RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT;
		linkList[ui].name = linkNames[ui];
		linkList[ui].type = 2;
		linkList[ui].linkState = 2;
		linkList[ui].linkCode = 2;
		linkList[ui].text = linkTexts2[ui];
	}

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	pService = &pDirectoryUpdate->serviceList[0];

	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

	for(ui = 0; ui < linkCount; ++ui)
	{
		RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
		RsslBool found_it = RSSL_FALSE;
		int j;

		ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		ASSERT_TRUE(pLink->type == 2);
		ASSERT_TRUE(pLink->linkState == 2);
		ASSERT_TRUE(pLink->linkCode == 2);
		for (j=0; j<3; j++)
		{
			if (pLink->name.length == linkList[j].name.length && rsslBufferIsEqual(&pLink->name, &linkList[j].name))
			{
				found_it = RSSL_TRUE;
				break;
			}
		}
		ASSERT_TRUE(found_it == RSSL_TRUE);
		ASSERT_TRUE(rsslBufferIsEqual(&pLink->text,&linkList[j].text));
	}


	/* Consumer re-requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter |= (RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh from the watchlist (not from network) */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_INFO);

	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Test link filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

	for(ui = 0; ui < linkCount; ++ui)
	{
		RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
		RsslBool found_it = RSSL_FALSE;
		int j;

		ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		ASSERT_TRUE(pLink->type == 2);
		ASSERT_TRUE(pLink->linkState == 2);
		ASSERT_TRUE(pLink->linkCode == 2);
		for (j=0; j<3; j++)
		{
			if (pLink->name.length == linkList[j].name.length && rsslBufferIsEqual(&pLink->name, &linkList[j].name))
			{
				found_it = RSSL_TRUE;
				break;
			}
		}
		ASSERT_TRUE(found_it == RSSL_TRUE);
		ASSERT_TRUE(rsslBufferIsEqual(&pLink->text,&linkList[j].text));
	}

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTestLinkFilterOnly_Links(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts wtfOpts;
	RsslUInt32 ui;

	RsslUInt serviceId = 555;
	RsslBuffer serviceName = { 9, const_cast<char*>("DUCK_SOUP") };

	RsslRDMServiceLink linkList[3];
	RsslBuffer linkNames[3] = { { 5, const_cast<char*>("Mario") },{ 5, const_cast<char*>("Luigi") },{ 6, const_cast<char*>("Edison") } };
	RsslBuffer linkTexts[3] = { { 10, const_cast<char*>("Can't help") },{ 16, const_cast<char*>("but be hooked on") },{ 12, const_cast<char*>("the brothers") } };
	RsslBuffer linkTexts2[3] = { { 6, const_cast<char*>("Galaga") },{ 8, const_cast<char*>("Galaxian") },{ 9, const_cast<char*>("Millipede") } };
	RsslUInt32 linkCount = 3;

	/* Setup links. */
	for (ui = 0; ui < linkCount; ++ui)
	{
		rsslClearRDMServiceLink(&linkList[ui]);
		linkList[ui].flags = RDM_SVC_LKF_HAS_TYPE | RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT;
		linkList[ui].name = linkNames[ui];
		linkList[ui].type = 1;
		linkList[ui].linkState = 1;
		linkList[ui].linkCode = 1;
		linkList[ui].text = linkTexts[ui];
	}


	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_LINK_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	/* Setup link filter. */
	service.flags |= RDM_SVCF_HAS_LINK;
	service.linkInfo.linkList = linkList;
	service.linkInfo.linkCount = linkCount;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	/* Test link filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

	for (ui = 0; ui < linkCount; ++ui)
	{
		RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
		RsslBool found_it = RSSL_FALSE;
		int j;

		ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		ASSERT_TRUE(pLink->type == 1);
		ASSERT_TRUE(pLink->linkState == 1);
		ASSERT_TRUE(pLink->linkCode == 1);
		for (j = 0; j<3; j++)
		{
			if (pLink->name.length == linkList[j].name.length && rsslBufferIsEqual(&pLink->name, &linkList[j].name))
			{
				found_it = RSSL_TRUE;
				break;
			}
		}
		ASSERT_TRUE(found_it == RSSL_TRUE);
		ASSERT_TRUE(rsslBufferIsEqual(&pLink->text, &linkList[j].text));
	}

	/* Consumer requests directory on a different stream. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_LINK_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	opts.pServiceName = &serviceName;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	/* Test link filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_LINK);
	ASSERT_TRUE(pService->linkInfo.linkCount == linkCount);

	for (ui = 0; ui < linkCount; ++ui)
	{
		RsslRDMServiceLink *pLink = &pService->linkInfo.linkList[ui];
		RsslBool found_it = RSSL_FALSE;
		int j;

		ASSERT_TRUE(pLink->flags == (RDM_SVC_LKF_HAS_CODE | RDM_SVC_LKF_HAS_TEXT));
		ASSERT_TRUE(pLink->type == 1);
		ASSERT_TRUE(pLink->linkState == 1);
		ASSERT_TRUE(pLink->linkCode == 1);
		for (j = 0; j < 3; j++)
		{
			if (pLink->name.length == linkList[j].name.length && rsslBufferIsEqual(&pLink->name, &linkList[j].name))
			{
				found_it = RSSL_TRUE;
				break;
			}
		}
		ASSERT_TRUE(found_it == RSSL_TRUE);
		ASSERT_TRUE(rsslBufferIsEqual(&pLink->text, &linkList[j].text));
	}

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTest_Groups(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMServiceGroupState groupStateList[2];
	char someRandomString[] = "some random String";
	char one[2] = { 0, 1 };
	char zero[2] = { 0, 0 };
	RsslBuffer group1 = { 2, (char *)one };
	RsslBuffer group0 = { 2, (char *)zero };
	RsslBuffer someRandomText = { sizeof(someRandomString), const_cast<char*>(someRandomString) };
	WtfSetupConnectionOpts wtfOpts;

	RsslUInt serviceId = 43;
	RsslBuffer serviceName = { 12, const_cast<char*>("CHICKEN_FEED") };

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives nothing (directory not yet requested). */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_INFO);

	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Provider received no more messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider updates groups. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.flags = RDM_DR_UPF_NONE; 

	directoryUpdate.serviceList = &service;
	directoryUpdate.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	service.groupStateCount = 2;
	service.groupStateList = groupStateList;

	rsslClearRDMServiceGroupState(&groupStateList[0]);
	groupStateList[0].flags = RDM_SVC_GRF_HAS_STATUS;
	groupStateList[0].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[0].group = group1;
	groupStateList[0].status.dataState = RSSL_DATA_SUSPECT;
	groupStateList[0].status.streamState = RSSL_STREAM_OPEN;
	groupStateList[0].status.code = RSSL_SC_NO_RESOURCES;
	groupStateList[0].status.text = someRandomText;

	rsslClearRDMServiceGroupState(&groupStateList[1]);
	groupStateList[1].flags = RDM_SVC_GRF_HAS_MERGED_TO_GROUP;
	groupStateList[1].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[1].group = group1;
	groupStateList[1].mergedToGroup = group0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Consumer receives directory update. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	pService = &pDirectoryUpdate->serviceList[0];

	ASSERT_TRUE(pService->flags == RDM_SVCF_NONE);
	ASSERT_TRUE(pService->groupStateCount == 2);

	ASSERT_TRUE(pService->groupStateList[0].flags == RDM_SVC_GRF_HAS_STATUS);
	ASSERT_TRUE(pService->groupStateList[0].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[0].group, &group1));
	ASSERT_TRUE(pService->groupStateList[0].status.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pService->groupStateList[0].status.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pService->groupStateList[0].status.code == RSSL_SC_NO_RESOURCES);
	ASSERT_TRUE(memcmp(pService->groupStateList[0].status.text.data, someRandomText.data, someRandomText.length - 1) == 0);

	ASSERT_TRUE(pService->groupStateList[1].flags == RDM_SVC_GRF_HAS_MERGED_TO_GROUP);
	ASSERT_TRUE(pService->groupStateList[1].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].group, &group1));
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].mergedToGroup, &group0));

	/* Consumer re-requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh from the watchlist (not from network) */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pService->groupStateCount == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTestGroupFilterOnly_Groups(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	RsslRDMServiceGroupState groupStateList[2];
	char someRandomString[] = "some random String";
	char one[2] = { 0, 1 };
	char zero[2] = { 0, 0 };
	RsslBuffer group1 = { 2, (char *)one };
	RsslBuffer group0 = { 2, (char *)zero };
	RsslBuffer someRandomText = { sizeof(someRandomString), const_cast<char*>(someRandomString) };
	WtfSetupConnectionOpts wtfOpts;

	RsslUInt serviceId = 43;
	RsslBuffer serviceName = { 12, const_cast<char*>("CHICKEN_FEED") };

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_INFO);

	/* Consumer requests directory on a different stream. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775556;
	opts.pServiceName = &serviceName;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider received no more messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives directory refresh on a different stream. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775556);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_INFO);

	/* Provider updates groups. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	directoryUpdate.flags = RDM_DR_UPF_NONE;

	directoryUpdate.serviceList = &service;
	directoryUpdate.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	service.groupStateCount = 2;
	service.groupStateList = groupStateList;

	rsslClearRDMServiceGroupState(&groupStateList[0]);
	groupStateList[0].flags = RDM_SVC_GRF_HAS_STATUS;
	groupStateList[0].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[0].group = group1;
	groupStateList[0].status.dataState = RSSL_DATA_SUSPECT;
	groupStateList[0].status.streamState = RSSL_STREAM_OPEN;
	groupStateList[0].status.code = RSSL_SC_NO_RESOURCES;
	groupStateList[0].status.text = someRandomText;

	rsslClearRDMServiceGroupState(&groupStateList[1]);
	groupStateList[1].flags = RDM_SVC_GRF_HAS_MERGED_TO_GROUP;
	groupStateList[1].action = RSSL_FTEA_SET_ENTRY;
	groupStateList[1].group = group1;
	groupStateList[1].mergedToGroup = group0;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE);

	/* Provider received no more messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Dispatch consumer. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives directory update. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	pService = &pDirectoryUpdate->serviceList[0];

	ASSERT_TRUE(pService->flags == RDM_SVCF_NONE);
	ASSERT_TRUE(pService->groupStateCount == 2);

	ASSERT_TRUE(pService->groupStateList[0].flags == RDM_SVC_GRF_HAS_STATUS);
	ASSERT_TRUE(pService->groupStateList[0].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[0].group, &group1));
	ASSERT_TRUE(pService->groupStateList[0].status.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pService->groupStateList[0].status.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pService->groupStateList[0].status.code == RSSL_SC_NO_RESOURCES);
	ASSERT_TRUE(memcmp(pService->groupStateList[0].status.text.data, someRandomText.data, someRandomText.length - 1) == 0);

	ASSERT_TRUE(pService->groupStateList[1].flags == RDM_SVC_GRF_HAS_MERGED_TO_GROUP);
	ASSERT_TRUE(pService->groupStateList[1].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].group, &group1));
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].mergedToGroup, &group0));

	/* Consumer receives directory update on different stream. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775556);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	pService = &pDirectoryUpdate->serviceList[0];

	ASSERT_TRUE(pService->flags == RDM_SVCF_NONE);
	ASSERT_TRUE(pService->groupStateCount == 2);

	ASSERT_TRUE(pService->groupStateList[0].flags == RDM_SVC_GRF_HAS_STATUS);
	ASSERT_TRUE(pService->groupStateList[0].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[0].group, &group1));
	ASSERT_TRUE(pService->groupStateList[0].status.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pService->groupStateList[0].status.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pService->groupStateList[0].status.code == RSSL_SC_NO_RESOURCES);
	ASSERT_TRUE(memcmp(pService->groupStateList[0].status.text.data, someRandomText.data, someRandomText.length - 1) == 0);

	ASSERT_TRUE(pService->groupStateList[1].flags == RDM_SVC_GRF_HAS_MERGED_TO_GROUP);
	ASSERT_TRUE(pService->groupStateList[1].action == RSSL_FTEA_SET_ENTRY);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].group, &group1));
	ASSERT_TRUE(rsslBufferIsEqual(&pService->groupStateList[1].mergedToGroup, &group0));



	/* Consumer re-requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh from the watchlist (not from network) */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pService->groupStateCount == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer re-requests directory on a different stream. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.pServiceName = &serviceName;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh from the watchlist (not from network) */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775556);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_INFO);
	ASSERT_TRUE(pService->groupStateCount == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->info.serviceName, &serviceName));

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryDataTestDataFilterOnly_Data(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	directoryRefresh, *pDirectoryRefresh;
	RsslRDMService service, *pService;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts wtfOpts;

	RsslUInt serviceId = 555;
	RsslBuffer serviceName = { 9, const_cast<char*>("DUCK_SOUP") };

	RsslBuffer dataBuffer = { 64, const_cast<char*>("I'd like to see an example of how to use this one of these days.") };

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupConnectionOpts(&wtfOpts);
	wtfOpts.provideDefaultDirectory = RSSL_FALSE;

	wtfSetupConnection(&wtfOpts, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_DATA_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer requests directory on a different stream id. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 3);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_DATA_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775553;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider sets up directory refresh (should have received request from watchlist already). */
	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryRefresh.filter = wtfGetProviderDirectoryFilter();
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	rsslClearRDMService(&service);
	service.serviceId = serviceId;

	/* Setup info filter. */
	service.flags |= RDM_SVCF_HAS_INFO;

	service.info.serviceName = serviceName;

	/* Setup data filter. */
	service.flags |= RDM_SVCF_HAS_DATA;

	service.data.flags |= RDM_SVC_DTF_HAS_DATA;
	service.data.data = dataBuffer;
	service.data.dataType = RSSL_DT_ASCII_STRING;
	service.data.type = 8;

	/* Provider sends directory refresh. */
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);
	/* Test data filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_DATA);

	ASSERT_TRUE(pService->data.flags & RDM_SVC_DTF_HAS_DATA);
	ASSERT_TRUE(pService->data.dataType == RSSL_DT_ASCII_STRING);
	ASSERT_TRUE(pService->data.type == 8);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->data.data, &dataBuffer));

	/* Consumer receives directory refresh. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 3);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775553);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);
	/* Test data filter. */
	ASSERT_TRUE(pService->flags & RDM_SVCF_HAS_DATA);

	ASSERT_TRUE(pService->data.flags & RDM_SVC_DTF_HAS_DATA);
	ASSERT_TRUE(pService->data.dataType == RSSL_DT_ASCII_STRING);
	ASSERT_TRUE(pService->data.type == 8);
	ASSERT_TRUE(rsslBufferIsEqual(&pService->data.data, &dataBuffer));

	/* Consumer requests directory on a different stream. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 5);
	directoryRequest.filter = RDM_DIRECTORY_SERVICE_DATA_FILTER;
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	opts.pServiceName = &serviceName;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 5);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);
	ASSERT_TRUE(!wtfGetEvent());

	ASSERT_TRUE(pDirectoryRefresh->serviceCount == 1);
	pService = &pDirectoryRefresh->serviceList[0];

	ASSERT_TRUE(pService->serviceId == serviceId);

	/* Test data filter. Should be false here since data filter is not cached. */
	ASSERT_FALSE(pService->flags & RDM_SVCF_HAS_DATA);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	wtfFinishTest();
}

void watchlistDirectoryTest_DuplicateServiceName(RsslConnectionTypes connectionType)
{
	WtfEvent		*pEvent;
	WtfChannelEvent *pChannelEvent;
	RsslRDMDirectoryRequest directoryRequest;
	RsslRDMDirectoryRefresh	*pDirectoryRefresh;
	RsslRDMDirectoryUpdate	directoryUpdate, *pDirectoryUpdate;
	RsslRDMLoginStatus *pLoginStatus;
	RsslRDMService service1, *serviceList;
	RsslReactorSubmitMsgOptions opts;

	ASSERT_TRUE(wtfStartTest());

	/* Tests that watchlist handles change in service ID such that the directory
	 * appears to have two services with the same name. This should result in a disconnect. */

	wtfSetupConnection(NULL, connectionType);

	/* Consumer requests directory. */
	rsslInitDefaultRDMDirectoryRequest(&directoryRequest, 2);
	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryRequest;
	opts.requestMsgOptions.pUserSpec = (void*)0x77775555;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_FALSE);

	/* Consumer receives directory refresh. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pDirectoryRefresh = (RsslRDMDirectoryRefresh*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.rdmMsgType == RDM_DR_MT_REFRESH);
	ASSERT_TRUE(pDirectoryRefresh->rdmMsgBase.streamId == 2);
	ASSERT_TRUE(pDirectoryRefresh->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pDirectoryRefresh->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x77775555);

	/* Provider received no messages. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(!wtfGetEvent());

	/* Provider sends directory update. */
	rsslClearRDMDirectoryUpdate(&directoryUpdate);
	directoryUpdate.rdmMsgBase.streamId = wtfGetProviderDirectoryStream();
	directoryUpdate.filter = wtfGetProviderDirectoryFilter();
	rsslClearRDMService(&service1);
	wtfSetService1Info(&service1);
	service1.serviceId = service1Id + 1;
	directoryUpdate.serviceCount = 1;
	directoryUpdate.serviceList = &service1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRDMMsg = (RsslRDMMsg*)&directoryUpdate;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);
	
	/* Consumer receives directory update removing service. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pDirectoryUpdate = (RsslRDMDirectoryUpdate*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pDirectoryUpdate->rdmMsgBase.rdmMsgType == RDM_DR_MT_UPDATE);
	ASSERT_TRUE(pDirectoryUpdate->serviceCount == 1);
	serviceList = pDirectoryUpdate->serviceList;
	ASSERT_TRUE(serviceList[0].serviceId == service1Id);
	ASSERT_TRUE(serviceList[0].action == RSSL_MPEA_DELETE_ENTRY);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE((pChannelEvent = wtfGetChannelEvent(pEvent)));
	ASSERT_TRUE(pChannelEvent->channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pChannelEvent->rsslErrorId == RSSL_RET_FAILURE);

	wtfFinishTest();
}
