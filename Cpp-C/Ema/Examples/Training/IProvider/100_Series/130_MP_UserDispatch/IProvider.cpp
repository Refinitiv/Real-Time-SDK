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

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

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

		OmmProvider provider( OmmIProviderConfig().operationModel(OmmIProviderConfig::UserDispatchEnum).port( "14002" ), appClient );

		while ( itemHandle == 0 ) provider.dispatch( 1000 );

		int count = 0;
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
		{
			provider.dispatch( 1000000 );
			
			provider.submit( UpdateMsg().payload( FieldList().
					addReal( 22, 3391 + count, OmmReal::ExponentNeg2Enum ).
					addReal( 30, 10 + count, OmmReal::Exponent0Enum ).
					complete() ), itemHandle );
			
			count++;			
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
