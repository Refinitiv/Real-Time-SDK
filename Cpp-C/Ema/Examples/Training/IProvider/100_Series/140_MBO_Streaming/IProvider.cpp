/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
EmaString OrderNr = "100";

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processMarketByOrderRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		processInvalidItemRequest(reqMsg, event);
		return;
	}

	event.getProvider().submit(RefreshMsg().domainType(MMT_MARKET_BY_ORDER).name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).
		payload( Map().
			summaryData( FieldList().addEnum( 15, 840 ).addEnum( 53, 1 ).addEnum( 3423, 1 ).addEnum( 1709, 2 ).complete() ).
			addKeyAscii( OrderNr, MapEntry::AddEnum, FieldList().
				addRealFromDouble( 3427, 7.76, OmmReal::ExponentNeg2Enum ).
				addRealFromDouble( 3429, 9600 ).
				addEnum( 3428, 2 ).
				addRmtes( 212, EmaBuffer( "Market Maker", 12 ) ).
				complete() ).
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
	case MMT_MARKET_BY_ORDER:
		processMarketByOrderRequest( reqMsg, event );
		break;
	default:
		processInvalidItemRequest( reqMsg, event );
		break;
	}
}

int main()
{
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig().port( "14002" ), appClient );
		
		while ( itemHandle == 0 ) sleep(1000);
		
		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.submit( UpdateMsg().domainType( MMT_MARKET_BY_ORDER ).payload( Map().
				addKeyAscii( OrderNr, MapEntry::UpdateEnum, FieldList().
					addRealFromDouble( 3427, 7.76 + i * 0.1, OmmReal::ExponentNeg2Enum ).
					addRealFromDouble( 3429, 9600 ).
					addEnum( 3428, 2 ).
					addRmtes( 212, EmaBuffer( "Market Maker", 12 ) ).
					complete() ).
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
