///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2019. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace thomsonreuters::ema::access;
using namespace std;

EmaString userName;
EmaString password;
EmaString clientId;
EmaString host;
EmaString port;
EmaString location("us-east");
EmaString proxyHostName;
EmaString proxyPort;
EmaString proxyUserName;
EmaString proxyPasswd;
EmaString proxyDomain;

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
	cout << "Failed to query EDP-RT service discovery. Error text: " << statusText << endl;
}

void createProgramaticConfig( Map& configDb )
{
	Map elementMap;
	ElementList elementList;

	elementMap.addKeyAscii( "Consumer_1", MapEntry::AddEnum,
		ElementList().addAscii( "Channel", "Channel_1" ).complete() ).complete();

	elementList.addMap( "ConsumerList", elementMap );

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii( "ConsumerGroup", MapEntry::AddEnum, elementList );
	elementList.clear();

	elementMap.addKeyAscii( "Channel_1", MapEntry::AddEnum,
		ElementList()
		.addEnum( "ChannelType", 1 ) // Use the RSSL_CONN_TYPE_ENCRYPTED connection
		.addAscii( "Host", host )
		.addAscii( "Port", port )
		.addUInt( "EnableSessionManagement", 1 )
		.addEnum( "EncryptedProtocolType", 0 ) // Use the standard TCP transport protocol and OpenSSL for encryption on Windows
		.complete() ).complete();

	elementList.addMap( "ChannelList", elementMap );

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii( "ChannelGroup", MapEntry::AddEnum, elementList );
	configDb.complete();
}

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< " -username machine ID to perform authorization with the token service (mandatory)." << endl
		<< " -password password to perform authorization with the token service (mandatory)." << endl
		<< " -clientId client ID to perform authorization with the token service (mandatory). " << endl
		<< " -location location to get an endpoint from EDP-RT service discovery (optional). Defaults to \"us-east\"" << endl
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
		ServiceEndpointDiscovery serviceDiscovery;

		for ( int i = 1; i < argc; i++ )
		{
			if ( strcmp( argv[i], "-?" ) == 0 )
			{
				printHelp();
				return false;
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

		if ( !userName.length() || !password.length() || !clientId.length() )
		{
			cout << "User name, password and client Id must be specified on the command line. Exiting...";
			printHelp();
			return -1;
		}

		// Query endpoints from EDP-RT service discovery for the TCP protocol
		serviceDiscovery.registerClient( ServiceEndpointDiscoveryOption().username( userName ).password( password )
			.clientId( clientId ).transport( ServiceEndpointDiscoveryOption::TcpEnum )
			.proxyHostName( proxyHostName ).proxyPort( proxyPort ).proxyUserName( proxyUserName ).proxyPassword( proxyPasswd )
			.proxyDomain( proxyDomain ), client );

		if ( !host.length() || !port.length() )
		{
			cout << "Both hostname and port are not available for establishing a connection with ERT in cloud. Exiting..." << endl;
			return -1;
		}

		createProgramaticConfig( configDb );

		OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_1" ).username( userName ).password( password )
			.clientId( clientId ).config( configDb ).tunnelingProxyHostName( proxyHostName ).tunnelingProxyPort( proxyPort )
			.proxyUserName( proxyUserName ).proxyPasswd( proxyPasswd ).proxyDomain( proxyDomain ) );

		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( "IBM.N" ), client );
		sleep( 900000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
