/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
{
	//APIQA
	if (refreshMsg.getDomainType() == 1)
	{
		cout << refreshMsg << "\nevent session info (refresh)\n" << endl;		// defaults to refreshMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << refreshMsg << "\nevent channel info (refresh)\n" << event.getChannelInformation() << endl;		// defaults to refreshMsg.toString()
	}
	//END APIQA
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
{
	//APIQA
	if (updateMsg.getDomainType() == 1)
	{
		cout << updateMsg << "\nevent session info (update)\n" << endl;		// defaults to updateMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << updateMsg << "\nevent channel info (update)\n" << event.getChannelInformation() << endl;		// defaults to updateMsg.toString()
	}
	//END APIQA
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
{
	//APIQA
	if (statusMsg.getDomainType() == 1)
	{
		cout << statusMsg << "\nevent session info (status)\n" << endl;		// defaults to statusMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << statusMsg << "\nevent channel info (status)\n" << event.getChannelInformation() << endl;		// defaults to statusMsg.toString()
	}
	//END APIQA	
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

		OmmConsumerConfig config;
		config.consumerName("Consumer_10").addServiceList(serviceList);

		OmmConsumer* consumer = new OmmConsumer(config, client);

		//OmmConsumer consumer( OmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList), client);
		consumer->registerClient( ReqMsg().serviceListName( "SVG1" ).name( "LSEG.L" ), client );
		consumer->registerClient( ReqMsg().serviceListName( "SVG1" ).name( "TRI.N" ), client);

		sleep(10000);
		cout << "\nAPIQA calls delete consumer!!\n" << endl;
		delete consumer;

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
