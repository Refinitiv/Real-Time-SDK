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
	cout << "Received Refresh. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeRefreshMsg( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Received Update. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeUpdateMsg( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Received Status. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeStatusMsg( statusMsg );
}

void AppClient::decodeRefreshMsg( const RefreshMsg& refreshMsg )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	cout << "Item State: " << refreshMsg.getState().toString() << endl;

	cout << "Attribute" << endl;
	decode( refreshMsg.getAttrib().getData() );

	cout << "Payload" << endl;
	decode( refreshMsg.getPayload().getData() );
}

void AppClient::decodeUpdateMsg( const UpdateMsg& updateMsg )
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	cout << "Attribute" << endl;
	decode( updateMsg.getAttrib().getData() );

	cout << "Payload" << endl;
	decode( updateMsg.getPayload().getData() );
}

void AppClient::decodeStatusMsg( const StatusMsg& statusMsg )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const Data& data )
{
	if ( data.getCode() == Data::BlankEnum )
		cout << "Blank data" << endl;
	else
		switch ( data.getDataType() )
		{
		case DataType::RefreshMsgEnum :
			decodeRefreshMsg( static_cast<const RefreshMsg&>( data ) );
			break;
		case DataType::UpdateMsgEnum :
			decodeUpdateMsg( static_cast<const UpdateMsg&>( data ) );
			break;
		case DataType::StatusMsgEnum :
			decodeStatusMsg( static_cast<const StatusMsg&>( data ) );
			break;
		case DataType::FieldListEnum :
			decodeFieldList( static_cast<const FieldList&>( data ) );
			break;
		case DataType::MapEnum :
			decodeMap( static_cast<const Map&>( data ) );
			break;
		case DataType::NoDataEnum :
			cout << "NoData" << endl;
			break;
		case DataType::TimeEnum :
			cout << "OmmTime: " << static_cast<const OmmTime&>( data ).toString() << endl;
			break;
		case DataType::DateEnum :
			cout << "OmmDate: " << static_cast<const OmmDate&>( data ).toString() << endl;
			break;
		case DataType::RealEnum :
			cout << "OmmReal::getAsDouble: " << static_cast<const OmmReal&>( data ).getAsDouble() << endl;
			break;
		case DataType::IntEnum :
			cout << "OmmInt: " << static_cast<const OmmInt&>( data ).getInt() << endl;
			break;
		case DataType::UIntEnum :
			cout << "OmmUInt: " << static_cast<const OmmUInt&>( data ).getUInt() << endl;
			break;
		case DataType::EnumEnum :
			cout << "OmmEnum: " << static_cast<const OmmEnum&>( data ).getEnum() << endl;
			break;
		case DataType::AsciiEnum :
			cout << "OmmAscii: " << static_cast<const OmmAscii&>( data ).toString() << endl;
			break;
		case DataType::ErrorEnum :
			cout << "Decoding error: " << static_cast<const OmmError&>( data ).getErrorCodeAsString() << endl;
			break;
		default :
			break;
		}
}

void AppClient::decodeMap( const Map& map )
{
	cout << "Map Summary" << endl;
	decode( map.getSummaryData().getData() );

	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		cout << "Action = " << me.getMapActionAsString() << endl;

		cout << "Key" << endl;
		decode( me.getKey().getData() );

		cout << "Load" << endl;
		decode( me.getLoad() );
	}
}

void AppClient::decodeFieldList( const FieldList& fl )
{
	if ( fl.hasInfo() )
		cout << "FieldListNum: " << fl.getInfoFieldListNum() << " DictionaryId: " << fl.getInfoDictionaryId() << endl;

	while ( fl.forth() )
	{
		cout << "Load" << endl;
		decode( fl.getEntry().getLoad() );
	}
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).host( "localhost:14002" ).username( "user" ) );
		void* closure = (void*)1;
		// request a custom domain (133) item IBM.XYZ
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( 133 ).serviceName( "DIRECT_FEED" ).name( "IBM.XYZ" ), client, closure );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
