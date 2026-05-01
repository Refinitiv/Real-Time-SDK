/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

AppClient::AppClient() :
	_channelInfoVector()
{
}

AppClient::~AppClient()
{
	_channelInfoVector.clear();
}

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent)
{
	//API QA
	if (refreshMsg.getDomainType() == 1)
	{
		cout << refreshMsg.toString() << endl << "event session info (refresh)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << refreshMsg.toString() << endl << "event channel info (refresh)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmProviderEvent& ommEvent)
{
	//API QA
	if (updateMsg.getDomainType() == 1)
	{
		cout << updateMsg.toString() << endl << "event session info (update)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << updateMsg.toString() << endl << "event channel info (update)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent)
{
	//API QA
	if (statusMsg.getDomainType() == 1)
	{
		cout << statusMsg.toString() << endl << "event session info (status)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << statusMsg.toString() << endl << "event channel info (status)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::printSessionInfo( const OmmProviderEvent& ommEvent )
{
	ommEvent.getSessionInformation(_channelInfoVector);
	
	for(UInt32 i = 0; i < _channelInfoVector.size(); ++i)
	{
		cout << _channelInfoVector[i].toString() << endl;
	}
}
//API QA
void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
		<< " -plogin User name on proxy server \n"
		<< " -ppasswd Password on proxy server \n"
		<< " -pdomain Proxy Domain \n"
		<< " -spTLSv1.2 enable use of cryptographic protocol TLSv1.2 used with linux encrypted connections \n"
		<< " -spTLSv1.3 enable use of cryptographic protocol TLSv1.3 used with linux encrypted connections \n"
		<< " -cipher Optional TLS cipher suite string \n"
		<< " -cipherTLSv1.3 Optional TLS 1.3 cipher suite string \n"
		<< " -libsslName name of the libssl.so shared library used with linux encrypted connections. \n"
		<< " -libcryptoName name of the libcrypto.so shared library used with linux encrypted connections \n" << endl;
}
//END API QA

int main(int argc, char* argv[])
{
	try
	{
		//API QA
		OmmNiProviderConfig config;
		int securityProtocol = 0;

		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return 0;
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
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_2;
			}
			else if (strcmp(argv[i], "-spTLSv1.3") == 0)
			{
				securityProtocol |= OmmNiProviderConfig::ENC_TLSV1_3;
			}
			else if (strcmp(argv[i], "-cipher") == 0)
			{
				config.cipherSuite(i < (argc - 1) ? argv[++i] : NULL);
			}
			else if (strcmp(argv[i], "-cipherTLSv1.3") == 0)
			{
				config.cipherSuite_TLSV1_3(i < (argc - 1) ? argv[++i] : NULL);
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
		//END API QA

		AppClient appClient;
		
		//OmmProvider provider( OmmNiProviderConfig("EmaConfig.xml").providerName("Provider_Session").username( "user" ),appClient );
		OmmProvider provider(config.username("apiqa").providerName("Provider_Session"),appClient);
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

		for ( Int32 i = 0; i < 60; i++ )
		{
			sleep( 1000 );
			try
			{
				provider.submit(update.clear().serviceName("TEST_NI_PUB").name("IBM.N")
					.payload(fieldList.clear()
						.addReal(22, 14400 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 10 + i, OmmReal::Exponent0Enum)
						.complete()), ibmHandle);
				provider.submit(update.clear().serviceName("TEST_NI_PUB").name("TRI.N")
					.payload(fieldList.clear()
						.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 21 + i, OmmReal::Exponent0Enum)
						.complete()), triHandle);
			}
			catch( OmmInvalidUsageException& excp )
			{
				// There is no active channel currently associated with this session, so just continue until the channels are back up.
				if ( excp.getErrorCode() == OmmInvalidUsageException::NoActiveChannelEnum )
				{
					continue;
				}
				else
				{
					cout << excp << endl;
					break;
				}
			}
		}
	}

	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
