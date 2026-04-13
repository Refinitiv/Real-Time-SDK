/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2024, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ADHSimulator.h"
#include "TestUtilities.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslTransport.h"
#include <thread>
#include <atomic>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

static RsslUInt64 itemHandle = 0;
static EmaString consChannelInfo;
static EmaString iprovChannelInfo;
static EmaString niprovChannelInfo;

class PreferredHostTest : public ::testing::Test {
public:
	static DataDictionary dataDictionary;
	static const char* emaConfigTest;

	void SetUp()
	{
		try
		{
			if (!dataDictionary.isFieldDictionaryLoaded())
				dataDictionary.loadFieldDictionary("RDMFieldDictionaryTest");
			if (!dataDictionary.isEnumTypeDefLoaded())
				dataDictionary.loadEnumTypeDictionary("enumtypeTest.def");
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in EmaMsgPackingTest load dictionary";
		}
	}

	void TearDown()
	{
	}
};

DataDictionary PreferredHostTest::dataDictionary;
const char* PreferredHostTest::emaConfigTest = "./EmaConfigTest.xml";

class OmmProviderTestClientPH : public refinitiv::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
			attrib(ElementList().complete()).solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
			event.getHandle());
	}

	void processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		if (itemHandle != 0)
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}

		event.getProvider().submit(RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
			state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
			payload(FieldList().
				addReal(22, 3990, OmmReal::ExponentNeg2Enum).
				addReal(25, 3994, OmmReal::ExponentNeg2Enum).
				addReal(30, 9, OmmReal::Exponent0Enum).
				addReal(31, 19, OmmReal::Exponent0Enum).
				complete()).
			complete(), event.getHandle());

		itemHandle = event.getHandle();
	}

	void processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		event.getProvider().submit(StatusMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
			domainType(reqMsg.getDomainType()).
			state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found"),
			event.getHandle());
	}

	void onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		switch (reqMsg.getDomainType())
		{
		case MMT_LOGIN:
			processLoginRequest(reqMsg, event);
			break;
		case MMT_MARKET_PRICE:
			processMarketPriceRequest(reqMsg, event);
			break;
		default:
			processInvalidItemRequest(reqMsg, event);
			break;
		}

		iprovChannelInfo = event.getChannelInformation();
	}
};

class OmmConsumerTestClientPH : public refinitiv::ema::access::OmmConsumerClient
{
public:
	OmmConsumerTestClientPH() :updateCalled(false) {};

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
	{
		consChannelInfo = event.getChannelInformation();
	}

	void onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
	{
		if (!updateCalled)
		{
			updateCalled = true;
			consChannelInfo = event.getChannelInformation();
		}
	}

	void onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
	{
		consChannelInfo = event.getChannelInformation();
	}

	bool updateCalled;
};


TEST_F(PreferredHostTest, PreferredHostTest_SwitchToPreferred_ChannelInfo)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: non-preferred"));

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback1);

		/*Wait consumer fall back to the preferred host*/
		count = 0;
		while (count != 15)
		{
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));

		//check consumer PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */10 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_13"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph wsb channel name:"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

		//check provider PH info. Values would be default cause PH is not applicable to provider.
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph preferred host option: disabled"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph detection time schedule: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph detection time interval: 0"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph channel name: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph wsb channel name: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph is channel preferred: non-preferred"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph remaining detection time: 0"));


	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}

TEST_F(PreferredHostTest, PreferredHostTest_ModifiedPHWithIOCTL)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(5);
		phOptions.phDetectionTimeSchedule("*/5 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		/*Wait 10 sec while consumer fall back to new preferred host*/
		count = 0;
		while (count != 11)
		{
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}



		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */5 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 5"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}

TEST_F(PreferredHostTest, PreferredHostTest_SetPHWithIOCtlAndPerformFallback)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(50);
		phOptions.phDetectionTimeSchedule("*/50 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		consumer.dispatch(1000);
		testSleep(1000);

		consumer.fallbackPreferredHost();

		/*Wait while consumer fall back to new preferred host.
		Fall back would happen before the time set in schedule because fallbackPreferredHost called  */

		count = 0;
		while (count != 20)
		{
			consumer.dispatch(1000);
			testSleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */50 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 50"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}


class OmmNIProviderTestClientPH : public refinitiv::ema::access::OmmProviderClient
{
	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}
};


TEST_F(PreferredHostTest, PreferredHostTest_ChannelInfo_NIProv)
{
	try
	{
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);
		ADHSimulatorOptions adhOpts(&reactorOpts, (char*)"14003");

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while ( !adh.isRunning() && k++ < 20 )
		{
			testSleep(200);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		niprovChannelInfo.clear();
		OmmNIProviderTestClientPH niProviderCallback;
		OmmProvider niProvider(OmmNiProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").host("localhost:14003").username("user"), niProviderCallback);

		//check niprovider PH info. Values would be default cause PH is not applicable to niprovider.
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph preferred host option: disabled"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph detection time schedule: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph detection time interval: 0"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph channel name: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph wsb channel name: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph is channel preferred: non-preferred"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph remaining detection time: 0"));
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "Exception NOT expected : " << excp;
	}
}
