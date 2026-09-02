/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

int wsbmode = 1;
EmaString host1 = "localhost";
EmaString host2 = "localhost";
EmaString port1 = "14002";
EmaString port2 = "14003";
EmaString dicttype = "Dictionary_2";
EmaString serviceName1 = "ELEKTRON_DD";
EmaString serviceName2 = "DIRECT_FEED";
EmaString itemName = "LSEG.L";
int serviceId1 = 0;
int serviceId2 = 1;
int dirfilter = -1;


void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	//API QA
	cout << endl << "Received RefreshMsg domain: " << refreshMsg.getDomainType() << endl;
	if (refreshMsg.getDomainType() != 5)
	{
		cout << refreshMsg << endl;
	}
	if (refreshMsg.getComplete() == 1)
	{
		cout << "QA prints Received Refresh Completed for Domain : " << refreshMsg.getDomainType() << endl;
	}

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
		.addUInt("XmlTraceToStdout", 1)
		.addUInt("XmlTraceToFile", 1)
		.addAscii("XmlTraceFileName", "traceXML")
		.addAscii("Dictionary", dicttype)
		.complete())
	.complete();

	elementList.addMap("ConsumerList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("ConsumerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("Channel_1", MapEntry::AddEnum, ElementList()
		.addEnum("ChannelType", 0)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", host1)
		.addAscii("Port", port1)
		.addUInt("TcpNodelay", 0).complete());

	elementMap.addKeyAscii("Channel_2", MapEntry::AddEnum, ElementList()
		.addEnum("ChannelType", 0)
		.addUInt("ConnectionPingTimeout", 50000)
		.addAscii("Host", host2)
		.addAscii("Port", port2)
		.addUInt("TcpNodelay", 0).complete());

	elementMap.complete();

	elementList.addMap("ChannelList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("ChannelGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("Server_Info_1", MapEntry::AddEnum, ElementList()
		.addAscii("Channel", "Channel_1")
		.addAscii("PerServiceNameSet", serviceName1).complete());

	elementMap.addKeyAscii("Server_Info_2", MapEntry::AddEnum, ElementList()
		.addAscii("Channel", "Channel_2")
	    .addAscii("PerServiceNameSet", serviceName1).complete());

	elementMap.complete();

	elementList.addMap("WarmStandbyServerInfoList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("WarmStandbyServerInfoGroup", MapEntry::AddEnum, elementList);
	elementList.clear();


	elementMap.addKeyAscii("WarmStandbyChannel_1", MapEntry::AddEnum, ElementList()
		.addAscii("StartingActiveServer", "Server_Info_1")
		.addAscii("StandbyServerSet", "Server_Info_2")
		.addEnum("WarmStandbyMode", wsbmode) /* 2 for service based while 1 for login based warm standby */
		.complete()).complete();
	elementList.addMap("WarmStandbyList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("WarmStandbyGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	//logger
	elementMap
		.addKeyAscii(
			"Logger_1", refinitiv::ema::access::MapEntry::AddEnum,
			refinitiv::ema::access::ElementList().addEnum("LoggerType", 1).addEnum("LoggerSeverity", 0).complete())
		.complete();

	elementList.addMap("LoggerList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("LoggerGroup", refinitiv::ema::access::MapEntry::AddEnum, elementList);
	elementList.clear();

	//dictionary
	elementMap
		.addKeyAscii(
			"Dictionary_1", refinitiv::ema::access::MapEntry::AddEnum,
			refinitiv::ema::access::ElementList().addEnum("DictionaryType", 1).complete())
		.addKeyAscii(
			"Dictionary_2", refinitiv::ema::access::MapEntry::AddEnum,
			refinitiv::ema::access::ElementList().addEnum("DictionaryType", 0)
			.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
			.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

	elementList.addMap("DictionaryList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("DictionaryGroup", refinitiv::ema::access::MapEntry::AddEnum, elementList);
	elementList.clear();

	configDb.complete();
}
void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< " -wsbmode for warm standby mode; 1 for login, 2 for service, default to 1. " << endl
		<< " -host1 for active_host. " << endl
		<< " -port1 for active_port. " << endl
		<< " -host2 for standby_host. " << endl
		<< " -port2 for standby_port." << endl
		<< " -dicttype dictionary type; Dictionary_1 for network, Dictionary_2 for file, defaulted to Dictionary_1" << endl
		<< " -s1 Request service name 1 (optional), defaulted to ELEKTRON_DD." << endl
		<< " -s2 Request service name 2 (optional), defaulted to DIRECT_FEED." << endl
		<< " -sid1 Request service id 1 (optional, defaulted to service name if sid is unspecified)." << endl
		<< " -sid2 Request service id 2 (optional, defaulted to service name if sid is unspecified)." << endl
		<< " -f Request source Directory filter, no default." << endl
		<< " -i Request item name (optional), defaulted to LSEG.L." <<  endl;
}

int main( int argc, char* argv[] )
{ 
	try {
		AppClient client;

		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return 0;
			}
			else if (strcmp(argv[i], "-wsbmode") == 0)
			{
				if (i < (argc - 1)) wsbmode = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-host1") == 0)
			{
				if (i < (argc - 1)) host1.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-port1") == 0)
			{
				if (i < (argc - 1)) port1.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-host2") == 0)
			{
				if (i < (argc - 1)) host2.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-port2") == 0)
			{
				if (i < (argc - 1)) port2.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-s1") == 0)
			{
				if (i < (argc - 1)) serviceName1.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-s2") == 0)
			{
				if (i < (argc - 1)) serviceName2.set(argv[++i]);
			}
			else if (strcmp(argv[i], "-sid1") == 0)
			{
				if (i < (argc - 1)) serviceId1 = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-sid2") == 0)
			{
				if (i < (argc - 1)) serviceId2 = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-f") == 0)
			{
				if (i < (argc - 1)) dirfilter = atoi(argv[++i]);
			}
			else if (strcmp(argv[i], "-i") == 0)
			{
				if (i < (argc - 1)) itemName.set(argv[++i]);
			}
		}

		UInt64 DirectoryHdl1{ 0 };
		UInt64 DirectoryHdl2{ 0 };

		Map configMap;
		createProgramaticConfig(configMap);
		OmmConsumer consumer(OmmConsumerConfig().config(configMap).username("user"));	// use programmatic configuration parameters
		sleep( 5000 ); // 5 seconds.
				
		if (serviceId1 > 0 && serviceId2 > 1)
		{
			cout << "QA prints : Request with serviceId." << endl;
			if (dirfilter != -1)
			{
					cout << "serviceId1 = " << serviceId1 << ". Filter = " << dirfilter << "." << endl;
					DirectoryHdl1 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceId(serviceId1)
						.filter(dirfilter),
						client);
					cout << "serviceId2 = " << serviceId2 << ". Filter = " << dirfilter << "." << endl;
					DirectoryHdl2 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceId(serviceId2)
						.filter(dirfilter),
						client);
			}
			else
			{
					cout << "serviceId1 = " << serviceId1 << ". No Filter." << endl;
					DirectoryHdl1 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceId(serviceId1),
						client);
					cout << "serviceId2 = " << serviceId2 << ". No Filter." << endl;
					DirectoryHdl2 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceId(serviceId2),
						client);
			}
			//Request item
			consumer.registerClient(ReqMsg().name(itemName).serviceId(serviceId1), client);
		}
		else
		{
			cout << "QA prints : Request with serviceName." << endl;

			//Request Directory/Source
			if (dirfilter != -1)
			{
					cout << "serviceName1 = " << serviceName1 << ". Filter = " << dirfilter << "." << endl;
					DirectoryHdl1 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceName(serviceName1)
						.filter(dirfilter),
						client);
					cout << "serviceName2 = " << serviceName2 << ". Filter = " << dirfilter << "." << endl;
					DirectoryHdl2 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceName(serviceName2)
						.filter(dirfilter),
						client);
			}
			else
			{
					cout << "serviceName1 = " << serviceName1 << ". No Filter." << endl;
					DirectoryHdl2 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceName(serviceName1),
						client);
					cout << "serviceName2 = " << serviceName2 << ". No Filter." << endl;
					DirectoryHdl2 = consumer.registerClient(refinitiv::ema::access::ReqMsg()
						.domainType(refinitiv::ema::rdm::MMT_DIRECTORY)
						.initialImage(true)
						.serviceName(serviceName2),
						client);
			}
			//Request item
			consumer.registerClient(ReqMsg().name(itemName).serviceName(serviceName1), client);
		}
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
		if (DirectoryHdl1 != 0)
			consumer.unregister(DirectoryHdl1);
		if (DirectoryHdl2 != 0)
			consumer.unregister(DirectoryHdl2);
		cout << "Program ended" << endl;

	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}

	return 0;
}