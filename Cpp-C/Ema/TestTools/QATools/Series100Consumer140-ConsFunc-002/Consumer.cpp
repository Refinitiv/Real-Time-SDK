///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	//APIQA
        cout << endl << "Clone Refresh Msg" << endl;
        RefreshMsg cloneRefreshMsg(refreshMsg);
        cout << endl << "Item Name: " << ( cloneRefreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (cloneRefreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << cloneRefreshMsg.getState().toString() << endl;

	if ( DataType::MapEnum == cloneRefreshMsg.getPayload().getDataType() )
		decode( cloneRefreshMsg.getPayload().getMap() );
	//END APIQA
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	//APIQA
        cout << endl << "Clone Update Msg" << endl;
        UpdateMsg cloneUpdateMsg(updateMsg);
	cout << endl << "Item Name: " << ( cloneUpdateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (cloneUpdateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( DataType::MapEnum == cloneUpdateMsg.getPayload().getDataType() )
		decode( cloneUpdateMsg.getPayload().getMap() );
	//END APIQA
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
		cout << "Fid: " << fl.getEntry().getFieldId() << " Name: " << fl.getEntry().getName() << " Value: " << fl.getEntry().getLoad().toString() << endl;
}

void AppClient::decode( const Map& map )
{
	if ( map.getSummaryData().getDataType() == DataType::FieldListEnum )
	{
		cout << "Map Summary data:" << endl;
		decode( map.getSummaryData().getFieldList() );
	}
		
	while ( map.forth() )
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

int main()
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
