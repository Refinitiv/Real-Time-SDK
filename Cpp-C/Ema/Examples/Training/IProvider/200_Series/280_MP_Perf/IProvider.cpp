///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 submittingTime = 300;
ReqInfo reqInfoList[1000];
Int32 numberOfRequestItems = 0;
Int32 updateMsgCount = 0;
RefreshMsg refreshMsg;
UpdateMsg updateMsg;
FieldList fieldList;
EmaBuffer emaBuffer;
unsigned long long startRefreshTime;
unsigned long long endRefreshTime;
unsigned long long startUpdateTime;

EmaString statusText("Refresh Completed");

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(refreshMsg.clear().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( numberOfRequestItems == 1000 )
	{
		event.getProvider().submit( StatusMsg().name(reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
			domainType( reqMsg.getDomainType() ).
			state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::TooManyItemsEnum, "Request more than 1000 items" ),
			event.getHandle() );
		return;
	}

	event.getProvider().submit( refreshMsg.clear().serviceId( reqMsg.getServiceId() ).name( reqMsg.getName() ).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, statusText ).solicited( true ).
		payload( fieldList.clear().
		    addUInt( 1, 6560 ).
		    addUInt( 2, 66 ).
		    addUInt( 3855, 52832001 ).
		    addRmtes( 296, EmaBuffer("BOS", 3) ).
		    addTime( 375, 21, 0 ).
		    addTime( 1025, 14, 40, 32 ).
		    addReal( 22, 14400, OmmReal::ExponentNeg2Enum ).
		    addReal( 25, 14700, OmmReal::ExponentNeg2Enum ).
		    addReal( 30, 9, OmmReal::Exponent0Enum ).
		    addReal( 31, 19, OmmReal::Exponent0Enum ).
		    complete() ).
		complete(), event.getHandle() );

	if ( ++numberOfRequestItems == 1 )
	{
		startRefreshTime = getCurrentTime();
	}
	else if ( numberOfRequestItems == 1000 )
	{
		endRefreshTime = getCurrentTime();

		float timeSpent = (float)(endRefreshTime - startRefreshTime) / (float)1000;

		cout << "total refresh count = " << numberOfRequestItems
			<< "\ttotal time = " << timeSpent << " sec"
			<< "\trefresh rate = " << (float)numberOfRequestItems / timeSpent << " refresh per sec" << endl;
	}

	reqInfoList[ numberOfRequestItems - 1 ].setHandle( event.getHandle() );
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

		OmmProvider provider( OmmIProviderConfig().operationModel( OmmIProviderConfig::UserDispatchEnum) , appClient );

		while ( numberOfRequestItems < 1000 ) provider.dispatch( 100 );

		bool submitting = true;
		UInt32 numberOfSeconds = 0;
		Int32 index = 0;

		startUpdateTime = getCurrentTime();

		while ( submitting )
		{		
			for ( index = 0; index < numberOfRequestItems; index++ )
			{
				provider.submit( updateMsg.clear().payload(fieldList.clear().
					addTime( 1025, 14, 40, 32 ).
					addUInt( 3855, 52832001 ).
					addReal( 22, 14400 + ( ( reqInfoList[index].getHandle() & 0x1 ) ? 1 : 10 ), OmmReal::ExponentNeg2Enum ).
					addReal( 30, 10 + ( ( reqInfoList[index].getHandle() & 0x1) ? 10 : 20 ), OmmReal::Exponent0Enum ).
					addRmtes(296, emaBuffer.setFrom("NAS", 3) ).
					complete() ), reqInfoList[index].getHandle() );

				if ( (index % 10) == 0)
					provider.dispatch(10);

				updateMsgCount++;
			}

			float timeSpent = (float)(getCurrentTime() - startUpdateTime) / 1000;
		
			if (timeSpent >= 1)
			{
				cout << "update count = " << updateMsgCount
				<< "\tupdate rate = " << (int)(updateMsgCount / timeSpent) << " update per sec" << endl;

				updateMsgCount = 0;

				startUpdateTime = getCurrentTime();

				if (++numberOfSeconds == submittingTime) submitting = false;
			}
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
