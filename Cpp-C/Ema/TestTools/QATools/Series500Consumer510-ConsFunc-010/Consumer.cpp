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

AppClient::AppClient() :
 postId( 1 )
{
}

void AppClient::setOmmConsumer( OmmConsumer& consumer )
{
	_pOmmConsumer = &consumer;
}

int main()
{
	try {
		AppClient client;
		//APIQA
		ServiceList serviceList("SVG1");

		serviceList.concreteServiceList().push_back("DIRECT_FEED");
		serviceList.concreteServiceList().push_back("DIRECT_FEED_2");

		//OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_10" ).username( "user" ) );
		OmmConsumer consumer(OmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList).username("user"),client);
		client.setOmmConsumer( consumer );
		void* closure1 = (void*)1;
		void* closure2 = (void*)2;

		// open login stream on which off stream posting will happen
		UInt64 handle1 = consumer.registerClient( ReqMsg().domainType( MMT_MARKET_PRICE ).serviceListName( "SVG1" ).name( "LSEG.L" ), client, closure1 );
		UInt64 handle2 = consumer.registerClient( ReqMsg().domainType( MMT_MARKET_PRICE ).serviceListName( "SVG1" ).name( "TRI.N" ), client, closure2);
		sleep(10000);
		cout << "\nAPIQA calls consumer.unregister!!\n" << endl;
		consumer.unregister(handle1);
		consumer.unregister(handle2);
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
