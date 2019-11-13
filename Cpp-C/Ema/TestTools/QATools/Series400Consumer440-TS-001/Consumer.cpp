///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.
//APIQA: add arguments -m 1-6 which set CosDataIntegrity, CosGuarantee 
//and  CosFlowControl different value , also -m 5 set recvWindowSize( 20 ) 
#include "Consumer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

AppClient::AppClient() :
 _pOmmConsumer( 0 ),
 _tunnelStreamHandle( 0 ),
 _bSubItemOpen( false )
{
}

AppClient::~AppClient()
{
}

void AppClient::setOmmConsumer( OmmConsumer& ommConsumer )
{
	_pOmmConsumer = &ommConsumer;
}

void AppClient::setTunnelStreamHandle( UInt64 handle )
{
	_tunnelStreamHandle = handle;
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event ) 
{
	cout << "Handle " << event.getHandle() << endl
		<< "Parent Handle " << event.getParentHandle() << endl
		<< "Closure " << event.getClosure() << endl
		<< refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event ) 
{
	cout << "Handle " << event.getHandle() << endl
		<< "Parent Handle " << event.getParentHandle() << endl
		<< "Closure " << event.getClosure() << endl
		<< updateMsg << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event ) 
{
	cout << "Handle " << event.getHandle() << endl
		<< "Parent Handle " << event.getParentHandle() << endl
		<< "Closure " << event.getClosure() << endl
		<< statusMsg << endl;		// defaults to statusMsg.toString()

	if ( !_bSubItemOpen &&
		event.getHandle() == _tunnelStreamHandle &&
		statusMsg.hasState() &&
		statusMsg.getState().getStreamState() == OmmState::OpenEnum )
	{
		_bSubItemOpen = true;
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_IBM" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_TRI" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_A" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_B" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_C" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_D" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
	}
}

int main( int argc, char* argv[] )
{
  if(argc != 2 && argc != 3)
    {
        cout<<"Error: Invalid number of arguments"<<endl;
        cout<<"Usage: "<<argv[0]<<" -m 1-6(testcase 1 or 2 or 3 or 4 ...) "<<endl;
        return -1;
    }
 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );

		client.setOmmConsumer( consumer );

		CosAuthentication cosAuthentication;
		cosAuthentication.type( CosAuthentication::OmmLoginEnum );
          
		int temp = 0;

		CosDataIntegrity cosDataIntegrity;
		CosFlowControl cosFlowControl;
		CosGuarantee cosGuarantee;
		    temp = atoi(argv[2]);
		if(strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "-M") == 0)
		{
			cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );
			cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 120 ).sendWindowSize( 1200 );
			cosGuarantee.type( CosGuarantee::NoneEnum );
			if(temp == 1 )
			{
				cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );
				cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 120 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::NoneEnum );
			}
			else if(temp == 2 )
			{
				cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );
				cosFlowControl.type( CosFlowControl::NoneEnum).recvWindowSize( 1200 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::NoneEnum );
			}
			else if(temp == 3 )
			{
				cosDataIntegrity.type( CosDataIntegrity::BestEffortEnum);
				cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 120 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::NoneEnum );
			}
			else if(temp == 4 )
			{
				cosDataIntegrity.type( CosDataIntegrity::BestEffortEnum);
				cosFlowControl.type( CosFlowControl::NoneEnum).recvWindowSize( 1200 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::NoneEnum );
			}
			else if(temp == 5 )
			{
				cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );
				cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 20 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::NoneEnum );
			}
			else if(temp == 6 )
			{
				cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );
				cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 120 ).sendWindowSize( 1200 );
				cosGuarantee.type( CosGuarantee::PersistentQueueEnum);
			}
			else
				return -1;
		}
		ClassOfService cos;
		cos.authentication( cosAuthentication ).dataIntegrity( cosDataIntegrity ).flowControl( cosFlowControl ).guarantee( cosGuarantee );
		TunnelStreamRequest tsr;
		tsr.classOfService( cos ).domainType( MMT_SYSTEM ).name( "TUNNEL" ).serviceName( "DIRECT_FEED" );
		
		tsr.loginReqMsg( ReqMsg().domainType( MMT_LOGIN ).name( "apiqa" ).attrib(
			ElementList().addUInt( "SingleOpen", 1 ).addUInt( "AllowSuspectData", 1 ).addAscii( "ApplicationName", "EMA" ).complete() ) );

		UInt64 tunnelStreamHandle = consumer.registerClient( tsr, client );

		client.setTunnelStreamHandle( tunnelStreamHandle );
		if( temp == 1 )
		{
			sleep( 10000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
			consumer.unregister(tunnelStreamHandle);	
		}

		sleep( 6000000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
//END APIQA
