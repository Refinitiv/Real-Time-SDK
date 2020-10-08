///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << endl << "onRefreshMsg() callback. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeRefreshMsg( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << endl << "onUpdateMsg() callback. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeUpdateMsg( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << endl << "onStatusMsg() callback. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	decodeStatusMsg( statusMsg );
}

void AppClient::decodeRefreshMsg( const RefreshMsg& refreshMsg )
{
	cout << endl << "RefreshMsg: "
		<< endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) )
		<< endl << "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) )
		<< endl << "Item State: " << refreshMsg.getState().toString();

	cout << endl << "Attribute: ";
	decode( refreshMsg.getAttrib().getData() );

	cout << endl << "Payload: ";
	decode( refreshMsg.getPayload().getData() );
}

void AppClient::decodeGenericMsg( const GenericMsg& genMsg )
{
	cout << endl << "GenericMsg: "
		<< endl << "Item Name: " << ( genMsg.hasName() ? genMsg.getName() : EmaString( "<not set>" ) );

	cout << endl << "Attribute: ";
	decode( genMsg.getAttrib().getData() );

	cout << endl << "Payload: ";
	decode( genMsg.getPayload().getData() );
}

void AppClient::decodeUpdateMsg( const UpdateMsg& updateMsg )
{
	cout << endl << "UpdateMsg: "
		<< endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) )
		<< endl << "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Attribute: ";
	decode( updateMsg.getAttrib().getData() );

	cout << endl << "Payload: ";
	decode( updateMsg.getPayload().getData() );
}

void AppClient::decodeStatusMsg( const StatusMsg& statusMsg )
{
	cout << endl << "StatusMsg: "
		<< endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) )
		<< endl << "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString();
}

void AppClient::decode( const Data& data )
{
	if ( data.getCode() == Data::BlankEnum )
		cout << "Blank data";
	else
		switch ( data.getDataType() )
		{
		case DataType::RefreshMsgEnum :
			decodeRefreshMsg( static_cast<const RefreshMsg&>( data ) );
			break;
		case DataType::GenericMsgEnum :
			decodeGenericMsg( static_cast<const GenericMsg&>( data ) );
			break;
		case DataType::UpdateMsgEnum :
			decodeUpdateMsg( static_cast<const UpdateMsg&>( data ) );
			break;
		case DataType::StatusMsgEnum :
			decodeStatusMsg( static_cast<const StatusMsg&>( data ) );
			break;
		case DataType::FieldListEnum :
			decodeFieldList( static_cast<const FieldList&>( data ) );
			break;
		case DataType::MapEnum :
			decodeMap( static_cast<const Map&>( data ) );
			break;
		case DataType::NoDataEnum :
			cout << "NoData";
			break;
		case DataType::TimeEnum :
			cout << "OmmTime: " << static_cast<const OmmTime&>( data ).toString();
			break;
		case DataType::DateEnum :
			cout << "OmmDate: " << static_cast<const OmmDate&>( data ).toString();
			break;
		case DataType::RealEnum :
			cout << "OmmReal::getAsDouble: " << static_cast<const OmmReal&>( data ).getAsDouble();
			break;
		case DataType::IntEnum :
			cout << "OmmInt: " << static_cast<const OmmInt&>( data ).getInt();
			break;
		case DataType::UIntEnum :
			cout << "OmmUInt: " << static_cast<const OmmUInt&>( data ).getUInt();
			break;
		case DataType::EnumEnum :
			cout << "OmmEnum: " << static_cast<const OmmEnum&>( data ).getEnum();
			break;
		case DataType::AsciiEnum :
			cout << "OmmAscii: " << static_cast<const OmmAscii&>( data ).getAscii();
			break;
		case DataType::BufferEnum :
			cout << "OmmBuffer: " << static_cast<const OmmBuffer&>( data ).getBuffer().operator const char *();
			break;
		case DataType::RmtesEnum :
			cout << "OmmRmtes: " << static_cast<const OmmRmtes&>( data ).getRmtes().toString();
			break;
		case DataType::ErrorEnum :
			cout << "Decoding error: " << static_cast<const OmmError&>( data ).getErrorCodeAsString();
			break;
		default :
			break;
		}
}

void AppClient::decodeMap( const Map& map )
{
	cout << endl << "Map: ";
	cout << endl << "Summary: ";
	decode( map.getSummaryData().getData() );

	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		cout << endl << endl << "Entry action: " << me.getMapActionAsString();

		cout << endl << "Entry key: ";
		decode( me.getKey().getData() );

		cout << endl << "Entry load: ";
		decode( me.getLoad() );
	}
}

void AppClient::decodeFieldList( const FieldList& fl )
{
	cout << endl << "FieldList: ";
	if ( fl.hasInfo() )
		cout << endl << "FieldListNum: " << fl.getInfoFieldListNum() << " DictionaryId: " << fl.getInfoDictionaryId();

	while ( fl.forth() )
	{
		cout << endl << "Entry fid#: " << fl.getEntry().getFieldId();
		cout << endl << "Entry load: ";
		decode( fl.getEntry().getLoad() );
	}
}

int main()
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).host( "localhost:14002" ).username( "user" ) );
		void* closure = (void*)1;
		// request a custom domain (133) item IBM.XYZ
		UInt64 handle = consumer.registerClient( ReqMsg().domainType( 133 ).serviceName( "DIRECT_FEED" ).name( "IBM.XYZ" ), client, closure );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
