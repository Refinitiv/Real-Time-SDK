/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "IProvider.h"
#include <unordered_map>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;


unordered_map<UInt64, bool> itemHandles;
EmaVector<UInt64> removeHandles;
EmaString OrderNr("100");

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
}

void AppClient::processMarketByPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	itemHandles[event.getHandle()] = reqMsg.getPrivateStream();

	event.getProvider().submit(RefreshMsg().domainType(MMT_MARKET_BY_PRICE).name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").solicited(true).privateStream(reqMsg.getPrivateStream()).
		payload(Map().
			summaryData(FieldList().
				addRealFromDouble(22, 3990, OmmReal::ExponentNeg2Enum).
				addRealFromDouble(25, 3994, OmmReal::ExponentNeg2Enum).
				addRealFromDouble(30, 9, OmmReal::Exponent0Enum).
				addRealFromDouble(31, 19, OmmReal::Exponent0Enum).
				complete()).
			addKeyAscii(OrderNr, MapEntry::AddEnum, FieldList().
				addRealFromDouble(22, 3990, OmmReal::ExponentNeg2Enum).
				addRealFromDouble(25, 3994, OmmReal::ExponentNeg2Enum).
				addRealFromDouble(30, 9, OmmReal::Exponent0Enum).
				addRealFromDouble(31, 19, OmmReal::Exponent0Enum).
				complete()).complete()).
		complete(), event.getHandle());
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
	case MMT_MARKET_BY_PRICE:
		processMarketByPriceRequest( reqMsg, event );
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

		while ( itemHandles.size() == 0) provider.dispatch(1000);

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.dispatch( 10000 );


			for (std::unordered_map<UInt64, bool>::const_iterator it = itemHandles.begin(); it != itemHandles.end(); ++it) {
				const UInt64 handle = it->first;
				const bool value = it->second;
				try {
					provider.submit(UpdateMsg().clear().domainType(MMT_MARKET_BY_PRICE).payload(Map().
						summaryData(FieldList().
							addRealFromDouble(22, 3990 + i, OmmReal::ExponentNeg2Enum).
							addRealFromDouble(25, 3994 + i, OmmReal::ExponentNeg2Enum).
							addRealFromDouble(30, 9 + i, OmmReal::Exponent0Enum).
							addRealFromDouble(31, 19 + i, OmmReal::Exponent0Enum).
							complete()).
						addKeyAscii(OrderNr, MapEntry::AddEnum, FieldList().
							addRealFromDouble(22, 3990 + i, OmmReal::ExponentNeg2Enum).
							addRealFromDouble(25, 3994 + i, OmmReal::ExponentNeg2Enum).
							addRealFromDouble(30, 9 + i, OmmReal::Exponent0Enum).
							addRealFromDouble(31, 19 + i, OmmReal::Exponent0Enum).
							complete()).
						complete()), handle);
				}
				catch (const OmmException& excp) {
					cout << excp << endl;
					removeHandles.push_back(handle);
					continue;
				}
			}

			while (removeHandles.size() > 0)
			{
				itemHandles.erase(removeHandles[0]);
				removeHandles.removePosition(0);
			}

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
