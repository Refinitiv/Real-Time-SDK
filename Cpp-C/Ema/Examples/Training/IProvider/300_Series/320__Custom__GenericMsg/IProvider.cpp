///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

#define APP_DOMAIN 200

UInt64 itemHandle = 0;
Int64 genericCount = 1;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib( ElementList().complete() ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ) ,
		event.getHandle() );
}

void AppClient::processAppDomainRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		cout << endl << "Received:    Invalid Request" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		return;
	}

	event.getProvider().submit( RefreshMsg().domainType(reqMsg.getDomainType()).name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).solicited( true ).
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

void AppClient::processAppDomainGenericMsg(const GenericMsg&, const OmmProviderEvent& event)
{
	if (itemHandle == 0 || itemHandle != event.getHandle())
	{
		cout << endl << "Received:    Invalid GenericMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		return;
	}

	cout << endl << "Received:    GenericMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;

	event.getProvider().submit(GenericMsg().domainType(APP_DOMAIN).name("genericMsg").payload(ElementList().addInt("valueFromProvider", ++genericCount).complete()), event.getHandle());
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	switch ( reqMsg.getDomainType() )
	{
	case MMT_LOGIN:
		processLoginRequest( reqMsg, event );
		break;
	case APP_DOMAIN:
		processAppDomainRequest( reqMsg, event );
		break;
	default:
		cout << endl << "Received:    Invalid Request" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		break;
	}
}

void AppClient::onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	switch (genericMsg.getDomainType())
	{
	case APP_DOMAIN:
		processAppDomainGenericMsg(genericMsg, event);
		break;
	default:
		cout << endl << "Received:    Invalid GenericMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		break;
	}
}

int main()
{
	try
	{
		AppClient appClient;

		OmmProvider provider(OmmIProviderConfig().operationModel(OmmIProviderConfig::UserDispatchEnum), appClient);

		while (itemHandle == 0) provider.dispatch(1000);

		unsigned int i = 0;
		unsigned long long startTime = getCurrentTime();
		while (startTime + 60000 > getCurrentTime())
		{
			provider.dispatch(1000);

			provider.submit( UpdateMsg().domainType(APP_DOMAIN).payload( FieldList().
					addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ).
					addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
					complete() ), itemHandle );
			++i;
					
			sleep( 1000 );
		} 
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
