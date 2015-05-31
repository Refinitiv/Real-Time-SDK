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

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	if ( refreshMsg.hasMsgKey() )
		cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::MapEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getMap() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	if ( updateMsg.hasMsgKey() )
		cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

	if ( DataType::MapEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getMap() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const Map& map )
{
	if ( map.getSummary().getDataType() == DataType::FieldListEnum )
	{
		cout << "Map Summary data:" << endl;
		decode( map.getSummary().getFieldList() );
	}

	while ( !map.forth() )
	{
		if ( map.getEntry().getKey().getDataType() != DataType::BufferEnum )
			return;

		const EmaBuffer& key = map.getEntry().getKey().getBuffer();

		DataType::DataTypeEnum dType = map.getEntry().getLoadType();

		switch ( map.getEntry().getAction() )		// MapEntry processing based on MapEntry action
		{
			case MapEntry::AddEnum :
			{
				if ( dType != DataType::FieldListEnum )
					return;
				decode( map.getEntry().getFieldList() );
			}
			break;
			case MapEntry::DeleteEnum :
			{
				if ( dType != DataType::NoDataEnum )
					return;
				break;
			}
			case MapEntry::UpdateEnum :
			{
				if ( dType != DataType::FieldListEnum )
					return;
				decode( map.getEntry().getFieldList() );
			}
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
			case DataType::EnumEnum :
				cout << fe.getEnum() << endl;
				break;
			case DataType::ErrorEnum :
				cout << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
			default :
				cout << endl;
				break;
			}
	}
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).host( "localhost:14002" ).username( "user" ) );
		consumer.registerClient( ReqMsg().domainType( MMT_MARKET_BY_ORDER ).serviceName( "DIRECT_FEED" ).name( "AAO.V" ) , client );
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 60000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
