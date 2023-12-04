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

bool connectWebSocket = false;
// API QA
EmaString restProxyHostName_pc;
EmaString restProxyPort_pc;
// END API QA

// API QA
void createProgrammaticConfig(Map& configDb, EmaString const& consumerName)
{
	Map elementMap;
	ElementList elementList;

	ElementList consumerElementList;

	if ( !restProxyHostName_pc.empty() )
		consumerElementList.addAscii("RestProxyHostName", restProxyHostName_pc);
	if ( !restProxyPort_pc.empty() )
		consumerElementList.addAscii("RestProxyPort", restProxyPort_pc);

	consumerElementList.complete();

	elementMap.addKeyAscii(consumerName, MapEntry::AddEnum,
		consumerElementList).complete();

	elementList.addMap("ConsumerList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	configDb.complete();
}
// END API QA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< " -username machine ID to perform authorization with the token service (mandatory for V1 oAuth password credentials)." << endl
		<< " -password password to perform authorization with the token service (mandatory for V1 oAuth password credentials)." << endl
		<< " -clientId client ID to perform authorization with the token service (mandatory)." << endl
		<< " -clientSecret client secret to perform authorization with the token service (mandatory for V2 oAuth client credentials with client secret)." << endl
		<< " -jwkFile path to the file containing the JWK encoded private key (mandatory for V2 oAuth client credentials with JWT). " << endl
		<< " -audience Audience value for JWT (optional for V2 oAuth client credentials with JWT). " << endl
		<< " -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials (optional, only for V1 oAuth password credential)." << endl
		<< " -tokenURL URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional)." << endl
		<< " -tokenURLV1 URL to perform authentication to get access and refresh tokens for V1 oAuth password credentials (optional)." << endl
		<< " -tokenURLV2 URL to perform authentication to get access and refresh tokens for V2 oAuth client credential (optional)." << endl
		<< " -serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional)." << endl
		<< " -itemName Request item name (optional)." << endl
		<< " -websocket Use the WebSocket transport protocol (optional)" << endl
		<< "\nOptional parameters for establishing a connection and sending requests through a proxy server:" << endl
		<< " -ph Proxy host name (optional)." << endl
		<< " -pp Proxy port number (optional)." << endl
		<< " -plogin User name on proxy server (optional)." << endl
		<< " -ppasswd Password on proxy server (optional)." << endl
		<< " -pdomain Proxy Domain (optional)." << endl
// API QA
		<< endl
		<< "Optional parameters of proxy server for Rest requests. Functional way." << endl
		<< " -restProxyHost Proxy host name (optional). Used for Rest requests only: service discovery, auth." << endl
		<< " -restProxyPort Proxy port number (optional). Used for Rest requests only: service discovery, auth." << endl
		<< " -restProxyUserName User name on proxy server (optional). Used for Rest requests only: service discovery, auth." << endl
		<< " -restProxyPasswd Password on proxy server (optional). Used for Rest requests only: service discovery, auth." << endl
		<< " -restProxyDomain Proxy Domain (optional). Used for Rest requests only: service discovery, auth." << endl
		<< endl
		<< "Optional parameters of proxy server for Rest requests. Program config way." << endl
		<< " -prestProxyHost Proxy host name (optional). Used for Rest requests only: service discovery, auth." << endl
		<< " -prestProxyPort Proxy port number (optional). Used for Rest requests only: service discovery, auth." << endl
// END API QA
	;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumerConfig config;
		UInt8 userNameSet = 0;
		UInt8 passwordSet = 0;
		UInt8 clientIdSet = 0;
		UInt8 clientSecretSet = 0;
		UInt8 clientJWKSet = 0;

		FILE* pFile;
		int readSize;
		EmaString clientJwk;
		char clientJwkMem[2048];
// API QA
		Map configDb;
// END API QA

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
				if ( i < (argc - 1) )
				{
					userNameSet = 1;
					config.username( argv[++i] );
				}
			}
			else if ( strcmp( argv[i], "-password" ) == 0 )
			{
				if ( i < (argc - 1) )
				{
					passwordSet = 1;
					config.password( argv[++i] );
				}
			}
			else if ( strcmp( argv[i], "-clientId" ) == 0 )
			{
				if ( i < (argc - 1) )
				{
					clientIdSet = 1;
					config.clientId( argv[++i] );
				}
			}
			else if (strcmp(argv[i], "-clientSecret") == 0)
			{
				if (i < (argc - 1))
				{
					clientSecretSet = 1;
					config.clientSecret(argv[++i]);
				}
			}
			else if (strcmp(argv[i], "-audience") == 0)
			{
				if (i < (argc - 1))
				{
					clientSecretSet = 1;
					config.audience(argv[++i]);
				}
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

					clientJwk.set(clientJwkMem, readSize);

					config.clientJWK(clientJwk);
				}
			}
			else if (strcmp(argv[i], "-takeExclusiveSignOnControl") == 0)
			{
				if (i < (argc - 1))
				{
					EmaString takeExclusiveSignOnControlStr = argv[++i];

					if (takeExclusiveSignOnControlStr.caseInsensitiveCompare("true"))
					{
						config.takeExclusiveSignOnControl( true );
					}
					else if (takeExclusiveSignOnControlStr.caseInsensitiveCompare("false"))
					{
						config.takeExclusiveSignOnControl( false );
					}
				}
			}
			else if (strcmp(argv[i], "-tokenURL") == 0)
			{
				if ( i < (argc - 1) )
				{
					config.tokenServiceUrlV1( argv[++i] );
				}
			}
			else if (strcmp(argv[i], "-tokenURLV1") == 0)
			{
				if (i < (argc - 1))
				{
					config.tokenServiceUrlV1(argv[++i]);
				}
			}
			else if (strcmp(argv[i], "-tokenURLV2") == 0)
			{
				if (i < (argc - 1))
				{
					config.tokenServiceUrlV2(argv[++i]);
				}
			}
			else if (strcmp(argv[i], "-serviceDiscoveryURL") == 0)
			{
				if ( i < (argc - 1) )
				{
					config.serviceDiscoveryUrl( argv[++i] );
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
				config.tunnelingProxyHostName( i < ( argc - 1 ) ? argv[++i] : NULL );
			}
			else if ( strcmp( argv[i], "-pp" ) == 0 )
			{
				config.tunnelingProxyPort( i < ( argc - 1 ) ? argv[++i] : NULL );
			}
			else if ( strcmp( argv[i], "-plogin" ) == 0 )
			{
				config.proxyUserName( i < (argc - 1 ) ? argv[++i] : NULL );
			}
			else if ( strcmp( argv[i], "-ppasswd" ) == 0 )
			{
				config.proxyPasswd( i < ( argc - 1 ) ? argv[++i] : NULL );
			}
			else if ( strcmp( argv[i], "-pdomain" ) == 0)
			{
				config.proxyDomain( i < (argc - 1 ) ? argv[++i] : NULL );
			}
// API QA
			// The command-line options for function call.
			else if (strcmp(argv[i], "-restProxyHost") == 0)
			{
				config.restProxyHostName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-restProxyPort") == 0)
			{
				config.restProxyPort(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-restProxyUserName") == 0)
			{
				config.restProxyUserName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-restProxyPasswd") == 0)
			{
				config.restProxyPasswd(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-restProxyDomain") == 0)
			{
				config.restProxyDomain(i < (argc - 1) ? argv[++i] : NULL);
			}
			// The command-line options for programmatic config.
			else if (strcmp(argv[i], "-prestProxyHost") == 0)
			{
				if ( i < ( argc - 1 ) ) restProxyHostName_pc.set( argv[++i] );
			}
			else if (strcmp(argv[i], "-prestProxyPort") == 0)
			{
				if ( i < ( argc - 1 ) ) restProxyPort_pc.set( argv[++i] );
			}
// END API QA
		}

		if ( (!userNameSet || !passwordSet || !clientIdSet) && (!clientIdSet || (!clientSecretSet && !clientJWKSet) ))
		{
			cout << "User name, password and client Id or client Id and client secret must be specified on the command line. Exiting..." << endl;
			printHelp();
			return -1;
		}

		// use the "Consumer_4" to select EncryptedProtocolType::RSSL_SOCKET predefined in EmaConfig.xml
		EmaString consumerName = "Consumer_4";

		if (connectWebSocket)
		{
			// use the "Consumer_5" to select EncryptedProtocolType::RSSL_WEBSOCKET predefined in EmaConfig.xml
			consumerName.set( "Consumer_5" );
		}

// API QA
		if ( !restProxyHostName_pc.empty() || !restProxyPort_pc.empty() )
		{
			createProgrammaticConfig(configDb, consumerName);
			config.config( configDb );
		}
// END API QA

		OmmConsumer consumer( config.consumerName( consumerName ) );
		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( itemName ), client );
		sleep( 900000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
