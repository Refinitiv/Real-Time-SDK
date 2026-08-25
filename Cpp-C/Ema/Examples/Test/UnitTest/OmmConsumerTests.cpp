/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "ActiveConfig.h"
#include "Ema.h"
#include "rtr/rsslThread.h"
#include "EmaTestClients.h"
#include <iostream>


using namespace refinitiv::ema::access;
using namespace std;

class OmmConsumerTest : public ::testing::Test {
public:

	void SetUp()
	{
	}

	void TearDown()
	{
		// Make sure all tests have enough time to close everything.
		testSleep(1000);
	}
};

void setupSingleServiceMap(Map& map, EmaString serviceName, UInt64 serviceId)
{
	map.addKeyUInt(serviceId, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, serviceName).
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST,"ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
}

/* This test runs through serviceList structure functionality, testing the clear function and all of the setters.
 */
TEST_F(OmmConsumerTest, ServiceListStructureTest)
{
	ServiceList serviceList("serviceList1");

	serviceList.addService("FEED_1");
	serviceList.addService("FEED_2");
	serviceList.addService("FEED_3");

	ASSERT_TRUE(serviceList.name() == "serviceList1");

	ASSERT_EQ(3, serviceList.concreteServiceList().size());
	ASSERT_TRUE(serviceList.concreteServiceList()[0] == "FEED_1");
	ASSERT_TRUE(serviceList.concreteServiceList()[1] == "FEED_2");
	ASSERT_TRUE(serviceList.concreteServiceList()[2] == "FEED_3");

	serviceList.clear();

	ASSERT_EQ(0, serviceList.concreteServiceList().size());

	serviceList.name("serviceList2");

	serviceList.addService("FEED_4");
	serviceList.addService("FEED_5");
	serviceList.addService("FEED_6");


	ASSERT_TRUE(serviceList.name() == "serviceList2");

	ASSERT_EQ(3, serviceList.concreteServiceList().size());
	ASSERT_TRUE(serviceList.concreteServiceList()[0] == "FEED_4");
	ASSERT_TRUE(serviceList.concreteServiceList()[1] == "FEED_5");
	ASSERT_TRUE(serviceList.concreteServiceList()[2] == "FEED_6");
}

/* This tests servicelist interactions with the OmmConsumerConfig class, ensuring that the correct exceptions are thrown in error cases:
 * 1. A blank service list name
 * 2. Adding a service list with the same name twice
 */
TEST_F(OmmConsumerTest, ServiceListConfigTest)
{
	ServiceList blankServiceList("");

	blankServiceList.addService("FEED_1");

	ServiceList serviceList("serviceList1");

	serviceList.addService("FEED_1");
	serviceList.addService("FEED_2");
	serviceList.addService("FEED_3");

	OmmConsumerConfig consConfig("EmaConfigTest.xml");

	try
	{
		consConfig.addServiceList(blankServiceList);

		ASSERT_TRUE(false) << "Blank exception not thrown";
	}
	catch (...)
	{
		ASSERT_TRUE(true);
	}

	try
	{
		consConfig.addServiceList(serviceList);

		consConfig.addServiceList(serviceList);

		ASSERT_TRUE(false) << "Identical name exception not thrown";

	}
	catch (...)
	{
		ASSERT_TRUE(true);
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with no providers
 *  2. Checks to see that the consumer does not initialize
 *  3. Checks to see that the consumer did not get anyhting other than OPEN/SUSPECT login status messages
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginTimeoutTestBothDown)
{
	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);

	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmConsumer cons(consConfig, consClient);

		ASSERT_FALSE(true) << "Expected exception";

	}
	catch (...)
	{
		ASSERT_TRUE(true);
	}

	ASSERT_TRUE(consClient.getMessageQueueSize() > 0);

	StatusMsg* msg = (StatusMsg*)consClient.popMsg();

	ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
	ASSERT_TRUE(msg->hasState());
	ASSERT_EQ(msg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
	ASSERT_EQ(msg->getState().getDataState(), OmmState::DataState::SuspectEnum);

}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with one provider
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has only one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginTimeoutTestOneDown)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient(provTestOptions);

	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");
	Msg* msg;

	try
	{
		OmmProvider prov(provConfig, provClient);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}



}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginBothUp)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		
		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. One provider is configured to not send a login response
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginTimeoutOneNoResponse)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap;

	ProviderTestOptions provTestOptions2;
	provTestOptions2.sendLoginRefresh = false;
	provTestOptions2.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			EmaVector<ChannelInformation> channelInfo;

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. One provider is configured to deny the login response
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginTimeoutOneDeny)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap;

	ProviderTestOptions provTestOptions2;
	provTestOptions2.acceptLoginRequest = false;
	provTestOptions2.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			EmaVector<ChannelInformation> channelInfo;

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. Both providers are configured to accept the resopnse
 *  2. Checks to see that the consumer initializes
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. One provider sends a login status with CLOSED/SUSPECT
 *  5. Consumer receives an OPEN/OK status
 *  6. Other provdier sends a login status with CLOSED/SUSPECT
 *  7. Consumer receives a login status with CLOSED/SUSPECT.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginBothDeny)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		prov1.submit(StatusMsg().domainType(MMT_LOGIN).nameType(USER_NAME).name(provClient1.loginUserName).
			state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Login Denied"), provClient1.loginHandle);

		testSleep(5000);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);
		
		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		prov2.submit(StatusMsg().domainType(MMT_LOGIN).nameType(USER_NAME).name(provClient2.loginUserName).
			state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Login Denied"), provClient2.loginHandle);

		testSleep(5000);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. Both providers are configured to accept the resopnse
 *  2. Checks to see that the consumer initializes
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Shut down provider 1 fully
 *  5. Consumer receives one status message OPEN/OK
 *  6. Shut down Provider 2 fully
 *  7. Consumer receives two login status with OPEN/SUSPECT.
 * NOTE: This test uses EXPECT instead of ASSERT to make sure that everything is cleaned up properly.
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginChannelDown)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelLongReconnect");
	OmmProvider* prov1 = NULL;
	OmmProvider* prov2 = NULL;
	try
	{
		prov1 = new OmmProvider(provConfig1, provClient1);
		prov2 = new OmmProvider(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		consClient.clear();

		delete prov1;
		prov1 = NULL;

		testSleep(500);

		cons.getSessionInformation(channelInfo);

		EXPECT_EQ(channelInfo.size(), 2);

		EXPECT_GE(consClient.getMessageQueueSize(), 1u);

		while ((msg = consClient.popMsg()) != NULL)
		{
			// All status messages here should be OPEN/OK
			EXPECT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

			// Channel down_reconnecting
			statusMsg = static_cast<StatusMsg*>(msg);
			EXPECT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
			EXPECT_TRUE(statusMsg->hasState());
			EXPECT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			EXPECT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


		EXPECT_EQ(consClient.getMessageQueueSize(), 0);

		delete prov2;
		prov2 = NULL;

		testSleep(500);

		cons.getSessionInformation(channelInfo);

		EXPECT_EQ(channelInfo.size(), 2);

		EXPECT_GE(consClient.getMessageQueueSize(), 1u);

		OmmState::StreamState streamState = OmmState::StreamState::ClosedEnum;
		OmmState::DataState dataState = OmmState::DataState::NoChangeEnum;
		while ((msg = consClient.popMsg()) != NULL)
		{
			// All status messages here should be OPEN/OK
			EXPECT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

			// Channel down_reconnecting
			statusMsg = static_cast<StatusMsg*>(msg);
			EXPECT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
			EXPECT_TRUE(statusMsg->hasState());
			streamState = statusMsg->getState().getStreamState();
			dataState = statusMsg->getState().getDataState();
		}

		// The last message(s) should be OPEN/SUSPECT
		EXPECT_EQ(streamState, OmmState::StreamState::OpenEnum);
		EXPECT_EQ(dataState, OmmState::DataState::SuspectEnum);

	}
	catch (...)
	{
		if (prov1 != NULL)
			delete prov1;
		if (prov2 != NULL)
			delete prov2;
		ASSERT_TRUE(false) << "uncaught exception in test";
	}

	// Cleanup
	if (prov1 != NULL)
		delete prov1;
	if (prov2 != NULL)
		delete prov2;

}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryRequestOneService)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& acceptingRequests = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_ACCEPTING_REQS);
			ASSERT_EQ(state.getUInt(), SERVICE_YES);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED and DIRECT_FEED_2 service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryRequestTwoServices)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED_2");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channel and that it got a OPEN/OK refresh message.
 *  4. Consumer registers for the directory domain with DIRECT_FEED specified
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryRequestByName)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY).serviceName("DIRECT_FEED");

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryRequestById)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY).serviceId(2);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED_2");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with SERVICE_INFO and SERVICE_STATE filter
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information with the appropriate filters
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryRequestTwoServicesWithFilter)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}
		// Request with 1 filter
		{
			ReqMsg directoryRequest;
			directoryRequest.domainType(MMT_DIRECTORY).filter(SERVICE_STATE_FILTER);

			UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

			testSleep(500);

			ASSERT_EQ(consClient.getMessageQueueSize(), 1);

			msg = consClient.popMsg();

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

			ASSERT_TRUE(refreshMsg->getSolicited());
			ASSERT_TRUE(refreshMsg->getComplete());
			ASSERT_TRUE(refreshMsg->hasMsgKey());
			ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

			const Map& mapPayload = refreshMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			{
				const MapEntry& mapEntry = mapPayload.getEntry();

				ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

				EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
				const FilterList& filter = mapEntry.getFilterList();
				ASSERT_TRUE(filter.forth());

				const FilterEntry& stateFilter = filter.getEntry();

				ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
				ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& stateList = stateFilter.getElementList();

				ASSERT_TRUE(stateList.forth());

				const ElementEntry& state = stateList.getEntry();

				ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
				ASSERT_EQ(state.getUInt(), SERVICE_UP);

				ASSERT_FALSE(stateList.forth());

				ASSERT_FALSE(filter.forth());
			}

			EXPECT_TRUE(mapPayload.forth());

			{
				const MapEntry& mapEntry = mapPayload.getEntry();

				ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

				EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
				const FilterList& filter = mapEntry.getFilterList();
				ASSERT_TRUE(filter.forth());

				const FilterEntry& stateFilter = filter.getEntry();

				ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
				ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& stateList = stateFilter.getElementList();

				ASSERT_TRUE(stateList.forth());

				const ElementEntry& state = stateList.getEntry();

				ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
				ASSERT_EQ(state.getUInt(), SERVICE_UP);

				ASSERT_FALSE(stateList.forth());

				ASSERT_FALSE(filter.forth());

			}

			ASSERT_FALSE(mapPayload.forth());
		}

		// Request with 2 filters
		{
			ReqMsg directoryRequest;
			const OmmArrayEntry* arrayEntry;
			directoryRequest.domainType(MMT_DIRECTORY).filter(SERVICE_INFO_FILTER | SERVICE_STATE_FILTER);

			UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

			testSleep(500);

			ASSERT_EQ(consClient.getMessageQueueSize(), 1);

			msg = consClient.popMsg();

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

			ASSERT_TRUE(refreshMsg->getSolicited());
			ASSERT_TRUE(refreshMsg->getComplete());
			ASSERT_TRUE(refreshMsg->hasMsgKey());
			ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

			const Map& mapPayload = refreshMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			{
				const MapEntry& mapEntry = mapPayload.getEntry();

				ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

				EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
				const FilterList& filter = mapEntry.getFilterList();
				ASSERT_TRUE(filter.forth());

				const FilterEntry& infoFilter = filter.getEntry();

				ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
				ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& infoList = infoFilter.getElementList();

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& name = infoList.getEntry();

				ASSERT_EQ(name.getName(), ENAME_NAME);
				ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& vendor = infoList.getEntry();

				ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
				ASSERT_EQ(vendor.getAscii(), "company");

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& capabilities = infoList.getEntry();
				ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
				ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

				const OmmArray& capabilitiesArray = capabilities.getArray();

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

				ASSERT_FALSE(capabilitiesArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& dictionariesProvided = infoList.getEntry();
				ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
				ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

				const OmmArray& dictionariesProvidedArray = capabilities.getArray();

				ASSERT_TRUE(dictionariesProvidedArray.forth());
				arrayEntry = &dictionariesProvidedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

				ASSERT_TRUE(dictionariesProvidedArray.forth());
				arrayEntry = &dictionariesProvidedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

				ASSERT_FALSE(dictionariesProvidedArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& dictionariesUsed = infoList.getEntry();
				ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
				ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

				const OmmArray& dictionariesUsedArray = capabilities.getArray();

				ASSERT_TRUE(dictionariesUsedArray.forth());
				arrayEntry = &dictionariesUsedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

				ASSERT_TRUE(dictionariesUsedArray.forth());
				arrayEntry = &dictionariesUsedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

				ASSERT_FALSE(dictionariesUsedArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& qos = infoList.getEntry();
				ASSERT_EQ(qos.getName(), ENAME_QOS);
				ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

				const OmmArray& qosArray = qos.getArray();

				ASSERT_TRUE(qosArray.forth());
				arrayEntry = &qosArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
				ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
				ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
				ASSERT_FALSE(qosArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& itemList = infoList.getEntry();
				ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
				ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(itemList.getAscii(), "ItemList");

				ASSERT_FALSE(infoList.forth());

				ASSERT_TRUE(filter.forth());

				const FilterEntry& stateFilter = filter.getEntry();

				ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
				ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& stateList = stateFilter.getElementList();

				ASSERT_TRUE(stateList.forth());

				const ElementEntry& state = stateList.getEntry();

				ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
				ASSERT_EQ(state.getUInt(), SERVICE_UP);

				ASSERT_FALSE(stateList.forth());

				ASSERT_FALSE(filter.forth());
			}

			EXPECT_TRUE(mapPayload.forth());

			{
				const MapEntry& mapEntry = mapPayload.getEntry();

				ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

				EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
				const FilterList& filter = mapEntry.getFilterList();
				ASSERT_TRUE(filter.forth());

				const FilterEntry& infoFilter = filter.getEntry();

				ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
				ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& infoList = infoFilter.getElementList();

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& name = infoList.getEntry();

				ASSERT_EQ(name.getName(), ENAME_NAME);
				ASSERT_EQ(name.getAscii(), "DIRECT_FEED_2");

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& vendor = infoList.getEntry();

				ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
				ASSERT_EQ(vendor.getAscii(), "company");

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& capabilities = infoList.getEntry();
				ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
				ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

				const OmmArray& capabilitiesArray = capabilities.getArray();

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

				ASSERT_TRUE(capabilitiesArray.forth());
				arrayEntry = &capabilitiesArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
				ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

				ASSERT_FALSE(capabilitiesArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& dictionariesProvided = infoList.getEntry();
				ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
				ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

				const OmmArray& dictionariesProvidedArray = capabilities.getArray();

				ASSERT_TRUE(dictionariesProvidedArray.forth());
				arrayEntry = &dictionariesProvidedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

				ASSERT_TRUE(dictionariesProvidedArray.forth());
				arrayEntry = &dictionariesProvidedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

				ASSERT_FALSE(dictionariesProvidedArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& dictionariesUsed = infoList.getEntry();
				ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
				ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

				const OmmArray& dictionariesUsedArray = capabilities.getArray();

				ASSERT_TRUE(dictionariesUsedArray.forth());
				arrayEntry = &dictionariesUsedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

				ASSERT_TRUE(dictionariesUsedArray.forth());
				arrayEntry = &dictionariesUsedArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

				ASSERT_FALSE(dictionariesUsedArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& qos = infoList.getEntry();
				ASSERT_EQ(qos.getName(), ENAME_QOS);
				ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

				const OmmArray& qosArray = qos.getArray();

				ASSERT_TRUE(qosArray.forth());
				arrayEntry = &qosArray.getEntry();
				ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
				ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
				ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
				ASSERT_FALSE(qosArray.forth());

				ASSERT_TRUE(infoList.forth());

				const ElementEntry& itemList = infoList.getEntry();
				ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
				ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
				ASSERT_EQ(itemList.getAscii(), "ItemList");

				ASSERT_FALSE(infoList.forth());

				ASSERT_TRUE(filter.forth());

				const FilterEntry& stateFilter = filter.getEntry();

				ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
				ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
				ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

				const ElementList& stateList = stateFilter.getElementList();

				ASSERT_TRUE(stateList.forth());

				const ElementEntry& state = stateList.getEntry();

				ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
				ASSERT_EQ(state.getUInt(), SERVICE_UP);

				ASSERT_FALSE(stateList.forth());

				ASSERT_FALSE(filter.forth());

			}
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. Both provdiers provide DIRECT_FEED, with mismatched QoS values
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has one active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchQoSDuringInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos(OmmQos::JustInTimeConflatedEnum, 2)			// Different rate, this should be closed.
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. Both provdiers provide DIRECT_FEED, with mismatched item name values
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has one active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consuemr gives a reponse with the correct DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchItemNameDuringInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "OtherItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. Both provdiers provide DIRECT_FEED, with mismatched QoS range flag values
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has one active channel and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consuemr gives a reponse with the correct DIRECT_FEED service information.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchQosRangeDuringInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			addUInt(ENAME_SUPPS_QOS_RANGE, 1).
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			addUInt(ENAME_SUPPS_QOS_RANGE, 0).
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qosRange = infoList.getEntry();
			ASSERT_EQ(qosRange.getName(), ENAME_SUPPS_QOS_RANGE);
			ASSERT_EQ(qosRange.getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(qosRange.getUInt(), 1);

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provdier provides DIRECT_FEED, the other provides a placeholder 
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consuemr gives a reponse with the correct DIRECT_FEED and placeholder service information.
 *  6. Provider two sends a directory update containing DIRECT_FEED with a mismatched QoS value
 *  7. Verify that no service update is sent and both channels are still connected.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchQoSAfterInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "placeholder", 4);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "placeholder");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());

		Map newService;
		newService.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos(OmmQos::JustInTimeConflatedEnum, 2)			// Different rate, this should be ignored
					.complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		UpdateMsg encodeUpdate;
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		// Do not close the connection, and no message should be sent to the consumer
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provdier provides DIRECT_FEED, the other provides a placeholder
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consuemr gives a reponse with the correct DIRECT_FEED and placeholder service information.
 *  6. Provider two sends a directory update containing DIRECT_FEED with a mismatched ItemName value
 *  7. Verify that no service update is sent and both channels are still connected.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchItemNameAfterInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "placeholder", 5);

	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "placeholder");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());

		Map newService;
		newService.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "OtherItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		UpdateMsg encodeUpdate;
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		// Do not close the connection, and no message should be sent to the consumer
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two provider
 *		a. One provdier provides DIRECT_FEED, the other provides a placeholder
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with service Id 1(DIRECT_FEED)
 *	5. Verify that the consuemr gives a reponse with the correct DIRECT_FEED and placeholder service information.
 *  6. Provider two sends a directory update containing DIRECT_FEED with a mismatched qos range value value
 *  7. Verify that no service update is sent and both channels are still connected.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryMismatchQosRangeAfterInit)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			addUInt(ENAME_SUPPS_QOS_RANGE, 1).
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "placeholder", 5);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		const OmmArrayEntry* arrayEntry;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);

		const Map& mapPayload = refreshMsg->getPayload().getMap();

		EXPECT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qosRange = infoList.getEntry();
			ASSERT_EQ(qosRange.getName(), ENAME_SUPPS_QOS_RANGE);
			ASSERT_EQ(qosRange.getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(qosRange.getUInt(), 1);

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_TRUE(mapPayload.forth());

		{
			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 2);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "placeholder");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& vendor = infoList.getEntry();

			ASSERT_EQ(vendor.getName(), ENAME_VENDOR);
			ASSERT_EQ(vendor.getAscii(), "company");

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& capabilities = infoList.getEntry();
			ASSERT_EQ(capabilities.getName(), ENAME_CAPABILITIES);
			ASSERT_EQ(capabilities.getLoadType(), DataType::ArrayEnum);

			const OmmArray& capabilitiesArray = capabilities.getArray();

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_DICTIONARY);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_MARKET_BY_PRICE);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYMBOL_LIST);

			ASSERT_TRUE(capabilitiesArray.forth());
			arrayEntry = &capabilitiesArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::UIntEnum);
			ASSERT_EQ(arrayEntry->getUInt(), MMT_SYSTEM);

			ASSERT_FALSE(capabilitiesArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesProvided = infoList.getEntry();
			ASSERT_EQ(dictionariesProvided.getName(), ENAME_DICTIONARYS_PROVIDED);
			ASSERT_EQ(dictionariesProvided.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesProvidedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesProvidedArray.forth());
			arrayEntry = &dictionariesProvidedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesProvidedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& dictionariesUsed = infoList.getEntry();
			ASSERT_EQ(dictionariesUsed.getName(), ENAME_DICTIONARYS_USED);
			ASSERT_EQ(dictionariesUsed.getLoadType(), DataType::ArrayEnum);

			const OmmArray& dictionariesUsedArray = capabilities.getArray();

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFFld");

			ASSERT_TRUE(dictionariesUsedArray.forth());
			arrayEntry = &dictionariesUsedArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(arrayEntry->getAscii(), "RWFEnum");

			ASSERT_FALSE(dictionariesUsedArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& qos = infoList.getEntry();
			ASSERT_EQ(qos.getName(), ENAME_QOS);
			ASSERT_EQ(qos.getLoadType(), DataType::ArrayEnum);

			const OmmArray& qosArray = qos.getArray();

			ASSERT_TRUE(qosArray.forth());
			arrayEntry = &qosArray.getEntry();
			ASSERT_EQ(arrayEntry->getLoadType(), DataType::QosEnum);
			ASSERT_EQ(arrayEntry->getQos().getTimeliness(), OmmQos::RealTimeEnum);
			ASSERT_EQ(arrayEntry->getQos().getRate(), OmmQos::TickByTickEnum);
			ASSERT_FALSE(qosArray.forth());

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& itemList = infoList.getEntry();
			ASSERT_EQ(itemList.getName(), ENAME_ITEM_LIST);
			ASSERT_EQ(itemList.getLoadType(), DataType::AsciiEnum);
			ASSERT_EQ(itemList.getAscii(), "ItemList");

			ASSERT_FALSE(infoList.forth());

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());
		}

		ASSERT_FALSE(mapPayload.forth());

		Map newService;
		newService.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addUInt(ENAME_SUPPS_QOS_RANGE, 0).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		UpdateMsg encodeUpdate;
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		// Do not close the connection, and no message should be sent to the consumer
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. Both providers are configured to not send a directory refresh
 *  2. Checks to see that the consumer does not intitialize
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryTimeoutBoth)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.sendDirectoryRefresh = false;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		try
		{
			OmmConsumer cons(consConfig, consClient);
			ASSERT_TRUE(false) << "Expected exception from consumer";
		}
		catch (const OmmInvalidUsageException&)
		{
			ASSERT_TRUE(true);
		}

		
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. One provider is configured to not send a directory refresh
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there one channel is active in the consumer
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryTimeoutOneNoResponse)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap;

	ProviderTestOptions provTestOptions2;
	provTestOptions2.sendDirectoryRefresh = false;
	provTestOptions2.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			EmaVector<ChannelInformation> channelInfo;

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with an invalid service name of "BADSERVICE"
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information with a status of CLOSED/SUSPECT
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryInvalidNameRequest)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions2.sendDirectoryRefresh = false;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		directoryRequest.domainType(MMT_DIRECTORY).serviceName("BADSERVICE");

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. One provider provides DIRECT_FEED and the other provider provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain with an invalid service Id of 135
 *	5. Verify that the consumer gives a reponse with the DIRECT_FEED service information with a status of CLOSED/SUSPECT
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryInvalidServiceIdRequest)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions2.sendDirectoryRefresh = false;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		directoryRequest.domainType(MMT_DIRECTORY).serviceId(135);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain and gets an OPEN/OK response for DIRECT_FEED
 *	5. One provider sends a delete action for DIRECT_FEED
 *  6. Verify that the consumer does not get any update, because DIRECT_FEED is still available.
 *  7. Other provider sends a delete action for DIRECT_FEED
 *  8. Verify that the consumer gets a delete action for DIRECT_FEED.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryDeleteService)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);
		{
			const Map& mapPayload = refreshMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");


			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& acceptingRequests = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_ACCEPTING_REQS);
			ASSERT_EQ(state.getUInt(), SERVICE_YES);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

			ASSERT_FALSE(mapPayload.forth());
		}

		Map encodeMap;

		encodeMap.addKeyUInt(2, MapEntry::DeleteEnum, FilterList()).complete();
		UpdateMsg encodeUpdate;
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// No message to user
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::UpdateMsgEnum);

		UpdateMsg* updateMsg = static_cast<UpdateMsg*>(msg);

		{
			const Map& mapPayload = updateMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			ASSERT_EQ(mapEntry.getAction(), MapEntry::DeleteEnum);
		}

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with a login client with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has two active channels and that it got a OPEN/OK login refresh message.
 *  4. Consumer registers for the directory domain and gets an OPEN/OK response for DIRECT_FEED
 *	5. One provider sends a status update with accepting requests set to off
 *  6. Verify that the consumer does not get any update
 *  7.  One provider sends a status update with accepting requests set to off
 *  8. Verify that the consumer gets an update action for DIRECT_FEED with accepting requests off
 *  9. One provider sends a status update with service state set to down
 *  10. Verify that the consumer does not get any update
 *  11.  One provider sends a status update with service state set to down
 *  12. Verify that the consumer gets an update action for DIRECT_FEED with accepting requests off and the service status set to down.
 */
TEST_F(OmmConsumerTest, RequestRoutingDirectoryStatusChange)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		Msg* msg;
		RefreshMsg* refreshMsg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}

		ReqMsg directoryRequest;
		directoryRequest.domainType(MMT_DIRECTORY);

		UInt64 dirHandle = cons.registerClient(directoryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_DIRECTORY);
		ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		ASSERT_TRUE(refreshMsg->getSolicited());
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_TRUE(refreshMsg->hasMsgKey());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::MapEnum);
		{
			const Map& mapPayload = refreshMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_EQ(infoFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(infoFilter.getFilterId(), SERVICE_INFO_FILTER);
			ASSERT_EQ(infoFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& infoList = infoFilter.getElementList();

			ASSERT_TRUE(infoList.forth());

			const ElementEntry& name = infoList.getEntry();

			ASSERT_EQ(name.getName(), ENAME_NAME);
			ASSERT_EQ(name.getAscii(), "DIRECT_FEED");


			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& acceptingRequests = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_ACCEPTING_REQS);
			ASSERT_EQ(state.getUInt(), SERVICE_YES);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

			ASSERT_FALSE(mapPayload.forth());
		}

		Map encodeMapNoAccept;

		encodeMapNoAccept.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO).
				complete()).
			complete()).complete();
		UpdateMsg encodeUpdate;
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapNoAccept);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// No message to user
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::UpdateMsgEnum);

		// Watchlist will send the message indicating that things have changed
		UpdateMsg* updateMsg = static_cast<UpdateMsg*>(msg);

		{
			const Map& mapPayload = updateMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_UP);

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& acceptingReq = stateList.getEntry();

			ASSERT_EQ(acceptingReq.getName(), ENAME_ACCEPTING_REQS);
			ASSERT_EQ(acceptingReq.getUInt(), SERVICE_NO);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

			ASSERT_FALSE(mapPayload.forth());
		}

		Map encodeMapDown;

		encodeMapDown.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_DOWN).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO).
				complete()).
			complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapDown);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// No message to user
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::UpdateMsgEnum);

		updateMsg = static_cast<UpdateMsg*>(msg);

		{
			const Map& mapPayload = updateMsg->getPayload().getMap();

			EXPECT_TRUE(mapPayload.forth());

			const MapEntry& mapEntry = mapPayload.getEntry();

			ASSERT_EQ(mapEntry.getKey().getUInt(), 1);

			EXPECT_EQ(mapEntry.getLoadType(), DataType::FilterListEnum);
			const FilterList& filter = mapEntry.getFilterList();
			ASSERT_TRUE(filter.forth());

			const FilterEntry& infoFilter = filter.getEntry();

			ASSERT_TRUE(filter.forth());

			const FilterEntry& stateFilter = filter.getEntry();

			ASSERT_EQ(stateFilter.getAction(), FilterEntry::SetEnum);
			ASSERT_EQ(stateFilter.getFilterId(), SERVICE_STATE_FILTER);
			ASSERT_EQ(stateFilter.getLoadType(), DataType::ElementListEnum);

			const ElementList& stateList = stateFilter.getElementList();

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& state = stateList.getEntry();

			ASSERT_EQ(state.getName(), ENAME_SVC_STATE);
			ASSERT_EQ(state.getUInt(), SERVICE_DOWN);

			ASSERT_TRUE(stateList.forth());

			const ElementEntry& acceptingReq = stateList.getEntry();

			ASSERT_EQ(acceptingReq.getName(), ENAME_ACCEPTING_REQS);
			ASSERT_EQ(acceptingReq.getUInt(), SERVICE_NO);

			ASSERT_FALSE(stateList.forth());

			ASSERT_FALSE(filter.forth());

			ASSERT_FALSE(mapPayload.forth());
		}

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with two providers
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Request an RWFFld dictionary with no associated service
 *  5. Verify that the RWFFld refresh is received.
 *  6. Request an RWFEnum dictionary with no associated service
 *  7. Verify that the RWFEnum refresh is received
 */
TEST_F(OmmConsumerTest, RequestRoutingDictionaryRequest)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).adminControlDictionary(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).adminControlDictionary(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelDownloadDict");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		// Login, directory, RWFFld and Enum dictionaries
		ASSERT_EQ(provClient1.getMessageQueueSize(), 4);
		provClient1.clear();
		// Login and directory
		ASSERT_EQ(provClient2.getMessageQueueSize(), 2);
		provClient2.clear();

		ReqMsg dictionaryRequest;
		dictionaryRequest.domainType(MMT_DICTIONARY).name("RWFFld").filter(DICTIONARY_NORMAL);

		UInt64 dirHandle = cons.registerClient(dictionaryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		RefreshMsg* refresh = static_cast<RefreshMsg*>(consClient.popMsg());

		ASSERT_EQ(refresh->getName(), "RWFFld");
		ASSERT_TRUE(refresh->getComplete());
		ASSERT_TRUE(refresh->getClearCache());

		dictionaryRequest.clear();
		dictionaryRequest.domainType(MMT_DICTIONARY).name("RWFEnum").filter(DICTIONARY_NORMAL);

		dirHandle = cons.registerClient(dictionaryRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		refresh = static_cast<RefreshMsg*>(consClient.popMsg());

		ASSERT_EQ(refresh->getName(), "RWFEnum");
		ASSERT_TRUE(refresh->getComplete());
		ASSERT_TRUE(refresh->getClearCache());

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a request routing consumer with two providers, using the basic config in the EmaConfigTest.xml options
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Request an RWFFld dictionary on the DIRECT_FEED service
 *  5. Verify that the RWFFld refresh is received.
 *  6. Request an RWFEnum dictionary on the DIRECT_FEED service
 *  7. Verify that the RWFEnum refresh is received
 */
TEST_F(OmmConsumerTest, RequestRoutingDictionaryRequestByServiceName)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelDownloadDict");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		// Just Login
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		provClient1.clear();
		// Login
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		provClient2.clear();

		ReqMsg dictionaryRequest;
		dictionaryRequest.domainType(MMT_DICTIONARY).name("RWFFld").filter(DICTIONARY_NORMAL).serviceName("DIRECT_FEED");

		UInt64 dirHandle = cons.registerClient(dictionaryRequest, consClient);

		testSleep(5000);

		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_GE(consClient.getMessageQueueSize(), 1u);

		RefreshMsg* refresh = static_cast<RefreshMsg*>(consClient.popMsg());

		ASSERT_EQ(refresh->getName(), "RWFFld");
		ASSERT_TRUE(refresh->getClearCache());

		dictionaryRequest.clear();
		dictionaryRequest.domainType(MMT_DICTIONARY).name("RWFEnum").filter(DICTIONARY_NORMAL).serviceName("DIRECT_FEED");

		consClient.clear();

		dirHandle = cons.registerClient(dictionaryRequest, consClient);

		testSleep(5000);

		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_GE(consClient.getMessageQueueSize(), 1u);

		refresh = static_cast<RefreshMsg*>(consClient.popMsg());

		ASSERT_EQ(refresh->getName(), "RWFEnum");
		ASSERT_TRUE(refresh->getClearCache());

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with service list SVG1 registered and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. SVG1 consists of two non-existent services
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests items with bad service names, service ides, an invalid service list, and the service list above
 *  5. Verifies that the consumer receives CLOSED/SUSPECT status messages for these requests
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestUnknownServiceNameServiceIdServiceList)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.closeItemRequest = true;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions1.closeItemRequest = true;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("badService1");
	serviceList.addService("badService2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);


	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("BAD_REQUEST");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "BAD_REQUEST");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		consRequest.clear().name("Item2").domainType(MMT_MARKET_PRICE).serviceId(50);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item2");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		provClient1.clear();
		provClient2.clear();

		consRequest.clear().name("Item3").domainType(MMT_MARKET_PRICE).serviceListName("BadServiceList");

		UInt64 handle3 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item3");
		ASSERT_EQ(statusMsg->getServiceName(), "BadServiceList");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		provClient1.clear();
		provClient2.clear();

		consRequest.clear().name("Item4").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		UInt64 handle4 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item4");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with service list SVG1 registered and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. SVG1 consists of DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests items on DIRECT_FEED and with SVG1 with QoS values that do not match what's provided for DIRECT_FEED or DIRECT_FEED_2
 *  5. Verifies that the consumer receives OPEN/SUSPECT status messages for these requests
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestUnknownQosServiceNameServiceIdServiceList)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.closeItemRequest = true;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions1.closeItemRequest = true;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);


	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").qos(123, 123);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		consRequest.clear().name("Item2").domainType(MMT_MARKET_PRICE).serviceId(2).qos(123, 123);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item2");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		consRequest.clear().name("Item3").domainType(MMT_MARKET_PRICE).serviceListName("SVG1").qos(123, 123);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle3 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item3");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests the same item twice on DIRECT_FEED
 *  5. Verify that one request goes out
 *  6. Verify that the consumer gets two refreshes
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSameItemTwice)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);
		UInt64 handle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED and the other provdies DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests the one item on DIRECT_FEED
 *  5. Verify that one request goes out to the correct provider
 *  6. Verify that the consumer gets the refresh
 *  7. Requests the one item on DIRECT_FEED_2
 *  8. Verify that one request goes out to the correct provider
 *  9. Verify that the consumer gets the refresh
 *  10. Requests the one item on service id 1(DIRECT_FEED)
 *  11. Verify that one request goes out to the correct provider
 *  12. Verify that the consumer gets the refresh
 *  13. Requests the one item on service id 2(DIRECT_FEED_2)
 *  14. Verify that one request goes out to the correct provider
 *  15. Verify that the consumer gets the refresh
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItem)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().name("Item2").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED_2");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item2");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item2");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().name("Item3").domainType(MMT_MARKET_PRICE).serviceId(1);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle3 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item3");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item3");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().name("Item4").domainType(MMT_MARKET_PRICE).serviceId(2);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle4 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item4");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item4");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED and the other provdies DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a batch of two items on DIRECT_FEED
 *  5. Verify that one request goes out to the correct provider
 *  6. Verify that the consumer gets the refreshes
 *  7. Requests a batch of two items on DIRECT_FEED_2
 *  8. Verify that one request goes out to the correct provider
 *  9. Verify that the consumer gets the refreshes
 *  10. Requests a batch of two items on service id 1(DIRECT_FEED)
 *  11. Verify that one request goes out to the correct provider
 *  12. Verify that the consumer gets the refreshes
 *  13. Requests a batch of two items on service id 2(DIRECT_FEED_2)
 *  14. Verify that one request goes out to the correct provider
 *  15. Verify that the consumer gets the refreshes
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestBatchItems)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray().addAscii("Item1").addAscii("Item2").complete()).complete());

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item2");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 3);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_TRUE(statusMsg->hasState());

		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item2");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED_2").payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray().addAscii("Item3").addAscii("Item4").complete()).complete());

		provClient1.clear();
		provClient2.clear();

		UInt64 handle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 2);
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item3");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item4");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 3);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_TRUE(statusMsg->hasState());

		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item3");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item4");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().domainType(MMT_MARKET_PRICE).serviceId(1).payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray().addAscii("Item5").addAscii("Item6").complete()).complete());

		provClient1.clear();
		provClient2.clear();

		UInt64 handle3 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item5");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item6");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 3);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_TRUE(statusMsg->hasState());

		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item5");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item6");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().domainType(MMT_MARKET_PRICE).serviceId(2).payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray().addAscii("Item7").addAscii("Item8").complete()).complete());

		provClient1.clear();
		provClient2.clear();

		UInt64 handle4 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 2);
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item7");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item8");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 3);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_TRUE(statusMsg->hasState());

		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item7");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item8");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. First provider is configured to not send any item refreshes, the second is configured to send them
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Waits for 5 seconds(request timeout is 2 seconds)
 *  6. Expects the following after the sleep:
 *		1. A request for the item on both providers
 *		2. A close message(indicated by a stored reqMsg) in the 1st provider
 *		2. The consumer has the following:
 *			1. A OPEN/SUSPECT status message indicating the first timeout
 *			2. A refresh from the second provider
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRecoveryAfterTimeout)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.sendItemRefresh = false;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap1;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		// Check to see that the request has been made to both providers
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// This is from the close message
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider sends a service down update with a service status of OPEN/SUSPECT
 *  7. 1st provider gets a close request 
 *  8. 2nd provider gets a request
 *  9. Consumer gets an OPEN/SUSPECT status
 *  10. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRecoveryAfterServiceDown)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		Map encodeMapDown;

		encodeMapDown.addKeyUInt(2, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_SVC_STATE, SERVICE_DOWN)
				.addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO)
				.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::SuspectEnum, OmmState::NoneEnum)
				.complete())
			.complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapDown);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		// This is the close message
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		// This is the close message
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// One message to the user
		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRerouteAfterChannelDownEnhancedRecoveryOn)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b.Consumer is configured with enhanced recovery off
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets nothing
 *  8. 1st provider gets a full reconnection, with item request
 *  9. Consumer gets an OPEN/SUSPECT status
 *  10. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRerouteAfterChannelDownEnhancedRecoveryOff)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelEnhancedOff");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		// Wait for reconnection and re-request
		testSleep(2000);

		// Check to see that the requests have been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 3);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Consumer is configured to not do any reconnection attempts
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRerouteAfterChannelClosedEnhancedRecoveryOn)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");
	
	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelNoReconnect");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		testSleep(1000);

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Consumer is configured to not do any reconnection attempts
 *		c. Consumer is configured with enhanced recovery off
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemRerouteAfterChannelClosedEnhancedRecoveryOff)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelEnhancedOffNoReconnect");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		// Wait for reconnection and re-request
		testSleep(2000);

		// Only one channel should be up
		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		// Check to see that the requests have been made to prov1
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Providers are configured to send CLOSED/SUSPECT statuses for requested items
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED
 *  6. Both providers get request, send CLOSED/SUSPECT
 *  7. Consumer gets an OPEN/SUSPECT status
 *  8. Consumer gets a CLOSED/SUSPECT status
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemBothCloseSuspect)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.closeItemRequest = true;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions2.closeItemRequest = true;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(),2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *			a. One provider provides DIRECT_FEED without market price, the other provider has DIRECT_FEED with market price
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Provider 1 does not get any request
 *  6. Provider 2 gets the request, sends a response
 *  7. Consumer gets a refresh
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemCapabilitiesOnDifferentServers)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	Map serviceMap2;
	serviceMap2.addKeyUInt(3, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 3);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Both providers provider an itemGroup of id "10"
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Provider 1 gets the request
 *  6. Consumer gets a refresh
 *  7. Provider 1 sends out a group status for id "10" with a state of CLOSEDRECOVER/SUSPECT
 *  8. Provider 2 gets the item request
 *  9. Consumer gets a status of OPEN/SUSPECT
 *  10. Consumer gets a refresh from provider 2
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemGroupCloseRecover)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.groupId.setFrom("10", 2);

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		msg = consClient.popMsg();

		Map newService;
		newService.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_GROUP_ID, FilterEntry::SetEnum, ElementList().
				addBuffer(ENAME_GROUP, EmaBuffer("10", 2)).
				addState(ENAME_STATUS, OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Group Status").
				complete()).
			complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED_2 with the market price domain
 *  5. Verify no requests go out
 *  6. Consumer gets an OPEN/SUSPECT status message
 *  7. On Provider 1, send a directory update containing the DIRECT_FEED_2 service
 *  8. Provider 1 gets the item request
 *  9. Consumer gets an item refresh
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemBringServiceUpAfterRequest)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED_2");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has not been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		Map newService;
		setupSingleServiceMap(newService, "DIRECT_FEED_2", 3);

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 3);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Provider 1 gets the request
 *  6. Consumer gets a refresh
 *  7. Provider 1 sends out a status for the item with a state of CLOSEDRECOVER/SUSPECT
 *  8. Provider 2 gets the item request
 *  9. Consumer gets a status of OPEN/SUSPECT
 *  10. Consumer gets a refresh from provider 2
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemItemCloseRecover)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.groupId.setFrom("10", 2);

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		StatusMsg encodeStatusMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		ASSERT_EQ(provClient1.activeRequests.size(), 1);

		encodeStatusMsg.domainType(MMT_MARKET_PRICE).state(OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Item temporarily closed");

		prov1.submit(encodeStatusMsg, provClient1.activeRequests[0].handle);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Provider 1 gets the request
 *  6. Consumer gets a refresh
 *  7. Provider 1 and 2 send out statues indicating that DIRECT_FEED is not accepting requests
 *  8. Verify Consumer does not get anything
 *  9. Provider 1 sends out a CLOSEDRECOVER/SUSPECT status on the item
 *  10. Consumer gets 2 OPEN/SUSPECT status messages
 *  11. Unregister the request on the consumer
 *  12. Provider 1 sends a directory update indicating that DIRECT_FEED is now accepting requests
 *  13. Verify that nothing is sent out after this.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemUnsubscribeDuringRecovery)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		StatusMsg encodeStatusMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		ASSERT_EQ(provClient1.activeRequests.size(), 1);

		Map encodeMapNoAccept;

		encodeMapNoAccept.addKeyUInt(2, MapEntry::UpdateEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO).
				complete()).
			complete()).complete();

		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapNoAccept);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// No message to user
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);


		ASSERT_EQ(provClient1.activeRequests.size(), 1);
		ASSERT_EQ(provClient2.activeRequests.size(), 0);

		encodeStatusMsg.domainType(MMT_MARKET_PRICE).state(OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Item temporarily closed");
		prov1.submit(encodeStatusMsg, provClient1.activeRequests[0].handle);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		cons.unregister(handle1);

		Map encodeMapAccept;

		encodeMapAccept.addKeyUInt(2, MapEntry::UpdateEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMapAccept);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Provider 1 gets the request
 *  6. Consumer gets a refresh
 *  7. Provider 1 and 2 send out statues indicating that DIRECT_FEED is not accepting requests
 *  8. Verify Consumer does not get anything
 *  9. Provider 1 sends out a CLOSEDRECOVER/SUSPECT status on the item
 *  10. Consumer gets 2 OPEN/SUSPECT status messages
 *  11. Provider 1 sends a directory update indicating that DIRECT_FEED is now accepting requests
 *  12. Verify that the consumer sends a request to provider 1 and that the consumer gets a refresh.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemClosedRecoverableAndRecoverAfterServiceAcceptingRequests)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		StatusMsg encodeStatusMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		ASSERT_EQ(provClient1.activeRequests.size(), 1);

		Map encodeMapNoAccept;

		encodeMapNoAccept.addKeyUInt(2, MapEntry::UpdateEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO).
				complete()).
			complete()).complete();

		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapNoAccept);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// No message to user
		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);


		ASSERT_EQ(provClient1.activeRequests.size(), 1);
		ASSERT_EQ(provClient2.activeRequests.size(), 0);

		encodeStatusMsg.domainType(MMT_MARKET_PRICE).state(OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Item temporarily closed");
		prov1.submit(encodeStatusMsg, provClient1.activeRequests[0].handle);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		Map encodeMapAccept;

		encodeMapAccept.addKeyUInt(2, MapEntry::UpdateEnum, FilterList().
			add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMapAccept);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED, the other provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain with a different QoS
 *  5. Verify that the consumer gets an OPEN/SUSPECT status
 *  6. Send a DELETE action on DIRECT_FEED 
 *  7. Send a directory update on Provider 1 with DIRECT_FEED that contains a matching QoS with the request
 *  8. Verify that the consumer sends a request to Provider 1
 *  9. Verify that the consumer gets a refresh.  
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemWithUnmatchedQoSThenServiceProvideTheRequestedQoS)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").qos(100, 100);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that no request has been sent
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		// Send delete action for DIRECT_FEED
		Map encodeMap;

		encodeMap.addKeyUInt(2, MapEntry::DeleteEnum, FilterList()).complete();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 0);

		// Send new version of DIRECT_FEED with qos in it

		encodeMap.clear().addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_DICTIONARY).
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					addUInt(MMT_SYSTEM).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					addQos(100, 100).
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);
		ASSERT_EQ(reqMsg->getQosRate(), 100);
		ASSERT_EQ(reqMsg->getQosRate(), 100);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getQos().getRate(), 100);
		ASSERT_EQ(refreshMsg->getQos().getTimeliness(), 100);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED without MARKET_PRICE capability, the other provides DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Verify that the consumer gets an OPEN/SUSPECT status
 *  6. Send a DELETE action on DIRECT_FEED
 *  7. Send a directory update on Provider 1 with DIRECT_FEED that contains the MARKET_PRICE capability
 *  8. Verify that the consumer sends a request to Provider 1
 *  9. Verify that the consumer gets a refresh.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos().
				complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED_2").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos().
				complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that no request has been sent
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		// Send delete action for DIRECT_FEED
		Map encodeMap;

		encodeMap.addKeyUInt(2, MapEntry::DeleteEnum, FilterList()).complete();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 0);

		// Send new version of DIRECT_FEED with qos in it

		encodeMap.clear().addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_DICTIONARY).
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					addUInt(MMT_SYSTEM).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers have DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Verify that provider 1 got the request
 *  6. Verify that the consumer gets a refresh
 *  7. Send a post on the consumer item's handle with the serviceName DIRECT_FEED
 *  8. Verify that Provider 1 gets it.
 *  9. Send a post on the consumer item's handle with the service Id of 1, and ack requested
 *  10. Verify that Provider 1 gets it
 *  11. Verify that the provider sent an ack.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemOnStreamPosting)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;
		PostMsg encodePostMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		PostMsg* postMsg;
		AckMsg* ackMsg;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		encodePostMsg.postId(1).serviceName("DIRECT_FEED").name("TestItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		cons.submit(encodePostMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), 2);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		encodePostMsg.clear().postId(2).serviceId(1).name("TestItem").solicitAck(true).payload(encodeInnerUpdate).complete();

		cons.submit(encodePostMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::AckMsgEnum);
		ackMsg = static_cast<AckMsg*>(msg);
		ASSERT_EQ(ackMsg->getName(), "TestItem");
		ASSERT_EQ(ackMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(ackMsg->getServiceId(), 1);
		ASSERT_EQ(ackMsg->getAckId(), 2);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers have DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Verify that provider 1 got the request
 *  6. Verify that the consumer gets a refresh
 *  7. Send a post on the consumer item's handle with an invalid service name "Baddirectory"
 *  8. Verify that an OmmInvalidUsageException was thrown
 *  9. Send a post on the consumer item's handle with an invalid service id number
 *  10. Verify that an OmmInvalidUsageException was thrown
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemInvalidOnStreamPosting)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;
		PostMsg encodePostMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		encodePostMsg.postId(1).serviceName("BADSERVICE").name("TestItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		try
		{
			cons.submit(encodePostMsg, handle1);
			ASSERT_TRUE(false) << "Exception not thrown";
		}
		catch (const OmmInvalidUsageException&)
		{
			ASSERT_TRUE(true);
		}

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);


		encodePostMsg.clear().postId(2).serviceId(50).name("TestItem").solicitAck(true).payload(encodeInnerUpdate).complete();

		try
		{
			cons.submit(encodePostMsg, handle1);
			ASSERT_TRUE(false) << "Exception not thrown";
		}
		catch (const OmmInvalidUsageException&)
		{
			ASSERT_TRUE(true);
		}
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers have DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *	4. Requests a LOGIN item for the handle
 *  5. Verify that the consumer gets a refresh
 *  6. Send a post on the consumer item's handle with the serviceName DIRECT_FEED
 *  7. Verify that both Providers get it
 *  8. Send a post on the consumer item's handle with the service Id of 1, and ack requested
 *  9. Verify that both providers get it
 *  10. Verify that both providers sent an ack.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemOffStreamPosting)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 3);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;
		PostMsg encodePostMsg;
		Msg* msg;
		RefreshMsg* refreshMsg;
		PostMsg* postMsg;
		AckMsg* ackMsg;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.domainType(MMT_LOGIN);

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);


		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);

		encodePostMsg.postId(1).serviceName("DIRECT_FEED").name("TestItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		cons.submit(encodePostMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), 2);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), 3);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		provClient1.clear();
		provClient2.clear();

		encodePostMsg.clear().postId(2).serviceId(1).name("TestItem").solicitAck(true).payload(encodeInnerUpdate).complete();

		cons.submit(encodePostMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), 2);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), 3);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::AckMsgEnum);
		ackMsg = static_cast<AckMsg*>(msg);
		ASSERT_EQ(ackMsg->getName(), "TestItem");
		ASSERT_EQ(ackMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(ackMsg->getServiceId(), 1);
		ASSERT_EQ(ackMsg->getAckId(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::AckMsgEnum);
		ackMsg = static_cast<AckMsg*>(msg);
		ASSERT_EQ(ackMsg->getName(), "TestItem");
		ASSERT_EQ(ackMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(ackMsg->getServiceId(), 1);
		ASSERT_EQ(ackMsg->getAckId(), 2);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers have DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a LOGIN item for the handle
 *  5. Verify that the consumer gets a refresh
 *  6. Send a post on the consumer item's handle with an invalid service name "Baddirectory"
 *  7. Verify that an OmmInvalidUsageException was thrown
 *  8. Send a post on the consumer item's handle with an invalid service id number
 *  9. Verify that an OmmInvalidUsageException was thrown
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemInvalidOffStreamPosting)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;
		PostMsg encodePostMsg;
		Msg* msg;
		RefreshMsg* refreshMsg;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.domainType(MMT_LOGIN);

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);


		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);

		encodePostMsg.postId(1).serviceName("BADSERVICE").name("TestItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		try
		{
			cons.submit(encodePostMsg, handle1);
			ASSERT_TRUE(false) << "Exception not thrown";
		}
		catch (const OmmInvalidUsageException&)
		{
			ASSERT_TRUE(true);
		}

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);


		encodePostMsg.clear().postId(2).serviceId(50).name("TestItem").solicitAck(true).payload(encodeInnerUpdate).complete();

		try
		{
			cons.submit(encodePostMsg, handle1);
			ASSERT_TRUE(false) << "Exception not thrown";
		}
		catch (const OmmInvalidUsageException&)
		{
			ASSERT_TRUE(true);
		}
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a consumer and with two single providers
 *		a. Provider 1 provides DIRECT_FEED, provider 2 provides DIRECT_FEED_2
 *		b. Consumer has a service list SVG1 with services "BADSERVICE" and "DIRECT_FEED"
 *		c. Consumer has a service list SVG2 with services "DIRECT_FEED" and "DIRECT_FEED_2"
 *		d. Consumer has a service list SVG3 with services "DIRECT_FEED" and "DIRECT_FEED_2"
 *  2. Checks to see that the consumer does intitialize
 *  3. Requests an item with service list name "SVG1"
 *  4. Verifies provider 2 gets a request
 *  5. Verifies that the consumer gets a response with service name SVG1
 *	6. Requests an item with service list name "SVG2"
 *  7. Verifies provider 1 gets a request
 *  8. Verifies that the consumer gets a response with service name SVG2
 *  9. Requests an item with service list name "SVG1"
 *  10. Verifies provider 1 gets a request
 *  11. Verifies that the consumer gets a response with service name SVG3
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceList)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000").adminControlDirectory(OmmIProviderConfig::UserControlEnum);

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 4);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.providerName("TestProvider_15001").adminControlDirectory(OmmIProviderConfig::UserControlEnum);

	ServiceList serviceList("SVG1");
	serviceList.addService("BADSERVICE");
	serviceList.addService("DIRECT_FEED_2");

	ServiceList serviceList2("SVG2");
	serviceList2.addService("DIRECT_FEED");
	serviceList2.addService("DIRECT_FEED_2");

	ServiceList serviceList3("SVG3");
	serviceList3.addService("DIRECT_FEED_2");
	serviceList3.addService("DIRECT_FEED");


	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList).addServiceList(serviceList2).addServiceList(serviceList3);


	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		// Login
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 2);
		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		UInt64 consHandle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 4);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().name("Item2").domainType(MMT_MARKET_PRICE).serviceListName("SVG2");

		UInt64 consHandle2 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item2");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item2");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG2");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		consRequest.clear().name("Item3").domainType(MMT_MARKET_PRICE).serviceListName("SVG3");

		UInt64 consHandle3 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item3");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 4);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item3");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG3");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider sends a service down update with a service status of OPEN/SUSPECT
 *  7. 1st provider gets a close request
 *  8. 2nd provider gets a request
 *  9. Consumer gets an OPEN/SUSPECT status
 *  10. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRecoveryAfterServiceDown)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		Map encodeMapDown;

		encodeMapDown.addKeyUInt(2, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_SVC_STATE, SERVICE_DOWN)
				.addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO)
				.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::SuspectEnum, OmmState::NoneEnum)
				.complete())
			.complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapDown);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		// This is the close message
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov2.submit(encodeUpdate, 0);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		// This is the close message
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// One message to the user
		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRerouteAfterChannelDownEnhancedRecoveryOn)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets nothing
 *  8. 1st provider gets a full reconnection, with item request
 *  9. Consumer gets an OPEN/SUSPECT status
 *  10. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRerouteAfterChannelDownEnhancedRecoveryOff)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelEnhancedOff").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		// Wait for reconnection and re-request
		testSleep(2000);

		// Check to see that the requests have been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 3);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Consumer is configured to not do any reconnection attempts
 *		c. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRerouteAfterChannelClosedEnhancedRecoveryOn)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelNoReconnect").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		testSleep(1000);
		// Check to see that there's only one channel left
		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Consumer is configured to not do any reconnection attempts
 *		c. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *		d. Consumer is configured with enhanced recovery off
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  5. Gets the refresh from the 1st provider
 *  6. 1st provider closes it's connection to the consumer
 *  7. 2nd provider gets a request
 *  8. Consumer gets an OPEN/SUSPECT status
 *  9. Consumer gets a refresh from 2nd provider.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRerouteAfterChannelClosedEnhancedRecoveryOff)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2ChannelEnhancedOffNoReconnect").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		// Wait for reconnection and re-request
		testSleep(2000);

		// Check to see that there's only one channel left
		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 1);

		// Check to see that the requests have been made to prov2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Provider 1 provides DIRECT_FEED and Provider 2 provides DIRECT_FEED_2
 *		b. Providers are configured to send CLOSED/SUSPECT statuses for requested items
 *		c. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list SVG1
 *  6. Both providers get request, send CLOSED/SUSPECT
 *  7. Consumer gets an OPEN/SUSPECT status
 *  8. Consumer gets a CLOSED/SUSPECT status
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListBothCloseSuspect)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.closeItemRequest = true;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;
	provTestOptions2.closeItemRequest = true;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Consumer has a service list "SVG1" with services badservice1, badservice2, and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1 with the market price domain
 *  5. Verify no requests go out
 *  6. Consumer gets an OPEN/SUSPECT status message
 *  7. On Provider 1, send a directory update containing the DIRECT_FEED_2 service
 *  8. Provider 1 gets the item request
 *  9. Consumer gets an item refresh
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListBringServiceUpAfterRequest)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("badService1");
	serviceList.addService("badService2");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has not been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		Map newService;
		setupSingleServiceMap(newService, "DIRECT_FEED_2", 3);

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 3);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. First provider is configured to not send any item refreshes, the second is configured to send them
 *		c. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1
 *  5. Waits for 5 seconds(request timeout is 2 seconds)
 *  6. Expects the following after the sleep:
 *		1. A request for the item on both providers(DIRECT_FEED on 1st provider, DIRECT_FEED_2 on 2nd provider)
 *		2. A close message(indicated by a stored reqMsg) in the 1st provider
 *		2. The consumer has the following:
 *			1. A OPEN/SUSPECT status message indicating the first timeout
 *			2. A refresh from the second provider
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListRecoveryAfterTimeout)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.sendItemRefresh = false;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		// Check to see that the request has been made to both providers
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// This is from the close message
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Both providers provider an itemGroup of id "10"
 *		c. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1 with the market price domain
 *  5. Provider 1 gets the request on DIRECT_FEED
 *  6. Consumer gets a refresh
 *  7. Provider 1 sends out a group status for id "10" with a state of CLOSEDRECOVER/SUSPECT
 *  8. Provider 2 gets the item request on DIRECT_FEED_2
 *  9. Consumer gets a status of OPEN/SUSPECT
 *  10. Consumer gets a refresh from provider 2
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListGroupCloseRecover)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.groupId.setFrom("10", 2);

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		Map newService;
		newService.addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_GROUP_ID, FilterEntry::SetEnum, ElementList().
				addBuffer(ENAME_GROUP, EmaBuffer("10", 2)).
				addState(ENAME_STATUS, OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Group Status").
				complete()).
			complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(newService);
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers provide DIRECT_FEED
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1 with the market price domain
 *  5. Provider 1 gets the request on DIRECT_FEED
 *  6. Consumer gets a refresh
 *  7. Provider 1 sends out a status for the item with a state of CLOSEDRECOVER/SUSPECT
 *  8. Provider 2 gets the item request on DIRECT_FEED_2
 *  9. Consumer gets a status of OPEN/SUSPECT
 *  10. Consumer gets a refresh from provider 2
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListItemCloseRecover)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.groupId.setFrom("10", 2);

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		StatusMsg encodeStatusMsg;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		ASSERT_EQ(provClient1.activeRequests.size(), 1);

		encodeStatusMsg.domainType(MMT_MARKET_PRICE).state(OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Item temporarily closed");

		prov1.submit(encodeStatusMsg, provClient1.activeRequests[0].handle);

		testSleep(1000);
		// Check to see that the request has been made to prov2
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED_2");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Two messages to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED without MARKET_PRICE capability, the other provides DIRECT_FEED_2 without MARKET_PRICE capability
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1 with the market price domain
 *  5. Verify that the consumer gets an OPEN/SUSPECT status
 *  6. Send a DELETE action on DIRECT_FEED
 *  7. Send a directory update on Provider 1 with DIRECT_FEED that contains the MARKET_PRICE capability
 *  8. Verify that the consumer sends a request to Provider 1
 *  9. Verify that the consumer gets a refresh.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListWithUnmatchedCapabilityThenServiceProvideTheRequestedCapability)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos().
				complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED_2").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos().
				complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that no request has been sent
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		// Send delete action for DIRECT_FEED
		Map encodeMap;

		encodeMap.addKeyUInt(2, MapEntry::DeleteEnum, FilterList()).complete();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 0);

		// Send new version of DIRECT_FEED with qos in it

		encodeMap.clear().addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_DICTIONARY).
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					addUInt(MMT_SYSTEM).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. One provider provides DIRECT_FEED, the other provides DIRECT_FEED_2
 *		b. Consumer has a service list "SVG1" with services DIRECT_FEED and DIRECT_FEED_2
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on service list name SVG1 with the market price domain with a different QoS
 *  5. Verify that the consumer gets an OPEN/SUSPECT status
 *  6. Send a DELETE action on DIRECT_FEED
 *  7. Send a directory update on Provider 1 with DIRECT_FEED that contains a matching QoS with the request
 *  8. Verify that the consumer sends a request to Provider 1
 *  9. Verify that the consumer gets a refresh.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestServiceListWithUnmatchedQoSThenServiceProvideTheRequestedQoS)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED_2", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ServiceList serviceList("SVG1");
	serviceList.addService("DIRECT_FEED");
	serviceList.addService("DIRECT_FEED_2");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel").addServiceList(serviceList);

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1").qos(100, 100);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that no request has been sent
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "SVG1");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		// Send delete action for DIRECT_FEED
		Map encodeMap;

		encodeMap.addKeyUInt(2, MapEntry::DeleteEnum, FilterList()).complete();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 0);

		// Send new version of DIRECT_FEED with qos in it

		encodeMap.clear().addKeyUInt(2, MapEntry::AddEnum, FilterList().
			add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
				addAscii(ENAME_NAME, "DIRECT_FEED").
				addAscii(ENAME_VENDOR, "company").
				addArray(ENAME_CAPABILITIES, OmmArray().
					addUInt(MMT_DICTIONARY).
					addUInt(MMT_MARKET_PRICE).
					addUInt(MMT_MARKET_BY_PRICE).
					addUInt(MMT_SYMBOL_LIST).
					addUInt(MMT_SYSTEM).
					complete()).
				addArray(ENAME_QOS, OmmArray().
					addQos().
					addQos(100, 100).
					complete()).
				addArray(ENAME_DICTIONARYS_USED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
					addAscii("RWFFld").
					addAscii("RWFEnum").
					complete()).
				addAscii(ENAME_ITEM_LIST, "ItemList").
				complete()).
			add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
				addUInt(ENAME_SVC_STATE, SERVICE_UP).
				addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
				complete()).
			complete()).complete();

		encodeUpdate.clear().domainType(MMT_DIRECTORY).payload(encodeMap);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(500);

		// Check to see nothing more
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);
		ASSERT_EQ(reqMsg->getQosRate(), 100);
		ASSERT_EQ(reqMsg->getQosRate(), 100);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getQos().getRate(), 100);
		ASSERT_EQ(refreshMsg->getQos().getTimeliness(), 100);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with the login domain client set and with two WSB groups
 *		a. WSB group 1 is Provider 1 and Provider 2, WSB group 2 is Provider 3 and Provider 4
 *		b. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that the consumer gets an OPEN/OK login message
 */
TEST_F(OmmConsumerTest, RequestRoutingLoginWSB)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2WSBGroups");
	
	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		OmmProvider prov3(provConfig3, provClient3);
		OmmProvider prov4(provConfig4, provClient4);

		OmmConsumer cons(consConfig, consClient);

		testSleep(1000);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);

		ASSERT_EQ(provClient1.wsbActiveState, 0);
		ASSERT_EQ(provClient2.wsbActiveState, 1);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		
		Msg* msg;

		while ((msg = consClient.popMsg()) != NULL)
		{
			if (msg->getDataType() == DataType::StatusMsgEnum)
				continue;

			ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);

			RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
			ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
			ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		}


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with two WSB groups
 *		a. WSB group 1 is Provider 1 and Provider 2, WSB group 2 is Provider 3 and Provider 4
 *		b. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that the current actives are provider 1 and 3, and the standbys are provider 2 and provider 4
 *  4. Send a request with service DIRECT_FEED
 *  5. Verify that both provider 1 and provider 2 get the request
 *  6. Verify that the consumer gets a refresh
 *  7. Close the channel on provider 1 and wait for a bit(allow the consumer to reconnect to provider 1)
 *  8. See that provider 1 is now standby and provider 2 is now active, with no messages or changes on provider 3 and provider 4
 *  9. Verify that the consumer got an OPEN/SUSPECT status message and a refresh message
 *  10. Close the channel on provider 2 and wait for a bit(allow the consumer to reconnect to provider 2)
 *  11. See that provider 2 is now standby and provider 1 is now active, with no messages or changes on provider 3 and provider 4
 *  12. Verify that the consumer got an OPEN/SUSPECT status message and a refresh message
 */
TEST_F(OmmConsumerTest, RequestRoutingWSBSingleItemRecovery)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2WSBGroups");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		OmmProvider prov3(provConfig3, provClient3);
		OmmProvider prov4(provConfig4, provClient4);

		OmmConsumer cons(consConfig);

		testSleep(1000);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);


		ASSERT_EQ(provClient1.wsbActiveState, 0);
		ASSERT_EQ(provClient2.wsbActiveState, 1);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);

		provClient1.clear();
		provClient2.clear();
		provClient3.clear();
		provClient4.clear();


		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		// Check to see that the request has been made to the 1st WSB group.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);

		provClient1.activeRequests.clear();
		provClient1.clear();
		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		
		// Shut down the current active
		prov1.closeChannel(provClient1.clientHandle);

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);

		ASSERT_EQ(provClient1.wsbActiveState, 1);
		ASSERT_EQ(provClient2.wsbActiveState, 0);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);

		// provClient1 should get a bunch of messages, we don't really care about the messages, just verify that one request has gone through.
		ASSERT_EQ(provClient1.activeRequests.size(), 1);
		ASSERT_EQ(provClient2.activeRequests.size(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);

		provClient2.activeRequests.clear();
		provClient1.clear();
		provClient2.clear();
		provClient3.clear();
		provClient4.clear();

		// Shut down the current active
		prov2.closeChannel(provClient2.clientHandle);

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);

		ASSERT_EQ(provClient1.wsbActiveState, 0);
		ASSERT_EQ(provClient2.wsbActiveState, 1);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);

		// provClient2 should get a bunch of messages, we don't really care about the messages, just verify that one request has gone through.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.activeRequests.size(), 1);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with two WSB groups
 *		a. WSB group 1 is Provider 1 and Provider 2, WSB group 2 is Provider 3 and Provider 4
 *		b. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that the current actives are provider 1 and 3, and the standbys are provider 2 and provider 4
 *  4. Send a request with service DIRECT_FEED as a private stream
 *  5. Verify that ony provider 1 gets the request
 *  6. Verify that the consumer gets a refresh
 *  7. Close the channel on provider 1 and wait for a bit(allow the consumer to reconnect to provider 1)
 *  8. See that provider 1 is now standby and provider 2 is now active, with no messages or changes on provider 3 and provider 4
 *  9. Consumer should get a CLOSEDRECOVER/SUSPECT state
 */
TEST_F(OmmConsumerTest, RequestRoutingWSBSingleItemPrivateStreamRecoveryAfterCloseChannel)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2WSBGroups");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		OmmProvider prov3(provConfig3, provClient3);
		OmmProvider prov4(provConfig4, provClient4);

		OmmConsumer cons(consConfig);

		testSleep(1000);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);


		ASSERT_EQ(provClient1.wsbActiveState, 0);
		ASSERT_EQ(provClient2.wsbActiveState, 1);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);

		provClient1.clear();
		provClient2.clear();
		provClient3.clear();
		provClient4.clear();


		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").privateStream(true);

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		// Check to see that the request has been made to the 1st WSB active, and no other.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);

		provClient1.activeRequests.clear();
		provClient1.clear();
		provClient2.clear();
		provClient3.clear();
		provClient4.clear();

		// Shut down the current active
		prov1.closeChannel(provClient1.clientHandle);

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		cons.getSessionInformation(channelInfo);
		ASSERT_EQ(channelInfo.size(), 2);

		ASSERT_EQ(provClient1.wsbActiveState, 1);
		ASSERT_EQ(provClient2.wsbActiveState, 0);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);

		// provClient1 should get a bunch of messages, we don't really care about the messages, just verify that one request has gone through.
		ASSERT_EQ(provClient1.activeRequests.size(), 0);
		ASSERT_EQ(provClient2.activeRequests.size(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 1);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedRecoverEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with two connections
 *		a. All providers provide DIRECT_FEED
 *		b. All providers are configred to not respond to requests
 *  2. Checks to see that the consumer does intitialize
 *  3. Send a request with service DIRECT_FEED as a private stream
 *  4. Verify that both provider 1 gets the request
 *  5. Consumer should get a CLOSEDRECOVER/SUSPECT state
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemPrivateStreamAfterTimeout)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;
	provTestOptions1.sendItemRefresh = false;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap1;

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").privateStream(true);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		// Check to see that the request has been made to both providers
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// This is from the close message
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedRecoverEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);


	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with two connections
 *		a. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that the current actives are provider 1 and 3, and the standbys are provider 2 and provider 4
 *  4. Send a request with service DIRECT_FEED as a private stream
 *  5. Verify that both provider 1 gets the request
 *  6. Verify that the consumer gets a refresh message
 *  7. Send a service down with a state of OPEN/SUSPECT
 *  8. Verify that neither provider gets any messages
 *  6. Consumer should get an OPEN/SUSPECT state 
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemPrivateStreamAfterServiceDown)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").privateStream(true);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		Map encodeMapDown;

		encodeMapDown.addKeyUInt(2, MapEntry::UpdateEnum, FilterList()
			.add(SERVICE_STATE_ID, FilterEntry::UpdateEnum, ElementList()
				.addUInt(ENAME_SVC_STATE, SERVICE_DOWN)
				.addUInt(ENAME_ACCEPTING_REQS, SERVICE_NO)
				.addState(ENAME_STATUS, OmmState::OpenEnum, OmmState::SuspectEnum, OmmState::NoneEnum)
				.complete())
			.complete()).complete();

		encodeUpdate.clear();
		encodeUpdate.domainType(MMT_DIRECTORY).payload(encodeMapDown);
		// Send delete action on prov1
		prov1.submit(encodeUpdate, 0);

		testSleep(1000);
		// Check to see that no messages were sent
		ASSERT_EQ(provClient1.getMessageQueueSize(), 0);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		// One message to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with two connections
 *		a. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that the current actives are provider 1 and 3, and the standbys are provider 2 and provider 4
 *  4. Send a request with service DIRECT_FEED as a private stream
 *  5. Verify that both provider 1 gets the request
 *  6. Verify that the consumer gets a refresh message
 *  7. Close the channel on Provider 1
 *  8. Verify that provider 2 did not get any request 
 *  6. Consumer should get a CLOSEDRECOVER/SUSPECT state
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestPrivateStreamAfterChannelDown)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		ReqMsg consRequest;
		UpdateMsg encodeUpdate;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED").privateStream(true);

		provClient1.clear();
		provClient2.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		prov1.closeChannel(provClient1.clientHandle);

		testSleep(1000);
		
		// Check to see that the request was not made to prov 2
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);


		// One message to the user
		EXPECT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::ClosedRecoverEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer and with two providers
 *		a. Both providers have DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests a one item on DIRECT_FEED with the market price domain
 *  5. Verify that provider 1 got the request
 *  6. Verify that the consumer gets a refresh
 *  7. Send a generic message on the consumer item's handle with no service id or service name
 *  8. Verify that Provider 1 gets it, and does not have a service id
 *  9. Send a generic message on the consumer item's handle with the service Id of 500
 *  10. Verify that Provider 1 gets a generic message with a service Id of 500
 *  11. Send a generic message on the consumer item's handle with the service Id of 1(DIRECT_FEED)
 *  12. Verify that Provider 1 gets a generic message with a service Id of 2(provider's DIRECT_FEED service id)
 *  13. Send a generic message on the consumer item's handle with the service Id of 125 and a domain of 200
 *  12. Verify that Provider 1 gets a generic message with a service Id of 125 with a domain of 200.
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemGenericMsg)
{
	Map serviceMap1;
	setupSingleServiceMap(serviceMap1, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	setupSingleServiceMap(serviceMap2, "DIRECT_FEED", 5);
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{

		ReqMsg consRequest;
		FieldList encodeFieldList;
		GenericMsg encodeGenericMsg;
		Msg* msg;
		RefreshMsg* refreshMsg;
		GenericMsg* genericMsg;
		ReqMsg* reqMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		provClient1.clear();
		provClient2.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1.
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		encodeGenericMsg.clear().name("TestItem").complete();

		cons.submit(encodeGenericMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestItem");
		ASSERT_FALSE(genericMsg->hasServiceId());
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);

		encodeGenericMsg.clear().serviceId(500).name("TestItem").complete();

		cons.submit(encodeGenericMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestItem");
		ASSERT_TRUE(genericMsg->hasServiceId());
		ASSERT_EQ(genericMsg->getServiceId(), 500);
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);

		encodeGenericMsg.clear().serviceId(1).name("TestItem").complete();

		cons.submit(encodeGenericMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestItem");
		ASSERT_TRUE(genericMsg->hasServiceId());
		ASSERT_EQ(genericMsg->getServiceId(), 2);
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);

		encodeGenericMsg.clear().serviceId(125).domainType(200).name("TestItem").complete();

		cons.submit(encodeGenericMsg, handle1);

		testSleep(500);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 0);

		ASSERT_EQ(consClient.getMessageQueueSize(), 0);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestItem");
		ASSERT_TRUE(genericMsg->hasServiceId());
		ASSERT_EQ(genericMsg->getServiceId(), 125);
		ASSERT_EQ(genericMsg->getDomainType(), 200);
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following:
 *	1. Starts up a consumer with a single provider , using the basic config in the EmaConfigTest.xml options
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Request an RWFFld dictionary on the DIRECT_FEED service
 *  5. Verify that the RWFFld refresh is received.
 *  6. Request an RWFEnum dictionary on the DIRECT_FEED service
 *  7. Verify that the RWFEnum refresh is received
 */
TEST_F(OmmConsumerTest, SingleConnectionDictionaryRequestByServiceName)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnectionDownloadDict");


	try
	{
		Msg* msg;
		RefreshMsg* refreshMsg;
		OmmProvider prov1(provConfig1, provClient1);

		OmmConsumer cons(consConfig);

		// Login
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		provClient1.clear();

		ReqMsg dictionaryRequest;
		dictionaryRequest.name("RWFFld").serviceName("DIRECT_FEED").filter(DICTIONARY_NORMAL).domainType(MMT_DICTIONARY);

		UInt64 dirHandle = cons.registerClient(dictionaryRequest, consClient);

		testSleep(5000);

		ASSERT_GE(consClient.getMessageQueueSize(), 1u);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "RWFFld");
		ASSERT_TRUE(refreshMsg->getClearCache());

		dictionaryRequest.clear();
		dictionaryRequest.domainType(MMT_DICTIONARY).name("RWFEnum").filter(DICTIONARY_NORMAL).serviceName("DIRECT_FEED");

		consClient.clear();

		UInt64 dirHandle2 = cons.registerClient(dictionaryRequest, consClient);

		testSleep(5000);

		ASSERT_GE(consClient.getMessageQueueSize(), 1u);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "RWFEnum");
		ASSERT_TRUE(refreshMsg->getClearCache());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a consumer and with a single provider
 *		a. Provider provides DIRECT_FEED
 *		b. Consumer has a service list SVG1 with services "BADSERVICE" and "DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Requests an item with service list name "SVG1"
 *  4. Verifies provider gets a request
 *  5. Verifies that the consumer gets a response with service name SVG1
 */
TEST_F(OmmConsumerTest, SingleConnectionRequestServiceList)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000").adminControlDirectory(OmmIProviderConfig::UserControlEnum);

	ServiceList serviceList("SVG1");
	serviceList.addService("BADSERVICE");
	serviceList.addService("DIRECT_FEED");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection").addServiceList(serviceList);


	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		OmmProvider prov1(provConfig1, provClient1);

		OmmConsumer cons(consConfig);

		// Login
		ASSERT_EQ(provClient1.getMessageQueueSize(), 2);
		provClient1.clear();

		consRequest.name("Item1").domainType(MMT_MARKET_PRICE).serviceListName("SVG1");

		UInt64 consHandle1 = cons.registerClient(consRequest, consClient);

		testSleep(5000);

		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "SVG1");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)2);
		ASSERT_TRUE(refreshMsg->getComplete());
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with preferred host enabled, and starts two providers on the non-preferred ports.
 *		a. All providers provide DIRECT_FEED
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests the one item on DIRECT_FEED
 *  5. Verify that one request goes out to the correct provider
 *  6. Start up the configured preferred providers
 *  7. Call cons.fallbackPreferredHost();
 *  8. Validate that all connections are handled correctly, with providers 1 and 3 getting a login message(indicating that the connection was closed), and providers 2 and 4 getting the login and directory requests
 *     and provider 2 also getting an item request
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemPHRecover)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 2);
	ProviderTestOptions provTestOptions;
	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_4Channel_PH");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov3(provConfig3, provClient3);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient3.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		//ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		OmmProvider prov2(provConfig2, provClient2);
		OmmProvider prov4(provConfig4, provClient4);
	
		// Sleep to make sure that everything's up
		testSleep(500);

		cons.fallbackPreferredHost();

		testSleep(1000);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient2.getMessageQueueSize(), 3);
		ASSERT_EQ(provClient3.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 2);
		
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		// Check client 2
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		// Check client 3
		msg = provClient3.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		// Check client 4
		msg = provClient4.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient4.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		//ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*	This test does the following
 *	1. Starts up a request routing consumer with preferred host enabled, and starts two providers on the non-preferred ports.
 *		a. All providers provide DIRECT_FEED
 *		b. Providers 1 and 3 provide DIRECT_FEED with RT/TBT QoS, providers 2 and 4 provide delayed.
 *  2. Checks to see that the consumer does intitialize
 *  3. Verify that there are two channels is active in the consumer
 *  4. Requests the one item on DIRECT_FEED with no QoS specified
 *  5. Verify that one request goes out to the correct provider
 *  6. Start up the configured preferred providers
 *  7. Call cons.fallbackPreferredHost();
 *  8. Validate that all connections are handled correctly, with providers 1 and 3 getting a login message(indicating that the connection was closed), and providers 2 and 4 getting the login and directory requests
 *     and provider 2 also getting an item request
 */
TEST_F(OmmConsumerTest, RequestRoutingRequestSingleItemPHRecoverDifferentQoS)
{
	Map serviceMap1;
	serviceMap1.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos()
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions1;
	provTestOptions1.directoryPayload = &serviceMap1;

	Map serviceMap2;
	serviceMap2.addKeyUInt(2, MapEntry::AddEnum, FilterList().
		add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
			addAscii(ENAME_NAME, "DIRECT_FEED").
			addAscii(ENAME_VENDOR, "company").
			addArray(ENAME_CAPABILITIES, OmmArray().
				addUInt(MMT_DICTIONARY).
				addUInt(MMT_MARKET_PRICE).
				addUInt(MMT_MARKET_BY_PRICE).
				addUInt(MMT_SYMBOL_LIST).
				addUInt(MMT_SYSTEM).
				complete()).
			addArray(ENAME_QOS, OmmArray().
				addQos(150, 150)
				.complete()).
			addArray(ENAME_DICTIONARYS_USED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addArray(ENAME_DICTIONARYS_PROVIDED, OmmArray().
				addAscii("RWFFld").
				addAscii("RWFEnum").
				complete()).
			addAscii(ENAME_ITEM_LIST, "ItemList").
			complete()).
		add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
			addUInt(ENAME_SVC_STATE, SERVICE_UP).
			addUInt(ENAME_ACCEPTING_REQS, SERVICE_YES).
			complete()).
		complete()).complete();
	ProviderTestOptions provTestOptions2;
	provTestOptions2.directoryPayload = &serviceMap2;

	IProviderTestClientBase provClient1(provTestOptions1);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001");

	IProviderTestClientBase provClient3(provTestOptions1);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002");

	IProviderTestClientBase provClient4(provTestOptions2);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_4Channel_PH");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		ReqMsg* reqMsg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov3(provConfig3, provClient3);

		OmmConsumer cons(consConfig);

		EmaVector<ChannelInformation> channelInfo;

		int totalRequestAfterPH = 0;

		cons.getSessionInformation(channelInfo);

		ASSERT_EQ(channelInfo.size(), 2);

		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		provClient1.clear();
		provClient3.clear();

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(500);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient3.getMessageQueueSize(), 0);
		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 2);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		//ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		OmmProvider prov2(provConfig2, provClient2);
		OmmProvider prov4(provConfig4, provClient4);

		// Sleep to make sure that everything's up
		testSleep(500);

		std::cout << "Calling PH" << std::endl;
		cons.fallbackPreferredHost();

		testSleep(10000);

		// Check to see that the request has been made to prov1
		ASSERT_EQ(provClient1.getMessageQueueSize(), 1);
		ASSERT_GE(provClient2.getMessageQueueSize(), 2u);		// prov 2 and 4 may both get the request
		ASSERT_EQ(provClient3.getMessageQueueSize(), 1);
		ASSERT_GE(provClient4.getMessageQueueSize(), 2u);

		msg = provClient1.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		// Check client 2
		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient2.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		msg = provClient2.popMsg();

		if (msg != NULL)
		{
			ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
			reqMsg = static_cast<ReqMsg*>(msg);
			ASSERT_EQ(reqMsg->getName(), "Item1");
			ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
			ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
			ASSERT_EQ(reqMsg->getServiceId(), 2);
			totalRequestAfterPH++;
		}

		// Check client 3
		msg = provClient3.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		// Check client 4
		msg = provClient4.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_LOGIN);

		msg = provClient4.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getDomainType(), MMT_DIRECTORY);

		msg = provClient4.popMsg();

		if (msg != NULL)
		{
			ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
			reqMsg = static_cast<ReqMsg*>(msg);
			ASSERT_EQ(reqMsg->getName(), "Item1");
			ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
			ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
			ASSERT_EQ(reqMsg->getServiceId(), 2);
			totalRequestAfterPH++;
		}

		// Make sure one request went out in total
		ASSERT_EQ(totalRequestAfterPH, 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 3);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		//ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/* Test preferred host fallback to the starting server of the preferred WSB group for the login based wsb mode.
*  Preferred Host, WSB-Login: WSB-G0(S1,S2), WSB-G1 (S3,S4, S5) are specified. WSB-G0 is preferred WSB group.
* The following is the test steps:
* - S1 is down, S2, S3, S4, S5 are up.
* - Start the consumer.
* - The consumer connects to S3 as active server and S4, S5 as standby servers.
* - Bring down S3 and S4 is the new active.
* - Bring up S1
* - Waits for PH timer to be triggered.
* - The consumer should fallback to S1 as active server and S2 as standby server.
*/
TEST_F(OmmConsumerTest, PreferredHostFallBackToPreferredWSBWhileWhileStartingServerIsDownForCurrentWSBGroup)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 1);
	ProviderTestOptions provTestOptions;

	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000").port("15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001").port("15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002").port("15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003").port("15003");

	IProviderTestClientBase provClient5(provTestOptions);

	OmmIProviderConfig provConfig5("EmaConfigTest.xml");
	provConfig5.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15004").port("15004");

	ConsumerTestOptions consTestOptions;
	consTestOptions.getChannelInformation = true;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("PreferredHostCons_2WSBGroups");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		/* WarmStandbyChannel_3 */
			// Starting server is down at startup 
		OmmProvider provider2(provConfig2, provClient2);

		/* WarmStandbyChannel_4 */
		OmmProvider* pProvider3 = new OmmProvider(provConfig3, provClient3);
		OmmProvider provider4(provConfig4, provClient4);
		OmmProvider provider5(provConfig5, provClient5);

		OmmConsumer cons(consConfig);

		testSleep(1000);


		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();


		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(3000);

		// Check to see that the request has been made to the active and standby servers of WarmStandbyChannel_4
		ASSERT_EQ(provClient3.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 1);

		msg = provClient3.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		ChannelInformation *pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();

		// Shutdown the active server of WarmStandbyChannel_4
		delete pProvider3;

		// Start the active server of WarmStandbyChannel_3
		OmmProvider provider1(provConfig1, provClient1);

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		/* Receives the WSB generic message to be the new active server */
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 0);

		ASSERT_EQ(provClient4.wsbActiveState, 0);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		/* Waits until the fallback timer is triggered to connected to the starting server of the preferred WSB Channel(WarmStandbyChannel_3)*/
		testSleep(10000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/* Test preferred host fallback to the starting server of the current WSB group when the PHFallBackWithInWSBGroup option is set to true for the login based wsb mode.
*  Preferred Host, WSB-Login: WSB-G0(S1,S2), WSB-G1 (S3,S4, S5) are specified. WSB-G0 is preferred WSB group.
* The following is the test steps:
* - S1 is down, S2, S3, S4, S5 are up.
* - Start the consumer.
* - The consumer connects to S3 as active server and S4, S5 as standby servers.
* - Bring down S3 and S4 is the new active.
* - Bring up S1 and S3.
* - Waits for PH timer to be triggered.
* - The consumer should fallback to S3 as active server and S4, S5 as standby servers.
*/
TEST_F(OmmConsumerTest, PreferredHostFallBackWithInCurrentWSBGroupWhileWhileStartingServerIsDown)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 1);
	ProviderTestOptions provTestOptions;

	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000").port("15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001").port("15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002").port("15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003").port("15003");

	IProviderTestClientBase provClient5(provTestOptions);

	OmmIProviderConfig provConfig5("EmaConfigTest.xml");
	provConfig5.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15004").port("15004");

	ConsumerTestOptions consTestOptions;
	consTestOptions.getChannelInformation = true;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("PreferredHostCons_2WSBGroups_fallbackwithingroup");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		/* WarmStandbyChannel_3 */
		// Starting server is down at startup 
		OmmProvider provider2(provConfig2, provClient2);

		/* WarmStandbyChannel_4 */
		OmmProvider* pProvider3 = new OmmProvider(provConfig3, provClient3);
		OmmProvider provider4(provConfig4, provClient4);
		OmmProvider provider5(provConfig5, provClient5);

		OmmConsumer cons(consConfig);

		testSleep(1000);


		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();


		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(3000);

		// Check to see that the request has been made to the active and standby servers of WarmStandbyChannel_4
		ASSERT_EQ(provClient3.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 1);

		msg = provClient3.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		ChannelInformation* pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();

		// Shutdown the active server of WarmStandbyChannel_4
		delete pProvider3;

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		/* Receives the WSB generic message to be the new active server */
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 0);

		ASSERT_EQ(provClient4.wsbActiveState, 0);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		testSleep(5000);

		provClient3.activeRequests.clear();
		provClient3.clear();

		// Start the active server of WarmStandbyChannel_3 and WarmStandbyChannel_4.
		OmmProvider provider1(provConfig1, provClient1);
		pProvider3 = new OmmProvider(provConfig3, provClient3);

		/* Waits until the fallback timer is triggered to connected to the starting server of the WarmStandbyChannel_4 */
		testSleep(10000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		delete pProvider3;
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/* Test preferred host fallback to the starting server of the current WSB group when the PHFallBackWithInWSBGroup option is set to true for the login based wsb mode.
*  Preferred Host, WSB-Login: WSB-G0(S1,S2), WSB-G1 (S3,S4, S5) are specified. WSB-G0 is preferred WSB group.
* The following is the test steps:
* - S1 is down, S2, S3, S4, S5 are up.
* - Start the consumer.
* - The consumer connects to S3 as active server and S4, S5 as standby servers.
* - Bring down S3 and S4 is the new active.
* - Bring up S1 and S3.
* - Calls the fallbackPreferredHost() method to fallback.
* - The consumer should fallback to S3 as active server and S4, S5 as standby servers.
*/
TEST_F(OmmConsumerTest, PreferredHostFallBackWithInCurrentWSBGroupWhileWhileStartingServerIsDown_fallbackPreferredHost)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 1);
	ProviderTestOptions provTestOptions;

	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000").port("15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001").port("15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002").port("15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003").port("15003");

	IProviderTestClientBase provClient5(provTestOptions);

	OmmIProviderConfig provConfig5("EmaConfigTest.xml");
	provConfig5.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15004").port("15004");

	ConsumerTestOptions consTestOptions;
	consTestOptions.getChannelInformation = true;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("PreferredHostCons_2WSBGroups_fallbackwithingroup_by_fallbackPreferredHost");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		ReqMsg* reqMsg;
		StatusMsg* statusMsg;

		/* WarmStandbyChannel_3 */
		// Starting server is down at startup
		OmmProvider provider2(provConfig2, provClient2);

		/* WarmStandbyChannel_4 */
		OmmProvider* pProvider3 = new OmmProvider(provConfig3, provClient3);
		OmmProvider provider4(provConfig4, provClient4);
		OmmProvider provider5(provConfig5, provClient5);

		OmmConsumer cons(consConfig);

		testSleep(1000);


		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();


		consRequest.clear().name("Item1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 handle1 = cons.registerClient(consRequest, consClient);

		testSleep(3000);

		// Check to see that the request has been made to the active and standby servers of WarmStandbyChannel_4
		ASSERT_EQ(provClient3.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 1);

		msg = provClient3.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "Item1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 1);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		ChannelInformation* pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();

		// Shutdown the active server of WarmStandbyChannel_4
		delete pProvider3;

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(5000);

		/* Receives the WSB generic message to be the new active server */
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 0);

		ASSERT_EQ(provClient4.wsbActiveState, 0);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		testSleep(5000);

		provClient3.activeRequests.clear();
		provClient3.clear();

		// Start the active server of WarmStandbyChannel_3 and WarmStandbyChannel_4.
		OmmProvider provider1(provConfig1, provClient1);
		pProvider3 = new OmmProvider(provConfig3, provClient3);

		/* Calls the fallbackPreferredHost() method to fallback to the starting server of the WarmStandbyChannel_4 */
		cons.fallbackPreferredHost();

		testSleep(5000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);

		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getName(), "Item1");
		ASSERT_EQ(statusMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(statusMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Service for this item was lost.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "Item1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_EQ(refreshMsg->getPayload().getDataType(), DataType::FieldListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		delete pProvider3;
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/*
* PH, WSB-Login, Scenario: test fallBackWithInWSBGroup outside of preferred group; Trigger: detectionTimeInterval; Config: detectionTimeInterval=15, 
default warmStandbyGroupListIndex=0, fallBackWithInWSBGroup=True, PreferredGroup activeServer down at start. 
This test case expects to receives the PREFERRED_HOST_NO_FALLBACK event after API is connected to S3 in the current preferred group when the detectionTimeInterval is triggered.
*/
TEST_F(OmmConsumerTest, LoginBasedPreferredHostFallBackWithInCurrentWSBGroupAndAPIIsOnPreferredHost_DetectionTimeInterval)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 1);
	ProviderTestOptions provTestOptions;

	provTestOptions.directoryPayload = &serviceMap;

	IProviderTestClientBase provClient1(provTestOptions);

	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000").port("15000");

	IProviderTestClientBase provClient2(provTestOptions);

	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001").port("15001");

	IProviderTestClientBase provClient3(provTestOptions);

	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002").port("15002");

	IProviderTestClientBase provClient4(provTestOptions);

	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003").port("15003");

	IProviderTestClientBase provClient5(provTestOptions);

	OmmIProviderConfig provConfig5("EmaConfigTest.xml");
	provConfig5.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15004").port("15004");

	ConsumerTestOptions consTestOptions;
	consTestOptions.getChannelInformation = true;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("PreferredHostCons_2WSBGroups_fallbackwithingroup");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		/* WarmStandbyChannel_3 */
		// Starting server is down at startup 
		OmmProvider provider2(provConfig2, provClient2);

		/* WarmStandbyChannel_4 */
		OmmProvider* pProvider3 = new OmmProvider(provConfig3, provClient3);
		OmmProvider provider4(provConfig4, provClient4);
		OmmProvider provider5(provConfig5, provClient5);

		OmmConsumer cons(consConfig, consClient); // Add a OmmConsumerClient to receive the preferred host channel events.

		testSleep(1000);


		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_STREQ(statusMsg->getState().getStatusText(), "channel down");
		ChannelInformation* pChannelInfo = consClient.popChannelInfo();
		ASSERT_STREQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);

		msg = consClient.popMsg();

		// Checks the login response message from the starting server of WarmStandbyChannel_4
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_STREQ(refreshMsg->getState().toString(), "Open / Ok / None / 'Login Accepted'");
		ASSERT_EQ(refreshMsg->getAttrib().getDataType(), DataType::ElementListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		provClient2.clear();
		provClient3.clear();
		provClient4.clear();
		provClient5.clear();

		// Shutdown the active server of WarmStandbyChannel_4
		delete pProvider3;

		// In this time, the consumer should reconnect and re-establish the former primary active as a standby
		testSleep(1000);

		/* Receives the WSB generic message to be the new active server */
		ASSERT_EQ(provClient4.getMessageQueueSize(), 1);
		ASSERT_EQ(provClient5.getMessageQueueSize(), 0);

		ASSERT_EQ(provClient4.wsbActiveState, 0);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		// Start S3 to fallback to when the timer is triggered later.
		provClient3.clear();
		pProvider3 = new OmmProvider(provConfig3, provClient3);

		// Waits for the timer to trigger
		testSleep(18000);

		ASSERT_EQ(provClient3.getMessageQueueSize(), 4);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHStartingFallback);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host starting fallback");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHComplete);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host complete");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		ASSERT_EQ(provClient3.wsbActiveState, 0);
		ASSERT_EQ(provClient4.wsbActiveState, 1);
		ASSERT_EQ(provClient5.wsbActiveState, 1);

		testSleep(15000);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHNoFallback);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host no fallback");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		testSleep(15000);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHNoFallback);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host no fallback");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		delete pProvider3;
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

/***
* PH, WSB-Service, Scenario: test fallBackWithInWSBGroup outside of preferred group; Trigger: detectionTimeInterval; Config: detectionTimeInterval=15
* Preferred Host, WSB-Service: WSB-G0 (S1*,S2), WSB-G1(S3*,S4) are specified. WSB-G1 is preferred WSB group.
* This test case expects to receives the PREFERRED_HOST_START_FALLBACK  and PREFERRED_HOST_COMPLETE events after API is connected to S1 in the current preferred group when the detectionTimeInterval is triggered.
* API does not cut over to WSB-G1: it stays on WSB-G0 (S1) as the PHFallBackWithInWSBGroup is set to true.
***/
TEST_F(OmmConsumerTest, ServiceBasedPreferredHostFallBackWithInCurrentWSBGroupAndAPIIsOnPreferredHost_DetectionTimeInterval)
{
	Map serviceMap;
	setupSingleServiceMap(serviceMap, "DIRECT_FEED", 1);
	ProviderTestOptions provTestOptions;

	provTestOptions.directoryPayload = &serviceMap;

	//WarmStandbyChannel_5(G0)
	IProviderTestClientBase provClient1(provTestOptions);
	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15000").port("15000");

	IProviderTestClientBase provClient2(provTestOptions);
	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15001").port("15001");

	//WarmStandbyChannel_6(G1)
	IProviderTestClientBase provClient3(provTestOptions);
	OmmIProviderConfig provConfig3("EmaConfigTest.xml");
	provConfig3.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15002").port("15002");

	IProviderTestClientBase provClient4(provTestOptions);
	OmmIProviderConfig provConfig4("EmaConfigTest.xml");
	provConfig4.adminControlDirectory(OmmIProviderConfig::UserControlEnum).providerName("TestProvider_15003").port("15003");

	ConsumerTestOptions consTestOptions;
	consTestOptions.getChannelInformation = true;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("PreferredHostCons_2WSBGroups_servicebased_fallbackwithingroup");

	try
	{
		ReqMsg consRequest;
		Msg* msg;
		RefreshMsg* refreshMsg;
		StatusMsg* statusMsg;

		/* WarmStandbyChannel_5 */
		// All servers are up at startup.
		OmmProvider provider1(provConfig1, provClient1);
		OmmProvider provider2(provConfig2, provClient2);

		/* WarmStandbyChannel_6 */
		// All servers are up at startup.
		OmmProvider* pProvider3 = new OmmProvider(provConfig3, provClient3);
		OmmProvider* pProvider4 = new OmmProvider(provConfig4, provClient4);

		OmmConsumer cons(consConfig, consClient); // Add a OmmConsumerClient to receive the preferred host channel events.

		testSleep(1000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 1);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 1);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);

		// Checks the login response message from the starting server of WarmStandbyChannel_6
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_STREQ(refreshMsg->getState().toString(), "Open / Ok / None / 'Login Accepted'");
		ASSERT_EQ(refreshMsg->getAttrib().getDataType(), DataType::ElementListEnum);
		ChannelInformation* pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15002");
		ASSERT_EQ(pChannelInfo->port(), 15002);

		// Brings down all servers in WarmStandbyChannel_6
		delete pProvider3;
		testSleep(1000);
		delete pProvider4;

		// Waits to switch over to WSB-G0
		testSleep(2000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);
		
		msg = consClient.popMsg();
		
		// Checks the login status messages when the channel is down.
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Channel is down.");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::SuspectEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "channel down");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15003");
		ASSERT_EQ(pChannelInfo->port(), 15003);

		testSleep(5000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		// Checks the login status messages when the channel is up in WSB-G0.
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::NoneEnum);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "channel up");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);

		// Checks the login response message from the starting server of WarmStandbyChannel_6
		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(refreshMsg->getComplete());
		ASSERT_STREQ(refreshMsg->getState().toString(), "Open / Ok / None / 'Login Accepted'");
		ASSERT_EQ(refreshMsg->getAttrib().getDataType(), DataType::ElementListEnum);
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);

		// Waits until the PH timer is triggered.
		testSleep(10000);

		ASSERT_EQ(consClient.getMessageQueueSize(), 2);
		ASSERT_EQ(consClient.getChannelInfoQueueSize(), 2);

		msg = consClient.popMsg();

		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHStartingFallback);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host starting fallback");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);

		msg = consClient.popMsg();
		ASSERT_TRUE(msg != NULL);
		ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
		statusMsg = static_cast<StatusMsg*>(msg);
		ASSERT_EQ(statusMsg->getDomainType(), MMT_LOGIN);
		ASSERT_TRUE(statusMsg->hasState());
		ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
		ASSERT_EQ(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
		ASSERT_EQ(statusMsg->getState().getStatusCode(), OmmState::StatusCode::SocketPHComplete);
		ASSERT_EQ(statusMsg->getState().getStatusText(), "Preferred host complete");
		pChannelInfo = consClient.popChannelInfo();
		ASSERT_EQ(pChannelInfo->getName(), "TestChannel_15000");
		ASSERT_EQ(pChannelInfo->port(), 15000);
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "uncaught exception in test";
	}
}

//////

/* OmmConsumer constructors overloading variant */
enum class OmmConsumerConstructorType
{
	config,
	config_client,
	config_oAuthClient,
	config_oAuthClient_errorClient,
	config_errorClient,
	config_adminClient_oAuthClient,
	config_adminClient_errorClient,
	config_adminClient_oAuthClient_errorClient
};

std::ostream& operator<<(std::ostream& ostream, OmmConsumerConstructorType constructorType)
{
	switch (constructorType) {
	case OmmConsumerConstructorType::config:
			ostream << "config"; break;
	case OmmConsumerConstructorType::config_client:
			ostream << "config_client"; break;
	case OmmConsumerConstructorType::config_oAuthClient:
			ostream << "config_oAuthClient"; break;
	case OmmConsumerConstructorType::config_oAuthClient_errorClient:
			ostream << "config_oAuthClient_errorClient"; break;
	case OmmConsumerConstructorType::config_errorClient:
			ostream << "config_errorClient"; break;
	case OmmConsumerConstructorType::config_adminClient_oAuthClient:
			ostream << "config_adminClient_oAuthClient"; break;
	case OmmConsumerConstructorType::config_adminClient_errorClient:
			ostream << "config_adminClient_errorClient"; break;
	case OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient:
			ostream << "config_adminClient_oAuthClient_errorClient"; break;
	default:
			ostream << "error: unknown constructor type!"; break;
	}
	return ostream;
}

class ConsumerErrorClientTest : public refinitiv::ema::access::OmmConsumerErrorClient
{
public:

	ConsumerErrorClientTest() :
		countOnInvalidHandle(0),
		countOnInaccessibleLogFile(0),
		countOnSystemError(0),
		countOnMemoryExhaustion(0),
		countOnInvalidUsage(0),
		countOnJsonConverter(0),
		countOnDispatchError(0)
	{ }

	~ConsumerErrorClientTest() = default;

	bool isCalledAnyErrorHandler() const
	{
		return countOnInvalidHandle > 0 ||
			countOnInaccessibleLogFile > 0 ||
			countOnSystemError > 0 ||
			countOnMemoryExhaustion > 0 ||
			countOnInvalidUsage > 0 ||
			countOnJsonConverter > 0 ||
			countOnDispatchError > 0;
	}

	unsigned getCountCalledHandlers() const {
		return (countOnInvalidHandle + countOnInaccessibleLogFile + countOnSystemError
			+ countOnMemoryExhaustion + countOnInvalidUsage + countOnJsonConverter
			+ countOnDispatchError
			);
	}

	unsigned getCountOnInvalidHandle() const { return countOnInvalidHandle; }
	unsigned getCountOnInaccessibleLogFile() const { return countOnInaccessibleLogFile; }
	unsigned getCountOnSystemError() const { return countOnSystemError; }
	unsigned getCountOnMemoryExhaustion() const { return countOnMemoryExhaustion; }
	unsigned getCountOnInvalidUsage() const { return countOnInvalidUsage; }
	unsigned getCountOnJsonConverter() const { return countOnJsonConverter; }
	unsigned getCountOnDispatchError() const { return countOnDispatchError; }


	/* Overide OmmConsumerErrorClient methods */

	void onInvalidHandle( UInt64 handle, const EmaString& text ) override
	{
		++countOnInvalidHandle;
		//cout << endl << "onInvalidHandle callback function" << endl;
		//cout << "Invalid handle: " << handle << endl;
		//cout << "Error text: " << text << endl;
	}

	void onInaccessibleLogFile( const EmaString& fileName, const EmaString& text ) override
	{
		++countOnInaccessibleLogFile;
		//cout << endl << "onInaccessibleLogFile callback function" << endl;
		//cout << "Inaccessible file name: " << fileName << endl;
		//cout << "Error text: " << text << endl;
	}

	void onSystemError( Int64 code, void* address, const EmaString& text ) override
	{
		++countOnSystemError;
		//cout << endl << "onSystemError callback function" << endl;
		//cout << "System Error code: " << code << endl;
		//cout << "System Error Address: " << address << endl;
		//cout << "Error text: " << text << endl;
	}

	void onMemoryExhaustion( const EmaString& text ) override
	{
		++countOnMemoryExhaustion;
		//cout << endl << "onMemoryExhaustion callback function" << endl;
		//cout << "Error text: " << text << endl;
	}

	using refinitiv::ema::access::OmmConsumerErrorClient::onInvalidUsage; // avoid name hiding of base overload
	void onInvalidUsage( const EmaString& text, Int32 errorCode ) override
	{
		++countOnInvalidUsage;
		//cout << "onInvalidUsage callback function" << endl;
		//cout << "Error text: " << text << endl;
		//cout << "Error code: " << errorCode << endl;
	}

	void onJsonConverter( const EmaString& text, Int32 errorCode, const ConsumerSessionInfo& sessionInfo ) override
	{
		++countOnJsonConverter;
		//cout << "onJsonConverter callback function" << endl;
		//cout << "Error text: " << text << endl;
		//cout << "Error code: " << errorCode << endl;
	}

	void onDispatchError( const EmaString& text, Int32 errorCode ) override
	{
		++countOnDispatchError;
		//cout << "onDispatchError callback function" << endl;
		//cout << "Error text: " << text << endl;
		//cout << "Error code: " << errorCode << endl;
	}
 
protected:
	unsigned countOnInvalidHandle;
	unsigned countOnInaccessibleLogFile;
	unsigned countOnSystemError;
	unsigned countOnMemoryExhaustion;
	unsigned countOnInvalidUsage;
	unsigned countOnJsonConverter;
	unsigned countOnDispatchError;
};

class OmmConsumerCreateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		pConsumerConfig = nullptr;
		pConsumerTestClient = nullptr;
		pOmmConsumerTest = nullptr;
	}

	void TearDown() override
	{
		if (pOmmConsumerTest) {
			delete pOmmConsumerTest;
			pOmmConsumerTest = nullptr;
		}
		if (pConsumerTestClient) {
			delete pConsumerTestClient;
			pConsumerTestClient = nullptr;
		}
		if (pConsumerConfig) {
			delete pConsumerConfig;
			pConsumerConfig = nullptr;
		}
	}

public:

	OmmConsumerConfig* pConsumerConfig;
	Map configMap;

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase* pConsumerTestClient;
	
	OAuthClientTest oAuthClient;
	ConsumerErrorClientTest errorClient;

	OmmConsumer* pOmmConsumerTest;

	/* Create OmmConsumer instance using different constructor options */
	/**/
	/* constructorType OmmConsumer constructors overloading variant */
	OmmConsumer* createOmmConsumer( OmmConsumerConstructorType constructorType, ConsumerProgrammaticTestConfig& consProgConfig )
	{
		/* Create OmmConsumer's programmatic configuration */
		consProgConfig.createProgrammaticConfig(configMap);

		pConsumerConfig = new OmmConsumerConfig();
		pConsumerConfig->config(configMap);

		/* Consumer client */
		pConsumerTestClient = new ConsumerTestClientBase(consTestOptions);

		/* Create OmmConsumer instance using different constructor */
		switch (constructorType) {
		case OmmConsumerConstructorType::config:
		{
			// OmmConsumer( const OmmConsumerConfig& config );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig);
			break;
		}

		case OmmConsumerConstructorType::config_client:		
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmConsumerClient& client, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, *pConsumerTestClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_client));
			break;
		}

		case OmmConsumerConstructorType::config_oAuthClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, oAuthClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_oAuthClient));
			break;
		}

		case OmmConsumerConstructorType::config_oAuthClient_errorClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, oAuthClient, errorClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_oAuthClient_errorClient));
			break;
		}

		case OmmConsumerConstructorType::config_errorClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmConsumerErrorClient& client );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, errorClient);
			break;
		}

		case OmmConsumerConstructorType::config_adminClient_oAuthClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, *pConsumerTestClient, oAuthClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_adminClient_oAuthClient));
			break;
		}

		case OmmConsumerConstructorType::config_adminClient_errorClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmConsumerErrorClient& errorClient, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, *pConsumerTestClient, errorClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_adminClient_errorClient));
			break;
		}

		case OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient:
		{
			// OmmConsumer( const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure = 0 );
			pOmmConsumerTest = new OmmConsumer(*pConsumerConfig, *pConsumerTestClient, oAuthClient, errorClient,
				(void*)static_cast<int>(OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient));
		}

		default:  // error case: unknown constructor option
			break;
		}

		return pOmmConsumerTest;
	}

	/* Is the error client configured for OmmConsumer? */
	/* When an error client is configured, the consumer will notify it of any errors. */
	/* Otherwise, the consumer will throw an exception. */
	/**/
	/* constructorType OmmConsumer constructors overloading variant */
	bool hasErrorClient( OmmConsumerConstructorType constructorType ) const
	{
		bool bRes = false;

		switch (constructorType) {
		case OmmConsumerConstructorType::config_oAuthClient_errorClient:
		case OmmConsumerConstructorType::config_errorClient:
		case OmmConsumerConstructorType::config_adminClient_errorClient:
		case OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient:
			bRes = true;
			break;
		};
		return bRes;
	}

	/* Is the test client assigned for the OmmConsumer? */
	bool hasTestClient( OmmConsumerConstructorType constructorType ) const
	{
		bool bRes = false;

		switch (constructorType) {
		case OmmConsumerConstructorType::config_client:
		case OmmConsumerConstructorType::config_adminClient_oAuthClient:
		case OmmConsumerConstructorType::config_adminClient_errorClient:
		case OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient:
			bRes = true;
			break;
		};
		return bRes;
	}
};

class OmmConsumerCreateTestParams {
public:
	OmmConsumerConstructorType		constructorType;		// What constructor variant to use

	OmmConsumerCreateTestParams( OmmConsumerConstructorType consumerConstructorType ) :
		constructorType(consumerConstructorType)
	{
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend std::ostream& operator<<(std::ostream& out, const OmmConsumerCreateTestParams& params)
	{
		out << "["
			<< "createType:" << params.constructorType // << ","
			<< "]";
		return out;
	}
};

class OmmConsumerCreateTestFixture
	: public OmmConsumerCreateTest, public ::testing::WithParamInterface<OmmConsumerCreateTestParams>
{
};

/*  Negative test case: expect an error. */
/*  When a consumer attempts to connect to a non-existent provider, */
/*  it must handle the exception or error properly. */
/*  This test does the following:
 *  1. Starts up an omm-consumer with a login client with no providers
 *  2. Checks to see that the consumer does not initialize
 *  3. Checks to see that the consumer did not get anyhting
 */
TEST_P(OmmConsumerCreateTestFixture, CreateConsumerWithNoProviderShouldThrowException)
{
	const OmmConsumerCreateTestParams& testParams = GetParam();

	try
	{
		ConsumerProgrammaticTestConfig consProgConfig;
		// Set loginRequestTimeOut and channelInitializationTimeout to low values to speed up the test
		consProgConfig.loginRequestTimeOut = 1000; // 1 second - timeout for login request
		consProgConfig.channelInitializationTimeout = 1; // 1 second - timeout for channel initialization

		/* While OmmConsumer creating we expect an error/exception because connection fails. */
		OmmConsumer* pOmmConsumer = createOmmConsumer( testParams.constructorType, consProgConfig);

		/* If an error handling client is configured, it should handle the error rather than throwing an exception. */
		if ( hasErrorClient( testParams.constructorType ) )
		{			
			ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "Expected that the errorClient handled the error.";

			ASSERT_EQ(1, errorClient.getCountOnInvalidUsage());
			ASSERT_EQ(1, errorClient.getCountCalledHandlers());
		}
		else
		{
			ASSERT_FALSE(true) << "Error: Expected exception";
		}

	}
	catch ( const OmmException& exception )
	{
		//cout << exception.toString() << endl;
		/* Test expected exception */
		ASSERT_TRUE(true) << "Expected exception: " << exception.getText();
	}
	catch (...)
	{
		/* Test expected exception */
		ASSERT_TRUE(true);
	}

	/* Check that ConsumerTestClient did not receive any messages */
	ASSERT_EQ(0, pConsumerTestClient->getMessageQueueSize()) << "Expected no messages in the ConsumerTestClient message queue: " << pConsumerTestClient->getMessageQueueSize();	
}

/*  Consumer connects to Provider correctly. */
/*  This test does the following:
 *  1. Starts up an omm-consumer with a login client with one provider
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has only one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_P(OmmConsumerCreateTestFixture, CreateConsumerWithOneProvider)
{
	const OmmConsumerCreateTestParams& testParams = GetParam();

	try
	{
		ProviderTestOptions provTestOptions;
		IProviderTestClientBase provClient(provTestOptions);

		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);

		OmmIProviderConfig provConfig("EmaConfigTest.xml");
		provConfig.config(configIProvMap);
		provConfig.providerName("ProviderProgrammaticTest_1");

		/* Create IProvider instance */
		OmmProvider prov( provConfig, provClient );
		//cout << "OmmProvider created." << endl;

		/* Create OmmConsumer instance */
		ConsumerProgrammaticTestConfig consProgConfig;
		OmmConsumer* pOmmConsumer = createOmmConsumer( testParams.constructorType, consProgConfig );

		/* Check that OmmConsumer is created */
		ASSERT_NE( pOmmConsumer, nullptr ) << "Expected OmmConsumer to be initialized.";
		//cout << "OmmConsumer created with constructor type: " << testParams.constructorType << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pOmmConsumer->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* When the test client assigned for the consumer we can check the message queue */
		if ( hasTestClient(testParams.constructorType) )
		{
			ASSERT_NE(0, pConsumerTestClient->getMessageQueueSize());
			//cout << "Message queue size: " << pConsumerTestClient->getMessageQueueSize() << endl;

			/* Checks to see that the consumer got a OPEN/OK login refresh message */
			while (Msg* msg = pConsumerTestClient->popMsg())
			{
				if (msg->getDataType() == DataType::StatusMsgEnum)
					continue;

				ASSERT_EQ(msg->getDomainType(), RsslDomainTypes::RSSL_DMT_LOGIN);
				ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
				RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(msg);
				ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
				ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
			}
		}

		if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
		}
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}	
}

/*  Consumer connects to Providers using a round robin algorithm. */
/*  The test configures the consumer to connect to a set of providers. */
/*  This test does the following:
 *  1. Starts up the set of providers
 *  2. Starts up an omm-consumer with a lot of providers, configured via ChannelSet
 *  3. Checks to see that the consumer does intitialize
 *  4. Checks to see that the consumer has only one active channel and that it got a OPEN/OK login refresh message
 *  5. Shut down the 1-st provider fully
 *  6. Check that the consumer switches to the next provider and that it got a OPEN/OK login refresh message
 *  7. Restore/Up the 1-st provider (previous)
 *  8. Shut down the 2-nd provider fully
 *  Repeats steps 6-8 for all the providers, checking that the consumer switches to the next provider (round robin algorithm)
 */
TEST_P(OmmConsumerCreateTestFixture, ConsumerRoundRobinProvidersBreakConnection)
{
	const OmmConsumerCreateTestParams& testParams = GetParam();
	unsigned i, j, k;

	const unsigned NUMConnections = 3;  // The number of providers and channels available to the consumer
	const unsigned startPortNum = 14220;

	Map configIProvMap;
	IProviderProgrammaticTestConfig iprovProgConfig;

	vector< shared_ptr<ProviderTestComponents> > providers;
	
	Msg* msg;
	RefreshMsg* refreshMsg;

	try
	{
		/* Create a programmatic configuration. It is used for all the Providers */
		iprovProgConfig.numProviders = NUMConnections;
		iprovProgConfig.startPortNum = startPortNum;

		iprovProgConfig.createProgrammaticConfig(configIProvMap);
		//std::cout << "IProvider programmatic configuration created." << std::endl;

		/* Create the set of test Providers */
		for (i = 0; i < NUMConnections; i++)
		{
			/* Create Provider and its components: IProviderTestClientBase, OmmIProviderConfig */
			std::string providerProgTestName = "ProviderProgrammaticTest_" + std::to_string(i + 1);
			ProviderTestComponents* providerTestComponent = new ProviderTestComponents();
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers.emplace_back(providerTestComponent);
			//std::cout << "OmmProvider_" << (i + 1) << ": created ProviderTestComponents and added it to vector providers." << std::endl;
		}

		/* Create OmmConsumer test instance */
		ConsumerProgrammaticTestConfig consProgConfig;
		//consProgConfig.channelInitializationTimeout = 3;
		consProgConfig.numChannels = NUMConnections;
		consProgConfig.startPortNum = startPortNum;
		//consProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;
	
		OmmConsumer* pOmmConsumer = nullptr;
		pOmmConsumer = createOmmConsumer(testParams.constructorType, consProgConfig);

		/* Check that OmmConsumer is created */
		ASSERT_NE(pOmmConsumer, nullptr) << "Expected OmmConsumer to be initialized.";
		//std::cout << "OmmConsumer created with constructor type: " << testParams.constructorType << std::endl;

		ChannelInformation channelInfo;
		EmaVector<ChannelInformation> channelInfoList;

		/* Loop to break connection with each provider in round robin manner */
		for (i = 0; i < (NUMConnections + 1); i++)
		{
			j = (i % NUMConnections);  // current provider index: 0, 1, 2, 0
			//std::cout << "OmmProvider_" << (j + 1) << ": checking connection with OmmConsumer." << std::endl;

			/* Provider side */
			/* Check that consumer connected to the j-th provider */
			OmmProvider* pProvider = providers[j]->pProvider;
			ASSERT_NE(pProvider, nullptr) << "Expected OmmProvider to be initialized. j: " << j;
		
			pProvider->getConnectedClientChannelInfo(channelInfoList);
			ASSERT_EQ(channelInfoList.size(), 1) << "Expected that the provider is connected to the consumer.";
			ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* Consumer side */
			/* Check the active channel */
			pOmmConsumer->getChannelInformation(channelInfo);
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << std::endl;
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* When the test client assigned for the consumer we can check the message queue */
			if ( hasTestClient(testParams.constructorType) )
			{
				ASSERT_NE(0, pConsumerTestClient->getMessageQueueSize());
				//std::cout << "Message queue size: " << pConsumerTestClient->getMessageQueueSize() << std::endl;

				/* Checks to see that the consumer got a OPEN/OK login refresh message */
				while ((msg = pConsumerTestClient->popMsg()))
				{
					if (msg->getDataType() == DataType::StatusMsgEnum)
						continue;

					ASSERT_EQ(msg->getDomainType(), RsslDomainTypes::RSSL_DMT_LOGIN);
					ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
					refreshMsg = static_cast<RefreshMsg*>(msg);
					ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
					ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
				}
			}

			/* Stop the connection with the j-th provider */
			providers[j].reset();
			//std::cout << "OmmProvider_" << (j + 1) << ": has been deleted, waiting for consumer switch." << std::endl;

			/* Check that consumer lost connection */
			pOmmConsumer->getChannelInformation(channelInfo);
			//std::cout << "Channel Info. channelState: " << channelInfo.getChannelState() << std::endl;
			ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* Wait for the consumer to switch to the next provider */
			k = 0;
			do {
				testSleep(250);
				pOmmConsumer->getChannelInformation(channelInfo);
				k++;
			} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum  &&  k < 20);
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that consumer connects to the next provider. i: " << i << "; k: " << k;
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << "; k: " << k << std::endl;

			/* Up the deleted provider: re-create the j-th Provider with components */
			std::string providerProgTestName = "ProviderProgrammaticTest_" + std::to_string(j + 1);
			auto providerTestComponent = std::make_shared<ProviderTestComponents>();
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers[j] = providerTestComponent;
		}
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

class OmmConsumerWithConfigIndexTestParams {
public:
	unsigned						configIndex;			// configuration index

	OmmConsumerWithConfigIndexTestParams(unsigned configIdx) :
		configIndex(configIdx)
	{
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend std::ostream& operator<<(std::ostream& out, const OmmConsumerWithConfigIndexTestParams& params)
	{
		out << "["
			<< "configIndex:" << params.configIndex  // << ","
			<< "]";
		return out;
	}
};

class OmmConsumerCreateIndexTestFixture
	: public OmmConsumerCreateTest, public ::testing::WithParamInterface<OmmConsumerWithConfigIndexTestParams>
{
};


/*  Consumer connects to Providers using a round robin algorithm. */
/*  The test configures the consumer to connect to a set of providers. */
/*  The test configures providers to send a LoginRefresh or not. */
/*  This test does the following:
 *  1. Starts up the set of providers
 *  2. Starts up an omm-consumer with a lot of providers, configured via ChannelSet
 *  3. Checks to see that the consumer does intitialize
 *  4. Checks to see that the consumer has only one active channel and that it got a OPEN/OK login refresh message
 *  5. Shut down the active provider fully
 *  6. Check that the consumer switches to the next provider and that it got a OPEN/OK login refresh message
 */
TEST_P(OmmConsumerCreateIndexTestFixture, ConsumerRoundRobinProvidersNotSendLoginRefresh)
{
	const OmmConsumerWithConfigIndexTestParams& testParams = GetParam();
	unsigned i, j, k, s;

	const unsigned NUMConnections = 3;  // The number of providers and channels available to the consumer
	const unsigned startPortNum = 14220;

	Map configIProvMap;
	IProviderProgrammaticTestConfig iprovProgConfig;

	std::string providerProgTestName;
	std::shared_ptr<ProviderTestComponents> providerTestComponent;

	vector< shared_ptr<ProviderTestComponents> > providers;

	ConsumerProgrammaticTestConfig consProgConfig;
	OmmConsumer* pOmmConsumer;
	OmmProvider* pProvider;

	ChannelInformation channelInfo;
	EmaVector<ChannelInformation> channelInfoList;

	Msg* msg;
	RefreshMsg* refreshMsg;

	ASSERT_TRUE(0 <= testParams.configIndex && testParams.configIndex < 3);

	try
	{
		/* P1(false), P2(false), P3(true) */
		/* P1(false), P2(true), P3(false) */
		/* P1(true), P2(false), P3(false) */
		bool sendLoginRefreshPattern[NUMConnections][NUMConnections] =
		{
			{ false, false, true },
			{ false, true, false },
			{ true, false, false }
		};

		/* Create a programmatic configuration. It is used for all the Providers */
		iprovProgConfig.numProviders = NUMConnections;
		iprovProgConfig.startPortNum = startPortNum;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);
		//std::cout << "IProvider programmatic configuration created." << std::endl;

		/* Loop to break connection with each provider in round robin manner */
		/* 3 Steps. */
		i = testParams.configIndex;

		{
			/* Create the set of test Providers */
			providerProgTestName = "ProviderProgrammaticTest_1";
			providerTestComponent = std::make_shared<ProviderTestComponents>();
			providerTestComponent->provTestOptions.sendLoginRefresh = sendLoginRefreshPattern[i][0];
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers.emplace_back(std::move(providerTestComponent));

			providerProgTestName = "ProviderProgrammaticTest_2";
			providerTestComponent = std::make_shared<ProviderTestComponents>();
			providerTestComponent->provTestOptions.sendLoginRefresh = sendLoginRefreshPattern[i][1];
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers.emplace_back(std::move(providerTestComponent));

			providerProgTestName = "ProviderProgrammaticTest_3";
			providerTestComponent = std::make_shared<ProviderTestComponents>();
			providerTestComponent->provTestOptions.sendLoginRefresh = sendLoginRefreshPattern[i][2];
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers.emplace_back(std::move(providerTestComponent));

			/* Create OmmConsumer test instance */
			//consProgConfig.loginRequestTimeOut = 3000; // 3 seconds - timeout for login request
			consProgConfig.channelInitializationTimeout = 1;
			consProgConfig.numChannels = NUMConnections;
			consProgConfig.startPortNum = startPortNum;
			consProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

			pOmmConsumer = createOmmConsumer(OmmConsumerConstructorType::config_adminClient_errorClient, consProgConfig);

			/* Check that OmmConsumer is created */
			ASSERT_NE(pOmmConsumer, nullptr) << "Expected OmmConsumer to be initialized.";
			//std::cout << "OmmConsumer created with constructor type: config_adminClient_errorClient" << std::endl;

			/* Wait for the consumer to switch to the next provider */
			k = 0;
			do {
				testSleep(250);
				pOmmConsumer->getChannelInformation(channelInfo);
				k++;
			} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that consumer connects to the start provider. k: " << k;
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << "; k: " << k << std::endl;

			// current active provider index: 2, 1, 0

			/* When the test client assigned for the consumer we can check the message queue */
			//if ( hasTestClient(testParams.constructorType) )
			{
				ASSERT_NE(0, pConsumerTestClient->getMessageQueueSize());
				//std::cout << "Message queue size: " << pConsumerTestClient->getMessageQueueSize() << std::endl;

				/* Checks to see that the consumer got a OPEN/OK login refresh message */
				while ((msg = pConsumerTestClient->popMsg()))
				{
					if (msg->getDataType() == DataType::StatusMsgEnum)
						continue;

					ASSERT_EQ(msg->getDomainType(), RsslDomainTypes::RSSL_DMT_LOGIN);
					ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
					refreshMsg = static_cast<RefreshMsg*>(msg);
					ASSERT_EQ(refreshMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
					ASSERT_EQ(refreshMsg->getState().getDataState(), OmmState::DataState::OkEnum);
				}
			}

			/* Provider side */
			/* Look for a provider that is configured as active */
			for (j = 0; j < NUMConnections; j++)
			{
				pProvider = providers[j]->pProvider;
				ASSERT_NE(pProvider, nullptr) << "Expected OmmProvider to be initialized. j: " << j;

				if ( providers[j]->provTestOptions.sendLoginRefresh )
					break;
			}
			ASSERT_TRUE(j < NUMConnections) << "Error: active provider is absent.";
			//std::cout << "Active OmmProvider_" << (j + 1) << ": checking connection with OmmConsumer." << std::endl;

			switch (i) {
			case 0: ASSERT_EQ(j, 2); break; // Provider3 is active
			case 1: ASSERT_EQ(j, 1); break; // Provider2 is active
			case 2: ASSERT_EQ(j, 0); break; // Provider1 is active
			}
			
			channelInfoList.clear();
			pProvider->getConnectedClientChannelInfo(channelInfoList);
			ASSERT_EQ(channelInfoList.size(), 1) << "Expected that the provider is connected to the consumer.";
			ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* Prepare the next step */
			/* Restart one of the providers that is not configured to send a Login refresh */
			s = (j + 1) % NUMConnections; // index of the provider to restart

			providers[s].reset();
			//std::cout << "OmmProvider_" << (s + 1) << ": has been deleted." << std::endl;
			/* Up the deleted provider: re-create the s-th Provider with components */
			providerProgTestName = "ProviderProgrammaticTest_" + std::to_string(s + 1);
			providerTestComponent = std::make_shared<ProviderTestComponents>();
			providerTestComponent->provTestOptions.sendLoginRefresh = true;
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers[s] = providerTestComponent;
			//std::cout << "OmmProvider_" << (s + 1) << ": is re-created. It is configured as active." << std::endl;

			testSleep(100);

			/* Stop the current connection with the j-th provider */
			providers[j].reset();
			//std::cout << "OmmProvider_" << (j + 1) << ": has been deleted, waiting for consumer switch." << std::endl;

			/* Check that consumer lost connection */
			pOmmConsumer->getChannelInformation(channelInfo);
			//std::cout << "Channel Info. channelState: " << channelInfo.getChannelState() << std::endl;
			ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* Wait for the consumer to switch to the next provider */
			k = 0;
			do {
				testSleep(250);
				pOmmConsumer->getChannelInformation(channelInfo);
				k++;
			} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that consumer connects to the next provider. i: " << i << "; k: " << k;
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << "; k: " << k << std::endl;
		}
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/*  Consumer and Provider exchange Generic messages. */
/*  This test does the following:
 *  1. Starts up an omm-consumer with a login client with one provider
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has only one active channel
 *  3. Consumer opens an item stream for sending a Generic message
 *   a. Provider receives the request and sends a refresh
 *   b. Consumer receives the refresh message
 *  4. Consumer sends a Generic message
 *  5. Check that provider receives the Generic message
 *  6. Provider sends a Generic message
 *  7. Check that consumer receives the Generic message
 */
TEST_F(OmmConsumerCreateTest, ConsumerProviderGenericMsg)
{
	try
	{
		ReqMsg consRequest;
		GenericMsg encodeGenericMsg;

		Msg* msg;
		RefreshMsg* refreshMsg;
		GenericMsg* genericMsg;
		ReqMsg* reqMsg;

		ProviderTestOptions provTestOptions;
		IProviderTestClientForGeneric provClient(provTestOptions);

		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);

		OmmIProviderConfig provConfig("EmaConfigTest.xml");
		provConfig.config(configIProvMap);
		provConfig.providerName("ProviderProgrammaticTest_1");

		/* Create IProvider instance */
		OmmProvider prov(provConfig, provClient);
		//cout << "OmmProvider created." << endl;

		/* Create OmmConsumer instance */
		ConsumerProgrammaticTestConfig consProgConfig;
		OmmConsumer* pOmmConsumer = createOmmConsumer(OmmConsumerConstructorType::config_errorClient, consProgConfig);

		/* Check that OmmConsumer is created */
		ASSERT_NE(pOmmConsumer, nullptr) << "Expected OmmConsumer to be initialized.";
		//cout << "OmmConsumer created." << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pOmmConsumer->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* Clear message queues */
		provClient.clear();
		pConsumerTestClient->clear();

		/* Consumer opens an item stream for sending a Generic message */
		consRequest.name("GenericItem1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 itemHandle = pOmmConsumer->registerClient(consRequest, *pConsumerTestClient);
		//cout << "Item handle: " << itemHandle << " (0x" << std::hex << itemHandle << ")" << std::dec << endl;
		ASSERT_NE(itemHandle, 0);

		/* Provider receives the request and sends a refresh */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);

		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		msg = provClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "GenericItem1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 1);

		/* Consumer receives the refresh msg */
		while (pConsumerTestClient->getMessageQueueSize() == 0)
		{
			testSleep(200);
		}

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "GenericItem1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		/* Consumer sends a Generic message */
		encodeGenericMsg.clear().name("TestConsumerGenericItem").complete();

		pOmmConsumer->submit(encodeGenericMsg, itemHandle);

		/* Check that provider receives the Generic message */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);
		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 0);

		msg = provClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestConsumerGenericItem");
		ASSERT_FALSE(genericMsg->hasServiceId());
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);

		/* Provider sends a Generic message */
		encodeGenericMsg.clear().name("TestProviderGenericItem").complete();

		UInt64 provGenericItemHandle = provClient.getGenericItemHandle();
		//cout << "Provider client genericItemHandle: " << provGenericItemHandle << " (0x" << std::hex << provGenericItemHandle << ")" << std::dec << endl;
		ASSERT_NE(provGenericItemHandle, 0);

		prov.submit(encodeGenericMsg, provGenericItemHandle);
		
		/* Check that consumer receives the Generic message */
		do {
			testSleep(200);
		} while (pConsumerTestClient->getMessageQueueSize() == 0);
		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		genericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(genericMsg->getName(), "TestProviderGenericItem");
		ASSERT_FALSE(genericMsg->hasServiceId());
		ASSERT_TRUE(genericMsg->getComplete());
		ASSERT_EQ(genericMsg->getPayload().getDataType(), DataType::NoDataEnum);


		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/*  Consumer and Provider exchange on-Stream Post messages. */
/*  This test does the following:
 *  1. Starts up an omm-consumer with a login client with one provider
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has only one active channel
 *  3. Consumer opens an item stream for sending a Post message (On-Stream Post)
 *   a. Provider receives the request and sends a refresh
 *   b. Consumer receives the refresh message
 *  4. Consumer sends a Post message
 *  5. Check that provider receives the Post message
 *  6. Consumer sends a post message and wants an acknowledge answer msg
 *  7. Check that consumer receives only one Ack message
 */
TEST_F(OmmConsumerCreateTest, ConsumerProviderPostOnStream)
{
	try
	{
		ReqMsg consRequest;
		PostMsg encodePostMsg;

		Msg* msg;
		RefreshMsg* refreshMsg;
		PostMsg* postMsg;
		ReqMsg* reqMsg;
		AckMsg* ackMsg;

		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		ProviderTestOptions provTestOptions;
		IProviderTestClientForGeneric provClient(provTestOptions);

		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);

		OmmIProviderConfig provConfig("EmaConfigTest.xml");
		provConfig.config(configIProvMap);
		provConfig.providerName("ProviderProgrammaticTest_1");

		/* Create IProvider instance */
		OmmProvider prov(provConfig, provClient);
		//cout << "OmmProvider created." << endl;

		/* Create OmmConsumer instance */
		ConsumerProgrammaticTestConfig consProgConfig;
		OmmConsumer* pOmmConsumer = createOmmConsumer(OmmConsumerConstructorType::config_errorClient, consProgConfig);

		/* Check that OmmConsumer is created */
		ASSERT_NE(pOmmConsumer, nullptr) << "Expected OmmConsumer to be initialized.";
		//cout << "OmmConsumer created." << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pOmmConsumer->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* Clear message queues */
		provClient.clear();
		pConsumerTestClient->clear();

		/* Consumer opens an item stream for sending a Post message (On-Stream Post) */
		consRequest.name("PostItem1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");

		UInt64 itemHandle = pOmmConsumer->registerClient(consRequest, *pConsumerTestClient);
		//cout << "Item handle: " << itemHandle << " (0x" << std::hex << itemHandle << ")" << std::dec << endl;
		ASSERT_NE(itemHandle, 0);

		/* Provider receives the request and sends a refresh */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);

		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		msg = provClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::ReqMsgEnum);
		reqMsg = static_cast<ReqMsg*>(msg);
		ASSERT_EQ(reqMsg->getName(), "PostItem1");
		ASSERT_EQ(reqMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(reqMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(reqMsg->getServiceId(), 1);

		/* Consumer receives the refresh msg */
		while (pConsumerTestClient->getMessageQueueSize() == 0)
		{
			testSleep(200);
		}

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getName(), "PostItem1");
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_MARKET_PRICE);
		ASSERT_EQ(refreshMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(refreshMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(refreshMsg->getComplete());

		/* Consumer sends a post message */
		encodePostMsg.postId(1).serviceName("DIRECT_FEED").name("TestConsumerPostItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		pOmmConsumer->submit(encodePostMsg, itemHandle);
		//cout << "OmmConsumer sent Post msg." << endl;

		/* Check that provider receives the Post message */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);
		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 0);

		msg = provClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestConsumerPostItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		/* Extra check that Consumer do not receive any messages */
		testSleep(300);
		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 0) << "Do not expect that Consumer received any messages from provider side.";
		ASSERT_EQ(provClient.getMessageQueueSize(), 0);

		/* Consumer sends a post message and wants an acknowledge answer msg */
		encodePostMsg.clear().postId(2).serviceName("DIRECT_FEED").name("TestConsumerPostItem1").solicitAck(true).payload(encodeInnerUpdate).complete();

		pOmmConsumer->submit(encodePostMsg, itemHandle);
		//cout << "OmmConsumer sent Post msg 2." << endl;

		/* Check that provider receives the Post message */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);
		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		//cout << "OmmConsumer is waiting for Ack msg." << endl;
		/* Check that consumer receives only one Ack message */
		while (pConsumerTestClient->getMessageQueueSize() == 0)
		{
			testSleep(200);
		}
		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		//cout << "OmmConsumer received msg." << endl;
		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::AckMsgEnum);
		ackMsg = static_cast<AckMsg*>(msg);
		ASSERT_EQ(ackMsg->getName(), "TestConsumerPostItem1");
		ASSERT_EQ(ackMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(ackMsg->getServiceId(), (UInt32)1);
		ASSERT_EQ(ackMsg->getAckId(), 2);
		

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/*  Consumer and Provider exchange off-Stream Post messages. */
/*  This test does the following:
 *  1. Starts up an omm-consumer with a login client with one provider
 *  2. Checks to see that the consumer does intitialize
 *  3. Checks to see that the consumer has only one active channel
 *  3. Consumer opens a stream on Login domain for sending a Post message (Off-Stream Post)
 *  4. Consumer sends a Post message
 *  5. Check that provider receives the Post message
 *  6. Consumer sends a post message and wants an acknowledge answer msg
 *  7. Check that consumer receives only one Ack message
 */
TEST_F(OmmConsumerCreateTest, ConsumerProviderPostOffStream)
{
	try
	{
		ReqMsg consRequest;
		PostMsg encodePostMsg;

		Msg* msg;
		RefreshMsg* refreshMsg;
		PostMsg* postMsg;
		//ReqMsg* reqMsg;
		AckMsg* ackMsg;

		UpdateMsg encodeInnerUpdate;
		FieldList encodeFieldList;

		encodeInnerUpdate.payload(
			FieldList().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete());

		ProviderTestOptions provTestOptions;
		IProviderTestClientForGeneric provClient(provTestOptions);

		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);

		OmmIProviderConfig provConfig("EmaConfigTest.xml");
		provConfig.config(configIProvMap);
		provConfig.providerName("ProviderProgrammaticTest_1");

		/* Create IProvider instance */
		OmmProvider prov(provConfig, provClient);
		//cout << "OmmProvider created." << endl;

		/* Create OmmConsumer instance */
		ConsumerProgrammaticTestConfig consProgConfig;
		OmmConsumer* pOmmConsumer = createOmmConsumer(OmmConsumerConstructorType::config_errorClient, consProgConfig);

		/* Check that OmmConsumer is created */
		ASSERT_NE(pOmmConsumer, nullptr) << "Expected OmmConsumer to be initialized.";
		//cout << "OmmConsumer created." << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pOmmConsumer->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* Clear message queues */
		provClient.clear();
		pConsumerTestClient->clear();

		/* Consumer opens a stream on Login domain for sending a Post message (Off-Stream Post) */
		consRequest.domainType(MMT_LOGIN);

		UInt64 loginHandle = pOmmConsumer->registerClient(consRequest, *pConsumerTestClient);
		//cout << "Login handle: " << loginHandle << " (0x" << std::hex << loginHandle << ")" << std::dec << endl;
		ASSERT_NE(loginHandle, 0);

		/* Provider receives the request and sends a refresh */
		/* Consumer receives the refresh msg */
		while (pConsumerTestClient->getMessageQueueSize() == 0)
		{
			testSleep(200);
		}

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::RefreshMsgEnum);
		refreshMsg = static_cast<RefreshMsg*>(msg);
		ASSERT_EQ(refreshMsg->getDomainType(), MMT_LOGIN);
		ASSERT_FALSE(refreshMsg->hasServiceId());
		ASSERT_FALSE(refreshMsg->hasServiceName());
		ASSERT_TRUE(refreshMsg->getComplete());

		/* Consumer sends a post message */
		encodePostMsg.postId(1).serviceName("DIRECT_FEED").name("TestConsumerPostItem").solicitAck(false).payload(encodeInnerUpdate).complete();

		pOmmConsumer->submit(encodePostMsg, loginHandle);
		//cout << "OmmConsumer sent Post msg." << endl;

		/* Check that provider receives the Post message */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);
		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 0);

		msg = provClient.popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::PostMsgEnum);
		postMsg = static_cast<PostMsg*>(msg);
		ASSERT_EQ(postMsg->getName(), "TestConsumerPostItem");
		ASSERT_EQ(postMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(postMsg->getServiceId(), (UInt32)1);
		ASSERT_TRUE(postMsg->getComplete());
		ASSERT_EQ(postMsg->getPayload().getDataType(), DataType::UpdateMsgEnum);
		ASSERT_EQ(postMsg->getPayload().getUpdateMsg().getPayload().getDataType(), DataType::FieldListEnum);

		/* Extra check that Consumer do not receive any messages */
		testSleep(300);
		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 0) << "Do not expect that Consumer received any messages from provider side.";
		ASSERT_EQ(provClient.getMessageQueueSize(), 0);

		/* Consumer sends a post message and wants an acknowledge answer msg (ACK) */
		encodePostMsg.clear().postId(2).serviceName("DIRECT_FEED").name("TestConsumerPostItem1").solicitAck(true).payload(encodeInnerUpdate).complete();

		pOmmConsumer->submit(encodePostMsg, loginHandle);
		//cout << "OmmConsumer sent Post msg 2." << endl;

		/* Check that provider receives the Post message */
		do {
			testSleep(200);
		} while (provClient.getMessageQueueSize() == 0);
		ASSERT_EQ(provClient.getMessageQueueSize(), 1);

		//cout << "OmmConsumer is waiting for Ack msg." << endl;
		/* Check that consumer receives only one Ack message */
		while (pConsumerTestClient->getMessageQueueSize() == 0)
		{
			testSleep(200);
		}
		ASSERT_EQ(pConsumerTestClient->getMessageQueueSize(), 1);

		msg = pConsumerTestClient->popMsg();

		ASSERT_EQ(msg->getDataType(), DataType::AckMsgEnum);
		ackMsg = static_cast<AckMsg*>(msg);
		ASSERT_EQ(ackMsg->getName(), "TestConsumerPostItem1");
		ASSERT_EQ(ackMsg->getServiceName(), "DIRECT_FEED");
		ASSERT_EQ(ackMsg->getServiceId(), (UInt32)1);
		ASSERT_EQ(ackMsg->getAckId(), 2);


		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
	}
	catch (const OmmException& exception)
	{
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

INSTANTIATE_TEST_SUITE_P(
	OmmConsumer,
	OmmConsumerCreateTestFixture,
	::testing::Values(
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_client ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_oAuthClient ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_oAuthClient_errorClient ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_errorClient ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_adminClient_oAuthClient ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_adminClient_errorClient ),
		OmmConsumerCreateTestParams( OmmConsumerConstructorType::config_adminClient_oAuthClient_errorClient )
	)
);

INSTANTIATE_TEST_SUITE_P(
	OmmConsumer,
	OmmConsumerCreateIndexTestFixture,
	::testing::Values(
		OmmConsumerWithConfigIndexTestParams( 0 ),
		OmmConsumerWithConfigIndexTestParams( 1 ),
		OmmConsumerWithConfigIndexTestParams( 2 )
	)
);
