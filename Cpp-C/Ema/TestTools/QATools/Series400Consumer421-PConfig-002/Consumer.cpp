///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------


#include "Consumer.h"
using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	if ( refreshMsg.hasMsgKey() )
		cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	if ( updateMsg.hasMsgKey() )
		cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Fid: " << fe.getFieldId() << " Name: " << fe.getName() << " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( fe.getLoadType() )
			{
			case DataType::RealEnum :
				cout << fe.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum :
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum :
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum :
				cout << fe.getInt() << endl;
				break;
			case DataType::UIntEnum :
				cout << fe.getUInt() << endl;
				break;
			case DataType::AsciiEnum :
				cout << fe.getAscii() << endl;
				break;
			case DataType::ErrorEnum :
				cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum :
				cout << fe.getEnum() << endl;
				break;
			default :
				cout << endl;
				break;
			}
	}
}

//APIQA
void createProgramaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;
	
	elementList.addAscii( "DefaultConsumer", "Consumer_1" );

	innerMap.addKeyAscii( "Consumer_1", MapEntry::AddEnum,
		ElementList()
		.addAscii( "ChannelSet", "Channel_1,Channel_2" )
		.addAscii( "Logger", "Logger_1" )
		.addAscii( "Dictionary", "Dictionary_1" )
		.addUInt( "ItemCountHint", 5000 )
		.addUInt( "ServiceCountHint", 5000 )
		.addUInt( "ObeyOpenWindow", 0 )
		.addUInt( "PostAckTimeout", 5000 )
		.addUInt( "RequestTimeout", 5000 )
		.addUInt( "MaxOutstandingPosts", 5000 )
		.addInt( "DispatchTimeoutApiThread", 100 )
		.addUInt( "HandleException", 0 )
		.addUInt( "MaxDispatchCountApiThread", 500 )
		.addUInt( "MaxDispatchCountUserThread", 500 )
		.addInt("ReconnectAttemptLimit", 10)
		.addInt( "ReconnectMinDelay", 2000 )
		.addInt( "ReconnectMaxDelay", 6000 )
		.addAscii( "XmlTraceFileName", "MyXMLTrace" )
		.addInt( "XmlTraceMaxFileSize", 50000000 )
		.addUInt( "XmlTraceToFile", 1 )
		.addUInt( "XmlTraceToStdout", 0 )
		.addUInt( "XmlTraceToMultipleFiles", 1 )
		.addUInt( "XmlTraceWrite", 1 )
		.addUInt( "XmlTraceRead", 1 )
		.addUInt( "MsgKeyInUpdates", 1 )
		.addInt( "PipePort", 4001 ).complete() ).complete();

	elementList.addMap( "ConsumerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "ConsumerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	innerMap.addKeyAscii( "Channel_1", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", 0 )
		.addEnum("CompressionType", 1)
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt( "ConnectionPingTimeout", 50000 )
		.addAscii( "Host", "channel1_host" )
		.addAscii("Port", "channel1_port" )
		.addUInt( "TcpNodelay", 0 ).complete() )

	    .addKeyAscii( "Channel_2", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", channel2Type_input )
		.addEnum("CompressionType", 0)
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt( "ConnectionPingTimeout", 50000 )
		.addAscii( "Host", "channel2_host" )
		.addAscii("Port", "channel2_port" )
		.addUInt( "TcpNodelay", 0 ).complete() ).complete();

	elementList.addMap( "ChannelList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "ChannelGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	innerMap.addKeyAscii( "Logger_1", MapEntry::AddEnum,
		ElementList()
		.addEnum( "LoggerType", 1 )
		.addAscii( "FileName", "logFile" )
		.addEnum( "LoggerSeverity", 0 ).complete() ).complete();

	elementList.addMap( "LoggerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "LoggerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	innerMap.addKeyAscii( "Dictionary_1", MapEntry::AddEnum,
		ElementList()
		.addEnum( "DictionaryType", 1 )
		.addAscii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary" )
		.addAscii( "EnumTypeDefFileName", "./enumtype.def" ).complete() ).complete();

	elementList.addMap( "DictionaryList", innerMap );
		
	elementList.complete();

	configMap.addKeyAscii( "DictionaryGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	configMap.complete();
}
//END APIQA
int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		Map configMap;
		createProgramaticConfig( configMap );
		OmmConsumer consumer( OmmConsumerConfig().config( configMap ) );		// use programmatic configuration parameters
		consumer.registerClient( ReqMsg().name( "IBM.N" ).serviceName( "DIRECT_FEED" ), client );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
