///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019-2024 LSEG. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()

	printSessionStatus(event);
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
	printSessionStatus(event);
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
	printSessionStatus(event);
}

void AppClient::printSessionStatus(const refinitiv::ema::access::OmmConsumerEvent& event)
{
	EmaVector<ChannelInformation> statusVector;

	event.getSessionInformation(statusVector);

	// Print out the channel information.
	for (UInt32 i = 0; i < statusVector.size(); ++i)
	{
		cout << statusVector[i] << endl;
	}
}

void AppClient::decode(const FieldList& fl)
{
	while (fl.forth())
	{
		const FieldEntry& fe = fl.getEntry();

		cout << "Name: " << fe.getName() << " Value: ";

		if (fe.getCode() == Data::BlankEnum)
			cout << " blank" << endl;
		else
			switch (fe.getLoadType())
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

void createProgramaticConfig(Map& configMap)
{
	Map innerMap;
	ElementList elementList;

	elementList.addAscii("DefaultConsumer", "Consumer_1");

	innerMap.addKeyAscii("Consumer_1", MapEntry::AddEnum,
		ElementList()
		.addAscii("SessionChannelSet", "Connection_1, Connection_2")
		.addAscii("Logger", "Logger_1")
		.addAscii("Dictionary", "Dictionary_1")
		.addUInt("ItemCountHint", 5000)
		.addUInt("ServiceCountHint", 5000)
		.addUInt("ObeyOpenWindow", 0)
		.addUInt("PostAckTimeout", 5000)
		.addUInt("RequestTimeout", 5000)
		.addUInt("MaxOutstandingPosts", 5000)
		.addInt("DispatchTimeoutApiThread", 100)
		.addUInt("HandleException", 0)
		.addUInt("MaxDispatchCountApiThread", 500)
		.addUInt("MaxDispatchCountUserThread", 500)
		.addInt("PipePort", 4001)
		.addInt("ReconnectAttemptLimit", 10)
		.addInt("ReconnectMinDelay", 2000)
		.addInt("ReconnectMaxDelay", 6000)
		.addAscii("XmlTraceFileName", "MyXMLTrace")
		.addInt("XmlTraceMaxFileSize", 50000000)
		.addUInt("XmlTraceToFile", 1)
		.addUInt("XmlTraceToStdout", 0)
		.addUInt("XmlTraceToMultipleFiles", 1)
		.addUInt("XmlTraceWrite", 1)
		.addUInt("XmlTraceRead", 1)
		.addUInt("XmlTracePing", 1)
		.addUInt("XmlTraceHex", 1)
		.addUInt("XmlTracePingOnly", 0)
		.addUInt("MsgKeyInUpdates", 1)
		.addUInt("SessionEnhancedItemRecovery", 1).complete()).complete();

	elementList.addMap("ConsumerList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	innerMap.addKeyAscii("Connection_1", MapEntry::AddEnum,
		ElementList()
		.addAscii("ChannelSet", "Channel_1, Channel_2")
		.addInt("ReconnectAttemptLimit", 4)
		.addInt("ReconnectMinDelay", 2000)
		.addInt("ReconnectMaxDelay", 6000).complete());

	innerMap.addKeyAscii("Connection_2", MapEntry::AddEnum,
		ElementList()
		.addAscii("ChannelSet", "Channel_10, Channel_11")
		.addInt("ReconnectAttemptLimit", 4)
		.addInt("ReconnectMinDelay", 2000)
		.addInt("ReconnectMaxDelay", 6000).complete()).complete();

	elementList.addMap("SessionChannelList", innerMap);
	
	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("SessionChannelGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("InterfaceName", "localhost")
		.addEnum("CompressionType", 1)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14002")
		.addUInt("TcpNodelay", 0).complete());

	innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("InterfaceName", "localhost")
		.addEnum("CompressionType", 1)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14003")
		.addUInt("TcpNodelay", 0).complete());


	innerMap.addKeyAscii("Channel_10", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("InterfaceName", "localhost")
		.addEnum("CompressionType", 1)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14004")
		.addUInt("TcpNodelay", 0).complete());

	innerMap.addKeyAscii("Channel_11", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("InterfaceName", "localhost")
		.addEnum("CompressionType", 1)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14005")
		.addUInt("TcpNodelay", 0).complete()).complete();

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

int main()
{
	try {
		AppClient client;
		ServiceList serviceList("SVG1");

		serviceList.concreteServiceList().push_back("DIRECT_FEED");
		serviceList.concreteServiceList().push_back("DIRECT_FEED_2");

		Map configMap;
		createProgramaticConfig(configMap);
		OmmConsumer consumer(OmmConsumerConfig().config(configMap).addServiceList(serviceList), client);		// use programmatic configuration parameters
		consumer.registerClient(ReqMsg().name("IBM.N").serviceListName("SVG1"), client);
		sleep(60000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}
	return 0;
}
