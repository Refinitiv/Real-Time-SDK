///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decode( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

	cout << "Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const RefreshMsg& refreshMsg )
{
	if ( refreshMsg.hasMsgKey() )
		cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg.getAttrib() );

	decode( refreshMsg.getPayload() );
}

void AppClient::decode( const UpdateMsg& updateMsg )
{
	if ( updateMsg.hasMsgKey() )
		cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName();

	decode( updateMsg.getAttrib() );

	decode( updateMsg.getPayload() );
}

void AppClient::decode( const Attrib& attrib )
{
	cout << endl << "Attribute";

	switch ( attrib.getDataType() )
	{
		case DataType::FieldListEnum :
			decode( attrib.getFieldList() );
			break;
		case DataType::MapEnum :
			decode( attrib.getMap() );
			break;
	}
}

void AppClient::decode( const Payload& payload )
{
	cout << endl << "Payload";

	switch ( payload.getDataType() )
	{
		case DataType::FieldListEnum :
			decode( payload.getFieldList() );
			break;
		case DataType::MapEnum :
			decode( payload.getMap() );
			break;
		case DataType::RefreshMsgEnum :
			decode( payload.getRefreshMsg() );
			break;
		case DataType::UpdateMsgEnum :
			decode( payload.getUpdateMsg() );
			break;
	}
}

void AppClient::decode( const Map& map )
{
	switch ( map.getSummary().getDataType() )
	{
	case DataType::FieldListEnum :
		decode( map.getSummary().getFieldList() );
		break;
	case DataType::MapEnum :
		decode( map.getSummary().getMap() );
		break;
	case DataType::RefreshMsgEnum :
		decode( map.getSummary().getRefreshMsg() );
		break;
	case DataType::UpdateMsgEnum :
		decode( map.getSummary().getUpdateMsg() );
		break;
	}

	while ( !map.forth() )
	{
		const MapEntry& me = map.getEntry();

		switch ( me.getKey().getDataType() )
		{
		case DataType::AsciiEnum :
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getAscii() << endl;
			break;
		case DataType::BufferEnum :
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getBuffer() << endl;
			break;
		}
		
		switch ( me.getLoadType() )
		{
		case DataType::FieldListEnum :
			decode( me.getFieldList() );
			break;
		case DataType::MapEnum :
			decode( me.getMap() );
			break;
		case DataType::RefreshMsgEnum :
			decode( me.getRefreshMsg() );
			break;
		case DataType::UpdateMsgEnum :
			decode( me.getUpdateMsg() );
			break;
		}
	}
}

void AppClient::decode( const FieldList& fl )
{
	while ( !fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Fid: " << fe.getFieldId() << " Name: " << fe.getName() << " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( fe.getLoadType() )
			{
			case DataType::FieldListEnum :
				cout << ",  contains FieldList." << endl;
				decode( fe.getFieldList() );
				break;
			case DataType::MapEnum :
				cout << ",  contains map." << endl;
				decode( fe.getMap() );
				break;
			case DataType::RefreshMsgEnum :
				cout << ",  contains refresh message." << endl;
				decode( fe.getRefreshMsg() );
				break;
			case DataType::UpdateMsgEnum :
				cout << ",  contains update message." << endl;
				decode( fe.getUpdateMsg() );
				break;
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

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		void* closure = (void*)1;
		// request a custom domain (133) item IBM.XYZ
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( 133 ).serviceName( "DIRECT_FEED" ).name( "IBM.XYZ" ), client, closure );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
