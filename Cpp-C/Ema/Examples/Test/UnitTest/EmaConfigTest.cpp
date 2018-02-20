/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "EmaAppClient.h"
#include "OmmConsumerConfigImpl.h"
#include "OmmConsumerImpl.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

class EmaConfigTest : public ::testing::Test {
public:

	void SetUp() {
		SCOPED_TRACE("EmaConfigTest SetUp");
		if (hasRun)
			return;
		hasRun = true;
		
		EmaString workingDir;
		ASSERT_EQ(getCurrentDir(workingDir), true)
			<< "Error: failed to load config file from current working dir "
			<< workingDir.c_str();
		
		configPath.append(workingDir).append("//EmaConfig.xml");
		SCOPED_TRACE("Loading Ema config file from ");
		SCOPED_TRACE(configPath);

		SCOPED_TRACE("Starting provider1 with port 14002 and provider2 with port 14008\n");
		provider1 = new OmmProvider(OmmIProviderConfig().port("14002"), appClient);
		provider2 = new OmmProvider(OmmIProviderConfig().port("14008"), appClient);
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
	RsslConnectionTypes channelType;
	RsslCompTypes compType;
	OmmLoggerClient::LoggerType loggerType;
	OmmLoggerClient::Severity loggerSeverity;
	Dictionary::DictionaryType dictionaryType;

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
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|XmlTraceHex", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting XmlTraceHex from EmaConfig.xml";
	debugResult = config.get<UInt64>("ConsumerGroup|ConsumerList|Consumer.Consumer_2|MsgKeyInUpdates", uintValue);
	EXPECT_TRUE(debugResult && uintValue == 1) << "extracting MsgKeyInUpdates from EmaConfig.xml";

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
	EXPECT_TRUE( debugResult && retrievedValue == "132.56.23.123" ) << "extracting Host from EmaConfig.xml";
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
	EXPECT_TRUE( debugResult && retrievedValue == "122.1.1.100" ) << "extracting Host from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|Port", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "15008" ) << "extracting Port from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|TcpNodelay", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 0) << "extracting TcpNodelay from EmaConfig.xml";
	debugResult = config.get<EmaString>( "ChannelGroup|ChannelList|Channel.Channel_2|ObjectName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "HttpObjectName" ) << "extracting ObjectName from EmaConfig.xml";
	debugResult = config.get<UInt64>( "ChannelGroup|ChannelList|Channel.Channel_2|CompressionThreshold", uintValue );
	EXPECT_TRUE( debugResult && uintValue == 4096) << "extracting CompressionThreshold from EmaConfig.xml";

	// Checks all values from Logger_1
	debugResult = config.get<EmaString>( "LoggerGroup|LoggerList|Logger|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Logger_1" ) << "extracting the first logger name from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::LoggerType>( "LoggerGroup|LoggerList|Logger.Logger_1|LoggerType", loggerType );
	EXPECT_TRUE( debugResult && loggerType == OmmLoggerClient::FileEnum) << "extracting LoggerType from EmaConfig.xml";
	debugResult = config.get<EmaString>( "LoggerGroup|LoggerList|Logger.Logger_1|FileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "emaLog" ) << "extracting FileName from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::Severity>( "LoggerGroup|LoggerList|Logger.Logger_1|LoggerSeverity", loggerSeverity );
	EXPECT_TRUE( debugResult && loggerSeverity == OmmLoggerClient::VerboseEnum) << "extracting LoggerSeverity from EmaConfig.xml";

	// Checks all values from Logger_2
	debugResult = config.get<OmmLoggerClient::LoggerType>( "LoggerGroup|LoggerList|Logger.Logger_2|LoggerType", loggerType );
	EXPECT_TRUE( debugResult && loggerType == OmmLoggerClient::StdoutEnum) << "extracting LoggerType from EmaConfig.xml";
	debugResult = config.get<OmmLoggerClient::Severity>( "LoggerGroup|LoggerList|Logger.Logger_2|LoggerSeverity", loggerSeverity );
	EXPECT_TRUE( debugResult && loggerSeverity == OmmLoggerClient::ErrorEnum) << "extracting LoggerSeverity from EmaConfig.xml";

	// Checks all values from Dictionary_1
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary|Name", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "Dictionary_1" ) << "extracting the first dictionary name from EmaConfig.xml";
	debugResult = config.get<Dictionary::DictionaryType>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_1|DictionaryType", dictionaryType );
	EXPECT_TRUE( debugResult && dictionaryType == Dictionary::ChannelDictionaryEnum) << "extracting DictionaryType from EmaConfig.xml";

	// Checks all values from Dictionary_2
	debugResult = config.get<Dictionary::DictionaryType>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|DictionaryType", dictionaryType );
	EXPECT_TRUE( debugResult && dictionaryType == Dictionary::FileDictionaryEnum) << "extracting DictionaryType from EmaConfig.xml";
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|RdmFieldDictionaryFileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "./RDMFieldDictionary" ) << "extracting RDMFieldDictionary from EmaConfig.xml";
	debugResult = config.get<EmaString>( "DictionaryGroup|DictionaryList|Dictionary.Dictionary_2|EnumTypeDefFileName", retrievedValue );
	EXPECT_TRUE( debugResult && retrievedValue == "./enumtype.def" ) << "extracting enumtype.def from EmaConfig.xml";

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

	config.configErrors().printErrors(OmmLoggerClient::WarningEnum);
}

// http connection supported on windows only
#ifdef WIN_32
TEST_F(EmaConfigTest, testLoadingConfigurationFromProgrammaticConfigHttp)
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
			.addUInt("ItemCountHint", 5000)
			.addUInt("ServiceCountHint", 2000)
			.addUInt("ObeyOpenWindow", 1)
			.addUInt("PostAckTimeout", 1200)
			.addUInt("RequestTimeout", 2400)
			.addUInt("MaxOutstandingPosts", 9999)
			.addInt("DispatchTimeoutApiThread", 60)
			.addUInt("CatchUnhandledException", 1)
			.addUInt("MaxDispatchCountApiThread", 300)
			.addUInt("MaxDispatchCountUserThread", 700).complete()).complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

		innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("ChannelType", 2)
			.addAscii("InterfaceName", "localhost")
			.addEnum("CompressionType", 1)
			.addUInt("GuaranteedOutputBuffers", 8000)
			.addUInt("NumInputBuffers", 7777)
			.addUInt("SysRecvBufSize", 150000)
			.addUInt("SysSendBufSize", 200000)
			.addUInt("ConnectionPingTimeout", 30000)
			.addInt("ReconnectAttemptLimit", 1)
			.addInt("ReconnectMinDelay", 500)
			.addInt("ReconnectMaxDelay", 500)
			.addAscii("Host", "localhost")
			.addAscii("Port", "14002")
			.addUInt("TcpNodelay", 0)
			.addAscii("ObjectName", "MyHttpObject")
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTraceHex", 1)
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
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));
				
		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );
		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_1" ) >= 0 ? true : false;
		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_1" ) << "Connection name , \"Channel_1\"";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
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
		EXPECT_TRUE( activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE( activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_HTTP) << "connectionType , ChannelType::RSSL_CONN_TYPE_HTTP";
		EXPECT_TRUE( static_cast<HttpChannelConfig* >( activeConfig.configChannelSet[0] )->hostName == "localhost" ) << "EncryptedChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE( static_cast<HttpChannelConfig* >( activeConfig.configChannelSet[0] )->serviceName == "14002" ) << "EncryptedChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE( static_cast<HttpChannelConfig* >( activeConfig.configChannelSet[0] )->objectName == "MyHttpObject" ) << "EncryptedChannelConfig::ObjectName , \"MyHttpObject\"";
		EXPECT_TRUE( static_cast<HttpChannelConfig* >( activeConfig.configChannelSet[0] )->tcpNodelay == false) << "SocketChannelConfig::tcpNodelay , false";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerFileName == "logFile" ) << "loggerFileName = \"logFile\"";
		EXPECT_TRUE( activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE( activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == "./RDMFieldDictionary" ) << "rdmfieldDictionaryFileName , \"./RDMFieldDictionary\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.enumtypeDefFileName == "./enumtype.def" ) << "enumtypeDefFileName , \"./enumtype.def\"";
	}
	catch ( const OmmException& excp )
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE( false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfigHttp()";
	}
}
#endif

TEST_F(EmaConfigTest, testLoadingConfigurationFromProgrammaticConfig)
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
			.addAscii("XmlTraceFileName", "MyXMLTrace")
			.addInt("XmlTraceMaxFileSize", 50000000)
			.addUInt("XmlTraceToFile", 0)
			.addUInt("XmlTraceToStdout", 1)
			.addUInt("XmlTraceToMultipleFiles", 1)
			.addUInt("XmlTraceWrite", 1)
			.addUInt("XmlTraceRead", 1)
			.addUInt("XmlTracePing", 1)
			.addUInt("XmlTraceHex", 1)
			.addUInt("MsgKeyInUpdates", 1)
			.addInt("ReconnectAttemptLimit", 10)
			.addInt("ReconnectMinDelay", 4444)
			.addInt("ReconnectMaxDelay", 7777)
			.addInt("PipePort", 13650).complete())
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
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());
		bool found = ommConsumerImpl.getInstanceName().find("Consumer_1") >= 0 ? true : false;
		EXPECT_TRUE(found) << "ommConsumerImpl.getConsumerName() , \"Consumer_1_1\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->name == "Channel_1" ) << "Connection name , \"Channel_1\"";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerName == "Logger_1" ) << "Logger name , \"Logger_1\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryName == "Dictionary_1" ) << "dictionaryName , \"Dictionary_1\"";
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
		EXPECT_TRUE(activeConfig.xmlTraceHex == 1) << "xmlTraceHex , 1";
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == 1) << "msgKeyInUpdates , 1";
		EXPECT_TRUE(activeConfig.pipePort == 13650) << "pipePort , 13650";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 8000) << "guaranteedOutputBuffers , 8000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->numInputBuffers == 7777) << "numInputBuffers , 7777";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysRecvBufSize == 150000) << "sysRecvBufSize , 150000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->sysSendBufSize == 200000) << "sysSendBufSize , 200000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->compressionThreshold == 12856) << "CompressionThreshold , 12856";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionPingTimeout == 30000) << "connectionPingTimeout , 30000";
		EXPECT_TRUE(activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->hostName == "localhost" ) << "SocketChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->serviceName == "14002" ) << "SocketChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE(static_cast<SocketChannelConfig* >(activeConfig.configChannelSet[0])->tcpNodelay == false) << "SocketChannelConfig::tcpNodelay , false";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE(activeConfig.loggerConfig.loggerFileName == "logFile" ) << "loggerFileName = \"logFile\"";
		EXPECT_TRUE(activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::ErrorEnum) << "minLoggerSeverity = OmmLoggerClient::ErrorEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum) << "dictionaryType , Dictionary::FileDictionaryEnum";
		EXPECT_TRUE(activeConfig.dictionaryConfig.rdmfieldDictionaryFileName == "./RDMFieldDictionary" ) << "rdmfieldDictionaryFileName , \"./RDMFieldDictionary\"";
		EXPECT_TRUE(activeConfig.dictionaryConfig.enumtypeDefFileName == "./enumtype.def" ) << "enumtypeDefFileName , \"./enumtype.def\"";
	}
	catch (const OmmException& excp)
	{
		std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
		EXPECT_TRUE(false) << "Unexpected exception in testLoadingConfigurationFromProgrammaticConfig()";
	}
}

TEST_F(EmaConfigTest, testOverridingFromInterface)
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
			.complete()).complete();

		elementList.addMap("ConsumerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();

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
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
		elementList.clear();

		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", 1)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();

		// Must load data dictionary files from current working location.
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap).host("localhost:14002"));

		const OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );

		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->hostName == "localhost" ) << "SocketChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->serviceName == "14002" ) << "SocketChannelConfig::serviceName , \"14002\"";
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
			.addAscii("XmlTraceFileName", "ConfigDbXMLTrace")
			.addInt("XmlTraceMaxFileSize", 70000000)
			.addUInt("XmlTraceToFile", 1)
			.addUInt("XmlTraceToStdout", 0)
			.addUInt("XmlTraceToMultipleFiles", 0)
			.addUInt("XmlTraceWrite", 0)
			.addUInt("XmlTraceRead", 0)
			.addUInt("XmlTracePing", 0)
			.addUInt("XmlTraceHex", 0)
			.addUInt("MsgKeyInUpdates", 0)
			.addInt("ReconnectAttemptLimit", 70)
			.addInt("ReconnectMinDelay", 7000)
			.addInt("ReconnectMaxDelay", 7000)
			.addInt("PipePort", 9696).complete()).complete();

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
			.complete()).complete();

		elementList.addMap("ChannelList", innerMap).complete();
		innerMap.clear();

		configDB2.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList).complete();
		elementList.clear();

		innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
			ElementList()
			.addEnum("LoggerType", 0)
			.addAscii("FileName", "ConfigDB2_logFile")
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

		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(configDB1).config(configDB2).config(configDB3).config(configDB4));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>(ommConsumerImpl.getActiveConfig());

		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_2" ) >= 0 ? true : false;
		EXPECT_TRUE( found) << "ommConsumerImpl.getConsumerName() , \"Consumer_2_3\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->name == "Channel_2" ) << "Connection name , \"Channel_2\"";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerName == "Logger_2" ) << "Logger name , \"Logger_2\"";
		EXPECT_TRUE( activeConfig.dictionaryConfig.dictionaryName == "Dictionary_2" ) << "dictionaryName , \"Dictionary_2\"";
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
		EXPECT_TRUE( activeConfig.xmlTraceHex == 0) << "xmlTraceHex , 0";
		EXPECT_TRUE( activeConfig.msgKeyInUpdates == 0) << "msgKeyInUpdates , 0";

		EXPECT_TRUE( activeConfig.configChannelSet[0]->interfaceName == "localhost" ) << "interfaceName , \"localhost\"";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->compressionType == 2) << "compressionType , 2";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->guaranteedOutputBuffers == 7000) << "guaranteedOutputBuffers , 7000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->numInputBuffers == 888888) << "numInputBuffers , 888888";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysRecvBufSize == 550000) << "sysRecvBufSize , 550000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->sysSendBufSize == 700000) << "sysSendBufSize , 700000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->compressionThreshold == 12758) << "compressionThreshold , compressionThreshold";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionPingTimeout == 70000) << "connectionPingTimeout , 70000";
		EXPECT_TRUE( activeConfig.configChannelSet[0]->connectionType == RSSL_CONN_TYPE_SOCKET) << "connectionType , ChannelType::RSSL_SOCKET";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->hostName == "localhost" ) << "SocketChannelConfig::hostname , \"localhost\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->serviceName == "14002" ) << "SocketChannelConfig::serviceName , \"14002\"";
		EXPECT_TRUE( static_cast<SocketChannelConfig* >( activeConfig.configChannelSet[0] )->tcpNodelay == 1) << "SocketChannelConfig::tcpNodelay , 1";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum) << "loggerType = OmmLoggerClient::FileEnum";
		EXPECT_TRUE( activeConfig.loggerConfig.loggerFileName == "ConfigDB2_logFile" ) << "loggerFileName = \"ConfigDB2_logFile\"";
		EXPECT_TRUE( activeConfig.loggerConfig.minLoggerSeverity == OmmLoggerClient::NoLogMsgEnum) << "minLoggerSeverity = OmmLoggerClient::NoLogMsgEnum";
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
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

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
				.addAscii("Host", "122.1.1.100")
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
			.addEnum("LoggerSeverity", 3).complete()).complete();

		elementList.addMap("LoggerList", innerMap);

		elementList.complete();
		innerMap.clear();

		outermostMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);

		elementList.clear();
		innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
			ElementList()
			.addEnum("DictionaryType", Dictionary::FileDictionaryEnum)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

		elementList.addMap("DictionaryList", innerMap);

		elementList.complete();

		outermostMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);

		outermostMap.complete();
		SCOPED_TRACE("Must load data dictionary files from current working location\n");
		OmmConsumerImpl ommConsumerImpl(OmmConsumerConfig().config(outermostMap));

		OmmConsumerActiveConfig& activeConfig = static_cast<OmmConsumerActiveConfig&>( ommConsumerImpl.getActiveConfig() );
		bool found = ommConsumerImpl.getInstanceName().find( "Consumer_3" ) >= 0 ? true : false;

		SocketChannelConfig* socCfg = static_cast<SocketChannelConfig*>( activeConfig.configChannelSet[0] );
		EncryptedChannelConfig* encCfg = static_cast<EncryptedChannelConfig*>( activeConfig.configChannelSet[1] );
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
		EXPECT_TRUE(activeConfig.xmlTraceHex == true) << "xmlTraceHex , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTraceWrite == true) << "xmlTraceWrite , \"true\"";
		EXPECT_TRUE(activeConfig.xmlTraceToFile == false) << "xmlTraceToFile , \"false\"";
		EXPECT_TRUE(activeConfig.xmlTraceToStdout == false) << "xmlTraceToStdout , \"false\"";
		EXPECT_TRUE(activeConfig.msgKeyInUpdates == false) << "msgKeyInUpdates , \"false\"";
		EXPECT_TRUE(activeConfig.xmlTraceMaxFileSize == 8800) << "xmlTraceMaxFileSize , \"8800\"";


		EXPECT_TRUE( socCfg->hostName == "localhost" ) << "hostname , \"localhost\"";
		EXPECT_TRUE( socCfg->serviceName == "14008" ) << "serviceName , \"14008\"";
		EXPECT_TRUE( socCfg->tcpNodelay == false) << "tcpNodelay , \"false\"";
		EXPECT_TRUE( socCfg->compressionType == RSSL_COMP_NONE) << "compressionType , \"RSSL_COMP_NONE\"";

		EXPECT_TRUE( activeConfig.configChannelSet[1]->name == "Channel_3" ) << "Connection name , \"Channel_3\"";
		EXPECT_TRUE( activeConfig.configChannelSet[1]->connectionType == RSSL_CONN_TYPE_ENCRYPTED) << "Connection type , \"RSSL_CONN_TYPE_ENCRYPTED\"";
		EXPECT_TRUE( encCfg->hostName == "122.1.1.100" ) << "hostname , \"122.1.1.100\"";
		EXPECT_TRUE( encCfg->serviceName == "14009" ) << "serviceName , \"14009\"";
		EXPECT_TRUE( encCfg->tcpNodelay == RSSL_TRUE) << "tcpNodelay , \"true\"";
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
