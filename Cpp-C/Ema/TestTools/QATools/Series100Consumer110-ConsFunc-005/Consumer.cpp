/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
    if ( refreshMsg.hasMsgKey() )
        cout << endl << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << refreshMsg.getServiceName();

    cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

    if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
          decode( refreshMsg.getPayload().getFieldList() );
	if ( DataType::ElementListEnum == refreshMsg.getPayload().getDataType() )
          decodeEList( refreshMsg.getPayload().getElementList() );
	if ( DataType::MapEnum == refreshMsg.getPayload().getDataType() )
          decodeM( refreshMsg.getPayload().getMap() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
    if ( updateMsg.hasMsgKey() )
	        cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;
	         if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
                        decode( updateMsg.getPayload().getFieldList() );
                if ( DataType::ElementListEnum == updateMsg.getPayload().getDataType() )
                        decodeEList( updateMsg.getPayload().getElementList() );
                if (  DataType::MapEnum == updateMsg.getPayload().getDataType() )
                        decodeM( updateMsg.getPayload().getMap() );

}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
    if ( statusMsg.hasMsgKey() )
        cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

    if ( statusMsg.hasState() )
        cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}


void AppClient::decodeM( const Map& map )
{
     if (  map.getSummaryData().getDataType() == DataType::FieldListEnum )
     {
		cout << "Map Summary data:" << endl;
		decode( map.getSummaryData().getFieldList() );
	}
		
	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

		if ( me.getLoadType() == DataType::BufferEnum )
			//cout << "Action: " << MapAction( me.getAction() ).toString() << " MAP KEY " <<  me.getKey() << " getKeyBuffer() is  " <<   me.getKeyBuffer() << endl;
			cout << "Action: " <<  me.getAction() <<  endl;

		if ( me.getLoadType() == DataType::FieldListEnum )
		{
			cout << "Entry data:" << endl;
			decode( me.getFieldList() );
		}
	}
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Fid: " << fe.getFieldId() << " Name = " << fe.getName() << " DataType: " << fe.getLoad().getDataType()  << " Value: ";

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
			case DataType::ErrorEnum :
			  cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum :
				cout << fe.getEnum() << endl;
				break;
			default :
				cout << endl;
				break;
			}
	}
}


void AppClient::decodeEList( const ElementList& el )
{
	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();
	     cout << "ElementEntry Data  " << ee.getAscii() << endl;
	}
}




int main( int argc, char* argv[] )
{ 
  try {
		AppClient client;
		OmmConsumerConfig cc;
		void* closure1 = (void*)1;
		void* closure2 = (void*)1;
		int temp = -1; 
				
                cc.consumerName( "Consumer_2" );
				cout << "setup for Consumer_2" << endl;
				OmmConsumer * consumer = new OmmConsumer(cc);
                ReqMsg reqmsg;
				UInt64 h1 = consumer->registerClient(reqmsg.serviceId( 999 ).name("TRI.N"), client,  closure1);
				UInt64 h2 = consumer->registerClient(reqmsg.name("IBM.N"), client, closure1);
		sleep( 360000 );				// API calls processRespMSg() with received messages
	} catch ( const OmmException& excp  ) {
		cout << excp << endl;
	}
	return 0;
}

//END APIQA
