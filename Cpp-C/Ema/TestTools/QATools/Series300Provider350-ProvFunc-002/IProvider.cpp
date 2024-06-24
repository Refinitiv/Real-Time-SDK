///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "IProvider.h"
#include <string.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;
UInt64 loginHandle = 0;
DataDictionary dataDictionary;
bool fldDictComplete = false;
bool enumTypeComplete = false;
bool dumpDictionary = false;
//APIQA
UInt32 filter = DICTIONARY_NORMAL;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& ommEvent )
{
	ommEvent.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		ommEvent.getHandle());

	loginHandle = ommEvent.getHandle();

	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << (reqMsg.hasName() ? reqMsg.getName() : EmaString("<not set>")) << endl
		<< "Service Name: " << (reqMsg.hasServiceName() ? reqMsg.getServiceName() : EmaString("<not set>"));
}

void AppClient::processMarketPriceRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( itemHandle != 0 )
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

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent)
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << (refreshMsg.hasName() ? refreshMsg.getName() : EmaString("<not set>")) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString("<not set>"));

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;
	//APIQA
	decode(refreshMsg, ((DataDictionaryCache*)ommEvent.getClosure()),refreshMsg.getComplete());
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent)
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << (statusMsg.hasName() ? statusMsg.getName() : EmaString("<not set>")) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString("<not set>"));

	if (statusMsg.hasState())
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}
//APIQA
void AppClient::decode(const Msg& msg, DataDictionaryCache* pDataDictionaryCache, bool complete)
{
	switch (msg.getPayload().getDataType())
	{
		case DataType::SeriesEnum:
		{
			if (msg.getName() == "RWFFld")
			{
				pDataDictionaryCache->_dataDictionary.decodeFieldDictionary(msg.getPayload().getSeries(), filter);

				if (complete)
				{
					pDataDictionaryCache->_fldComplete = true;
				}
			}

			else if (msg.getName() == "RWFEnum")
			{
				pDataDictionaryCache->_dataDictionary.decodeEnumTypeDictionary(msg.getPayload().getSeries(), filter);

				if (complete)
				{
					pDataDictionaryCache->_enumComplete = true;
				}
			}

			if (pDataDictionaryCache->_fldComplete && pDataDictionaryCache->_enumComplete)
			{
				cout << endl << "Dictionary download complete" << endl;
				cout << "Dictionary Id : " << pDataDictionaryCache->_dataDictionary.getDictionaryId() << endl;
				cout << "Dictionary field version : " << pDataDictionaryCache->_dataDictionary.getFieldVersion() << endl;
				cout << "Number of dictionary entries : " << pDataDictionaryCache->_dataDictionary.getEntries().size() << endl;

				if ( dumpDictionary )
					cout << pDataDictionaryCache->_dataDictionary << endl;
		}
	}
}
}
//END APIQA
int main( int argc, char* argv[] )
{
	try
	{
		int iargs = 1;

		while ( iargs < argc )
		{
			if ( ( strcmp("-dumpDictionary", argv[iargs] ) == 0 ) )
			{
				dumpDictionary = true;
			}
			//APIQA
			else if ((strcmp("-filter", argv[iargs]) == 0))
			{
				if (++iargs == argc)
					break;
				EmaString value(argv[iargs]);
				if (value.caseInsensitiveCompare("INFO"))
				{
					filter = DICTIONARY_INFO;
				}
				else if (value.caseInsensitiveCompare("MINIMAL"))
				{
					filter = DICTIONARY_MINIMAL;
				}
				else if (value.caseInsensitiveCompare("NORMAL"))
				{
					filter = DICTIONARY_NORMAL;
				}
				else if (value.caseInsensitiveCompare("VERBOSE"))
				{
					filter = DICTIONARY_VERBOSE;
				}
			}
			//END APIQA

			++iargs;
		}

		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig(), appClient );

		// Waiting for a consumer to login
		while (loginHandle == 0) sleep(1000);
		//APIQA
		DataDictionaryCache dict1, dict2, dict3;
		// Open Dictionary streams
		UInt64 fldHandle = provider.registerClient(ReqMsg().name("RWFFld").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict1);

		UInt64 enumHandle = provider.registerClient(ReqMsg().name("RWFEnum").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict1);
		UInt64 fldHandle1 = provider.registerClient(ReqMsg().name("RWFFld").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict2);

		UInt64 enumHandle1 = provider.registerClient(ReqMsg().name("RWFEnum").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict2);
		UInt64 fldHandle2 = provider.registerClient(ReqMsg().name("RWFFld").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict3);
		UInt64 enumHandle2 = provider.registerClient(ReqMsg().name("RWFEnum").filter(filter).serviceName("DIRECT_FEED").domainType(MMT_DICTIONARY).interestAfterRefresh(false), appClient, &dict3);
		//END APIQA
		while (itemHandle == 0) sleep(1000);

		for (Int32 i = 0; i < 60; i++)
		{
			provider.submit(UpdateMsg().payload(FieldList().
				addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum).
				addReal(25, 3994 + i, OmmReal::ExponentNeg2Enum).
				addReal(30, 10 + i, OmmReal::Exponent0Enum).
				addReal(31, 19 + i, OmmReal::Exponent0Enum).
				complete()), itemHandle);

			sleep(1000);
			//APIQA
			if ( i == 6 )
			{
				cout <<"Unregister Dictionary handles" << endl;
				provider.unregister(fldHandle);
				provider.unregister(enumHandle);
			}
			//END APIQA
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
