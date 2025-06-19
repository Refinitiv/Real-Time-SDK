/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "IProvider.h"
#include <stdlib.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
UInt32 fragmentationSize = 96000;
DataDictionary dataDictionary;
RefreshMsg refreshMsg;
Series series;
Int32 currentValue;
bool result;



void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit( refreshMsg.clear().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		attrib( ElementList().complete() ).solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ) ,
		event.getHandle() );
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
	{
		processInvalidItemRequest( reqMsg, event );
		return;
	}
       try{
            event.getProvider().submit( refreshMsg.clear().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
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
        catch ( const OmmException& excp )
        {
                cout << excp << endl;
        }

}

void AppClient::processDictionaryRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	result = false;
	refreshMsg.clear().clearCache( true );

	if ( reqMsg.getName() == "RWFFld" )
	{
		currentValue = dataDictionary.getMinFid();

		while ( !result )
		{
			result = dataDictionary.encodeFieldDictionary( series, currentValue, reqMsg.getFilter(), fragmentationSize );

			event.getProvider().submit( refreshMsg.name( reqMsg.getName() ).serviceName( "INVALID_SERVICE" ).domainType( MMT_DICTIONARY ).
				filter( reqMsg.getFilter() ).payload( series ).complete( result ).solicited( true ), event.getHandle() );

			refreshMsg.clear();
		}
	}
	else if ( reqMsg.getName() == "RWFEnum" )
	{
		currentValue = 0;

		while ( !result )
		{
			result = dataDictionary.encodeEnumTypeDictionary( series, currentValue, reqMsg.getFilter(), fragmentationSize );

			event.getProvider().submit( refreshMsg.name( reqMsg.getName() ).serviceName( "INVALID_SERVICE" ).domainType( MMT_DICTIONARY ).
				filter( reqMsg.getFilter() ).payload( series ).complete( result ).solicited( true ), event.getHandle() );

			refreshMsg.clear();
		}
	}
}

void AppClient::processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	event.getProvider().submit( StatusMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		domainType( reqMsg.getDomainType() ).
		state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found" ),
		event.getHandle() );
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
  try
  {
	switch (reqMsg.getDomainType())
	  {
	  case MMT_LOGIN:
		processLoginRequest(reqMsg, event);
		break;
	  case MMT_MARKET_PRICE:
		processMarketPriceRequest(reqMsg, event);
		break;
	  case MMT_DICTIONARY:
		processDictionaryRequest(reqMsg, event);
		break;
	  default:
		processInvalidItemRequest(reqMsg, event);
		break;
	  }
  }
  catch ( const OmmInvalidUsageException & e )
  {
	cout << e << endl;
  }
}

void AppClient::onReissue(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
  try
  {
	switch (reqMsg.getDomainType())
	{
	case MMT_DICTIONARY:
		processDictionaryRequest(reqMsg, event);
		break;
	default:
		processInvalidItemRequest(reqMsg, event);
		break;
	}
  }
  catch ( const OmmInvalidUsageException & e )
  {
	cout << e << endl;
  }
}



int main( int argc, char* argv[] )
{
	try
	{
		AppClient appClient;
		dataDictionary.loadFieldDictionary( "RDMFieldDictionary" );
		dataDictionary.loadEnumTypeDictionary( "enumtype.def" );

		OmmProvider provider( OmmIProviderConfig().adminControlDictionary( OmmIProviderConfig::UserControlEnum ), appClient );
		
		while ( itemHandle == 0 ) sleep( 1000 );

		for (Int32 i = 0; i < 60; i++)
		{
			provider.submit( UpdateMsg().payload( FieldList().
				addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ).
				addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
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
