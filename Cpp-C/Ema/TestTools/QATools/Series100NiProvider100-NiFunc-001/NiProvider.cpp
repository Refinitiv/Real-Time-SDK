///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "NiProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
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

int main( int argc, char* argv[] )
{
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmNiProviderConfig().operationModel( OmmNiProviderConfig::UserDispatchEnum ).username( "user" ) );
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		UInt64 loginHandle = provider.registerClient( ReqMsg().domainType( MMT_LOGIN ).name( "user" ), appClient, (void*) 1 );

		provider.dispatch( 1000000 );		// calls to onRefreshMsg(), or onStatusMsg() execute on this thread

		provider.submit( refresh.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 32, 20000, OmmReal::InfinityEnum )
				.addTime( 14317, 8,30,40,999,999,999 )
				.addDateTime( 14318, 1992,2,22,8,30,40,999,999,999 )
				.addDateTime( 14319, 1992,2,22,8,30,40,65535,2047,2047 )
				.addDateTime( 14321, 1992,2,22,23,59,60,65535,2047,2047 )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		provider.dispatch( 1000000 );

		for ( Int32 i = 0; i < 35; )
		{
			if ( appClient.isConnectionUp() )
			{
				provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
					.payload( fieldList.clear()
						.addReal( 22, 4100 + i, OmmReal::ExponentNeg2Enum )
						.addReal( 30, 21 + i, OmmReal::Exponent0Enum )
				         .addTime( 14317, 8,30,40,999,999,999 )
				         .addDateTime( 14318, 1992,2,22,8,30,40,999,999,999 )
				         .addDateTime( 14319, 1992,2,22,8,30,40,65535,2047,2047 )
				        .addReal( 32, 20000, OmmReal::InfinityEnum )
						.complete() ), triHandle );

				i++;
			}

			provider.dispatch( 1000000 );
		}
		for ( Int32 i = 0; i < 10; )
		{
			if ( appClient.isConnectionUp() )
			{
				provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
					.payload( fieldList.clear()
						.addReal( 22, 4100 + i, OmmReal::ExponentNeg2Enum )
						.addReal( 30, 21 + i, OmmReal::Exponent0Enum )
				        .addReal( 32, 20000, OmmReal::NegInfinityEnum )
				         .addDateTime( 14319, 0,0,0,255,255,255,65535,2047,2047 )
				         .addTime( 14317, 255,255,255,65535,2047,2047 )
						.complete() ), triHandle );

				i++;
			}

			provider.dispatch( 1000000 );
		}
		for ( Int32 i = 0; i < 10; )
		{
			if ( appClient.isConnectionUp() )
			{
				provider.submit( update.clear().serviceName( "NI_PUB" ).name( "IBM.N" )
					.payload( fieldList.clear()
						.addReal( 22, 4100 + i, OmmReal::ExponentNeg2Enum )
						.addReal( 30, 21 + i, OmmReal::Exponent0Enum )
				        .addReal( 32, 20000, OmmReal::NotANumberEnum )
				        .addDateTime( 14321, 4095,12,31,23,59,60,999,2047,2047 )
						.complete() ), triHandle );

				i++;
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
//END APIQA
