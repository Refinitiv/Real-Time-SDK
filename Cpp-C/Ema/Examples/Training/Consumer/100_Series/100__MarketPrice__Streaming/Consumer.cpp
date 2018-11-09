///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <stdlib.h>

using namespace thomsonreuters::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
  static int i(0);  
  cout << "updateMsg " << ++i << " for " << updateMsg.getName() << endl;
  //	cout << updateMsg << endl;		// defaults to updateMsg.toString()
  if (i == 20)
	exit(1);
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

int main()
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig("EmaConfig.xml").username( "user" ) );
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );
		UInt64 h(consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "GOOG.N" ), client ));
		sleep( 2000 );
		consumer.unregister(h);
		cout << "called unregister" << endl;
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
