///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA This file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg );
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
	case DataType::MapEnum:
		decode( msg.getPayload().getMap() );
		break;
	case DataType::FieldListEnum:
		decode( msg.getPayload().getFieldList() );
		break;
	}
}

void AppClient::decode( const ElementList& el )
{
	while ( el.forth() )
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
				decode(ee.getArray());
				break;
			default:
				cout << endl;
				break;
			}
	}
}

void AppClient::decode( const Map& map )
{
	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		switch ( me.getKey().getDataType() )
		{
		case DataType::AsciiEnum:
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getAscii() << endl;
			break;
		case DataType::BufferEnum:
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getBuffer() << endl;
			break;
		}

		switch ( me.getLoadType() )
		{
		case DataType::FilterListEnum:
			decode( me.getFilterList() );
			break;
		case DataType::ElementListEnum:
			decode( me.getElementList() );
			break;
		}
	}
}

void AppClient::decode( const FilterList& fl )
{
	while ( fl.forth() )
	{
		const FilterEntry& fe = fl.getEntry();

		cout << "ID: " << fe.getFilterId() << " Action: " << fe.getAction() <<  " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";
		switch ( fe.getLoadType() )
		{
		case DataType::ElementListEnum:
			decode( fe.getElementList() );
			break;
		case DataType::MapEnum:
			decode( fe.getMap() );
			break;
		default:
			cout << endl;
			break;
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

void AppClient::decode(const OmmArray& ommArray)
{
	while (ommArray.forth())
	{
		const OmmArrayEntry& ae = ommArray.getEntry();

		cout << "DataType: " << DataType(ae.getLoad().getDataType()) << " Value: ";

		if (ae.getCode() == Data::BlankEnum)
			cout << " blank" << endl;
		else
			switch (ae.getLoadType())
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

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		void* closure = (void*)1;

		// APIQA: Use invalid service for directoryHandle Reissue on closed handle
		// open directory stream
		UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).filter(29).serviceId(999), client, closure );
		cout <<"DIR REQUEST HANDLE"<< endl;
		cout <<"directoryHandle"<< endl;
		cout <<"REISSUE DIRECTORY"<< endl;
		consumer.reissue( ReqMsg().domainType(MMT_DIRECTORY).interestAfterRefresh(true).filter(3).serviceName("DIRECT_FEED"),dirHandle);
		// END APIQA:
		UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
