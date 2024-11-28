/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2023, 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Thread.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

static UInt64 clientHandle = 0;
static UInt64 itemHandle = 0;
static const Int32 packedMsgNum = 10;

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

class EmaMsgPackingTest : public ::testing::Test {
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

DataDictionary EmaMsgPackingTest::dataDictionary;
const char* EmaMsgPackingTest::emaConfigTest = "./EmaConfigTest.xml";

class OmmProviderTestClient : public refinitiv::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
			attrib(ElementList().complete()).solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
			event.getHandle());

		clientHandle = event.getClientHandle();
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

class OmmConsumerTestClient : public refinitiv::ema::access::OmmConsumerClient
{
public:

	OmmConsumerTestClient():_updateMsgCount(0)
	{
		memset(_exponent0Enum, 0, packedMsgNum);
	}

	void onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent&)
	{
		if (_updateMsgCount > packedMsgNum)
		{
			EXPECT_TRUE(false) << "Received more UpdateMsg than expected. Received: " << _updateMsgCount;
		}

		const FieldList &fList = updateMsg.getPayload().getFieldList();
		while (fList.forth())
		{
			_exponent0Enum[_updateMsgCount] = atoi(fList.getEntry().getReal().toString().c_str());
		}
		_updateMsgCount++;
	}

	Int32 _updateMsgCount;
	Int32 _exponent0Enum[packedMsgNum];
};

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_Encoding_Decoding)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14003"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14003").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;


		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		FieldList fieldList;
		UpdateMsg msg;
		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle);
		Int32 exponent0Enum[packedMsgNum] = { 0 };

		// Pack Refresh Message first
		fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(25, 3994, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(30, 9, OmmReal::Exponent0Enum);
		fieldList.addReal(31, 19, OmmReal::Exponent0Enum);
		fieldList.complete();

		packedMsg.addMsg(RefreshMsg().serviceName("DIRECT_FEED").name("IBM.N").solicited(true).
			state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
			payload(fieldList).
			complete(), itemHandle);

		for (Int32 i = 0; i < packedMsgNum; i++)
		{
			msg.clear();
			fieldList.clear();
			fieldList.addReal(30, 10 + i, OmmReal::Exponent0Enum);
			exponent0Enum[i] = 10 + i;
			fieldList.complete();

			msg.serviceName("DIRECT_FEED").name("IBM.N");
			msg.payload(fieldList);

			(void)packedMsg.addMsg(msg, itemHandle);
		}

		EXPECT_TRUE(packedMsg.maxSize() > packedMsg.remainingSize());
		EXPECT_TRUE(packedMsg.packedMsgCount() == packedMsgNum + 1); // 1 RefreshMsg + 10 UpdateMsg

		provider.submit(packedMsg);
		packedMsg.clear();
		consumer.dispatch(1000);
		sleep(1000);

		EXPECT_TRUE(packedMsg.maxSize() == 6000);
		EXPECT_TRUE(packedMsg.remainingSize() == 0);
		EXPECT_TRUE(packedMsg.packedMsgCount() == 0);

		EXPECT_TRUE(packedMsgNum == consumerCallback._updateMsgCount);

		for (Int32 i = 0; i < packedMsgNum; i++)
		{
			EXPECT_TRUE(exponent0Enum[i] == consumerCallback._exponent0Enum[i])
				<< "Failed: "<< exponent0Enum[i] << " != " << consumerCallback._exponent0Enum[i];
		}
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PackedMsg Encoding & Decoding -- exception NOT expected : " << excp;
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_BufferOverflow)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14004"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14004").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;

		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		FieldList flist;
		UpdateMsg msg;
		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle, 10);
		Int32 exponent0Enum[packedMsgNum] = { 0 };

		for (Int32 i = 0; i < packedMsgNum; i++)
		{
			msg.clear();
			flist.clear();
			flist.addReal(30, 10 + i, OmmReal::Exponent0Enum);
			exponent0Enum[i] = 10 + i;
			flist.complete();

			msg.serviceName("DIRECT_FEED").name("IBM.N");
			msg.payload(flist);

			(void)packedMsg.addMsg(msg, itemHandle);
		}

		EXPECT_TRUE(false) << "PackedMsg buffer overflow -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Failed rsslEncodeBuffer(). Buffer too small.");
		EXPECT_TRUE(strPos != EmaString::npos) << "PackedMsg buffer overflow get wrong exception: "<< excp.getText();
	}
}


TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_ChannelNotActive)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14004"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14004").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;

		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		PackedMsg packedMsg(provider);

		provider.submit(packedMsg);
		packedMsg.clear();
		consumer.dispatch(1000);
		sleep(1000);


		EXPECT_TRUE(false) << "PackedMsg channel not set -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Attempt to submit PackedMsg with non set channel");
		EXPECT_TRUE(strPos != EmaString::npos) << "Attempt to submit PackedMsg with non set channel != " << excp.getText();
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_BufferNotSet)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14004"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14004").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;

		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		UpdateMsg msg;
		PackedMsg packedMsg(provider);

		(void)packedMsg.addMsg(msg, itemHandle);
		provider.submit(packedMsg);
		packedMsg.clear();
		consumer.dispatch(1000);
		sleep(1000);


		EXPECT_TRUE(false) << "initBuffer() was not called -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("addMsg() fails because initBuffer() was not called.");
		EXPECT_TRUE(strPos != EmaString::npos) << "addMsg() fails because initBuffer() was not called. != " << excp.getText();
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_WrongClientHandle)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14003"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14003").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;


		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle+1);
		Int32 exponent0Enum[packedMsgNum] = { 0 };

		EXPECT_TRUE(false) << "PackedMsg wrong client handle -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Client handle is not valid");
		EXPECT_TRUE(strPos != EmaString::npos) << "Client handle is not valid. != " << excp.getText();
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_WrongItemHandle)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14003"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14003").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;


		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		FieldList flist;
		UpdateMsg msg;
		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle);
		Int32 exponent0Enum =  0 ;

		flist.clear();
		flist.addReal(30, 10, OmmReal::Exponent0Enum);
		exponent0Enum = 10;
		flist.complete();

		msg.serviceName("DIRECT_FEED").name("IBM.N");
		msg.payload(flist);

		(void)packedMsg.addMsg(msg, itemHandle+1);

		EXPECT_TRUE(false) << "PackedMsg wrong item handle -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Incorrect handler incoming message.");
		EXPECT_TRUE(strPos != EmaString::npos) << "Incorrect handler incoming message. != " << excp.getText();
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_WrongServiceName)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14003"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14003").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;


		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		FieldList fieldList;
		UpdateMsg msg;
		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle);
		Int32 exponent0Enum[packedMsgNum] = { 0 };

		// Pack Refresh Message first
		fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(25, 3994, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(30, 9, OmmReal::Exponent0Enum);
		fieldList.addReal(31, 19, OmmReal::Exponent0Enum);
		fieldList.complete();

		packedMsg.addMsg(RefreshMsg().serviceName("DIRECT_FEED_123").name("IBM.N").solicited(true).
			state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
			payload(fieldList).
			complete(), itemHandle);

		EXPECT_TRUE(false) << "PackedMsg wrong service name -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Attempt to add RSSL_MC_REFRESH with service name of DIRECT_FEED_123 that was not included in the SourceDirectory. Dropping this \"RSSL_MC_REFRESH\"");
		EXPECT_TRUE(strPos != EmaString::npos) << "Attempt to add RSSL_MC_REFRESH with service name of DIRECT_FEED_123 that was not included in the SourceDirectory. Dropping this \"RSSL_MC_REFRESH\" != " << excp.getText();
	}
}

TEST_F(EmaMsgPackingTest, EmaMsgPackingTest_WrongServiceId)
{
	try
	{
		clientHandle = 0;
		itemHandle = 0;

		OmmProviderTestClient providerCallback;
		OmmProvider provider(OmmIProviderConfig(emaConfigTest).port("14003"), providerCallback);

		OmmConsumerTestClient consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig().dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).host("localhost:14003").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;


		while (clientHandle == 0 || itemHandle == 0)
		{
			/*Timeout 5 sec*/
			if (count == 10) FAIL() << "UNABLE TO CONNECT TO CLIENT";
			consumer.dispatch(500);
			sleep(500);
			count++;
		}

		FieldList fieldList;
		UpdateMsg msg;
		PackedMsg packedMsg(provider);
		packedMsg.initBuffer(clientHandle);
		Int32 exponent0Enum[packedMsgNum] = { 0 };

		// Pack Refresh Message first
		fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(25, 3994, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(30, 9, OmmReal::Exponent0Enum);
		fieldList.addReal(31, 19, OmmReal::Exponent0Enum);
		fieldList.complete();

		packedMsg.addMsg(RefreshMsg().serviceId(12345).name("IBM.N").solicited(true).
			state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
			payload(fieldList).
			complete(), itemHandle);

		EXPECT_TRUE(false) << "PackedMsg wrong service id -- exception expected";
	}
	catch (const OmmException& excp)
	{
		Int32 strPos = excp.getText().find("Attempt to add RSSL_MC_REFRESH with service id of 12345 that was not included in the SourceDirectory. Dropping this \"RSSL_MC_REFRESH\"");
		EXPECT_TRUE(strPos != EmaString::npos) << "Attempt to add RSSL_MC_REFRESH with service id of 12345 that was not included in the SourceDirectory. Dropping this \"RSSL_MC_REFRESH\" != " << excp.getText();
	}
}