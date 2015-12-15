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
	}
}

int main( int argc, char* argv[] )
{ 
	try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ) );

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
