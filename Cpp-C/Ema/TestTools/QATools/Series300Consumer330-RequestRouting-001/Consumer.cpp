/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg );
	//API QA
	cout << "\nevent session info (refresh)\n" << endl;		// defaults to refreshMsg.toString()
	printSessionStatus(ommEvent);
	//END APIQA
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	decode( updateMsg );
	//API QA
	cout << "\nevent session info (update)\n" << endl;		// defaults to updateMsg.toString()
	printSessionStatus(ommEvent);
	//END APIQA
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
	//API QA
	cout << "\nevent session info (status)\n" << endl;		// defaults to statusMsg.toString()
	printSessionStatus(ommEvent);
	//END APIQA
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
			case DataType::VectorEnum:
				decode( ee.getVector() );
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

void AppClient::decode( const Vector& vt )
{
	switch ( vt.getSummaryData().getDataType() )
	{
	case DataType::ElementListEnum:
		decode( vt.getSummaryData().getElementList() );
		break;
	default:
		cout << endl;
		break;
	}

	while ( vt.forth() )
	{
		const VectorEntry& ve = vt.getEntry();

		cout << "Position: " << ve.getPosition() << " Action: " << ve.getAction() << " DataType: " << DataType( ve.getLoad().getDataType() ) << " Value: ";

		switch ( ve.getLoadType() )
		{
		case DataType::ElementListEnum:
			decode( ve.getElementList() );
			break;
		default:
			cout << endl;
			break;
		}
	}
}
//APIQA
void AppClient::printSessionStatus(const refinitiv::ema::access::OmmConsumerEvent& event)
{
	EmaVector<ChannelInformation> statusVector;

	event.getSessionInformation(statusVector);

	// Print out the channel information.
	for (UInt32 i = 0; i < statusVector.size(); ++i)
	{
		cout << statusVector[i] << endl;
	}
}
//END APIQA
int main()
{
	try {
		AppClient client;
		//APIQA
		//OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).username( "user" ), client);
		OmmConsumer consumer(OmmConsumerConfig().operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"), client);
		//END APIQA
		void* closure = (void*)1;
		
		UInt64 itemHandle = consumer.registerClient(ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );
		
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );			// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
