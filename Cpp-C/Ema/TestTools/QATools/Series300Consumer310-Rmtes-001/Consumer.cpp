///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Refresh. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Update. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	cout << endl << "Received Status. Item Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );
	
	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	// In the below loop partial updates for the specific field of RMTES type are handled.
	// Note that in case it is necessary to handle partial updates for multiple fields,
	// the application has to cache each RMTES string in a separate RmtesBuffer
	// (e.g., use a hashmap to track RmtesBuffer instances corresponding to specific FIDs)
	// and apply the updates accordingly.
	// API QA
	DataDictionary dictionary;
	dictionary.loadFieldDictionary("RDMFieldDictionary");
	dictionary.loadEnumTypeDictionary("enumtype.def");
	EmaBuffer rmtesAsEmaBuffer;
	FieldList fieldList;
	while ( fl.forth( "HEADLINE1" ) )
	{
		const FieldEntry& fe = fl.getEntry();
		
		cout << "Fid: " << fe.getFieldId() << " Name: " << fe.getName() << " DataType: " << DataType(fe.getLoadType()).toString() << " Value: " << endl;

		if (fe.getCode() == Data::BlankEnum)
			cout << " blank" << endl;
		else
		{							
			rmtesBuffer.apply(fe.getRmtes());
			cout << rmtesBuffer.toString() << endl;

			std::string utf8Str = rmtesBuffer.getAsUTF8().c_buf();
			cout << "HEADLINE1 as utf8Str : " << endl;
			cout << utf8Str << endl;
			std::string rawHexString = rmtesBuffer.getAsUTF8().asRawHexString();
			cout << "HEADLINE1 as rawHexString : " << endl;
			cout << rawHexString << endl;

			//new function RTSDK-8145
			rmtesAsEmaBuffer = rmtesBuffer.getAsEmaBuffer();

			cout << "HEADLINE1 as rmtesBuffer.getAsEmaBuffer() : " << endl;
			cout << rmtesAsEmaBuffer.asRawHexString() << endl;
		
			fieldList.addRmtes(4272, rmtesAsEmaBuffer);
			fieldList.complete();
			cout << "Encoding fieldList with rmtesAsEmaBuffer: ";
			cout << fieldList.toString(dictionary) << endl;
		}
	}
	//END API QA
}

AppClient::AppClient() :
	rmtesBuffer()
{
}

int main()
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().operationModel( OmmConsumerConfig::UserDispatchEnum ).username( "user" ) );
		void* closure = (void*)1;
		UInt64 handle = consumer.registerClient( ReqMsg().serviceName( "ELEKTRON_DD" ).name( "NFCP_UBMS" ), client, closure ); 
		unsigned long long startTime = getCurrentTime();
		while ( startTime + 120000 > getCurrentTime() )
			consumer.dispatch( 10 );		// calls to onRefreshMsg(), onUpdateMsg(), or onStatusMsg() execute on this thread
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
