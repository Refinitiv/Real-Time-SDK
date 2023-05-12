///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2023 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

DataDictionary channelDictionary;
bool fldDictComplete = false;
bool enumTypeComplete = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );

	decode(refreshMsg, refreshMsg.getComplete());
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}
//APIQA
void AppClient::decode(const Msg& msg, bool complete)
{
	switch (msg.getPayload().getDataType())
	{
		case DataType::FieldListEnum:
			decode(msg.getPayload().getFieldList());
			break;

		case DataType::SeriesEnum:
		{
			if (msg.getName() == "RWFFld")
			{
				channelDictionary.decodeFieldDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

				if (complete)
				{
					fldDictComplete = true;
				}
			}

			else if (msg.getName() == "RWFEnum")
			{
				channelDictionary.decodeEnumTypeDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

				if (complete)
				{
					enumTypeComplete = true;
				}
			}

			if (fldDictComplete && enumTypeComplete)
			{
				cout << channelDictionary << endl;
			}
		}
	}
}
//END APIQA
void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Name: " << fe.getName() << " Value: ";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( fe.getLoadType() )
			{
			case DataType::RealEnum :
				cout << fe.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum :
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum :
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum :
				cout << fe.getInt() << endl;
				break;
			case DataType::UIntEnum :
				cout << fe.getUInt() << endl;
				break;
			case DataType::AsciiEnum :
				cout << fe.getAscii() << endl;
				break;
			case DataType::ErrorEnum :
				cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum :
				fe.hasEnumDisplay() ? cout << fe.getEnumDisplay() << endl : cout << fe.getEnum() << endl;
				break;
			case DataType::RmtesEnum:
				cout << fe.getRmtes().toString() << endl;
				break;
			default :
				cout << endl;
				break;
			}
	}
}


int main(int argc, char **argv)
{ 
	try {
		//APIQA
		bool loadLocalDictTwoConsSeq = false;
		bool loadChannelDictTwoConsSeq = false;
		bool loadLocalDictTwoConsConcur = true;
		bool loadChannelDictTwoConsConcur = false;
        bool shouldCopyIntoAPI = false;

		for (int iargs = 1; iargs < argc; ++iargs)
		{
			if (strcmp("-loadLocalDictTwoConsSeq", argv[iargs]) == 0)
			{
				loadLocalDictTwoConsSeq = true;
			}
			else if (strcmp("-loadChannelDictTwoConsSeq", argv[iargs]) == 0)
			{
				loadChannelDictTwoConsSeq = true;
			}
			else if (strcmp("-loadLocalDictTwoConsConcur", argv[iargs]) == 0)
			{
				loadLocalDictTwoConsConcur = true;
			}
			else if (strcmp("-loadChannelDictRunTwoConsumerConcurrently", argv[iargs]) == 0)
			{
				loadChannelDictRunTwoConsumerConcurrently = true;
			}
            else if (strcmp("-shouldCopyIntoAPI", argv[iargs]) == 0)}
            {
                shouldCopyIntoAPI = true;
            }
			else
			{
				printf("Error: Unrecognized option: %s\n\n", argv[iargs]);
				exit(-1);
			}
		}

		if (loadLocalDictTwoConsSeq)
		{
			AppClient client1, client2;

			DataDictionary localDictionary;
			localDictionary.loadFieldDictionary("RDMFieldDictionary");
			localDictionary.loadEnumTypeDictionary("enumtype.def");

			{
				OmmConsumerConfig config1;

				OmmConsumer consumer1(config1.dataDictionary(localDictionary, shouldCopyIntoAPI));
				consumer1.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client1);
				sleep(60000);
			}
			{
				OmmConsumerConfig  config2;
				OmmConsumer consumer2(config2.dataDictionary(localDictionary, shouldCopyIntoAPI));
				consumer2.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("TRI.N"), client2);
				sleep(60000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}
		}
		else if (loadChannelDictTwoConsSeq)
		{
			OmmConsumerConfig config, config1, config2;
			{
				/*Load Dictionary from channel */

				AppClient client;
				void* closure = (void*)1;
				OmmConsumer consumer(OmmConsumerConfig().username("user"));

				// Open Dictionary streams
				UInt64 fldHandle = consumer.registerClient(ReqMsg().name("RWFFld").filter(DICTIONARY_NORMAL).domainType(MMT_DICTIONARY), client, closure);

				UInt64 enumHandle = consumer.registerClient(ReqMsg().name("RWFEnum").filter(DICTIONARY_NORMAL).domainType(MMT_DICTIONARY), client, closure);

				UInt64 itemHandle = consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client, closure);

				sleep(6000);			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}
			{
				AppClient client1;
				/*Dictionary loaded from channel, so use it now*/
				OmmConsumer consumer1(config1.dataDictionary(channelDictionary, shouldCopyIntoAPI));
				consumer1.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client1);
				sleep(6000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}
			{
				AppClient client2;
				OmmConsumer consumer2(config2.dataDictionary(channelDictionary, shouldCopyIntoAPI));
				consumer2.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("TRI.N"), client2);
				sleep(6000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}
		}
		else if (loadLocalDictTwoConsConcur)
		{
			AppClient client1, client2;
			OmmConsumerConfig  config1, config2;
			DataDictionary localDictionary;

			localDictionary.loadFieldDictionary("RDMFieldDictionary");
			localDictionary.loadEnumTypeDictionary("enumtype.def");

			OmmConsumer consumer1(config1.dataDictionary(localDictionary, shouldCopyIntoAPI));
			consumer1.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client1);
		
			OmmConsumer consumer2(config2.dataDictionary(localDictionary, shouldCopyIntoAPI));
			consumer2.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("TRI.N"), client2);
			sleep(6000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
		
		}
		else if (loadChannelDictRunTwoConsumerConcurrently)
		{
			{
				/*Load Dictionary from channel */
				AppClient client;
				void* closure = (void*)1;
				OmmConsumer consumer(OmmConsumerConfig().username("user"));

				// Open Dictionary streams
				UInt64 fldHandle = consumer.registerClient(ReqMsg().name("RWFFld").filter(DICTIONARY_NORMAL).domainType(MMT_DICTIONARY), client, closure);

				UInt64 enumHandle = consumer.registerClient(ReqMsg().name("RWFEnum").filter(DICTIONARY_NORMAL).domainType(MMT_DICTIONARY), client, closure);

				UInt64 itemHandle = consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client, closure);

				sleep(6000);			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}

			AppClient client1, client2;
			OmmConsumerConfig config1, config2;

			/*Dictionary loaded from channel, so use it now*/
			OmmConsumer consumer1(config1.dataDictionary(channelDictionary, shouldCopyIntoAPI));
			consumer1.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client1);
				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()


			OmmConsumer consumer2(config2.dataDictionary(channelDictionary, shouldCopyIntoAPI));
			consumer2.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("TRI.N"), client2);
			sleep(60000);				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
		}
		//END APIQA
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
