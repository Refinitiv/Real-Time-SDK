///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
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
	//API QA
	cout << "\nevent session info (refresh)\n" << endl;		// defaults to ackMsg.toString()
	printSessionStatus(event);
	//END APIQA
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event ) 
{
	cout << "Handle " << event.getHandle() << endl
		<< "Parent Handle " << event.getParentHandle() << endl
		<< "Closure " << event.getClosure() << endl
		<< updateMsg << endl;		// defaults to updateMsg.toString()
	//API QA
	cout << "\nevent session info (update)\n" << endl;		// defaults to ackMsg.toString()
	printSessionStatus(event);
	//END APIQA
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
	}
	//API QA
	cout << "\nevent session info (status)\n" << endl;		// defaults to ackMsg.toString()
	printSessionStatus(event);
	//END APIQA
}
//APIQA
void AppClient::printSessionStatus(const refinitiv::ema::access::OmmConsumerEvent& event)
{
	EmaVector<ChannelInformation> statusVector;

	event.getSessionInformation(statusVector);

	// Print out the channel information.
	for (UInt32 i = 0; i < statusVector.size(); ++i)
	{
		cout << statusVector[i] << endl;
	}
}
//END APIQA

int main()
{ 
	try { 
		AppClient client;
		//API QA
		//OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );
	    OmmConsumer consumer(OmmConsumerConfig().consumerName("Consumer_10").username("user"), client);

		client.setOmmConsumer( consumer );

		CosAuthentication cosAuthentication;
		cosAuthentication.type( CosAuthentication::OmmLoginEnum );

		CosDataIntegrity cosDataIntegrity;
		cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );

		CosFlowControl cosFlowControl;
		cosFlowControl.type( CosFlowControl::BidirectionalEnum ).recvWindowSize( 1200 ).sendWindowSize( 1200 );

		CosGuarantee cosGuarantee;
		cosGuarantee.type( CosGuarantee::NoneEnum );

		ClassOfService cos;
		cos.authentication( cosAuthentication ).dataIntegrity( cosDataIntegrity ).flowControl( cosFlowControl ).guarantee( cosGuarantee );

		TunnelStreamRequest tsr;
		tsr.classOfService( cos ).domainType( MMT_SYSTEM ).name( "TUNNEL" ).serviceName( "DIRECT_FEED" );

		UInt64 tunnelStreamHandle = consumer.registerClient( tsr, client );

		client.setTunnelStreamHandle( tunnelStreamHandle );

		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
