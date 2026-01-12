/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ADHSimulator.h"
#include "Ema.h"
#include "EmaTestClients.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderActiveConfig.h"

#include <iostream>

using namespace refinitiv::ema::access;
using namespace std;


void NiProviderProgrammaticTestConfig::createProgrammaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;
	ElementList niproviderElementList;
	ElementList channelElementList;

	EmaString strChannel;
	unsigned i;
	char portNumStr[32];

	// Need at least one channel
	if (numChannels < 1)
		numChannels = 1;

	// Start <NiProviderGroup>
	elementList.addAscii("DefaultNiProvider", "NiProviderProgrammaticTest_1");

	/* Add one of the lines to the configuration: */
	/* 1) configure only one channel */
	/* <Channel value="ChannelProgrammaticNi_1"/> */
	/* 2) configure multiple channels */
	/* <ChannelSet value="ChannelProgrammaticNi_1,ChannelProgrammaticNi_2,...,ChannelProgrammaticNi_n"/> */
	if (numChannels == 1)
	{
		strChannel = "ChannelProgrammaticNi_1";
		niproviderElementList
			.addAscii("Channel", strChannel);
	}
	else
	{
		for (i = 0; i < numChannels; i++)
		{
			if (i > 0)
				strChannel.append(",");

			strChannel.append("ChannelProgrammaticNi_").append(i + 1);
		}

		niproviderElementList
			.addAscii("ChannelSet", strChannel);
	}

	niproviderElementList
		.addAscii("Directory", "DirectoryProgrammaticNi_1")
		.addAscii("Logger", "LoggerProgrammaticNi_1")
		.addUInt("LoginRequestTimeOut", loginRequestTimeOut)
		.addUInt("RequestTimeout", 5000)
		//.addInt("ReconnectAttemptLimit", 10)
		//.addUInt("CatchUnhandledException", 0)
		.addInt("ReconnectMinDelay", 2000)
		.addInt("ReconnectMaxDelay", 6000);

	niproviderElementList.complete();

	// Start <NiProviderList>
	innerMap.addKeyAscii("NiProviderProgrammaticTest_1",
		MapEntry::AddEnum,
		niproviderElementList);

	innerMap.complete();

	elementList.addMap("NiProviderList", innerMap);
	elementList.complete();

	configMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);
	// Finish <NiProviderGroup>

	// <ChannelGroup>
	niproviderElementList.clear();
	innerMap.clear();
	elementList.clear();

	for (i = 0; i < numChannels; i++)
	{
		strChannel.clear().append("ChannelProgrammaticNi_").append(i + 1);
		snprintf(portNumStr, sizeof(portNumStr), "%u", (startPortNum + i));

		channelElementList.clear();
		channelElementList
			.addEnum("ChannelType", 0)
			.addUInt("GuaranteedOutputBuffers", 5000)
			.addUInt("ConnectionPingTimeout", 30000)
			.addUInt("InitializationTimeout", channelInitializationTimeout)
			.addAscii("Host", "localhost")
			.addAscii("Port", portNumStr)
			.addUInt("TcpNodelay", 1);
		
		if (enableCompression)
		{
			channelElementList
				.addEnum("CompressionType", 1) // 1 - ZLib
				.addUInt("CompressionThreshold", 1024);
		}

		channelElementList.complete();

		innerMap.addKeyAscii(strChannel, MapEntry::AddEnum, channelElementList);
	}

	innerMap.complete();

	elementList.addMap("ChannelList", innerMap);
	elementList.complete();

	configMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
	// End of <ChannelGroup>
	innerMap.clear();
	elementList.clear();

	innerMap.addKeyAscii("LoggerProgrammaticNi_1",
		MapEntry::AddEnum,
		ElementList()
		//.addEnum("LoggerType", 1)
		.addAscii("FileName", "logFile")
		//.addEnum("LoggerSeverity", 1)
		.addEnum("LoggerSeverity", static_cast<UInt16>(loggerSeverity))
		.complete())
	.complete();

	elementList.addMap("LoggerList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	Map serviceMap;
	serviceMap.addKeyAscii("NI_PUB", MapEntry::AddEnum,
		ElementList()
		.addElementList("InfoFilter",
			ElementList().addUInt("ServiceId", 2)
			.addAscii("Vendor", "company name")
			.addUInt("IsSource", 0)
			.addUInt("AcceptingConsumerStatus", 0)
			.addUInt("SupportsQoSRange", 0)
			.addUInt("SupportsOutOfBandSnapshots", 0)
			.addAscii("ItemList", "#.itemlist")
			.addArray("Capabilities",
				OmmArray().addAscii("MMT_MARKET_PRICE")
				.addAscii("MMT_MARKET_BY_PRICE")
				.addAscii("200")
				.complete())
			.addArray("DictionariesUsed",
				OmmArray().addAscii("DictionaryProgrammaticNi_1")
				.complete())
			.addSeries("QoS",
				Series()
				.add(
					ElementList().addAscii("Timeliness", "Timeliness::RealTime")
					.addAscii("Rate", "Rate::TickByTick")
					.complete())
				.add(
					ElementList().addUInt("Timeliness", 100)
					.addUInt("Rate", 100)
					.complete())
				.complete())
			.complete())

		.addElementList("StateFilter",
			ElementList().addUInt("ServiceState", 1)
			.addUInt("AcceptingRequests", 1)
			.complete())
		.complete())
		.complete();

	innerMap.addKeyAscii("DirectoryProgrammaticNi_1", MapEntry::AddEnum, serviceMap).complete();

	elementList.clear();
	elementList.addAscii("DefaultDirectory", "DirectoryProgrammaticNi_1");
	elementList.addMap("DirectoryList", innerMap).complete();

	configMap.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList);

	innerMap.clear();
	elementList.clear();

	innerMap.addKeyAscii("DictionaryProgrammaticNi_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("DictionaryType", 0)
		.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionaryTest")
		.addAscii("EnumTypeDefFileName", "./enumtypeTest.def").complete())
		.complete();

	elementList.addMap("DictionaryList", innerMap);
	elementList.complete();

	configMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

	innerMap.clear();
	elementList.clear();

	configMap.complete();
}

/* OmmProvider constructors overloading variant */
enum class OmmProviderConstructorType
{
	config,
	config_providerClient,
	config_errorClient,
	config_providerClient_errorClient,
};

std::ostream& operator<<(std::ostream& ostream, OmmProviderConstructorType constructorType)
{
	switch (constructorType) {
	case OmmProviderConstructorType::config:
		ostream << "config"; break;
	case OmmProviderConstructorType::config_providerClient:
		ostream << "config_providerClient"; break;
	case OmmProviderConstructorType::config_errorClient:
		ostream << "config_errorClient"; break;
	case OmmProviderConstructorType::config_providerClient_errorClient:
		ostream << "config_providerClient_errorClient"; break;
	default:
		ostream << "error: unknown constructor type!"; break;
	}
	return ostream;
}

class ProviderErrorClientTest : public refinitiv::ema::access::OmmProviderErrorClient
{
public:

	ProviderErrorClientTest() :
		countOnInvalidHandle(0),
		countOnInaccessibleLogFile(0),
		countOnSystemError(0),
		countOnMemoryExhaustion(0),
		countOnInvalidUsage(0),
		countOnJsonConverter(0),
		countOnDispatchError(0)
	{
	}

	~ProviderErrorClientTest() = default;
	
	void clear()
	{
		countOnInvalidHandle = 0;
		countOnInaccessibleLogFile = 0;
		countOnSystemError = 0;
		countOnMemoryExhaustion = 0;
		countOnInvalidUsage = 0;
		countOnJsonConverter = 0;
		countOnDispatchError = 0;
	}

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


	/* Overide OmmProviderErrorClient methods */

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

	void onMemoryExhaustion( const EmaString& text ) override
	{
		++countOnMemoryExhaustion;
		//cout << endl << "onMemoryExhaustion callback function" << endl;
		//cout << "Error text: " << text << endl;
	}

	void onInvalidUsage( const EmaString& text, Int32 errorCode ) override
	{
		++countOnInvalidUsage;
		//cout << "onInvalidUsage callback function" << endl;
		//cout << "Error text: " << text << endl;
		//cout << "Error code: " << errorCode << endl;
	}

	void onSystemError( Int64 code, void* ptr, const EmaString& text ) override
	{
		++countOnSystemError;
		//cout << endl << "onSystemError callback function" << endl;
		//cout << "System Error code: " << code << endl;
		//cout << "System Error Address: " << address << endl;
		//cout << "Error text: " << text << endl;
	}

	void onJsonConverter( const EmaString& text, Int32 errorCode, const ProviderSessionInfo& sessionInfo ) override
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

class OmmNiProviderCreateTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		pNiProviderConfig = nullptr;
		pNiProviderTestClient = nullptr;
		pOmmNiProviderTest = nullptr;
	}

	void TearDown() override
	{
		stop();

		testSleep(1000);  // Allow time for cleanup - close network connections, etc
	}

public:

	OmmNiProviderConfig* pNiProviderConfig;
	Map configMap;

	ProviderTestOptions niProvTestOptions;
	NiProviderTestClientBase* pNiProviderTestClient;

	OAuthClientTest oAuthClient;
	ProviderErrorClientTest errorClient;

	OmmProvider* pOmmNiProviderTest;

	/* Create OmmNiProvider instance using different constructor options */
	/**/
	/* constructorType OmmNiProvider constructors overloading variant */
	OmmProvider* createOmmNiProvider(OmmProviderConstructorType constructorType, NiProviderProgrammaticTestConfig& niProvProgConfig)
	{
		/* Create OmmNiProvider's programmatic configuration */
		niProvProgConfig.createProgrammaticConfig( configMap );

		//pNiProviderConfig = new OmmNiProviderConfig("./EmaConfigTest.xml");
		pNiProviderConfig = new OmmNiProviderConfig();
		pNiProviderConfig->config(configMap);

		/* NiProvider client */
		pNiProviderTestClient = new NiProviderTestClientBase(niProvTestOptions);

		/* Create OmmNiProvider instance using different constructor */
		switch ( constructorType ) {
		case OmmProviderConstructorType::config:
		{
			// OmmProvider( const OmmProviderConfig& config );
			pOmmNiProviderTest = new OmmProvider( *pNiProviderConfig );
			break;
		}

		case OmmProviderConstructorType::config_providerClient:
		{
			// OmmProvider( const OmmProviderConfig& config, OmmProviderClient& client, void* closure = 0 );
			pOmmNiProviderTest = new OmmProvider( *pNiProviderConfig, *pNiProviderTestClient,
				(void*)static_cast<int>(OmmProviderConstructorType::config_providerClient) );
			break;
		}

		case OmmProviderConstructorType::config_errorClient:
		{
			// OmmProvider( const OmmProviderConfig& config, OmmProviderErrorClient& errorclient );
			pOmmNiProviderTest = new OmmProvider( *pNiProviderConfig, errorClient );
			break;
		}

		case OmmProviderConstructorType::config_providerClient_errorClient:
		{
			// OmmProvider( const OmmProviderConfig& config, OmmProviderClient& client, OmmProviderErrorClient& errorclient, void* closure = 0 );
			pOmmNiProviderTest = new OmmProvider( *pNiProviderConfig, *pNiProviderTestClient, errorClient,
				(void*)static_cast<int>(OmmProviderConstructorType::config_providerClient_errorClient) );
			break;
		}

		default:  // error case: unknown constructor option
			break;
		}

		return pOmmNiProviderTest;
	}

	/* Stop the OmmNiProvider instance and clean up all the resources */
	void stop()
	{
		if (pOmmNiProviderTest) {
			delete pOmmNiProviderTest;
			pOmmNiProviderTest = nullptr;
		}
		if (pNiProviderTestClient) {
			delete pNiProviderTestClient;
			pNiProviderTestClient = nullptr;
		}
		if (pNiProviderConfig) {
			delete pNiProviderConfig;
			pNiProviderConfig = nullptr;
		}
	}

	/* Is the error client configured for the non interactive OmmProvider? */
	/* When an error client is configured, the niprovider will notify it of any errors. */
	/* Otherwise, the niprovider will throw an exception. */
	/**/
	/* constructorType OmmProvider constructors overloading variant */
	bool hasErrorClient( OmmProviderConstructorType constructorType ) const
	{
		bool bRes = false;

		switch ( constructorType ) {
		case OmmProviderConstructorType::config_errorClient:
		case OmmProviderConstructorType::config_providerClient_errorClient:
			bRes = true;
			break;
		};
		return bRes;
	}

	/* Is the test client assigned for the non interactive OmmProvider? */
	bool hasTestClient( OmmProviderConstructorType constructorType ) const
	{
		bool bRes = false;

		switch ( constructorType ) {
		case OmmProviderConstructorType::config_providerClient:
		case OmmProviderConstructorType::config_providerClient_errorClient:
			bRes = true;
			break;
		};
		return bRes;
	}

};  // class OmmNiProviderCreateTest

class OmmNiProviderCreateTestParams {
public:
	OmmProviderConstructorType		constructorType;		// What constructor variant to use

	OmmNiProviderCreateTestParams(OmmProviderConstructorType providerConstructorType) :
		constructorType(providerConstructorType)
	{
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend std::ostream& operator<<(std::ostream& out, const OmmNiProviderCreateTestParams& params)
	{
		out << "["
			<< "createType:" << params.constructorType
			<< "]";
		return out;
	}
};

class OmmNiProviderCreateTestFixture
	: public OmmNiProviderCreateTest, public ::testing::WithParamInterface<OmmNiProviderCreateTestParams>
{

};

/*  Negative test case: expect an error. */
/*  Create OmmProvider as a non-interactive provider to connect to a non-existent provider, */
/*  it must handle the exception or error properly. */
/*  This test does the following:
 *  1. Creates an non-interactive provider
 *  2. Checks to see that the non-interactive provider does not initialized
 *  3. Checks to see that the non-interactive provider did not get anyhting
 */
TEST_P(OmmNiProviderCreateTestFixture, CreateNiProviderWithNoProviderShouldThrowException)
{
	const OmmNiProviderCreateTestParams& testParams = GetParam();

	try
	{
		NiProviderProgrammaticTestConfig niProvProgConfig;

		// Set loginRequestTimeOut and channelInitializationTimeout to low values to speed up the test
		niProvProgConfig.loginRequestTimeOut = 1000; // 1 second - timeout for login request
		niProvProgConfig.channelInitializationTimeout = 1; // 1 second - timeout for channel initialization

		/* While non-interactive OmmProvider creating we expect an error/exception because connection fails. */
		OmmProvider* pNiProvider = createOmmNiProvider( testParams.constructorType, niProvProgConfig );

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

	/* Check that non-interactive provider TestClient did not receive any messages */
	ASSERT_EQ(0, pNiProviderTestClient->getMessageQueueSize()) << "Expected no messages in pNiProviderTestClient message queue: " << pNiProviderTestClient->getMessageQueueSize();
}

#if 0
/*  Non-interactive Provider connects to Provider correctly. */
/*  This test does the following:
 *  1. Starts up an non-interactive provider with a login client with one provider
 *  2. Checks to see that the ni provider does intitialize
 *  3. Checks to see that the ni provider has only one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_P(OmmNiProviderCreateTestFixture, CreateNiProviderWithOneIProvider)
{
	// Test disabled pending investigation RTSDK-10246 crash.
	FAIL() << "Test disabled pending investigation issue.";

	const OmmNiProviderCreateTestParams& testParams = GetParam();
	unsigned startPortNum = 14220;

	try
	{
		ProviderTestOptions provTestOptions;
		IProviderTestClientBase provClient(provTestOptions);

		startPortNum += static_cast<int>( testParams.constructorType );

		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.startPortNum = startPortNum;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);

		OmmIProviderConfig provConfig("EmaConfigTest.xml");
		provConfig.config(configIProvMap);
		provConfig.providerName("ProviderProgrammaticTest_1");

		/* Create IProvider instance */
		OmmProvider prov( provConfig, provClient );
		//cout << "OmmProvider created." << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider = createOmmNiProvider( testParams.constructorType, niProvProgConfig );

		/* Check that NiProvider is created */
		ASSERT_NE( pNiProvider, nullptr ) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* When the test client assigned for the non-interactive provider we can check the message queue */
		if ( hasTestClient(testParams.constructorType) )
		{
			ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
			//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

			/* Checks to see that the ni provider got a OPEN/OK login refresh message */
			while (Msg* msg = pNiProviderTestClient->popMsg())
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
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}	
}

/*  NiProvider connects to Providers using a round robin algorithm. */
/*  The test configures the non-interactive provider to connect to a set of interactive providers. */
/*  This test does the following:
 *  1. Starts up the set of providers
 *  2. Starts up an ni-provider with a lot of providers, configured via ChannelSet
 *  3. Checks to see that the ni-provider does intitialize
 *  4. Checks to see that the ni-provider has only one active channel and that it got a OPEN/OK login refresh message
 *  5. Shut down the 1-st provider fully
 *  6. Check that the ni-provider switches to the next provider and that it got a OPEN/OK login refresh message
 *  7. Restore/Up the 1-st provider (previous)
 *  8. Shut down the 2-nd provider fully
 *  Repeats steps 6-8 for all the providers, checking that the ni-provider switches to the next provider (round robin algorithm)
 */
TEST_P(OmmNiProviderCreateTestFixture, NiProviderRoundRobinProvidersBreakConnection)
{
	// Test disabled pending investigation RTSDK-10246 crash.
	FAIL() << "Test disabled pending investigation issue.";

	const OmmNiProviderCreateTestParams& testParams = GetParam();
	unsigned i, j, k;

	const unsigned NUMConnections = 3;  // The number of providers and channels available to the ni-provider
	unsigned startPortNum = 14230;

	Map configIProvMap;
	IProviderProgrammaticTestConfig iprovProgConfig;

	vector< shared_ptr<ProviderTestComponents> > providers;

	Msg* msg;
	RefreshMsg* refreshMsg;

	try
	{
		startPortNum += static_cast<int>( testParams.constructorType ) * NUMConnections;

		/* Create a programmatic configuration. It is used for all the Providers */
		iprovProgConfig.numProviders = NUMConnections;
		iprovProgConfig.startPortNum = startPortNum;

		iprovProgConfig.createProgrammaticConfig(configIProvMap);
		//std::cout << "IProvider programmatic configuration created. " << startPortNum << std::endl;

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

		/* Create NiProvider test instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.numChannels = NUMConnections;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider = createOmmNiProvider(testParams.constructorType, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;

		ChannelInformation channelInfo;
		EmaVector<ChannelInformation> channelInfoList;

		/* Loop to break connection with each provider in round robin manner */
		for (i = 0; i < (NUMConnections + 1); i++)
		{
			j = (i % NUMConnections);  // current provider index: 0, 1, 2, 0
			//std::cout << "OmmProvider_" << (j + 1) << ": checking connection with NiProvider." << std::endl;

			/* Provider side */
			/* Check that ni-provider connected to the j-th provider */
			OmmProvider* pProvider = providers[j]->pProvider;
			ASSERT_NE(pProvider, nullptr) << "Expected OmmProvider to be initialized. j: " << j;

			pProvider->getConnectedClientChannelInfo(channelInfoList);
			ASSERT_EQ(channelInfoList.size(), 1) << "Expected that the provider is connected to the ni-provider.";
			ASSERT_EQ(channelInfoList[0].getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* NiProvider side */
			/* Check the active channel */
			pNiProvider->getChannelInformation(channelInfo);
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << std::endl;
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* When the test client assigned for the ni-provider we can check the message queue */
			if ( hasTestClient(testParams.constructorType) )
			{
				ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
				//std::cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << std::endl;

				/* Checks to see that the ni-provider got a OPEN/OK login refresh message */
				while (msg = pNiProviderTestClient->popMsg())
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
			//std::cout << "OmmProvider_" << (j + 1) << ": has been deleted, waiting for ni-provider switch." << std::endl;

			/* Check that ni-provider lost connection */
			pNiProvider->getChannelInformation(channelInfo);
			//std::cout << "Channel Info. channelState: " << channelInfo.getChannelState() << std::endl;
			ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

			/* Wait for the ni-provider to switch to the next provider */
			k = 0;
			do {
				testSleep(500);
				pNiProvider->getChannelInformation(channelInfo);
				k++;
			} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that ni-provider connects to the next provider. i: " << i << "; k: " << k;
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << "; k: " << k << std::endl;

			/* Up the deleted provider: re-create the j-th Provider with components */
			std::string providerProgTestName = "ProviderProgrammaticTest_" + std::to_string(j + 1);
			ProviderTestComponents* providerTestComponent = new ProviderTestComponents();
			providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
			providers[j] = make_shared<ProviderTestComponents>(providerTestComponent);
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

/*  Non interactive Provider connects to Providers and fails when the first provider does not send a Login Refresh. */
/*  The test configures the ni-provider to connect to a set of providers. */
/*  The test configures providers to send a LoginRefresh or not. */
/*  This test does the following:
 *  1. Starts up the set of providers. The first provider is configured not to send a login refresh
 *  2. Starts up an ni-provider with a lot of providers, configured via ChannelSet
 *  3. Checks for an initialization error if the error client is configured, otherwise an exception will be thrown
 */
TEST_P(OmmNiProviderCreateTestFixture, NiProviderRoundRobinProviderNotSendLoginRefresh)
{
	// Test disabled pending investigation RTSDK-10246 crash.
	FAIL() << "Test disabled pending investigation issue.";

	const OmmNiProviderCreateTestParams& testParams = GetParam();

	const unsigned NUMConnections = 2;  // The number of providers and channels available to the ni-provider
	unsigned startPortNum = 14250;

	try
	{
		std::string providerProgTestName;
		ProviderTestComponents* providerTestComponent;

		vector< shared_ptr<ProviderTestComponents> > providers;

		NiProviderProgrammaticTestConfig niProvProgConfig;
		OmmProvider* pNiProvider;

		startPortNum += static_cast<int>( testParams.constructorType ) * NUMConnections;

		/* Create a programmatic configuration. It is used for all the Providers */
		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.numProviders = NUMConnections;
		iprovProgConfig.startPortNum = startPortNum;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);
		//std::cout << "IProvider programmatic configuration created. " << startPortNum << std::endl;

		/* Create the set of test Providers */
		providerProgTestName = "ProviderProgrammaticTest_1";
		providerTestComponent = new ProviderTestComponents();
		providerTestComponent->provTestOptions.sendLoginRefresh = false;
		providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
		providers.emplace_back(providerTestComponent);

		providerProgTestName = "ProviderProgrammaticTest_2";
		providerTestComponent = new ProviderTestComponents();
		providerTestComponent->provTestOptions.sendLoginRefresh = true;
		providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
		providers.emplace_back(providerTestComponent);

		//cout << "Set of providers with their components created." << endl;

		/* Create NiProvider test instance */
		niProvProgConfig.loginRequestTimeOut = 3000; // 3 seconds - timeout for login request
		niProvProgConfig.channelInitializationTimeout = 2;
		niProvProgConfig.numChannels = NUMConnections;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		pNiProvider = createOmmNiProvider(testParams.constructorType, niProvProgConfig);

		/* Check for an initialization error if the error client is configured (otherwise an exception will be thrown) */
		if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error. countOnInvalidUsage: " << errorClient.getCountOnInvalidUsage();
			ASSERT_EQ(1, errorClient.getCountOnInvalidUsage());
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}
	}
	catch (const OmmInvalidUsageException& invalidUsageException)
	{
		ASSERT_TRUE(true) << "Expected OmmInvalidUsageException: " << invalidUsageException.toString();
		//cout << invalidUsageException.toString() << endl;
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/*  Non interactive Provider connects to Providers and fails when the first provider sends a Login Reject. */
/*  The test configures the ni-provider to connect to a set of providers. */
/*  The test configures providers to send a Login Reject or not. */
/*  This test does the following:
 *  1. Starts up the set of providers. The first provider is configured to send a login reject
 *  2. Starts up an ni-provider with a lot of providers, configured via ChannelSet
 *  3. Checks for an initialization error if the error client is configured, otherwise an exception will be thrown
 */
TEST_P(OmmNiProviderCreateTestFixture, NiProviderRoundRobinProviderSendLoginReject)
{
	// Test disabled pending investigation RTSDK-10246 crash.
	FAIL() << "Test disabled pending investigation issue.";

	const OmmNiProviderCreateTestParams& testParams = GetParam();

	const unsigned NUMConnections = 2;  // The number of providers and channels available to the ni-provider
	unsigned startPortNum = 14260;

	try
	{
		std::string providerProgTestName;
		ProviderTestComponents* providerTestComponent;

		vector< shared_ptr<ProviderTestComponents> > providers;

		NiProviderProgrammaticTestConfig niProvProgConfig;
		OmmProvider* pNiProvider;

		startPortNum += static_cast<int>( testParams.constructorType ) * NUMConnections;

		/* Create a programmatic configuration. It is used for all the Providers */
		Map configIProvMap;
		IProviderProgrammaticTestConfig iprovProgConfig;
		iprovProgConfig.numProviders = NUMConnections;
		iprovProgConfig.startPortNum = startPortNum;
		iprovProgConfig.createProgrammaticConfig(configIProvMap);
		//std::cout << "IProvider programmatic configuration created. " << startPortNum << std::endl;

		/* Create the set of test Providers */
		providerProgTestName = "ProviderProgrammaticTest_1";
		providerTestComponent = new ProviderTestComponents();
		providerTestComponent->provTestOptions.acceptLoginRequest = false;
		providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
		providers.emplace_back(providerTestComponent);

		providerProgTestName = "ProviderProgrammaticTest_2";
		providerTestComponent = new ProviderTestComponents();
		providerTestComponent->provTestOptions.acceptLoginRequest = true;
		providerTestComponent->createProvider(configIProvMap, providerProgTestName.c_str());
		providers.emplace_back(providerTestComponent);

		//cout << "Set of providers with their components created." << endl;

		/* Create NiProvider test instance */
		niProvProgConfig.loginRequestTimeOut = 3000; // 3 seconds - timeout for login request
		niProvProgConfig.channelInitializationTimeout = 2;
		niProvProgConfig.numChannels = NUMConnections;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		pNiProvider = createOmmNiProvider(testParams.constructorType, niProvProgConfig);

		/* Check for an initialization error if the error client is configured (otherwise an exception will be thrown) */
		if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error. countOnInvalidUsage: " << errorClient.getCountOnInvalidUsage();
			ASSERT_EQ(1, errorClient.getCountOnInvalidUsage());
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}
	}
	catch (const OmmInvalidUsageException& invalidUsageException)
	{
		ASSERT_TRUE(true) << "Expected OmmInvalidUsageException: " << invalidUsageException.toString();
		//cout << invalidUsageException.toString() << endl;
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}
#endif

/* Non-interactive Provider connects to ADH correctly. */
/* This test does the following:
 * 1. Starts an ADH simulator that accepts login requests and sends a solicited login refresh
 * 2. Starts up a non-interactive provider configured to connect to the ADH simulator
 * 3. Checks to see that the ni provider does initialize
 * 4. Checks to see that the ni provider has only one active channel and that it got a OPEN/OK login refresh message.
 */
TEST_P(OmmNiProviderCreateTestFixture, CreateNiProviderWithADH)
{
	const OmmNiProviderCreateTestParams& testParams = GetParam();
	unsigned startPortNum = 14310;

	try
	{
		startPortNum += static_cast<int>( testParams.constructorType );

		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);
		
		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();
		//cout << "ADH Simulator created. " << portNo << endl;
		testSleep(500);

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!adh.isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider = createOmmNiProvider( testParams.constructorType, niProvProgConfig );

		/* Check that NiProvider is created */
		ASSERT_NE( pNiProvider, nullptr ) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;

		/* Check that NiProvider name is NiProviderProgrammaticTest_1. It is set by programmatic configuration */
		EmaString niProviderName = pNiProvider->getProviderName();
		//cout << "NiProvider name: " << niProviderName << endl;
		Int32 findPos = niProviderName.find("NiProviderProgrammaticTest_1");
		ASSERT_EQ(findPos, 0) << "Expected NiProvider name to be NiProviderProgrammaticTest_1";

		/* Check NiProvider role */
		ASSERT_EQ(pNiProvider->getProviderRole(), OmmProviderConfig::ProviderRole::NonInteractiveEnum);

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* When the test client assigned for the non-interactive provider we can check the message queue */
		if ( hasTestClient(testParams.constructorType) )
		{
			ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
			//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

			/* Checks to see that the ni provider got a OPEN/OK login refresh message */
			while (Msg* msg = pNiProviderTestClient->popMsg())
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
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		stop();
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

#if 0
/* NiProvider connects to ADHs using a round robin algorithm.
 * The test starts three separate ADH simulator instances and configures the NiProvider
 * with three channels that target these ADHs.
 * 
 * This test does the following:
 * 1. Start 3 ADH simulator instances
 * 2. Configure the NiProvider with 3 channels in ChannelSet, each targeting one of the 3 ADH simulators
 * 3. In the loop, verify that the NiProvider connects to each ADH in round robin manner
 * 3a Checks to see that the ni-provider does intitialize
 * 3b Checks to see that the ni-provider has only one active channel and that it got a OPEN/OK login refresh message
 * 4. Stop the active ADH
 * 5. Checks to see that the ni-provider switches to the next ADH
 * 6. Restart the stopped ADH
 * Repeats steps 3-6 for all the ADHs, checking that the ni-provider switches to the next ADH (round robin algorithm)
 */
TEST_P(OmmNiProviderCreateTestFixture, NiProviderRoundRobinADHBreakConnection)
{
	const OmmNiProviderCreateTestParams& testParams = GetParam();
	unsigned i, j, k;

	const unsigned NUMConnections = 3;  // The number of providers and channels available to the ni-provider
	unsigned startPortNum = 14320;

	RsslCreateReactorOptions reactorOpts[NUMConnections];

	ADHSimulatorOptions adhOpts[NUMConnections];
	ADHSimulator* pADH;

	ADHSimulator* arrADH[NUMConnections] = { nullptr, nullptr, nullptr };

	ChannelInformation channelInfo;
	Msg* msg;
	RefreshMsg* refreshMsg;
	
	UInt16 port = -1;
	bool switchOccurred = false;

	try
	{
		startPortNum += static_cast<int>( testParams.constructorType ) * NUMConnections;

		/* Initialize set of ADH Simulators */
		for (i = 0; i < NUMConnections; i++)
		{
			rsslClearCreateReactorOptions(&reactorOpts[i]);
			
			adhOpts[i].clear();
			adhOpts[i].pReactorOptions = &reactorOpts[i];
			snprintf(adhOpts[i].portNo, sizeof(adhOpts[i].portNo), "%u", (startPortNum + i));

			pADH = new ADHSimulator(adhOpts[i]);
			pADH->start();

			arrADH[i] = pADH;
			//std::cout << "ADHs[" << i << "]: created ADHSimulator and added it to vector adhs. " << portNo << std::endl;
			testSleep(250);
		}

		/* Create NiProvider test instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.numChannels = NUMConnections;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider = createOmmNiProvider(testParams.constructorType, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;
		testSleep(250);

		/* Check that NiProvider name is NiProviderProgrammaticTest_1. It is set by programmatic configuration */
		EmaString niProviderName = pNiProvider->getProviderName();
		//cout << "NiProvider name: " << niProviderName << endl;
		Int32 findPos = niProviderName.find("NiProviderProgrammaticTest_1");
		ASSERT_EQ(findPos, 0) << "Expected NiProvider name to be NiProviderProgrammaticTest_1.";

		/* Check NiProvider role */
		ASSERT_EQ(pNiProvider->getProviderRole(), OmmProviderConfig::ProviderRole::NonInteractiveEnum);

		/* Loop to break connection with each ADH in round robin manner */
		for (i = 0; i < (NUMConnections + 1); i++)
		{
			j = (i % NUMConnections);  // current ADH index: 0, 1, 2, 0
			pADH = arrADH[j];
			//std::cout << "ADH[" << j << "]: checking connection with NiProvider." << std::endl;

			/* NiProvider side */
			/* Check the active channel */
			pNiProvider->getChannelInformation(channelInfo);
			port = channelInfo.port();
			
			//std::cout << "Channel Info.port: " << channelInfo.port() << " " << channelInfo.getName() << std::endl;
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that NiProvider is in active state";
			ASSERT_EQ(channelInfo.port(), (startPortNum + j)) << "Expected that NiProvider is connected to ADH[" << j << "]";
			//std::cout << "ADH[" << j << "] port: " << pADH->options.getPortNo() << " : " << adhOpts[j].portNo << std::endl;

			/* When the test client assigned for the ni-provider we can check the message queue */
			if ( hasTestClient(testParams.constructorType) )
			{
				ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
				//std::cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << std::endl;

				/* Checks to see that the ni-provider got a OPEN/OK login refresh message */
				while (msg = pNiProviderTestClient->popMsg())
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

			/* Stop the connection with the j-th ADH */
			//std::cout << "ADH[" << j << "]: is deleting." << std::endl;
			if ( pADH != nullptr )
			{
				pADH->stop();
				testSleep(1000);
			}
			//std::cout << "ADH[" << j << "]: has been stopped, waiting for ni-provider switch." << std::endl;

			/* Check that ni-provider lost connection */
			switchOccurred = false;
			k = 0;
			do {
				testSleep(500);
				pNiProvider->getChannelInformation(channelInfo);
				k++;

				// Break the loop if the ni-provider switched to another ADH (port number is different)
				if (channelInfo.port() > 0 && channelInfo.port() != port)
				{
					switchOccurred = true;
					break;
				}
			} while (channelInfo.getChannelState() == ChannelInformation::ChannelState::ActiveEnum && k < 20);

			//std::cout << "Break Connection. Channel Info. channelState: " << channelInfo.getChannelState() << "; k: " << k << "; switchOccurred: " << switchOccurred << std::endl;
			if ( !switchOccurred )
			{
				ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that ni-provider is not in active state after ADH shutdown. k: " << k;
			}
			else
			{
				ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that ni-provider connects to the next ADH (1). k: " << k;
			}

			/* Wait for the ni-provider to switch to the next ADH */
			if ( !switchOccurred )
			{
				k = 0;
				do {
					testSleep(500);
					pNiProvider->getChannelInformation(channelInfo);
					k++;
				} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
			}
			ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected that ni-provider connects to the next ADH (2). i: " << i << "; k: " << k;
			//std::cout << "Channel New Info.port: " << channelInfo.port() << " " << channelInfo.getName() << "; k: " << k << std::endl;

			/* Restart ADH */
			if (pADH != nullptr)
			{
				//std::cout << "ADH[" << j << "]: is restarting." << std::endl;
				pADH->start();
				testSleep(500);
			}
		}

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		stop();

		testSleep(250);
		/* Stop ADH Simulators */
		//std::cout << "--- Finish.Stop ADHs" << std::endl;
		for (i = 0; i < NUMConnections; i++)
		{
			pADH = arrADH[i];
			if (pADH != nullptr)
			{
				//pADH->stop();
				delete pADH;

				arrADH[i] = nullptr;
				testSleep(250);
			}
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
#endif

/*  NiProvider and ADH exchange Generic messages. Login domain. */
/*  This test does the following:
 *  1. Starts up a ni-provider and an ADH simulator
 *  2. Checks to see that the ni-provider does intitialize
 *  3. Checks to see that the ni-provider has only one active channel
 *  4. NiProvider opens an item stream for sending a Generic message
 *   a. ADH receives the request and sends a refresh
 *   b. NiProvider receives the refresh message
 *  5. NiProvider sends a Generic message
 *  6. ADH receives the Generic message and sends a new Generic message as answer
 *  7. Check that NiProvider receives the Generic message
 */
TEST_F(OmmNiProviderCreateTestFixture, NiProviderGenericMsgADHLogin)
{
	const unsigned startPortNum = 14340;

	try
	{
		ReqMsg genericRequest;
		GenericMsg genericMsg;

		Msg* msg;
		GenericMsg* pGenericMsg;

		unsigned adhCountOfGenerics = 0;

		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();
		//cout << "ADH Simulator created." << endl;
		testSleep(500);

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!adh.isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: config_providerClient_errorClient" << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		//if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Clear message queues */
		pNiProviderTestClient->clear();
		ASSERT_EQ(pNiProviderTestClient->getMessageQueueSize(), 0);

		/* NiProvider opens an item stream for sending a Generic message */
		adh.enableSendGenericMsg( true );  // ADH will send Generic messages as answer
		adhCountOfGenerics = adh.getCountGeneric();

		//genericRequest.name("GenericItem1").domainType(MMT_MARKET_PRICE).serviceName("DIRECT_FEED");
		genericRequest.name("GenericItem1").domainType(MMT_LOGIN).serviceName("DIRECT_FEED");

		UInt64 genericItemHandle = pNiProvider->registerClient(genericRequest, *pNiProviderTestClient);
		//cout << "Generic Item handle: " << genericItemHandle << " (0x" << std::hex << genericItemHandle << ")" << std::dec << endl;
		ASSERT_NE(genericItemHandle, 0);

		/* ADH simulator receives the request and sends a refresh */
		/* NiProvider receives the refresh msg */
		k = 0;
		while (pNiProviderTestClient->getMessageQueueSize() == 0 && ++k < 20)
		{
			testSleep(200);
		}
		//cout << "NiProvider received the refresh msg" << endl;
		ASSERT_EQ(pNiProviderTestClient->getMessageQueueSize(), 1) << "NiProvider should receive the refresh msg. k: " << k;
		
		msg = pNiProviderTestClient->popMsg();
		//cout << "NiProvider pops refresh msg from queue. " << msg->toString() << endl;

		pNiProviderTestClient->clear();
		ASSERT_EQ(pNiProviderTestClient->getMessageQueueSize(), 0);

		/* NiProvider sends a Generic message */
		genericMsg.clear().name("TestNiGenericItem").complete();

		pNiProvider->submit(genericMsg, genericItemHandle);
		//cout << "NiProvider submit the generic msg." << endl;

		/* ADH simulator receives the Generic and sends a Generic-RTT as answer */
		/* Check that NiProvider receives the Generic message */
		k = 0;
		do {
			testSleep(200);
			k++;
		} while (pNiProviderTestClient->getMessageQueueSize() == 0 && k < 20);
		ASSERT_GE(pNiProviderTestClient->getMessageQueueSize(), 1u) << "NiProvider receives the generic msg. k: " << k;
		ASSERT_GT(adh.getCountGeneric(), adhCountOfGenerics) << "ADH should receive generic from NiProvider.";
		//cout << "NiProvider receives the generic msg. getMessageQueueSize: " << pNiProviderTestClient->getMessageQueueSize() << " k: " << k << endl;
		//cout << "ADH count of generic msgs: " << adh.getCountGeneric() << " (" << adhCountOfGenerics << ")" << endl;

		msg = pNiProviderTestClient->popMsg();
		//cout << "NiProvider pops msg from queue. " << msg->toString() << endl;

		ASSERT_EQ(msg->getDataType(), DataType::GenericMsgEnum);
		pGenericMsg = static_cast<GenericMsg*>(msg);
		ASSERT_EQ(pGenericMsg->getName(), "TestNiGenericItem");
		ASSERT_FALSE(pGenericMsg->hasServiceId());
		ASSERT_EQ(pGenericMsg->getPayload().getDataType(), DataType::ElementListEnum);

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		/* After that NiProvider won't receive/send any messages from/to the ADH */
		stop();
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

/*  NiProvider and ADH exchange Generic messages. On item handle (non Login). */
/*  This test does the following:
 *  1. Starts up a ni-provider and an ADH simulator
 *  2. Checks to see that the ni-provider does intitialize
 *  3. Checks to see that the ni-provider has only one active channel
 *  4. NiProvider sends a Generic message in non-open item handle
 *  5. Check that error client handles invalid handle error
 *  6. NiProvider opens an item stream for sending a Generic message
 *  7. NiProvider sends a Generic message
 *  8. Check that ADH receives the Generic message
 */
TEST_F(OmmNiProviderCreateTestFixture, NiProviderGenericMsgADHItem)
{
	const unsigned startPortNum = 14350;

	try
	{
		GenericMsg genericMsg;

		RefreshMsg itemRefresh;
		FieldList fieldList;
		UInt64 triHandle = 6;

		unsigned adhCountOfRefreshes = 0;
		unsigned adhCountOfGenerics = 0;

		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();
		//cout << "ADH Simulator created." << endl;
		testSleep(500);

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!adh.isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: config_providerClient_errorClient" << endl;
		testSleep(250);

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		//if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Clear message queues */
		pNiProviderTestClient->clear();
		ASSERT_EQ(pNiProviderTestClient->getMessageQueueSize(), 0);

		/* Negative case. */
		/* NiProvider sends a Generic message in non-open item handle (6) */
		adh.enableSendGenericMsg( false );  // Disable ADH sending Generic messages as answer
		adhCountOfGenerics = adh.getCountGeneric();
		pNiProviderTestClient->clear();

		genericMsg.clear().name("TestNiGenericItemMP_NON_OPEN").complete();

		pNiProvider->submit(genericMsg, triHandle);
		//cout << "NiProvider submit the generic msg in non-open item handle." << endl;

		ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "Expected that the errorClient handled an error.";
		ASSERT_EQ(errorClient.getCountOnInvalidHandle(), 1) << "Expected that errorClient handled invalid handle once.";

		ASSERT_EQ(adhCountOfGenerics, adh.getCountGeneric()) << "ADH should not receive generic from NiProvider with invalid handle.";
		//cout << "ADH count of generic msgs: " << adh.getCountGeneric() << " (" << adhCountOfGenerics << ")" << endl;

		/* Open the item handle (6), i.e. send Refresh message */
		
		/* Store counter of received ADH Refresh and Generic messages */
		adhCountOfRefreshes = adh.getCountRefresh();
		adhCountOfGenerics = adh.getCountGeneric();

		pNiProviderTestClient->clear();
		errorClient.clear();

		/* Sends an item refresh from the ni-provider to the ADH */
		itemRefresh.clear()
			.serviceName("NI_PUB").name("TRI.N")
			.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
			.payload(
				fieldList.clear()
				.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
				.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
				.addReal(30, 20, OmmReal::Exponent0Enum)
				.addReal(31, 40, OmmReal::Exponent0Enum)
				.complete())
			.complete();

		pNiProvider->submit(itemRefresh, triHandle);

		/* Checks to see that the ADH received the item refresh */
		k = 0;
		while (adh.getCountRefresh() == adhCountOfRefreshes && adh.isRunning() && k++ < 20)
		{
			testSleep(200);
		}
		//std::cout << "ADH count of Refreshes: " << adh.getCountRefresh() << " (" << adhCountOfRefreshes << ")"
		//	<< " k: " << k << std::endl;

		/* NiProvider sends a Generic message in open item handle (6) */
		genericMsg.clear().name("TestNiGenericItemMP_OPEN").complete();

		pNiProvider->submit(genericMsg, triHandle);
		//cout << "NiProvider submit the generic msg in item handle." << endl;

		/* Checks to see that the ADH received the item refresh */
		k = 0;
		while (adh.getCountGeneric() == adhCountOfGenerics && adh.isRunning() && k++ < 20)
		{
			testSleep(200);
		}
		//std::cout << "ADH count of Generics: " << adh.getCountGeneric() << " (" << adhCountOfGenerics << ")"
		//	<< " k: " << k << std::endl;

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
		ASSERT_EQ(errorClient.getCountOnInvalidHandle(), 0) << "Did not expect that errorClient handled invalid handle.";

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		/* After that NiProvider won't receive/send any messages from/to the ADH */
		stop();
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

#if 0
/* Non-interactive Provider connects to ADH correctly. */
/* Checks whether the ni-provider sends Refresh and Update messages correctly. */
/* It also verifies that the ni-provider reconnects to the ADH simulator properly and continues sending messages as expected. */
/* This test does the following:
 * 1. Starts an ADH simulator that accepts login requests and sends a solicited login refresh
 * 2. Starts up a non-interactive provider configured to connect to the ADH simulator
 * 3. Checks to see that the ni provider does initialize
 * 4. Checks to see that the ni provider has only one active channel and that it got a OPEN/OK login refresh message
 * 5. Sends an item refresh from the ni-provider to the ADH
 * 6. Checks to see that the ADH received the item refresh
 * 7. Simulates a lost connection in the ADH
 * 8. Checks to see that the ni-provider indicates the connection loss: onRefresh? or onStatus? channelInfo?
 * 9. Restarts the ADH
 * 10. Sends an item refresh from the ni-provider to the ADH
 */
TEST_F(OmmNiProviderCreateTestFixture, NiProviderItemADHRefreshUpdateAndRecovery)
{
	const unsigned startPortNum = 14360;
	const unsigned numberOfADHRestarts = 1;  // Number of ADH restarts in the test

	shared_ptr<ADHSimulator> pADH;
	ChannelInformation channelInfo;

	Msg* msg;
	RefreshMsg* refreshMsg;
	StatusMsg* statusMsg;
	unsigned adhCountOfRefreshes = 0;
	unsigned adhCountOfUpdates = 0;

	RefreshMsg itemRefresh;
	UpdateMsg itemUpdate;
	FieldList fieldList;
	UInt64 triHandle = 6;

	try
	{
		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		pADH = make_shared<ADHSimulator>(adhOpts);
		ASSERT_NE(nullptr, pADH) << "ADHSimulator should be initialized.";

		pADH->start();
		//cout << "ADH Simulator created." << endl;
		testSleep(500);

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!pADH->isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE( pADH->isRunning() ) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;
		testSleep(250);

		/* Check the active channel */
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* When the test client assigned for the non-interactive provider we can check the message queue */
		//if ( hasTestClient(testParams.constructorType) )
		{
			ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
			//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

			/* Checks to see that the ni provider got a OPEN/OK login refresh message */
			while (msg = pNiProviderTestClient->popMsg())
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

		//if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Checks to see that NiProvider sends Refresh and Update messages properly */
		for (unsigned i = 0; i < (numberOfADHRestarts + 1); i++)
		{
			/* Store counter of received ADH Refresh and Update messages */
			adhCountOfRefreshes = pADH->getCountRefresh();
			adhCountOfUpdates = pADH->getCountUpdate();

			/* Sends an item refresh and update from the ni-provider to the ADH */
			itemRefresh.clear()
				.serviceName("NI_PUB").name("TRI.N")
				.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
				.payload(
					fieldList.clear()
					.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
					.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
					.addReal(30, 20, OmmReal::Exponent0Enum)
					.addReal(31, 40, OmmReal::Exponent0Enum)
					.complete())
				.complete();

			pNiProvider->submit(itemRefresh, triHandle);

			testSleep(100);

			itemUpdate.clear()
				.serviceName("NI_PUB").name("TRI.N")
				.payload(fieldList.clear()
					.addReal(22, 4101 + i, OmmReal::ExponentNeg2Enum)
					.addReal(30, 21 + i, OmmReal::Exponent0Enum)
					.complete());

			pNiProvider->submit(itemUpdate, triHandle);
		
			/* Checks to see that the ADH received the item refresh/update */
			k = 0;
			while ((pADH->getCountRefresh() == adhCountOfRefreshes || pADH->getCountUpdate() == adhCountOfUpdates)
				&& pADH->isRunning() && k++ < 20)
			{
				testSleep(200);
			}
			//std::cout << "ADH count of Refreshes: " << pADH->getCountRefresh() << " (" << adhCountOfRefreshes << ")"
			//	<< ", Updates: " << pADH->getCountUpdate() << " (" << adhCountOfUpdates << ")"
			//	<< " k: " << k << std::endl;

			/* Simulates a lost connection in the ADH */
			if (i < numberOfADHRestarts)
			{
				pNiProviderTestClient->clear();
				pADH->stop();
				testSleep(1000);

				/* Check that ni-provider lost connection */
				k = 0;
				do {
					testSleep(500);
					pNiProvider->getChannelInformation(channelInfo);
					k++;
				} while (channelInfo.getChannelState() == ChannelInformation::ChannelState::ActiveEnum && k < 20);
				ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected NiProvider lost connection. k: " << k;
				//cout << "NiProvider lost connection. k: " << k << endl;

				/* Checks to see Status message */
				{
					ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
					//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

					while (msg = pNiProviderTestClient->popMsg())
					{
						ASSERT_EQ(msg->getDomainType(), RsslDomainTypes::RSSL_DMT_LOGIN);
						ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
						statusMsg = static_cast<StatusMsg*>(msg);
						ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
						ASSERT_NE(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
					}
				}

				/* Restarts the ADH */
				pADH->start();
				//cout << "ADH Simulator re-started." << endl;
				testSleep(500);

				/* Wait for ADH simulator to start */
				k = 0;
				while (!pADH->isRunning() && k++ < 20)
				{
					testSleep(250);
				}
				EXPECT_TRUE(pADH->isRunning()) << "ADH Simulator failed to start. k: " << k;
				//cout << "ADH Simulator thread is running. k: " << k << endl;

				/* Checks to see NiProvider the active channel */
				k = 0;
				do {
					testSleep(500);
					pNiProvider->getChannelInformation(channelInfo);
					k++;
				} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
				ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected NiProvider re-established connection. k: " << k;
			}
		}

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		/* After that NiProvider won't receive/send any messages from/to the ADH */
		stop();
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/* Non-interactive Provider connects to set of ADH correctly. */
/* Checks whether the ni-provider sends Refresh and Update messages correctly. */
/* It also verifies that the ni-provider reconnects to the ADH simulator properly and continues sending messages as expected. */
/* This test does the following:
 * 1. Starts set of ADH simulators
 * 2. Starts up a non-interactive provider configured to connect to the ChannelSet of ADH simulators
 * 3. Checks to see that the ni provider does initialize
 * 4. Checks to see that the ni provider has only one active channel and that it got a OPEN/OK login refresh message
 * 5. Sends an item refresh and update from the ni-provider to the ADH
 * 6. Checks to see that the ADH received the item refresh and update
 * 7. Simulates a lost connection in the ADH
 * 8. Checks to see that the ni-provider indicates the connection loss: onRefresh? or onStatus? channelInfo?
 * 9. Checks to see that the ni-provider switches to the next ADH
 * 10. Repeats steps 5-9 for all ADHs in the set
 */
TEST_F(OmmNiProviderCreateTestFixture, NiProviderMultiADHItemRefreshUpdateAndRecovery)
{
	unsigned i, j, k;
	const unsigned NUMConnections = 3;  // The number of ADHs and channels available to the ni-provider
	const unsigned startPortNum = 14370;
	const unsigned numberOfADHRestarts = NUMConnections;  // Number of ADH restarts in the test

	ADHSimulatorOptions adhOpts[NUMConnections];

	shared_ptr<ADHSimulator> arrADH[NUMConnections];
	ADHSimulator* pADH;

	RsslCreateReactorOptions reactorOpts[NUMConnections];
	ChannelInformation channelInfo;

	Msg* msg;
	RefreshMsg* refreshMsg;
	StatusMsg* statusMsg;
	unsigned adhCountOfRefreshes = 0;
	unsigned adhCountOfUpdates = 0;

	RefreshMsg itemRefresh;
	UpdateMsg itemUpdate;
	FieldList fieldList;
	UInt64 triHandle = 6;

	try
	{
		/* Initialize ADH Simulator */
		for (i = 0; i < NUMConnections; i++)
		{	
			rsslClearCreateReactorOptions(&reactorOpts[i]);
			
			adhOpts[i].clear();
			adhOpts[i].pReactorOptions = &reactorOpts[i];
			snprintf(adhOpts[i].portNo, sizeof(adhOpts[i].portNo), "%u", (startPortNum + i));

			arrADH[i] = make_shared<ADHSimulator>(adhOpts[i]);
			ASSERT_NE(nullptr, arrADH[i]) << "ADHSimulator[" << i << "] should be initialized.";

			arrADH[i]->start();
			//cout << "ADH Simulator[" << i << "] created." << endl;
			testSleep(500);
			
			/* Wait for ADH simulator to start */
			k = 0;
			while (!arrADH[i]->isRunning() && k++ < 20)
			{
				testSleep(250);
			}
			EXPECT_TRUE(arrADH[i]->isRunning()) << "ADH Simulator[" << i << "] failed to start. k: " << k;
			//cout << "ADH Simulator[" << i << "] thread is running. k: " << k << endl;
		}

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.numChannels = NUMConnections;  // The number of channels available to the ni-provider
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: onfig_providerClient_errorClient" << endl;
		testSleep(250);

		/* Check the active channel */
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

		/* When the test client assigned for the non-interactive provider we can check the message queue */
		//if ( hasTestClient(testParams.constructorType) )
		{
			ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
			//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

			/* Checks to see that the ni provider got a OPEN/OK login refresh message */
			while (msg = pNiProviderTestClient->popMsg())
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

		//if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Checks to see that NiProvider sends Refresh and Update messages properly */
		for (i = 0; i < (numberOfADHRestarts + 1); i++)
		{
			j = (i % NUMConnections);  // current ADH index: 0, 1, 2, 0
			pADH = arrADH[j].get();

			/* Check the port connection */
			pNiProvider->getChannelInformation(channelInfo);
			ASSERT_EQ(channelInfo.port(), (startPortNum + j)) << "Expected that NiProvider is connected to ADH[" << j << "]";
			//cout << endl << "NiProvider is connected to ADH[" << j << "] on port " << channelInfo.port() << endl;
			//cout << "ADH[" << j << "].opts.portNo: " << pADH->options.portNo << " : " << adhOpts[j].portNo << endl;

			/* Store counter of received ADH Refresh and Update messages */
			adhCountOfRefreshes = pADH->getCountRefresh();
			adhCountOfUpdates = pADH->getCountUpdate();

			/* Sends an item refresh and update from the ni-provider to the ADH */
			itemRefresh.clear()
				.serviceName("NI_PUB").name("TRI.N")
				.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
				.payload(
					fieldList.clear()
					.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
					.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
					.addReal(30, 20, OmmReal::Exponent0Enum)
					.addReal(31, 40, OmmReal::Exponent0Enum)
					.complete())
				.complete();

			pNiProvider->submit(itemRefresh, triHandle);

			testSleep(100);

			itemUpdate.clear()
				.serviceName("NI_PUB").name("TRI.N")
				.payload(fieldList.clear()
					.addReal(22, 4101 + i, OmmReal::ExponentNeg2Enum)
					.addReal(30, 21 + i, OmmReal::Exponent0Enum)
					.complete());

			pNiProvider->submit(itemUpdate, triHandle);

			/* Checks to see that the ADH received the item refresh/update */
			k = 0;
			while ((pADH->getCountRefresh() == adhCountOfRefreshes || pADH->getCountUpdate() == adhCountOfUpdates)
				&& pADH->isRunning() && k++ < 20)
			{
				testSleep(200);
			}
			//std::cout << "ADH count of Refreshes: " << pADH->getCountRefresh() << " (" << adhCountOfRefreshes << ")"
			//	<< ", Updates: " << pADH->getCountUpdate() << " (" << adhCountOfUpdates << ")"
			//	<< " k: " << k << std::endl;

			/* Simulates a lost connection in the ADH */
			if (i < numberOfADHRestarts)
			{
				pNiProviderTestClient->clear();
				pADH->stop();
				//cout << "ADH[" << j << "] stopped." << endl;
				testSleep(1000);

				/* Check that ni-provider lost connection */
				k = 0;
				do {
					testSleep(500);
					pNiProvider->getChannelInformation(channelInfo);
					k++;
				} while (channelInfo.getChannelState() == ChannelInformation::ChannelState::ActiveEnum && k < 20);
				ASSERT_NE(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected NiProvider lost connection. k: " << k;
				//cout << "NiProvider lost connection. k: " << k << endl;

				/* Checks to see Status message */
				{
					ASSERT_NE(0, pNiProviderTestClient->getMessageQueueSize());
					//cout << "Message queue size: " << pNiProviderTestClient->getMessageQueueSize() << endl;

					while (msg = pNiProviderTestClient->popMsg())
					{
						ASSERT_EQ(msg->getDomainType(), RsslDomainTypes::RSSL_DMT_LOGIN);
						ASSERT_EQ(msg->getDataType(), DataType::StatusMsgEnum);
						statusMsg = static_cast<StatusMsg*>(msg);
						ASSERT_EQ(statusMsg->getState().getStreamState(), OmmState::StreamState::OpenEnum);
						ASSERT_NE(statusMsg->getState().getDataState(), OmmState::DataState::OkEnum);
					}
				}

				/* Restarts the ADH */
				pADH->start();
				//cout << "ADH[" << j << "] Simulator re-started." << endl;
				testSleep(500);

				/* Wait for ADH simulator to start */
				k = 0;
				while (!pADH->isRunning() && k++ < 20)
				{
					testSleep(250);
				}
				EXPECT_TRUE(pADH->isRunning()) << "ADH Simulator failed to start. k: " << k;
				//cout << "ADH[" << j << "] Simulator thread is running. k: " << k << endl;

				/* Checks to see NiProvider switched to the next ADH */
				k = 0;
				do {
					testSleep(500);
					pNiProvider->getChannelInformation(channelInfo);
					k++;
				} while (channelInfo.getChannelState() != ChannelInformation::ChannelState::ActiveEnum && k < 20);
				ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum) << "Expected NiProvider switched to the next ADH. k: " << k;
			}
			//cout << endl;
		}

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		/* After that NiProvider won't receive/send any messages from/to the ADH */
		stop();
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}
#endif

/* Checks that Non-interactive Provider calls modifyIOCtl */
/* This sets new values for the set of parameters */
/* the changes to which can be checked using the getChannelInformation method. */
/* This test does the following:
 * 1. Starts an ADH simulator that accepts login requests and sends a solicited login refresh
 * 2. Starts up a non-interactive provider configured to connect to the ADH simulator
 * 3. Checks to see that the ni provider does initialize
 * 4. Checks to see that the ni provider has only one active channel.
 * 5. Calls modifyIOCtl for the set of parameters
 * RSSL_MAX_NUM_BUFFERS, RSSL_NUM_GUARANTEED_BUFFERS
 * RSSL_HIGH_WATER_MARK, RSSL_DEBUG_FLAGS
 * RSSL_SERVER_NUM_POOL_BUFFERS, RSSL_COMPRESSION_THRESHOLD
 * Under Windows only: RSSL_SYSTEM_READ_BUFFERS, RSSL_SYSTEM_WRITE_BUFFERS
 */
TEST_F(OmmNiProviderCreateTestFixture, modifyIOCtl)
{
	//const OmmNiProviderCreateTestParams& testParams = GetParam();
	unsigned startPortNum = 14380;

	try
	{
		//startPortNum += static_cast<int>(testParams.constructorType);

		/* Initialize ADH Simulator */
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);

		char portNo[ADHSimulatorOptions::MAX_PORTNO_LEN];
		snprintf(portNo, sizeof(portNo), "%u", startPortNum);

		ADHSimulatorOptions adhOpts(&reactorOpts, portNo);

		ADHSimulator adh(adhOpts);
		adh.start();
		//cout << "ADH Simulator created. " << portNo << endl;
		testSleep(500);

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!adh.isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);

//		if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Call modifyIOCtl */
		Int32 Value;
		ChannelInformation channelInfo1;

		/* The maximum number of output buffers */

		/* RSSL_MAX_NUM_BUFFERS: value is not changed if it less than current  */
		Value = channelInfo.getMaxOutputBuffers() > 1 ? channelInfo.getMaxOutputBuffers() - 1 : 0;
		pNiProvider->modifyIOCtl(RSSL_MAX_NUM_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;

		ASSERT_EQ(channelInfo.getMaxOutputBuffers(), channelInfo1.getMaxOutputBuffers())
			<< "Expected that modifyIOCtl does not change the number of input buffers.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_MAX_NUM_BUFFERS */
		Value = channelInfo.getMaxOutputBuffers() + 1000;
		pNiProvider->modifyIOCtl(RSSL_MAX_NUM_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;

		ASSERT_EQ(channelInfo1.getMaxOutputBuffers(), Value);
		ASSERT_NE(channelInfo.getMaxOutputBuffers(), channelInfo1.getMaxOutputBuffers())
			<< "Expected that modifyIOCtl changed the number of input buffers.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_NUM_GUARANTEED_BUFFERS: value is less than the current one */
		Value = 50;
		pNiProvider->modifyIOCtl(RSSL_NUM_GUARANTEED_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;

		ASSERT_EQ(channelInfo1.getGuaranteedOutputBuffers(), Value);
		ASSERT_NE(channelInfo.getGuaranteedOutputBuffers(), channelInfo1.getGuaranteedOutputBuffers())
			<< "Expected that modifyIOCtl does not change the guaranteed output buffers.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_NUM_GUARANTEED_BUFFERS: value is greater than the current one */
		Value = channelInfo.getGuaranteedOutputBuffers() + 1000;
		pNiProvider->modifyIOCtl(RSSL_NUM_GUARANTEED_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;

		ASSERT_EQ(channelInfo1.getGuaranteedOutputBuffers(), Value);
		ASSERT_NE(channelInfo.getGuaranteedOutputBuffers(), channelInfo1.getGuaranteedOutputBuffers())
			<< "Expected that modifyIOCtl changed the guaranteed output buffers.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_HIGH_WATER_MARK: value is negative value - failure */
		Value = -500;
		pNiProvider->modifyIOCtl(RSSL_HIGH_WATER_MARK, Value);

		ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "The test case expects an error.";
		ASSERT_EQ(errorClient.getCountOnInvalidUsage(), 1) << "Expected that errorClient handled invalid usage.";
		errorClient.clear();

		/* RSSL_HIGH_WATER_MARK: value is positive value - ok */
		Value = 7500;
		pNiProvider->modifyIOCtl(RSSL_HIGH_WATER_MARK, Value);

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_DEBUG_FLAGS: value is positive value - ok (any value) */
		Value = RSSL_DEBUG_IPC_DUMP_IN | RSSL_DEBUG_IPC_DUMP_OUT;
		pNiProvider->modifyIOCtl(RSSL_DEBUG_FLAGS, Value);

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

#if 0	// disabled because RTSDK-10324
		/* RSSL_PRIORITY_FLUSH_ORDER: negative cases */
		{
			char testFlush[32] = "";

			Value = reinterpret_cast<Int32>( testFlush );
			pNiProvider->modifyIOCtl(RSSL_PRIORITY_FLUSH_ORDER, Value);

			cout << "modifyIOCtl with RSSL_PRIORITY_FLUSH_ORDER testFlush should fail: " << errorClient.isCalledAnyErrorHandler() << endl;
			ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "The test case expects an error.";
			ASSERT_EQ(errorClient.getCountOnInvalidUsage(), 1) << "Expected that errorClient handled invalid usage.";
			errorClient.clear();
		}
#endif

		/* RSSL_SERVER_NUM_POOL_BUFFERS: this is a negative case, because NiProvider is not supported it */
		Value = 100;
		pNiProvider->modifyIOCtl(RSSL_SERVER_NUM_POOL_BUFFERS, Value);

		ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "The test case expects an error.";
		ASSERT_EQ(errorClient.getCountOnInvalidUsage(), 1) << "Expected that errorClient handled invalid usage.";
		errorClient.clear();

		/* RSSL_COMPRESSION_THRESHOLD: value is positive value - ok (any value) */
		/* the current channel is not configured for compression */
		Value = 512;
		pNiProvider->modifyIOCtl(RSSL_COMPRESSION_THRESHOLD, Value);

		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_SERVER_PEAK_BUF_RESET: this is a negative case, because NiProvider is not supported it */
		Value = 100;
		pNiProvider->modifyIOCtl(RSSL_SERVER_PEAK_BUF_RESET, Value);

		ASSERT_TRUE(errorClient.isCalledAnyErrorHandler()) << "The test case expects an error.";
		ASSERT_EQ(errorClient.getCountOnInvalidUsage(), 1) << "Expected that errorClient handled invalid usage.";
		errorClient.clear();

#ifdef _WIN32
		/* RSSL_SYSTEM_READ_BUFFERS: value is less than the current one */
		Value = 30000;
		pNiProvider->modifyIOCtl(RSSL_SYSTEM_READ_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getSysRecvBufSize()1: " << channelInfo1.getSysRecvBufSize() << endl;

		ASSERT_EQ(channelInfo1.getSysRecvBufSize(), Value);
		ASSERT_NE(channelInfo.getSysRecvBufSize(), channelInfo1.getSysRecvBufSize())
			<< "Expected that modifyIOCtl does not change the system receive buffer size.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_SYSTEM_READ_BUFFERS: value is greater than the current one */
		Value = channelInfo.getSysRecvBufSize() + 1000;
		pNiProvider->modifyIOCtl(RSSL_SYSTEM_READ_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getSysRecvBufSize()1: " << channelInfo1.getSysRecvBufSize() << endl;

		ASSERT_EQ(channelInfo1.getSysRecvBufSize(), Value);
		ASSERT_NE(channelInfo.getSysRecvBufSize(), channelInfo1.getSysRecvBufSize())
			<< "Expected that modifyIOCtl changed the system receive buffer size.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_SYSTEM_WRITE_BUFFERS: value is less than the current one */
		Value = 35000;
		pNiProvider->modifyIOCtl(RSSL_SYSTEM_WRITE_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getSysSendBufSize()1: " << channelInfo1.getSysSendBufSize() << endl;

		ASSERT_EQ(channelInfo1.getSysSendBufSize(), Value);
		ASSERT_NE(channelInfo.getSysSendBufSize(), channelInfo1.getSysSendBufSize())
			<< "Expected that modifyIOCtl does not change the system send buffer size.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* RSSL_SYSTEM_WRITE_BUFFERS: value is greater than the current one */
		Value = channelInfo.getSysSendBufSize() + 1500;
		pNiProvider->modifyIOCtl(RSSL_SYSTEM_WRITE_BUFFERS, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getSysSendBufSize()1: " << channelInfo1.getSysSendBufSize() << endl;

		ASSERT_EQ(channelInfo1.getSysSendBufSize(), Value);
		ASSERT_NE(channelInfo.getSysSendBufSize(), channelInfo1.getSysSendBufSize())
			<< "Expected that modifyIOCtl changed the system send buffer size.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
#endif  // _WIN32

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		stop();
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

/* Checks that Non-interactive Provider calls modifyIOCtl for a channel with compression */
/* This sets new values for the set of parameters */
/* the changes to which can be checked using the getChannelInformation method. */
/* This test does the following:
 * 1. Starts an ADH simulator that accepts login requests and sends a solicited login refresh
 *    ADH is configured to accept a connection with compression
 * 2. Starts up a non-interactive provider configured to connect to the ADH simulator
 * 3. Checks to see that the ni provider does initialize
 * 4. Checks to see that the ni provider has only one active channel.
 * 5. Calls modifyIOCtl for the set of parameters
 * RSSL_COMPRESSION_THRESHOLD
 */
TEST_F(OmmNiProviderCreateTestFixture, modifyIOCtlCompression)
{
	unsigned startPortNum = 14385;

	try
	{
		//startPortNum += static_cast<int>(testParams.constructorType);

		/* Initialize ADH Simulator */
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
		//cout << "ADH Simulator created. " << portNo << endl;

		/* Wait for ADH simulator to start */
		unsigned k = 0;
		while (!adh.isRunning() && k++ < 20)
		{
			testSleep(250);
		}
		EXPECT_TRUE(adh.isRunning()) << "ADH Simulator failed to start. k: " << k;
		//cout << "ADH Simulator thread is running. k: " << k << endl;

		/* Create NiProvider instance, configure a channel with compression */
		NiProviderProgrammaticTestConfig niProvProgConfig;
		niProvProgConfig.startPortNum = startPortNum;
		niProvProgConfig.enableCompression = true;
		//niProvProgConfig.loggerSeverity = OmmLoggerClient::VerboseEnum;

		OmmProvider* pNiProvider =
			createOmmNiProvider(OmmProviderConstructorType::config_providerClient_errorClient, niProvProgConfig);

		/* Check that NiProvider is created */
		ASSERT_NE(pNiProvider, nullptr) << "Expected NiProvider to be initialized.";
		//cout << "NiProvider created with constructor type: " << testParams.constructorType << endl;

		/* Check the active channel */
		ChannelInformation channelInfo;
		pNiProvider->getChannelInformation(channelInfo);
		//cout << "Channel Info: " << channelInfo.toString() << endl;
		ASSERT_EQ(channelInfo.getChannelState(), ChannelInformation::ChannelState::ActiveEnum);
		ASSERT_NE(channelInfo.getCompressionType(), ChannelInformation::CompressionType::NoneEnum)
			<< "Expected that the channel was configured with compression enabled.";

//		if ( hasErrorClient(testParams.constructorType) )
		{
			ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";
			//cout << "isCalledAnyErrorHandler: " << errorClient.isCalledAnyErrorHandler() << endl;
		}

		/* Call modifyIOCtl */
		Int32 Value;
		ChannelInformation channelInfo1;

		/* RSSL_COMPRESSION_THRESHOLD: value is positive value - ok (any value) */
		/* the current channel is configured with compression enabled */
		Value = 512;
		pNiProvider->modifyIOCtl(RSSL_COMPRESSION_THRESHOLD, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getCompressionThreshold()1: " << channelInfo1.getCompressionThreshold() << endl;

		ASSERT_NE(channelInfo1.getCompressionType(), ChannelInformation::CompressionType::NoneEnum)
			<< "Expected that the channel was configured with compression enabled.";
		ASSERT_EQ(channelInfo.getCompressionType(), channelInfo1.getCompressionType());

		ASSERT_EQ(channelInfo1.getCompressionThreshold(), Value);
		ASSERT_NE(channelInfo.getCompressionThreshold(), channelInfo1.getCompressionThreshold())
			<< "Expected that modifyIOCtl changed the compression threshold value.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";


		Value = 5120;
		pNiProvider->modifyIOCtl(RSSL_COMPRESSION_THRESHOLD, Value);

		pNiProvider->getChannelInformation(channelInfo1);
		//cout << endl << "Channel Info1: " << channelInfo1.toString() << endl;
		//cout << endl << "getCompressionThreshold()1: " << channelInfo1.getCompressionThreshold() << endl;

		ASSERT_NE(channelInfo1.getCompressionType(), ChannelInformation::CompressionType::NoneEnum)
			<< "Expected that the channel was configured with compression enabled.";
		ASSERT_EQ(channelInfo.getCompressionType(), channelInfo1.getCompressionType());

		ASSERT_EQ(channelInfo1.getCompressionThreshold(), Value);
		ASSERT_NE(channelInfo.getCompressionThreshold(), channelInfo1.getCompressionThreshold())
			<< "Expected that modifyIOCtl changed the compression threshold value.";
		ASSERT_FALSE(errorClient.isCalledAnyErrorHandler()) << "Did not expect that the errorClient handled any error.";

		/* Finish test. Clean all resources */
		/* Stop NiProvider gracefully and clean its resources */
		stop();
	}
	catch (const OmmException& exception)
	{
		cout << exception.toString() << endl;
		ASSERT_TRUE(false) << "uncaught exception in test: " << exception.getText();
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
		ASSERT_TRUE(false) << "Unexpected exception caught.";
	}
}

INSTANTIATE_TEST_SUITE_P(
	OmmNiProvider,
	OmmNiProviderCreateTestFixture,
	::testing::Values(
		OmmNiProviderCreateTestParams( OmmProviderConstructorType::config ),
		OmmNiProviderCreateTestParams( OmmProviderConstructorType::config_providerClient ),
		OmmNiProviderCreateTestParams( OmmProviderConstructorType::config_errorClient ),
		OmmNiProviderCreateTestParams( OmmProviderConstructorType::config_providerClient_errorClient )
	)
);
