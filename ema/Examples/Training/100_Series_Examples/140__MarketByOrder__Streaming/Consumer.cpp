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

void AppClient::decode( const FieldList& fl )
{
	while ( !fl.forth() )
		cout << "Fid: " << fl.getEntry().getFieldId() << " Name: " << fl.getEntry().getName() << " Value: " << fl.getEntry().getLoad().toString() << endl;
}

void AppClient::decode( const Map& map )
{
	if ( map.getSummary().getDataType() == DataType::FieldListEnum )
	{
		cout << "Map Summary data:" << endl;
		decode( map.getSummary().getFieldList() );
	}
		
	while ( !map.forth() )
	{
		const MapEntry& me = map.getEntry();

		if ( me.getKey().getDataType() == DataType::BufferEnum )
			cout << "Action: " << me.getMapActionAsString() << " key value: " << me.getKey().getBuffer() << endl;

		if ( me.getLoadType() == DataType::FieldListEnum )
		{
			cout << "Entry data:" << endl;
			decode( me.getFieldList() );
		}
	}
}	

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		consumer.registerClient( ReqMsg().domainType( MMT_MARKET_BY_ORDER ).serviceName( "DIRECT_FEED" ).name( "AAO.V" ), client );
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
