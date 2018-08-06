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
	cout << "Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const RefreshMsg& refreshMsg )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg.getAttrib() );

	decode( refreshMsg.getPayload() );
}

void AppClient::decode( const UpdateMsg& updateMsg )
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

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
	switch ( map.getSummaryData().getDataType() )
	{
	case DataType::FieldListEnum :
		decode( map.getSummaryData().getFieldList() );
		break;
	case DataType::MapEnum :
		decode( map.getSummaryData().getMap() );
		break;
	case DataType::RefreshMsgEnum :
		decode( map.getSummaryData().getRefreshMsg() );
		break;
	case DataType::UpdateMsgEnum :
		decode( map.getSummaryData().getUpdateMsg() );
		break;
	}

	while ( map.forth() )
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
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Name: " << fe.getName() << " Value: ";

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
				fe.hasEnumDisplay() ? cout << fe.getEnumDisplay() << endl : cout << fe.getEnum() << endl;
				break;
			case DataType::RmtesEnum:
				cout << fe.getRmtes().toString() << endl;
				break;
			default :
				cout << endl;
				break;
			}
	}
}

int main()
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
