///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

int totalRefreshReceived = 0;
void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if (++totalRefreshReceived == 14)
		cout << endl << "Received total " << totalRefreshReceived << " refresh msgs" << endl;

}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{

}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}


/*need to run rsslProvider app with only send refresh without updates
* set logger level as Verbose level, turn off XmlTrace
* In ItemCallbackClient.cpp, sets CONSUMER_STARTING_STREAM_ID = 2147483636
* run 4 minutes
* validate the following:
* capture "Reach max number available for next stream id, will wrap around"
* capture 4 status msg with text "Batch request acknowledged."
* capture 2 msg with text "StreamId 2147483637 from ItemList" 
* capture 2 msg with text "StreamId 2147483637 to ItemList"
* capture msg text with "Received total 14 refresh msgs"
*/

int main(int argc, char* argv[])
{
	try {
		AppClient client;

		int requiredNum = 14;
		int numItems = 1;
		bool snapshot = false;
		OmmConsumer consumer(OmmConsumerConfig().username("user"));
		EmaString itemName;
		while (numItems <= requiredNum)
		{
			if (snapshot)
				snapshot = false;
			else
				snapshot = true;

			if (snapshot)
			{
				consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").interestAfterRefresh(false).payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray()
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.complete()).complete() ), client);
			}
			else
			{
				consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").interestAfterRefresh(true).payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray()
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.addAscii((EmaString().append("TRI.N_").append(numItems++)))
					.complete()).complete()), client);
			}


			sleep(2000);
		}

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

//END APIQA
