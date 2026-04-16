/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}

int main()
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().consumerName( "Consumer_2" ) );		// uses configuration from EmaConfig.xml
		//API QA
		cout << "---Test Set Values via FileConfig---" << endl;
		cout << "GlobalConfig::getComplexTypePoolLimit(): " << GlobalConfig::getComplexTypePoolLimit() << endl;
		cout << "GlobalConfig::getMsgTypePoolLimit(): " << GlobalConfig::getMsgTypePoolLimit() << endl;
		cout << "GlobalConfig::getDataTypePoolLimit(): " << GlobalConfig::getDataTypePoolLimit() << endl;
		cout << "GlobalConfig::getAckMsgInPoolCount(): " << GlobalConfig::getAckMsgInPoolCount() << endl;
		cout << "GlobalConfig::getGenericMsgInPoolCount(): " << GlobalConfig::getGenericMsgInPoolCount() << endl;
		cout << "GlobalConfig::getPostMsgInPoolCount(): " << GlobalConfig::getPostMsgInPoolCount() << endl;
		cout << "GlobalConfig::getReqMsgInPoolCount(): " << GlobalConfig::getReqMsgInPoolCount() << endl;
		cout << "GlobalConfig::getRefreshMsgInPoolCount(): " << GlobalConfig::getRefreshMsgInPoolCount() << endl;
		cout << "GlobalConfig::getStatusMsgInPoolCount(): " << GlobalConfig::getStatusMsgInPoolCount() << endl;
		cout << "GlobalConfig::getUpdateMsgInPoolCount(): " << GlobalConfig::getUpdateMsgInPoolCount() << endl;
		GlobalConfig::setComplexTypePoolLimit(15000);
		GlobalConfig::setMsgTypePoolLimit(15000);
		GlobalConfig::setDataTypePoolLimit(15000);
		cout << "---Test Set Values via GlobalConfig::set function---" << endl;
		cout << "GlobalConfig::getComplexTypePoolLimit(): " << GlobalConfig::getComplexTypePoolLimit() << endl;
		cout << "GlobalConfig::getMsgTypePoolLimit(): " << GlobalConfig::getMsgTypePoolLimit() << endl;
		cout << "GlobalConfig::getDataTypePoolLimit(): " << GlobalConfig::getDataTypePoolLimit() << endl;
		cout << "GlobalConfig::getAckMsgInPoolCount(): " << GlobalConfig::getAckMsgInPoolCount() << endl;
		cout << "GlobalConfig::getGenericMsgInPoolCount(): " << GlobalConfig::getGenericMsgInPoolCount() << endl;
		cout << "GlobalConfig::getPostMsgInPoolCount(): " << GlobalConfig::getPostMsgInPoolCount() << endl;
		cout << "GlobalConfig::getReqMsgInPoolCount(): " << GlobalConfig::getReqMsgInPoolCount() << endl;
		cout << "GlobalConfig::getRefreshMsgInPoolCount(): " << GlobalConfig::getRefreshMsgInPoolCount() << endl;
		cout << "GlobalConfig::getStatusMsgInPoolCount(): " << GlobalConfig::getStatusMsgInPoolCount() << endl;
		cout << "GlobalConfig::getUpdateMsgInPoolCount(): " << GlobalConfig::getUpdateMsgInPoolCount() << endl;
		
		//END API QA
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "IBM.N" ) , client );
		sleep( 60000 );			// API calls onRefreshMsg(), onUpdateMsg() and onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
