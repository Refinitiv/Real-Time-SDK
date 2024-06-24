///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

bool updateCalled = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event ) 
{
	cout << endl << refreshMsg << endl << "event channel info (refresh)" 
	  << endl << event.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event ) 
{
	if ( !updateCalled )
	{
		updateCalled = true;
		cout << endl << updateMsg << endl << "event channel info (update)"
			<< endl << event.getChannelInformation() << endl;
	}
	else
		cout << "skipped printing updateMsg" << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event ) 
{
	cout << endl << statusMsg << endl << "event channel info (status)" << endl << event.getChannelInformation();
}

int main()
{
	try { 
		AppClient client;
		ChannelInformation channelInfo;
		OmmConsumer consumer( OmmConsumerConfig( "EmaConfig.xml" ).username( "user" ) );
		consumer.getChannelInformation( channelInfo );
		cout << endl << "channel information (consumer):" << endl << channelInfo << endl;

		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
	  cout << excp << endl;
	}
	return 0;
}
