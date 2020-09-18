///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"
#include <cstring>

using namespace rtsdk::ema::access;
using namespace std;

void createProgrammaticConfig( Map& configMap )
{
	Map innerMap;
	ElementList elementList;

	elementList.addAscii( "DefaultNiProvider", "Provider_4" );

	innerMap.addKeyAscii( "Provider_4", MapEntry::AddEnum, ElementList()
		.addAscii( "Channel", "Channel_13" )
		.addAscii( "Directory", "Directory_1" )
		.addAscii( "Logger", "Logger_1" )
		.addUInt( "XmlTraceToStdout", 1 )
		.addUInt( "RefreshFirstRequired", 1 ).complete() ).complete();

	elementList.addMap( "NiProviderList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "NiProviderGroup", MapEntry::AddEnum, elementList );

	elementList.clear();

	innerMap.addKeyAscii( "Channel_13", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", 1 )
		.addEnum( "CompressionType", 99 )
		.addUInt( "GuaranteedOutputBuffers", 5000 )
		.addUInt( "ConnectionPingTimeout", 30000 )
		.addAscii( "Host", "localhost" )
		.addAscii( "Port", "14002" )
		.addAscii( "ObjectName", "P_ObjectName")
		.addAscii( "OpenSSLCAStore", "MYCA")
		.addAscii( "ProxyHost", "proxyHostToConnectTo")
		.addAscii("ProxyPort", "proxyPortToConnectTo" )
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
						.addEnum( "LoggerSeverity", 0 ).complete()).complete();

	elementList.addMap( "LoggerList", innerMap );

	elementList.complete();
	innerMap.clear();

	configMap.addKeyAscii( "LoggerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	Map serviceMap;
	serviceMap.addKeyAscii( "TEST_NI_PUB", MapEntry::AddEnum,
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

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
		<< " -spTLSv1.2 enable use of cryptographic protocol TLSv1.2 used with linux encrypted connections \n"
		<< " -libsslName name of the libssl.so shared library used with linux encrypted connections. \n"
		<< " -libcryptoName name of the libcrypto.so shared library used with linux encrypted connections \n" << endl;
}

int main( int argc, char* argv[] )
{
	try
	{
		OmmNiProviderConfig config;
		int securityProtocol = 0;

		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return false;
			}
			else if (strcmp(argv[i], "-ph") == 0)
			{
				config.tunnelingProxyHostName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-pp") == 0)
			{
				config.tunnelingProxyPort(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-spTLSv1.2") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_2;
			}
			else if (strcmp(argv[i], "-libsslName") == 0)
			{
				config.tunnelingLibSslName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-libcryptoName") == 0)
			{
				config.tunnelingLibCryptoName(i < (argc - 1) ? argv[++i] : NULL);
			}
		}

		if (securityProtocol > 0)
			config.tunnelingSecurityProtocol(securityProtocol);
		//API QA
		Map configMap;
        createProgrammaticConfig(configMap);	
		OmmProvider provider( config.config(configMap).username( "user" ).providerName( "Provider_4" ) );
		//END API QA
		UInt64 ibmHandle = 5;
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		provider.submit( refresh.serviceName( "TEST_NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		provider.submit( refresh.clear().serviceName( "TEST_NI_PUB" ).name( "TRI.N" )
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
			provider.submit( update.clear().serviceName( "TEST_NI_PUB" ).name( "IBM.N" )
				.payload( fieldList.clear()
					.addReal( 22, 14400 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
					.complete() ), ibmHandle );
			provider.submit( update.clear().serviceName( "TEST_NI_PUB" ).name( "TRI.N" )
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
