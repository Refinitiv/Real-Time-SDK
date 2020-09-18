///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

DataDictionary dataDictionary;
bool fldDictComplete = false;
bool enumTypeComplete = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) ) << endl
		<< "Service Id: " << (refreshMsg.hasServiceId() ? refreshMsg.getServiceId() : 0  );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;
	cout << endl << "REFRESH COMPLETE FLAG: " << refreshMsg.getComplete() << endl;

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
				dataDictionary.decodeFieldDictionary(msg.getPayload().getSeries(), DICTIONARY_INFO);

				if (complete)
				{
					fldDictComplete = true;
				}
			}

			else if (msg.getName() == "RWFEnum")
			{
				dataDictionary.decodeEnumTypeDictionary(msg.getPayload().getSeries(), DICTIONARY_INFO);

				if (complete)
				{
					enumTypeComplete = true;
				}
			}
				cout << dataDictionary << endl;
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

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		void* closure = (void*)1;
		// Open Dictionary streams
		//APIQA
		UInt64 fldHandle = consumer.registerClient( ReqMsg().name("RWFFld").filter( DICTIONARY_INFO ).domainType( MMT_DICTIONARY ).serviceName( "DIRECT_FEED" ), client, closure );

		UInt64 enumHandle = consumer.registerClient( ReqMsg().name("RWFEnum").filter( DICTIONARY_INFO ).domainType(MMT_DICTIONARY).serviceName( "DIRECT_FEED" ), client, closure);
		// END APIQA
		UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );
		sleep( 180000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
