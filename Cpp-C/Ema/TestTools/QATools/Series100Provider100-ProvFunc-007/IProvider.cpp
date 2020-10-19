///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"
//APIQA
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
//APIQA
unsigned int symbolListSize(100);

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib( ElementList().complete() ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ) ,
		event.getHandle() );
}

//APIQA
void AppClient::processSymbolListRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event ) {
  Map m;
  for (int i = 0; i < symbolListSize; ++i) {
	if (i % 25 == 0)
	  std::cout << std::endl << i << ' ';
	else
	  std::cout << '.';
	std::ostringstream oss;
	oss << "A" << i;
	try {
	  m.addKeyAscii( EmaString(oss.str().c_str(), oss.str().length()), MapEntry::AddEnum, FieldList().complete() );
	}
	catch (const OmmInvalidUsageException& e) {
	  std::cout << "caught OmmInvalidUsageException [" << e.getText() << ']' << std::endl;
	}
	catch (const OmmException& e) {
	  std::cout << "caught OmmException [" << e.getExceptionTypeAsString() << '|'
				<< e.getText() << ']' << std::endl;
	}
  }
  m.complete();

  event.getProvider()
	.submit( RefreshMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() )
			 .domainType( reqMsg.getDomainType() )
			 .solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).
		payload( m.complete() ).complete(), event.getHandle() );

  /*		payload( Map().
				 addKeyAscii( "A", MapEntry::AddEnum ).
				 addKeyAscii( "B", MapEntry::AddEnum ).				 
				 complete() ), event.getHandle() );
  */
  std::cout << "processSymbolListRequest" << std::endl;
}
//END APIQA
void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		processInvalidItemRequest(reqMsg, event);
		return;
	}

	event.getProvider().submit( RefreshMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).solicited( true ).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).
		payload( FieldList().
			addReal( 22, 3990, OmmReal::ExponentNeg2Enum ).
			addReal( 25, 3994, OmmReal::ExponentNeg2Enum ).
			addReal( 30, 9, OmmReal::Exponent0Enum ).
			addReal( 31, 19, OmmReal::Exponent0Enum ).
			complete() ).
		complete(), event.getHandle() );

	itemHandle = event.getHandle();
}

void AppClient::processInvalidItemRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit( StatusMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		domainType( reqMsg.getDomainType() ).
		state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found" ),
		event.getHandle() );
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	switch ( reqMsg.getDomainType() )
	{
	case MMT_LOGIN:
		processLoginRequest( reqMsg, event );
		break;
	case MMT_MARKET_PRICE:
		processMarketPriceRequest( reqMsg, event );
		break;
//APIQA
	case MMT_SYMBOL_LIST:
	  processSymbolListRequest( reqMsg, event );
	  break;
	default:
		processInvalidItemRequest( reqMsg, event );
		break;
	}
}

int main( int argc, char* argv[] )
{
//APIQA
  for (int i = 0; i < argc; ++i)
	if (strcmp(argv[i], "-c") == 0) {
	  if (argc == i + 1) {		// no more arguments
		cout << "USAGE: " << argv[0] << " -c size" << endl;
		exit(1);
	  }
	  symbolListSize = atoi(argv[i + 1]);
	  break;
	}
//END APIQA
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig().port( "14002" ), appClient );
		
		while ( itemHandle == 0 ) sleep(1000);
		
		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.submit( UpdateMsg().payload( FieldList().
					addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ).
					addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
					complete() ), itemHandle );
					
			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
