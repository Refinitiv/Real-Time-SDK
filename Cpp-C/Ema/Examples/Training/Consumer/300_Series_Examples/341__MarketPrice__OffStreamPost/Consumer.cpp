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
	cout << endl << "Received:    " << "RefreshMsg" << endl << "Handle: " << ommEvent.getHandle() << " Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( refreshMsg.getDomainType() == MMT_LOGIN && 
		refreshMsg.getState().getStreamState() == OmmState::OpenEnum &&
		refreshMsg.getState().getDataState() == OmmState::OkEnum )
	{
		_pOmmConsumer->submit( PostMsg().postId( 1 ).serviceId( 1501 ).name( "TRI.N" ).solicitAck( true ).complete().payload( 
		RefreshMsg().payload( FieldList().addReal( 25, 35, OmmReal::ExponentPos1Enum ).complete() ).complete() ), ommEvent.getHandle() );
	}

	decode( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    " << "UpdateMsg" << endl << "Handle: " << ommEvent.getHandle() << " Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received:    " << "StatusMsg" << endl << "Item Handle: " << ommEvent.getHandle() << " Closure:     " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& event )
{
	cout << endl << "Received:    AckMsg" << endl << "Item Handle: " << event.getHandle() << endl << "Closure:     " << event.getClosure() << endl;
	
	decode( ackMsg );
}

void AppClient::decode( const AckMsg& ackMsg )
{
	if ( ackMsg.hasMsgKey() )
		cout << "Item Name: " << ( ackMsg.hasName() ? ackMsg.getName() : EmaString( "not set" ) )
		<< endl << "Service Name: " << ( ackMsg.hasServiceName() ? ackMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	cout << "Ack Id: " << ackMsg.getAckId() << endl;

	if ( ackMsg.hasNackCode() )
		cout << "Nack Code: " << ackMsg.getNackCodeAsString() << endl;

	if ( ackMsg.hasText() )
		cout << "Text: " << ackMsg.getText() << endl;

	switch ( ackMsg.getAttrib().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( ackMsg.getAttrib().getElementList() );
		break;
	case DataType::FieldListEnum:
		decode( ackMsg.getAttrib().getFieldList() );
		break;
	}

	switch ( ackMsg.getPayload().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( ackMsg.getPayload().getElementList() );
		break;
	case DataType::FieldListEnum:
		decode( ackMsg.getPayload().getFieldList() );
		break;
	}
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
		decode(msg.getPayload().getFieldList() );
		break;
	}
}

void AppClient::decode(const ElementList& el)
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

void AppClient::setOmmConsumer( OmmConsumer& consumer )
{
	_pOmmConsumer = &consumer;
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		client.setOmmConsumer( consumer );
		void* closure = (void*)1;

		// open login stream on which off stream posting will happen
		UInt64 loginHandle = consumer.registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure );
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( MMT_MARKET_PRICE ).serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
