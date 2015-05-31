///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( refreshMsg.hasMsgKey() )
		cout << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << ( refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	cout << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( updateMsg.hasMsgKey() )
		cout << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << ( updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( statusMsg.hasMsgKey() )
		cout << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << ( statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const Msg& msg )
{
	switch ( msg.getAttrib().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( msg.getAttrib().getElementList() );
		break;
	}

	switch ( msg.getPayload().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( msg.getPayload().getElementList() );
		break;

	case DataType::FieldListEnum:
		decode( msg.getPayload().getFieldList() );
		break;

	case DataType::SeriesEnum:
		decode( msg.getPayload().getSeries() );
	}
}

void AppClient::decode( const Series & sr )
{
	switch ( sr.getSummary().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( sr.getSummary().getElementList() );
		break;
	default:
		cout << endl;
		break;
	}

	while ( !sr.forth() )
	{
		const SeriesEntry& se = sr.getEntry();

		cout << "DataType: " << DataType( se.getLoad().getDataType() ) << " Value: " << endl;

		switch ( se.getLoadType() )
		{
		case DataType::ElementListEnum:
			decode( se.getElementList() );
			break;
		default:
			cout << endl;
			break;
		}
	}
}

void AppClient::decode( const ElementList& el )
{
	while ( !el.forth() )
	{
		const ElementEntry& ee = el.getEntry();

		cout << "Name: " << ee.getName() << " DataType: " << DataType( ee.getLoad().getDataType() ) << " Value: ";

		if ( ee.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( ee.getLoadType() )
			{
			case DataType::RealEnum:
				cout << ee.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum:
				cout << (UInt64)ee.getDate().getDay() << " / " << (UInt64)ee.getDate().getMonth() << " / " << (UInt64)ee.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum:
				cout << (UInt64)ee.getTime().getHour() << ":" << (UInt64)ee.getTime().getMinute() << ":" << (UInt64)ee.getTime().getSecond() << ":" << (UInt64)ee.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum:
				cout << ee.getInt() << endl;
				break;
			case DataType::UIntEnum:
				cout << ee.getUInt() << endl;
				break;
			case DataType::AsciiEnum:
				cout << ee.getAscii() << endl;
				break;
			case DataType::ErrorEnum:
				cout << ee.getError().getErrorCode() << "( " << ee.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum:
				cout << ee.getEnum() << endl;
				break;
			case DataType::ArrayEnum:
				cout << endl;
				decode( ee.getArray() );
				break;
			default:
				cout << endl;
				break;
			}
	}
}

void AppClient::decode( const OmmArray& ommArray )
{
	while ( !ommArray.forth() )
	{
		const OmmArrayEntry& ae = ommArray.getEntry();

		cout << "DataType: " << DataType( ae.getLoad().getDataType() ) << " Value: ";

		if ( ae.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( ae.getLoadType() )
			{
			case DataType::RealEnum:
				cout << ae.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum:
				cout << (UInt64)ae.getDate().getDay() << " / " << (UInt64)ae.getDate().getMonth() << " / " << (UInt64)ae.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum:
				cout << (UInt64)ae.getTime().getHour() << ":" << (UInt64)ae.getTime().getMinute() << ":" << (UInt64)ae.getTime().getSecond() << ":" << (UInt64)ae.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum:
				cout << ae.getInt() << endl;
				break;
			case DataType::UIntEnum:
				cout << ae.getUInt() << endl;
				break;
			case DataType::AsciiEnum:
				cout << ae.getAscii() << endl;
				break;
			case DataType::ErrorEnum:
				cout << ae.getError().getErrorCode() << "( " << ae.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum:
				cout << ae.getEnum() << endl;
				break;
			default:
				cout << endl;
				break;
			}
	}
}

void AppClient::decode( const FieldList& fl )
{
	while ( !fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		cout << "Fid: " << fe.getFieldId() << " Name: " << fe.getName() << " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";

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
				cout << fe.getEnum() << endl;
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
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		void* closure = (void*)1;
		
		// Open Dictionary streams
		UInt64 fldHandle = consumer.registerClient( ReqMsg().name("RWFFld").filter( DICTIONARY_NORMAL ).domainType( MMT_DICTIONARY ), client, closure );

		UInt64 enumHandle = consumer.registerClient( ReqMsg().name("RWFEnum").filter( DICTIONARY_NORMAL ).domainType( MMT_DICTIONARY ), client, closure );
		
		UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );

		sleep( 300000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}