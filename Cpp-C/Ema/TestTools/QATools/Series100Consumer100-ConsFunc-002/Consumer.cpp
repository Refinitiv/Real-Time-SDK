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
	    //APIQA
		for (int i = 0; i < 1000000; i++)
		{
			AppClient client;
			cout << "!!! createOmmConsumer() " << i << " !!!" << endl;
			OmmConsumer *consumer = new OmmConsumer(OmmConsumerConfig().host("localhost:14002").username("user"));
			consumer->registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client);
			sleep(1000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			cout << "!!! ~OmmConsumer() " << i << " !!!" << endl;
			delete consumer;
		}
		//END APIQA
	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}
	return 0;
}
