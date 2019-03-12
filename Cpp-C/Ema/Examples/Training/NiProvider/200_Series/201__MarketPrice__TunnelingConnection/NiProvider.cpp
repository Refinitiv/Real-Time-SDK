///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"
#include <cstring>

using namespace thomsonreuters::ema::access;
using namespace std;

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
		<< " -plogin User name on proxy server \n"
		<< " -ppasswd Password on proxy server \n"
		<< " -pdomain Proxy Domain \n"
		<< " -spTLSv1 enable use of cryptographic protocol TLSv1 used with linux encrypted connections \n"
		<< " -spTLSv1.1 enable use of cryptographic protocol TLSv1.1 used with linux encrypted connections \n"
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
			else if (strcmp(argv[i], "-spTLSv1") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1;
			}
			else if (strcmp(argv[i], "-spTLSv1.1") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_1;
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

		OmmProvider provider( config.username( "user" ).providerName( "Provider_4" ) );
		UInt64 ibmHandle = 5;
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		provider.submit( refresh.serviceName( "NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
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
			provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
				.payload( fieldList.clear()
					.addReal( 22, 14400 + i, OmmReal::ExponentNeg2Enum )
					.addReal( 30, 10 + i, OmmReal::Exponent0Enum )
					.complete() ), ibmHandle );
			provider.submit( update.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
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
