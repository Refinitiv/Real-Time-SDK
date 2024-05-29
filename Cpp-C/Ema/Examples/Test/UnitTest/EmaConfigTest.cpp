/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2024 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "EmaAppClient.h"
#include "OmmConsumerConfigImpl.h"
#include "OmmIProviderConfigImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "OmmConsumerImpl.h"
#include "OmmIProviderImpl.h"
#include "OmmNiProviderImpl.h"
#include "EmaConfig.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

#define OVERFLOW_MAX_UNSIGNED_INT16		0x10000 // 0xFFFF + 1
#define OVERFLOW_MAX_UNSIGNED_INT32		0x100000000 // 0xFFFFFFFF + 1

class EmaConfigTest : public ::testing::Test {
public:
	static const char* emaConfigXMLFileNameTest;
	static const char* fieldDictionaryFileNameTest;
	static const char* enumTableFileNameTest;

	static const char* fieldDictionaryFileNameDefault;
	static const char* enumTableFileNameDefault;

	void SetUp() {
		SCOPED_TRACE("EmaConfigTest SetUp");
		if (hasRun == true)
			return;

		// The default path uses when creating Config instance
		// and was set an empty path to EmaConfigTest.xml or a directory
		EmaConfigBaseImpl::setDefaultConfigFileName(emaConfigXMLFileNameTest);

		EmaString workingDir;
		ASSERT_EQ(getCurrentDir(workingDir), true)
			<< "Error: failed to load config file from current working dir "
			<< workingDir.c_str();
		
		configPath.append(workingDir).append(emaConfigXMLFileNameTest);
		SCOPED_TRACE("Loading Ema config file from ");
		SCOPED_TRACE(configPath);

		SCOPED_TRACE("Starting provider1 with port 14002 and provider2 with port 14008\n");
		try
		{
			provider1 = new OmmProvider(OmmIProviderConfig(configPath).port("14002"), appClient);
			provider2 = new OmmProvider(OmmIProviderConfig(configPath).port("14008"), appClient);
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfigHttp()";
		}
		hasRun = true;
	}

	void TearDown() {
	}

	static bool hasRun;
	static EmaString configPath;
	static AppClient appClient;
	static OmmProvider* provider1;
	static OmmProvider* provider2;
};

bool EmaConfigTest::hasRun(false);
EmaString EmaConfigTest::configPath = EmaString("");
OmmProvider* EmaConfigTest::provider1 = 0;
OmmProvider* EmaConfigTest::provider2 = 0;
AppClient EmaConfigTest::appClient;

const char* EmaConfigTest::emaConfigXMLFileNameTest = "//EmaConfigTest.xml";
const char* EmaConfigTest::fieldDictionaryFileNameTest = "./RDMFieldDictionaryTest";
const char* EmaConfigTest::enumTableFileNameTest = "./enumtypeTest.def";

const char* EmaConfigTest::fieldDictionaryFileNameDefault = "./RDMFieldDictionary";
const char* EmaConfigTest::enumTableFileNameDefault = "./enumtype.def";

TEST_F(EmaConfigTest, testLoadingConfigurationsFromFile)
{
	OmmConsumerConfigImpl config(configPath);
	config.configErrors().printErrors(OmmLoggerClient::WarningEnum);

	//SCOPED_TRACE("printing config ...\n");
	//config.print();

	bool debugResult;
	EmaString retrievedValue;
	Int64 intValue;
	UInt64 uintValue;
	Double doubleValue;
	RsslConnectionTypes channelType, serverType;
	RsslCompTypes compType;
	OmmLoggerClient::LoggerType loggerType;
	OmmLoggerClient::Severity loggerSeverity;
	Dictionary::DictionaryType dictionaryType;
	RsslReactorWarmStandbyMode warmStandbyMode;

	config.configErrors().clear();

	// get default consumer name from the DefaultXML.h file
	debugResult = config.get<EmaString>( "hostName", retrievedValue );
	EXPECT_FALSE( debugResult ) << "correctly detecting missing value in configuration " ;

	// expectation: set in EmaConfig.xml
	retrievedValue = config.getConfiguredName();
	EXPECT_TRUE( retrievedValue == "Consumer_2" ) << "retrieving default consumer from OmmConsumerConfigImpl";
	debugResult = config.get<EmaString>( "ConsumerGroup|DefaultConsumer", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Consumer_2" ) << "extracting default consumer from EmaConfig.xml";

	// Check all values from Consumer_1
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Consumer_1" ) << "extracting the first consumer name from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_1|Channel", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Channel_1" ) << "extracting Channel name from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_1|Logger", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Logger_1" ) << "extracting Logger name from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_1|Dictionary", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Dictionary_1" ) << "extracting Dictionary name from EmaConfig.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_1|XmlTraceFileName", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "EmaMyTrace" ) << "extracting XmlTraceFileName from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_1|XmlTraceToFile", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceToFile from EmaConfig.xml";

	// Checks all values from Consumer_2
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|Channel", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Channel_2" ) << "extracting Channel name from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|Logger", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Logger_2" ) << "extracting Logger name from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|Dictionary", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Dictionary_2" ) << "extracting Dictionary name from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|ItemCountHint", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 500000) << "extracting ItemCountHint from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|ServiceCountHint", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 655) << "extracting ServiceCountHint from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|ObeyOpenWindow", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 0) << "extracting ObeyOpenWindow from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|PostAckTimeout", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 7000) << "extracting PostAckTimeout from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|RequestTimeout", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 8000) << "extracting RequestTimeout from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|MaxOutstandingPosts", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 90000) << "extracting MaxOutstandingPosts from EmaConfig.xml";
	debugResult = config.get<Int64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|DispatchTimeoutApiThread", intValue );
	EXPECT_TRUE( debugResult && intValue == 90) << "extracting DispatchTimeoutApiThread from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|CatchUnhandledException", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 0) << "extracting CatchUnhandledException from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|MaxDispatchCountApiThread", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 400) << "extracting MaxDispatchCountApiThread from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|MaxDispatchCountUserThread", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 5) << "extracting MaxDispatchCountUserThread from EmaConfig.xml";
	debugResult = config.get<Int64>( "ConsumerGroup|ConsumerList|Consumer.Consumer_2|PipePort", intValue );
	EXPECT_TRUE( debugResult && intValue == 7001) << "extracting PipePort from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ReconnectAttemptLimit", intValue);
	EXPECT_TRUE(debugResult && intValue == 10) << "extracting ReconnectAttemptLimit from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ReconnectMinDelay", intValue);
	EXPECT_TRUE(debugResult && intValue == 123) << "extracting ReconnectMinDelay from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ReconnectMaxDelay", intValue);
	EXPECT_TRUE(debugResult && intValue == 456) << "extracting ReconnectMaxDelay from EmaConfig.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceFileName", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "EmaMyTrace2" ) << "extracting XmlTraceFileName from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceMaxFileSize", intValue);
	EXPECT_TRUE(debugResult && intValue == 66666666) << "extracting XmlTraceMaxFileSize from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceToFile", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceToFile from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceToStdout", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting XmlTraceToStdout from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceToMultipleFiles", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceToMultipleFiles from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceWrite", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceWrite from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceRead", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceRead from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTracePing", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTracePing from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTracePingOnly", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTracePingOnly from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceHex", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceHex from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|MsgKeyInUpdates", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting MsgKeyInUpdates from EmaConfig.xml";
	debugResult = config.get<Double>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|TokenReissueRatio", doubleValue);
	EXPECT_TRUE(debugResult && doubleValue == 0.70) << "extracting TokenReissueRatio from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ReissueTokenAttemptLimit", intValue);
	EXPECT_TRUE(debugResult && intValue == 5) << "extracting ReissueTokenAttemptLimit from EmaConfig.xml";
	debugResult = config.get<Int64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ReissueTokenAttemptInterval", intValue);
	EXPECT_TRUE(debugResult && intValue == 6000) << "extracting ReissueTokenAttemptInterval from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceDump", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceDump from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|CatchUnknownJsonFids", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting CatchUnknownJsonFids from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|CatchUnknownJsonKeys", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting CatchUnknownJsonKeys from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|CloseChannelFromConverterFailure", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting CloseChannelFromConverterFailure from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|DefaultServiceID", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting DefaultServiceID from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|JsonExpandedEnumFields", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting JsonExpandedEnumFields from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|OutputBufferSize", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 99999) << "extracting OutputBufferSize from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|JsonTokenIncrementSize", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 999992) << "extracting JsonTokenIncrementSize from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|ShouldInitializeCPUIDlib", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting ShouldInitializeCPUIDlib from EmaConfig.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|RestProxyHostName", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "restproxylocalhost") << "extracting RestProxyHostName name from EmaConfig.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|RestProxyPort", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "9028") << "extracting RestProxyPort name from EmaConfig.xml";

	/*Check sendJsonConvError in Consumer group*/
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|SendJsonConvError", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 1) << "extracting  SendJsonConvError from EmaConfig.xml";

	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_3|ShouldInitializeCPUIDlib", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting ShouldInitializeCPUIDlib from EmaConfig.xml";

	// Checks all values from Consumer_8
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_8|WarmStandbyChannelSet", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "WarmStandbyChannel_1, WarmStandbyChannel_2") << "extracting WarmStandbyChannelSet name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_8|Logger", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Logger_1") << "extracting Logger name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("ConsumerGroup|ConsumerList|Consumer.Consumer_8|Dictionary", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Dictionary_2") << "extracting Dictionary name from EmaConfigTest.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_8|XmlTraceToStdout", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceToStdout name from EmaConfigTest.xml";

	// Checks all values from WarmStandbyChannel_1
	debugResult = config.get<EmaString>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_1|StartingActiveServer", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Server_Info_1") << "extracting StartingActiveServer name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_1|StandbyServerSet", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Server_Info_2") << "extracting StandbyServerSet name from EmaConfigTest.xml";
	debugResult = config.get<RsslReactorWarmStandbyMode>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_1|WarmStandbyMode", warmStandbyMode);
	EXPECT_TRUE(debugResult && warmStandbyMode == RSSL_RWSB_MODE_LOGIN_BASED) << "extracting WarmStandbyMode name from EmaConfigTest.xml";

	// Checks all values from WarmStandbyChannel_2
	debugResult = config.get<EmaString>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_2|StartingActiveServer", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Server_Info_2") << "extracting StartingActiveServer name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_2|StandbyServerSet", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Server_Info_1") << "extracting StandbyServerSet name from EmaConfigTest.xml";
	debugResult = config.get<RsslReactorWarmStandbyMode>("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.WarmStandbyChannel_2|WarmStandbyMode", warmStandbyMode);
	EXPECT_TRUE(debugResult && warmStandbyMode == RSSL_RWSB_MODE_SERVICE_BASED) << "extracting WarmStandbyMode name from EmaConfigTest.xml";

	// Checks all values from Server_Info_1
	debugResult = config.get<EmaString>("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.Server_Info_1|Channel", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Channel_1") << "extracting Channel name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.Server_Info_1|PerServiceNameSet", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "DIRECT_FEED") << "extracting PerServiceNameSet name from EmaConfigTest.xml";

	// Checks all values from Server_Info_2
	debugResult = config.get<EmaString>("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.Server_Info_2|Channel", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "Channel_4") << "extracting Channel name from EmaConfigTest.xml";
	debugResult = config.get<EmaString>("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.Server_Info_2|PerServiceNameSet", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "DIRECT_FEED2, DIRECT_FEED3") << "extracting PerServiceNameSet name from EmaConfigTest.xml";

	// Checks all values from Channel_1
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Channel_1" ) << "extracting the first channel name from EmaConfig.xml";
	debugResult = config.get<RsslConnectionTypes>( "ChannelGroup|ChannelList|Channel.Channel_1|ChannelType", channelType );
	EXPECT_TRUE( debugResult && channelType == RSSL_CONN_TYPE_SOCKET) << "extracting ChannelType from EmaConfig.xml";
	debugResult = config.get<RsslCompTypes>( "ChannelGroup|ChannelList|Channel.Channel_1|CompressionType", compType );
	EXPECT_TRUE( debugResult && compType == RSSL_COMP_NONE) << "extracting CompressionType from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|GuaranteedOutputBuffers", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 5000) << "extracting GuaranteedOutputBuffers from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|NumInputBuffers", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 7000) << "extracting NumInputBuffers from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|SysRecvBufSize", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 125236) << "extracting SysRecvBufSize from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|SysSendBufSize", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 569823) << "extracting SysSendBufSize from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|ConnectionPingTimeout", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 30000) << "extracting ConnectionPingTimeout from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|TcpNodelay", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 1) << "extracting TcpNodelay from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_1|Host", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "0.0.0.1" ) << "extracting Host from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_1|Port", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "19001" ) << "extracting Port from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_1|CompressionThreshold", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 2048) << "extracting CompressionThreshold from EmaConfig.xml";

	// Checks all values from Channel_2
	debugResult = config.get<RsslConnectionTypes>( "ChannelGroup|ChannelList|Channel.Channel_2|ChannelType", channelType );
	EXPECT_TRUE( debugResult && channelType == RSSL_CONN_TYPE_ENCRYPTED) << "extracting ChannelType from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|InterfaceName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "localhost4file" ) << "extracting InterfaceName from EmaConfig.xml";
	debugResult = config.get<RsslCompTypes>( "ChannelGroup|ChannelList|Channel.Channel_2|CompressionType", compType );
	EXPECT_TRUE( debugResult && compType == RSSL_COMP_ZLIB) << "extracting CompressionType from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|GuaranteedOutputBuffers", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 6000) << "extracting GuaranteedOutputBuffers from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|NumInputBuffers", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 9000) << "extracting NumInputBuffers from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|ConnectionPingTimeout", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 55555) << "extracting ConnectionPingTimeout from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|Host", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "0.0.0.2" ) << "extracting Host from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|Port", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "15008" ) << "extracting Port from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|TcpNodelay", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 0) << "extracting TcpNodelay from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|ObjectName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "HttpObjectName" ) << "extracting ObjectName from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|CompressionThreshold", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 4096) << "extracting CompressionThreshold from EmaConfig.xml";
	debugResult = config.get<EmaString>("ChannelGroup|ChannelList|Channel.Channel_2|Location", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "eu-west") << "extracting Location from EmaConfig.xml";
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_2|EnableSessionManagement", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting EnableSessionManagement from EmaConfig.xml";
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_2|ServiceDiscoveryRetryCount", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 4) << "extracting ServiceDiscoveryRetryCount from EmaConfig.xml";
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_2|InitializationTimeout", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 55) << "extracting InitializationTimeout from EmaConfig.xml";

	// Checks proxy settings
	debugResult = config.get<EmaString>("ChannelGroup|ChannelList|Channel.Channel_4|ProxyHost", retrievedValue);
	EXPECT_TRUE( debugResult && retrievedValue == "proxylocalhost" ) << "extracting ProxyHost from EmaConfig.xml";
	debugResult = config.get<EmaString>("ChannelGroup|ChannelList|Channel.Channel_4|ProxyPort", retrievedValue);
	EXPECT_TRUE( debugResult && retrievedValue == "9018" ) << "extracting ProxyPort from EmaConfig.xml";
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_4|ProxyConnectionTimeout", uintValue);
	EXPECT_TRUE( debugResult && uintValue == 17 ) << "extracting ProxyConnectionTimeout from EmaConfig.xml";

	// Checks ChannelType == RSSL_CONN_TYPE_WEBSOCKET values from Channel_11
	channelType = RSSL_CONN_TYPE_INIT;
	debugResult = config.get<RsslConnectionTypes>("ChannelGroup|ChannelList|Channel.Channel_11|ChannelType", channelType);
	EXPECT_TRUE(debugResult && channelType == RSSL_CONN_TYPE_WEBSOCKET) << "extracting ChannelType from EmaConfig.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_11|WsMaxMsgSize", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 100500) << "extracting WsMaxMsgSize from EmaConfig.xml";
	retrievedValue.clear();
	debugResult = config.get<EmaString>("ChannelGroup|ChannelList|Channel.Channel_11|WsProtocols", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "rssl.json.v2, rssl.rwf, tr_json2") << "extracting WsProtocols from EmaConfig.xml";

	// Checks EncryptedProtocolType == RSSL_CONN_TYPE_WEBSOCKET values from Channel_12
	channelType = RSSL_CONN_TYPE_INIT;
	debugResult = config.get<RsslConnectionTypes>("ChannelGroup|ChannelList|Channel.Channel_12|ChannelType", channelType);
	EXPECT_TRUE(debugResult && channelType == RSSL_CONN_TYPE_ENCRYPTED) << "extracting ChannelType from EmaConfig.xml";
	channelType = RSSL_CONN_TYPE_INIT;
	debugResult = config.get<RsslConnectionTypes>("ChannelGroup|ChannelList|Channel.Channel_12|EncryptedProtocolType", channelType);
	EXPECT_TRUE(debugResult && channelType == RSSL_CONN_TYPE_WEBSOCKET) << "extracting EncryptedProtocolType from EmaConfig.xml";

	// Checks ServerType == RSSL_CONN_TYPE_ENCRYPTED values from Server_13
	serverType = RSSL_CONN_TYPE_INIT;
	debugResult = config.get<RsslConnectionTypes>("ServerGroup|ServerList|Server.Server_13|ServerType", serverType);
	EXPECT_TRUE(debugResult && serverType == RSSL_CONN_TYPE_ENCRYPTED) << "extracting ServerType from EmaConfig.xml";
	// Checks Server:SecurityProtocol values from Server_13
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_13|SecurityProtocol", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 8) << "extracting Server_13|SecurityProtocol from EmaConfig.xml";

	// Checks all values from Channel:DirectWrite
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_1|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_1|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_2|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_2|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_3|DirectWrite", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting Channel_3|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_4|DirectWrite", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting Channel_4|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_5|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_5|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_10|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_10|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_11|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_11|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ChannelGroup|ChannelList|Channel.Channel_12|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Channel_12|DirectWrite from EmaConfigTest.xml";

	// Checks ServerType == RSSL_CONN_TYPE_WEBSOCKET values from Server_11
	serverType = RSSL_CONN_TYPE_INIT;
	debugResult = config.get<RsslConnectionTypes>("ServerGroup|ServerList|Server.Server_11|ServerType", serverType);
	EXPECT_TRUE(debugResult && serverType == RSSL_CONN_TYPE_WEBSOCKET) << "extracting ServerType from EmaConfig.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_11|MaxFragmentSize", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 100500) << "extracting MaxFragmentSize from EmaConfig.xml";
	retrievedValue.clear();
	debugResult = config.get<EmaString>("ServerGroup|ServerList|Server.Server_11|WsProtocols", retrievedValue);
	EXPECT_TRUE(debugResult && retrievedValue == "rssl.json.v2, rssl.rwf, tr_json2") << "extracting WsProtocols from EmaConfig.xml";

	// Checks all values from Logger_1
	debugResult = config.get<EmaString>( "LoggerGroup|LoggerList|Logger|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Logger_1" ) << "extracting the first logger name from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::LoggerType>( "LoggerGroup|LoggerList|Logger.Logger_1|LoggerType", loggerType );
	EXPECT_TRUE( debugResult && loggerType == OmmLoggerClient::FileEnum) << "extracting LoggerType from EmaConfig.xml";
	debugResult = config.get<EmaString>( "LoggerGroup|LoggerList|Logger.Logger_1|FileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "emaLog" ) << "extracting FileName from EmaConfig.xml";
	debugResult = config.get<UInt64>("LoggerGroup|LoggerList|Logger.Logger_1|NumberOfLogFiles", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 42) << "extracting NumberOfLogFiles from EmaConfig.xml";
	debugResult = config.get<UInt64>("LoggerGroup|LoggerList|Logger.Logger_1|MaxLogFileSize", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 84000) << "extracting MaxLogFileSize from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::Severity>( "LoggerGroup|LoggerList|Logger.Logger_1|LoggerSeverity", loggerSeverity );
	EXPECT_TRUE( debugResult && loggerSeverity == OmmLoggerClient::VerboseEnum) << "extracting LoggerSeverity from EmaConfig.xml";

	// Checks all values from Logger_2
	debugResult = config.get<OmmLoggerClient::LoggerType>( "LoggerGroup|LoggerList|Logger.Logger_2|LoggerType", loggerType );
	EXPECT_TRUE( debugResult && loggerType == OmmLoggerClient::StdoutEnum) << "extracting LoggerType from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::Severity>( "LoggerGroup|LoggerList|Logger.Logger_2|LoggerSeverity", loggerSeverity );
	EXPECT_TRUE( debugResult && loggerSeverity == OmmLoggerClient::ErrorEnum) << "extracting LoggerSeverity from EmaConfig.xml";

	// Checks all values from Logger_3
	debugResult = config.get<OmmLoggerClient::LoggerType>("LoggerGroup|LoggerList|Logger.Logger_3|LoggerType", loggerType);
	EXPECT_TRUE(debugResult && loggerType == OmmLoggerClient::StderrEnum) << "extracting LoggerType::StderrEnum from EmaConfig.xml";

	// Checks all values from Dictionary_1
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Dictionary_1" ) << "extracting the first dictionary name from EmaConfig.xml";
	debugResult = config.get<Dictionary::DictionaryType>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_1|DictionaryType", dictionaryType );
	EXPECT_TRUE( debugResult && dictionaryType == Dictionary::ChannelDictionaryEnum) << "extracting DictionaryType from EmaConfig.xml";

	// Checks all values from Dictionary_2
	debugResult = config.get<Dictionary::DictionaryType>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|DictionaryType", dictionaryType );
	EXPECT_TRUE( debugResult && dictionaryType == Dictionary::FileDictionaryEnum) << "extracting DictionaryType from EmaConfig.xml";
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|RdmFieldDictionaryFileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == fieldDictionaryFileNameTest ) << "extracting RDMFieldDictionary from EmaConfig.xml";
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|EnumTypeDefFileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == enumTableFileNameTest ) << "extracting enumtype.def from EmaConfig.xml";

	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_15|Dictionary", retrievedValue );
	EXPECT_TRUE( ! debugResult ) << "correctly detecting missing name item in a list";

	// verify item with multiple occurrences
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Check_Multiple_Occurrences|Logger", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Logger_3" ) << "extracting last value for item with multiple values";

	EmaVector< EmaString > v;
	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Check_Multiple_Occurrences|Logger", v );
	EXPECT_TRUE( debugResult && v.size() == 3 && v[0] == "Logger_1" && v[1] == "Logger_2" && v[2] == "Logger_3" ) << "extracting all values for item with multiple values";

	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Check_Multiple_Occurrences|Dictionary", v );
	EXPECT_TRUE( debugResult && v.size() == 1 && v[0] == "Dictionary_1") << "extracting values into vector for item with single value";

	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Check_Multiple_Occurrences|Channel", v );
	EXPECT_TRUE( debugResult == false) << "correctly ignored Channels when Channel_Set was last";

	debugResult = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Check_Multiple_Occurrences|ChannelSet", v );
	EXPECT_TRUE( debugResult && v.size() == 1 && v[0] == "CS2" ) << "correctly extracted only last ChannelSet value (into vector) when both Channel and Channel_Sets configured";

	// Checks all values from Server:ServerSharedSocket
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_1|ServerSharedSocket", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting Server_1|ServerSharedSocket from EmaConfig.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_2|ServerSharedSocket", uintValue);
	EXPECT_TRUE(debugResult == false && uintValue == 0) << "extracting Server_2|ServerSharedSocket from EmaConfig.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_11|ServerSharedSocket", uintValue);
	EXPECT_TRUE(debugResult == false && uintValue == 0) << "extracting Server_11|ServerSharedSocket from EmaConfig.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_12|ServerSharedSocket", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting Server_12|ServerSharedSocket from EmaConfig.xml";

	// Checks all values from Server:DirectWrite
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_1|DirectWrite", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting Server_1|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_2|DirectWrite", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 0) << "extracting Server_2|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_11|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Server_11|DirectWrite from EmaConfigTest.xml";
	uintValue = 0;
	debugResult = config.get<UInt64>("ServerGroup|ServerList|Server.Server_12|DirectWrite", uintValue);
	EXPECT_FALSE(debugResult && uintValue == 0) << "extracting Server_12|DirectWrite from EmaConfigTest.xml";

	/* Check loadFilter in the Directory_1 */
	uintValue = 0;
	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_1|Service.TEST_NI_PUB|LoadFilter|OpenLimit", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 1) << "extracting  LoadFilter|OpenLimit from EmaConfig.xml";
	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_1|Service.TEST_NI_PUB|LoadFilter|OpenWindow", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 2) << "extracting  LoadFilter|OpenWindow from EmaConfig.xml";
	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_1|Service.TEST_NI_PUB|LoadFilter|LoadFactor", uintValue);
	EXPECT_TRUE(debugResult == true &&  uintValue == 3) << "extracting  LoadFilter|LoadFactor from EmaConfig.xml";

	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_2|Service.DIRECT_FEED|LoadFilter|OpenLimit", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 4294967295) << "extracting  LoadFilter|OpenLimit from EmaConfig.xml";
	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_2|Service.DIRECT_FEED|LoadFilter|OpenWindow", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 4294967295) << "extracting  LoadFilter|OpenWindow from EmaConfig.xml";
	debugResult = config.get<UInt64>("DirectoryGroup|DirectoryList|Directory.Directory_2|Service.DIRECT_FEED|LoadFilter|LoadFactor", uintValue);
	EXPECT_TRUE(debugResult == true && uintValue == 65535) << "extracting  LoadFilter|LoadFactor from EmaConfig.xml";

	config.configErrors().printErrors(OmmLoggerClient::WarningEnum);
}

// http connection supported on windows only
#ifdef WIN32
TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfigHttp)
{
	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addAscii("RestLogFileName", "Rest.log")
			.addUInt("RestEnableLog", 1)
			.addUInt("RestVerboseMode", 1)
			.addUInt("SendJsonConvError", 1)
			.addUInt("ItemCountHint", 5000)
			.addUInt("ServiceCountHint", 2000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 1200)
			.addUInt("RequestTimeout", 2400)
			.addUInt("MaxOutstandingPosts", 9999)
			.addInt("DispatchTimeoutApiThread", 60)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 300)
			.addInt("MaxEventsInPool", 100)
			.addInt("ReconnectAttemptLimit", 1)
			.addInt("ReconnectMinDelay", 500)
			.addInt("ReconnectMaxDelay", 500)
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTracePingOnly", 1)
			.addUInt("XmlTraceHex", 1)
			.addAscii("RestProxyHostName", "restProxySrv1")
			.addAscii("RestProxyPort", "39918")
			.addUInt("MaxDispatchCountUserThread", 700).complete()).complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_HTTP)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 1)
			.addUInt("GuaranteedOutputBuffers", 8000)
			.addUInt("NumInputBuffers", 7777)
			.addUInt("SysRecvBufSize", 150000)
			.addUInt("SysSendBufSize", 200000)
			.addUInt("ConnectionPingTimeout", 30000)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.addUInt("InitializationTimeout", 96)
			.addAscii("ObjectName", "MyHttpObject")
			.addUInt("MsgKeyInUpdates", 1).complete()).complete();

		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));
				
		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );
		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_1" ) >= 0 ? true : false;
		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_1" ) << "Connection name , \"Channel_1\"";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
		EXPECT_TRUE( activeConfig.restLogFileName == "Rest.log" ) << "restLogFileName , \"Rest.log\"";
		EXPECT_TRUE( activeConfig.restEnableLog == 1) << "restEnableLog , \"True\"";
		EXPECT_TRUE(activeConfig.restVerboseMode == 1) << "restVerboseMode , \"True\"";
		EXPECT_TRUE( activeConfig.sendJsonConvError == 1) << "sendJsonConvError , \"True\"";
		EXPECT_TRUE( activeConfig.shouldInitializeCPUIDlib == 1) << "shouldInitializeCPUIDlib , \"True\"";
		EXPECT_TRUE( activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
		EXPECT_TRUE( activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
		EXPECT_TRUE( activeConfig.obeyOpenWindow == 1) << "obeyOpenWindow , 1";
		EXPECT_TRUE( activeConfig.postAckTimeout == 1200) << "postAckTimeout , 1200";
		EXPECT_TRUE( activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
		EXPECT_TRUE( activeConfig.maxOutstandingPosts == 9999) << "maxOutstandingPosts , 9999";
		EXPECT_TRUE( activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
		EXPECT_TRUE( activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
		EXPECT_TRUE( activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
		EXPECT_TRUE( activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
		EXPECT_TRUE( activeConfig.maxEventsInPool == 100) << "maxEventsInPool , 100";
		EXPECT_TRUE( activeConfig.reconnectAttemptLimit == 1) << "reconnectAttemptLimit , 1";
		EXPECT_TRUE( activeConfig.reconnectMinDelay == 500) << "reconnectMinDelay , 500";
		EXPECT_TRUE( activeConfig.reconnectMaxDelay == 500) << "reconnectMaxDelay , 500";
		EXPECT_TRUE( activeConfig.xmlTraceFileName == "MyXMLTrace" ) << "xmlTraceFileName == \"MyXMLTrace\"";
		EXPECT_TRUE( activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
		EXPECT_TRUE( activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
		EXPECT_TRUE( activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
		EXPECT_TRUE( activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
		EXPECT_TRUE( activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
		EXPECT_TRUE( activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
		EXPECT_TRUE( activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
		EXPECT_TRUE( activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
		EXPECT_TRUE( activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE( activeConfig.restProxyHostName == "restProxySrv1" ) << "restProxyHostName == \"restProxySrv1\"";
		EXPECT_TRUE( activeConfig.restProxyPort == "39918" ) << "restProxyPort == \"39918\"";
		EXPECT_TRUE( activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_HTTP) << "connectionType , ChannelType::RSSL_CONN_TYPE_HTTP";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->hostName == "localhost" ) << "EncryptedChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->serviceName == "14002" ) << "EncryptedChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->objectName == "MyHttpObject" ) << "EncryptedChannelConfig::ObjectName , \"MyHttpObject\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->tcpNodelay == 0) << "SocketChannelConfig::tcpNodelay , 0";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->initializationTimeout == 96) << "SocketChannelConfig::initializationTimeout , 96";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerFileName == "logFile" ) << "loggerFileName = \"logFile\"";
		EXPECT_TRUE( activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE( activeConfig.loggerConfig.maxFileSize == 84000 ) << "maxFileSize = 84000";
		EXPECT_TRUE( activeConfig.loggerConfig.maxFileNumber == 42 ) << "maxFileNumber = 42";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE( activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == fieldDictionaryFileNameTest ) << "rdmfieldDictionaryFileName , " << fieldDictionaryFileNameTest;
		EXPECT_TRUE( activeConfig.dictionaryConfig.enumtypeDefFileName == enumTableFileNameTest ) << "enumtypeDefFileName , " << enumTableFileNameTest;
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfigHttp()";
	}
}
#endif

TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfigWS)
{
	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addAscii("RestLogFileName", "Rest.log")
			.addUInt("RestEnableLog", 1)
			.addUInt("RestVerboseMode", 1)
			.addUInt("SendJsonConvError", 1)
			.addUInt("ShouldInitializeCPUIDlib", 1)
			.addUInt("ItemCountHint", 5000)
			.addUInt("ServiceCountHint", 2000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 1200)
			.addUInt("RequestTimeout", 2400)
			.addUInt("MaxOutstandingPosts", 9999)
			.addInt("DispatchTimeoutApiThread", 60)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 300)
			.addInt("ReconnectAttemptLimit", 1)
			.addInt("ReconnectMinDelay", 500)
			.addInt("ReconnectMaxDelay", 500)
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTracePingOnly", 1)
			.addUInt("XmlTraceHex", 1)
			.addUInt("XmlTraceDump", 1)
			.addUInt("CatchUnknownJsonFids", 0)
			.addUInt("CatchUnknownJsonKeys", 1)
			.addUInt("CloseChannelFromConverterFailure", 0)
			.addUInt("DefaultServiceID", 1)
			.addUInt("JsonExpandedEnumFields", 1)
			.addUInt("OutputBufferSize", 4294967296)
			.addUInt("JsonTokenIncrementSize", 4294967296)
			.addUInt("MaxDispatchCountUserThread", 700)
			.addUInt("SendJsonConvError", 1).complete()).complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_WEBSOCKET)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 1)
			.addUInt("GuaranteedOutputBuffers", 8000)
			.addUInt("NumInputBuffers", 7777)
			.addUInt("SysRecvBufSize", 150000)
			.addUInt("SysSendBufSize", 200000)
			.addUInt("ConnectionPingTimeout", 30000)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.addUInt("InitializationTimeout", 96)
			.addAscii("ObjectName", "MyHttpObject")
			.addUInt("WsMaxMsgSize", 100500)
			.addAscii("WsProtocols", "rssl.json.v2, rssl.rwf, tr_json2")
			.addUInt("MsgKeyInUpdates", 1).complete()).complete();

		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap), true);
		//OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1") << "Connection name , \"Channel_1\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1") << "dictionaryName , \"Dictionary_1\"";
		EXPECT_TRUE(activeConfig.restLogFileName == "Rest.log") << "restLogFileName , \"Rest.log\"";
		EXPECT_TRUE(activeConfig.restEnableLog == 1) << "restEnableLog , \"True\"";
		EXPECT_TRUE(activeConfig.restVerboseMode == 1) << "restVerboseMode , \"True\"";
		EXPECT_TRUE(activeConfig.sendJsonConvError == 1) << "sendJsonConvError , \"True\"";
		EXPECT_TRUE(activeConfig.shouldInitializeCPUIDlib == 1) << "shouldInitializeCPUIDlib , \"True\"";
		EXPECT_TRUE(activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
		EXPECT_TRUE(activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
		EXPECT_TRUE(activeConfig.obeyOpenWindow == 1) << "obeyOpenWindow , 1";
		EXPECT_TRUE(activeConfig.postAckTimeout == 1200) << "postAckTimeout , 1200";
		EXPECT_TRUE(activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
		EXPECT_TRUE(activeConfig.maxOutstandingPosts == 9999) << "maxOutstandingPosts , 9999";
		EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
		EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
		EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
		EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
		EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 1) << "reconnectAttemptLimit , 1";
		EXPECT_TRUE(activeConfig.reconnectMinDelay == 500) << "reconnectMinDelay , 500";
		EXPECT_TRUE(activeConfig.reconnectMaxDelay == 500) << "reconnectMaxDelay , 500";
		EXPECT_TRUE(activeConfig.xmlTraceFileName == "MyXMLTrace") << "xmlTraceFileName == \"MyXMLTrace\"";
		EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
		EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
		EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
		EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
		EXPECT_TRUE(activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
		EXPECT_TRUE(activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
		EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
		EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
		EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE(activeConfig.xmlTraceDump == 1) << "xmlTraceDump , 1";
		EXPECT_TRUE(activeConfig.catchUnknownJsonFids == 0) << "catchUnknownJsonFids , 0";
		EXPECT_TRUE(activeConfig.catchUnknownJsonKeys == 1) << "catchUnknownJsonKeys , 1";
		EXPECT_TRUE(activeConfig.closeChannelFromFailure == 0) << "closeChannelFromConverterFailure , 0";
		EXPECT_TRUE(activeConfig.defaultServiceIDForConverter == 1) << "defaultServiceID , 1";
		EXPECT_TRUE(activeConfig.jsonExpandedEnumFields == 1) << "jsonExpandedEnumFields , 1";
		EXPECT_TRUE(activeConfig.outputBufferSize == 4294967295) << "outputBufferSize , 4294967295"; // Use the max UINT32 instead
		EXPECT_TRUE(activeConfig.jsonTokenIncrementSize == 4294967295) << "jsonTokenIncrementSize , 4294967295"; // Use the max UINT32 instead
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost") << "interfaceName , \"localhost\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_WEBSOCKET) << "connectionType , ChannelType::RSSL_CONN_TYPE_WEBSOCKET";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->hostName == "localhost") << "EncryptedChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->serviceName == "14002") << "EncryptedChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->objectName == "MyHttpObject") << "EncryptedChannelConfig::ObjectName , \"MyHttpObject\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->tcpNodelay == 0) << "SocketChannelConfig::tcpNodelay , 0";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->initializationTimeout == 96) << "SocketChannelConfig::initializationTimeout , 96";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->wsMaxMsgSize == 100500) << "SocketChannelConfig::wsMaxMsgSize , 100500";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->wsProtocols == "rssl.json.v2, rssl.rwf, tr_json2") << "SocketChannelConfig::wsProtocols , \"rssl.json.v2, rssl.rwf, tr_json2\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile") << "loggerFileName = \"logFile\"";
		EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == "./RDMFieldDictionary") << "rdmfieldDictionaryFileName , \"./RDMFieldDictionary\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == "./enumtype.def") << "enumtypeDefFileName , \"./enumtype.def\"";
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfigHttp()";
	}
}

TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfigWSEncrypted)
{
	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addAscii("RestLogFileName", "Rest.log")
			.addUInt("RestEnableLog", 1)
			.addUInt("RestVerboseMode", 1)
			.addUInt("SendJsonConvError", 1)
			.addUInt("ItemCountHint", 5000)
			.addUInt("ServiceCountHint", 2000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 1200)
			.addUInt("RequestTimeout", 2400)
			.addUInt("MaxOutstandingPosts", 9999)
			.addInt("DispatchTimeoutApiThread", 60)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 300)
			.addInt("ReconnectAttemptLimit", 1)
			.addInt("ReconnectMinDelay", 500)
			.addInt("ReconnectMaxDelay", 500)
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTracePingOnly", 1)
			.addUInt("XmlTraceHex", 1)
			.addUInt("XmlTraceDump", 1)
			.addUInt("CatchUnknownJsonFids", 0)
			.addUInt("CatchUnknownJsonKeys", 1)
			.addUInt("CloseChannelFromConverterFailure", 0)
			.addUInt("DefaultServiceID", 1)
			.addUInt("JsonExpandedEnumFields", 1)
			.addUInt("OutputBufferSize", 4294967296)
			.addUInt("JsonTokenIncrementSize", 4294967296)
			.addUInt("MaxDispatchCountUserThread", 700).complete()).complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_ENCRYPTED)
			.addEnum("EncryptedProtocolType", RSSL_CONN_TYPE_WEBSOCKET)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 1)
			.addUInt("GuaranteedOutputBuffers", 8000)
			.addUInt("NumInputBuffers", 7777)
			.addUInt("SysRecvBufSize", 150000)
			.addUInt("SysSendBufSize", 200000)
			.addUInt("ConnectionPingTimeout", 30000)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.addUInt("InitializationTimeout", 96)
			.addAscii("ObjectName", "MyHttpObject")
			.addUInt("WsMaxMsgSize", 100500)
			.addAscii("WsProtocols", "rssl.json.v2, rssl.rwf, tr_json2")
			.addUInt("MsgKeyInUpdates", 1).complete()).complete();

		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap), true);
		//OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1") << "Connection name , \"Channel_1\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1") << "dictionaryName , \"Dictionary_1\"";
		EXPECT_TRUE(activeConfig.restLogFileName == "Rest.log") << "restLogFileName , \"Rest.log\"";
		EXPECT_TRUE(activeConfig.restEnableLog == 1) << "restEnableLog , \"True\"";
		EXPECT_TRUE(activeConfig.restVerboseMode == 1) << "restVerboseMode , \"True\"";
		EXPECT_TRUE(activeConfig.sendJsonConvError == 1) << "sendJsonConvError , \"True\"";
		EXPECT_TRUE(activeConfig.shouldInitializeCPUIDlib == 1) << "shouldInitializeCPUIDlib , \"True\"";
		EXPECT_TRUE(activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
		EXPECT_TRUE(activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
		EXPECT_TRUE(activeConfig.obeyOpenWindow == 1) << "obeyOpenWindow , 1";
		EXPECT_TRUE(activeConfig.postAckTimeout == 1200) << "postAckTimeout , 1200";
		EXPECT_TRUE(activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
		EXPECT_TRUE(activeConfig.maxOutstandingPosts == 9999) << "maxOutstandingPosts , 9999";
		EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
		EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
		EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
		EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
		EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 1) << "reconnectAttemptLimit , 1";
		EXPECT_TRUE(activeConfig.reconnectMinDelay == 500) << "reconnectMinDelay , 500";
		EXPECT_TRUE(activeConfig.reconnectMaxDelay == 500) << "reconnectMaxDelay , 500";
		EXPECT_TRUE(activeConfig.xmlTraceFileName == "MyXMLTrace") << "xmlTraceFileName == \"MyXMLTrace\"";
		EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
		EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
		EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
		EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
		EXPECT_TRUE(activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
		EXPECT_TRUE(activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
		EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
		EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePing , 1";
		EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE(activeConfig.xmlTraceDump == 1) << "xmlTraceDump , 1";
		EXPECT_TRUE(activeConfig.catchUnknownJsonFids == 0) << "catchUnknownJsonFids , 0";
		EXPECT_TRUE(activeConfig.catchUnknownJsonKeys == 1) << "catchUnknownJsonKeys , 1";
		EXPECT_TRUE(activeConfig.closeChannelFromFailure == 0) << "closeChannelFromConverterFailure , 0";
		EXPECT_TRUE(activeConfig.defaultServiceIDForConverter == 1) << "defaultServiceID , 1";
		EXPECT_TRUE(activeConfig.jsonExpandedEnumFields == 1) << "jsonExpandedEnumFields , 1";
		EXPECT_TRUE(activeConfig.outputBufferSize == 4294967295) << "outputBufferSize , 4294967295"; // Use the max UINT32 instead
		EXPECT_TRUE(activeConfig.jsonTokenIncrementSize == 4294967295) << "jsonTokenIncrementSize , 4294967295"; // Use the max UINT32 instead
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost") << "interfaceName , \"localhost\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_ENCRYPTED) << "connectionType , ChannelType::RSSL_CONN_TYPE_ENCRYPTED";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET) << "encryptedConnectionType , EncryptedProtocolType::RSSL_WEBSOCKET";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->hostName == "localhost") << "EncryptedChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->serviceName == "14002") << "EncryptedChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->objectName == "MyHttpObject") << "EncryptedChannelConfig::ObjectName , \"MyHttpObject\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->tcpNodelay == 0) << "SocketChannelConfig::tcpNodelay , 0";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->initializationTimeout == 96) << "SocketChannelConfig::initializationTimeout , 96";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->wsMaxMsgSize == 100500) << "SocketChannelConfig::wsMaxMsgSize , 100500";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->wsProtocols == "rssl.json.v2, rssl.rwf, tr_json2") << "SocketChannelConfig::wsProtocols , \"rssl.json.v2, rssl.rwf, tr_json2\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile") << "loggerFileName = \"logFile\"";
		EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == "./RDMFieldDictionary") << "rdmfieldDictionaryFileName , \"./RDMFieldDictionary\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == "./enumtype.def") << "enumtypeDefFileName , \"./enumtype.def\"";
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfigHttp()";
	}
}

TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfig)
{
	Map outermostMap, innerMap;
	ElementList elementList;
	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addAscii("RestLogFileName", "Rest.log")
			.addUInt("RestEnableLog", 1)
			.addUInt("RestVerboseMode", 1)
			.addUInt("SendJsonConvError", 1)
			.addUInt("ItemCountHint", 5000)
			.addUInt("ServiceCountHint", 2000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 1200)
			.addUInt("RequestTimeout", 2400)
			.addUInt("MaxOutstandingPosts", 9999)
			.addInt("DispatchTimeoutApiThread", 60)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 300)
			.addUInt("MaxDispatchCountUserThread", 700)
			.addInt("MaxEventsInPool", 300)
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTracePingOnly", 1)
			.addUInt("XmlTraceHex", 1)
			.addAscii("RestProxyHostName", "restProxySrv11")
			.addAscii("RestProxyPort", "39919")
			.addUInt("MsgKeyInUpdates", 1)
			.addInt("ReconnectAttemptLimit", 10)
			.addInt("ReconnectMinDelay", 4444)
			.addInt("ReconnectMaxDelay", 7777)
			.addInt("PipePort", 13650)
			.addDouble("TokenReissueRatio", 0.55).complete())
			.addKeyAscii("Consumer_2", MapEntry::AddEnum, ElementList()
				.addAscii("Channel", "Channel_2")
				.addAscii("Dictionary", "Dictionary_1")
				.addAscii("Logger", "Logger_1").complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 0)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 1)
			.addUInt("GuaranteedOutputBuffers", 8000)
			.addUInt("NumInputBuffers", 7777)
			.addUInt("SysRecvBufSize", 150000)
			.addUInt("SysSendBufSize", 200000)
			.addUInt("CompressionThreshold", 12856)
			.addUInt("ConnectionPingTimeout", 30000)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.addUInt("InitializationTimeout", 56)
			.addUInt("DirectWrite", 0)
			.complete())
			.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 2)
				.addAscii("Host", "localhost")
				.addAscii("Port", "14002")
				.addAscii("ObjectName", "MyHttpObject").complete())
			.complete();

		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1" ) << "Connection name , \"Channel_1\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
		EXPECT_TRUE(activeConfig.restLogFileName == "Rest.log") << "restLogFileName , \"Rest.log\"";
		EXPECT_TRUE(activeConfig.restEnableLog == 1) << "restEnableLog , \"True\"";
		EXPECT_TRUE(activeConfig.restVerboseMode == 1) << "restVerboseMode , \"True\"";
		EXPECT_TRUE(activeConfig.sendJsonConvError == 1) << "sendJsonConvError , \"True\"";
		EXPECT_TRUE(activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
		EXPECT_TRUE(activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
		EXPECT_TRUE(activeConfig.obeyOpenWindow == 1) << "obeyOpenWindow , 1";
		EXPECT_TRUE(activeConfig.postAckTimeout == 1200) << "postAckTimeout , 1200";
		EXPECT_TRUE(activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
		EXPECT_TRUE(activeConfig.maxOutstandingPosts == 9999) << "maxOutstandingPosts , 9999";
		EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
		EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
		EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
		EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
		EXPECT_TRUE(activeConfig.maxEventsInPool == 300) << "maxEventsInPool , 300";
		EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 10) << "reconnectAttemptLimit , 10";
		EXPECT_TRUE(activeConfig.reconnectMinDelay == 4444) << "reconnectMinDelay , 4444";
		EXPECT_TRUE(activeConfig.reconnectMaxDelay == 7777) << "reconnectMaxDelay , 7777";
		EXPECT_TRUE(activeConfig.xmlTraceFileName == "MyXMLTrace" ) << "xmlTraceFileName , \"MyXMLTrace\"";
		EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
		EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
		EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
		EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
		EXPECT_TRUE(activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
		EXPECT_TRUE(activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
		EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
		EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
		EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE(activeConfig.restProxyHostName == "restProxySrv11") << "restProxyHostName == \"restProxySrv11\"";
		EXPECT_TRUE(activeConfig.restProxyPort == "39919") << "restProxyPort == \"39919\"";
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE(activeConfig.pipePort == 13650) << "pipePort , 13650";
		EXPECT_TRUE(activeConfig.tokenReissueRatio == 0.55) << "tokenReissueRatio , 0.55";
		EXPECT_TRUE(activeConfig.reissueTokenAttemptLimit == -1) << "reissueTokenAttemptLimit , -1";
		EXPECT_TRUE(activeConfig.reissueTokenAttemptInterval == 5000) << "reissueTokenAttemptInterval , 5000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionThreshold == 12856) << "CompressionThreshold , 12856";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->directWrite == 0) << "directWrite , 0";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->hostName == "localhost" ) << "SocketChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->serviceName == "14002" ) << "SocketChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->tcpNodelay == 0) << "SocketChannelConfig::tcpNodelay , 0";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->initializationTimeout == 56) << "SocketChannelConfig::initializationTimeout , 56";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile" ) << "loggerFileName = \"logFile\"";
		EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == fieldDictionaryFileNameTest ) << "rdmfieldDictionaryFileName , " << fieldDictionaryFileNameTest;
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == enumTableFileNameTest ) << "enumtypeDefFileName , " << enumTableFileNameTest;
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfig()";
	}
}

TEST_F(EmaConfigTest, testLoadingConfigurationFromProgrammaticConfigForSessionManagement)
{
	Map outermostMap, innerMap;
	ElementList elementList;
	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addDouble("TokenReissueRatio", 0.20)
			.addInt("ReissueTokenAttemptLimit", 10)
			.addInt("ReissueTokenAttemptInterval", 10000).complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 1)
			.addAscii("Location", "eu-west")
			.addUInt("EnableSessionManagement", 1)
			.addUInt("ServiceDiscoveryRetryCount", 5)
			.addEnum("EncryptedProtocolType", 0)
			.complete())
			.complete();

		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap)
			.username(g_userName).password(g_password).clientId(g_userName)
			.tunnelingProxyHostName(g_proxyHost).tunnelingProxyPort(g_proxyPort));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.tokenReissueRatio == 0.20) << "tokenReissueRatio, 0.20";
		EXPECT_TRUE(activeConfig.reissueTokenAttemptLimit == 10) << "reissueTokenAttemptLimit, 10";
		EXPECT_TRUE(activeConfig.reissueTokenAttemptInterval == 10000) << "reissueTokenAttemptInterval, 10000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1") << "Connection name , \"Channel_1\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1") << "dictionaryName , \"Dictionary_1\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->location == "eu-west") << "EncryptedChannelConfig::location , \"eu-west\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->enableSessionMgnt == 1) << "EncryptedChannelConfig::enableSessionMgnt , 1";
		EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->serviceDiscoveryRetryCount == 5) << "EncryptedChannelConfig::serviceDiscoveryRetryCount , 5";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == fieldDictionaryFileNameTest) << "rdmfieldDictionaryFileName , " << fieldDictionaryFileNameTest;
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == enumTableFileNameTest) << "enumtypeDefFileName , " << enumTableFileNameTest;
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfig()";
	}
}

TEST_F(EmaConfigTest, testLoadingProgrammaticConfigForWarmStandby)
{
	Map outermostMap, innerMap;
	ElementList elementList;
	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_8");

		innerMap.addKeyAscii("Consumer_8", MapEntry::AddEnum, ElementList()
			.addAscii("WarmStandbyChannelSet", "WarmStandbyChannel_1, WarmStandbyChannel_2")
			.addAscii("Logger", "Logger_1")
			.addUInt("XmlTraceToStdout", 0)
			.addAscii("Dictionary", "Dictionary_2").complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 0)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002").complete());

		innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 0)
			.addAscii("Host", "localhost")
			.addAscii("Port", "15008").complete());

		innerMap.addKeyAscii("Channel_3", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 0)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14008").complete());

		innerMap.complete();

		elementList.clear();
		elementList.addMap("ChannelList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Server_Info_1", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("PerServiceNameSet", "Service_A, Service_B").complete());

		innerMap.addKeyAscii("Server_Info_2", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_2")
			.addAscii("PerServiceNameSet", "Service_C, Service_D").complete());

		innerMap.addKeyAscii("Server_Info_3", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_3")
			.addAscii("PerServiceNameSet", "Service_E, Service_F").complete());

		innerMap.complete();
			
		elementList.addMap("WarmStandbyServerInfoList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("WarmStandbyServerInfoGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("WarmStandbyChannel_1", MapEntry::AddEnum, ElementList()
			.addAscii("StartingActiveServer", "Server_Info_1")
			.addAscii("StandbyServerSet", "Server_Info_2, Server_Info_3")
			.addEnum("WarmStandbyMode", 1)
			.complete());

		innerMap.addKeyAscii("WarmStandbyChannel_2", MapEntry::AddEnum, ElementList()
			.addAscii("StartingActiveServer", "Server_Info_2")
			.addAscii("StandbyServerSet", "Server_Info_1, Server_Info_3")
			.addEnum("WarmStandbyMode", 2)
			.complete());

		innerMap.complete();

		elementList.addMap("WarmStandbyList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("WarmStandbyGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Dictionary_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_8") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_8_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet.size() == 0) << "Connection list size , \"0\"";
		EXPECT_TRUE(activeConfig.configWarmStandbySet.size() == 2) << "Warm standby Channel size, 2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->name == "WarmStandbyChannel_1") << "Warm standby Channel name, WarmStandbyChannel_1";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->startingActiveServer->name == "Server_Info_1") << "StartingActiveServer name, Server_Info_1";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->startingActiveServer->channelConfig->name == "Channel_1") << "Channel config name, Channel_1";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->startingActiveServer->perServiceNameSet.size() == 2) << "Per service name list size, 2";	
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->startingActiveServer->perServiceNameSet[0] == "Service_A") << "The first per service name, Service_A";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->startingActiveServer->perServiceNameSet[1] == "Service_B") << "The second per service name, Service_B";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->warmStandbyMode == 1) << "Warm standby mode, 1";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->standbyServerSet.size() == 2) << "Standby server list size, 2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->standbyServerSet[0]->name == "Server_Info_2") << "First standby server name, Server_Info_2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->standbyServerSet[0]->channelConfig->name == "Channel_2") << "Channel config name, Channel_2";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->standbyServerSet[0]->perServiceNameSet[0] == "Service_C") << "The first per service name, Service_C";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->standbyServerSet[0]->perServiceNameSet[1] == "Service_D") << "The second per service name, Service_D";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->standbyServerSet[1]->name == "Server_Info_3") << "Second standby server name, Server_Info_3";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[0]->standbyServerSet[1]->channelConfig->name == "Channel_3") << "Channel config name, Channel_3";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->standbyServerSet[1]->perServiceNameSet[0] == "Service_E") << "The first per service name, Service_E";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[0]->standbyServerSet[1]->perServiceNameSet[1] == "Service_F") << "The second per service name, Service_F";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->name == "WarmStandbyChannel_2") << "Warm standby Channel name, WarmStandbyChannel_2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->startingActiveServer->name == "Server_Info_2") << "StartingActiveServer name, Server_Info_2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->startingActiveServer->perServiceNameSet.size() == 2) << "Per service name list size, 2";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->startingActiveServer->perServiceNameSet[0] == "Service_C") << "The first per service name, Service_C";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->startingActiveServer->perServiceNameSet[1] == "Service_D") << "The second per service name, Service_D";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->warmStandbyMode == 2) << "Warm standby mode, 2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->standbyServerSet.size() == 2) << "Standby server list size, 2";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->standbyServerSet[0]->name == "Server_Info_1") << "First standby server name, Server_Info_1";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->standbyServerSet[0]->channelConfig->name == "Channel_1") << "Channel config name, Channel_1";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->standbyServerSet[0]->perServiceNameSet[0] == "Service_A") << "The first per service name, Service_A";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->standbyServerSet[0]->perServiceNameSet[1] == "Service_B") << "The second per service name, Service_B";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->standbyServerSet[1]->name == "Server_Info_3") << "Second standby server name, Server_Info_3";
		EXPECT_TRUE(activeConfig.configWarmStandbySet[1]->standbyServerSet[1]->channelConfig->name == "Channel_3") << "Channel config name, Channel_3";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->standbyServerSet[1]->perServiceNameSet[0] == "Service_E") << "The first per service name, Service_E";
		EXPECT_TRUE(*activeConfig.configWarmStandbySet[1]->standbyServerSet[1]->perServiceNameSet[1] == "Service_F") << "The second per service name, Service_F";
		EXPECT_TRUE(activeConfig.configChannelSetForWSB.size() == 6) << "Connection list size for warm standby, 6";
		EXPECT_TRUE(activeConfig.configChannelSetForWSB[0]->name == "Channel_1") << "Channe name, Channel_1";
		EXPECT_TRUE(activeConfig.configChannelSetForWSB[1]->name == "Channel_2") << "Channe name, Channel_2";
		EXPECT_TRUE(activeConfig.configChannelSetForWSB[2]->name == "Channel_3") << "Channe name, Channel_3";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_2") << "dictionaryName , \"Dictionary_2\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == fieldDictionaryFileNameTest) << "rdmfieldDictionaryFileName , " << fieldDictionaryFileNameTest;
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == enumTableFileNameTest) << "enumtypeDefFileName , " << enumTableFileNameTest;
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingProgrammaticConfigForWarmStandby()";
	}
}

TEST_F(EmaConfigTest, testOverridingFromInterface)
{
	//two testcases:
	//case 1: override consumer config
	//case 2: override niprov config
	try
	{
		for (int testCase = 1; testCase <= 2; testCase++)
		{
			Map outermostMap, innerMap;
			ElementList elementList;

			std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

			if (testCase == 1)
			{

				elementList.addAscii("DefaultConsumer", "Consumer_1");

				innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum,
					ElementList()
					.addAscii("Channel", "Channel_1")
					.addAscii("Logger", "Logger_1")
					.addAscii("Dictionary", "Dictionary_1")
					.complete()).complete();

				elementList.addMap("ConsumerList", innerMap);

				elementList.complete();
				innerMap.clear();

				outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
				elementList.clear();
			}
			else if (testCase == 2)
			{
				elementList.addAscii("DefaultNiProvider", "Provider_5");
				innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
					.addAscii("Channel", "Channel_1")
					.addAscii("Logger", "Logger_1")
					.addAscii("Directory", "Directory_5")
					.addUInt("LoginRequestTimeOut", 50000).complete())
					.complete();

				elementList.addMap("NiProviderList", innerMap);
				elementList.complete();
				innerMap.clear();

				outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);
				elementList.clear();
			}

			innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", 0)
				.addAscii("Host", "10.0.0.1")
				.addAscii("Port", "8001")
				.complete()).complete();

			elementList.addMap("ChannelList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 0)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

			elementList.addMap("DictionaryList", innerMap);

			elementList.complete();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

			outermostMap.complete();

			EmaString localConfigPath;
			EmaString workingDir;
			ASSERT_EQ(getCurrentDir(workingDir), true)
				<< "Error: failed to load config file from current working dir "
				<< workingDir.c_str();
			localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			if (testCase == 1)
			{
				// Must load data dictionary files from current working location.
				OmmConsumerConfig ommConsumerConfig(localConfigPath);
				OmmConsumerImpl ommConsumerImpl(ommConsumerConfig.config(outermostMap).host("localhost:14002").channelType(EmaConfig::ConnectionTypeEnum::HTTP));

				const OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());

				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_HTTP) << "connectionType , ChannelType::RSSL_HTTP";
				EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->hostName == "localhost") << "SocketChannelConfig::hostname , \"localhost\"";
				EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->serviceName == "14002") << "SocketChannelConfig::serviceName , \"14002\"";
			}
			else if (testCase == 2)
			{
				OmmNiProviderConfig niprovConfig(localConfigPath);
				OmmNiProviderImpl ommNiProviderImpl(niprovConfig.config(outermostMap).host("localhost:14002").channelType(EmaConfig::ConnectionTypeEnum::HTTP), appClient);

				OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());

				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_HTTP) << "connectionType , ChannelType::RSSL_HTTP";
				EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->hostName == "localhost") << "SocketChannelConfig::hostname , \"localhost\"";
				EXPECT_TRUE(static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0])->serviceName == "14002") << "SocketChannelConfig::serviceName , \"14002\"";
			}
		}
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testOverridingFromInterface()";
	}
}

TEST_F(EmaConfigTest, testMergingConfigBetweenFileAndProgrammaticConfig)
{
	Map configDB1, configDB2, configDB3, configDB4, innerMap;
	ElementList elementList;

	try
	{
		// Default Consumer name Consumer_2 is defined in EmaConfig.xml
		innerMap.addKeyAscii("Consumer_2", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_2")
			.addAscii("Logger", "Logger_2")
			.addAscii("Dictionary", "Dictionary_2")
			.addAscii("RestLogFileName", "Rest.log")
			.addUInt("RestEnableLog", 1)
			.addUInt("RestVerboseMode", 1)
			.addUInt("SendJsonConvError", 0)
			.addUInt("ItemCountHint", 9000)
			.addUInt("ServiceCountHint", 9000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 9000)
			.addUInt("RequestTimeout", 9000)
			.addUInt("MaxOutstandingPosts", 9000)
			.addInt("DispatchTimeoutApiThread", 5656)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 900)
			.addUInt("MaxDispatchCountUserThread", 900)
			.addInt("MaxEventsInPool", 900)
			.addAscii("XmlTraceFileName", "ConfigDbXMLTrace")
			.addInt("XmlTraceMaxFileSize", 70000000)
			.addUInt("XmlTraceToFile", 1)
			.addUInt("XmlTraceToStdout", 0)
			.addUInt("XmlTraceToMultipleFiles", 0)
			.addUInt("XmlTraceWrite", 0)
			.addUInt("XmlTraceRead", 0)
			.addUInt("XmlTracePing", 0)
			.addUInt("XmlTracePingOnly", 0)
			.addUInt("XmlTraceHex", 0)
			.addUInt("MsgKeyInUpdates", 0)
			.addInt("ReconnectAttemptLimit", 70)
			.addInt("ReconnectMinDelay", 7000)
			.addInt("ReconnectMaxDelay", 7000)
			.addInt("PipePort", 9696)
			.addDouble("TokenReissueRatio", 0.90)
			.addInt("ReissueTokenAttemptLimit",20)
			.addInt("ReissueTokenAttemptInterval", 15000)
			.addAscii("RestProxyHostName", "restProxyNonLocalHost")
			.addAscii("RestProxyPort", "9083")
			.complete()).complete();

		elementList.addMap("ConsumerList", innerMap).complete();
		innerMap.clear();

		configDB1.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", 0)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 2)
			.addUInt("GuaranteedOutputBuffers", 7000)
			.addUInt("NumInputBuffers", 888888)
			.addUInt("SysRecvBufSize", 550000)
			.addUInt("SysSendBufSize", 700000)
			.addUInt("CompressionThreshold", 12758)
			.addUInt("ConnectionPingTimeout", 70000)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 1)
			.addUInt("InitializationTimeout", 77) // Overried the 55 value defined in the config file
			.addUInt("DirectWrite", 1)
			.addUInt("EnableSessionManagement", 0) // Override the 1 value defined in the config file as the OAuth credential is required by the session mgnt.
			.complete()).complete();

		elementList.addMap("ChannelList", innerMap).complete();
		innerMap.clear();

		configDB2.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "ConfigDB2_logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 4).complete()).complete();

		elementList.addMap("LoggerList", innerMap).complete();
		innerMap.clear();

		configDB3.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 1)
			.addAscii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def").complete()).complete();

		elementList.addMap("DictionaryList", innerMap).complete();
		innerMap.clear();

		configDB4.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		EmaString workingDir;
		ASSERT_EQ(getCurrentDir(workingDir), true)
			<< "Error: failed to load config file from current working dir "
			<< workingDir.c_str();
		EmaString localConfigPath;
		localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig(localConfigPath).config(configDB1).config(configDB2).config(configDB3).config(configDB4));
		//OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig(localConfigPath).config(configDB1).config(configDB2).config(configDB3).config(configDB4), true);

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());

		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_2" ) >= 0 ? true : false;
		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_2_3\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_2" ) << "Connection name , \"Channel_2\"";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_2" ) << "Logger name , \"Logger_2\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_2" ) << "dictionaryName , \"Dictionary_2\"";
		EXPECT_TRUE(activeConfig.restLogFileName == "Rest.log") << "restLogFileName , \"Rest.log\"";
		EXPECT_TRUE(activeConfig.restEnableLog == 1) << "restEnableLog , \"True\"";
		EXPECT_TRUE(activeConfig.restVerboseMode == 1) << "restVerboseMode , \"True\"";
		EXPECT_TRUE(activeConfig.sendJsonConvError == 0) << "SendJsonConvError , \"False\"";
		EXPECT_TRUE( activeConfig.itemCountHint == 9000) << "itemCountHint , 9000";
		EXPECT_TRUE( activeConfig.serviceCountHint == 9000) << "serviceCountHint , 9000";
		EXPECT_TRUE( activeConfig.obeyOpenWindow == 1) << "obeyOpenWindow , 1";
		EXPECT_TRUE( activeConfig.postAckTimeout == 9000) << "postAckTimeout , 9000";
		EXPECT_TRUE( activeConfig.requestTimeout == 9000) << "requestTimeout , 9000";
		EXPECT_TRUE( activeConfig.maxOutstandingPosts == 9000) << "maxOutstandingPosts , 9000";
		EXPECT_TRUE( activeConfig.dispatchTimeoutApiThread == 5656) << "dispatchTimeoutApiThread , 5656";
		EXPECT_TRUE( activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
		EXPECT_TRUE( activeConfig.maxDispatchCountApiThread == 900) << "maxDispatchCountApiThread , 900";
		EXPECT_TRUE( activeConfig.maxDispatchCountUserThread == 900) << "maxDispatchCountUserThread , 900";
		EXPECT_TRUE( activeConfig.maxEventsInPool == 900) << "maxEventsInPool , 900";
		EXPECT_TRUE( activeConfig.pipePort == 9696) << "pipePort , 9696";
		EXPECT_TRUE( activeConfig.reconnectAttemptLimit == 70) << "reconnectAttemptLimit , 70";
		EXPECT_TRUE( activeConfig.reconnectMinDelay == 7000) << "reconnectMinDelay , 7000";
		EXPECT_TRUE( activeConfig.reconnectMaxDelay == 7000) << "reconnectMaxDelay , 7000";
		EXPECT_TRUE( activeConfig.xmlTraceFileName == "ConfigDbXMLTrace" ) << "xmlTraceFileName , \"ConfigDbXMLTrace\"";
		EXPECT_TRUE( activeConfig.xmlTraceMaxFileSize == 70000000) << "xmlTraceMaxFileSize , 70000000";
		EXPECT_TRUE( activeConfig.xmlTraceToFile == 1) << "xmlTraceToFile , 1";
		EXPECT_TRUE( activeConfig.xmlTraceToStdout == 0) << "xmlTraceToStdout , 0";
		EXPECT_TRUE( activeConfig.xmlTraceToMultipleFiles == 0) << "xmlTraceToMultipleFiles , 0";
		EXPECT_TRUE( activeConfig.xmlTraceWrite == 0) << "xmlTraceWrite , 0";
		EXPECT_TRUE( activeConfig.xmlTraceRead == 0) << "xmlTraceRead , 0";
		EXPECT_TRUE( activeConfig.xmlTracePing == 0) << "xmlTracePing , 0";
		EXPECT_TRUE( activeConfig.xmlTracePingOnly == 0) << "xmlTracePingOnly , 0";
		EXPECT_TRUE( activeConfig.xmlTraceHex == 0) << "xmlTraceHex , 0";
		EXPECT_TRUE( activeConfig.msgKeyInUpdates == 0) << "msgKeyInUpdates , 0";
		EXPECT_TRUE( activeConfig.tokenReissueRatio == 0.90) << "tokenReissueRatio , 0.90";
		EXPECT_TRUE( activeConfig.reissueTokenAttemptLimit == 20) << "reissueTokenAttemptLimit , 20";
		EXPECT_TRUE( activeConfig.reissueTokenAttemptInterval == 15000) << "reissueTokenAttemptInterval , 15000";
		EXPECT_TRUE( activeConfig.restProxyHostName == "restProxyNonLocalHost" ) << "restProxyHostName , \"restProxyNonLocalHost\"";
		EXPECT_TRUE( activeConfig.restProxyPort == "9083" ) << "restProxyPort , \"9083\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->compressionType == 2) << "compressionType , 2";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 7000) << "guaranteedOutputBuffers , 7000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->numInputBuffers == 888888) << "numInputBuffers , 888888";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysRecvBufSize == 550000) << "sysRecvBufSize , 550000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysSendBufSize == 700000) << "sysSendBufSize , 700000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->compressionThreshold == 12758) << "compressionThreshold , compressionThreshold";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionPingTimeout == 70000) << "connectionPingTimeout , 70000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->directWrite == 1) << "directWrite , 1";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->hostName == "localhost" ) << "SocketChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->serviceName == "14002" ) << "SocketChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->tcpNodelay == 1) << "SocketChannelConfig::tcpNodelay , 1";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->initializationTimeout == 77) << "SocketChannelConfig::initializationTimeout , 77";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->enableSessionMgnt == 0) << "SocketChannelConfig::enableSessionMgnt , 0";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerFileName == "ConfigDB2_logFile" ) << "loggerFileName = \"ConfigDB2_logFile\"";
		EXPECT_TRUE( activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::NoLogMsgEnum) << "minLoggerSeverity = OmmLoggerClient::NoLogMsgEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
		EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum) << "dictionaryType , Dictionary::ChannelDictionaryEnum";
		EXPECT_TRUE( activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == "./ConfigDB3_RDMFieldDictionary" ) << "rdmfieldDictionaryFileName , \"./ConfigDB3_RDMFieldDictionary\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.enumtypeDefFileName == "./ConfigDB3_enumtype.def" ) << "enumtypeDefFileName , \"./ConfigDB3_enumtype.def\"";
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testMergingConfigBetweenFileAndProgrammaticConfig()";
	}
}

TEST_F(EmaConfigTest, testLoadFromFileCfgChannelSet)
{
	OmmConsumerConfigImpl config(configPath);

	config.configErrors().printErrors( OmmLoggerClient::WarningEnum );
	EmaString channelVal;
	EmaString channelSet;
	EmaString temp;
	config.getChannelName( "Consumer_3", temp );
	EXPECT_TRUE( temp == "Channel_1" ) << "gettting channel name";

	bool result = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_3|Channel", channelVal );
	EXPECT_TRUE( result && channelVal == "Channel_1" ) << "extracting Channel name from EmaConfig.xml";

	result = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_3|ChannelSet", channelSet );
	EXPECT_TRUE( ( !result ) && channelSet != "Channel_2, Channel_3" ) << "extracting ChannelSet value from EmaConfig.xml";

	channelVal.clear();
	channelSet.clear();
	result = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_4|Channel", channelVal );
	EXPECT_TRUE( !result && channelVal != "Channel_1" ) << "extracting Channel name from EmaConfig.xml";

	result = config.get<EmaString>( "ConsumerGroup|ConsumerList|Consumer.Consumer_4|ChannelSet", channelSet );
	EXPECT_TRUE( result && channelSet == "Channel_4, Channel_5" ) << "extracting ChannelSet value from EmaConfig.xml";
}

TEST_F(EmaConfigTest, testProgCfgChannelAfterChannelSet)
{
	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_3");

		innerMap.addKeyAscii("Consumer_3", MapEntry::AddEnum,
			ElementList()
			.addAscii("ChannelSet", "Channel_2, Channel_3")
			.addAscii("Channel", "Channel_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1").complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap);
		elementList.complete();
		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.clear();
		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_SOCKET)
			.addAscii("InterfaceName", "localhost")
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0).complete())
			.addKeyAscii("Channel_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", RSSL_CONN_TYPE_HTTP)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "localhost")
				.addAscii("Port", "14008")
				.addUInt("TcpNodelay", 0).complete())
			.addKeyAscii("Channel_3", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", RSSL_CONN_TYPE_ENCRYPTED)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "localhost")
				.addAscii("Port", "14009")
				.addUInt("TcpNodelay", 0).complete())
			.complete();
		elementList.addMap("ChannelList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		innerMap.clear();
		elementList.clear();
		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();
		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );
		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_3" ) >= 0 ? true : false;

		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_3_1\"";
		EXPECT_TRUE( activeConfig.configChannelSet.size() == 1) << "Channel Count , 1";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_1" ) << "Connection name , \"Channel_1\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "Connection type , \"RSSL_CONN_TYPE_SOCKET\"";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testProgCfgChannelAfterChannelSet()";
	}
}

TEST_F(EmaConfigTest, testProgCfgChannelSetAfterChannel)
{
	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		elementList.addAscii("DefaultConsumer", "Consumer_3");

		innerMap.addKeyAscii("Consumer_3", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_1")
			.addAscii("ChannelSet", "Channel_2, Channel_3")
			.addAscii("Logger", "Logger_1")
			.addAscii("Dictionary", "Dictionary_1")
			.addInt("ReconnectAttemptLimit", 2)
			.addInt("ReconnectMaxDelay", 775)
			.addInt("ReconnectMinDelay", 725)
			.addAscii("XmlTraceFileName", "Consumer_3Trace")
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTracePingOnly", 1)
			.addUInt("XmlTraceHex", 1)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 0)
			.addUInt("MsgKeyInUpdates", 0)
			.addUInt("XmlTraceWrite", 1)
			.addInt("XmlTraceMaxFileSize", 8800).complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap);
		elementList.complete();
		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.clear();
		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_HTTP)
			.addAscii("InterfaceName", "localhost")
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.complete())
			.addKeyAscii("Channel_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", RSSL_CONN_TYPE_SOCKET)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "localhost")
				.addAscii("Port", "14008")
				.addUInt("TcpNodelay", 0)
				.addEnum("CompressionType", RSSL_COMP_NONE)
				.complete())
			.addKeyAscii("Channel_3", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", RSSL_CONN_TYPE_ENCRYPTED)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "0.0.0.2")
				.addAscii("Port", "14009")
				.addUInt("TcpNodelay", 1)
				.addEnum("CompressionType", RSSL_COMP_ZLIB)
				.complete())
			.complete();
		elementList.addMap("ChannelList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		innerMap.clear();
		elementList.clear();
		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
			.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
			.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();
		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );
		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_3" ) >= 0 ? true : false;

		SocketChannelConfig* socCfg = static_cast<SocketChannelConfig*>( activeConfig.configChannelSet[0] );
		SocketChannelConfig* encCfg = static_cast<SocketChannelConfig*>( activeConfig.configChannelSet[1] );
		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_3_1\"";
		EXPECT_TRUE( activeConfig.configChannelSet.size() == 2) << "Channel Count , 2";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_2" ) << "Connection name , \"Channel_2\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "Connection type , \"RSSL_CONN_TYPE_SOCKET\"";
		
		EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 2) << "reconnectAttemptLimit , \"2\"";
		EXPECT_TRUE(activeConfig.reconnectMaxDelay == 775) << "reconnectMaxDelay , \"775\"";
		EXPECT_TRUE(activeConfig.reconnectMinDelay == 725) << "reconnectMaxDelay , \"725\"";
		EXPECT_TRUE(activeConfig.xmlTraceFileName == "Consumer_3Trace" ) << "xmlTraceFileName , \"Consumer_3Trace\"";
		EXPECT_TRUE(activeConfig.xmlTraceRead == true) << "xmlTraceRead , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTracePing == true) << "xmlTracePing , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTracePingOnly == true) << "xmlTracePingOnly , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTraceHex == true) << "xmlTraceHex , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTraceWrite == true) << "xmlTraceWrite , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTraceToFile == false) << "xmlTraceToFile , \"false\"";
		EXPECT_TRUE(activeConfig.xmlTraceToStdout == false) << "xmlTraceToStdout , \"false\"";
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == false) << "msgKeyInUpdates , \"false\"";
		EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 8800) << "xmlTraceMaxFileSize , \"8800\"";


		EXPECT_TRUE( socCfg->hostName == "localhost" ) << "hostname , \"localhost\"";
		EXPECT_TRUE( socCfg->serviceName == "14008" ) << "serviceName , \"14008\"";
		EXPECT_TRUE( socCfg->tcpNodelay == 0) << "tcpNodelay , 0";
		EXPECT_TRUE( socCfg->compressionType == RSSL_COMP_NONE) << "compressionType , \"RSSL_COMP_NONE\"";

		EXPECT_TRUE( activeConfig.configChannelSet[1]->name == "Channel_3" ) << "Connection name , \"Channel_3\"";
		EXPECT_TRUE( activeConfig.configChannelSet[1]->connectionType == RSSL_CONN_TYPE_ENCRYPTED) << "Connection type , \"RSSL_CONN_TYPE_ENCRYPTED\"";
		EXPECT_TRUE( encCfg->hostName == "0.0.0.2" ) << "hostname , \"0.0.0.2\"";
		EXPECT_TRUE( encCfg->serviceName == "14009" ) << "serviceName , \"14009\"";
		EXPECT_TRUE( encCfg->tcpNodelay == 1) << "tcpNodelay , 1";
		EXPECT_TRUE( encCfg->compressionType == RSSL_COMP_ZLIB) << "compressionType , \"RSSL_COMP_ZLIB\"";

		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testProgCfgChannelSetAfterChannel()";
	}
}

TEST_F(EmaConfigTest, testLoadChannelSetBwteenFileProgrammaticForNiProv)
{
	//testcase 1: test setting channel, then channelset. ChannelSet takes priority.
	//testcase 2: test setting channelset, then channel. Channel takes priority.

	for (int testCase = 0; testCase < 2; testCase++)
	{
		Map outermostMap, innerMap;
		ElementList elementList;

		try
		{
			elementList.addAscii("DefaultNiProvider", "Provider_3");

			if (testCase == 0)
			{
				innerMap.addKeyAscii("Provider_3", MapEntry::AddEnum,
					ElementList()
					.addAscii("Channel", "Channel_1")
					.addAscii("ChannelSet", "Channel_2, Channel_3")
					.addAscii("Logger", "Logger_1")
					.addAscii("Directory", "Directory_5")
					.complete())
					.complete();
			}
			else if (testCase == 1)
			{
				innerMap.addKeyAscii("Provider_3", MapEntry::AddEnum,
					ElementList()
					.addAscii("ChannelSet", "Channel_2, Channel_3")
					.addAscii("Channel", "Channel_1")
					.addAscii("Logger", "Logger_1")
					.addAscii("Directory", "Directory_5")
					.complete())
					.complete();
			}
				
			elementList.addMap("NiProviderList", innerMap);
			elementList.complete();
			outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();
			innerMap.clear();
			innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("ChannelType", RSSL_CONN_TYPE_SOCKET)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "localhost1")
				.addAscii("Port", "14002")
				.addUInt("TcpNodelay", 0).complete())
				.addKeyAscii("Channel_2", MapEntry::AddEnum,
					ElementList()
					.addEnum("ChannelType", RSSL_CONN_TYPE_HTTP)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Host", "localhost2")
					.addAscii("Port", "14008")
					.addAscii("ProxyHost", "proxyhost2")
					.addAscii("ProxyPort", "proxyport2")
					.addUInt("ProxyConnectionTimeout", 32)
					.addAscii("ObjectName", "objectname2")
					.addUInt("TcpNodelay", 0).complete())
				.addKeyAscii("Channel_3", MapEntry::AddEnum,
					ElementList()
					.addEnum("ChannelType", RSSL_CONN_TYPE_ENCRYPTED)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Host", "localhost3")
					.addAscii("Port", "14009")
					.addAscii("ProxyHost", "proxyhost3")
					.addAscii("ProxyPort", "proxyport3")
					.addUInt("ProxyConnectionTimeout", 33)
					.addAscii("ObjectName", "objectname3")
					.addUInt("TcpNodelay", 0).complete())
				.complete();
			elementList.addMap("ChannelList", innerMap);

			elementList.complete();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

			innerMap.clear();
			elementList.clear();
			innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 0)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();
			innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

			elementList.addMap("DictionaryList", innerMap);

			elementList.complete();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

			outermostMap.complete();
			
			EmaString localConfigPath;
			EmaString workingDir;
			ASSERT_EQ(getCurrentDir(workingDir), true)
				<< "Error: failed to load config file from current working dir "
				<< workingDir.c_str();
			localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			OmmNiProviderConfig niprovConfig(localConfigPath);
			OmmNiProviderImpl ommNiProviderImpl(niprovConfig.config(outermostMap), appClient);

			OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());
			bool found = ommNiProviderImpl.getInstanceName().find("Provider_3") >= 0 ? true : false;

			EXPECT_TRUE(found) << "ommProviderImpl.getProviderName() , \"Provider_3_1\"";
			if (testCase == 0)
			{
				EXPECT_TRUE(activeConfig.configChannelSet.size() == 2) << "Channel Count , 2";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_2") << "Connection name , \"Channel_2\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_HTTP) << "Connection type , \"RSSL_CONN_TYPE_HTTP\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyHostName == "proxyhost2") << "Proxy hostname , \"proxyhost2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyPort == "proxyport2") << "Proxy port , \"proxyport2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyConnectionTimeout == 32) << "Proxy connection timeout , \"32\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->objectName == "objectname2") << "Object name , \"objectname2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->hostName == "localhost2") << "hostname , \"localhost2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->serviceName == "14008") << "serviceName , \"14009\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->tcpNodelay == 0) << "tcpNodelay , 0";

				EXPECT_TRUE(activeConfig.configChannelSet[1]->name == "Channel_3") << "Connection name , \"Channel_3\"";
				EXPECT_TRUE(activeConfig.configChannelSet[1]->connectionType == RSSL_CONN_TYPE_ENCRYPTED) << "Connection type , \"RSSL_CONN_TYPE_ENCRYPTED\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->proxyHostName == "proxyhost3") << "Proxy hostname , \"proxyhost3\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->proxyPort == "proxyport3") << "Proxy port , \"proxyport3\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->proxyConnectionTimeout == 33) << "Proxy connection timeout , \"33\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->objectName == "objectname3") << "Object name , \"objectname3\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->hostName == "localhost3") << "hostname , \"localhost3\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->serviceName == "14009") << "serviceName , \"14009\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[1]))->tcpNodelay == 0) << "tcpNodelay , 0";
			}
			else if (testCase == 1)
			{
				EXPECT_TRUE(activeConfig.configChannelSet.size() == 1) << "Channel Count , 1";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1") << "Connection name , \"Channel_1\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "Connection type , \"RSSL_CONN_TYPE_SOCKET\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->hostName == "localhost1") << "hostname , \"localhost1\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->serviceName == "14002") << "serviceName , \"14002\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->tcpNodelay == 0) << "tcpNodelay , 0";
			}

			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";

			//retrieve directory
			const DirectoryCache& dirCache = (static_cast<OmmNiProviderDirectoryStore&>(ommNiProviderImpl.getDirectoryServiceStore())).getApiControlDirectory();
			EXPECT_TRUE(dirCache.directoryName == "Directory_5") << "directoryName = \"Directory_5\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 1) << "services.size() , 1";

			/*********retrieve first service *************/
			Service* pTemp = services.front();
			EXPECT_TRUE(pTemp) << "services.front() , true";
			EXPECT_TRUE(pTemp->serviceId == 0) << "serviceId , 0";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "NI_PUB") << "infoFilter.serviceName , \"NI_PUB\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 0) << "infoFilter.acceptingConsumerStatus , 0";

			//retrieve capabilities
			int idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 4) << "infoFilter.capabilities.size , 4";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 6) << "infoFilter.capabilities[0] , \"MMT_MARKET_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 7) << "infoFilter.capabilities[1] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 8) << "infoFilter.capabilities[2] , \"MMT_MARKET_BY_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 9) << "infoFilter.capabilities[3] , \"MMT_MARKET_MAKER\"";

			//retrieve qos
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 1) << "infoFilter.qos.size , 1";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TICK_BY_TICK) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TICK_BY_TICK\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_REALTIME) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_REALTIME\"";

			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 0) << "infoFilter.dictionariesProvided.size , 0";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testProgCfgChannelAfterChannelSet()";
		}
	}
}

TEST_F(EmaConfigTest, testMergCfgBetweenFunctionCallAndFileAndProgrammatic)
{
	//testcase 1: test function call for socket connection
	//testcase 2: test function call for http/encrypted connection

	for (int testCase = 0; testCase < 2; testCase++)
	{
		Map outermostMap, innerMap;
		ElementList elementList;

		try
		{
			if (testCase == 0)
			{
				innerMap.addKeyAscii("Consumer_2", MapEntry::AddEnum,
					ElementList()
					.addAscii("Channel", "Channel_2")
					.addAscii("Logger", "Logger_1")
					.addAscii("Dictionary", "Dictionary_1")
					.complete())
					.complete();
			}
			else if (testCase == 1)
			{
				innerMap.addKeyAscii("Consumer_2", MapEntry::AddEnum,
					ElementList()
					.addAscii("Channel", "Channel_6")
					.addAscii("Logger", "Logger_1")
					.addAscii("Dictionary", "Dictionary_1")
					.addAscii("RestProxyHostName", "ProgrammaticRestProxyHostName")
					.addAscii("RestProxyPort", "9898")
					.complete())
					.complete();
			}

			elementList.addMap("ConsumerList", innerMap);
			elementList.complete();
			outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();
			innerMap.clear();
			if (testCase == 0)
			{
				innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum,
					ElementList()
					.addEnum("ChannelType", RSSL_CONN_TYPE_HTTP)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Host", "localhost")
					.addAscii("Port", "14008")
					.addAscii("ProxyHost", "proxyhost2")
					.addAscii("ProxyPort", "proxyport2")
					.addUInt("ProxyConnectionTimeout", 22)
					.addAscii("ObjectName", "objectname2")
					.addUInt("TcpNodelay", 0).complete()).complete();
			}
			else if (testCase == 1)
			{
				innerMap.addKeyAscii("Channel_6", MapEntry::AddEnum,
					ElementList()
					.addEnum("ChannelType", RSSL_CONN_TYPE_SOCKET)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Host", "localhost")
					.addAscii("Port", "14009")
					.addAscii("ProxyHost", "proxyhost6")
					.addAscii("ProxyPort", "proxyport6")
					.addUInt("ProxyConnectionTimeout", 26)
					.addAscii("ObjectName", "objectname6")
					.addUInt("TcpNodelay", 0).complete())
					.complete();
			}
			elementList.addMap("ChannelList", innerMap);

			elementList.complete();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

			innerMap.clear();
			elementList.clear();
			innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 0)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();
			innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete()).complete();

			elementList.addMap("DictionaryList", innerMap);

			elementList.complete();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

			outermostMap.complete();

			EmaString localConfigPath;
			EmaString workingDir;
			ASSERT_EQ(getCurrentDir(workingDir), true)
				<< "Error: failed to load config file from current working dir "
				<< workingDir.c_str();
			localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			OmmConsumerConfig consumerConfig(localConfigPath);
			if (testCase == 0)
			{
				consumerConfig.config(outermostMap).host("localhostFC:14022");
				consumerConfig.restProxyHostName("aaa.proxyrest.srv").restProxyPort("12099");
				consumerConfig.restProxyUserName("uname.proxyrest").restProxyPasswd("upasswd");
				consumerConfig.restProxyDomain("rest.proxy.domain");
			}
			else if (testCase == 1)
			{
				consumerConfig.config(outermostMap).tunnelingProxyHostName("proxyHost").tunnelingProxyPort("14032").tunnelingObjectName("objectName");
				consumerConfig.restProxyHostName("bbb.proxyrest.srv").restProxyPort("12399");
				consumerConfig.restProxyUserName("uname1.proxyrest").restProxyPasswd("upasswd1");
				consumerConfig.restProxyDomain("rest.proxy1.domain");
			}

			OmmConsumerImpl ommConsumerImpl(consumerConfig, true);

			OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
			bool found = ommConsumerImpl.getInstanceName().find("Consumer_2") >= 0 ? true : false;

			EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_2_1\"";
			if (testCase == 0)
			{
				EXPECT_TRUE(activeConfig.configChannelSet.size() == 1) << "Channel Count , 1";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_2") << "Connection name , \"Channel_2\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "Connection type , \"RSSL_CONN_TYPE_SOCKET\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->hostName == "localhostFC") << "hostname , \"localhostFC\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->serviceName == "14022") << "serviceName , \"14022\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->tcpNodelay == 0) << "tcpNodelay , 0";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyHostName == "proxyhost2") << "Proxy hostname , \"proxyhost2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyPort == "proxyport2") << "Proxy port , \"proxyport2\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyConnectionTimeout == 22) << "proxyConnectionTimeout , 22";

				EXPECT_TRUE(activeConfig.restProxyHostName == "aaa.proxyrest.srv") << "restProxyHostName " << activeConfig.restProxyHostName;
				EXPECT_TRUE(activeConfig.restProxyPort == "12099") << "restProxyPort " << activeConfig.restProxyPort;
				EXPECT_TRUE(activeConfig.restProxyUserName == "uname.proxyrest") << "restProxyUserName " << activeConfig.restProxyUserName;
				EXPECT_TRUE(activeConfig.restProxyPasswd == "upasswd") << "restProxyPasswd " << activeConfig.restProxyPasswd;
				EXPECT_TRUE(activeConfig.restProxyDomain == "rest.proxy.domain") << "restProxyDomain " << activeConfig.restProxyDomain;
			}
			else if (testCase == 1)
			{
				EXPECT_TRUE(activeConfig.configChannelSet.size() == 1) << "Channel Count , 1";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_6") << "Connection name , \"Channel_6\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "Connection type , \"RSSL_CONN_TYPE_SOCKET\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyHostName == "proxyHost") << "Proxy hostname , \"proxyHost\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyPort == "14032") << "Proxy port , \"14032\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyConnectionTimeout == 26) << "proxyConnectionTimeout , 26";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->objectName == "objectName") << "Object name , \"objectName\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->hostName == "localhost") << "hostname , \"localhost\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->serviceName == "14009") << "serviceName , \"14009\"";
				EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->tcpNodelay == 0) << "tcpNodelay , 0";

				EXPECT_TRUE(activeConfig.restProxyHostName == "ProgrammaticRestProxyHostName") << "restProxyHostName " << activeConfig.restProxyHostName;
				EXPECT_TRUE(activeConfig.restProxyPort == "9898") << "restProxyPort " << activeConfig.restProxyPort;
				EXPECT_TRUE(activeConfig.restProxyUserName == "uname1.proxyrest") << "restProxyUserName " << activeConfig.restProxyUserName;
				EXPECT_TRUE(activeConfig.restProxyPasswd == "upasswd1") << "restProxyPasswd " << activeConfig.restProxyPasswd;
				EXPECT_TRUE(activeConfig.restProxyDomain == "rest.proxy1.domain") << "restProxyDomain " << activeConfig.restProxyDomain;
			}

			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
			EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1") << "dictionaryName , \"Dictionary_1\"";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testMergCfgBetweenFunctionCallAndFileAndProgrammatic()";
		}
	}
}

TEST_F(EmaConfigTest, testMergCfgBetweenFunctionCallAndFileAndProgrammaticNiProv)
{
	//test function call for niprov encrypted connection

	Map outermostMap, innerMap;
	ElementList elementList;

	try
	{
		innerMap.addKeyAscii("Provider_2", MapEntry::AddEnum,
			ElementList()
			.addAscii("Channel", "Channel_6")
			.addAscii("Logger", "Logger_1")
			.complete())
			.complete();

		elementList.addMap("NiProviderList", innerMap);
		elementList.complete();
		outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.clear();
		innerMap.addKeyAscii("Channel_6", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", RSSL_CONN_TYPE_ENCRYPTED)
			.addAscii("InterfaceName", "localhost")
			.addAscii("Host", "localhost")
			.addAscii("Port", "14009")
			.addAscii("ProxyHost", "proxyhost6")
			.addAscii("ProxyPort", "proxyport6")
			.addUInt("ProxyConnectionTimeout", 30)
			.addAscii("ObjectName", "objectname6")
			.addUInt("TcpNodelay", 0).complete())
			.complete();
		elementList.addMap("ChannelList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

		innerMap.clear();
		elementList.clear();
		innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "logFile")
			.addUInt("NumberOfLogFiles", 42)
			.addUInt("MaxLogFileSize", 84000)
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		EmaString localConfigPath;
		EmaString workingDir;
		ASSERT_EQ(getCurrentDir(workingDir), true)
			<< "Error: failed to load config file from current working dir "
			<< workingDir.c_str();
		localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

		OmmNiProviderConfig niProviderConfig(localConfigPath);
		niProviderConfig.config(outermostMap).tunnelingProxyHostName("proxyHost").tunnelingProxyPort("14032").tunnelingObjectName("objectName");

		OmmNiProviderImpl ommNiProviderImpl(niProviderConfig, appClient);

		OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());
		bool found = ommNiProviderImpl.getInstanceName().find("Provider_2") >= 0 ? true : false;

		EXPECT_TRUE(found) << "ommNiProviderImpl.getNiProviderName() , \"Provider_2_1\"";

		EXPECT_TRUE(activeConfig.configChannelSet.size() == 1) << "Channel Count , 1";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_6") << "Connection name , \"Channel_6\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_ENCRYPTED) << "Connection type , \"RSSL_CONN_TYPE_ENCRYPTED\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyHostName == "proxyHost") << "Proxy hostname , \"proxyHost\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyPort == "14032") << "Proxy port , \"14032\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->proxyConnectionTimeout == 30) << "proxyConnectionTimeout , 30";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->objectName == "objectName") << "Object name , \"objectName\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->hostName == "localhost") << "hostname , \"localhost\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->serviceName == "14009") << "serviceName , \"14009\"";
		EXPECT_TRUE((static_cast<SocketChannelConfig*>(activeConfig.configChannelSet[0]))->tcpNodelay == 0) << "tcpNodelay , 0";

		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testMergCfgBetweenFunctionCallAndFileAndProgrammaticNiProv()";
	}
}

TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfigForIProv)
{
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	//test case 3: ServerType == RSSL_CONN_TYPE_WEBSOCKET
	for (int testCase = 0; testCase < 3; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			elementList.addAscii("DefaultIProvider", "Provider_1");

			innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
				.addAscii("Server", "Server_1")
				.addAscii("Logger", "Logger_1")
				.addAscii("Directory", "Directory_1")
				.addUInt("ItemCountHint", 5000)
				.addUInt("ServiceCountHint", 2000)
				.addUInt("AcceptDirMessageWithoutMinFilters", 1)
				.addUInt("AcceptMessageSameKeyButDiffStream", 1)
				.addUInt("AcceptMessageThatChangesService", 1)
				.addUInt("AcceptMessageWithoutAcceptingRequests", 1)
				.addUInt("AcceptMessageWithoutBeingLogin", 1)
				.addUInt("AcceptMessageWithoutQosInRange", 1)
				.addUInt("FieldDictionaryFragmentSize", 2000)
				.addUInt("EnumTypeFragmentSize", 1000)
				.addUInt("RefreshFirstRequired", 0)
				.addUInt("EnforceAckIDValidation", 0)
				.addUInt("RequestTimeout", 2400)
				.addInt("DispatchTimeoutApiThread", 60)
				.addUInt("CatchUnhandledException", 1)
				.addUInt("MaxDispatchCountApiThread", 300)
				.addUInt("MaxDispatchCountUserThread", 700)
				.addInt("MaxEventsInPool", 600)
				.addAscii("XmlTraceFileName", "MyXMLTrace")
				.addInt("XmlTraceMaxFileSize", 50000000)
				.addUInt("XmlTraceToFile", 0)
				.addUInt("XmlTraceToStdout", 1)
				.addUInt("XmlTraceToMultipleFiles", 1)
				.addUInt("XmlTraceWrite", 1)
				.addUInt("XmlTraceRead", 1)
				.addUInt("XmlTracePing", 1)
				.addUInt("XmlTracePingOnly", 1)
				.addUInt("XmlTraceHex", 1)
				.addInt("PipePort", 13650).complete())
				.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
					.addAscii("Server", "Server_2")
					.addAscii("Directory", "Directory_2")
					.addAscii("Logger", "Logger_2").complete())
				.complete();

			elementList.addMap("IProviderList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			if (testCase == 2)
			{
				innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", RSSL_CONN_TYPE_WEBSOCKET)
					.addEnum("CompressionType", 1)
					.addUInt("GuaranteedOutputBuffers", 8000)
					.addUInt("NumInputBuffers", 7777)
					.addUInt("SysRecvBufSize", 150000)
					.addUInt("SysSendBufSize", 200000)
					.addUInt("CompressionThreshold", 12856)
					.addUInt("ConnectionPingTimeout", 30000)
					.addUInt("ConnectionMinPingTimeout", 8000)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Port", "14010")
					.addUInt("TcpNodelay", 0)
					.addUInt("InitializationTimeout", 66)
					.addUInt("MaxFragmentSize", 100500)
					.addUInt("DirectWrite", 0)
					.addAscii("WsProtocols", "rssl.json.v2, rssl.rwf, tr_json2")
					.complete())
					.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
						.addEnum("ServerType", 1)
						.addAscii("Port", "14011").complete())
					.complete();

			}
			else
			{
				innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", 0)
					.addEnum("CompressionType", 1)
					.addUInt("GuaranteedOutputBuffers", 8000)
					.addUInt("NumInputBuffers", 7777)
					.addUInt("SysRecvBufSize", 150000)
					.addUInt("SysSendBufSize", 200000)
					.addUInt("CompressionThreshold", 12856)
					.addUInt("ConnectionPingTimeout", 30000)
					.addUInt("ConnectionMinPingTimeout", 8000)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Port", "14010")
					.addUInt("TcpNodelay", 0)
					.addUInt("InitializationTimeout", 66)
					.addUInt("DirectWrite", 0)
					.complete())
					.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
						.addEnum("ServerType", 1)
						.addAscii("Port", "14011").complete())
					.complete();
			}

			elementList.addMap("ServerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 1)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete())
				.addKeyAscii("Logger_1", MapEntry::AddEnum,
					ElementList()
					.addEnum("LoggerType", 0)
					.addAscii("FileName", "logFile")
					.addUInt("NumberOfLogFiles", 42)
					.addUInt("MaxLogFileSize", 84000)
					.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_3", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryItemName", "RWFFld")
				.addAscii("EnumTypeDefItemName", "RWFEnum")
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete())
				.addKeyAscii("Dictionary_4", MapEntry::AddEnum,
					ElementList()
					.addEnum("DictionaryType", 1)
					.addAscii("RdmFieldDictionaryItemName", "RWFFld_ID4")
					.addAscii("EnumTypeDefItemName", "RWFEnum_ID4")
					.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4")
					.addAscii("EnumTypeDefFileName", "./enumtype_ID4.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap;

			//encode service1
			serviceMap.addKeyAscii("DIRECT_FEED", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 3)
					.addAscii("Vendor", "company name")
					.addUInt("IsSource", 0)
					.addUInt("AcceptingConsumerStatus", 0)
					.addUInt("SupportsQoSRange", 0)
					.addUInt("SupportsOutOfBandSnapshots", 0)
					.addAscii("ItemList", "#.itemlist")
					.addArray("Capabilities",
						OmmArray().addAscii("MMT_DICTIONARY")
						.addAscii("MMT_MARKET_PRICE")
						.addAscii("MMT_MARKET_BY_ORDER")
						.addAscii("MMT_MARKET_BY_PRICE")
						.addAscii("200")
						.complete())
					.addArray("DictionariesProvided",
						OmmArray().addAscii("Dictionary_3")
						.complete())
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_4")
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
					.addElementList("Status",
						ElementList().addAscii("StreamState", "StreamState::CloseRecover")
						.addAscii("DataState", "DataState::Suspect")
						.addAscii("StatusCode", "StatusCode::DacsDown")
						.addAscii("StatusText", "dacsDown")
						.complete())
					.complete())
				.addElementList("LoadFilter",
					ElementList()
					.addUInt("OpenLimit", 1)
					.addUInt("OpenWindow", 2)
					.addUInt("LoadFactor", 3)
				.complete())
				.complete());

			//encode service2
			serviceMap.addKeyAscii("DIRECT_FEED1", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 4)
					.addAscii("Vendor", "company name")
					.addUInt("AcceptingConsumerStatus", 0)
					.addUInt("SupportsQoSRange", 0)
					.addUInt("SupportsOutOfBandSnapshots", 0)
					.addAscii("ItemList", "#.itemlist")
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_6")
						.complete())
					.addArray("DictionariesProvided",
						OmmArray().addAscii("Dictionary_2")
						.complete())
					.addArray("Capabilities",
						OmmArray().addAscii("MMT_DICTIONARY")
						.addAscii("MMT_MARKET_PRICE")
						.addAscii("MMT_MARKET_BY_ORDER")
						.addAscii("200")
						.complete())
					.complete())

				.addElementList("StateFilter",
					ElementList()
					.addUInt("ServiceState", 1)
					.addUInt("AcceptingRequests", 1)
					.complete())

				.addElementList("LoadFilter",
					ElementList()
					.addUInt("OpenLimit", 4294967295)
					.addUInt("OpenWindow", 4294967295)
					.addUInt("LoadFactor", 65535)
					.complete())
				.complete())
				.complete();


			innerMap.addKeyAscii("Directory_1", MapEntry::AddEnum, serviceMap).complete();

			elementList.clear();
			elementList.addAscii("DefaultDirectory", "Directory_1");
			elementList.addMap("DirectoryList", innerMap).complete();
			outermostMap.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList).complete();

			EmaString localConfigPath;
			if (testCase == 1)
			{
				EmaString workingDir;
				ASSERT_EQ(getCurrentDir(workingDir), true)
					<< "Error: failed to load config file from current working dir "
					<< workingDir.c_str();
				localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			}

			OmmIProviderConfig iprovConfig(localConfigPath);
			OmmIProviderImpl ommIProviderImpl(iprovConfig.config(outermostMap), appClient);

			OmmIProviderActiveConfig& activeConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
			bool found = ommIProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
			EXPECT_TRUE(activeConfig.pServerConfig->name == "Server_1") << "Server name , \"Server_1\"";
			EXPECT_TRUE(activeConfig.acceptDirMessageWithoutMinFilters == true) << "acceptDirMessageWithoutMinFilters , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutAcceptingRequests == true) << "acceptMessageWithoutAcceptingRequests , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutBeingLogin == true) << "acceptMessageWithoutBeingLogin , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutQosInRange == true) << "acceptMessageWithoutQosInRange , true";
			EXPECT_TRUE(activeConfig.acceptMessageSameKeyButDiffStream == true) << "acceptMessageSameKeyButDiffStream , true";
			EXPECT_TRUE(activeConfig.acceptMessageThatChangesService == true) << "acceptMessageThatChangesService , true";
			EXPECT_TRUE(activeConfig.getRefreshFirstRequired() == false) << "refreshFirstRequired , false";
			EXPECT_TRUE(activeConfig.getEnforceAckIDValidation() == false) << "enforceAckIDValidation , false";
			EXPECT_TRUE(activeConfig.getMaxFieldDictFragmentSize() == 2000) << "maxFieldDictFragmentSize , 2000";
			EXPECT_TRUE(activeConfig.getMaxEnumTypeFragmentSize() == 1000) << "maxEnumTypeFragmentSize , 1000";

			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
			EXPECT_TRUE(activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
			EXPECT_TRUE(activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
			EXPECT_TRUE(activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
			EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
			EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
			EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
			EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
			EXPECT_TRUE(activeConfig.maxEventsInPool == 600) << "maxEventsInPool , 600";
			EXPECT_TRUE(activeConfig.xmlTraceFileName == "MyXMLTrace") << "xmlTraceFileName , \"MyXMLTrace\"";
			EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
			EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
			EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
			EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
			EXPECT_TRUE(activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
			EXPECT_TRUE(activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
			EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
			EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
			EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
			EXPECT_TRUE(activeConfig.pipePort == 13650) << "pipePort , 13650";
			EXPECT_TRUE(activeConfig.pServerConfig->interfaceName == "localhost") << "interfaceName , \"localhost\"";
			EXPECT_TRUE(activeConfig.pServerConfig->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
			EXPECT_TRUE(activeConfig.pServerConfig->numInputBuffers == 7777) << "numInputBuffers , 7777";
			EXPECT_TRUE(activeConfig.pServerConfig->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
			EXPECT_TRUE(activeConfig.pServerConfig->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
			if (testCase == 2)
			{
				EXPECT_TRUE(activeConfig.pServerConfig->connectionType == RSSL_CONN_TYPE_WEBSOCKET) << "connectionType , RSSL_CONN_TYPE_WEBSOCKET";
				EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->maxFragmentSize == 100500) << "SocketServerConfig::maxFragmentSize , 0";
				EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->wsProtocols == "rssl.json.v2, rssl.rwf, tr_json2") << "SocketServerConfig::wsProtocols , 0";
			}
			else
			{
				EXPECT_TRUE(activeConfig.pServerConfig->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , RSSL_CONN_TYPE_SOCKET";
			}
			EXPECT_TRUE(activeConfig.pServerConfig->compressionType == 1) << "compressionType , 1";
			EXPECT_TRUE(activeConfig.pServerConfig->compressionThreshold == 12856) << "CompressionThreshold , 12856";
			EXPECT_TRUE(activeConfig.pServerConfig->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
			EXPECT_TRUE(activeConfig.pServerConfig->connectionMinPingTimeout == 8000) << "connectionMinPingTimeout , 8000";
			EXPECT_TRUE(activeConfig.pServerConfig->directWrite == 0) << "directWrite , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->getType() == 0) << "SocketServerConfig::getType , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serviceName == "14010") << "SocketServerConfig::serviceName , \"14002\"";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->tcpNodelay == 0) << "SocketServerConfig::tcpNodelay , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->initializationTimeout == 66) << "SocketServerConfig::initializationTimeout , 66";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serverSharedSocket == 0) << "SocketServerConfig::serverSharedSocket , 0";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile") << "loggerFileName = \"logFile\"";
			EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";

			//retrieve directory
			const DirectoryCache& dirCache = ommIProviderImpl.getDirectoryServiceStore().getDirectoryCache();
			EXPECT_TRUE(dirCache.directoryName == "Directory_1") << "directoryName = \"Directory_1\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 2) << "services.size() , 2";

			/*********retrieve first service *************/
			Service* pTemp = services.front();
			EXPECT_TRUE(pTemp) << "services.front() , true";
			EXPECT_TRUE(pTemp->serviceId == 3) << "serviceId , 3";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED") << "infoFilter.serviceName , \"DIRECT_FEED\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "company name") << "infoFilter.vendorName ,  \"company name\"";
			EXPECT_TRUE(pTemp->infoFilter.isSource == 0) << "isSource , 0";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist") << "infoFilter.itemList , \"#.itemlist\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 0) << "infoFilter.acceptingConsumerStatus , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 0) << "infoFilter.supportsQosRange , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 0) << "infoFilter.supportsOutOfBandSnapshots , 0";
			int flags = RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS |
				RDM_SVC_IFF_HAS_DICTS_PROVIDED |
				RDM_SVC_IFF_HAS_DICTS_USED |
				RDM_SVC_IFF_HAS_IS_SOURCE |
				RDM_SVC_IFF_HAS_ITEM_LIST |
				RDM_SVC_IFF_HAS_QOS |
				RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS |
				RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE |
				RDM_SVC_IFF_HAS_VENDOR;
			EXPECT_TRUE(pTemp->infoFilter.flags == flags) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_PROVIDED, RDM_SVC_IFF_HAS_DICTS_USED, RDM_SVC_IFF_HAS_IS_SOURCE, RDM_SVC_IFF_HAS_ITEM_LIST, RDM_SVC_IFF_HAS_QOS, RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS, RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE, RDM_SVC_IFF_HAS_VENDOR\"";


			//retrieve capabilities
			int idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 5) << "infoFilter.capabilities.size , 5";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 5) << "infoFilter.capabilities[0] , \"MMT_DICTIONARY\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 6) << "infoFilter.capabilities[1] , \"MMT_MARKET_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 7) << "infoFilter.capabilities[2] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 8) << "infoFilter.capabilities[3] , \"MMT_MARKET_BY_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[4] == 200) << "infoFilter.capabilities[4] , \"200\"";

			//retrieve qos
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 2) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TICK_BY_TICK) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TICK_BY_TICK\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_REALTIME) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_REALTIME\"";

			EXPECT_TRUE(pTemp->infoFilter.qos[1].rate == RSSL_QOS_RATE_TIME_CONFLATED) << "infoFilter.qos[1].rate , \"RSSL_QOS_RATE_TIME_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].rateInfo == 100) << "infoFilter.qos[1].rateInfo , 100";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeliness == RSSL_QOS_TIME_DELAYED) << "infoFilter.qos[1].timeliness , \"RSSL_QOS_TIME_DELAYED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeInfo == 100) << "infoFilter.qos[1].timeInfo , 100";

			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 2) << "infoFilter.dictionariesProvided.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[0] == "RWFFld") << "infoFilter.dictionariesProvided[0], \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[1] == "RWFEnum") << "infoFilter.dictionariesProvided[1]  , \"RWFEnum\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld_ID4") << "infoFilter.dictionariesUsed[0] , \"RWFFld_ID4\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum_ID4") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum_ID4\"";

			//retrieve dictionary defined in directory
			ServiceDictionaryConfig* pDictConfig = activeConfig.getServiceDictionaryConfig(pTemp->serviceId);
			const EmaList<DictionaryConfig*>& dictUsed3 = pDictConfig->getDictionaryUsedList();
			const EmaList<DictionaryConfig*>& dictProvided3 = pDictConfig->getDictionaryProvidedList();
			EXPECT_TRUE(dictProvided3.size() == 1) << "dictProvided.size() , 1";
			EXPECT_TRUE(dictUsed3.size() == 1) << "dictUsed.size() , 1";
			DictionaryConfig* dictionaryProvidedConfig = dictProvided3.back();
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryName == "Dictionary_3") << "dictionaryProvidedConfig->dictionaryName , \"Dictionary_3\"";
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryProvidedConfig->dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmfieldDictionaryFileName == fieldDictionaryFileNameTest) << "dictionaryProvidedConfig->rdmfieldDictionaryFileName , " << fieldDictionaryFileNameTest;
			EXPECT_TRUE(dictionaryProvidedConfig->enumtypeDefFileName == enumTableFileNameTest) << "dictionaryProvidedConfig->enumtypeDefFileName , " << enumTableFileNameTest;
			EXPECT_TRUE(dictionaryProvidedConfig->rdmFieldDictionaryItemName == "RWFFld") << "dictionaryProvidedConfig->rdmFieldDictionaryItemName , \"RWFFld\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumTypeDefItemName == "RWFEnum") << "dictionaryProvidedConfig->enumTypeDefItemName  , \"RWFEnum\"";

			DictionaryConfig* dictionaryUsedConfig = dictUsed3.back();
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryName == "Dictionary_4") << "dictionaryUsedConfig.dictionaryName , \"Dictionary_4\"";
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryUsedConfig->dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryUsedConfig->rdmFieldDictionaryItemName == "RWFFld_ID4") << "dictionaryUsedConfig->rdmfieldDictionaryItemeName , \"RWFFld_ID4\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumTypeDefItemName == "RWFEnum_ID4") << "dictionaryUsedConfig->enumtypeDefItemName  , \"RWFEnum_ID4\"";
			EXPECT_TRUE(dictionaryUsedConfig->rdmfieldDictionaryFileName == "./RDMFieldDictionary_ID4") << "dictionaryUsedConfig->rdmFieldDictionaryFileName , \"./RDMFieldDictionary_ID4\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumtypeDefFileName == "./enumtype_ID4.def") << "dictionaryUsedConfig->enumTypeDefFileName  , \"./enumtype_ID4.def\"";

			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 1) << "stateFilter.acceptingRequests , 1";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 1) << "stateFilter.serviceState , 1";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_STATUS ||
				RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_STATUS,RDM_SVC_STF_HAS_ACCEPTING_REQS\"";
			EXPECT_TRUE(pTemp->stateFilter.status.streamState == 3) << "status.streamState , \"StreamState::ClosedRecover\"";
			EXPECT_TRUE(pTemp->stateFilter.status.dataState == 2) << "stateFilter.status.dataState , \"DataState::Suspect\"";
			EXPECT_TRUE(pTemp->stateFilter.status.code == 29) << "stateFilter.status.code , \"StatusCode::DacsDown\"";
			EXPECT_TRUE(!strcmp(pTemp->stateFilter.status.text.data, "dacsDown")) << "stateFilter.status.test.data , \"dacsDown\"";
			EXPECT_TRUE(pTemp->stateFilter.status.text.length == 8) << "stateFilter.status.test.length , 8";


			EXPECT_TRUE(pTemp->loadFilter.openLimit == 1) << "loadFilter.openLimit, 1";
			EXPECT_TRUE(pTemp->loadFilter.openWindow == 2) << "loadFilter.openWindow, 2";
			EXPECT_TRUE(pTemp->loadFilter.loadFactor == 3) << "loadFilter.loadFactor, 3";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_LIMIT";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_WINDOW";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) << "loadFilter.flags, RDM_SVC_LDF_HAS_LOAD_FACTOR";

			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_INFO) << "flags, RDM_SVCF_HAS_INFO";
			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_STATE) << "flags, RDM_SVCF_HAS_STATE";
			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_LOAD) << "flags, RDM_SVCF_HAS_LOAD";
			/*********retrieve second service *************/
			pTemp = pTemp->next();
			EXPECT_TRUE(pTemp) << "pTemp->next() , true";


			EXPECT_TRUE(pTemp->serviceId == 4) << "serviceId , 4";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED1") << "infoFilter.serviceName , \"DIRECT_FEED1\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "company name") << "infoFilter.vendorName ,  \"company name\"";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist") << "infoFilter.itemList , \"#.itemlist\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 0) << "infoFilter.acceptingConsumerStatus , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 0) << "infoFilter.supportsQosRange , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 0) << "infoFilter.supportsOutOfBandSnapshots , 0";
			EXPECT_TRUE(pTemp->infoFilter.flags == RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS ||
				RDM_SVC_IFF_HAS_DICTS_PROVIDED ||
				RDM_SVC_IFF_HAS_ITEM_LIST ||
				RDM_SVC_IFF_HAS_QOS ||
				RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS ||
				RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE ||
				RDM_SVC_IFF_HAS_VENDOR) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_PROVIDED,RDM_SVC_IFF_HAS_ITEM_LIST,RDM_SVC_IFF_HAS_QOS,RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS,RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE,RDM_SVC_IFF_HAS_VENDOR\"";


			//retrieve capabilities
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 4) << "infoFilter.capabilities.size , 4";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 5) << "infoFilter.capabilities[0] , \"MMT_DICTIONARY\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 6) << "infoFilter.capabilities[1] , \"MMT_MARKET_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 7) << "infoFilter.capabilities[2] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 200) << "infoFilter.capabilities[4] , \"200\"";

			//retrieve qos
			idx = 0;
			//use default qos
			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 1) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TICK_BY_TICK) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TICK_BY_TICK\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_REALTIME) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_REALTIME\"";

			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 2) << "infoFilter.dictionariesProvided.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[0] == "RWFFld") << "infoFilter.dictionariesProvided[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[1] == "RWFEnum") << "infoFilter.dictionariesProvided[1]  , \"RWFEnum\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";


			//retrieve dictionary defined in directory
			pDictConfig = activeConfig.getServiceDictionaryConfig(pTemp->serviceId);
			const EmaList<DictionaryConfig*>& dictUsed4 = pDictConfig->getDictionaryUsedList();
			const EmaList<DictionaryConfig*>& dictProvided4 = pDictConfig->getDictionaryProvidedList();
			EXPECT_TRUE(dictProvided4.size() == 1) << "dictProvided.size() , 1";
			EXPECT_TRUE(dictUsed4.size() == 1) << "dictUsed.size() , 1";
			dictionaryProvidedConfig = dictProvided4.back();
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryName == "Dictionary_2") << "dictionaryProvidedConfig->dictionaryName , \"Dictionary_2\"";
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryProvidedConfig->dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmfieldDictionaryFileName == fieldDictionaryFileNameDefault) << "dictionaryProvidedConfig->rdmfieldDictionaryFileName = " << fieldDictionaryFileNameDefault;
			EXPECT_TRUE(dictionaryProvidedConfig->enumtypeDefFileName == enumTableFileNameDefault) << "dictionaryProvidedConfig->enumtypeDefFileName , " << enumTableFileNameDefault;
			EXPECT_TRUE(dictionaryProvidedConfig->rdmFieldDictionaryItemName == "RWFFld") << "dictionaryProvidedConfig->rdmFieldDictionaryItemName , \"RWFFld\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumTypeDefItemName == "RWFEnum") << "dictionaryProvidedConfig->enumTypeDefItemName  , \"RWFEnum\"";
			dictionaryUsedConfig = dictUsed4.back();
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryName == "Dictionary_6") << "dictionaryUsedConfig.dictionaryName , \"Dictionary_6\"";
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryUsedConfig.dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryUsedConfig->rdmfieldDictionaryFileName == fieldDictionaryFileNameDefault) << "dictionaryUsedConfig.rdmfieldDictionaryFileName , " << fieldDictionaryFileNameDefault;
			EXPECT_TRUE(dictionaryUsedConfig->enumtypeDefFileName == enumTableFileNameDefault) << "dictionaryUsedConfig.enumtypeDefFileName , " << enumTableFileNameDefault;
			EXPECT_TRUE(dictionaryUsedConfig->rdmFieldDictionaryItemName == "RWFFld") << "dictionaryUsedConfig.rdmFieldDictionaryItemName , \"RWFFld\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumTypeDefItemName == "RWFEnum") << "dictionaryUsedConfig.enumTypeDefItemName  , \"RWFEnum\"";

			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 1) << "stateFilter.acceptingRequests , 1";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 1) << "stateFilter.serviceState , 1";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_ACCEPTING_REQS\"";

			EXPECT_TRUE(pTemp->loadFilter.openLimit == 4294967295) << "loadFilter.openLimit, 4294967295";
			EXPECT_TRUE(pTemp->loadFilter.openWindow == 4294967295) << "loadFilter.openWindow, 4294967295";
			EXPECT_TRUE(pTemp->loadFilter.loadFactor == 65535) << "loadFilter.loadFactor, 65535";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_LIMIT";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_WINDOW";
			EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) << "loadFilter.flags, RDM_SVC_LDF_HAS_LOAD_FACTOR";

			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_INFO) << "flags, RDM_SVCF_HAS_INFO";
			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_STATE) << "flags, RDM_SVCF_HAS_STATE";
			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_LOAD) << "flags, RDM_SVCF_HAS_LOAD";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
	}
}

TEST_F(EmaConfigTest, testLoadingCfgFromProgrammaticConfigForNiProv)
{
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			elementList.addAscii("DefaultNiProvider", "Provider_1");

			innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
				.addAscii("Channel", "Channel_10")
				.addAscii("Logger", "Logger_1")
				.addAscii("Directory", "Directory_1")
				.addUInt("ItemCountHint", 5000)
				.addUInt("ServiceCountHint", 2000)
				.addUInt("MergeSourceDirectoryStreams", 0)
				.addUInt("RefreshFirstRequired", 0)
				.addUInt("RecoverUserSubmitSourceDirectory", 0)
				.addUInt("RemoveItemsOnDisconnect", 0)
				.addUInt("RequestTimeout", 2400)
				.addUInt("LoginRequestTimeOut", 50000)
				.addInt("ReconnectAttemptLimit", 1)
				.addInt("ReconnectMinDelay", 500)
				.addInt("ReconnectMaxDelay", 600)
				.addInt("DispatchTimeoutApiThread", 60)
				.addUInt("CatchUnhandledException", 1)
				.addUInt("MaxDispatchCountApiThread", 300)
				.addUInt("MaxDispatchCountUserThread", 700)
				.addInt("MaxEventsInPool", 400)
				.addAscii("XmlTraceFileName", "MyXMLTrace")
				.addInt("XmlTraceMaxFileSize", 50000000)
				.addUInt("XmlTraceToFile", 0)
				.addUInt("XmlTraceToStdout", 1)
				.addUInt("XmlTraceToMultipleFiles", 1)
				.addUInt("XmlTraceWrite", 1)
				.addUInt("XmlTraceRead", 1)
				.addUInt("XmlTracePing", 1)
				.addUInt("XmlTracePingOnly", 1)
				.addUInt("XmlTraceHex", 1)
				.addInt("PipePort", 13650).complete())
				.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
					.addAscii("Server", "Server_2")
					.addAscii("Directory", "Directory_2")
					.addAscii("Logger", "Logger_2").complete())
				.complete();

			elementList.addMap("NiProviderList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Channel_10", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 0)
				.addEnum("CompressionType", 1)
				.addUInt("GuaranteedOutputBuffers", 8000)
				.addUInt("NumInputBuffers", 7777)
				.addUInt("SysRecvBufSize", 150000)
				.addUInt("SysSendBufSize", 200000)
				.addUInt("CompressionThreshold", 12856)
				.addUInt("ConnectionPingTimeout", 30000)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Host", "10.0.0.1")
				.addAscii("Port", "8001")
				.addUInt("TcpNodelay", 0)
				.complete())
				.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
					.addEnum("ChannelType", 1)
					.addAscii("Host", "10.0.0.2")
					.addAscii("Port", "8002").complete())
				.complete();

			elementList.addMap("ChannelList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 1)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete())
				.addKeyAscii("Logger_1", MapEntry::AddEnum,
					ElementList()
					.addEnum("LoggerType", 0)
					.addUInt("NumberOfLogFiles", 42)
					.addUInt("MaxLogFileSize", 84000)
					.addAscii("FileName", "logFile")
					.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_4", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4")
				.addAscii("EnumTypeDefFileName", "./enumtype_ID4.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap;

			//encode service1
			serviceMap.addKeyAscii("DIRECT_FEED", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 3)
					.addAscii("Vendor", "company name")
					.addUInt("IsSource", 0)
					.addUInt("AcceptingConsumerStatus", 0)
					.addUInt("SupportsQoSRange", 0)
					.addUInt("SupportsOutOfBandSnapshots", 0)
					.addAscii("ItemList", "#.itemlist")
					.addArray("Capabilities",
						OmmArray().addUInt(5)
						.addUInt(6)
						.addUInt(7)
						.addUInt(8)
						.addUInt(200)
						.complete())
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_4")
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
					.addElementList("Status",
						ElementList().addAscii("StreamState", "StreamState::CloseRecover")
						.addAscii("DataState", "DataState::Suspect")
						.addAscii("StatusCode", "StatusCode::DacsDown")
						.addAscii("StatusText", "dacsDown")
						.complete())
					.complete())
				.complete());

			//encode service2
			serviceMap.addKeyAscii("DIRECT_FEED1", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 4)
					.addAscii("Vendor", "company name")
					.addUInt("AcceptingConsumerStatus", 0)
					.addUInt("SupportsQoSRange", 0)
					.addUInt("SupportsOutOfBandSnapshots", 0)
					.addAscii("ItemList", "#.itemlist")
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_6")
						.complete())
					.addArray("Capabilities",
						OmmArray().addAscii("MMT_DICTIONARY")
						.addAscii("MMT_MARKET_PRICE")
						.addAscii("MMT_MARKET_BY_ORDER")
						.addAscii("200")
						.complete())
					.complete())

				.addElementList("StateFilter",
					ElementList()
					.addUInt("ServiceState", 1)
					.addUInt("AcceptingRequests", 1)
					.complete())
				.complete())
				.complete();


			innerMap.addKeyAscii("Directory_1", MapEntry::AddEnum, serviceMap).complete();

			elementList.clear();
			elementList.addAscii("DefaultDirectory", "Directory_1");
			elementList.addMap("DirectoryList", innerMap).complete();
			outermostMap.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList).complete();

			EmaString localConfigPath;
			if (testCase == 1)
			{
				EmaString workingDir;
				ASSERT_EQ(getCurrentDir(workingDir), true)
					<< "Error: failed to load config file from current working dir "
					<< workingDir.c_str();
				localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);
			}

			OmmNiProviderConfig niprovConfig(localConfigPath);
			OmmNiProviderImpl ommNiProviderImpl(niprovConfig.config(outermostMap), appClient);

			OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());
			bool found = ommNiProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_10") << "Channel name , \"Channel_10\"";
			EXPECT_TRUE(activeConfig.getRefreshFirstRequired() == false) << "refreshFirstRequired , false";
			EXPECT_TRUE(activeConfig.getMergeSourceDirectoryStreams() == false) << "MergeSourceDirectoryStreams , false";
			EXPECT_TRUE(activeConfig.getRecoverUserSubmitSourceDirectory() == false) << "RecoverUserSubmitSourceDirectory , false";
			EXPECT_TRUE(activeConfig.getRemoveItemsOnDisconnect() == false) << "RemoveItemsOnDisconnect , false";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
			EXPECT_TRUE(activeConfig.itemCountHint == 5000) << "itemCountHint , 5000";
			EXPECT_TRUE(activeConfig.requestTimeout == 2400) << "requestTimeout , 2400";
			EXPECT_TRUE(activeConfig.loginRequestTimeOut == 50000) << "LoginRequestTimeout , 50000";
			EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 1) << "reconnectAttemptLimit , 1";
			EXPECT_TRUE(activeConfig.reconnectMaxDelay == 600) << "reconnectMaxDelay , 600";
			EXPECT_TRUE(activeConfig.reconnectMinDelay == 500) << "reconnectMinDelay , 500";
			EXPECT_TRUE(activeConfig.serviceCountHint == 2000) << "serviceCountHint , 2000";
			EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 60) << "dispatchTimeoutApiThread , 60";
			EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
			EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 300) << "maxDispatchCountApiThread , 300";
			EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 700) << "maxDispatchCountUserThread , 700";
			EXPECT_TRUE(activeConfig.maxEventsInPool == 400) << "MaxEventsInPool , 400";
			EXPECT_TRUE(activeConfig.xmlTraceFileName == "MyXMLTrace") << "xmlTraceFileName , \"MyXMLTrace\"";
			EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 50000000) << "xmlTraceMaxFileSize , 50000000";
			EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
			EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
			EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 1) << "xmlTraceToMultipleFiles , 1";
			EXPECT_TRUE(activeConfig.xmlTraceWrite == 1) << "xmlTraceWrite , 1";
			EXPECT_TRUE(activeConfig.xmlTraceRead == 1) << "xmlTraceRead , 1";
			EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
			EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
			EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
			EXPECT_TRUE(activeConfig.pipePort == 13650) << "pipePort , 13650";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost") << "interfaceName , \"localhost\"";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionType == 1) << "compressionType , 1";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionThreshold == 12856) << "CompressionThreshold , 12856";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->getType() == 0) << "SocketChannelConfig::getType , RSSL_SOCKET";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->hostName == "10.0.0.1") << "SocketChannelConfig::serviceName , \"10.0.0.1\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->serviceName == "8001") << "SocketChannelConfig::hostName , \"8001\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->tcpNodelay == 0) << "SocketChannelConfig::tcpNodelay , 0";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile") << "loggerFileName = \"logFile\"";
			EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";

			//retrieve directory
			const DirectoryCache& dirCache = (static_cast<OmmNiProviderDirectoryStore&>(ommNiProviderImpl.getDirectoryServiceStore())).getApiControlDirectory();
			EXPECT_TRUE(dirCache.directoryName == "Directory_1") << "directoryName = \"Directory_1\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 2) << "services.size() , 2";


			/*********retrieve first service *************/
			Service* pTemp = services.front();
			EXPECT_TRUE(pTemp) << "services.front() , true";
			EXPECT_TRUE(pTemp->serviceId == 3) << "serviceId , 3";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED") << "infoFilter.serviceName , \"DIRECT_FEED\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "company name") << "infoFilter.vendorName ,  \"company name\"";
			EXPECT_TRUE(pTemp->infoFilter.isSource == 0) << "isSource , 0";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist") << "infoFilter.itemList , \"#.itemlist\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 0) << "infoFilter.acceptingConsumerStatus , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 0) << "infoFilter.supportsQosRange , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 0) << "infoFilter.supportsOutOfBandSnapshots , 0";

			int flags = RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS |
			RDM_SVC_IFF_HAS_DICTS_USED |
			RDM_SVC_IFF_HAS_IS_SOURCE |
			RDM_SVC_IFF_HAS_ITEM_LIST |
			RDM_SVC_IFF_HAS_QOS |
			RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS |
			RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE |
			RDM_SVC_IFF_HAS_VENDOR;
			EXPECT_TRUE(pTemp->infoFilter.flags == flags) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_USED, RDM_SVC_IFF_HAS_IS_SOURCE, RDM_SVC_IFF_HAS_ITEM_LIST, RDM_SVC_IFF_HAS_QOS, RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS,RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE, RDM_SVC_IFF_HAS_VENDOR\"";

			//retrieve capabilities
			int idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 5) << "infoFilter.capabilities.size , 5";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 5) << "infoFilter.capabilities[0] , \"MMT_DICTIONARY\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 6) << "infoFilter.capabilities[1] , \"MMT_MARKET_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 7) << "infoFilter.capabilities[2] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 8) << "infoFilter.capabilities[3] , \"MMT_MARKET_BY_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[4] == 200) << "infoFilter.capabilities[4] , \"200\"";

			//retrieve qos
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 2) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TICK_BY_TICK) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TICK_BY_TICK\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_REALTIME) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_REALTIME\"";

			EXPECT_TRUE(pTemp->infoFilter.qos[1].rate == RSSL_QOS_RATE_TIME_CONFLATED) << "infoFilter.qos[1].rate , \"RSSL_QOS_RATE_TIME_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].rateInfo == 100) << "infoFilter.qos[1].rateInfo , 100";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeliness == RSSL_QOS_TIME_DELAYED) << "infoFilter.qos[1].timeliness , \"RSSL_QOS_TIME_DELAYED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeInfo == 100) << "infoFilter.qos[1].timeInfo , 100";

			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 0) << "infoFilter.dictionariesProvided.size , 0";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";

			//retrieve Service state
			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 1) << "stateFilter.acceptingRequests , 1";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 1) << "stateFilter.serviceState , 1";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_STATUS ||
				RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_STATUS,RDM_SVC_STF_HAS_ACCEPTING_REQS\"";
			EXPECT_TRUE(pTemp->stateFilter.status.streamState == 3) << "status.streamState , \"StreamState::ClosedRecover\"";
			EXPECT_TRUE(pTemp->stateFilter.status.dataState == 2) << "stateFilter.status.dataState , \"DataState::Suspect\"";
			EXPECT_TRUE(pTemp->stateFilter.status.code == 29) << "stateFilter.status.code , \"StatusCode::DacsDown\"";
			EXPECT_TRUE(!strcmp(pTemp->stateFilter.status.text.data, "dacsDown")) << "stateFilter.status.test.data , \"dacsDown\"";
			EXPECT_TRUE(pTemp->stateFilter.status.text.length == 8) << "stateFilter.status.test.length , 8";


			/*********retrieve second service *************/
			pTemp = pTemp->next();
			EXPECT_TRUE(pTemp) << "pTemp->next() , true";


			EXPECT_TRUE(pTemp->serviceId == 4) << "serviceId , 4";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED1") << "infoFilter.serviceName , \"DIRECT_FEED1\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "company name") << "infoFilter.vendorName ,  \"company name\"";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist") << "infoFilter.itemList , \"#.itemlist\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 0) << "infoFilter.acceptingConsumerStatus , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 0) << "infoFilter.supportsQosRange , 0";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 0) << "infoFilter.supportsOutOfBandSnapshots , 0";
			EXPECT_TRUE(pTemp->infoFilter.flags == RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS ||
				RDM_SVC_IFF_HAS_DICTS_PROVIDED ||
				RDM_SVC_IFF_HAS_ITEM_LIST ||
				RDM_SVC_IFF_HAS_QOS ||
				RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS ||
				RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE ||
				RDM_SVC_IFF_HAS_VENDOR) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_PROVIDED,RDM_SVC_IFF_HAS_ITEM_LIST,RDM_SVC_IFF_HAS_QOS,RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS,RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE,RDM_SVC_IFF_HAS_VENDOR\"";

			//retrieve capabilities
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 4) << "infoFilter.capabilities.size , 4";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 5) << "infoFilter.capabilities[0] , \"MMT_DICTIONARY\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 6) << "infoFilter.capabilities[1] , \"MMT_MARKET_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 7) << "infoFilter.capabilities[2] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 200) << "infoFilter.capabilities[4] , \"200\"";

			//retrieve qos
			idx = 0;
			//use default qos
			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 1) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TICK_BY_TICK) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TICK_BY_TICK\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_REALTIME) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_REALTIME\"";

			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 0) << "infoFilter.dictionariesProvided.size , 0";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";

			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 1) << "stateFilter.acceptingRequests , 1";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 1) << "stateFilter.serviceState , 1";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_ACCEPTING_REQS\"";

		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
	}
}

TEST_F(EmaConfigTest, testMergingCfgBetweenFileAndProgrammaticConfigForIProv)
{
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map configDB1, configDB2, configDB3, configDB4, configDB5, innerMap;
		ElementList elementList;
		try
		{
			innerMap.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
				.addAscii("Server", "Server_2")
				.addAscii("Logger", "Logger_2")
				.addAscii("Directory", "Directory_2")
				.addUInt("ItemCountHint", 9000)
				.addUInt("ServiceCountHint", 9000)
				.addUInt("RequestTimeout", 9000)
				.addInt("DispatchTimeoutApiThread", 5656)
				.addUInt("CatchUnhandledException", 1)
				.addUInt("MaxDispatchCountApiThread", 900)
				.addUInt("MaxDispatchCountUserThread", 900)
				.addInt("MaxEventsInPool", 1000)
				.addAscii("XmlTraceFileName", "ConfigDbXMLTrace")
				.addInt("XmlTraceMaxFileSize", 70000000)
				.addUInt("XmlTraceToFile", 0)
				.addUInt("XmlTraceToStdout", 1)
				.addUInt("XmlTraceToMultipleFiles", 0)
				.addUInt("XmlTraceWrite", 0)
				.addUInt("XmlTraceRead", 0)
				.addUInt("XmlTracePing", 1)
				.addUInt("XmlTracePingOnly", 1)
				.addUInt("XmlTraceHex", 1)
				.addInt("PipePort", 9696)
				.addUInt("RefreshFirstRequired", 0)
				.addUInt("EnforceAckIDValidation", 0)
				.addUInt("AcceptDirMessageWithoutMinFilters", 1)
				.addUInt("AcceptMessageSameKeyButDiffStream", 1)
				.addUInt("AcceptMessageThatChangesService", 1)
				.addUInt("AcceptMessageWithoutAcceptingRequests", 1)
				.addUInt("AcceptMessageWithoutBeingLogin", 1)
				.addUInt("AcceptMessageWithoutQosInRange", 1)
				.addUInt("FieldDictionaryFragmentSize", 2000)
				.addUInt("EnumTypeFragmentSize", 1000)
				.complete())
				.complete();

			elementList.addMap("IProviderList", innerMap).complete();
			innerMap.clear();

			configDB1.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			innerMap.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
				.addEnum("ServerType", 0)
				.addAscii("InterfaceName", "localhost")
				.addEnum("CompressionType", 2)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addUInt("NumInputBuffers", 888888)
				.addUInt("SysRecvBufSize", 550000)
				.addUInt("SysSendBufSize", 700000)
				.addUInt("CompressionThreshold", 12758)
				.addUInt("ConnectionPingTimeout", 70000)
				.addUInt("ConnectionMinPingTimeout", 4000)
				.addAscii("Port", "8003")
				.addUInt("TcpNodelay", 0)
				.addUInt("InitializationTimeout", 77)
				.complete())
				.complete();

			elementList.addMap("ServerList", innerMap).complete();
			innerMap.clear();

			configDB2.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 0)
				.addAscii("FileName", "ConfigDB2_logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 4).complete())
				.complete();

			elementList.addMap("LoggerList", innerMap).complete();
			innerMap.clear();

			configDB3.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 1)
			.addAscii("RdmFieldDictionaryItemName", "./ConfigDB3_RWFFld")
			.addAscii("EnumTypeDefItemName", "./ConfigDB3_RWFEnum")
			.addAscii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap).complete();
			innerMap.clear();

			configDB4.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap;

			//encode service1
			serviceMap.addKeyAscii("DIRECT_FEED", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 1)
					.addAscii("Vendor", "Vendor")
					.addUInt("IsSource", 1)
					.addUInt("AcceptingConsumerStatus", 1)
					.addUInt("SupportsQoSRange", 1)
					.addUInt("SupportsOutOfBandSnapshots", 1)
					.addAscii("ItemList", "#.itemlist2")
					.addArray("Capabilities",
						OmmArray().addAscii("8")
						.addAscii("9")
						.addAscii("MMT_MARKET_BY_ORDER")
						.addAscii("130")
						.complete())
					.addArray("DictionariesProvided",
						OmmArray().addAscii("Dictionary_2")
						.complete())
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_3")
						.complete())
					.addSeries("QoS",
						Series()
						.add(
							ElementList().addUInt("Timeliness", 200)
							.addUInt("Rate", 200)
							.complete())
						.add(
							ElementList().addAscii("Timeliness", "Timeliness::InexactDelayed")
							.addAscii("Rate", "Rate::JustInTimeConflated")
							.complete())
						.complete())
					.complete())

				.addElementList("StateFilter",
					ElementList().addUInt("ServiceState", 0)
					.addUInt("AcceptingRequests", 0)
					.addElementList("Status",
						ElementList().addAscii("StreamState", "StreamState::CloseRecover")
						.addAscii("DataState", "DataState::Suspect")
						.addAscii("StatusCode", "StatusCode::DacsDown")
						.addAscii("StatusText", "dacsDown")
						.complete())
					.complete())
				.complete()).complete();

			innerMap.addKeyAscii("Directory_2", MapEntry::AddEnum, serviceMap).complete();

			elementList.clear();
			elementList.addMap("DirectoryList", innerMap).complete();
			configDB5.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList).complete();

			EmaString fieldDictionaryFileNm;
			EmaString enumTableFileNm;

			EmaString localConfigPath;
			if (testCase == 0)
			{
				fieldDictionaryFileNm.set(fieldDictionaryFileNameDefault);
				enumTableFileNm.set(enumTableFileNameDefault);
			}
			else if (testCase == 1)
			{
				EmaString workingDir;
				ASSERT_EQ(getCurrentDir(workingDir), true)
					<< "Error: failed to load config file from current working dir "
					<< workingDir.c_str();
				localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

				fieldDictionaryFileNm.set(fieldDictionaryFileNameTest);
				enumTableFileNm.set(enumTableFileNameTest);
			}

			OmmIProviderConfig iprovConfig(localConfigPath);
			OmmIProviderImpl ommIProviderImpl(iprovConfig.config(configDB1).config(configDB2).config(configDB3).config(configDB4).config(configDB5).providerName("Provider_2"), appClient);

			OmmIProviderActiveConfig& activeConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
			bool found = ommIProviderImpl.getInstanceName().find("Provider_2") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_2_1\"";
			EXPECT_TRUE(activeConfig.pServerConfig->name == "Server_2") << "Server name , \"Server_2\"";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_2") << "Logger name , \"Logger_2\"";
			EXPECT_TRUE(activeConfig.itemCountHint == 9000) << "itemCountHint , 9000";
			EXPECT_TRUE(activeConfig.serviceCountHint == 9000) << "serviceCountHint , 9000";
			EXPECT_TRUE(activeConfig.requestTimeout == 9000) << "requestTimeout , 9000";
			EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 5656) << "dispatchTimeoutApiThread , 5656";
			EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
			EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 900) << "maxDispatchCountApiThread , 900";
			EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 900) << "maxDispatchCountUserThread , 900";
			EXPECT_TRUE(activeConfig.maxEventsInPool == 1000) << "maxEventsInPool , 1000";
			EXPECT_TRUE(activeConfig.pipePort == 9696) << "pipePort , 9696";
			EXPECT_TRUE(activeConfig.xmlTraceFileName == "ConfigDbXMLTrace") << "xmlTraceFileName , \"ConfigDbXMLTrace\"";
			EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 70000000) << "xmlTraceMaxFileSize , 70000000";
			EXPECT_TRUE(activeConfig.xmlTraceToFile == 0) << "xmlTraceToFile , 0";
			EXPECT_TRUE(activeConfig.xmlTraceToStdout == 1) << "xmlTraceToStdout , 1";
			EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 0) << "xmlTraceToMultipleFiles , 0";
			EXPECT_TRUE(activeConfig.xmlTraceWrite == 0) << "xmlTraceWrite , 0";
			EXPECT_TRUE(activeConfig.xmlTraceRead == 0) << "xmlTraceRead , 0";
			EXPECT_TRUE(activeConfig.xmlTracePing == 1) << "xmlTracePing , 1";
			EXPECT_TRUE(activeConfig.xmlTracePingOnly == 1) << "xmlTracePingOnly , 1";
			EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
			EXPECT_TRUE(activeConfig.getRefreshFirstRequired() == false) << "refreshFirstRequired , false";
			EXPECT_TRUE(activeConfig.getEnforceAckIDValidation() == false) << "enforceAckIDValidation , false";
			EXPECT_TRUE(activeConfig.acceptDirMessageWithoutMinFilters == true) << "acceptDirMessageWithoutMinFilters , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutAcceptingRequests == true) << "acceptMessageWithoutAcceptingRequests , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutBeingLogin == true) << "acceptMessageWithoutBeingLogin , true";
			EXPECT_TRUE(activeConfig.acceptMessageWithoutQosInRange == true) << "acceptMessageWithoutQosInRange , true";
			EXPECT_TRUE(activeConfig.acceptMessageSameKeyButDiffStream == true) << "acceptMessageSameKeyButDiffStream , true";
			EXPECT_TRUE(activeConfig.acceptMessageThatChangesService == true) << "acceptMessageThatChangesService , true";
			EXPECT_TRUE(activeConfig.getMaxFieldDictFragmentSize() == 2000) << "maxFieldDictFragmentSize , 2000";
			EXPECT_TRUE(activeConfig.getMaxEnumTypeFragmentSize() == 1000) << "maxEnumTypeFragmentSize , 1000";

			EXPECT_TRUE(activeConfig.pServerConfig->interfaceName == "localhost") << "interfaceName , \"localhost\"";
			EXPECT_TRUE(activeConfig.pServerConfig->compressionType == 2) << "compressionType , 2";
			EXPECT_TRUE(activeConfig.pServerConfig->guaranteedOutputBuffers == 7000) << "guaranteedOutputBuffers , 7000";
			EXPECT_TRUE(activeConfig.pServerConfig->numInputBuffers == 888888) << "numInputBuffers , 888888";
			EXPECT_TRUE(activeConfig.pServerConfig->sysRecvBufSize == 550000) << "sysRecvBufSize , 550000";
			EXPECT_TRUE(activeConfig.pServerConfig->sysSendBufSize == 700000) << "sysSendBufSize , 700000";
			EXPECT_TRUE(activeConfig.pServerConfig->compressionThreshold == 12758) << "compressionThreshold , compressionThreshold";
			EXPECT_TRUE(activeConfig.pServerConfig->connectionPingTimeout == 70000) << "connectionPingTimeout , 70000";
			EXPECT_TRUE(activeConfig.pServerConfig->connectionMinPingTimeout == 4000) << "connectionMinPingTimeout , 4000";
			EXPECT_TRUE(activeConfig.pServerConfig->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ServerType::RSSL_SOCKET";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serviceName == "8003") << "SocketServerConfig::hostName , \"8003\"";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->tcpNodelay == 0) << "SocketServerConfig::tcpNodelay , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->initializationTimeout == 77) << "SocketServerConfig::initializationTimeout , 77";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "ConfigDB2_logFile") << "loggerFileName = \"ConfigDB2_logFile\"";
			EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::NoLogMsgEnum) << "minLoggerSeverity = OmmLoggerClient::NoLogMsgEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";

			//retrieve directory
			const DirectoryCache& dirCache = ommIProviderImpl.getDirectoryServiceStore().getDirectoryCache();
			EXPECT_TRUE(dirCache.directoryName == "Directory_2") << "directoryName = \"Directory_2\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 1) << "services.size() , 1";

			/*********retrieve first service *************/
			Service* pTemp = services.front();
			EXPECT_TRUE(pTemp) << "services.front() , true";
			EXPECT_TRUE(pTemp->serviceId == 1) << "serviceId , 1";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED") << "infoFilter.serviceName , \"DIRECT_FEED\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "Vendor") << "infoFilter.vendorName ,  \"Vendor\"";
			EXPECT_TRUE(pTemp->infoFilter.isSource == 1) << "isSource , 1";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist2") << "infoFilter.itemList , \"#.itemlist2\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 1) << "infoFilter.acceptingConsumerStatus , 1";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 1) << "infoFilter.supportsQosRange , 1";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 1) << "infoFilter.supportsOutOfBandSnapshots , 1";

			int flags = RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS |
				RDM_SVC_IFF_HAS_DICTS_PROVIDED |
				RDM_SVC_IFF_HAS_DICTS_USED |
				RDM_SVC_IFF_HAS_IS_SOURCE |
				RDM_SVC_IFF_HAS_ITEM_LIST |
				RDM_SVC_IFF_HAS_QOS |
				RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS |
				RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE |
				RDM_SVC_IFF_HAS_VENDOR;
			EXPECT_TRUE(pTemp->infoFilter.flags == flags) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_PROVIDED, RDM_SVC_IFF_HAS_DICTS_USED, RDM_SVC_IFF_HAS_IS_SOURCE, RDM_SVC_IFF_HAS_ITEM_LIST, RDM_SVC_IFF_HAS_QOS, RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS, RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE, RDM_SVC_IFF_HAS_VENDOR\"";

			//retrieve capabilities
			int idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 4) << "infoFilter.capabilities.size , 4";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 7) << "infoFilter.capabilities[0] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 8) << "infoFilter.capabilities[1] , \"MMT_MARKET_BY_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 9) << "infoFilter.capabilities[2] , \"MMT_MARKET_MAKER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 130) << "infoFilter.capabilities[3] , \"130\"";

			//retrieve qos
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TIME_CONFLATED) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TIME_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rateInfo == 200) << "infoFilter.qos[1].rateInfo , 200";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_DELAYED) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_DELAYED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeInfo == 200) << "infoFilter.qos[1].timeInfo , 200";

			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 2) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].rate == RSSL_QOS_RATE_JIT_CONFLATED) << "infoFilter.qos[1].rate , \"RSSL_QOS_RATE_JIT_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeliness == RSSL_QOS_TIME_DELAYED_UNKNOWN) << "infoFilter.qos[1].timeliness , \"RSSL_QOS_TIME_DELAYED_UNKNOWN\"";


			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 2) << "infoFilter.dictionariesProvided.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[0] == "./ConfigDB3_RWFFld") << "infoFilter.dictionariesProvided[0], \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[1] == "./ConfigDB3_RWFEnum") << "infoFilter.dictionariesProvided[1]  , \"./ConfigDB3_RWFEnum\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";

			//retrieve dictionary defined in directory
			ServiceDictionaryConfig* pDictConfig = activeConfig.getServiceDictionaryConfig(pTemp->serviceId);
			const EmaList<DictionaryConfig*>& dictUsed3 = pDictConfig->getDictionaryUsedList();
			const EmaList<DictionaryConfig*>& dictProvided3 = pDictConfig->getDictionaryProvidedList();
			EXPECT_TRUE(dictProvided3.size() == 1) << "dictProvided.size() , 1";
			EXPECT_TRUE(dictUsed3.size() == 1) << "dictUsed.size() , 1";
			DictionaryConfig* dictionaryProvidedConfig = dictProvided3.back();
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryName == "Dictionary_2") << "dictionaryProvidedConfig->dictionaryName , \"Dictionary_2\"";
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryProvidedConfig->dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmfieldDictionaryFileName == "./ConfigDB3_RDMFieldDictionary") << "dictionaryProvidedConfig->rdmfieldDictionaryFileName , \"./ConfigDB3_RDMFieldDictionary\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumtypeDefFileName == "./ConfigDB3_enumtype.def") << "dictionaryProvidedConfig->enumtypeDefFileName  , \"./ConfigDB3_enumtype.def\"";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmFieldDictionaryItemName == "./ConfigDB3_RWFFld") << "dictionaryProvidedConfig->rdmFieldDictionaryItemName , \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumTypeDefItemName == "./ConfigDB3_RWFEnum") << "dictionaryProvidedConfig->enumTypeDefItemName  , \"./ConfigDB3_RWFEnum\"";

			DictionaryConfig* dictionaryUsedConfig = dictUsed3.back();
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryName == "Dictionary_3") << "dictionaryUsedConfig->dictionaryName , \"Dictionary_3\"";
			EXPECT_TRUE(dictionaryUsedConfig->rdmfieldDictionaryFileName == fieldDictionaryFileNm) << "dictionaryUsedConfig->rdmfieldDictionaryFileName , " << fieldDictionaryFileNm;
			EXPECT_TRUE(dictionaryUsedConfig->enumtypeDefFileName == enumTableFileNm) << "dictionaryUsedConfig->enumtypeDefFileName , " << enumTableFileNm;
			EXPECT_TRUE(dictionaryUsedConfig->rdmFieldDictionaryItemName == "RWFFld") << "dictionaryUsedConfig->rdmFieldDictionaryItemName , \"RWFFld\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumTypeDefItemName == "RWFEnum") << "dictionaryUsedConfig->enumTypeDefItemName  , \"RWFEnum\"";

			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 0) << "stateFilter.acceptingRequests , 0";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 0) << "stateFilter.serviceState , 0";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_STATUS ||
				RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_STATUS,RDM_SVC_STF_HAS_ACCEPTING_REQS\"";
			EXPECT_TRUE(pTemp->stateFilter.status.streamState == 3) << "status.streamState , \"StreamState::ClosedRecover\"";
			EXPECT_TRUE(pTemp->stateFilter.status.dataState == 2) << "stateFilter.status.dataState , \"DataState::Suspect\"";
			EXPECT_TRUE(pTemp->stateFilter.status.code == 29) << "stateFilter.status.code , \"StatusCode::DacsDown\"";
			EXPECT_TRUE(!strcmp(pTemp->stateFilter.status.text.data, "dacsDown")) << "stateFilter.status.test.data , \"dacsDown\"";
			EXPECT_TRUE(pTemp->stateFilter.status.text.length == 8) << "stateFilter.status.test.length , 8";


			/*********there is no second service *************/
			pTemp = pTemp->next();
			EXPECT_TRUE(pTemp == 0) << "pTemp->next() , 0";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
	}
}
TEST_F(EmaConfigTest, testMergingCfgBetweenFileAndProgrammaticConfigNiProv)
{
	//two testcases:
	//test case 1: NOT loading EmaConfig file from working dir.
	//test case 2: loading EmaConfigTest file
	for (int testCase = 0; testCase < 2; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			innerMap.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
				.addAscii("Channel", "Channel_2")
				.addAscii("Logger", "Logger_2")
				.addAscii("Directory", "Directory_2")
				.addUInt("ItemCountHint", 9000)
				.addUInt("ServiceCountHint", 9000)
				.addUInt("RequestTimeout", 9000)
				.addInt("DispatchTimeoutApiThread", 5656)
				.addUInt("CatchUnhandledException", 1)
				.addUInt("MaxDispatchCountApiThread", 900)
				.addUInt("MaxDispatchCountUserThread", 900)
				.addInt("MaxEventsInPool", 500)
				.addAscii("XmlTraceFileName", "ConfigDbXMLTrace")
				.addInt("XmlTraceMaxFileSize", 70000000)
				.addUInt("XmlTraceToFile", 1)
				.addUInt("XmlTraceToStdout", 0)
				.addUInt("XmlTraceToMultipleFiles", 0)
				.addUInt("XmlTraceWrite", 0)
				.addUInt("XmlTraceRead", 0)
				.addUInt("XmlTracePing", 0)
				.addUInt("XmlTracePingOnly", 0)
				.addUInt("XmlTraceHex", 0)
				.addInt("ReconnectAttemptLimit", 70)
				.addInt("ReconnectMinDelay", 7000)
				.addInt("ReconnectMaxDelay", 7000)
				.addInt("PipePort", 9696)
				.addUInt("MergeSourceDirectoryStreams", 0)
				.addUInt("RefreshFirstRequired", 0)
				.addUInt("RecoverUserSubmitSourceDirectory", 0)
				.addUInt("RemoveItemsOnDisconnect", 0)
				.addUInt("LoginRequestTimeOut", 50000).complete())
				.complete();

			elementList.addMap("NiProviderList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 0)
				.addAscii("InterfaceName", "localhost")
				.addEnum("CompressionType", 2)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addUInt("NumInputBuffers", 888888)
				.addUInt("SysRecvBufSize", 550000)
				.addUInt("SysSendBufSize", 700000)
				.addUInt("CompressionThreshold", 12758)
				.addUInt("ConnectionPingTimeout", 70000)
				.addAscii("Host", "10.0.0.1")
				.addAscii("Port", "8001")
				.addUInt("TcpNodelay", 1)
				.complete())
				.complete();

			elementList.addMap("ChannelList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 0)
				.addAscii("FileName", "ConfigDB2_logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 4).complete())
				.complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			/////////////////////////////////////
			//DirectoryGroup
			Map serviceMap;

			//encode service1
			serviceMap.addKeyAscii("DIRECT_FEED", MapEntry::AddEnum,
				ElementList()
				.addElementList("InfoFilter",
					ElementList().addUInt("ServiceId", 3)
					.addAscii("Vendor", "Vendor")
					.addUInt("IsSource", 1)
					.addUInt("AcceptingConsumerStatus", 1)
					.addUInt("SupportsQoSRange", 1)
					.addUInt("SupportsOutOfBandSnapshots", 1)
					.addAscii("ItemList", "#.itemlist2")
					.addArray("Capabilities",
						OmmArray().addAscii("8")
						.addAscii("9")
						.addAscii("MMT_MARKET_BY_ORDER")
						.addAscii("130")
						.complete())
					.addArray("DictionariesUsed",
						OmmArray().addAscii("Dictionary_2")
						.complete())
					.addSeries("QoS",
						Series()
						.add(
							ElementList().addUInt("Timeliness", 200)
							.addUInt("Rate", 200)
							.complete())
						.add(
							ElementList().addAscii("Timeliness", "Timeliness::InexactDelayed")
							.addAscii("Rate", "Rate::JustInTimeConflated")
							.complete())
						.complete())
					.complete())

				.addElementList("StateFilter",
					ElementList().addUInt("ServiceState", 0)
					.addUInt("AcceptingRequests", 0)
					.addElementList("Status",
						ElementList().addAscii("StreamState", "StreamState::CloseRecover")
						.addAscii("DataState", "DataState::Suspect")
						.addAscii("StatusCode", "StatusCode::DacsDown")
						.addAscii("StatusText", "dacsDown")
						.complete())
					.complete())
				.complete()).complete();

			innerMap.addKeyAscii("Directory_2", MapEntry::AddEnum, serviceMap).complete();

			elementList.clear();
			elementList.addMap("DirectoryList", innerMap).complete();
			outermostMap.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList).complete();

			EmaString localConfigPath;
			if (testCase == 1)
			{
				EmaString workingDir;
				ASSERT_EQ(getCurrentDir(workingDir), true)
					<< "Error: failed to load config file from current working dir "
					<< workingDir.c_str();
				localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);
			}

			OmmNiProviderConfig niprovConfig(localConfigPath);
			OmmNiProviderImpl ommNiProviderImpl(niprovConfig.config(outermostMap).providerName("Provider_2"), appClient);

			OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());
			bool found = ommNiProviderImpl.getInstanceName().find("Provider_2") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_2_1\"";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_2") << "Channel name , \"Channel_2\"";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_2") << "Logger name , \"Logger_2\"";
			EXPECT_TRUE(activeConfig.itemCountHint == 9000) << "itemCountHint , 9000";
			EXPECT_TRUE(activeConfig.serviceCountHint == 9000) << "serviceCountHint , 9000";
			EXPECT_TRUE(activeConfig.requestTimeout == 9000) << "requestTimeout , 9000";
			EXPECT_TRUE(activeConfig.dispatchTimeoutApiThread == 5656) << "dispatchTimeoutApiThread , 5656";
			EXPECT_TRUE(activeConfig.catchUnhandledException == 1) << "catchUnhandledException , 1";
			EXPECT_TRUE(activeConfig.maxDispatchCountApiThread == 900) << "maxDispatchCountApiThread , 900";
			EXPECT_TRUE(activeConfig.maxDispatchCountUserThread == 900) << "maxDispatchCountUserThread , 900";
			EXPECT_TRUE(activeConfig.maxEventsInPool == 500) << "MaxEventsInPool , 500";
			EXPECT_TRUE(activeConfig.pipePort == 9696) << "pipePort , 9696";
			EXPECT_TRUE(activeConfig.reconnectAttemptLimit == 70) << "reconnectAttemptLimit , 70";
			EXPECT_TRUE(activeConfig.reconnectMinDelay == 7000) << "reconnectMinDelay , 7000";
			EXPECT_TRUE(activeConfig.reconnectMaxDelay == 7000) << "reconnectMaxDelay , 7000";
			EXPECT_TRUE(activeConfig.xmlTraceFileName == "ConfigDbXMLTrace") << "xmlTraceFileName , \"ConfigDbXMLTrace\"";
			EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 70000000) << "xmlTraceMaxFileSize , 70000000";
			EXPECT_TRUE(activeConfig.xmlTraceToFile == 1) << "xmlTraceToFile , 1";
			EXPECT_TRUE(activeConfig.xmlTraceToStdout == 0) << "xmlTraceToStdout , 0";
			EXPECT_TRUE(activeConfig.xmlTraceToMultipleFiles == 0) << "xmlTraceToMultipleFiles , 0";
			EXPECT_TRUE(activeConfig.xmlTraceWrite == 0) << "xmlTraceWrite , 0";
			EXPECT_TRUE(activeConfig.xmlTraceRead == 0) << "xmlTraceRead , 0";
			EXPECT_TRUE(activeConfig.xmlTracePing == 0) << "xmlTracePing , 0";
			EXPECT_TRUE(activeConfig.xmlTracePingOnly == 0) << "xmlTracePingOnly , 0";
			EXPECT_TRUE(activeConfig.xmlTraceHex == 0) << "xmlTraceHex , 0";
			EXPECT_TRUE(activeConfig.getRefreshFirstRequired() == false) << "refreshFirstRequired , false";
			EXPECT_TRUE(activeConfig.getMergeSourceDirectoryStreams() == false) << "MergeSourceDirectoryStreams , false";
			EXPECT_TRUE(activeConfig.getRecoverUserSubmitSourceDirectory() == false) << "RecoverUserSubmitSourceDirectory , false";
			EXPECT_TRUE(activeConfig.getRemoveItemsOnDisconnect() == false) << "RemoveItemsOnDisconnect , false";

			EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost") << "interfaceName , \"localhost\"";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionType == 2) << "compressionType , 2";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 7000) << "guaranteedOutputBuffers , 7000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 888888) << "numInputBuffers , 888888";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 550000) << "sysRecvBufSize , 550000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 700000) << "sysSendBufSize , 700000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionThreshold == 12758) << "compressionThreshold , compressionThreshold";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 70000) << "connectionPingTimeout , 70000";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->hostName == "10.0.0.1") << "SocketChannelConfig::serviceName , \"10.0.0.1\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->serviceName == "8001") << "SocketChannelConfig::hostName , \"8001\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->tcpNodelay == 1) << "SocketChannelConfig::tcpNodelay , 1";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "ConfigDB2_logFile") << "loggerFileName = \"ConfigDB2_logFile\"";
			EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::NoLogMsgEnum) << "minLoggerSeverity = OmmLoggerClient::NoLogMsgEnum";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileNumber == 42) << "maxFileNumber = 42";
			EXPECT_TRUE(activeConfig.loggerConfig.maxFileSize == 84000) << "maxFileSize = 84000";

			//retrieve directory
			const DirectoryCache& dirCache = (static_cast<OmmNiProviderDirectoryStore&>(ommNiProviderImpl.getDirectoryServiceStore())).getApiControlDirectory();
			EXPECT_TRUE(dirCache.directoryName == "Directory_2") << "directoryName = \"Directory_2\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 1) << "services.size() , 1";

			/*********retrieve first service *************/
			Service* pTemp = services.front();
			EXPECT_TRUE(pTemp) << "services.front() , true";
			EXPECT_TRUE(pTemp->serviceId == 3) << "serviceId , 3";
			EXPECT_TRUE(pTemp->infoFilter.serviceName == "DIRECT_FEED") << "infoFilter.serviceName , \"DIRECT_FEED\"";
			EXPECT_TRUE(pTemp->infoFilter.vendorName == "Vendor") << "infoFilter.vendorName ,  \"Vendor\"";
			EXPECT_TRUE(pTemp->infoFilter.isSource == 1) << "isSource , 1";
			EXPECT_TRUE(pTemp->infoFilter.itemList == "#.itemlist2") << "infoFilter.itemList , \"#.itemlist2\"";
			EXPECT_TRUE(pTemp->infoFilter.acceptingConsumerStatus == 1) << "infoFilter.acceptingConsumerStatus , 1";
			EXPECT_TRUE(pTemp->infoFilter.supportsQosRange == 1) << "infoFilter.supportsQosRange , 1";
			EXPECT_TRUE(pTemp->infoFilter.supportsOutOfBandSnapshots == 1) << "infoFilter.supportsOutOfBandSnapshots , 1";
			
			int flags = RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS |
				RDM_SVC_IFF_HAS_DICTS_USED |
				RDM_SVC_IFF_HAS_IS_SOURCE |
				RDM_SVC_IFF_HAS_ITEM_LIST |
				RDM_SVC_IFF_HAS_QOS |
				RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS |
				RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE |
				RDM_SVC_IFF_HAS_VENDOR;

			if (testCase == 0)
				EXPECT_TRUE(pTemp->infoFilter.flags == flags) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_USED, RDM_SVC_IFF_HAS_IS_SOURCE, RDM_SVC_IFF_HAS_ITEM_LIST, RDM_SVC_IFF_HAS_QOS, RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS,RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE, RDM_SVC_IFF_HAS_VENDOR\"";
			else if (testCase == 1)
			{   //there is one dictProvided from config file.
				flags |= RDM_SVC_IFF_HAS_DICTS_PROVIDED;
				EXPECT_TRUE(pTemp->infoFilter.flags == flags) << "infoFilter.flags , \"RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS, RDM_SVC_IFF_HAS_DICTS_PROVIDED, RDM_SVC_IFF_HAS_DICTS_USED, RDM_SVC_IFF_HAS_IS_SOURCE, RDM_SVC_IFF_HAS_ITEM_LIST, RDM_SVC_IFF_HAS_QOS, RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS, RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE, RDM_SVC_IFF_HAS_VENDOR\"";
			}

			//retrieve capabilities
			int idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.capabilities.size() == 4) << "infoFilter.capabilities.size , 4";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[0] == 7) << "infoFilter.capabilities[0] , \"MMT_MARKET_BY_ORDER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[1] == 8) << "infoFilter.capabilities[1] , \"MMT_MARKET_BY_PRICE\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[2] == 9) << "infoFilter.capabilities[2] , \"MMT_MARKET_MAKER\"";
			EXPECT_TRUE(pTemp->infoFilter.capabilities[3] == 130) << "infoFilter.capabilities[3] , \"130\"";

			//retrieve qos
			idx = 0;
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rate == RSSL_QOS_RATE_TIME_CONFLATED) << "infoFilter.qos[0].rate , \"RSSL_QOS_RATE_TIME_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].rateInfo == 200) << "infoFilter.qos[1].rateInfo , 200";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeliness == RSSL_QOS_TIME_DELAYED) << "infoFilter.qos[0].timeliness , \"RSSL_QOS_TIME_DELAYED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[0].timeInfo == 200) << "infoFilter.qos[1].timeInfo , 200";

			EXPECT_TRUE(pTemp->infoFilter.qos.size() == 2) << "infoFilter.qos.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].rate == RSSL_QOS_RATE_JIT_CONFLATED) << "infoFilter.qos[1].rate , \"RSSL_QOS_RATE_JIT_CONFLATED\"";
			EXPECT_TRUE(pTemp->infoFilter.qos[1].timeliness == RSSL_QOS_TIME_DELAYED_UNKNOWN) << "infoFilter.qos[1].timeliness , \"RSSL_QOS_TIME_DELAYED_UNKNOWN\"";

			
			//retrieve dictionary provided/used by this service
			if (testCase == 0)
			{
				EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 0) << "infoFilter.dictionariesProvided.size , 0";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";
			}
			else if (testCase == 1)
			{
				//there is one dictionariesProvided config from file.
				EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 2) << "infoFilter.dictionariesProvided.size , 2";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[0] == "RWFFld") << "infoFilter.dictionariesProvided[0], \"RWFFld\"";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[1] == "RWFEnum") << "infoFilter.dictionariesProvided[1]  , \"RWFEnum\"";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "RWFFld") << "infoFilter.dictionariesUsed[0] , \"RWFFld\"";
				EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"RWFEnum\"";
			}

			EXPECT_TRUE(pTemp->stateFilter.acceptingRequests == 0) << "stateFilter.acceptingRequests , 0";
			EXPECT_TRUE(pTemp->stateFilter.serviceState == 0) << "stateFilter.serviceState , 0";
			EXPECT_TRUE(pTemp->stateFilter.flags == RDM_SVC_STF_HAS_STATUS ||
				RDM_SVC_STF_HAS_ACCEPTING_REQS) << "stateFilter.flags , \"RDM_SVC_STF_HAS_STATUS,RDM_SVC_STF_HAS_ACCEPTING_REQS\"";
			EXPECT_TRUE(pTemp->stateFilter.status.streamState == 3) << "status.streamState , \"StreamState::ClosedRecover\"";
			EXPECT_TRUE(pTemp->stateFilter.status.dataState == 2) << "stateFilter.status.dataState , \"DataState::Suspect\"";
			EXPECT_TRUE(pTemp->stateFilter.status.code == 29) << "stateFilter.status.code , \"StatusCode::DacsDown\"";
			EXPECT_TRUE(!strcmp(pTemp->stateFilter.status.text.data, "dacsDown")) << "stateFilter.status.test.data , \"dacsDown\"";
			EXPECT_TRUE(pTemp->stateFilter.status.text.length == 8) << "stateFilter.status.test.length , 8";


			/*********there is no second service *************/
			pTemp = pTemp->next();
			EXPECT_TRUE(pTemp == 0) << "pTemp->next() , 0";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
	}
}

TEST_F(EmaConfigTest, testSetInstanceNameByFunctionCall)
{
	//three testcases:
	//test case 1: set niprovideName through function call.
	//test case 2: set iprovideName through function call.
	//test case 3: set consumerName through function call.
	for (int testCase = 0; testCase < 3; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			if (testCase == 0)
			{
				elementList.addAscii("DefaultNiProvider", "Provider_5");
				innerMap.addKeyAscii("Provider_5", MapEntry::AddEnum, ElementList()
					.addAscii("Channel", "Channel_5")
					.addAscii("Logger", "Logger_5")
					.addAscii("Directory", "Directory_5")
					.addUInt("LoginRequestTimeOut", 50000).complete())
					.complete();

				elementList.addMap("NiProviderList", innerMap);
				elementList.complete();
				innerMap.clear();

				outermostMap.addKeyAscii("NiProviderGroup", MapEntry::AddEnum, elementList);
				elementList.clear();

			}
			else if (testCase == 1)
			{
				elementList.addAscii("DefaultIProvider", "Provider_5");
				innerMap.addKeyAscii("Provider_5", MapEntry::AddEnum, ElementList()
					.addAscii("Server", "Server_5")
					.addAscii("Logger", "Logger_5")
					.addAscii("Directory", "Directory_5")
					.complete())
					.complete();

				elementList.addMap("IProviderList", innerMap);
				elementList.complete();
				innerMap.clear();

				outermostMap.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList);
				elementList.clear();
			}
			else if (testCase == 2)
			{
				elementList.addAscii("DefaultConsumer", "Consumer_5");
				innerMap.addKeyAscii("Consumer_5", MapEntry::AddEnum, ElementList()
					.addAscii("Channel", "Channel_5")
					.addAscii("Logger", "Logger_5")
					.addUInt("LoginRequestTimeOut", 50000).complete())
					.complete();

				elementList.addMap("ConsumerList", innerMap);
				elementList.complete();
				innerMap.clear();

				outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
				elementList.clear();

			}

			innerMap.addKeyAscii("Channel_5", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 0)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addAscii("Host", "host5")
				.addAscii("Port", "port5")
				.addUInt("TcpNodelay", 1)
				.complete())
				.complete();

			elementList.addMap("ChannelList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Server_5", MapEntry::AddEnum, ElementList()
				.addEnum("ServerType", 0)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addAscii("Port", "port5")
				.addUInt("TcpNodelay", 1)
				.complete())
				.complete();

			elementList.addMap("ServerList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			EmaString localConfigPath;
			EmaString workingDir;
			ASSERT_EQ(getCurrentDir(workingDir), true)
					<< "Error: failed to load config file from current working dir "
					<< workingDir.c_str();
			localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			if (testCase == 0)
			{
				OmmNiProviderConfig niprovConfig(localConfigPath);
				OmmNiProviderImpl ommNiProviderImpl(niprovConfig.config(outermostMap).providerName("Provider_1"), appClient);

				OmmNiProviderActiveConfig& activeConfig = static_cast<OmmNiProviderActiveConfig&>(ommNiProviderImpl.getActiveConfig());
				bool found = ommNiProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
				EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_10") << "Channel name , \"Channel_10\"";
				EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->hostName == "localhost") << "SocketChannelConfig::serviceName , \"localhost\"";
				EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->serviceName == "14003") << "SocketChannelConfig::hostName , \"14003\"";
			}
			else if (testCase == 1)
			{
				OmmIProviderConfig iprovConfig(localConfigPath);
				OmmIProviderImpl ommIProviderImpl(iprovConfig.config(outermostMap).providerName("Provider_1"), appClient);

				OmmIProviderActiveConfig& activeConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
				bool found = ommIProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
				EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
				EXPECT_TRUE(activeConfig.pServerConfig->name == "Server_1") << "Server name , \"Server_1\"";
				EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serviceName == "14002") << "SocketServerConfig::serviceName , \"14002\"";

			}
			else if (testCase == 2)
			{
				OmmConsumerConfig consConfig(localConfigPath);
				OmmConsumerImpl ommConsumerImpl(consConfig.config(outermostMap).consumerName("Consumer_1"), true);

				OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
				bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
				EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
				EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1") << "Channel name , \"Channel_1\"";
				EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->hostName == "0.0.0.1") << "SocketChannelConfig::serviceName , \"0.0.0.1\"";
				EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->serviceName == "19001") << "SocketChannelConfig::hostName , \"19001\"";

			}
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
	}
}

TEST_F(EmaConfigTest, testLoadDictConfigBetweenProgrammaticAndFileForIProv)
{
		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			innerMap.addKeyAscii("Dictionary_3", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryItemName", "./ConfigDB3_RWFFld")
				.addAscii("EnumTypeDefItemName", "./ConfigDB3_RWFEnum")
				.addAscii("RdmFieldDictionaryFileName", "./ConfigDB3_RDMFieldDictionary")
				.addAscii("EnumTypeDefFileName", "./ConfigDB3_enumtype.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			EmaString localConfigPath;
			EmaString workingDir;
			ASSERT_EQ(getCurrentDir(workingDir), true)
				<< "Error: failed to load config file from current working dir "
				<< workingDir.c_str();
			localConfigPath.append(workingDir).append(emaConfigXMLFileNameTest);

			OmmIProviderConfig iprovConfig(localConfigPath);
			OmmIProviderImpl ommIProviderImpl(iprovConfig.config(outermostMap).providerName("Provider_2"), appClient);

			OmmIProviderActiveConfig& activeConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
			bool found = ommIProviderImpl.getInstanceName().find("Provider_2") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_2_1\"";
			
			//retrieve directory
			const DirectoryCache& dirCache = ommIProviderImpl.getDirectoryServiceStore().getDirectoryCache();
			EXPECT_TRUE(dirCache.directoryName == "Directory_2") << "directoryName = \"Directory_2\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 1) << "services.size() , 1";

			/*********retrieve first service *************/
			Service* pTemp = services.front();

			//retrieve dictionary provided/used by this service
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided.size() == 2) << "infoFilter.dictionariesProvided.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[0] == "./ConfigDB3_RWFFld") << "infoFilter.dictionariesProvided[0], \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesProvided[1] == "./ConfigDB3_RWFEnum") << "infoFilter.dictionariesProvided[1]  , \"./ConfigDB3_RWFEnum\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed.size() == 2) << "infoFilter.dictionariesUsed.size , 2";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[0] == "./ConfigDB3_RWFFld") << "infoFilter.dictionariesUsed[0] , \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(pTemp->infoFilter.dictionariesUsed[1] == "./ConfigDB3_RWFEnum") << "infoFilter.dictionariesUsed[1]  , \"./ConfigDB3_RWFEnum\"";

			//retrieve dictionary defined in directory
			ServiceDictionaryConfig* pDictConfig = activeConfig.getServiceDictionaryConfig(pTemp->serviceId);
			const EmaList<DictionaryConfig*>& dictUsed3 = pDictConfig->getDictionaryUsedList();
			const EmaList<DictionaryConfig*>& dictProvided3 = pDictConfig->getDictionaryProvidedList();
			EXPECT_TRUE(dictProvided3.size() == 1) << "dictProvided.size() , 1";
			EXPECT_TRUE(dictUsed3.size() == 1) << "dictUsed.size() , 1";
			DictionaryConfig* dictionaryProvidedConfig = dictProvided3.back();
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryName == "Dictionary_3") << "dictionaryProvidedConfig->dictionaryName , \"Dictionary_3\"";
			EXPECT_TRUE(dictionaryProvidedConfig->dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryProvidedConfig->dictionaryType , Dictionary::FileDictionaryEnum";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmfieldDictionaryFileName == "./ConfigDB3_RDMFieldDictionary") << "dictionaryProvidedConfig->rdmfieldDictionaryFileName , \"./ConfigDB3_RDMFieldDictionary\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumtypeDefFileName == "./ConfigDB3_enumtype.def") << "dictionaryProvidedConfig->enumtypeDefFileName  , \"./ConfigDB3_enumtype.def\"";
			EXPECT_TRUE(dictionaryProvidedConfig->rdmFieldDictionaryItemName == "./ConfigDB3_RWFFld") << "dictionaryProvidedConfig->rdmFieldDictionaryItemName , \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(dictionaryProvidedConfig->enumTypeDefItemName == "./ConfigDB3_RWFEnum") << "dictionaryProvidedConfig->enumTypeDefItemName  , \"./ConfigDB3_RWFEnum\"";

			DictionaryConfig* dictionaryUsedConfig = dictUsed3.back();
			EXPECT_TRUE(dictionaryUsedConfig->dictionaryName == "Dictionary_3") << "dictionaryUsedConfig->dictionaryName , \"Dictionary_3\"";
			EXPECT_TRUE(dictionaryUsedConfig->rdmfieldDictionaryFileName == "./ConfigDB3_RDMFieldDictionary") << "dictionaryUsedConfig->rdmfieldDictionaryFileName , \"./ConfigDB3_RDMFieldDictionary\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumtypeDefFileName == "./ConfigDB3_enumtype.def") << "dictionaryUsedConfig->enumtypeDefFileName  , \"./ConfigDB3_enumtype.def\"";
			EXPECT_TRUE(dictionaryUsedConfig->rdmFieldDictionaryItemName == "./ConfigDB3_RWFFld") << "dictionaryUsedConfig->rdmFieldDictionaryItemName , \"./ConfigDB3_RWFFld\"";
			EXPECT_TRUE(dictionaryUsedConfig->enumTypeDefItemName == "./ConfigDB3_RWFEnum") << "dictionaryUsedConfig->enumTypeDefItemName  , \"./ConfigDB3_RWFEnum\"";
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
}

TEST_F(EmaConfigTest, testLoadConfigFromProgrammaticForIProvConsMix)
{
	Map outermostMapIProv, outermostMapCons, innerMap;
	ElementList elementList;
	try
	{
		//programmatically config for iprovider
		elementList.addAscii("DefaultIProvider", "Provider_1");

		innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
			.addAscii("Server", "Server_1")
			.addAscii("Logger", "Logger_1")
			.addAscii("Directory", "Directory_1")
			.addInt("PipePort", 13650).complete())
			.complete();

		elementList.addMap("IProviderList", innerMap).complete();
		innerMap.clear();

		outermostMapIProv.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
			.addEnum("ServerType", 0)
			.addUInt("TcpNodelay", 0)
			.complete())
			.complete();

		elementList.addMap("ServerList", innerMap).complete();
		innerMap.clear();

		outermostMapIProv.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		//programmatically config for consumer
		elementList.addAscii("DefaultConsumer", "Consumer_1");

		innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum, ElementList()
			.addAscii("Channel", "Channel_2")
			.addAscii("Logger", "Logger_2")
			.addAscii("Dictionary", "Dictionary_2")
			.addInt("PipePort", 13650).complete())
			.complete();

		elementList.addMap("ConsumerList", innerMap).complete();
		innerMap.clear();

		outermostMapCons.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
			.addEnum("ChannelType", 0)
			.addUInt("TcpNodelay", 0)
			.complete())
			.complete();

		elementList.addMap("ChannelList", innerMap).complete();
		innerMap.clear();

		outermostMapCons.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		OmmIProviderImpl ommIProviderImpl(OmmIProviderConfig().config(outermostMapIProv), appClient);

		OmmIProviderActiveConfig& activeServerConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
		bool found = ommIProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
		EXPECT_TRUE(activeServerConfig.pServerConfig->name == "Server_1") << "Server name , \"Server_1\"";
		EXPECT_TRUE(activeServerConfig.loggerConfig.loggerName == "Logger_1") << "Logger name , \"Logger_1\"";
		const DirectoryCache& dirCache = ommIProviderImpl.getDirectoryServiceStore().getDirectoryCache();
		EXPECT_TRUE(dirCache.directoryName == "Directory_1") << "directoryName = \"Directory_1\"";

		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMapCons), true);

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_2") << "Connection name , \"Channel_2\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_2") << "Logger name , \"Logger_2\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_2") << "dictionaryName , \"Dictionary_2\"";
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
	}
}

TEST_F(EmaConfigTest, testReuseProgrammaticInterface)
{
		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			elementList.addAscii("DefaultConsumer", "Consumer_5");
			innerMap.addKeyAscii("Consumer_5", MapEntry::AddEnum, ElementList()
				.addAscii("Channel", "Channel_5")
				.addAscii("Logger", "Logger_5")
				.addUInt("LoginRequestTimeOut", 50000).complete())
				.complete();

			elementList.addMap("ConsumerList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Channel_5", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 0)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addAscii("Host", "host5")
				.addAscii("Port", "port5")
				.addUInt("TcpNodelay", 1)
				.complete())
				.complete();

			elementList.addMap("ChannelList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();
						
			OmmConsumerConfig consConfig;
			OmmConsumerImpl ommConsumerImpl(consConfig.config(outermostMap), true);

			OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
			bool found = ommConsumerImpl.getInstanceName().find("Consumer_5") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_5_1\"";
			EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_5") << "Channel name , \"Channel_5\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->hostName == "host5") << "SocketChannelConfig::serviceName , \"host5\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig.configChannelSet[0])->serviceName == "port5") << "SocketChannelConfig::hostName , \"port5\"";

			//reuse OmmConsumerConfig interface
			outermostMap.clear();
			elementList.addAscii("DefaultConsumer", "Consumer_6");
			innerMap.addKeyAscii("Consumer_6", MapEntry::AddEnum, ElementList()
				.addAscii("Channel", "Channel_6")
				.addAscii("Logger", "Logger_6")
				.addUInt("LoginRequestTimeOut", 50000).complete())
				.complete();

			elementList.addMap("ConsumerList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Channel_6", MapEntry::AddEnum, ElementList()
				.addEnum("ChannelType", 0)
				.addUInt("GuaranteedOutputBuffers", 7000)
				.addAscii("Host", "host6")
				.addAscii("Port", "port6")
				.addUInt("TcpNodelay", 1)
				.complete())
				.complete();

			elementList.addMap("ChannelList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList).complete();
			elementList.clear();

			consConfig.clear();
			OmmConsumerImpl ommConsumerImpl1(consConfig.config(outermostMap), true);

			OmmConsumerActiveConfig& activeConfig1 = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl1.getActiveConfig());
			found = ommConsumerImpl1.getInstanceName().find("Consumer_6") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_6_1\"";
			EXPECT_TRUE(activeConfig1.configChannelSet[0]->name == "Channel_6") << "Channel name , \"Channel_6\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig1.configChannelSet[0])->hostName == "host6") << "SocketChannelConfig::serviceName , \"host5\"";
			EXPECT_TRUE(((SocketChannelConfig*)activeConfig1.configChannelSet[0])->serviceName == "port6") << "SocketChannelConfig::hostName , \"port5\"";
			
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testLoadingIProvConfigurationFromProgrammaticConfig()";
		}
}

TEST_F(EmaConfigTest, testServerSharedSocketProgrammaticConfigForIProv)
{
	//test case 0: section "ServerSharedSocket" is absent
	//test case 1: section "ServerSharedSocket" is equal 0 (False)
	//test case 2: section "ServerSharedSocket" is equal 1 (True)
	for (int testCase = 0; testCase < 3; testCase++)
	{
		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;

		Map outermostMap, innerMap;
		ElementList elementList;
		try
		{
			elementList.addAscii("DefaultIProvider", "Provider_1");

			innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
				.addAscii("Server", "Server_1")
				.addAscii("Logger", "Logger_1")
				.addAscii("Directory", "Directory_1").complete())
				.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
					.addAscii("Server", "Server_2")
					.addAscii("Directory", "Directory_2")
					.addAscii("Logger", "Logger_2").complete())
				.complete();

			elementList.addMap("IProviderList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			switch (testCase)
			{
			case 0:  // section "ServerSharedSocket" is absent
				innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", 0)
					.addEnum("CompressionType", 1)
					.addUInt("GuaranteedOutputBuffers", 8000)
					.addUInt("NumInputBuffers", 7777)
					.addUInt("SysRecvBufSize", 150000)
					.addUInt("SysSendBufSize", 200000)
					.addUInt("CompressionThreshold", 12856)
					.addUInt("ConnectionPingTimeout", 30000)
					.addUInt("ConnectionMinPingTimeout", 8000)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Port", "14010")
					.addUInt("TcpNodelay", 0)
					.addUInt("InitializationTimeout", 66)
					.complete())
					.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
						.addEnum("ServerType", 1)
						.addAscii("Port", "14011").complete())
					.complete();
				break;

			case 1:  // section "ServerSharedSocket" is equal 0 (False)
				innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", 0)
					.addEnum("CompressionType", 1)
					.addUInt("GuaranteedOutputBuffers", 8000)
					.addUInt("NumInputBuffers", 7777)
					.addUInt("SysRecvBufSize", 150000)
					.addUInt("SysSendBufSize", 200000)
					.addUInt("CompressionThreshold", 12856)
					.addUInt("ConnectionPingTimeout", 30000)
					.addUInt("ConnectionMinPingTimeout", 8000)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Port", "14010")
					.addUInt("TcpNodelay", 0)
					.addUInt("InitializationTimeout", 66)
					.addUInt("ServerSharedSocket", 0)
					.addUInt("DirectWrite", 0)
					.complete())
					.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
						.addEnum("ServerType", 1)
						.addAscii("Port", "14011").complete())
					.complete();
				break;

			case 2:  // section "ServerSharedSocket" is equal 1 (True)
				innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", 0)
					.addEnum("CompressionType", 1)
					.addUInt("GuaranteedOutputBuffers", 8000)
					.addUInt("NumInputBuffers", 7777)
					.addUInt("SysRecvBufSize", 150000)
					.addUInt("SysSendBufSize", 200000)
					.addUInt("CompressionThreshold", 12856)
					.addUInt("ConnectionPingTimeout", 30000)
					.addUInt("ConnectionMinPingTimeout", 8000)
					.addAscii("InterfaceName", "localhost")
					.addAscii("Port", "14010")
					.addUInt("TcpNodelay", 0)
					.addUInt("InitializationTimeout", 66)
					.addUInt("ServerSharedSocket", 1)
					.addUInt("DirectWrite", 1)
					.complete())
					.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
						.addEnum("ServerType", 1)
						.addAscii("Port", "14011").complete())
					.complete();
				break;
			}

			elementList.addMap("ServerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 1)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete())
				.addKeyAscii("Logger_1", MapEntry::AddEnum,
					ElementList()
					.addEnum("LoggerType", 0)
					.addUInt("NumberOfLogFiles", 42)
					.addUInt("MaxLogFileSize", 84000)
					.addAscii("FileName", "logFile")
					.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_3", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryItemName", "RWFFld")
				.addAscii("EnumTypeDefItemName", "RWFEnum")
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete())
				.addKeyAscii("Dictionary_4", MapEntry::AddEnum,
					ElementList()
					.addEnum("DictionaryType", 1)
					.addAscii("RdmFieldDictionaryItemName", "RWFFld_ID4")
					.addAscii("EnumTypeDefItemName", "RWFEnum_ID4")
					.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4")
					.addAscii("EnumTypeDefFileName", "./enumtype_ID4.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
			elementList.clear();


			EmaString localConfigPath;
			OmmIProviderConfig iprovConfig(localConfigPath);
			OmmIProviderImpl ommIProviderImpl(iprovConfig.config(outermostMap), appClient);

			OmmIProviderActiveConfig& activeConfig = static_cast<OmmIProviderActiveConfig&>(ommIProviderImpl.getActiveConfig());
			bool found = ommIProviderImpl.getInstanceName().find("Provider_1") >= 0 ? true : false;
			EXPECT_TRUE(found) << "ommIProviderImpl.getIProviderName() , \"Provider_1_1\"";
			EXPECT_TRUE(activeConfig.pServerConfig->name == "Server_1") << "Server name , \"Server_1\"";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->getType() == 0) << "SocketServerConfig::getType , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serviceName == "14010") << "SocketServerConfig::serviceName , \"14002\"";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->tcpNodelay == 0) << "SocketServerConfig::tcpNodelay , 0";
			EXPECT_TRUE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->initializationTimeout == 66) << "SocketServerConfig::initializationTimeout , 66";
			if (testCase < 2)
			{
				EXPECT_EQ(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serverSharedSocket, 0) << "SocketServerConfig::serverSharedSocket , 0";
			}
			else
			{
				EXPECT_NE(static_cast<SocketServerConfig*>(activeConfig.pServerConfig)->serverSharedSocket, 0) << "SocketServerConfig::serverSharedSocket , 1";
			}

			if (testCase < 2)
			{
				EXPECT_EQ(activeConfig.pServerConfig->directWrite, 0) << "ServerConfig::directWrite , 0";
			}
			else
			{
				EXPECT_NE(activeConfig.pServerConfig->directWrite, 0) << "ServerConfig::directWrite, 1";
			}
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testServerSharedSocketProgrammaticConfigForIProv()";
		}
	}
}

TEST_F(EmaConfigTest, testLoadFilterValueOverflowProgrammaticConfigForIProv)
{
	for (int testCase = 0; testCase < 3; testCase++)
	{

		std::cout << std::endl << " #####Now it is running test case " << testCase << std::endl;
		Map outermostMap, innerMap;
		ElementList elementList;

		try
		{
			elementList.addAscii("DefaultIProvider", "Provider_1");

			innerMap.addKeyAscii("Provider_1", MapEntry::AddEnum, ElementList()
				.addAscii("Server", "Server_1")
				.addAscii("Logger", "Logger_1")
				.addAscii("Directory", "Directory_1").complete())
				.addKeyAscii("Provider_2", MapEntry::AddEnum, ElementList()
					.addAscii("Server", "Server_2")
					.addAscii("Directory", "Directory_2")
					.addAscii("Logger", "Logger_2").complete())
				.complete();

			elementList.addMap("IProviderList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("IProviderGroup", MapEntry::AddEnum, elementList);

			elementList.clear();


			innerMap.addKeyAscii("Server_1", MapEntry::AddEnum, ElementList()
				.addEnum("ServerType", 0)
				.addEnum("CompressionType", 1)
				.addUInt("GuaranteedOutputBuffers", 8000)
				.addUInt("NumInputBuffers", 7777)
				.addUInt("SysRecvBufSize", 150000)
				.addUInt("SysSendBufSize", 200000)
				.addUInt("CompressionThreshold", 12856)
				.addUInt("ConnectionPingTimeout", 30000)
				.addUInt("ConnectionMinPingTimeout", 8000)
				.addAscii("InterfaceName", "localhost")
				.addAscii("Port", "14010")
				.addUInt("TcpNodelay", 0)
				.addUInt("InitializationTimeout", 66)
				.complete())
				.addKeyAscii("Server_2", MapEntry::AddEnum, ElementList()
					.addEnum("ServerType", 1)
					.addAscii("Port", "14011").complete())
				.complete();

			elementList.addMap("ServerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("ServerGroup", MapEntry::AddEnum, elementList);

			elementList.clear();

			innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
				ElementList()
				.addEnum("LoggerType", 1)
				.addAscii("FileName", "logFile")
				.addUInt("NumberOfLogFiles", 42)
				.addUInt("MaxLogFileSize", 84000)
				.addEnum("LoggerSeverity", 3).complete())
				.addKeyAscii("Logger_1", MapEntry::AddEnum,
					ElementList()
					.addEnum("LoggerType", 0)
					.addAscii("FileName", "logFile")
					.addUInt("NumberOfLogFiles", 42)
					.addUInt("MaxLogFileSize", 84000)
					.addEnum("LoggerSeverity", 3).complete()).complete();

			elementList.addMap("LoggerList", innerMap);

			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
				ElementList()
				.addEnum("DictionaryType", 1)
				.addAscii("RdmFieldDictionaryItemName", "RWFFld")
				.addAscii("EnumTypeDefItemName", "RWFEnum")
				.addAscii("RdmFieldDictionaryFileName", fieldDictionaryFileNameTest)
				.addAscii("EnumTypeDefFileName", enumTableFileNameTest).complete())
				.addKeyAscii("Dictionary_2", MapEntry::AddEnum,
					ElementList()
					.addEnum("DictionaryType", 2)
					.addAscii("RdmFieldDictionaryItemName", "RWFFld_ID4")
					.addAscii("EnumTypeDefItemName", "RWFEnum_ID4")
					.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary_ID4")
					.addAscii("EnumTypeDefFileName", "./enumtype_ID4.def").complete()).complete();

			elementList.addMap("DictionaryList", innerMap);
			elementList.complete();
			innerMap.clear();

			outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
			elementList.clear();

			Map serviceMap;
			switch (testCase)
			{
			case 0:
				//encode service "OpenLimit" overflow
				serviceMap.addKeyAscii("DIRECT_FEED1", MapEntry::AddEnum,
					ElementList()
					.addElementList("LoadFilter",
						ElementList()
						.addUInt("OpenLimit", OVERFLOW_MAX_UNSIGNED_INT32)
						.addUInt("OpenWindow", 1)
						.addUInt("LoadFactor", 2)
						.complete())
					.complete())
					.complete();
				break;
			case 1:
				//encode service "OpenWindow" overflow
				serviceMap.addKeyAscii("DIRECT_FEED2", MapEntry::AddEnum,
					ElementList()
					.addElementList("LoadFilter",
						ElementList()
						.addUInt("OpenLimit", 3)
						.addUInt("OpenWindow", OVERFLOW_MAX_UNSIGNED_INT32)
						.addUInt("LoadFactor", 4)
						.complete())
					.complete())
					.complete();
				break;
			case 2:
				//encode service "LoadFactor" overflow
				serviceMap.addKeyAscii("DIRECT_FEED3", MapEntry::AddEnum,
					ElementList()
					.addElementList("LoadFilter",
						ElementList()
						.addUInt("OpenLimit", 5)
						.addUInt("OpenWindow", 6)
						.addUInt("LoadFactor", OVERFLOW_MAX_UNSIGNED_INT16)
						.complete())
					.complete())
					.complete();
				break;
			}

			innerMap.addKeyAscii("Directory_1", MapEntry::AddEnum, serviceMap).complete();

			serviceMap.clear();
			elementList.clear();
			elementList.addAscii("DefaultDirectory", "Directory_1");
			elementList.addMap("DirectoryList", innerMap).complete();
			outermostMap.addKeyAscii("DirectoryGroup", MapEntry::AddEnum, elementList).complete();


			EmaString localConfigPath;
			OmmIProviderConfig iprovConfig(localConfigPath);
			OmmIProviderImpl ommIProviderImpl(iprovConfig.config(outermostMap), appClient);
			outermostMap.clear();

			//retrieve directory
			const DirectoryCache& dirCache = ommIProviderImpl.getDirectoryServiceStore().getDirectoryCache();
			EXPECT_TRUE(dirCache.directoryName == "Directory_1") << "directoryName = \"Directory_1\"";
			const EmaList< Service* >& services = dirCache.getServiceList();
			EXPECT_TRUE(services.size() == 1) << "services.size() , 1";

			Service* pTemp = services.front();

			switch (testCase)
			{
			case 0:
				EXPECT_TRUE(pTemp->loadFilter.openLimit == 0);
				EXPECT_TRUE(pTemp->loadFilter.openWindow == 1);
				EXPECT_TRUE(pTemp->loadFilter.loadFactor == 2);

				EXPECT_FALSE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_LIMIT";
				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_WINDOW";
				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) << "loadFilter.flags, RDM_SVC_LDF_HAS_LOAD_FACTOR";
				break;
			case 1:
				EXPECT_TRUE(pTemp->loadFilter.openLimit == 3);
				EXPECT_TRUE(pTemp->loadFilter.openWindow == 0);
				EXPECT_TRUE(pTemp->loadFilter.loadFactor == 4);

				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_LIMIT";
				EXPECT_FALSE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_WINDOW";
				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) << "loadFilter.flags, RDM_SVC_LDF_HAS_LOAD_FACTOR";
				break;
			case 2:
				EXPECT_TRUE(pTemp->loadFilter.openLimit == 5);
				EXPECT_TRUE(pTemp->loadFilter.openWindow == 6);
				EXPECT_TRUE(pTemp->loadFilter.loadFactor == 0);

				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_LIMIT) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_LIMIT";
				EXPECT_TRUE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_OPEN_WINDOW) << "loadFilter.flags, RDM_SVC_LDF_HAS_OPEN_WINDOW";
				EXPECT_FALSE(pTemp->loadFilter.flags & RDM_SVC_LDF_HAS_LOAD_FACTOR) << "loadFilter.flags, RDM_SVC_LDF_HAS_LOAD_FACTOR";
				break;
			}

			EXPECT_FALSE(pTemp->flags & RDM_SVCF_HAS_INFO) << "flags, RDM_SVCF_HAS_INFO";
			EXPECT_FALSE(pTemp->flags & RDM_SVCF_HAS_STATE) << "flags, RDM_SVCF_HAS_STATE";
			EXPECT_TRUE(pTemp->flags & RDM_SVCF_HAS_LOAD) << "flags, RDM_SVCF_HAS_LOAD";
		}

		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in testServerSharedSocketProgrammaticConfigForIProv()";
		}
	}
}

class EmaConfigTestNotExist : public ::testing::Test {
public:
	void SetUp() {
		// The default path uses when creating Config instance
		// and was set an empty path to EmaConfigTest.xml or a directory
		EmaConfigBaseImpl::setDefaultConfigFileName(defaultFileNameNotExist);
		
		errorMsgText.append("configuration path [").append(userDefinedFileNameNotExist).append("] does not exist;");
	}

	void TearDown() {
	}

	static const char* defaultFileNameNotExist;
	static const char* userDefinedFileNameNotExist;
	EmaString errorMsgText;
};

const char* EmaConfigTestNotExist::defaultFileNameNotExist = "EmaConfigFileNotExist.xml";
const char* EmaConfigTestNotExist::userDefinedFileNameNotExist = "EmaConfigUserFileNotExist.xml";

TEST_F(EmaConfigTestNotExist, OmmConsumerConfigShouldThrowException)
{
	try {
		OmmConsumerConfig config( EmaConfigTestNotExist::userDefinedFileNameNotExist );
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist.";
	}
	catch ( const OmmInvalidConfigurationException& exp )
	{
		const EmaString& expErrorText = exp.getText().substr( 0, errorMsgText.length() );
		ASSERT_STREQ( errorMsgText.c_str(), expErrorText.c_str() );
	}
	catch (...)
	{
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist. Actual: it throws a different type.";
	}
}

TEST_F(EmaConfigTestNotExist, OmmConsumerConfigDefaultShouldNoException)
{
	ASSERT_NO_THROW(
		{
			OmmConsumerConfig config;
		}
	) << "When the default EmaConfig file is not available OmmConsumerConfig does not throw any exception.";
}

TEST_F(EmaConfigTestNotExist, OmmIProviderConfigShouldThrowException)
{
	try {
		OmmIProviderConfig config( EmaConfigTestNotExist::userDefinedFileNameNotExist );
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist.";
	}
	catch ( const OmmInvalidConfigurationException& exp )
	{
		const EmaString& expErrorText = exp.getText().substr( 0, errorMsgText.length() );
		ASSERT_STREQ( errorMsgText.c_str(), expErrorText.c_str() );
	}
	catch (...)
	{
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist. Actual: it throws a different type.";
	}
}

TEST_F(EmaConfigTestNotExist, OmmIProviderConfigDefaultShouldNoException)
{
	ASSERT_NO_THROW(
		{
			OmmIProviderConfig config;
		}
	) << "When the default EmaConfig file is not available OmmIProviderConfig does not throw any exception.";
}

TEST_F(EmaConfigTestNotExist, OmmNiProviderConfigShouldThrowException)
{
	try {
		OmmNiProviderConfig config( EmaConfigTestNotExist::userDefinedFileNameNotExist );
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist.";
	}
	catch (const OmmInvalidConfigurationException& exp)
	{
		const EmaString& expErrorText = exp.getText().substr( 0, errorMsgText.length() );
		ASSERT_STREQ( errorMsgText.c_str(), expErrorText.c_str() );
	}
	catch (...)
	{
		FAIL() << "OmmInvalidConfigurationException was expected because the configuration file is not exist. Actual: it throws a different type.";
	}
}

TEST_F(EmaConfigTestNotExist, OmmNiProviderConfigDefaultShouldNoException)
{
	ASSERT_NO_THROW(
		{
			OmmNiProviderConfig config;
		}
	) << "When the default EmaConfig file is not available OmmNiProviderConfig does not throw any exception.";
}
