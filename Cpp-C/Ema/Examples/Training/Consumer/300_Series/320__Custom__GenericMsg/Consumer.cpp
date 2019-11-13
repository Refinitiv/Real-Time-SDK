///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    " << "RefreshMsg" << endl << "Item Handle: " << ommEvent.getHandle() << endl << "Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;
	
	// submit a generic message when stream becomes open / ok
	if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum &&
		refreshMsg.getState().getDataState() == OmmState::OkEnum )
		_pOmmConsumer->submit( GenericMsg().domainType( 200 ).name( "genericMsg" ).payload( ElementList().addInt( "value", ++count ).complete() ), ommEvent.getHandle() );

	decode( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    " << "UpdateMsg" << endl << "Item Handle: " << ommEvent.getHandle() << endl << "Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    " << "StatusMsg" << endl << "Item Handle: " << ommEvent.getHandle() << endl << "Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::onGenericMsg( const GenericMsg& genMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    GenericMsg" << endl << "Item Handle: " << ommEvent.getHandle() << endl << "Closure:     " << ommEvent.getClosure() << endl;

	_pOmmConsumer->submit( GenericMsg().domainType( 200 ).name( "genericMsg" ).payload( ElementList().addInt( "value", ++count ).complete() ), ommEvent.getHandle() );

	decode( genMsg );
}

void AppClient::decode( const Msg& msg )
{

	switch ( msg.getAttrib().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( msg.getAttrib().getElementList() );
		break;
	case DataType::FieldListEnum:
		decode( msg.getAttrib().getFieldList() );
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
	}
}

void AppClient::decode( const GenericMsg& genMsg )
{
	if ( genMsg.hasServiceId() )
		cout << "ServiceId: " << genMsg.getServiceId() << endl;

	if ( genMsg.hasPartNum() )
		cout << "PartNum:  " << genMsg.getPartNum() << endl;

	if ( genMsg.hasSeqNum() )
		cout << "SeqNum:   " << genMsg.getSeqNum() << endl;

	switch ( genMsg.getAttrib().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( genMsg.getAttrib().getElementList() );
		break;
	case DataType::FieldListEnum:
		decode( genMsg.getAttrib().getFieldList() );
		break;
	}

	switch ( genMsg.getPayload().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( genMsg.getPayload().getElementList() );
		break;
	case DataType::FieldListEnum:
		decode( genMsg.getPayload().getFieldList() );
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

AppClient::AppClient() :
 count( 0 )
{
}

void AppClient::setOmmConsumer( OmmConsumer& consumer )
{
	_pOmmConsumer = &consumer;
}

int main()
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).username( "user" ) );
		client.setOmmConsumer( consumer );
		void* closure = (void*)1;
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( 200 ).serviceName( "DIRECT_FEED" ).name( "IBM.XYZ" ), client, closure );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
