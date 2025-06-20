///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
//APIQA
#include <cstring>
//END APIQA
using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//APIQA
UInt64 pUpdateTypeFilter = 0;
UInt64 pNegativeUpdateTypeFilter = 0;
UInt64 fUpdateTypeFilter = 0;
UInt64 fNegativeUpdateTypeFilter = 0;
UInt64 clmUpdateTypeFilter = 0;
UInt64 clmNegativeUpdateTypeFilter = 0;
bool utfFuncClm = true;
bool nutfFuncClm = true;
bool setCLM = false;
bool programmaticConfig = false;
EmaString serviceName = "DIRECT_FEED";
EmaString itemName = "IBM.N";
EmaString progHost = "localhost";
EmaString progPort = "14002";

//END APIQA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		cout << "Name: " << fe.getName() << " Value: ";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( fe.getLoadType() )
		{
			case DataType::RealEnum:
				cout << fe.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum:
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum:
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum:
				cout << fe.getInt() << endl;
				break;
			case DataType::UIntEnum:
				cout << fe.getUInt() << endl;
				break;
			case DataType::AsciiEnum:
				cout << fe.getAscii() << endl;
				break;
			case DataType::ErrorEnum:
				cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum:
				fe.hasEnumDisplay() ? cout << fe.getEnumDisplay() << endl : cout << fe.getEnum() << endl;
				break;
			case DataType::RmtesEnum:
				cout << fe.getRmtes().toString() << endl;
				break;
			default:
				cout << endl;
				break;
		}
	}
}

// APIQA
void createProgramaticConfig(Map& configMap)
{
	Map innerMap;
	ElementList elementList, elementList1;

	elementList.addAscii("DefaultConsumer", "Consumer_UTF");

	elementList1.addAscii("Channel", "Channel_1");
	elementList1.addAscii("Logger", "Logger_1");
	elementList1.addAscii("Dictionary", "Dictionary_1");
	elementList1.addUInt("ItemCountHint", 5000);
	elementList1.addUInt("ServiceCountHint", 5000);
	elementList1.addUInt("ObeyOpenWindow", 0);
	elementList1.addUInt("PostAckTimeout", 5000);
	elementList1.addUInt("RequestTimeout", 5000);
	elementList1.addUInt("MaxOutstandingPosts", 5000);
	elementList1.addInt("DispatchTimeoutApiThread", 100);
	elementList1.addUInt("HandleException", 0);
	elementList1.addUInt("MaxDispatchCountApiThread", 500);
	elementList1.addUInt("MaxDispatchCountUserThread", 500);
	elementList1.addInt("ReconnectAttemptLimit", 10);
	elementList1.addInt("ReconnectMinDelay", 2000);
	elementList1.addInt("ReconnectMaxDelay", 6000);
	elementList1.addAscii("XmlTraceFileName", "MyXMLTrace");
	elementList1.addInt("XmlTraceMaxFileSize", 50000000);
	elementList1.addUInt("XmlTraceToFile", 1);
	elementList1.addUInt("XmlTraceToStdout", 0);
	elementList1.addUInt("XmlTraceToMultipleFiles", 1);
	elementList1.addUInt("XmlTraceWrite", 1);
	elementList1.addUInt("XmlTraceRead", 1);
	elementList1.addUInt("XmlTracePing", 1);
	elementList1.addUInt("XmlTraceHex", 1);
	elementList1.addUInt("XmlTracePingOnly", 0);
	elementList1.addUInt("MsgKeyInUpdates", 1);
	
	if (pUpdateTypeFilter)
		elementList1.addUInt("UpdateTypeFilter", pUpdateTypeFilter);

	if (pNegativeUpdateTypeFilter)
		elementList1.addUInt("NegativeUpdateTypeFilter", pNegativeUpdateTypeFilter);

	innerMap.addKeyAscii("Consumer_UTF", MapEntry::AddEnum,
		elementList1.complete()).complete();

	elementList.addMap("ConsumerList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("InterfaceName", "localhost")
		.addEnum("CompressionType", 1)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", progHost)
		.addAscii("Port", progPort)
		.addUInt("TcpNodelay", 1).complete()).complete();

	elementList.addMap("ChannelList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	innerMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("LoggerType", 0)
		.addAscii("FileName", "logFile")
		.addEnum("LoggerSeverity", 1).complete()).complete();

	elementList.addMap("LoggerList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("DictionaryType", 0)
		.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
		.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

	elementList.addMap("DictionaryList", innerMap);

	elementList.complete();

	configMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	configMap.complete();
}

void printHelp()
{
	printf("\nOptions:\n" "  -?\tShows this usage\n"
		"  -putf UpdateTypeFilter in programmatic config.\n"
		"  -pnutf NegativeUpdateTypeFilter in programmatic config.\n"
		"  -futf UpdateTypeFilter via functional call.\n"
		"  -fnutf NegativeUpdateTypeFilter via functional call.\n"
		"  -clmutf UpdateTypeFilter via custom Login message.\n"
		"  -clmnutf NegativeUpdateTypeFilter via custom Login message.\n"
		"  -utfFuncClm UpdateTypeFilter setting via functional call will precede setting the message.\n"
		"  -nutfFuncClm NegativeUpdateTypeFilter setting via functional call will precede setting the message.\n"
		"  -utfClmFunc UpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
		"  -nutfClmFunc NegativeUpdateTypeFilter setting via functional call will take place after setting the custom Login message.\n"
		"  -setCLM if present, custom LoginRequest message will be set via config interface, otherwise it won't be set.\n"
		"  -service set service name. Default is DIRECT_FEED. \n"
		"  -item set item name. Default is IBM.N. \n"
		"  -progConf enable programmatic config. Default is disable. \n"
		"  -progHost set host for programmatic config. Default is localhost. \n"
		"  -progPort set port for programmatic config. Default is 14002. \n"
		"\n");
	printf("For instance, the following arguments \"-putf 1 -pnutf 2 -futf 4 -fnutf 8 -clmutf 16 -clmnutf 32 -utfFuncClm -nutfClmFunc\""
		" indicate that UpdateTypeFilter set via programmatic config equals 1, \n"
		" NegativeUpdateTypeFilter set via programmatic config equals 2, \n"
		" UpdateTypeFilter set via functional call equals 4, \n"
		" NegativeUpdateTypeFilter set via functional call equals 8, \n"
		" UpdateTypeFilter set via custom login message equals 16, \n"
		" NegativeUpdateTypeFilter set via custom login message equals 32, \n"
		" setting UpdateTypeFilter via functional call will precede setting custom login message with this value set, "
		" and setting custom login message with NegativeUpdateTypeFilter will precede setting it via function call.");
}

int main( int argc, char* argv[])
{
	try {
		
		int i = 1;

		while (i < argc)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				break;
			}
			else if (strcmp(argv[i], "-putf") == 0)
			{
				i++;
				pUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-pnutf") == 0)
			{
				i++;
				pNegativeUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-futf") == 0)
			{
				i++;
				fUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-fnutf") == 0)
			{
				i++;
				fNegativeUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-clmutf") == 0)
			{
				i++;
				clmUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-clmnutf") == 0)
			{
				i++;
				clmNegativeUpdateTypeFilter = atoi(argv[i]);
			}
			else if (strcmp(argv[i], "-utfFuncClm") == 0)
			{
				utfFuncClm = true;
			}
			else if (strcmp(argv[i], "-nutfFuncClm") == 0)
			{
				nutfFuncClm = true;
			}
			else if (strcmp(argv[i], "-utfClmFunc") == 0)
			{
				utfFuncClm = false;
			}
			else if (strcmp(argv[i], "-nutfClmFunc") == 0)
			{
				nutfFuncClm = false;
			}
			else if (strcmp(argv[i], "-setCLM") == 0)
			{
				setCLM = true;
			}
			else if (strcmp(argv[i], "-service") == 0)
			{
				i++;
				serviceName = argv[i];
			}
			else if (strcmp(argv[i], "-item") == 0)
			{
				i++;
				itemName = argv[i];
			}
			else if (strcmp(argv[i], "-progConf") == 0)
			{
				programmaticConfig = true;
			}
			else if (strcmp(argv[i], "-progHost") == 0)
			{
				i++;
				progHost = argv[i];
			}
			else if (strcmp(argv[i], "-progPort") == 0)
			{
				i++;
				progPort = argv[i];
			}
			else
			{
				printf("Used unsupported argument: %s \n", argv[i]);
				printHelp();
				break;
			}
			i++;
		}
		
		AppClient client;

		// Modify Administrative domains with ReqMsg to override default configurations
		
		Map configMap;
		Login::LoginReq loginReq;
		OmmConsumerConfig  config;

		if (programmaticConfig)
		{
			createProgramaticConfig(configMap);
			config.operationModel(OmmConsumerConfig::UserDispatchEnum).config(configMap);
		}
		else
			config.operationModel(OmmConsumerConfig::UserDispatchEnum);

		loginReq.name("user")
			.applicationId("127")
			.position("127.0.0.1/net")
			.allowSuspectData(true);

		if (clmUpdateTypeFilter) loginReq.updateTypeFilter(clmUpdateTypeFilter);
		if (clmNegativeUpdateTypeFilter) loginReq.negativeUpdateTypeFilter(clmNegativeUpdateTypeFilter);

		if (fUpdateTypeFilter && utfFuncClm) config.updateTypeFilter(fUpdateTypeFilter);
		if (fNegativeUpdateTypeFilter && nutfFuncClm) config.negativeUpdateTypeFilter(fNegativeUpdateTypeFilter);
		if (setCLM) config.addAdminMsg(loginReq.getMessage());
		if (fUpdateTypeFilter && !utfFuncClm) config.updateTypeFilter(fUpdateTypeFilter);
		if (fNegativeUpdateTypeFilter && !nutfFuncClm) config.negativeUpdateTypeFilter(fNegativeUpdateTypeFilter);

		OmmConsumer consumer (config);
		consumer.registerClient(ReqMsg().serviceName(serviceName).name(itemName), client);

		unsigned long long startTime = getCurrentTime();
		while (startTime + 60000 > getCurrentTime())
			consumer.dispatch(10);			// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
