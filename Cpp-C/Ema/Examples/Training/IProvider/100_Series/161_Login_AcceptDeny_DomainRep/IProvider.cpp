///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	Login::LoginReq loginRequest(reqMsg);

	if (loginRequest.getName() != "user")
	{
		Login::LoginStatus loginStatus = Login::LoginStatus();

		if (loginRequest.hasNameType())
			loginStatus.nameType(loginRequest.getNameType());

		event.getProvider().submit(loginStatus.name(loginRequest.getName()).
			state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Login denied").getMessage(),
			event.getHandle() );
	}
	else
	{
		Login::LoginRefresh loginRefresh = Login::LoginRefresh();

		if (loginRequest.hasAllowSuspectData())
			loginRefresh.allowSuspectData(loginRequest.getAllowSuspectData());

		if (loginRequest.hasSingleOpen())
			loginRefresh.singleOpen(loginRequest.getSingleOpen());

		if (loginRequest.hasPosition())
			loginRefresh.position(loginRequest.getPosition());

		if (loginRequest.hasApplicationId())
			loginRefresh.applicationId(loginRequest.getApplicationId());

		if (loginRequest.hasNameType())
			loginRefresh.nameType(loginRequest.getNameType());

		event.getProvider().submit( loginRefresh.name( loginRequest.getName() ).solicited(true).
			state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ).getMessage(),
			event.getHandle() );
	}
}

void AppClient::processInvalidDomainRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	Login::LoginReq loginRequest(reqMsg);

	Login::LoginStatus loginStatus = Login::LoginStatus();

	if (loginRequest.hasNameType())
		loginStatus.nameType(loginRequest.getNameType());

	event.getProvider().submit(loginStatus.name(loginRequest.getName()).
		state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Invalid domain").getMessage(),
		event.getHandle() );
}

void AppClient::onReqMsg( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	switch ( reqMsg.getDomainType() )
	{
	case MMT_LOGIN :
		processLoginRequest( reqMsg, event );
		break;
	default :
		processInvalidDomainRequest( reqMsg, event );
		break;
	}
}

int main()
{
	try
	{
		AppClient appClient;

		OmmProvider provider( OmmIProviderConfig().port( "14002" ), appClient );

		sleep( 60000 );
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}

	return 0;
}
