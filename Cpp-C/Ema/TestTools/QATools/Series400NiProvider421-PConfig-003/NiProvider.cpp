///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

AppClient::AppClient() :
	_bConnectionUp( false )
{
}

AppClient::~AppClient()
{
}

bool AppClient::isConnectionUp() const
{
	return _bConnectionUp;
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << refreshMsg << endl;

	if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum )
	{
		if ( refreshMsg.getState().getDataState() == OmmState::OkEnum )
			_bConnectionUp = true;
		else
			_bConnectionUp = false;
	}
	else
		_bConnectionUp = false;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << statusMsg << endl;

	if ( statusMsg.hasState() )
	{
		if ( statusMsg.getState().getStreamState() == OmmState::OpenEnum )
		{
			if ( statusMsg.getState().getDataState() == OmmState::OkEnum )
				_bConnectionUp = true;
			else
				_bConnectionUp = false;
		}
		else
			_bConnectionUp = false;
	}
	else
		_bConnectionUp = true;
}

void createProgrammaticConfig( Map& configMap )
{
    Map innerMap;
    ElementList elementList;

    elementList.addAscii( "DefaultNiProvider", "Provider_1" );

    innerMap.addKeyAscii( "Provider_1", MapEntry::AddEnum, ElementList()
        .addAscii( "ChannelSet", "Channel_10,Channel_11" )
        .addAscii( "Directory", "Directory_1" )
        .addAscii( "Logger", "Logger_1" )
        .addUInt( "CatchUnhandledException", 0 )
        .addUInt( "ItemCountHint", 100 )
        .addUInt( "DispatchTimeoutApiThread", 1000)
        .addUInt( "MaxDispatchCountApiThread", 500 )
        .addUInt( "MaxDispatchCountUserThread", 500 )
        .addUInt( "MergeSourceDirectoryStreams", 1)
        .addAscii( "Name", "Series400NiProvider421-PConfig-001")
        .addInt( "PipePort", 9001 )
        .addInt( "ReconnectAttemptLimit", 10)
        .addInt( "ReconnectMaxDelay", 5000)
        .addInt( "ReconnectMinDelay", 1000)
        .addUInt( "RecoverUserSubmitSourceDirectory", 1)
        .addUInt( "RemoveItemsOnDisconnect", 1)
        .addInt("RequestTimeout", 15000)
        .addUInt( "ServiceCountHint", 100 )
        .addAscii("XmlTraceFileName", "NiProvXMLTrace")
        .addInt("XmlTraceMaxFileSize", 50000000)
        .addUInt("XmlTraceToFile", 1)
        .addUInt("XmlTraceToStdout", 0)
        .addUInt("XmlTraceToMultipleFiles", 1)
        .addUInt("XmlTraceWrite", 1)
        .addUInt("XmlTraceRead", 1)
        .addUInt("XmlTracePing", 1)
        .addUInt("XmlTraceHex", 1)
        .addUInt( "LoginRequestTimeOut" , 45000 )
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
        .addEnum( "CompressionType", 0 )
        .addAscii("InterfaceName", "providerInterface")
        .addUInt("CompressionThreshold", 100)
        .addUInt("HighWaterMark", 6144)
        .addUInt("NumInputBuffers", 10)
        .addUInt("SysRecvBufSize", 0)
        .addUInt("SysSendBufSize", 0)
        .addUInt( "TcpNodelay", 1 ).complete() )

        .addKeyAscii( "Channel_11", MapEntry::AddEnum,
        ElementList()
        .addEnum( "ChannelType", 5 )
        .addUInt( "GuaranteedOutputBuffers", 5000 )
        .addUInt( "ConnectionPingTimeout", 30000 )
        .addAscii( "Host", "channel11_host" )
        .addAscii( "Port", "channel11_port" )
        .addEnum( "CompressionType", 0 )
        .addAscii("InterfaceName", "providerInterface")
        .addUInt("CompressionThreshold", 100)
        .addUInt("HighWaterMark", 6144)
        .addUInt("NumInputBuffers", 10)
        .addUInt("SysRecvBufSize", 0)
        .addUInt("SysSendBufSize", 0)
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

    innerMap.addKeyAscii( "Dictionary_1", MapEntry::AddEnum,
                        ElementList()
                        .addEnum( "DictionaryType", 1 )
                        .addAscii( "RdmFieldDictionaryItemName", "RWFFld" )
                        .addAscii( "EnumTypeDefItemName", "RWFEnum" ).complete()).complete();

    elementList.addMap( "DictionaryList", innerMap );
    elementList.complete();
    innerMap.clear();

    configMap.addKeyAscii( "DictionaryGroup", MapEntry::AddEnum, elementList );
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
                OmmArray().addAscii( "MMT_DICTIONARY" )
                .addAscii( "MMT_MARKET_PRICE" )
                .addAscii( "MMT_MARKET_BY_ORDER" )
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
                    ElementList().addAscii( "Timeliness", "Timeliness::InexactDelayed" )
                    .addAscii( "Rate", "Rate::JustInTimeConflated" )
                    .complete() )
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
	elementList.clear();
}

int main( int argc, char* argv[] )
{
	try
	{
		AppClient appClient;
        Map configMap;
        createProgrammaticConfig( configMap );

		OmmProvider provider( OmmNiProviderConfig().config( configMap).operationModel( OmmNiProviderConfig::UserDispatchEnum ).username( "user" ), appClient );
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;
		bool sendRefreshMsg = false;

		provider.dispatch( 1000000 );		// calls to onRefreshMsg(), or onStatusMsg() execute on this thread

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		provider.dispatch( 1000000 );

		for ( Int32 i = 0; i < 60; )
		{
			if ( appClient.isConnectionUp() )
			{
				if (sendRefreshMsg)
				{
					provider.submit(refresh.clear().serviceName("NI_PUB").name("TRI.N")
						.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
						.payload(fieldList.clear()
							.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
							.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
							.addReal(30, 20, OmmReal::Exponent0Enum)
							.addReal(31, 40, OmmReal::Exponent0Enum)
							.complete())
						.complete(), triHandle);

					sendRefreshMsg = false;
				}
				else
				{
					provider.submit(update.clear().serviceName("NI_PUB").name("TRI.N")
						.payload(fieldList.clear()
							.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
							.addReal(30, 21 + i, OmmReal::Exponent0Enum)
							.complete()), triHandle);

					i++;
				}
			}
			else
			{
				sendRefreshMsg = true;
			}

			provider.dispatch( 1000000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
