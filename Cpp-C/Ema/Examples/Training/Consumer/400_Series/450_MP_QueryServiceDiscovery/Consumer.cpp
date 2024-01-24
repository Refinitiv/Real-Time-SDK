///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

using namespace refinitiv::ema::access;
using namespace std;

EmaString userName;
EmaString password;
EmaString clientId;
EmaString clientSecret;
EmaString clientJWK;
EmaString audience;
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
EmaString tokenUrlV1("https://api.refinitiv.com/auth/oauth2/v1/token");
EmaString tokenUrlV2("https://api.refinitiv.com/auth/oauth2/v2/token");
EmaString serviceDiscoveryUrl("https://api.refinitiv.com/streaming/pricing/v1/");
EmaString libSslName;
EmaString libCryptoName;
EmaString libCurlName;

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

		if ( locationList.size() >= 2 ) // Get an endpoint that provides auto failover for the specified location.
		{
			if ( locationList[0].find(location) != -1 )
			{
				host = serviceEndpointResp.getServiceEndpointInfoList()[index].getEndPoint();
				port = serviceEndpointResp.getServiceEndpointInfoList()[index].getPort();
				break;
			}
		}
		else if (locationList.size() > 0 && host.empty() && port.empty())
		{
			if ( locationList[0].find(location) != -1 )
			{
				host = serviceEndpointResp.getServiceEndpointInfoList()[index].getEndPoint();
				port = serviceEndpointResp.getServiceEndpointInfoList()[index].getPort();
			}
		}
		
	}
}

void AppClient::onError( const EmaString& statusText, const ServiceEndpointDiscoveryEvent& event )
{
	cout << "Failed to query Refinitiv Data Platform service discovery. Error text: " << statusText << endl;
}

void createProgramaticConfig( Map& configDb )
{
	Map elementMap;
	ElementList elementList;

	if (connectWebSocket)
	{
		// Use FileDictionary instead of ChannelDictionary as WebSocket connection has issue to download dictionary from Refinitiv Data Platform
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
		<< " -username machine ID to perform authorization with the token service (mandatory for V1 oauth password grant)." << endl
		<< " -password password to perform authorization with the token service (mandatory V1 oauth password grant)." << endl
		<< " -clientId client ID to perform authorization with the token service (mandatory for both V1 and V2 grant types). " << endl
		<< " -clientSecret client secret to perform authorization with the token service (mandatory for V2 oauth client credential grant). " << endl
		<< " -jwkFile path to the file containing the JWK encoded private key (mandatory for V2 oAuth client credentials with JWT). " << endl
		<< " -audience Audience value for JWT (optional for V2 oAuth client credentials with JWT). " << endl
		<< " -location location to get an endpoint from RDP service discovery (optional). Defaults to \"us-east-1\"" << endl
		<< " -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials (optional, only used with V1 oauth password grant)." << endl
		<< " -tokenURL URL for V1 to perform authentication to get access and refresh tokens (optional)." << endl
		<< " -tokenURLV1 URL for V1 to perform authentication to get access and refresh tokens (optional)." << endl
		<< " -tokenURLV2 URL for V2 to perform authentication to get access and refresh tokens (optional)." << endl
		<< " -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional)." << endl
		<< " -itemName Request item name (optional)." << endl
		<< " -websocket Use the WebSocket transport protocol (optional)" << endl
		<< " -libsslName specifies the name of libssl shared library (optional)." << endl
		<< " -libcryptoName specifies the name of libcrypto shared library (optional)." << endl
		<< " -libcurlName specifies the name of libcurl shared library (optional)." << endl
		<< "\nOptional parameters for establishing a connection and sending requests through a proxy server:" << endl
		<< " -ph Proxy host name (optional)." << endl
		<< " -pp Proxy port number (optional)." << endl
		<< " -plogin User name on proxy server (optional)." << endl
		<< " -ppasswd Password on proxy server (optional)." << endl
		<< " -pdomain Proxy Domain (optional)." << endl;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		Map configDb;
		OmmConsumerConfig config;

		FILE* pFile;
		int readSize;
		char clientJwkMem[2048];

		EmaString itemName = "IBM.N";

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
			else if (strcmp(argv[i], "-clientSecret") == 0)
			{
				if (i < (argc - 1)) clientSecret.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-jwkFile") == 0)
			{
				if (i < (argc - 1))
				{
					/* As this is an example program showing API, this handling of the JWK is not secure. */
					pFile = fopen(argv[++i], "rb");
					if (pFile == NULL)
					{
						printf("Cannot load jwk file.\n");
						return 0;
					}
					/* Read the JWK contents into a pre-allocated buffer*/
					readSize = (int)fread(clientJwkMem, sizeof(char), 2048, pFile);
					if (readSize == 0)
					{
						printf("Cannot load jwk file.\n");
						return 0;
					}

					clientJWK.set(clientJwkMem, readSize);
				}
			}
			else if (strcmp(argv[i], "-audience") == 0)
			{
				if (i < (argc - 1)) audience.set(argv[++i]);
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
					tokenUrlV1.set( argv[++i] );
					config.tokenServiceUrlV1( tokenUrlV1 );
				}
			}
			else if (strcmp(argv[i], "-tokenURLV1") == 0)
			{
				if (i < (argc - 1))
				{
					tokenUrlV1.set(argv[++i]);
					config.tokenServiceUrlV1(tokenUrlV1);
				}
			}
			else if (strcmp(argv[i], "-tokenURLV2") == 0)
			{
				if (i < (argc - 1))
				{
					tokenUrlV2.set(argv[++i]);
					config.tokenServiceUrlV2(tokenUrlV2);
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
			else if (strcmp(argv[i], "-libsslName") == 0)
			{
				if ( i < ( argc - 1 ) ) libSslName.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-libcryptoName") == 0)
			{
				if ( i < (argc - 1) ) libCryptoName.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-libcurlName") == 0)
			{
				if ( i < ( argc - 1 ) ) libCurlName.set( argv[++i]);
			}
		}

		if ( userName.length() == 0 || password.length() == 0 )
		{
			if (clientId.length() == 0 || (clientSecret.length() == 0 && clientJWK.length() == 0))
			{
				cout << "User name, password and client Id/client Id and client Secret must be specified on the command line. Exiting...";
				printHelp();
				return -1;
			}
		}

		// Query endpoints from RDP service discovery for the TCP or Websocket protocol
		//ServiceEndpointDiscovery serviceDiscovery(tokenUrlV1, tokenUrlV2, serviceDiscoveryUrl);
		ServiceEndpointDiscoveryConfig serviceEndpointDiscoveryConfig;
		serviceEndpointDiscoveryConfig
			.tokenServiceUrlV1(tokenUrlV1)
			.tokenServiceUrlV2(tokenUrlV2)
			.serviceDiscoveryUrl(serviceDiscoveryUrl)
			.libSslName(libSslName)
			.libCryptoName(libCryptoName)
			.libCurlName(libCurlName);
		ServiceEndpointDiscovery serviceDiscovery(serviceEndpointDiscoveryConfig);

		ServiceEndpointDiscoveryOption serviceEndpointDiscoveryOption;

		/* If this is a V1 password grant type login, set the username, password, and clientId on the OmmConsumerConfig object.  Otherwise, 
		   set the clientId and clientSecret for a V2 client credential grant type. */
		if (!password.empty())
		{
			serviceEndpointDiscoveryOption
				.username(userName)
				.password(password)
				.clientId(clientId)
				.takeExclusiveSignOnControl(takeExclusiveSignOnControl);
		}
		else
		{
			if (clientJWK.empty())
			{
				serviceEndpointDiscoveryOption
					.clientId(clientId)
					.clientSecret(clientSecret);
			}
			else
			{
				serviceEndpointDiscoveryOption
					.clientId(clientId)
					.clientJWK(clientJWK);

				if (!audience.empty())
				{
					serviceEndpointDiscoveryOption
						.audience(audience);
				}
			}
		}

		serviceEndpointDiscoveryOption
			.transport(connectWebSocket ? ServiceEndpointDiscoveryOption::WebsocketEnum : ServiceEndpointDiscoveryOption::TcpEnum)
			.proxyHostName(proxyHostName)
			.proxyPort(proxyPort)
			.proxyUserName(proxyUserName)
			.proxyPassword(proxyPasswd)
			.proxyDomain(proxyDomain);

		serviceDiscovery.registerClient(serviceEndpointDiscoveryOption, client);

		if ( !host.length() || !port.length() )
		{
			cout << "Both hostname and port are not available for establishing a connection with Refinitiv Real-Time - Optimized. Exiting..." << endl;
			return -1;
		}

		createProgramaticConfig( configDb );
		config.consumerName("Consumer_1")
			.config(configDb)
			.tunnelingProxyHostName(proxyHostName)
			.tunnelingProxyPort(proxyPort)
			.proxyUserName(proxyUserName)
			.proxyPasswd(proxyPasswd)
			.proxyDomain(proxyDomain);
		/* Set the configured credentials as above */
		if (!password.empty())
		{
			config
				.username(userName)
				.password(password)
				.clientId(clientId)
				.takeExclusiveSignOnControl(takeExclusiveSignOnControl);
		}
		else
		{
			if (clientJWK.empty())
			{
				config
					.clientId(clientId)
					.clientSecret(clientSecret);
			}
			else
			{
				config
					.clientId(clientId)
					.clientJWK(clientJWK);

				if (!audience.empty())
				{
					config.audience(audience);
				}
			}
		}

		OmmConsumer consumer(config);

		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( itemName ), client );
		sleep( 900000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
