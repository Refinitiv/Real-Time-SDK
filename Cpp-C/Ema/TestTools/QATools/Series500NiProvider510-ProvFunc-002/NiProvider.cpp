/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace std;

AppClient::AppClient() :
	_channelInfoVector()
{
}

AppClient::~AppClient()
{
	_channelInfoVector.clear();
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmProviderEvent& ommEvent )
{
	//API QA
	if (refreshMsg.getDomainType() == 1)
	{
		cout << refreshMsg.toString() << endl << "event session info (refresh)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << refreshMsg.toString() << endl << "event channel info (refresh)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmProviderEvent& ommEvent )
{
	//API QA
	if (updateMsg.getDomainType() == 1)
	{
		cout << updateMsg.toString() << endl << "event session info (update)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << updateMsg.toString() << endl << "event channel info (update)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmProviderEvent& ommEvent )
{
	//API QA
	if (statusMsg.getDomainType() == 1)
	{
		cout << statusMsg.toString() << endl << "event session info (status)" << endl;
		printSessionInfo(ommEvent);
	}
	else
		cout << statusMsg.toString() << endl << "event channel info (status)\n" << ommEvent.getChannelInformation() << endl;
}

void AppClient::printSessionInfo( const OmmProviderEvent& ommEvent )
{
	ommEvent.getSessionInformation(_channelInfoVector);
	
	for(UInt32 i = 0; i < _channelInfoVector.size(); ++i)
	{
		cout << _channelInfoVector[i].toString() << endl;
	}
}


int main()
{
	try
	{
		AppClient appClient;
		
		OmmProvider provider( OmmNiProviderConfig("EmaConfig.xml").providerName("Provider_Session").username( "userinvalid" ),appClient );
		UInt64 ibmHandle = 5;
		UInt64 triHandle = 6;
		RefreshMsg refresh;
		UpdateMsg update;
		FieldList fieldList;

		provider.submit( refresh.serviceName( "TEST_NI_PUB" ).name( "IBM.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList
				.addReal( 22, 14400, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 14700, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 9, OmmReal::Exponent0Enum )
				.addReal( 31, 19, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), ibmHandle );

		provider.submit( refresh.clear().serviceName( "TEST_NI_PUB" ).name( "TRI.N" )
			.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
			.payload( fieldList.clear()
				.addReal( 22, 4100, OmmReal::ExponentNeg2Enum )
				.addReal( 25, 4200, OmmReal::ExponentNeg2Enum )
				.addReal( 30, 20, OmmReal::Exponent0Enum )
				.addReal( 31, 40, OmmReal::Exponent0Enum )
				.complete() )
			.complete(), triHandle );

		for ( Int32 i = 0; i < 60; i++ )
		{
			sleep( 1000 );
			try
			{
				provider.submit(update.clear().serviceName("TEST_NI_PUB").name("IBM.N")
					.payload(fieldList.clear()
						.addReal(22, 14400 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 10 + i, OmmReal::Exponent0Enum)
						.complete()), ibmHandle);
				provider.submit(update.clear().serviceName("TEST_NI_PUB").name("TRI.N")
					.payload(fieldList.clear()
						.addReal(22, 4100 + i, OmmReal::ExponentNeg2Enum)
						.addReal(30, 21 + i, OmmReal::Exponent0Enum)
						.complete()), triHandle);
			}
			catch( OmmInvalidUsageException& excp )
			{
				// There is no active channel currently associated with this session, so just continue until the channels are back up.
				if ( excp.getErrorCode() == OmmInvalidUsageException::NoActiveChannelEnum )
				{
					continue;
				}
				else
				{
					cout << excp << endl;
					break;
				}
			}
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
