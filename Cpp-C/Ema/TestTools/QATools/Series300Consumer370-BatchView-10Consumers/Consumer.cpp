///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2024 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	RefreshMsg cloneMsg(refreshMsg);
	cout << refreshMsg.toString() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	UpdateMsg cloneMsg(updateMsg);
	cout << cloneMsg.toString() << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	StatusMsg cloneMsg(statusMsg);
	cout << cloneMsg.toString() << endl;
}

int main()
{
	//APIQA bring multiple consumers up and down to trigger any memory issues
	for (int i = 0; i < 10; i++)
	{
		try {
			AppClient appClient;
			OmmConsumer consumer( OmmConsumerConfig().host("localhost:14002").username( "user" ));
			consumer.registerClient(ReqMsg().serviceName("ELEKTRON_DD").name("IBM.N"), appClient);
			
			OmmArray batchArray;
			OmmArray viewArray;
			ElementList batchView;
			
			batchArray.addAscii("TRI.N")
				.addAscii("IBM.N")
				.addAscii("MSFT.O")
				.complete();
		
			viewArray.fixedWidth( 2 )
				.addInt( 22 )
				.addInt( 25 )
				.complete();
		
			batchView
				.addArray(ENAME_BATCH_ITEM_LIST, batchArray)
				.addUInt(ENAME_VIEW_TYPE, 1)
				.addArray(ENAME_VIEW_DATA, viewArray)
				.complete();

			consumer.registerClient(ReqMsg().domainType( MMT_DIRECTORY ).serviceName("ELEKTRON_DD").payload(batchView), appClient);
			
			sleep( 3000 );			// API calls onRefreshMsg, onUpdateMsg, onStatusMsg
		} catch ( const OmmException& excp ) {
			cout << excp << endl;
		}
	}
	cout << "Waiting to exit application" << endl;
	sleep( 600000 ); // API calls onRefreshMsg, onUpdateMsg, onStatusMsg
	return 0;
}
