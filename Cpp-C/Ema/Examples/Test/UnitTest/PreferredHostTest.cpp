/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2024 LSEG. All rights reserved.	                  --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Thread.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

static RsslUInt64 itemHandle = 0;
static EmaString channelInfo;

static void sleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}

class PreferredHostTest : public ::testing::Test {
public:
	static DataDictionary dataDictionary;
	static const char* emaConfigTest;

	void SetUp()
	{
		try
		{
			dataDictionary.loadFieldDictionary("RDMFieldDictionaryTest");
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
		dataDictionary.clear();
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
	}
};

class OmmConsumerTestClientPH : public refinitiv::ema::access::OmmConsumerClient
{
public:
	OmmConsumerTestClientPH() :updateCalled(false) {};

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
	{
		channelInfo = event.getChannelInformation();
	}

	void onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
	{
		if (!updateCalled)
		{
			updateCalled = true;
			channelInfo = event.getChannelInformation();
		}
	}

	void onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
	{
		channelInfo = event.getChannelInformation();
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
			if (count == 10)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != channelInfo.find("14003"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: non-preferred"));

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14002"), providerCallback1);

		/*Wait consumer fall back to the preferred host*/
		count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 10)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != channelInfo.find("14002"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time schedule: */10 * * * * *"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time interval: 10"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph channel name: Channel_13"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph wsb channel name:"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph remaining detection time:"));

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
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14002"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 10)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != channelInfo.find("14002"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(5);
		phOptions.phDetectionTimeSchedule("*/5 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		/*Wait 10 sec while consumer fall back to new preferred host*/
		count = 0;
		while (count != 10)
		{
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != channelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time schedule: */5 * * * * *"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time interval: 5"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph remaining detection time:"));

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
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14002"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 10)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != channelInfo.find("14002"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(50);
		phOptions.phDetectionTimeSchedule("*/50 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		consumer.fallbackPreferredHost();

		/*Wait 10 sec while consumer fall back to new preferred host. 
		Fall back would happen before the time set in schedule because fallbackPreferredHost called  */

		count = 0;
		while (count != 15)
		{
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != channelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time schedule: */50 * * * * *"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph detection time interval: 50"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != channelInfo.find("ph remaining detection time:"));

	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}
