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
		<< " -username user name to perform authorization with the token service." << endl
		<< " -password password to perform authorization with the token service." << endl
		<< " -location location to get an endpoint from EDP-RT service discovery. Defaults to \"us-east\"" << endl
		<< " -clientId client ID to perform authorization with the token service. The user name is used if not specified." << endl;
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
		}

		if ( !userName.length() || !password.length() )
		{
			cout << "Both username and password must be specified on the command line. Exiting...";
			printHelp();
			return -1;
		}

		// Query endpoints from EDP-RT service discovery for the TCP protocol
		serviceDiscovery.registerClient( ServiceEndpointDiscoveryOption().username( userName ).password( password )
			.clientId( clientId ).transprot( ServiceEndpointDiscoveryOption::TcpEnum ), client );

		if ( !host.length() || !port.length() )
		{
			cout << "Both hostname and port are not avaiable for establishing a connection with ERT in cloud. Exiting..." << endl;
			return -1;
		}

		createProgramaticConfig( configDb );

		OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_1" ).username( userName ).password( password )
			.clientId( clientId ).config( configDb ) );

		consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( "IBM.N" ), client );
		sleep( 900000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
