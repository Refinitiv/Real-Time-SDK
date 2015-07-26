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

	if ( refreshMsg.hasMsgKey() )
		cout << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );

	// Reissue item's view after receiving the first refresh message for each request
	if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum &&
		refreshMsg.getState().getDataState() == OmmState::OkEnum &&
		( !item1Reissued || !item2Reissued ) )
	{
		if ( !item1Reissued && refreshMsg.getName() == "TRI.N" )
		{
			_pOmmConsumer->reissue( ReqMsg().name( refreshMsg.getName() ).payload( ElementList().addUInt( ":ViewType", 1 ).
				addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).complete() ).complete() ), ommEvent.getHandle() );
			item1Reissued = true;
		}

		if ( !item2Reissued && refreshMsg.getName() == "IBM.N" )
		{
			_pOmmConsumer->reissue( ReqMsg().name( refreshMsg.getName() ).payload( ElementList().addUInt( ":ViewType", 1 ).
				addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 32 ).complete() ).complete() ), ommEvent.getHandle() );
			item2Reissued = true;
		}
	}
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( updateMsg.hasMsgKey() )
		cout << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	if ( statusMsg.hasMsgKey() )
		cout << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

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
				cout << fe.getEnum() << endl;
				break;
			default:
				cout << endl;
				break;
			}
	}
}

AppClient::AppClient() :
 item1Reissued( false ),
 item2Reissued( false )
{
}

void AppClient::setOmmConsumer( OmmConsumer& ommConsumer )
{
	_pOmmConsumer = &ommConsumer;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		client.setOmmConsumer( consumer );
		void* closure = (void*)1;
		UInt64 handle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" )
			.payload( ElementList().addUInt( ":ViewType", 1 ).
				addArray( ":ItemList", OmmArray().addAscii( "TRI.N" ).addAscii( "IBM.N" ).complete() ).
				addArray( ":ViewData", OmmArray().fixedWidth( 4 ).addInt( 11 ).addInt( 22 ).addInt( 25 ).addInt( 32 ).complete() ).complete() ), client, closure );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
