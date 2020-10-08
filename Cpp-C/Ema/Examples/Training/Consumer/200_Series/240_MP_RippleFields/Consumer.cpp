///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent ) 
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	cout << "Item Handle: " << ommEvent.getHandle() << endl << "Item Closure: " << ommEvent.getClosure() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent ) 
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	cout << "Item Handle: " << ommEvent.getHandle() << endl << "Item Closure: " << ommEvent.getClosure() << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent ) 
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;

	cout << "Item Handle: " << ommEvent.getHandle() << endl << "Item Closure: " << ommEvent.getClosure() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		switch ( fe.getLoadType() )
		{
			case DataType::RealEnum:
			if ( fe.getFieldId() == 22 )	// Display data for BID field name and its ripple fields
			{
				if ( fe.getRippleTo(fe.getRippleTo()) == 24 )
					BID_2 = BID_1;

				if ( fe.getRippleTo() == 23 )
					BID_1 = BID;

				BID = fe.getReal().getAsDouble();

				cout << "DataType: " << DataType( fe.getLoad().getDataType() ) << endl;
				cout << "Name: " << fe.getName() << " (" << fe.getFieldId() << ") Value: " << BID << endl;
				cout << "Name: " << fe.getRippleToName() << " (" << fe.getRippleTo() << ") Value: " << BID_1 << endl;
				cout << "Name: " << fe.getRippleToName(fe.getRippleTo()) << " (" << fe.getRippleTo(fe.getRippleTo()) << 
					") Value: " << BID_2 << endl;
			}
			else if ( fe.getFieldId() == 25 ) // Display data for ASK field name and its ripple fields
			{
				if ( fe.getRippleTo(fe.getRippleTo()) == 27 )
					ASK_2 = ASK_1;

				if ( fe.getRippleTo() == 26 )
					ASK_1 = ASK;

				ASK = fe.getReal().getAsDouble();

				cout << "DataType: " << DataType( fe.getLoad().getDataType() ) << endl;
				cout << "Name: " << fe.getName() << " (" << fe.getFieldId() << ") Value: " << ASK << endl;
				cout << "Name: " << fe.getRippleToName() << " (" << fe.getRippleTo() << ") Value: " << ASK_1 << endl;
				cout << "Name: " << fe.getRippleToName(fe.getRippleTo()) << " (" << fe.getRippleTo(fe.getRippleTo()) <<
					") Value: " << ASK_2 << endl;
			}
			break;
		}
	}
}

AppClient::AppClient() :
 BID( 0 ), BID_1( 0 ), BID_2( 0 ), ASK( 0 ), ASK_1( 0 ), ASK_2( 0 )
{
}

int main()
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).host( "localhost:14002" ).username( "user" ) );
		void* closure = (void*)1;
		UInt64 handle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
