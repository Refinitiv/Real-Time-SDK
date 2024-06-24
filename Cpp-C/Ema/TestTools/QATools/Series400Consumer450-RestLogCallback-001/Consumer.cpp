///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

EmaString userName;
EmaString password;
EmaString clientId;
EmaString host;
EmaString port;
EmaString location("us-east-1");
EmaString proxyHostName;
EmaString proxyPort;
EmaString proxyUserName;
EmaString proxyPasswd;
EmaString proxyDomain;
bool takeExclusiveSignOnControl = true;
bool connectWebSocket = false;
EmaString tokenUrl("https://api.refinitiv.com/auth/oauth2/v1/token");
EmaString serviceDiscoveryUrl("https://api.refinitiv.com/streaming/pricing/v1/");
//API QA
EmaString fileNameRestLog("emaRestCallback_450.log");
//END API QA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;
}

void AppClient::onSuccess( const ServiceEndpointDiscoveryResp& serviceEndpointResp, const ServiceEndpointDiscoveryEvent& event )
{
	cout << serviceEndpointResp << endl; // dump service discovery endpoints 
	
	for ( UInt32 index = 0; index < serviceEndpointResp.getServiceEndpointInfoList().size(); index++ )
	{
		const EmaVector<EmaString>& locationList = serviceEndpointResp.getServiceEndpointInfoList()[index].getLocationList();

		if ( locationList.size() == 2 ) // Get an endpoint that provides auto failover for the specified location.
		{
			if ( locationList[0].find(location) != -1 )
			{
				host = serviceEndpointResp.getServiceEndpointInfoList()[index].getEndPoint();
				port = serviceEndpointResp.getServiceEndpointInfoList()[index].getPort();
				break;
			}
		}
	}
}

void AppClient::onError( const EmaString& statusText, const ServiceEndpointDiscoveryEvent& event )
{
	cout << "Failed to query Delivery Platform service discovery. Error text: " << statusText << endl;
}

//API QA
AppRestClient::AppRestClient(const EmaString& fileName)
{
	fsOutput.open(fileName.c_str());
}

AppRestClient::~AppRestClient()
{
	if (fsOutput.is_open())
		fsOutput.close();
}

void AppRestClient::onRestLoggingEvent(const OmmConsumerRestLoggingEvent& ommLogRestEvent)
{
	fsOutput << "{AppRestClient}" << ommLogRestEvent.getRestLoggingMessage() << endl;
}
//END API QA

void createProgramaticConfig( Map& configDb )
{
	Map elementMap;
	ElementList elementList;

	if (connectWebSocket)
	{
		// Use FileDictionary instead of ChannelDictionary as WebSocket connection has issue to download dictionary from Delivery Platform
		elementMap.addKeyAscii( "Consumer_1", MapEntry::AddEnum,
			ElementList().addAscii( "Channel", "Channel_1" ).addAscii( "Dictionary", "Dictionary_1" ).complete() ).complete();
	}
	else
	{
		elementMap.addKeyAscii( "Consumer_1", MapEntry::AddEnum,
			ElementList().addAscii( "Channel", "Channel_1" ).complete() ).complete();
	}

	elementList.addMap( "ConsumerList", elementMap );

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii( "ConsumerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	ElementList channelElementList;

	channelElementList
		.addEnum("ChannelType", 1) // Use the RSSL_CONN_TYPE_ENCRYPTED connection
		.addAscii("Host", host)
		.addAscii("Port", port)
		.addUInt("EnableSessionManagement", 1);

	if (connectWebSocket)
	{
		channelElementList.addEnum("EncryptedProtocolType", 7); // Use the WebSocket transport protocol and OpenSSL for encryption on Windows
		channelElementList.addAscii("WsProtocols", "tr_json2");
	}
	else
	{
		channelElementList.addEnum("EncryptedProtocolType", 0); // Use the standard TCP transport protocol and OpenSSL for encryption on Windows
	}

	channelElementList.complete();

	elementMap.addKeyAscii("Channel_1", MapEntry::AddEnum, channelElementList);
	elementMap.complete();

	elementList.addMap( "ChannelList", elementMap );

	elementList.complete();

	configDb.addKeyAscii( "ChannelGroup", MapEntry::AddEnum, elementList );

	if (connectWebSocket)
	{
		// Use FileDictionary instead of ChannelDictionary as WebSocket connection has issue to download dictionary from RDP
		configDb.addKeyAscii("DictionaryGroup", MapEntry::AddEnum,
			ElementList().addMap("DictionaryList",
				Map().addKeyAscii("Dictionary_1", MapEntry::AddEnum,
					ElementList().
					addEnum("DictionaryType", 0). // Use FileDictionaryEnum
					addAscii("RdmFieldDictionaryFileName", "RDMFieldDictionary").
					addAscii("EnumTypeDefFileName", "enumtype.def").
					complete()).
				complete()).
			complete());
	}

	configDb.complete();
}

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< " -username machine ID to perform authorization with the token service (mandatory)." << endl
		<< " -password password to perform authorization with the token service (mandatory)." << endl
		<< " -clientId client ID to perform authorization with the token service (mandatory). " << endl
		<< " -location location to get an endpoint from RDP service discovery (optional). Defaults to \"us-east-1\"" << endl
		<< " -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials (optional)." << endl
		<< " -tokenURL URL to perform authentication to get access and refresh tokens (optional)." << endl
		<< " -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional)." << endl
		<< " -itemName Request item name (optional)." << endl
		<< " -websocket Use the WebSocket transport protocol (optional)" << endl
		<< "\nOptional parameters for establishing a connection and sending requests through a proxy server:" << endl
		<< " -ph Proxy host name (optional)." << endl
		<< " -pp Proxy port number (optional)." << endl
		<< " -plogin User name on proxy server (optional)." << endl
		<< " -ppasswd Password on proxy server (optional)." << endl
		<< " -pdomain Proxy Domain (optional)." << endl
//API QA
		<< "\nOptional parameters to enable REST logging callback:" << endl
		<< " -restLogCallback Enable REST logging callback (optional)." << endl
		<< " -restLogFilename File name for printing the REST logging messages (optional)." << endl;
//END API QA
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		Map configDb;
		OmmConsumerConfig config;

		EmaString itemName = "IBM.N";

		//API QA
		bool restLogCallback = false;
		//END API QA

		for ( int i = 1; i < argc; i++ )
		{
			if ( strcmp( argv[i], "-?" ) == 0 )
			{
				printHelp();
				return 0;
			}
			else if ( strcmp( argv[i], "-username" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) userName.set( argv[++i] );
			}
			else if ( strcmp( argv[i] , "-password" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) password.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-location" ) == 0 )
			{
				if ( i < ( argc - 1) ) location.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-clientId" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) clientId.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-takeExclusiveSignOnControl") == 0)
			{
				if (i < (argc - 1))
				{
					EmaString takeExclusiveSignOnControlStr = argv[++i];

					if (takeExclusiveSignOnControlStr.caseInsensitiveCompare("true"))
					{
						takeExclusiveSignOnControl = true;
					}
					else if (takeExclusiveSignOnControlStr.caseInsensitiveCompare("false"))
					{
						takeExclusiveSignOnControl = false;
					}
				}
			}
			else if (strcmp(argv[i], "-tokenURL") == 0)
			{
				if ( i < (argc - 1) )
				{
					tokenUrl.set( argv[++i] );
					config.tokenServiceUrl( tokenUrl );
				}
			}
			else if (strcmp(argv[i], "-serviceDiscoveryURL") == 0)
			{
				if ( i < (argc - 1) )
				{
					serviceDiscoveryUrl.set( argv[++i] );
					config.serviceDiscoveryUrl( serviceDiscoveryUrl );
				}
			}
			else if (strcmp(argv[i], "-itemName") == 0)
			{
				itemName.set(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-websocket") == 0)
			{
				connectWebSocket = true;
			}
			else if ( strcmp( argv[i], "-ph" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) proxyHostName.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-pp") == 0 )
			{
				if ( i < ( argc - 1 ) ) proxyPort.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-plogin" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) proxyUserName.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-ppasswd" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) proxyPasswd.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-pdomain" ) == 0)
			{
				if ( i < ( argc - 1 ) ) proxyDomain.set( argv[++i] );
			}
			//API QA
			else if (strcmp(argv[i], "-restLogCallback") == 0)
			{
				restLogCallback = true;
			}
			else if (strcmp(argv[i], "-restLogFilename") == 0)
			{
				if (i < (argc - 1))
				{
					fileNameRestLog.set(argv[++i]);
				}
			}
			//END API QA
		}

		if ( !userName.length() || !password.length() || !clientId.length() )
		{
			cout << "User name, password and client Id must be specified on the command line. Exiting...";
			printHelp();
			return -1;
		}

		ServiceEndpointDiscoveryOption::TransportProtocol transportProtocol = ServiceEndpointDiscoveryOption::TcpEnum;
		if (connectWebSocket)
		{
			transportProtocol = ServiceEndpointDiscoveryOption::WebsocketEnum;
		}

		// Query endpoints from RDP service discovery for the TCP protocol
		ServiceEndpointDiscovery serviceDiscovery(tokenUrl, serviceDiscoveryUrl);
		serviceDiscovery.registerClient( ServiceEndpointDiscoveryOption().username( userName ).password( password )
			.clientId( clientId ).transport( transportProtocol ).takeExclusiveSignOnControl( takeExclusiveSignOnControl )
			.proxyHostName( proxyHostName ).proxyPort( proxyPort ).proxyUserName( proxyUserName ).proxyPassword( proxyPasswd )
			.proxyDomain( proxyDomain ), client );

		if ( !host.length() || !port.length() )
		{
			cout << "Both hostname and port are not available for establishing a connection with Real-Time - Optimized. Exiting..." << endl;
			return -1;
		}

		createProgramaticConfig( configDb );

		config.consumerName( "Consumer_1" ).username( userName ).password( password )
			.clientId( clientId ).config( configDb ).takeExclusiveSignOnControl( takeExclusiveSignOnControl )
			.tunnelingProxyHostName( proxyHostName ).tunnelingProxyPort( proxyPort )
			.proxyUserName( proxyUserName ).proxyPasswd( proxyPasswd ).proxyDomain( proxyDomain );

		//API QA
		// RestClient to receive REST logging messages via callback-method onRestLoggingMessage.
		AppRestClient restClient(fileNameRestLog);

		// Specifies the user callback client to receive REST logging messages.
		if (restLogCallback)
		{
			config.restLoggingCallback( restClient );
		}
		//END API QA

		OmmConsumer consumer( config );

		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( itemName ), client );
		sleep( 900000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
