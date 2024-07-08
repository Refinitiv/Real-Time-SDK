///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( refreshMsg.hasMsgKey() )
		cout << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << ( refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	cout << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( updateMsg.hasMsgKey() )
		cout << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << ( updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
	if ( updateMsg.getDomainType() == MMT_MARKET_PRICE )
	{
		if ( ++_updateNumber == 10 )
		{
			cout << endl << "Got 10 Updates" << endl;
		}
	}
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	if ( statusMsg.hasMsgKey() )
		cout << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << ( statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "not set" ) ) << endl;

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
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

AppClient::AppClient() :
 _updateNumber( 0 )
{
}

void AppClient::setOmmConsumer( OmmConsumer& consumer, UInt64 loginHandle )
{
	_pOmmConsumer = &consumer;
	_loginHandle = loginHandle;
}

int main( int argc, char* argv[] )
{
	try {
		AppClient client;
		OmmConsumerConfig cc;
		
		cc.addAdminMsg( ReqMsg().domainType( MMT_LOGIN).name( "apiqa" ).nameType(3).attrib(
		              ElementList().addUInt( "SingleOpen", 1 ).addUInt( "AllowSuspectData", 1 ).addUInt( "ProvidePermissionExpressions", 0 ).addUInt( "ProvidePermissionProfile", 0 ).addUInt( "DownloadConnectionConfig", 1 ).addAscii( "InstanceId", "2" ).addAscii( "ApplicationId", "256" ).addAscii( "ApplicationName", "APIQA's test application" ).addAscii( "Password", "secrete" ).addAscii( "Position", "99.22.4.11/net" ).addUInt( "Role", 0 ).addUInt( "SupportProviderDictionaryDownload", 0 ).complete() ) );
        OmmConsumer * consumer = new OmmConsumer(cc.host("localhost:14002"));
        void* closure = (void*)1;
		void* closure1 = (void*)1;
		UInt64 handle = consumer->registerClient(ReqMsg().domainType( MMT_LOGIN ),client,closure);
        UInt64 handle1 = consumer->registerClient(ReqMsg().serviceName("DIRECT_FEED").name("CSCO.O"), client,  closure1);
        sleep(100);
        cout << endl << "Login Reissue with different name from apiqa to apiqa2" << endl;
        consumer->reissue(ReqMsg().initialImage(true).name("apiqa2").domainType( MMT_LOGIN ).attrib(ElementList().addUInt( "SingleOpen", 1 ).addUInt( "AllowSuspectData", 1 ).addUInt( "ProvidePermissionExpressions", 0 ).addUInt( "ProvidePermissionProfile", 0 ).addUInt( "DownloadConnectionConfig", 1 ).addAscii( "InstanceId", "2" ).addAscii( "ApplicationId", "256" ).addAscii( "ApplicationName", "APIQA's test application" ).addAscii( "Password", "secrete" ).addAscii( "Position", "99.22.4.11/net" ).addUInt( "Role", 0 ).addUInt( "SupportProviderDictionaryDownload", 0 ).complete()).serviceName("DIRECT_FEED").nameType(3), handle );
		cout << endl << "Login Reissue done with nametype apiqa2" << endl;
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
