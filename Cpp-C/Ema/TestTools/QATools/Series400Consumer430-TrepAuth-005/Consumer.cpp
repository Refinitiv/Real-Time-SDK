///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"
#include <stdlib.h>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace thomsonreuters::ema::domain::login;
using namespace std;

const UInt32 maxLength = 256;
EmaString authenticationToken;
EmaString newauthenticationToken;
EmaString authenticationExtended;
EmaString appId( "256" );

void printUsage()
{
	cout << "\nOptions:\n"
		<< "  -?                            Shows this usage\n\n"
		<< "  -at <authentication token>    Required Authentication token to use in login request as msgKey.name \n"
		<< "  -newat <second authentication token>    Required Authentication token to use in login request as msgKey.name \n"
		<< "  -aid <applicationId>          ApplicationId set as login Attribute [default = 256]\n"
		<< "  -ax <extended information>    Authentication extended information\n"
		<< "\n";
	exit( -1 );
}

void printActiveConfig()
{
	cout << "\nFollowing options are selected:" << endl;

	cout << "appId = " << appId << endl
		<< "authenticationToken = " << authenticationToken << endl
		<< "newauthenticationToken = " << newauthenticationToken << endl
		<< "authenticationextended = " << authenticationExtended << endl;
}

void processCommandLineOptions( int argc, char* argv[] )
{
	int iargs = 1;
	authenticationToken.clear();
	newauthenticationToken.clear();
	authenticationExtended.clear();

	while ( iargs < argc )
	{
		if ( 0 == strcmp( "-?", argv[iargs] ) )
		{
			printUsage();
		}
		else if ( strcmp( "-at", argv[iargs] ) == 0 )
		{
			++iargs; if ( iargs == argc ) printUsage();
			authenticationToken.set(argv[iargs++]);
		}
		else if ( strcmp( "-newat", argv[iargs] ) == 0 )
		{
			++iargs; if ( iargs == argc ) printUsage();
			newauthenticationToken.set(argv[iargs++]);
		}
		else if ( strcmp( "-aid", argv[iargs] ) == 0 )
		{
			++iargs; if ( iargs == argc ) printUsage();
			appId.set(argv[iargs++]);
		}
		else if (strcmp("-ax", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();
			authenticationExtended.set(argv[iargs++]);
		}
		else
		{
			cout << "Invalid argument: " << argv[iargs] << endl;
			printUsage();
		}

		if (authenticationToken.empty())
		{
			cout << "Missing Authentication Token." << endl;
			printUsage();
		}
	}
}

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	cout << endl << "Received refresh" << endl;

	cout << endl << refreshMsg << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	cout << endl << "Received update" << endl;
	cout << endl << updateMsg << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	cout << endl << "Received status" << endl;

	cout << endl << statusMsg << endl;
}

AppLoginClient::AppLoginClient()
{
	_handle = 0;
	_TTReissue = 0;
}

void AppLoginClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
{
	Login::LoginRefresh tempRefresh;
	cout << endl << "Received login refresh message" << endl;
	
	_handle = event.getHandle();

	cout << endl << refreshMsg << endl;

	tempRefresh.message(refreshMsg);
	if (tempRefresh.hasAuthenticationTTReissue())
		_TTReissue = tempRefresh.getAuthenticationTTReissue();
	else
		_TTReissue = 0;
}

void AppLoginClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
{
	cout << endl << updateMsg << endl;
}

void AppLoginClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
{
	if (statusMsg.getDomainType() == MMT_LOGIN)
		cout << endl << "Received login status message" << endl;

	cout << endl << statusMsg << endl;
}

int main( int argc, char* argv[] )
{
	try {
		authenticationToken.clear();
		newauthenticationToken.clear();
		authenticationExtended.clear();
		processCommandLineOptions( argc, argv );
		printActiveConfig();
		AppLoginClient loginClient;

		EmaBuffer authnExtendedBuf;

		Login::LoginReq loginMsg;

		OmmConsumerConfig consumerConfig;

		loginMsg.name(authenticationToken).applicationId(appId).nameType(USER_AUTH_TOKEN);
		if (!authenticationExtended.empty())
		{
			authnExtendedBuf.setFrom(authenticationExtended.c_str(), authenticationExtended.length());
			loginMsg.authenticationExtended(authnExtendedBuf);
		}
			
		
		consumerConfig.addAdminMsg(loginMsg.getMessage());

		AppClient appClient;
		OmmConsumer consumer( consumerConfig, loginClient);
		
		
		consumer.registerClient( ReqMsg().serviceName( "DIRECT_FEED" ).name( "TRI.N" ), appClient );
		
		for (int i = 0; i < 60; i++)
		{
			
			if (loginClient._TTReissue != 0 && getCurrentTime() >= loginClient._TTReissue)
			{
				loginMsg.clear().name(authenticationToken).applicationId(appId).nameType(USER_AUTH_TOKEN);
				if (!authenticationExtended.empty())
				{
					loginMsg.authenticationExtended(authnExtendedBuf);
				}
				consumer.reissue(loginMsg.getMessage(), loginClient._handle);
				loginClient._TTReissue = 0;
			}
			
			sleep(1000);
			//APIQA
			if ( i == 3 )
			{
				cout << endl << "SEND PAUSE USING first TOKEN" << endl;
					loginMsg.clear().name(authenticationToken).applicationId(appId).nameType(USER_AUTH_TOKEN).pause(true);
				consumer.reissue(loginMsg.getMessage(), loginClient._handle);
			}
			if ( i == 10 )
			{
				cout << endl << "SEND RESUME USING second TOKEN which is not valid" << endl;
					loginMsg.clear().name(newauthenticationToken).applicationId(appId).nameType(USER_AUTH_TOKEN).pause(false);
				consumer.reissue(loginMsg.getMessage(), loginClient._handle);
			}
		}
		//END APIQA
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

//END APIQA

