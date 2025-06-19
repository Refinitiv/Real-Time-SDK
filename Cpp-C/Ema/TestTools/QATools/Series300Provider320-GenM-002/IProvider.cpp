/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

#define APP_DOMAIN 200

UInt64 itemHandle = 0;
Int64 genericCount = 1;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib( ElementList().complete() ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ) ,
		event.getHandle() );
        {
                ReqMsg cloneReqMsg(reqMsg);
                cout << "Clone ReqMsg" << endl;
                cout << cloneReqMsg << endl;
                cout << endl << "Clone ReqMsg Name: " << cloneReqMsg.getName() << endl << endl;
        }
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
//API QA
void AppClient::processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	if (itemHandle != 0)
	{
		cout << endl << "Received:    Invalid Request" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		return;
	}

	event.getProvider().submit(RefreshMsg().domainType(reqMsg.getDomainType()).name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
		state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
		payload(FieldList().
			addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete()).
		complete(), event.getHandle());

	itemHandle = event.getHandle();
}
//APIQA 
void AppClient::processLoginDomainGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	cout << endl << "Received:    GenericMsg" << endl << "Login Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
	cout << endl << genericMsg.toString() << endl;
        {
                GenericMsg cloneGenericMsg(genericMsg);
                cout << "Clone GenericMsg" << endl;
                cout << cloneGenericMsg << endl;
                cout << endl << "Clone GenericMsg Name: " << cloneGenericMsg.getName() << endl << endl;
        }

	event.getProvider().submit(GenericMsg().domainType(MMT_LOGIN).name("genericMsgInGeneral").payload(ElementList().addInt("valueFromProvider", ++genericCount).complete()), event.getHandle());
}

void AppClient::processDirectoryDomainGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	cout << endl << "Received:    GenericMsg" << endl << "Dir Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
	cout << endl << genericMsg.toString() << endl;

	event.getProvider().submit(GenericMsg().domainType(MMT_DIRECTORY).name("genericMsgInGeneral").payload(ElementList().addInt("valueFromProvider", 3).complete()), event.getHandle());
}
//END APIQA
void AppClient::processAppDomainGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
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
    //APIQA
	case MMT_MARKET_PRICE:
		processMarketPriceRequest(reqMsg, event);
		break;
    //END APIQA
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
    //APIQA
	case MMT_LOGIN:
		processLoginDomainGenericMsg(genericMsg, event);
		break;
	case MMT_DIRECTORY:
		processDirectoryDomainGenericMsg(genericMsg, event);
		break;
    //END APIQA
	case APP_DOMAIN:
		processAppDomainGenericMsg(genericMsg, event);
		break;
	default:
		cout << endl << "Received:    Invalid GenericMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
		break;
	}
}

int main( int argc, char* argv[] )
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
					addReal( 30, 10 + i++, OmmReal::Exponent0Enum ).
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
//END APIQA
