///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		ChannelInformation channelInfo;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ).consumerName("Consumer_9"), client);
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "TRI.N" ), client );
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED1" ).name( "IBM.N" ), client );

		for (int i = 1; i < 60; i++)
		{
			consumer.getChannelInformation(channelInfo);
			cout << channelInfo << endl;
			channelInfo.clear();
			sleep(1000);
		}
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
