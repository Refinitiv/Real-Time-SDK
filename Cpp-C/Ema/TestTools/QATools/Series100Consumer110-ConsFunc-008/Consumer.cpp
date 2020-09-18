///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace rtsdk::ema::access;
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
		OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_2" ) );		// uses configuration from EmaConfig.xml
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ).qos(ReqMsg::BestTimelinessEnum, ReqMsg::BestRateEnum) , client );
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
