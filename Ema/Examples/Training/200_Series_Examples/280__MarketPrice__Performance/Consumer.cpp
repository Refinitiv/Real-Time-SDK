///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace std;

UInt64 updateCount = 0;
UInt64 refreshCount = 0;
UInt64 statusCount = 0;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	++refreshCount;

	const Payload& payload = refreshMsg.getPayload();
	if ( payload.getDataType() == DataType::FieldListEnum )
		decode( payload.getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	++updateCount;

	const Payload& payload = updateMsg.getPayload();
	if ( payload.getDataType() == DataType::FieldListEnum )
		decode( payload.getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& , const OmmConsumerEvent& )
{
	++statusCount;
}

void AppClient::decode( const FieldList& fl )
{
	try
	{
	while ( !fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		if ( fe.getCode() == Data::NoCodeEnum )
			switch ( fe.getLoadType() )
			{
			case DataType::RealEnum :
				{
					const OmmReal& re = fe.getReal();
				}
				break;
			case DataType::DateEnum :
				{
					const OmmDate& date = fe.getDate();
				}
				break;
			case DataType::TimeEnum :
				{
					const OmmTime& time = fe.getTime();
				}
				break;
			case DataType::DateTimeEnum :
				{
					const OmmDateTime& dateTime = fe.getDateTime();
				}
				break;
			case DataType::IntEnum :
				{
					Int64 value = fe.getInt();
				}
				break;
			case DataType::UIntEnum :
				{
					UInt64 value = fe.getUInt();
				}
				break;
			case DataType::AsciiEnum :
				{
					const EmaString& asciiString = fe.getAscii();
				}
				break;
			case DataType::RmtesEnum :
				{
					const RmtesBuffer& rmtesBuffer = fe.getRmtes();
				}
				break;
			case DataType::Utf8Enum :
				{
					const EmaBuffer& uthf8Buffer = fe.getUtf8();
				}
				break;
			case DataType::EnumEnum :
				{
					UInt16 value = fe.getEnum();
				}
				break;
			case DataType::ErrorEnum :
				{
					const OmmError& error = fe.getError();
				}
				break;
			default :
				break;
			}
	}
	}catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
}

int main( int argc, char* argv[] )
{
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().host( "localhost:14002" ).username( "user" ) );
		ReqMsg reqMsg;
		EmaString itemName;
		for ( UInt32 idx = 0; idx < 1000; ++idx )
		{
			itemName.set( "RTR", 3 ).append( idx ).append( ".N" );
			reqMsg.serviceName( "DIRECT_FEED" ).name( itemName );
			consumer.registerClient( reqMsg, client );
			reqMsg.clear();
		}
		for ( UInt32 idx = 0; idx < 300; ++idx )
		{
			sleep( 1000 );
			cout << "total refresh count: " << refreshCount << "\ttotal status count: " << statusCount << "\tupdate rate (per sec): " << updateCount << endl;
			updateCount = 0;
		}
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
