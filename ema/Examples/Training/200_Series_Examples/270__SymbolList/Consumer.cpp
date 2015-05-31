///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	if ( refreshMsg.hasMsgKey() )
		cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::MapEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getMap() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	if ( updateMsg.hasMsgKey() )
		cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

	if ( DataType::MapEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getMap() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl, bool newLine )
{
	while ( !fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		cout << fe.getName() << "\t";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank";
		else
			switch ( fe.getLoadType() )
			{
			case DataType::RealEnum:
				cout << fe.getReal().getAsDouble();
				break;
			case DataType::DateEnum:
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear();
				break;
			case DataType::TimeEnum:
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond();
				break;
			case DataType::IntEnum:
				cout << fe.getInt();
				break;
			case DataType::UIntEnum:
				cout << fe.getUInt();
				break;
			case DataType::AsciiEnum:
				cout << fe.getAscii();
				break;
			case DataType::RmtesEnum :
				cout << fe.getRmtes().toString();
				break;
			case DataType::EnumEnum:
				cout << fe.getEnum();
				break;
			case DataType::ErrorEnum:
				cout << "( " << fe.getError().getErrorCodeAsString() << " )";
				break;
			default:
				break;
			}

		if ( newLine ) cout << endl;
	}
}

void AppClient::decode( const Map& map )
{
	if ( map.getSummary().getDataType() == DataType::FieldListEnum )
	{
		cout << endl << "Summary :" << endl;
		decode( map.getSummary().getFieldList(), true );
	}

	bool firstEntry = true;

	while ( !map.forth() )
	{
		if ( firstEntry )
		{
			firstEntry = false;
			cout << endl << "Name\tAction" << endl << endl;
		}

		const MapEntry& me = map.getEntry();

		switch ( me.getKey().getDataType() )
		{
		case DataType::BufferEnum :
			{
				const EmaBuffer& buf = me.getKey().getBuffer();
				cout << EmaString( buf.c_buf(), buf.length() ) << "\t" << me.getMapActionAsString();
			}
			break;
		case DataType::AsciiEnum :
			cout << me.getKey().getAscii() << "\t" << me.getMapActionAsString();
			break;
		case DataType::RmtesEnum :
			cout << me.getKey().getRmtes().toString() << "\t" << me.getMapActionAsString();
			break;
		}

		if ( me.getLoadType() == DataType::FieldListEnum )
		{
			cout << "\t";
			decode( me.getFieldList(), false );
		}

		cout << endl;
	}
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( MMT_SYMBOL_LIST ).serviceName( "ELEKTRON_DD" ).name( ".AV.N" ), client );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
