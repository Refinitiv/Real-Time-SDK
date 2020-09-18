///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace rtsdk::ema::access;
using namespace rtsdk::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Refresh on Domain " << refreshMsg.getDomainType() << ". Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	decode( refreshMsg );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Update on Domain " << updateMsg.getDomainType() << ". Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	decode( updateMsg );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Status on Domain " << statusMsg.getDomainType() << ". Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout <<  "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << "Item State: " << statusMsg.getState().toString() << endl;
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
	cout << endl;
}

void AppClient::decode( const ElementList& el )
{
	cout << endl;
	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();

		cout << "\tName: " << ee.getName() << " DataType: " << DataType( ee.getLoad().getDataType() ) << " Value: ";

		if ( ee.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
		{
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
				decode(ee.getArray());
				break;
			default:
				cout << endl;
				break;
			}
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
		case DataType::IntEnum :
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getInt() << endl;
			break;
		case DataType::UIntEnum:
			cout << "Action = " << me.getMapActionAsString() << ", key = " << me.getKey().getUInt() << endl;
			break;
		default:
			break;
		}

		switch ( me.getLoadType() )
		{
		case DataType::FilterListEnum:
			decode( me.getFilterList() );
			break;
		case DataType::ElementListEnum:
			cout << endl;
			decode( me.getElementList() );
			break;
		default:
			break;
		}
	}
	cout << endl;
}

void AppClient::decode( const FilterList& fl )
{
	while ( fl.forth() )
	{
		const FilterEntry& fe = fl.getEntry();

		cout << "ID: " << (int) fe.getFilterId() << " Action = " << fe.getFilterActionAsString() <<  " DataType: " << DataType( fe.getLoad().getDataType() ) << " Value: ";
		switch ( fe.getLoadType() )
		{
		case DataType::ElementListEnum:
			decode( fe.getElementList() );
			break;
		case DataType::MapEnum:
			decode( fe.getMap() );
			break;
		default:
			break;
		}
	}
	//cout << endl;
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
				//cout << endl;
				break;
		}
	}
}

void AppClient::decode(const OmmArray& ommArray)
{
	bool first = false;
	while (ommArray.forth())
	{
		const OmmArrayEntry& ae = ommArray.getEntry();

		//cout << "DataType: " << DataType(ae.getLoad().getDataType()) << " Value: ";

		if ( first == false )
			first = true;
		else
			cout << ", ";
		if (ae.getCode() == Data::BlankEnum)
			cout << " blank" << endl;
		else
			switch (ae.getLoadType())
			{
			case DataType::RealEnum:
				cout << ae.getReal().getAsDouble();
				break;
			case DataType::DateEnum:
				cout << (UInt64)ae.getDate().getDay() << " / " << (UInt64)ae.getDate().getMonth() << " / " << (UInt64)ae.getDate().getYear();
				break;
			case DataType::TimeEnum:
				cout << (UInt64)ae.getTime().getHour() << ":" << (UInt64)ae.getTime().getMinute() << ":" << (UInt64)ae.getTime().getSecond() << ":" << (UInt64)ae.getTime().getMillisecond();
				break;
			case DataType::IntEnum:
				cout << ae.getInt();
				break;
			case DataType::UIntEnum:
				cout << ae.getUInt();
				break;
			case DataType::AsciiEnum:
				cout << ae.getAscii();
				break;
			case DataType::ErrorEnum:
				cout << ae.getError().getErrorCode() << "( " << ae.getError().getErrorCodeAsString() << " )";
				break;
			case DataType::EnumEnum:
				cout << ae.getEnum();
				break;
			default:
				break;
			}
	}
	cout << endl;
}

void printUsage()
{
	cout <<"\nOptions:\n"
		<< "  -?\tShows this usage\n\n"
		<< "  -f <souree directory filter in decimal; default = no filter is specified>\n" 
		<< "     Possible values for filter, valid range = 0-63:\n" 
		<< "     0 :  No Filter \n" 
		<< "     1 :  SERVICE_INFO_FILTER 0x01 \n" 
		<< "     2 :  SERVICE_STATE_FILTER 0x02 \n"
		<< "     4 :  SERVICE_GROUP_FILTER 0x04 \n"
		<< "     8 :  SERVICE_LOAD_FILTER 0x08 \n"
		<< "    16 :  SERVICE_DATA_FILTER 0x10 \n"
		<< "    32 :  SERVICE_LINK_FILTER 0x20 \n"
		<< "    ?? :  Mix of above values upto 63 \n\n"
		<< "  -m <option>; default = option 0\n"
		<< "     Possible values for option, valid range = 0-4:\n" 
		<< "     0 :  Request source directory without serviceName or serviceId\n"
		<< "     1 :  Request source directory with serviceName\n"
		<< "     2 :  Request source directory with serviceName; Request item on that service\n"
		<< "     3 :  Request source directory with serviceId\n"
		<< "     4 :  Request source directory with serviceId; Request item on that service\n\n"
		<< "  -s <amount of time to wait before requesting an item in seconds; default = no wait>\n"
		<< "     This option only applies to -m 2 or -m 4\n"
		<< "\n";
	exit(-1);
}

int main(int argc, char* argv[])
{
	try {
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		void* closure = (void*)1;
		int _FILTER = -1;
		int _OPTION = 0;
		int _SLEEPTIME = 0;
		int iargs = 1;
			
		// read command line arguments
		while (iargs < argc)
		{
			if (0 == strcmp("-?", argv[iargs]))
				printUsage();
			else if (strcmp("-f", argv[iargs]) == 0)
			{
				++iargs; if (iargs == argc) printUsage();
				_FILTER =  atoi(argv[iargs++]);
			}
			else if (strcmp("-m", argv[iargs]) == 0)
			{
				++iargs; if (iargs == argc) printUsage();
				_OPTION = atoi(argv[iargs++]);
			}
			else if (strcmp("-s", argv[iargs]) == 0)
			{
				++iargs; if (iargs == argc) printUsage();
				_SLEEPTIME = atoi(argv[iargs++]);
			}
			else
			{
				cout << "Invalid argument: " << argv[iargs] << endl;
				printUsage();
			}
		}
		switch ( _OPTION )
		{
		default:
		case 0: //  do directory request with filter if specified 
		case 5: //  do directory request with filter if specified; do a item request after a sleep if specified 
			if ( _FILTER >= 0 )
			{
				cout << "********APIQA: Requesting directory without service name, service id, and filter=" << _FILTER << "\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).filter(_FILTER), client, closure );
			} else {
				cout << "********APIQA: Requesting directory without service name, service id\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ), client, closure );
			}
			break;
		case 1:   //  do directory request with filter if specified and servieName
		case 2:   //  do directory request with filter if specified and servieName; do item request after a sleep if specified
			if ( _FILTER >= 0 )
			{
				cout << "********APIQA: Requesting directory with service=DIRECT_FEED and filter=" << _FILTER << "\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).serviceName( "DIRECT_FEED" ).filter(_FILTER), client, closure );
			} else {
				cout << "********APIQA: Requesting directory with service=DIRECT_FEED\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).serviceName( "DIRECT_FEED" ), client, closure );
			}
			break;
		case 3:   //  do directory request with filter if specified and servieId
		case 4:   //  do directory request with filter if specified and servieId;  do item request after a sleep if specified
			if ( _FILTER >= 0 )
			{
				cout << "********APIQA: Requesting directory with service=SERVICE_ID and filter=" << _FILTER << "\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).serviceId( 8090 ).filter(_FILTER), client, closure );
			} else {
				cout << "********APIQA: Requesting directory with service=SERVICE_ID\n\n"; 
				UInt64 dirHandle = consumer.registerClient( ReqMsg().domainType( MMT_DIRECTORY ).serviceId( 8090 ), client, closure );
			}
			break;
		}
		if ( ( _OPTION == 2 ) || ( _OPTION == 4 ) || ( _OPTION == 5 ) )
		{
			if (_SLEEPTIME > 0 ) 
			{
				cout << "********APIQA: Sleeping (in seconds): " << _SLEEPTIME << "\n";
				sleep( _SLEEPTIME * 1000 );    // API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			}
			cout << "********APIQA: Requesting item\n"; 
			if ( ( _OPTION == 2 ) || ( _OPTION == 5 ) )
				UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ), client, closure );
			else
				UInt64 itemHandle = consumer.registerClient( ReqMsg().serviceId( 8090 ).name( "IBM.N" ), client, closure );
		}
		
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
