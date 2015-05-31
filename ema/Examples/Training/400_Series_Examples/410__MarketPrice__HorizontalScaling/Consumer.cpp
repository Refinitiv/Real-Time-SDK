///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include "ConsumerManager.h"

using namespace thomsonreuters::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( refreshMsg.hasMsgKey() )
		cout << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << ( refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	cout << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( updateMsg.hasMsgKey() )
		cout << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << ( updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( statusMsg.hasMsgKey() )
		cout << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << ( statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << endl << "item State: " << statusMsg.getState().toString() << endl;
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

int main( int argc, char* argv[] )
{
	try {
		// Create two ConsumerManager objects to demonstrate horizontal scaling feature on user thread of control
		ConsumerManager consumerMgr1( "localhost:14002", "user1" );
		ConsumerManager consumerMgr2( "localhost:14002", "user2" );
		
		AppClient appClient1;
		AppClient appClient2;

		consumerMgr1.getOmmConsumer().registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), appClient1, (void *)"consumerMgr1" );
		consumerMgr2.getOmmConsumer().registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "TRI.N" ), appClient2, (void *)"consumerMgr2" );

		// Start dispatching messages from each ConsumerManager
		consumerMgr1.start();
		consumerMgr2.start();

		sleep( 60000 );				// User thread calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()

		consumerMgr1.stop();
		consumerMgr2.stop();
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
