///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

DataDictionary dataDictionary;
bool fldDictComplete = false;
bool enumTypeComplete = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode(refreshMsg, refreshMsg.getComplete());
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

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
				dataDictionary.decodeFieldDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

				if (complete)
				{
					fldDictComplete = true;
				}
			}

			else if (msg.getName() == "RWFEnum")
			{
				dataDictionary.decodeEnumTypeDictionary(msg.getPayload().getSeries(), DICTIONARY_NORMAL);

				if (complete)
				{
					enumTypeComplete = true;
				}
			}

			if (fldDictComplete && enumTypeComplete)
			{
				//cout << dataDictionary << endl;
				// API QA
				const DictionaryEntry& entry22 = dataDictionary.getEntry(22);
				const DictionaryEntry& entry25 = dataDictionary.getEntry(25);
				cout << "QA Prints entry22 : " << entry22.getAcronym() << endl;
				cout << "QA Prints entry25 : " << entry25.getAcronym() << endl;
				if (entry22 == entry25)
					cout << "Test 1 : QA Prints check entry22 and entry25 are equal " << endl;
				const char* nameBID = "BID";
				const char* nameASK = "ASK";
				const DictionaryEntry& entryBID = dataDictionary.getEntry(nameBID);
				const DictionaryEntry& entryASK = dataDictionary.getEntry(nameASK);
				if (entryBID == entryASK)
					cout << "Test 2 : QA Prints check entryBID and entryASK are equal " << endl;

				DictionaryEntry entry30;
				dataDictionary.getEntry(30, entry30);
				cout << "QA Prints entry30 fname : " << entry30.getAcronym() << endl;

				DictionaryEntry entry31;
				dataDictionary.getEntry(31, entry31);
				cout << "QA Prints entry31 fname : " << entry31.getAcronym() << endl;

				if (entry30 == entry31)
					cout << "Test 3 : QA Prints check entry30 and entry31 are equal " << endl;
				else
					cout << "Test 3 : QA Prints check entry30 and entry31 are NOT equal " << endl;
				
				const char* nameBIDSIZE = "BIDSIZE";
				const char* nameASKSIZE = "ASKSIZE";
				DictionaryEntry entryBIDSIZE;
				dataDictionary.getEntry(nameBIDSIZE, entryBIDSIZE);
				cout << "QA Prints entryBIDSIZE fname : " << entryBIDSIZE.getFid() << endl;

				DictionaryEntry entryASKSIZE;
				dataDictionary.getEntry(nameASKSIZE, entryASKSIZE);
				cout << "QA Prints entryASKSIZE fname : " << entryASKSIZE.getFid() << endl;

				if (entryBIDSIZE == entryASKSIZE)
					cout << "Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are equal " << endl;
				else
					cout << "Test 4 : QA Prints check entryBIDSIZE and entryASKSIZE are NOT equal " << endl;


				cout << "Test 5 Error Case : Trying to use entry owned by API (entry22)..." << endl;
				const DictionaryEntry& dictionaryEntryFid22 = dataDictionary.getEntry(22);
				DictionaryEntry* dictionaryEntryInternalRef22 = const_cast<DictionaryEntry*>(&dictionaryEntryFid22);
				try
				{
					dataDictionary.getEntry(22, *dictionaryEntryInternalRef22);
				}
				catch (const OmmException& ex)
				{
					cout << " QA Prints Exception Type : " << ex.getExceptionTypeAsString() << endl;
					cout << " QA Prints Exception Message : " << ex.getText() << endl;
				}

				cout << "Test 6 Error Case : Trying to use entry owned by API (entry25)..." << endl;
				const DictionaryEntry& dictionaryEntryFid25 = dataDictionary.getEntry(nameASK);
				DictionaryEntry* dictionaryEntryInternalRef25 = const_cast<DictionaryEntry*>(&dictionaryEntryFid25);
				try
				{
					dataDictionary.getEntry(25, *dictionaryEntryInternalRef25);
				}
				catch (const OmmException& ex)
				{
					cout << " QA Prints Exception Type : " << ex.getExceptionTypeAsString() << endl;
					cout << " QA Prints Exception Message : " << ex.getText() << endl;
				}
				// END API QA
			}
		}
	}
}

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
			case DataType::RealEnum:
				cout << fe.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum:
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum:
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum:
				cout << fe.getInt() << endl;
				break;
			case DataType::UIntEnum:
				cout << fe.getUInt() << endl;
				break;
			case DataType::AsciiEnum:
				cout << fe.getAscii() << endl;
				break;
			case DataType::ErrorEnum:
				cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum:
				fe.hasEnumDisplay() ? cout << fe.getEnumDisplay() << endl : cout << fe.getEnum() << endl;
				break;
			case DataType::RmtesEnum:
				cout << fe.getRmtes().toString() << endl;
				break;
			default:
				cout << endl;
				break;
		}
	}
}

int main()
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		void* closure = (void*)1;
		
		// Open Dictionary streams
		UInt64 fldHandle = consumer.registerClient( ReqMsg().name("RWFFld").filter( DICTIONARY_NORMAL ).domainType( MMT_DICTIONARY ), client, closure );

		UInt64 enumHandle = consumer.registerClient( ReqMsg().name("RWFEnum").filter( DICTIONARY_NORMAL ).domainType( MMT_DICTIONARY ), client, closure );
		
		UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );

		sleep( 180000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
