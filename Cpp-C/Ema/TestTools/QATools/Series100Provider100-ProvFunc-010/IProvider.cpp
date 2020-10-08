///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;

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
// END API QA

int main()
{
	try
	{
		AppClient appClient;
		AppErrorClient errorClient;
		OmmProvider provider( OmmIProviderConfig().port( "14002" ), appClient, errorClient );
		UInt64 invalidHandle = 0;
		Int32 modifyGuaranteedOutputBuff = 10000;
		provider.submit(GenericMsg(), invalidHandle);
		while ( itemHandle == 0 ) sleep(1000);
		
		for ( Int32 i = 0; i < 20; i++ )
		{
			try
			{
				for (Int32 j = 0; j < 100; j++)
				{
					provider.submit(UpdateMsg().payload(FieldList().
						addReal(22, 3391 + j, OmmReal::ExponentNeg2Enum).
						addReal(30, 10 + j, OmmReal::Exponent0Enum).
						addAscii(315, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(316, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(317, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(318, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(319, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(320, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(321, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(322, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(323, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(324, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(325, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(326, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(327, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(328, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(329, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(330, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(331, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(332, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(333, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(334, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(335, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(336, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(337, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						addAscii(338, EmaString("A1234567890123456789012345678901234567890123546789012345678901234567890")).
						complete()), itemHandle);
				}
			}
			catch (const OmmException& excp)
			{
				cout << "Exception is never been caught because provider registered for Error callback : " << endl;
				cout << excp << endl;
			}			
			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
