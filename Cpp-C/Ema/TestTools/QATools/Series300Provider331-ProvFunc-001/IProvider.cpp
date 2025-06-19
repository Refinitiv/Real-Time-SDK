/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "IProvider.h"
#include <cstring>
#include <string>


using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
//API QA
EmaString portNumber("14002");
UInt64 delayDir = 0;


void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib( ElementList().complete() ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ) ,
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

void AppClient::processDirectoryRequest(const ReqMsg&, const OmmProviderEvent& event)
{
	sleep(delayDir);
	event.getProvider().submit(RefreshMsg().domainType(MMT_DIRECTORY).filter(SERVICE_INFO_FILTER | SERVICE_STATE_FILTER).
		payload(Map().
			addKeyUInt(2, MapEntry::AddEnum, FilterList().
				add(SERVICE_INFO_ID, FilterEntry::SetEnum, ElementList().
					addAscii(ENAME_NAME, "DIRECT_FEED").
					addArray(ENAME_CAPABILITIES, OmmArray().
						addUInt(MMT_MARKET_PRICE).
						addUInt(MMT_MARKET_BY_PRICE).
						complete()).
					addArray(ENAME_DICTIONARYS_USED, OmmArray().
						addAscii("RWFFld").
						addAscii("RWFEnum").
						complete()).
					complete()).
				add(SERVICE_STATE_ID, FilterEntry::SetEnum, ElementList().
					addUInt(ENAME_SVC_STATE, SERVICE_UP).
					complete()).
				complete()).
			complete()).complete().solicited(true), event.getHandle());
}

void AppClient::processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	event.getProvider().submit(StatusMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
		domainType(reqMsg.getDomainType()).
		state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found"),
		event.getHandle());
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
	case MMT_DIRECTORY:
		processDirectoryRequest( reqMsg, event );
		break;
	default:
		processInvalidItemRequest( reqMsg, event );
		break;
	}
}

int main( int argc, char* argv[] )
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp("-p", argv[i]) == 0)
		{
			portNumber.set(argv[++i]);
		}
		if (strcmp("-delayDir", argv[i]) == 0)
		{
			delayDir = std::stoi(argv[++i]);
		}
	}

	try
	{
		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig().port(portNumber).adminControlDirectory( OmmIProviderConfig::UserControlEnum ), appClient );
		
		while ( itemHandle == 0 ) sleep(1000);

		for (Int32 i = 0; i < 60; i++)
		{
			provider.submit(UpdateMsg().payload(FieldList().
				addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
				addReal(30, 10 + i, OmmReal::Exponent0Enum).
				complete()), itemHandle);

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
