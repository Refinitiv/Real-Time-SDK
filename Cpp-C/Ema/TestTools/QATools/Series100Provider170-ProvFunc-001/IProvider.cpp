///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include <map>

#include "IProvider.h"
#include <stdlib.h>
#include <cstring>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

// Key: client handle; value: item handles associated with client
typedef map<UInt64, EmaVector<UInt64> >ItemHandles;
ItemHandles itemHandles;

// the client itself has a event handle which will is used when the client disconnects
EmaVector<UInt64>clientHandles;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	ommEvent.getProvider().submit( RefreshMsg().domainType( MMT_LOGIN ).name( reqMsg.getName() ).
							 nameType( USER_NAME ).complete().
							 attrib( ElementList().complete() ).solicited( true ).
							 state( OmmState::OpenEnum, OmmState::OkEnum,
									OmmState::NoneEnum, "Login accepted" ) ,
							 ommEvent.getHandle() );

	cout << endl << "event channel info (processLoginRequest)" << endl 
		<< ommEvent.getChannelInformation();

	cout << endl << "login for event handle " << ommEvent.getHandle() << ", client handle "
		<< ommEvent.getClientHandle() << endl;

	pair<ItemHandles::iterator, bool> retVal =
		itemHandles.insert(pair<UInt64, EmaVector<UInt64> >( ommEvent.getClientHandle(),
														EmaVector<UInt64>()) );
	if ( !retVal.second ) {
		cerr << "login request received for item already in itemHandles map" << endl;
		return;
	}
	clientHandles.push_back( ommEvent.getHandle() );
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	ommEvent.getProvider().submit( RefreshMsg().name( reqMsg.getName() )
							  .serviceName( reqMsg.getServiceName() ).solicited( true ).
							  state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).
							  payload( FieldList().
									   addReal( 22, 3990, OmmReal::ExponentNeg2Enum ).
									   addReal( 25, 3994, OmmReal::ExponentNeg2Enum ).
									   addReal( 30, 9, OmmReal::Exponent0Enum ).
									   addReal( 31, 19, OmmReal::Exponent0Enum ).
									   complete() ).complete(), ommEvent.getHandle() );
	ItemHandles::iterator I = itemHandles.find( ommEvent.getClientHandle() );
	if ( I == itemHandles.end() ) {
		cerr << "did not find client in itemHandles for processMarketPriceRequest" << endl;
		return;
	}
	I->second.push_back( ommEvent.getHandle() );

	cout << endl << "added itemHandle " << ommEvent.getHandle() << " for client handle " <<
	ommEvent.getClientHandle() << " for symbol " << reqMsg.getName();

	cout << endl << "event channel info (processMarketPriceRequest)" << endl 
		<< ommEvent.getChannelInformation() << endl;
}

void AppClient::processInvalidItemRequest( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	ommEvent.getProvider().submit( StatusMsg().name( reqMsg.getName() ).
							  serviceName( reqMsg.getServiceName() ).
							  domainType( reqMsg.getDomainType() ).
							  state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found" ),
							  ommEvent.getHandle() );

	cout << endl << "event channel info (processInvalidItemRequest)" << endl 
		<< ommEvent.getChannelInformation() << endl;
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "event channel info (onReqMsg)" << endl << ommEvent.getChannelInformation();

	switch ( reqMsg.getDomainType() ) {
	case MMT_LOGIN:
		processLoginRequest( reqMsg, ommEvent );
		break;
	case MMT_MARKET_PRICE:
		processMarketPriceRequest( reqMsg, ommEvent );
		break;
	default:
		processInvalidItemRequest( reqMsg, ommEvent );
	break;
	}
}

// called when a client disconnects or when an item is unregistered.
void AppClient::onClose( const ReqMsg& , const OmmProviderEvent& ommEvent ) 
{
	cout << endl << "close for event handle " << ommEvent.getHandle() << ", client handle "
	   << ommEvent.getClientHandle() << endl << "event channel info (onClose)" << endl
	   << ommEvent.getChannelInformation() << endl;

	// are we removing a client? If we find event.getHandle() in clientHandles, then the
	// client is gone; otherwise we are just removing an item
	bool removingClient( false );
	UInt32 pos( ( UInt32 )clientHandles.getPositionOf( ommEvent.getHandle() ) );
	if ( pos != -1 ) {
		clientHandles.removePosition( pos );
		removingClient = true;
	}
	
	ItemHandles::iterator I = itemHandles.find( ommEvent.getClientHandle() );
	if ( I == itemHandles.end() ) {
		cerr << "did not find client in itemHandles for onClose" << endl;
		return;
	}
	if ( removingClient ) {
		itemHandles.erase( I );
		return;
	}
	
	pos = ( UInt32 )I->second.getPositionOf( ommEvent.getHandle() );
	if ( pos == -1 ) {
		cerr << "did not find item handle " << ommEvent.getHandle() << " in data for client "
		 << ommEvent.getClientHandle() << endl;
		return;
	}
	I->second.removePosition( pos );
	if ( I->second.empty() )
		cout << "client " << I->first << " has nothing registered " << endl;
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

int main(int argc, char* argv[])
{
	try
	{
		AppClient appClient;
		// API QA
		AppErrorClient errorClient;
		OmmIProviderConfig config;
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

		OmmProvider provider(config.port("14002"), appClient, errorClient);

		while ( itemHandles.empty() ) sleep( 1000 );

		static size_t itemHandlesCount(0);
		EmaVector<ChannelInformation> ci;

		for ( Int32 i = 0; i < 60; i++ ) {
		if (itemHandles.size() != 0) 
		  for ( ItemHandles::iterator I = itemHandles.begin(); I != itemHandles.end(); ++I)
			  for (UInt32 index = 0; index < I->second.size(); ++index)
			  {
				  //API QA
				  if (i == 0)
				  {
					  cout << "Modify maxOutputBuffers to " << maxOutputBuffers << endl;
					  cout << "Modify guaranteedOutputBuffers to " << guaranteedOutputBuffers << endl;
					  cout << "Modify highWaterMark to " << highWaterMark << endl;
					  cout << "Modify serverNumPoolBuffers to " << serverNumPoolBuffers << endl;
					  cout << "Modify compressionThreshold to " << compressionThreshold << endl;
					  provider.modifyIOCtl(1, maxOutputBuffers, I->second[index]); //maxNumBuffer
					  provider.modifyIOCtl(2, guaranteedOutputBuffers, I->second[index]); //guaranteedOutputBuffer
					  provider.modifyIOCtl(3, highWaterMark, I->second[index]); //highWaterMark
					  provider.modifyIOCtl(8, serverNumPoolBuffers, I->second[index]); //serverNumPoolBuffer
					  provider.modifyIOCtl(9, compressionThreshold, I->second[index]); //compressionThreshold				
				  }
				  //END API QA
				  provider.submit(UpdateMsg()
					  .payload(FieldList().
						  addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
						  addReal(30, 10 + i, OmmReal::Exponent0Enum).
						  complete()), I->second[index]);
				
				  cout << "----Sent Update message index : " << i << endl;			  		
			  }
			  sleep(1000);
		  // print connected client info each time the number of connected clients changes
		  if ( itemHandles.size() != itemHandlesCount ) {
			itemHandlesCount = itemHandles.size();
			provider.getConnectedClientChannelInfo( ci );
			cout << "number of clients: " << ci.size() << endl;
			for (UInt32 index = 0; index < ci.size(); ++index)
			{
				cout << ci[index] << endl;
				//API QA
				cout << " Test getMaxOutputBuffers() : " << ci[index].getMaxOutputBuffers() << endl;
				cout << " Test getGuaranteedOutputBuffers() : " << ci[index].getGuaranteedOutputBuffers() << endl;
				cout << " Test getCompressionThreshold() : " << ci[index].getCompressionThreshold() << endl;
				// END API QA
			}
			cout << endl;
		  }
		}
		provider.getConnectedClientChannelInfo( ci );
		cout << "number of clients (end of processing): " << ci.size() << endl;
		for ( UInt32 index = 0; index < ci.size(); ++index )
		  cout << ci[index] << endl;
		cout << endl;
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}

	return 0;
}
