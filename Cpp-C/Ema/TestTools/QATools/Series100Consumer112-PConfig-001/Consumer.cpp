///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
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

//APIQA
void createProgramaticConfig(Map& configMap)
{
        Map innerMap;
        ElementList elementList;

        elementList.addAscii("DefaultConsumer", "Consumer_3");

        innerMap.addKeyAscii("Consumer_3", MapEntry::AddEnum,
                ElementList()
                .addAscii("Channel", "Channel_1")
                .addAscii("Logger", "Logger_2")
                .addUInt("ItemCountHint", 5000)
                .addUInt("MsgKeyInUpdates", 1).complete()).complete();

        elementList.addMap("ConsumerList", innerMap);

        elementList.complete();
        innerMap.clear();

        configMap.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
        elementList.clear();

        innerMap.addKeyAscii("Channel_1", MapEntry::AddEnum,
                ElementList()
                .addEnum("ChannelType", 1)
                .addEnum("CompressionType", 0)
                .addUInt("GuaranteedOutputBuffers", 5000)
                .addUInt("ConnectionPingTimeout", 50000)
                .addAscii("Host", "localhost")
                .addAscii("Port", "14002")
                .addAscii("ObjectName", "P_ObjectName")
                .addAscii("ProxyHost", "proxyHostToConnectTo")
                .addAscii("ProxyPort", "proxyPortToConnectTo")
                .addAscii("OpenSSLCAStore", "ELBCASTORE")
                .addEnum("EncryptedProtocolType", 0)
                .addUInt("TcpNodelay", 1).complete()).complete();

        elementList.addMap("ChannelList", innerMap);

        elementList.complete();
        innerMap.clear();

        configMap.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
        elementList.clear();

        innerMap.addKeyAscii("Logger_2", MapEntry::AddEnum,
                ElementList()
                .addEnum("LoggerType", 1)
                .addAscii("FileName", "logFile")
                .addEnum("LoggerSeverity", 0).complete()).complete();

        elementList.addMap("LoggerList", innerMap);

        elementList.complete();
        innerMap.clear();

        configMap.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
        elementList.clear();

        innerMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
                ElementList()
                .addEnum("DictionaryType", 1)
                .addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
                .addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

        elementList.addMap("DictionaryList", innerMap);

        elementList.complete();

        configMap.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
        elementList.clear();
        configMap.complete();
}
//END APIQA

void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage\n"
		<< " -ph Proxy host name \n"
		<< " -pp Proxy port number \n"
        << " -plogin User name on proxy server \n"
        << " -ppasswd Password on proxy server \n"
        << " -pdomain Proxy Domain \n"
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
			if (argv[i] == "-?")
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
		}

		if (securityProtocol > 0)
			config.tunnelingSecurityProtocol(securityProtocol);
        //APIQA
        Map configMap;
        createProgramaticConfig(configMap);
        OmmConsumer consumer(config.config(configMap).username("user").consumerName("Consumer_3"));
        //END APIQA
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
