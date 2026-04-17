/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Ema.h"
#include "EmaTestClients.h"
#include "IOCtlCode.h"
#include "rtr/rsslTransport.h"
#include "ADHSimulator.h"

#include <iostream>

using namespace refinitiv::ema::access;
using namespace std;

class EmaModifyIOCtlTest : public ::testing::Test
{
public:
	void SetUp() {}

	void TearDown()
	{
		testSleep(1000);
	}
};

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) succeeds for
 * IOCtlCode::MaxNumBuffersEnum when the new value is greater than the current one.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Record the current maxOutputBuffers from the channel information.
 * 3. Call modifyIOCtl(MaxNumBuffersEnum, currentValue + 1000).
 * 4. Verify the new maxOutputBuffers value is set as requested.
 */
TEST_F(EmaModifyIOCtlTest, ModifyIOCtlMaxNumBuffers)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection"); // Establishes a connection on port 15000

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());
		ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ActiveEnum);

		Int32 newValue = channelInfoList[0].getMaxOutputBuffers() + 1000;
		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, newValue, provClient.loginHandle));

		channelInfoList.clear();
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(channelInfoList[0].getMaxOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update MaxNumBuffers to " << newValue;

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		newValue = channelInfo.getMaxOutputBuffers() + 1000;
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, newValue));

		channelInfo.clear();
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getMaxOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update MaxNumBuffers to " << newValue;

		unsigned startPortNum = 16000;

		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		newValue = channelInfo.getMaxOutputBuffers() + 1000;
		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, newValue));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getMaxOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update MaxNumBuffers to " << newValue;

	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) succeeds for
 * IOCtlCode::NumGuaranteedBuffersEnum.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Record the current guaranteedOutputBuffers from the channel information.
 * 3. Call modifyIOCtl(NumGuaranteedBuffersEnum, currentValue + 1000).
 * 4. Verify the new guaranteedOutputBuffers value is set as requested.
 */
TEST_F(EmaModifyIOCtlTest, ModifyIOCtlNumGuaranteedBuffers)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());
		ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ActiveEnum);

		Int32 newValue = channelInfoList[0].getGuaranteedOutputBuffers() + 1000;
		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, newValue, provClient.loginHandle));

		channelInfoList.clear();
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(channelInfoList[0].getGuaranteedOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update NumGuaranteedBuffers to " << newValue;

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		newValue = channelInfo.getGuaranteedOutputBuffers() + 1000;
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, newValue));

		ChannelInformation channelInfo1;
		cons.getChannelInformation(channelInfo1);
		ASSERT_EQ(channelInfo1.getGuaranteedOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update NumGuaranteedBuffers to " << newValue;

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		newValue = channelInfo.getGuaranteedOutputBuffers() + 1000;
		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, newValue));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getGuaranteedOutputBuffers(), newValue)
			<< "Expected modifyIOCtl to update NumGuaranteedBuffers to " << newValue;

	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) succeeds for
 * IOCtlCode::HighWaterMarkEnum with a positive value.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(HighWaterMarkEnum, 7500).
 * 3. Verify no exception is thrown.
 */
TEST_F(EmaModifyIOCtlTest, ModifyIOCtlHighWaterMark)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);
		int newValue = 7500;

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());
		ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ActiveEnum);

		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, newValue, provClient.loginHandle));

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, newValue));

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, newValue));
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) throws
 * OmmInvalidUsageException when called with a negative value for
 * IOCtlCode::HighWaterMarkEnum (invalid argument).
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(HighWaterMarkEnum, -500).
 * 3. Verify that OmmInvalidUsageException is thrown.
 */
TEST_F(EmaModifyIOCtlTest, ModifyIOCtlHighWaterMarkNegativeValueThrows)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);
		
		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());
		ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ActiveEnum);
		EXPECT_THROW(prov.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, -500, provClient.loginHandle), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, -500)";

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);
		EXPECT_THROW(cons.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, -500), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, -500)";

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EXPECT_THROW(niProvider.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, -500), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, -500)";
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) succeeds for
 * IOCtlCode::CompressionThresholdEnum (channel is not configured for compression).
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(CompressionThresholdEnum, 1024).
 * 3. Verify no exception is thrown.
 */
TEST_F(EmaModifyIOCtlTest, ModifyIOCtlCompressionThreshold)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15005_ZLib");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection_15005_ZLib");

	try
	{
		int newValue = 1024;
		{
			OmmProvider prov(provConfig, provClient);
			OmmConsumer cons(consConfig, consClient);

			/* Provider channel */
			EmaVector<ChannelInformation> channelInfoList;
			prov.getConnectedClientChannelInfo(channelInfoList);
			ASSERT_EQ(1, channelInfoList.size());
			ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ActiveEnum);

			EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::CompressionThresholdEnum, newValue, provClient.loginHandle));

			channelInfoList.clear();
			prov.getConnectedClientChannelInfo(channelInfoList);
			ASSERT_EQ(newValue, channelInfoList[0].getCompressionThreshold());

			/* Consumer channel */
			ChannelInformation channelInfo;
			cons.getChannelInformation(channelInfo);
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);
			EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::CompressionThresholdEnum, newValue));
			cons.getChannelInformation(channelInfo);
			ASSERT_EQ(newValue, channelInfo.getCompressionThreshold());
		}
		testSleep(1000); //  Waits for releasing Provider and Consumer

		/* Initialize ADH Simulator */
		unsigned startPortNum = 15005;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);
		/* Configure compression type and level for a new connection from ADH side */
		adhOpts.compressionType = RSSL_COMP_ZLIB;
		adhOpts.compressionLevel = 6;

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig("EmaConfigTest.xml").username("user").providerName("Provider_4_15005_ZLib"));

		ChannelInformation channelInfo;
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::CompressionThresholdEnum, newValue));
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(newValue, channelInfo.getCompressionThreshold());
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) throws
 * OmmInvalidUsageException when called with IOCtlCode::PriorityFlushOrder,
 * because PriorityFlushOrder only accepts a string value, not an integer.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(PriorityFlushOrder, 0).
 * 3. Verify that OmmInvalidUsageException is thrown.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlIntPriorityFlushOrderThrows)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EmaString expectedErrorText("Failed to modify I/O option for code = 7.\n\tError Text invalid code for an integer value.");

		bool foundExpectedException = false;
		try
		{
			prov.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, 0);
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		ASSERT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(PriorityFlushOrder, Int32)";

		try
		{
			foundExpectedException = false;
			cons.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, 0);
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		ASSERT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(PriorityFlushOrder, Int32)";

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig("EmaConfigTest.xml").username("user").providerName("Provider_5"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		try
		{
			foundExpectedException = false;
			niProvider.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, 0);
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		ASSERT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(PriorityFlushOrder, Int32)";
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, const EmaString& value) succeeds
 * for IOCtlCode::PriorityFlushOrder, which is the only code that accepts a string value.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(PriorityFlushOrder, EmaString("HLHMHLHL")).
 * 3. Verify no exception is thrown.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlStringPriorityFlushOrder)
{
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());
		ASSERT_STREQ("HMHLHM", channelInfoList[0].getPriorityFlushStrategy().c_str());

		EmaString priorityFlushStrategy("HLHMHLHL");
		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, priorityFlushStrategy, provClient.loginHandle));

		channelInfoList.clear();
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_STREQ(priorityFlushStrategy.c_str(), channelInfoList[0].getPriorityFlushStrategy().c_str());

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);
		ASSERT_STREQ("HMHLHM", channelInfo.getPriorityFlushStrategy().c_str());

		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, priorityFlushStrategy));

		cons.getChannelInformation(channelInfo);
		ASSERT_STREQ(priorityFlushStrategy.c_str(), channelInfo.getPriorityFlushStrategy().c_str());

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);
		ASSERT_STREQ("HMHLHM", channelInfo.getPriorityFlushStrategy().c_str());

		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, priorityFlushStrategy));

		niProvider.getChannelInformation(channelInfo);
		ASSERT_STREQ(priorityFlushStrategy.c_str(), channelInfo.getPriorityFlushStrategy().c_str());

	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, const EmaString& value) throws
 * OmmInvalidUsageException for every code that is NOT PriorityFlushOrder,
 * since string values are only valid for PriorityFlushOrder.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(NumGuaranteedBuffersEnum, EmaString("100")) - expect OmmInvalidUsageException.
 * 3. Call modifyIOCtl(MaxNumBuffersEnum, EmaString("1000"))       - expect OmmInvalidUsageException.
 * 4. Call modifyIOCtl(HighWaterMarkEnum, EmaString("5000"))       - expect OmmInvalidUsageException.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlStringInvalidCodesThrow)
{
	ProviderTestOptions provTestOptions;

	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig("EmaConfigTest.xml").username("user").providerName("Provider_5"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		EmaString expectedErrorText("Failed to modify I/O option for code = 2.\n\tError Text invalid code for a string value.");

		/* NumGuaranteedBuffersEnum does not accept a string value */
		bool foundExpectedException = false;
		try
		{
			prov.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, EmaString("100"));
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		EXPECT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(NumGuaranteedBuffersEnum, EmaString)";

		try
		{
			foundExpectedException = false;
			cons.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, EmaString("100"));
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		EXPECT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(NumGuaranteedBuffersEnum, EmaString)";

		try
		{
			foundExpectedException = false;
			niProvider.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, EmaString("100"));
		}
		catch (const OmmInvalidUsageException& ommIUE)
		{
			foundExpectedException = true;
			EXPECT_NE(nullptr, strstr(ommIUE.getText(), expectedErrorText));
		}
		EXPECT_TRUE(foundExpectedException) << "Expected OmmInvalidUsageException for modifyIOCtl(NumGuaranteedBuffersEnum, EmaString)";

		/* MaxNumBuffersEnum does not accept a string value */
		EXPECT_THROW(prov.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, EmaString("1000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(MaxNumBuffersEnum, EmaString)";
		EXPECT_THROW(cons.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, EmaString("1000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(MaxNumBuffersEnum, EmaString)";
		EXPECT_THROW(niProvider.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, EmaString("1000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(MaxNumBuffersEnum, EmaString)";

		/* HighWaterMarkEnum does not accept a string value */
		EXPECT_THROW(prov.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, EmaString("5000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, EmaString)";
		EXPECT_THROW(cons.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, EmaString("5000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, EmaString)";
		EXPECT_THROW(niProvider.modifyIOCtl(IOCtlCode::HighWaterMarkEnum, EmaString("5000")), OmmInvalidUsageException)
			<< "Expected OmmInvalidUsageException for modifyIOCtl(HighWaterMarkEnum, EmaString)";
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, Int32 value) applies to all channels
 * when the consumer uses request routing with two active channels.
 * It tests both MaxNumBuffersEnum and NumGuaranteedBuffersEnum.
 * 1. Start two providers and connect a consumer with two active channels.
 * 2. Call modifyIOCtl(MaxNumBuffersEnum, newValue)        - expect no exception.
 * 3. Call modifyIOCtl(NumGuaranteedBuffersEnum, newValue) - expect no exception.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlIntTwoChannels)
{
	ProviderTestOptions provTestOptions1;
	ProviderTestOptions provTestOptions2;

	IProviderTestClientBase provClient1(provTestOptions1);
	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);
	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfoList;
		cons.getSessionInformation(channelInfoList);
		ASSERT_EQ(channelInfoList.size(), 2);

		for(UInt32 i = 0; i < channelInfoList.size(); i++)
			ASSERT_EQ(channelInfoList[i].getChannelState(), ChannelInformation::ActiveEnum);

		Int32 newMaxBuffers = channelInfoList[0].getMaxOutputBuffers() + 2000;
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::MaxNumBuffersEnum, newMaxBuffers));

		Int32 newGuaranteedBuffers = channelInfoList[0].getGuaranteedOutputBuffers() + 1000;
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::NumGuaranteedBuffersEnum, newGuaranteedBuffers));

		cons.getSessionInformation(channelInfoList);
		ASSERT_EQ(channelInfoList.size(), 2);

		for (UInt32 i = 0; i < channelInfoList.size(); i++)
		{
			ASSERT_EQ(channelInfoList[i].getMaxOutputBuffers(), newMaxBuffers + 1000)
				<< "Expected modifyIOCtl to update MaxNumBuffers to " << newMaxBuffers + 1000;
			ASSERT_EQ(channelInfoList[i].getGuaranteedOutputBuffers(), newGuaranteedBuffers)
				<< "Expected modifyIOCtl to update NumGuaranteedBuffers to " << newGuaranteedBuffers;
		}

	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

/* This test verifies that modifyIOCtl(Int32 code, const EmaString& value) with
 * PriorityFlushOrder applies successfully when the consumer has two active channels.
 * 1. Start two providers and connect a consumer with two active channels.
 * 2. Call modifyIOCtl(PriorityFlushOrder, EmaString("HMLHML")) - expect no exception.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlStringPriorityFlushOrderTwoChannels)
{
	ProviderTestOptions provTestOptions1;
	ProviderTestOptions provTestOptions2;

	IProviderTestClientBase provClient1(provTestOptions1);
	OmmIProviderConfig provConfig1("EmaConfigTest.xml");
	provConfig1.providerName("TestProvider_15000");

	IProviderTestClientBase provClient2(provTestOptions2);
	OmmIProviderConfig provConfig2("EmaConfigTest.xml");
	provConfig2.providerName("TestProvider_15001");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("RequestRoutingTestCons_2Channel");

	try
	{
		OmmProvider prov1(provConfig1, provClient1);
		OmmProvider prov2(provConfig2, provClient2);
		OmmConsumer cons(consConfig, consClient);

		EmaVector<ChannelInformation> channelInfoList;
		cons.getSessionInformation(channelInfoList);
		ASSERT_EQ(channelInfoList.size(), 2);

		EmaString priorityFlushOrder("HMLHML");

		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::PriorityFlushOrderEnum, priorityFlushOrder));

		channelInfoList.clear();
		cons.getSessionInformation(channelInfoList);

		for (UInt32 i = 0; i < channelInfoList.size(); i++)
		{
			ASSERT_STREQ(channelInfoList[i].getPriorityFlushStrategy(), priorityFlushOrder.c_str())
				<< "Expected modifyIOCtl to update PriorityFlushOrder to " << priorityFlushOrder;
			ASSERT_STREQ(channelInfoList[i].getPriorityFlushStrategy(), priorityFlushOrder.c_str())
				<< "Expected modifyIOCtl to update NumGuaranteedBuffers to " << priorityFlushOrder;
		}

	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}

#ifdef _WIN32
/* This test verifies that modifyIOCtl(Int32 code, Int32 value) succeeds for
 * IOCtlCode::SystemReadBuffers and IOCtlCode::SystemWriteBuffers (Windows only),
 * updating the channel's TCP receive/send buffer sizes.
 * 1. Start one provider and connect a consumer with one active channel.
 * 2. Call modifyIOCtl(SystemReadBuffers, 30000)  - verify new sysRecvBufSize.
 * 3. Call modifyIOCtl(SystemWriteBuffers, 35000) - verify new sysSendBufSize.
 */
TEST_F(EmaModifyIOCtlTest, modifyIOCtlSystemBuffers)
{
	ProviderTestOptions provTestOptions;

	IProviderTestClientBase provClient(provTestOptions);
	OmmIProviderConfig provConfig("EmaConfigTest.xml");
	provConfig.providerName("TestProvider_15000");

	ConsumerTestOptions consTestOptions;
	ConsumerTestClientBase consClient(consTestOptions);
	OmmConsumerConfig consConfig("EmaConfigTest.xml");
	consConfig.consumerName("SingleConnection");

	try
	{
		OmmProvider prov(provConfig, provClient);
		OmmConsumer cons(consConfig, consClient);

		Int32 newReadBufSize = 30000;
		Int32 newWriteBufSize = 35000;

		/* Provider channel */
		EmaVector<ChannelInformation> channelInfoList;
		prov.getConnectedClientChannelInfo(channelInfoList);
		ASSERT_EQ(1, channelInfoList.size());

		/* SystemReadBuffers */
		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::SystemReadBuffers, newReadBufSize, provClient.loginHandle));

		/* SystemWriteBuffers */
		EXPECT_NO_THROW(prov.modifyIOCtl(IOCtlCode::SystemWriteBuffers, newWriteBufSize, provClient.loginHandle));

		channelInfoList.clear();
		prov.getConnectedClientChannelInfo(channelInfoList);

		ASSERT_EQ(channelInfoList[0].getSysRecvBufSize(), newReadBufSize)
			<< "Expected modifyIOCtl to update SystemReadBuffers to " << newReadBufSize;

		ASSERT_EQ(channelInfoList[0].getSysSendBufSize(), newWriteBufSize)
			<< "Expected modifyIOCtl to update SystemWriteBuffers to " << newWriteBufSize;

		/* Consumer channel */
		ChannelInformation channelInfo;
		cons.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		/* SystemReadBuffers */
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::SystemReadBuffers, newReadBufSize));

		ChannelInformation channelInfo1;
		cons.getChannelInformation(channelInfo1);
		ASSERT_EQ(channelInfo1.getSysRecvBufSize(), newReadBufSize)
			<< "Expected modifyIOCtl to update SystemReadBuffers to " << newReadBufSize;

		/* SystemWriteBuffers */
		EXPECT_NO_THROW(cons.modifyIOCtl(IOCtlCode::SystemWriteBuffers, newWriteBufSize));

		ChannelInformation channelInfo2;
		cons.getChannelInformation(channelInfo2);
		ASSERT_EQ(channelInfo2.getSysSendBufSize(), newWriteBufSize)
			<< "Expected modifyIOCtl to update SystemWriteBuffers to " << newWriteBufSize;

		/* Initialize ADH Simulator */
		unsigned startPortNum = 16000;
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();

		/* Wait for ADH simulator to start */
		UInt16 k = 0;
		while (!adh.isRunning() && k++ < 10) testSleep(250);

		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;

		/* Non-Interactive provider channel */
		OmmProvider niProvider(OmmNiProviderConfig().host("localhost:16000").username("user"));

		channelInfo.clear();
		niProvider.getChannelInformation(channelInfo);
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ActiveEnum);

		/* SystemReadBuffers */
		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::SystemReadBuffers, newReadBufSize));

		channelInfo1.clear();
		niProvider.getChannelInformation(channelInfo1);
		ASSERT_EQ(channelInfo1.getSysRecvBufSize(), newReadBufSize)
			<< "Expected modifyIOCtl to update SystemReadBuffers to " << newReadBufSize;

		/* SystemWriteBuffers */
		EXPECT_NO_THROW(niProvider.modifyIOCtl(IOCtlCode::SystemWriteBuffers, newWriteBufSize));

		channelInfo2.clear();
		niProvider.getChannelInformation(channelInfo2);
		ASSERT_EQ(channelInfo2.getSysSendBufSize(), newWriteBufSize)
			<< "Expected modifyIOCtl to update SystemWriteBuffers to " << newWriteBufSize;
	}
	catch (const OmmException& ex)
	{
		ASSERT_TRUE(false) << "Unexpected exception: " << ex.getText();
	}
}
#endif
