///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2021 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

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

void createProgramaticConfig( Map& configDb )
{
	Map elementMap;
	ElementList elementList;

	elementList.addAscii("DefaultConsumer", "Consumer_8");

	elementMap.addKeyAscii("Consumer_8", MapEntry::AddEnum, ElementList()
		.addAscii("WarmStandbyChannelSet", "WarmStandbyChannel_1")
		.addUInt("XmlTraceToStdout", 0).complete())
		.complete();

	elementList.addMap("ConsumerList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14002").complete());

	elementMap.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
		.addEnum("ChannelType", 0)
		.addAscii("Host", "localhost")
		.addAscii("Port", "14003").complete());

	elementMap.complete();

	elementList.addMap("ChannelList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("Server_Info_1", MapEntry::AddEnum, ElementList()
		.addAscii("Channel", "Channel_1")
		.addAscii("PerServiceNameSet", "DIRECT_FEED").complete());

	elementMap.addKeyAscii("Server_Info_2", MapEntry::AddEnum, ElementList()
		.addAscii("Channel", "Channel_2").complete());

	elementMap.complete();

	elementList.addMap("WarmStandbyServerInfoList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("WarmStandbyServerInfoGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("WarmStandbyChannel_1", MapEntry::AddEnum, ElementList()
		.addAscii("StartingActiveServer", "Server_Info_1")
		.addAscii("StandbyServerSet", "Server_Info_2")
		.addEnum("WarmStandbyMode", 2) /* 2 for service based while 1 for login based warm standby */
		.complete()).complete();

	elementList.addMap("WarmStandbyList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("WarmStandbyGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	configDb.complete();
}

int main( int argc, char* argv[] )
{ 
	try {
		AppClient client;
		Map configMap;
		createProgramaticConfig(configMap);
		OmmConsumer consumer(OmmConsumerConfig().config(configMap));	// use programmatic configuration parameters
		consumer.registerClient(ReqMsg().name("IBM.N").serviceName("DIRECT_FEED"), client);
		sleep(60000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}

	return 0;
}
