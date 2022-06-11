///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

EmaString clientId;
EmaString clientSecret;
EmaString host;
EmaString port;
EmaString proxyHostName;
EmaString proxyPort;
EmaString proxyUserName;
EmaString proxyPasswd;
EmaString proxyDomain;
EmaString tokenV2URL;
bool takeExclusiveSignOnControl = true;
bool connectWebSocket = false;

OmmConsumer* pOmmConsumer = NULL;

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

void AppClient::onError( const EmaString& statusText, const ServiceEndpointDiscoveryEvent& event )
{
	cout << "Failed to query Refinitiv Data Platform service discovery. Error text: " << statusText << endl;
}

void OAuthClient::onCredentialRenewal( const OmmConsumerEvent& consumerEvent )
{
	/* In this function, an application would normally retrieve the user credentials(clientId and clientSecret) 
	   from a secure credential store.  For this example, the credentials will be stored as plain text.  This is
	   not secure, and is done for example purposes only */
	   
	OAuth2CredentialRenewal credentialRenewal;
	credentialRenewal.clientId(clientId);
	credentialRenewal.clientSecret(clientSecret);

	cout << "Renewal event called!" << endl;
	
	/* Call ommConsumer::renewOAuthCredentials to apply the credentials to the OmmConsumer object */
	pOmmConsumer->renewOAuth2Credentials(credentialRenewal);
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
		<< " -clientId client ID to perform authorization with the token service (mandatory). " << endl
		<< " -clientSecret ClientSecret to perform authorization with the token service (mandatory). " << endl
		<< " -h hostname to connect to(mandatory)." << endl
		<< " -p port to connect to (mandatory)." << endl
		<< " -itemName Request item name (optional)." << endl
		<< " -websocket Use the WebSocket transport protocol (optional)" << endl
		<< " -tokenURLV2 URL for the V2 token generator(optional)." << endl
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
		OAuthClient oAuthClient;
		Map configDb;
		OmmConsumerConfig config;

		EmaString itemName = "IBM.N";

		for ( int i = 1; i < argc; i++ )
		{
			if ( strcmp( argv[i], "-?" ) == 0 )
			{
				printHelp();
				return 0;
			}
			else if ( strcmp( argv[i], "-clientId" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) clientId.set( argv[++i] );
			}
			else if ( strcmp( argv[i] , "-clientSecret" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) clientSecret.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-itemName") == 0)
			{
				itemName.set(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-websocket") == 0)
			{
				connectWebSocket = true;
			}
			else if (strcmp(argv[i], "-tokenURLV2") == 0)
			{
				if (i < (argc - 1)) tokenV2URL.set(argv[++i]);
			}
			else if ( strcmp( argv[i], "-h" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) host.set( argv[++i] );
			}
			else if ( strcmp( argv[i], "-p" ) == 0 )
			{
				if ( i < ( argc - 1 ) ) port.set( argv[++i] );
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
		}

		if ( !clientId.length() || !clientSecret.length() )
		{
			cout << "User name, password and client Id must be specified on the command line. Exiting...";
			printHelp();
			return -1;
		}

		createProgramaticConfig( configDb );

		config.consumerName("Consumer_1")
			.clientId(clientId).clientSecret(clientSecret).config(configDb).tunnelingProxyHostName(proxyHostName).tunnelingProxyPort(proxyPort)
			.proxyUserName(proxyUserName).proxyPasswd(proxyPasswd).proxyDomain(proxyDomain);

		if (!tokenV2URL.empty())
			config.tokenServiceUrlV2(tokenV2URL);

		OmmConsumer consumer( config, oAuthClient);
			
		pOmmConsumer = &consumer;

		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( itemName ), client );
		sleep( 900000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
