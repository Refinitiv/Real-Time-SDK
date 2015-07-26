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

	if ( refreshMsg.hasMsgKey() )
		cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Update. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( updateMsg.hasMsgKey() )
		cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Received Status. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();
	
	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	RmtesBuffer rmtesBuffer;
	
	while ( !fl.forth( "BCAST_TEXT" ) )
	{
		const FieldEntry& fe = fl.getEntry();

		cout << "Fid: " << fe.getFieldId() << " Name: " << fe.getName() << " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";

		rmtesBuffer.apply( fe.getRmtes() );
		cout << rmtesBuffer.toString() << endl;
	}
}

AppClient::AppClient() :
 hasFirstRefresh( false )
{
}

void AppClient::setOmmConsumer( OmmConsumer & ommConsumer )
{
	_pOmmConsumer = &ommConsumer;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).username( "user" ) );
		client.setOmmConsumer( consumer );
		void* closure = (void*)1;
		UInt64 handle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "N2_UBMS" ), client, closure );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
