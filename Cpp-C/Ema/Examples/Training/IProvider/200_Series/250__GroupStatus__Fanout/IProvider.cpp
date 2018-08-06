///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
unsigned short groupId = 10;
EmaBuffer groupIdBuffer;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit( RefreshMsg().domainType( MMT_LOGIN ).name( reqMsg.getName() ).nameType( USER_NAME ).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processUpdateServiceGroup( OmmProvider& provider )
{
	provider.submit( UpdateMsg().domainType( MMT_DIRECTORY ).filter( SERVICE_GROUP_FILTER ).
		payload( Map().
			addKeyUInt( 1, MapEntry::UpdateEnum, FilterList().
				add( SERVICE_GROUP_ID, FilterEntry::UpdateEnum, ElementList().
					addBuffer( ENAME_GROUP, groupIdBuffer ).
					addState( ENAME_STATUS, OmmState::ClosedRecoverEnum, OmmState::SuspectEnum, OmmState::NoneEnum, "Group Status Msg" ).
					complete() ).
				complete() ).
			complete()), 0); // use 0 item handle to fanout to all subscribers
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		processInvalidItemRequest(reqMsg, event);
		return;
	}

	groupIdBuffer.append( (char *) &groupId, 2);

	event.getProvider().submit( RefreshMsg().domainType( MMT_MARKET_PRICE ).name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).solicited( true ).itemGroup( groupIdBuffer ).
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

		OmmProvider provider( OmmIProviderConfig(), appClient );
		
		while ( itemHandle == 0 ) sleep( 1000 );
		
		for ( Int32 i = 0; i < 10; i++ )
		{
			provider.submit( UpdateMsg().payload( FieldList().
					addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ).
					addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
					complete() ), itemHandle );
					
			sleep( 1000 );
		}
		
		appClient.processUpdateServiceGroup( provider );

		sleep( 50000 );
		
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}

