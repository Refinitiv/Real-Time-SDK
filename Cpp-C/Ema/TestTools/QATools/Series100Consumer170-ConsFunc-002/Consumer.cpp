///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <stdlib.h>
#include <cstring>


using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

bool updateCalled = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event ) 
{
	cout << endl << refreshMsg << endl << "event channel info (refresh)" 
	  << endl << event.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event ) 
{
	if ( !updateCalled )
	{
		updateCalled = true;
		cout << endl << updateMsg << endl << "event channel info (update)"
			<< endl << event.getChannelInformation() << endl;
		//API QA
		cout << " Test getMaxOutputBuffers() : " << event.getChannelInformation().getMaxOutputBuffers() << endl;
		cout << " Test getGuaranteedOutputBuffers() : " << event.getChannelInformation().getGuaranteedOutputBuffers() << endl;
		cout << " Test getCompressionThreshold() : " << event.getChannelInformation().getCompressionThreshold() << endl;
		//END API QA
	}
	else
		cout << "skipped printing updateMsg" << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event ) 
{
	cout << endl << statusMsg << endl << "event channel info (status)" << endl << event.getChannelInformation();
}
//API QA
void AppErrorClient::onInvalidHandle(UInt64 handle, const EmaString& text)
{
	cout << endl << "onInvalidHandle callback function" << endl;
	cout << "Invalid handle: " << handle << endl;
	cout << "Error text: " << text << endl;
}

void AppErrorClient::onInaccessibleLogFile(const EmaString& fileName, const EmaString& text)
{
	cout << endl << "onInaccessibleLogFile callback function" << endl;
	cout << "Inaccessible file name: " << fileName << endl;
	cout << "Error text: " << text << endl;
}

void AppErrorClient::onSystemError(Int64 code, void* address, const EmaString& text)
{
	cout << endl << "onSystemError callback function" << endl;
	cout << "System Error code: " << code << endl;
	cout << "System Error Address: " << address << endl;
	cout << "Error text: " << text << endl;
}

void AppErrorClient::onMemoryExhaustion(const EmaString& text)
{
	cout << endl << "onMemoryExhaustion callback function" << endl;
	cout << "Error text: " << text << endl;
}

void AppErrorClient::onInvalidUsage(const EmaString& text, Int32 errorCode)
{
	cout << "onInvalidUsage callback function" << endl;
	cout << "Error text: " << text << endl;
	cout << "Error code: " << errorCode << endl;
}
void printHelp()
{
	cout << endl << "Optional options:\n" << endl
		<< " -?\tShows this usage" << endl
		<< " -maxOutputBuffers : value of maxOutputBuffer to modify. " << endl
		<< " -guaranteedOutputBuffers : value of guaranteedOutputBuffers to modify. " << endl
		<< " -highWaterMark : value of highWaterMark to modify. " << endl
		<< " -serverNumPoolBuffers : value of serverNumPoolBuffer to modify. " << endl
		<< " -compressionThreshold : value of compressionThreshold to modify. " << endl;
}
// END API QA

//int main()
int main(int argc, char* argv[])
{
	try { 
		AppClient client;
		ChannelInformation channelInfo;
		AppErrorClient errorClient;
		Int32 maxOutputBuffers = 2000;
		Int32 guaranteedOutputBuffers = 2000;
		Int32 highWaterMark = 1000;
		Int32 serverNumPoolBuffers = 3000;
		Int32 compressionThreshold = 40;
		// END API QA
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return false;
			}
			else if (strcmp(argv[i], "-maxOutputBuffers") == 0)
			{
				maxOutputBuffers = (i < (argc - 1) ? atoi(argv[++i]) : maxOutputBuffers);
			}
			else if (strcmp(argv[i], "-guaranteedOutputBuffers") == 0)
			{
				guaranteedOutputBuffers = (i < (argc - 1) ? atoi(argv[++i]) : guaranteedOutputBuffers);
			}
			else if (strcmp(argv[i], "-highWaterMark") == 0)
			{
				highWaterMark = (i < (argc - 1) ? atoi(argv[++i]) : highWaterMark);
			}
			else if (strcmp(argv[i], "-serverNumPoolBuffers") == 0)
			{
				serverNumPoolBuffers = (i < (argc - 1) ? atoi(argv[++i]) : serverNumPoolBuffers);
			}
			else if (strcmp(argv[i], "-compressionThreshold") == 0)
			{
				compressionThreshold = (i < (argc - 1) ? atoi(argv[++i]) : compressionThreshold);
			}
		}

		OmmConsumer consumer( OmmConsumerConfig( "EmaConfig.xml" ).username( "user" ), errorClient );
		consumer.getChannelInformation( channelInfo );
		cout << endl << "channel information (consumer):" << endl << channelInfo << endl;
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );
		//API QA
		cout << "Modify maxOutputBuffers to " << maxOutputBuffers << endl;
		cout << "Modify guaranteedOutputBuffers to " << guaranteedOutputBuffers << endl;
		cout << "Modify highWaterMark to " << highWaterMark << endl;
		cout << "Modify serverNumPoolBuffers to " << serverNumPoolBuffers << endl;
		cout << "Modify compressionThreshold to " << compressionThreshold << endl;
		consumer.modifyIOCtl(1, maxOutputBuffers); //maxNumBuffer
		consumer.modifyIOCtl(2, guaranteedOutputBuffers); //guaranteedOutputBuffer
		consumer.modifyIOCtl(3, highWaterMark); //highWaterMark
		consumer.modifyIOCtl(8, serverNumPoolBuffers); //serverNumPoolBuffer
		consumer.modifyIOCtl(9, compressionThreshold); //compressionThreshold			
		// END API QA
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
	  cout << excp << endl;
	}
	return 0;
}
