///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

bool connectWebSocket = false;

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
		<< " -username machine ID to perform authorization with the token service (mandatory)." << endl
		<< " -password password to perform authorization with the token service (mandatory)." << endl
		<< " -clientId client ID to perform authorization with the token service (mandatory)." << endl
		<< " -takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials (optional)." << endl
		<< " -itemName Request item name (optional)." << endl
		<< " -websocket Use the WebSocket transport protocol (optional)" << endl
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
		OmmConsumerConfig config;
		UInt8 userNameSet = 0;
		UInt8 passwordSet = 0;
		UInt8 clientIdSet = 0;

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
		}

		if ( !userNameSet || !passwordSet || !clientIdSet )
		{
			cout << "User name, password and client Id must be specified on the command line. Exiting...";
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

		OmmConsumer consumer( config.consumerName( consumerName ) );
		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( itemName ), client );
		sleep( 900000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
