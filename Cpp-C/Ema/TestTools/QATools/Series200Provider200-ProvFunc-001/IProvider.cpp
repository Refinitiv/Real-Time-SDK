///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"
#include <atomic>
#include <queue>

#include <mutex>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//APIQA
std::atomic<UInt64> itemHandle(0);
std::queue<UInt64> itemHndleQ;
std::mutex mtx;
//END APIQA

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if (itemHandle != 0)
	{
		processInvalidItemRequest( reqMsg, event );
		return;
	}

	event.getProvider().submit( RefreshMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).solicited( true ).
		payload( FieldList().
			addAscii( 3, reqMsg.getName() ).
			addEnum( 15, 840 ).
			addReal( 21, 3900, OmmReal::ExponentNeg2Enum ).
			addReal( 22, 3990, OmmReal::ExponentNeg2Enum ).
			addReal( 25, 3994, OmmReal::ExponentNeg2Enum ).
			addReal( 30, 9, OmmReal::Exponent0Enum ).
			addReal( 31, 19, OmmReal::Exponent0Enum ).
			complete() ).
		complete(), event.getHandle() );
//APIQA
	itemHandle = event.getHandle();
	mtx.lock();
	itemHndleQ.push(itemHandle);
	mtx.unlock();
	itemHandle = 0;
//END APIQA
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

		OmmProvider provider( OmmIProviderConfig().operationModel( OmmIProviderConfig::UserDispatchEnum ), appClient );

//APIQA
		for ( Int32 i = 0; i < 600; i++ )
		{
			if (!itemHndleQ.empty())
			{
				provider.submit(UpdateMsg().payload(FieldList().
					addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
					addReal(25, 3994 + i, OmmReal::ExponentNeg2Enum).
					addReal(30, 10 + i, OmmReal::Exponent0Enum).
					addReal(31, 19 + i, OmmReal::Exponent0Enum).
					complete()), itemHndleQ.front());

				mtx.lock();
				itemHndleQ.pop();
				mtx.unlock();
			}

			provider.dispatch(1000000000);
			sleep(100);
		}
//END APIQA
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
