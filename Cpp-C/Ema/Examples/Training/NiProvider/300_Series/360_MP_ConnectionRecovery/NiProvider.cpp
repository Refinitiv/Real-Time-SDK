/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

AppClient::AppClient() :
	_bConnectionUp( false )
{
}

AppClient::~AppClient()
{
}

bool AppClient::isConnectionUp() const
{
	return _bConnectionUp;
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << refreshMsg << endl;

	if ( refreshMsg.getState().getStreamState() == OmmState::OpenEnum )
	{
		if ( refreshMsg.getState().getDataState() == OmmState::OkEnum )
			_bConnectionUp = true;
		else
			_bConnectionUp = false;
	}
	else
		_bConnectionUp = false;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
	cout << endl << "Handle: " << ommEvent.getHandle() << " Closure: " << ommEvent.getClosure() << endl;
	cout << statusMsg << endl;

	if ( statusMsg.hasState() )
	{
		if ( statusMsg.getState().getStreamState() == OmmState::OpenEnum )
		{
			if ( statusMsg.getState().getDataState() == OmmState::OkEnum )
				_bConnectionUp = true;
			else
				_bConnectionUp = false;
		}
		else
			_bConnectionUp = false;
	}
	else
		_bConnectionUp = true;
}

int main()
{
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmNiProviderConfig().operationModel( OmmNiProviderConfig::UserDispatchEnum ).username( "user" ), appClient );
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;
		bool sendRefreshMsg = false;

		provider.dispatch( 1000000 );		// calls to onRefreshMsg(), or onStatusMsg() execute on this thread

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		provider.dispatch( 1000000 );

		for ( Int32 i = 0; i < 60; )
		{
			if ( appClient.isConnectionUp() )
			{
				if (sendRefreshMsg)
				{
					provider.submit(refresh.clear().serviceName("NI_PUB").name("TRI.N")
						.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
						.payload(fieldList.clear()
							.addReal(22, 4100, OmmReal::ExponentNeg2Enum)
							.addReal(25, 4200, OmmReal::ExponentNeg2Enum)
							.addReal(30, 20, OmmReal::Exponent0Enum)
							.addReal(31, 40, OmmReal::Exponent0Enum)
							.complete())
						.complete(), triHandle);

					sendRefreshMsg = false;
				}
				else
				{
					provider.submit(update.clear().serviceName("NI_PUB").name("TRI.N")
						.payload(fieldList.clear()
							.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
							.addReal(30, 21 + i, OmmReal::Exponent0Enum)
							.complete()), triHandle);

					i++;
				}
			}
			else
			{
				sendRefreshMsg = true;
			}

			provider.dispatch( 1000000 );
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
