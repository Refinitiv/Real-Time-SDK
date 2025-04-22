///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2024 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()

	printSessionStatus(event);
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
	printSessionStatus(event);
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
	printSessionStatus(event);
}

void AppClient::printSessionStatus(const refinitiv::ema::access::OmmConsumerEvent& event)
{
	EmaVector<ChannelInformation> statusVector; 
	
	event.getSessionInformation(statusVector);

	// Print out the channel information.
	for (UInt32 i = 0; i < statusVector.size(); ++i)
	{
		cout << statusVector[i] << endl;
	}
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		ServiceList serviceList("SVG1");

		serviceList.concreteServiceList().push_back("DIRECT_FEED");
		serviceList.concreteServiceList().push_back("DIRECT_FEED_2");

		OmmConsumer consumer( OmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList), client);
		consumer.registerClient( ReqMsg().serviceListName( "SVG1" ).name( "LSEG.L" ), client );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
