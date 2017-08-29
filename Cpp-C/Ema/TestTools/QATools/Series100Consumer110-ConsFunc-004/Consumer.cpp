///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace std;


void AppClient::decodeFieldList( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		if ( fe.getLoadType() != DataType::RmtesEnum )
			cout << "fid name = " << fe.getName() << "\tfid = " << fe.getFieldId() << "\tvalue = " << fe.getLoad().toString() << endl;
		else
		{
			Int64 fid = fe.getFieldId();

			cout << "fid name = " << fe.getName() << "\tfid = " << fid << "\tvalue = ";

			for ( UInt64 idx = 0; idx < 1000; ++idx )
			{
				if ( _array[idx]._fid != 0 && _array[idx]._fid == fid )
				{
					_array[idx]._buffer.apply( fe.getRmtes() );
					cout << _array[idx]._buffer.toString() << endl;
					break;
				}
				else if ( _array[idx]._fid == 0 )
				{
					_array[idx]._buffer.apply( fe.getRmtes() );
					_array[idx]._fid = fid;
					_array[idx]._fidName = fe.getName();
					cout << _array[idx]._buffer.toString() << endl;
					break;
				}
			}
		}
	}
}

void AppClient::printPage()
{
	cout << endl;

	for ( UInt64 idx = 0; idx < 1000; ++idx )
	{
		if ( _array[idx]._fid == 0 ) break;

		cout << _array[idx]._fidName << " \t" << _array[idx]._fid << "\t" << _array[idx]._buffer.toString() << endl;
	}
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	if ( refreshMsg.hasName() )
		cout << endl << "Item Name: " << refreshMsg.getName();
	
	if ( refreshMsg.hasServiceName() )
		cout << endl << "Service Name: " << refreshMsg.getServiceName();

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decodeFieldList( refreshMsg.getPayload().getFieldList() );

	printPage();
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	if ( updateMsg.hasName() )
		cout << endl << "Item Name: " << updateMsg.getName();
	
	if ( updateMsg.hasServiceName() )
		cout << endl << "Service Name: " << updateMsg.getServiceName();

	cout << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decodeFieldList( updateMsg.getPayload().getFieldList() );

	printPage();
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	if ( statusMsg.hasName() )
		cout << endl << "Item Name: " << statusMsg.getName();
	
	if ( statusMsg.hasServiceName() )
		cout << endl << "Service Name: " << statusMsg.getServiceName();

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString();

	cout << endl;
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "SPOT" ), client );
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "TIME" ), client );
		sleep( 60000000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
