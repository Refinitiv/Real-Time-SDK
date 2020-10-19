///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

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
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
		<< " -plogin User name on proxy server \n"
		<< " -ppasswd Password on proxy server \n"
		<< " -pdomain Proxy Domain \n"
			//APIQA
		<< " -objectname ObjectName \n"
		<< " -spTLSv1.2 enable use of cryptopgrahic protocol TLSv1.2 used with linux encrypted connections \n"
		<< " -libsslName name of the libssl.so shared library used with linux encrypted connections. \n"
		<< " -libcryptoName name of the libcrypto.so shared library used with linux encrypted connections \n" << endl;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumerConfig config;
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
			else if (strcmp(argv[i], "-plogin") == 0)
			{
				config.proxyUserName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-ppasswd") == 0)
			{
				config.proxyPasswd(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-pdomain") == 0)
			{
				config.proxyDomain(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-spTLSv1.2") == 0)
			{
				securityProtocol |= OmmConsumerConfig::ENC_TLSV1_2;
			}
			else if (strcmp(argv[i], "-libsslName") == 0)
			{
				config.tunnelingLibSslName(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-libcryptoName") == 0)
			{
				config.tunnelingLibCryptoName(i < (argc - 1) ? argv[++i] : NULL);
			}
			//APIQA
			else if (strcmp(argv[i], "-objectname") == 0) 
			{
				config.tunnelingObjectName(i < (argc - 1) ? argv[++i] : NULL);
			}
			//END APIQA
		}

		if (securityProtocol > 0)
			config.tunnelingSecurityProtocol(securityProtocol);

		OmmConsumer consumer(config.username("user").consumerName("Consumer_3"));
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
