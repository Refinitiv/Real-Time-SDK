///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//END APIQA
#include "Consumer.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

bool updateCalled = false;
//API QA
bool userDispatch = false; 
bool testChannelInfoWithLoginHandle = false; 
bool testChannelInfoValue = false;
//END API QA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event ) 
{
	cout << endl << refreshMsg << endl << "event channel info (refresh)"
          << endl << event.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event) 
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

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event) 
{
	cout << endl << statusMsg << endl << "event channel info (status)" << endl << event.getChannelInformation();
}
void printUsage(bool reflect)
{
	if (!reflect)
	{
		cout << "\nOptions:\n"
			<< "  -?\tShows this usage\n\n"
			<< "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
			<< "  -dirAdminControl \tSet if user controls sending directory msg [default = false]\n"
			<< "  -testChannelInfoWithLoginHandle \tSet if test getting channel info when register login on function registerClient() default = false]\n"
			<< "\n";
		exit(-1);
	}
	else
	{
		cout << "\nOptions will be used:\n"
		     << "  -userDispatch \t" << userDispatch << endl;
		cout << "  -testChannelInfoWithLoginHandle \t" << testChannelInfoWithLoginHandle << endl;
		cout << "  -testChannelInfoValue \t" << testChannelInfoValue << "\n\n";
	}
}

void  readCommandlineArgs(int argc, char* argv[])
{
	int iargs = 1;
	while (iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			printUsage(false);
		}
		else if (strcmp("-userDispatch", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			userDispatch = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-testChannelInfoWithLoginHandle", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			testChannelInfoWithLoginHandle = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-testChannelInfoValue", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage(false);
			testChannelInfoValue = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else
		{
			cout << "Invalid argument: " << argv[iargs] << endl;
			printUsage(false);
		}
	}
	printUsage(true);
	sleep(2000);
}
int main( int argc, char* argv[] )
{
  // API QA
  OmmConsumerConfig* config = new OmmConsumerConfig();
  // END API QA
	try { 
		AppClient appClient;
		ChannelInformation channelInfo;
		//API QA
		readCommandlineArgs(argc, argv);
		if (userDispatch)
			config->operationModel(OmmConsumerConfig::UserDispatchEnum);
		
		OmmConsumer *pOmmconsumer = 0;
        if (testChannelInfoWithLoginHandle) 
            pOmmconsumer = new OmmConsumer(config->username("user"));
		else
            pOmmconsumer = new OmmConsumer(config->username("user"), appClient);
		OmmConsumer& consumer = *pOmmconsumer;

		// END API QA
		consumer.getChannelInformation(channelInfo);
		cout << endl << "channel information (consumer):" << endl << channelInfo << endl;
		
		//API QA
		if (testChannelInfoWithLoginHandle) 
            UInt64 loginHandle = consumer.registerClient( ReqMsg().domainType( MMT_LOGIN ).name( "user" ), appClient );
		// END API QA
		
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), appClient );
		
		if (userDispatch)
		{
			unsigned long long startTime = getCurrentTime();
			while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread	
		}
		else
			sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
	  cout << excp << endl;
	}
	return 0;
}
