///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace thomsonreuters::ema::access;
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
                .addAscii("LibsslName", "libssl.so")
                .addAscii("LibcryptoName", "libcrypto.so")
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
                .addEnum("SecurityProtocol", 7)
                .addAscii("Host", "localhost")
                .addAscii("Port", "14002")
                .addAscii("ObjectName", "P_ObjectName")
                .addAscii("ProxyHost", "proxyHostToConnectTo")
                .addAscii("ProxyPort", "proxyPortToConnectTo")
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
int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
                //APIQA
                Map configMap;
                createProgramaticConfig(configMap);
                OmmConsumer consumer(OmmConsumerConfig().config(configMap).username("user").consumerName("Consumer_3"));
                //END APIQA
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client );
		sleep( 3240000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
