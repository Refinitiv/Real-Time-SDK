///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace rtsdk::ema::access;
using namespace std;

void createProgrammaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;

	elementList.addAscii( "DefaultNiProvider", "Provider_1" );

	innerMap.addKeyAscii( "Provider_1", MapEntry::AddEnum, ElementList()
		.addAscii( "Channel", "Channel_10" )
		.addAscii( "Directory", "Directory_1" )
		.addAscii( "Logger", "Logger_1" )
		.addUInt( "XmlTraceToStdout", 1 )
		.addUInt( "RefreshFirstRequired", 1 ).complete() ).complete();

	elementList.addMap( "NiProviderList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "NiProviderGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii( "Channel_10", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", 0 )
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt( "ConnectionPingTimeout", 30000 )
		.addAscii( "Host", "localhost" )
		.addAscii( "Port", "14003" )
		.addUInt( "TcpNodelay", 1 ).complete() ).complete();

	elementList.addMap( "ChannelList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "ChannelGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii( "Logger_1", MapEntry::AddEnum,
						ElementList()
						.addEnum( "LoggerType", 1 )
						.addAscii( "FileName", "logFile" )
						.addEnum( "LoggerSeverity", 1 ).complete()).complete();

	elementList.addMap( "LoggerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "LoggerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	Map serviceMap;
	serviceMap.addKeyAscii( "NI_PUB", MapEntry::AddEnum,
		ElementList()
		.addElementList( "InfoFilter",
			ElementList().addUInt( "ServiceId", 0 )
			.addAscii( "Vendor", "company name" )
			.addUInt( "IsSource", 0 )
			.addUInt( "AcceptingConsumerStatus", 0 )
			.addUInt( "SupportsQoSRange", 0 )
			.addUInt( "SupportsOutOfBandSnapshots", 0 )
			.addAscii( "ItemList", "#.itemlist" )
			.addArray( "Capabilities",
				OmmArray().addAscii( "MMT_MARKET_PRICE" )
				.addAscii( "MMT_MARKET_BY_PRICE" )
				.addAscii( "200" )
				.complete() )
			.addArray( "DictionariesUsed",
				OmmArray().addAscii( "Dictionary_1" )
				.complete() )
			.addSeries( "QoS",
				Series()
				.add(
					ElementList().addAscii( "Timeliness", "Timeliness::RealTime" )
					.addAscii( "Rate", "Rate::TickByTick" )
					.complete() )
				.add(
					ElementList().addUInt("Timeliness", 100)
					.addUInt("Rate", 100)
					.complete())
				.complete() )
			.complete() )

		.addElementList( "StateFilter",
			ElementList().addUInt( "ServiceState", 1 )
			.addUInt( "AcceptingRequests", 1 )
			.complete() )
		.complete() )
	.complete();

	innerMap.addKeyAscii( "Directory_1", MapEntry::AddEnum, serviceMap ).complete();

	elementList.clear();
	elementList.addAscii( "DefaultDirectory", "Directory_1" );
	elementList.addMap( "DirectoryList", innerMap ).complete();

	configMap.addKeyAscii( "DirectoryGroup", MapEntry::AddEnum, elementList ).complete();
}

int main()
{
	try
	{
		Map configMap;
		createProgrammaticConfig( configMap );

		OmmProvider provider( OmmNiProviderConfig().config( configMap ).username( "user" ) );
		UInt64 ibmHandle = 5;
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		provider.submit( refresh.serviceName( "NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		sleep( 1000 );

		for ( Int32 i = 0; i < 60; i++ )
		{
			provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
				.payload( fieldList.clear()
					.addReal( 22, 14400 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
					.complete() ), ibmHandle );
			provider.submit( update.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
				.payload( fieldList.clear()
					.addReal( 22, 4100 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 21 + i, OmmReal::Exponent0Enum )
					.complete() ), triHandle );
			sleep( 1000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
