///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::OpaqueEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getOpaque() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( DataType::OpaqueEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getOpaque() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const OmmOpaque& oq )
{
	cout << endl<<  "OmmOpaque data: " << oq.getAsHex() << endl;
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( 200 ).serviceName( "DIRECT_FEED" ).name( "IBM.XYZ" ), client );
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
