/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace std;

void createProgrammaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;

	elementList.addAscii( "DefaultNiProvider", "Provider_1" );

	innerMap.addKeyAscii( "Provider_1", MapEntry::AddEnum, ElementList()
		.addAscii( "SessionChannelSet", "Connection_1, Connection_2" )
		.addAscii( "Directory", "Directory_1" )
		.addAscii( "Logger", "Logger_1" )
		.addUInt( "XmlTraceToStdout", 1 )
		.addUInt( "RefreshFirstRequired", 1 ).complete() ).complete();

	elementList.addMap( "NiProviderList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "NiProviderGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 30000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14003")
		.addUInt("TcpNodelay", 1).complete());


	innerMap.addKeyAscii("Channel_2", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 30000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14004")
		.addUInt("TcpNodelay", 1).complete());

	innerMap.addKeyAscii( "Channel_10", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", 0 )
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt( "ConnectionPingTimeout", 30000 )
		.addAscii( "Host", "localhost" )
		.addAscii( "Port", "14005" )
		.addUInt( "TcpNodelay", 1 ).complete() );


	innerMap.addKeyAscii("Channel_11", MapEntry::AddEnum,
		ElementList()
		.addEnum("ChannelType", 0)
		.addUInt("GuaranteedOutputBuffers", 5000)
		.addUInt("ConnectionPingTimeout", 30000)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14006")
		.addUInt("TcpNodelay", 1).complete()).complete();

	elementList.addMap( "ChannelList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "ChannelGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii("Connection_1", MapEntry::AddEnum,
		ElementList()
		.addAscii("ChannelSet", "Channel_1, Channel_2")
		.addInt("ReconnectAttemptLimit", 4)
		.addInt("ReconnectMinDelay", 2000)
		.addInt("ReconnectMaxDelay", 6000).complete());

	innerMap.addKeyAscii("Connection_2", MapEntry::AddEnum,
		ElementList()
		.addAscii("ChannelSet", "Channel_10, Channel_11")
		.addInt("ReconnectAttemptLimit", 4)
		.addInt("ReconnectMinDelay", 3000)
		.addInt("ReconnectMaxDelay", 4000).complete()).complete();

	elementList.addMap("SessionChannelList", innerMap);

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii("SessionChannelGroup", MapEntry::AddEnum, elementList);

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
			ElementList().addUInt( "ServiceId", 2 )
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

AppClient::AppClient() :
	_channelInfoVector()
{
}

AppClient::~AppClient()
{
	_channelInfoVector.clear();
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
	cout << refreshMsg.toString() << endl << "event session info (refresh)" << endl;
	
	printSessionInfo(ommEvent);
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmProviderEvent& ommEvent )
{
	cout << updateMsg.toString() << endl << "event session info (update)" << endl;
	
	printSessionInfo(ommEvent);
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
	cout << statusMsg.toString() << endl << "event session info (status)" << endl;
	
	printSessionInfo(ommEvent);
}

void AppClient::printSessionInfo( const OmmProviderEvent& ommEvent )
{
	ommEvent.getSessionInformation(_channelInfoVector);
	
	for(UInt32 i = 0; i < _channelInfoVector.size(); ++i)
	{
		cout << _channelInfoVector[i].toString() << endl;
	}
}

int main()
{
	try
	{
		Map configMap;
		createProgrammaticConfig( configMap );
		
		AppClient appClient;

		OmmProvider provider( OmmNiProviderConfig().config( configMap ).username( "user" ), appClient );
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

		for ( Int32 i = 0; i < 60; i++ )
		{
			sleep( 1000 );

			try
			{
				provider.submit(update.clear().serviceName("NI_PUB").name("IBM.N")
					.payload(fieldList.clear()
						.addReal(22, 14400 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 10 + i, OmmReal::Exponent0Enum)
						.complete()), ibmHandle);
				provider.submit(update.clear().serviceName("NI_PUB").name("TRI.N")
					.payload(fieldList.clear()
						.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 21 + i, OmmReal::Exponent0Enum)
						.complete()), triHandle);
			}
			catch( OmmInvalidUsageException& excp )
			{
				// There is no active channel currently associated with this session, so just continue until the channels are back up.
				if ( excp.getErrorCode() == OmmInvalidUsageException::NoActiveChannelEnum )
				{
					continue;
				}
				else
				{
					cout << excp << endl;
					break;
				}
			}
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
