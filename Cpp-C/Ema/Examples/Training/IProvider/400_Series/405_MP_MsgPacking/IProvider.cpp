///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2023 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
UInt64 clientHandle = 0;
OmmProvider* pProvider = NULL;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	clientHandle = event.getClientHandle();

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

	PackedMsg packedMsg(*pProvider);
	FieldList fieldList;
	UpdateMsg updMsg;

	packedMsg.initBuffer(clientHandle);

	fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum);
	fieldList.addReal(25, 3994, OmmReal::ExponentNeg2Enum);
	fieldList.addReal(30, 9, OmmReal::Exponent0Enum);
	fieldList.addReal(31, 19, OmmReal::Exponent0Enum);
	fieldList.complete();

	packedMsg.addMsg( RefreshMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).solicited( true ).
		state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed" ).
		payload(fieldList).
		complete(), event.getHandle() );

	for (Int32 i = 0; i < 10; i++)
	{
		updMsg.clear();
		fieldList.clear();
		fieldList.addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum);
		fieldList.addReal(30, 10 + i, OmmReal::Exponent0Enum);
		fieldList.complete();

		updMsg.payload(fieldList);

		packedMsg.addMsg(updMsg, event.getHandle());
	}

	event.getProvider().submit(packedMsg);

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
		FieldList flist;
		EmaVector<ChannelInformation> channleInfoList;
		pProvider = new OmmProvider( OmmIProviderConfig().port( "14002" ), appClient );

		while ( itemHandle == 0 ) 
            sleep(1000);
		
		UpdateMsg msg;
		PackedMsg packedMsg(*pProvider);

		unsigned long long startTime = getCurrentTime();
		while (startTime + 60000 > getCurrentTime())
		{
			pProvider->getConnectedClientChannelInfo(channleInfoList);

			if (channleInfoList.empty()) 
			{
				cerr << "Our ongoing connection has been closed, ending application" << endl;
				break;
			}

			packedMsg.initBuffer(clientHandle);

			for (Int32 i = 0; i < 10; i++)
			{
				msg.clear();
				flist.clear();
				flist.addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum);
				flist.addReal(30, 10 + i, OmmReal::Exponent0Enum);
				flist.complete();

				msg.payload(flist);

				packedMsg.addMsg(msg, itemHandle);
			}

			if (packedMsg.packedMsgCount() > 0) 
			{
				pProvider->submit(packedMsg);
				packedMsg.clear();
			}
			else
			{
				cerr << "No one message was added to the packed buffer" << endl;
				return -1;
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
