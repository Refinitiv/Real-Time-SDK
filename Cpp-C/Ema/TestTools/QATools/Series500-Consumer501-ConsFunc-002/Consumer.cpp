///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024, 2025 LSEG. All rights reserved.             --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace std;

UInt32 enablePH = 1;
EmaString channelNamePreferred = "Channel_1";
UInt32 detectionTimeInterval = 15;
EmaString detectionTimeSchedule = "*/15 * * * * *"; // each 15 seconds

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent&)
{
	cout << refreshMsg << endl;
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
{
	cout << updateMsg << endl;
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent&)
{
	if (statusMsg.getState().getStatusCode() == OmmState::StatusCode::SocketPHComplete) 
	{
		cout <<"Status text"<<statusMsg.getState().getStatusText()<<endl;
	}

	cout << statusMsg << endl;
}

void createProgramaticConfig(Map& configDb)
{
	Map elementMap;
	ElementList elementList;

	elementList.addAscii("DefaultConsumer", "Consumer_9");
	
	ElementList elList;

	 elList.addAscii("ChannelSet", "Channel_1, Channel_2")
		.addAscii("Logger", "Logger_1")
		.addUInt("EnablePreferredHostOptions", enablePH)
		.addAscii("PreferredChannelName", channelNamePreferred);
	
	if (detectionTimeInterval > 0)
		elList.addUInt("PHDetectionTimeInterval", detectionTimeInterval);
	if (!detectionTimeSchedule.empty())
		elList.addAscii("PHDetectionTimeSchedule", detectionTimeSchedule);
		
	elList.addAscii("Dictionary", "Dictionary_1")
		.addUInt("XmlTraceToStdout", 0).complete();

	elementMap.addKeyAscii("Consumer_9", MapEntry::AddEnum, elList).complete();

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

	elementMap.addKeyAscii("Logger_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("LoggerType", 1)
		.addAscii("FileName", "logFile")
		.addEnum("LoggerSeverity", 0).complete()).complete();

	elementList.addMap("LoggerList", elementMap);

	elementList.complete();
	elementMap.clear();

	configDb.addKeyAscii("LoggerGroup", MapEntry::AddEnum, elementList);
	elementList.clear();

	elementMap.addKeyAscii("Dictionary_1", MapEntry::AddEnum,
		ElementList()
		.addEnum("DictionaryType", 0)
		.addAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary")
		.addAscii("EnumTypeDefFileName", "./enumtype.def").complete()).complete();

	elementList.addMap("DictionaryList", elementMap);

	elementList.complete();

	configDb.addKeyAscii("DictionaryGroup", MapEntry::AddEnum, elementList);
	elementList.clear();


	configDb.complete();
}

int main(int argc, char* argv[])
{
	int i = 1;

	while (i < argc)
	{
		if (strcmp(argv[i], "-enablePH") == 0)
		{
			enablePH = 1;
		}
		else if (strcmp(argv[i], "-channelNamePreferred") == 0) 
		{
			i++;
			channelNamePreferred = argv[i];
		}
		else if (strcmp(argv[i], "-detectionTimeInterval") == 0)
		{
			i++;
			detectionTimeInterval = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-detectionTimeSchedule") == 0)
		{
			detectionTimeSchedule.clear();
			i++;

			if ( argv[i] == NULL || *argv[i] == '-') { continue; }

			do {
				
				detectionTimeSchedule += argv[i];
				
				if (argv[i+1] != NULL && *argv[i+1] != '-' ) detectionTimeSchedule += " ";
				else break;
				
				i++;
			}
			while ( 1 );
		}
		else 
		{
			cout << "Unknown argument: " << argv[i] << endl;
			exit(-1);
		}

		i++;
	}

	try {
		AppClient client;
		Map configMap;
		ChannelInformation channelInfo;
		createProgramaticConfig( configMap );
		OmmConsumer consumer(OmmConsumerConfig().config( configMap ), client);	// use programmatic configuration parameters
		consumer.registerClient(ReqMsg().name("IBM.N").serviceName("DIRECT_FEED"), client);

		for (int i = 1; i < 60; i++)
		{
			consumer.getChannelInformation(channelInfo);
			cout << channelInfo << endl;
			channelInfo.clear();
			sleep(1000);
		}
	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}

	return 0;
}
