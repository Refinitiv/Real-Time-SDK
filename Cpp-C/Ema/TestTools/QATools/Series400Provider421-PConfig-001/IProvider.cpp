///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019,2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

UInt64 itemHandle = 0;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
		solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
		event.getHandle() );
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

void createProgrammaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;

	elementList.addAscii( "DefaultIProvider", "Provider_1" );

	innerMap.addKeyAscii( "Provider_1", MapEntry::AddEnum, ElementList()
		.addAscii( "Server", "Server_1" )
		.addAscii( "Directory", "Directory_2" )
		.addAscii( "Logger", "Logger_1" )
		.addUInt( "ItemCountHint", 10000 )
		.addUInt( "ServiceCountHint", 10000 )
		//APIQA
		.addInt( "DispatchTimeoutUserThread", 500 )
		.addUInt( "CatchUnhandledException", 0 )
		.addUInt( "MaxDispatchCountApiThread", 500 )
		.addUInt( "MaxDispatchCountUserThread", 500 )
		.addUInt("AcceptDirMessageWithoutMinFilters", 0)
		.addUInt("AcceptMessageSameKeyButDiffStream", 0)
		.addUInt("AcceptMessageThatChangesService", 0)
		.addUInt("AcceptMessageWithoutAcceptingRequests", 0)
		.addUInt("AcceptMessageWithoutBeingLogin", 0)
		.addUInt("AcceptMessageWithoutQosInRange", 0)
		.addUInt("EnumTypeFragmentSize", 128000)
		.addUInt("FieldDictionaryFragmentSize", 8192)
		.addInt("DispatchTimeoutApiThread", 0)
		.addUInt("LoginRequestTimeOut", 45000)
		.addInt("RequestTimeout", 15000)
		.addAscii("XmlTraceFileName", "IProvXMLTrace")
		.addInt("XmlTraceMaxFileSize", 50000000)
		.addUInt("XmlTraceToFile", 1)
		.addUInt("XmlTraceToStdout", 0)
		.addUInt("XmlTraceToMultipleFiles", 1)
		.addUInt("XmlTraceWrite", 1)
		.addUInt("XmlTraceRead", 1)
		.addUInt("XmlTracePing", 1)
		.addUInt("XmlTraceHex", 1)
        .addUInt( "RefreshFirstRequired", 1 ).complete() ).complete();

		//END APIQA
	elementList.addMap( "IProviderList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "IProviderGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii( "Server_1", MapEntry::AddEnum, ElementList()
		.addEnum( "ServerType", 0 )
		.addEnum( "CompressionType", 0 )
		//APIQA
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt("ConnectionMinPingTimeout", 20000)
		.addUInt( "ConnectionPingTimeout", 30000 )
		.addUInt( "TcpNodelay", 1 )
		.addUInt("CompressionThreshold", 100)
		.addUInt("HighWaterMark", 6144)
		.addAscii( "Port", "14002" )
		.addAscii("InterfaceName", "providerInterface")
		.addUInt("NumInputBuffers", 10)
		.addUInt("SysRecvBufSize", 10)
		.addUInt("SysSendBufSize", 10)
		//END APIQA
		.complete()).complete();

	elementList.addMap( "ServerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "ServerGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii( "Logger_1", MapEntry::AddEnum,
						ElementList()
						.addEnum( "LoggerType", 1 )
						.addAscii( "FileName", "logFile" )
						.addEnum( "LoggerSeverity", 0 ).complete()).complete();

	elementList.addMap( "LoggerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "LoggerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	innerMap.addKeyAscii( "Dictionary_3", MapEntry::AddEnum,
						ElementList()
						.addEnum( "DictionaryType", 0 )
						.addAscii( "RdmFieldDictionaryFileName", "./RDMFieldDictionary" )
						.addAscii( "EnumTypeDefFileName", "./enumtype.def" )
						.addAscii( "RdmFieldDictionaryItemName", "RWFFld" )
						.addAscii( "EnumTypeDefItemName", "RWFEnum" ).complete()).complete();

	elementList.addMap( "DictionaryList", innerMap );
	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "DictionaryGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	Map serviceMap;
	serviceMap.addKeyAscii( "DIRECT_FEED", MapEntry::AddEnum,
		ElementList()
		.addElementList( "InfoFilter",
			ElementList().addUInt( "ServiceId", 1 )
			.addAscii( "Vendor", "company name" )
			.addUInt( "IsSource", 0 )
			.addUInt( "AcceptingConsumerStatus", 0 )
			.addUInt( "SupportsQoSRange", 0 )
			.addUInt( "SupportsOutOfBandSnapshots", 0 )
			.addAscii( "ItemList", "#.itemlist" )
			.addArray( "Capabilities",
				OmmArray().addAscii( "MMT_MARKET_PRICE" )
				.addAscii( "MMT_MARKET_BY_PRICE" )
				.addAscii( "MMT_DICTIONARY" )
				.addAscii( "200" )
				.complete() )
			.addArray( "DictionariesProvided",
				OmmArray().addAscii( "Dictionary_3" )
				.complete() )
			.addArray( "DictionariesUsed",
				OmmArray().addAscii( "Dictionary_3" )
				.complete() )
			.addSeries( "QoS",
				Series()
				.add(
					ElementList().addAscii( "Timeliness", "Timeliness::RealTime" )
					.addAscii( "Rate", "Rate::TickByTick" )
					.complete() )
				.add(
					ElementList().addUInt( "Timeliness", 100 )
					.addUInt( "Rate", 100 )
					.complete() )
				.complete() )
			.complete() )

		.addElementList( "StateFilter",
			ElementList().addUInt( "ServiceState", 1 )
			.addUInt( "AcceptingRequests", 1 )
			.complete() )
		.complete() )
	.complete();

	innerMap.addKeyAscii( "Directory_2", MapEntry::AddEnum, serviceMap ).complete();

	elementList.clear();
	elementList.addAscii( "DefaultDirectory", "Directory_2" );
	elementList.addMap( "DirectoryList", innerMap ).complete();

	configMap.addKeyAscii( "DirectoryGroup", MapEntry::AddEnum, elementList ).complete();
}

int main( int argc, char* argv[] )
{
	try
	{
		AppClient appClient;
		Map configMap;
		createProgrammaticConfig( configMap );

		OmmProvider provider( OmmIProviderConfig().config( configMap ).operationModel( OmmIProviderConfig::UserDispatchEnum ), appClient );

		while ( itemHandle == 0 ) provider.dispatch( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.dispatch( 10000 );

			provider.submit( UpdateMsg().payload(FieldList().
				addReal( 22, 3391 + i, OmmReal::ExponentNeg2Enum ).
				addReal( 25, 3994 + i, OmmReal::ExponentNeg2Enum ).
				addReal( 30, 10 + i, OmmReal::Exponent0Enum ).
				addReal( 31, 19 + i, OmmReal::Exponent0Enum).
				complete() ), itemHandle );

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
