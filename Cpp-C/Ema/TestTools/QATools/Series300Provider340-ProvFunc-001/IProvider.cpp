///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"
//APIQA
#include <stdlib.h>
#include <string.h>
//END APIQA
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
UInt64 loginStreamHandle = 0;

//APIQA
Int32 ackId = -1;
//END APIQA

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	ElementList encodedElementList;

	/* This provider supports posting. */
	encodedElementList.addUInt( ENAME_SUPPORT_POST, 1 );
	encodedElementList.complete();

	event.getProvider().submit( RefreshMsg().
		domainType(MMT_LOGIN).
		name(reqMsg.getName()).
		nameType(USER_NAME).
		complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ).
		attrib(encodedElementList),
		event.getHandle() );

	loginStreamHandle = event.getHandle();
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		processInvalidItemRequest( reqMsg, event );
		return;
	}

	event.getProvider().submit(RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
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

void AppClient::onPostMsg( const PostMsg& postMsg, const OmmProviderEvent& event )
{
	// if the post is on the login stream, then it's an off-stream post, else it's an on-stream post
	cout << "Received an " << (event.getHandle() == loginStreamHandle ? "off-stream" : "on-stream" ) <<
		" item post (item=" << (postMsg.hasName() ? postMsg.getName() : "n/a") << ")" << endl;
    	cout << postMsg << endl;

	if (postMsg.getSolicitAck())
	{
		AckMsg ackMsg;

		ackMsg.domainType(postMsg.getDomainType());
		//APIQA
		if(ackId != -1)
		{
		    ackMsg.ackId(ackId);
		}else
		{
		    ackMsg.ackId(postMsg.getPostId());
		}
		//END APIQA
		if (postMsg.hasSeqNum())
		{
			ackMsg.seqNum(postMsg.getSeqNum());
		}
        //APIQA
        try
        {
            event.getProvider().submit(ackMsg, event.getHandle());
        }
        catch ( const OmmException& excp )
        {
            cout << excp << endl;
        }
        //END APIQA
	}
}

//APIQA
void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< "  -ackId AckId sent in Ack messages.\n" << endl;
}
//END APIQA

int main(int argc, char* argv[])
{
	try
	{
		AppClient appClient;
		
		//APIQA
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "-?") == 0)
            {
                printHelp();
                return false;
            }
            else if (strcmp(argv[i], "-ackId") == 0)
            {
                ackId = (i < (argc - 1) ? atoi(argv[++i]) : -1);
            }
        }
        //END APIQA

		OmmProvider provider( OmmIProviderConfig().operationModel( OmmIProviderConfig::UserDispatchEnum ), appClient );

		while ( itemHandle == 0 ) provider.dispatch( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.dispatch( 10000 );

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
