///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA: Alter Consumer440 to accept -bufSize and -fillSize as inputs.  

#include "Consumer.h"
//APIQA
#include <stdlib.h>
// END APIQA
using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

AppClient::AppClient() :
 _pOmmConsumer( 0 ),
 _tunnelStreamHandle( 0 ),
 _bSubItemOpen( false )
 // APIQA: subitem handle
 ,_subItemHandle( 0 )
 // END APIQA:
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
	// APIQA:  save subitem handle
    _subItemHandle = event.getHandle();
    // END APIQA
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

		// APIQA: register subitem on SYSTEM domain
		_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_IBM" ).serviceId( 1 ).domainType( MMT_SYSTEM ), *this, (void*)1, _tunnelStreamHandle );
		// END APIQA:
		//_pOmmConsumer->registerClient( ReqMsg().name( "TUNNEL_IBM" ).serviceId( 1 ), *this, (void*)1, _tunnelStreamHandle );
	}
}

// APIIQA: GenericMsg handler
void AppClient::onGenericMsg( const GenericMsg& genMsg, const OmmConsumerEvent& event )
{
	cout << "Handle " << event.getHandle() << endl
		<< "Parent Handle " << event.getParentHandle() << endl
		<< "Closure " << event.getClosure() << endl
		<< genMsg << endl;		// defaults to genMsg.toString()
}
// END APIQA:

int main( int argc, char* argv[] )
{ 
	try { 
		// APIQA: buffer size, fill size and runtime
		int _tunnelBufSize = 0;
		int _tunnelFillSize = 0;
		int _runTime = 60;
		// get command line arguments
		int idx = 1;
		int count = argc - 1;
		while ( idx < count )
		{
			if ( 0 == strcmp(argv[idx], "-bufSize") )
			{
				if ( ++idx <= count )
				{
					_tunnelBufSize = atoi(argv[idx]);
				}
				++idx;
			}
			else if ( 0 == strcmp(argv[idx], "-fillSize") )
			{
				if ( ++idx <= count )
				{
					_tunnelFillSize = atoi(argv[idx]);
				}
				++idx;
			}
			else if ( 0 == strcmp(argv[idx], "-runtime") )
			{
				if ( ++idx <= count )
				{
					_runTime = atoi(argv[idx]);
				}
				++idx;
			}
			else
			{
				cout << "Unrecognized command line option.\n" << endl;
			}
		}
		// END APIQA: 
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

	    // APIQA:  Create a buffer of _tunnelBufSize and fill buffer up to _tunnelFillSize with values 0 to 255 repeatedly
		if (_tunnelBufSize > 614400)
		{
			_tunnelBufSize = 614400;
		}
		if (_tunnelFillSize == 0 || _tunnelFillSize > _tunnelBufSize)
		{
			_tunnelFillSize = _tunnelBufSize;
		}
		else if (_tunnelFillSize < 10)
		{
			_tunnelFillSize = 10;
		}
		for (int i = 0; i < _runTime; i++)
		{
			if (client._subItemHandle != 0)
			{
				char *charBuffer = new char[_tunnelBufSize]();
				for (int j = 0, b = 0; j < _tunnelFillSize - 10; j++)
				{
					if (b == 256)
					{
						b = 0;
					}
					charBuffer[j] = b++;
				}
				EmaBuffer buffer = EmaBuffer(charBuffer, _tunnelFillSize - 10);
				OmmOpaque opaque;
				opaque.set(buffer);
				// send buffer nested in a generic message
				try
				{
					consumer.submit(GenericMsg().domainType(MMT_SYSTEM).payload(opaque), client._subItemHandle);
				}
				catch ( const OmmException& excp )
				{
					cout << "consumer.submit() GenericMsg Exception: " << excp << endl;
				}
				delete charBuffer;
			}
			sleep(1000);
		}
	    // END APIQA:
		//sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}

	return 0;
}
//END APIQA
