///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "IProvider.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void AppClient::processLoginRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	if ( reqMsg.getName() != "user" )
	{
		event.getProvider().submit(StatusMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).
			state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Login denied" ),
			event.getHandle() );
	}
	else
	{
		RefreshMsg loginRefresh;
		ElementList refreshAtributes;

		if ( reqMsg.getAttrib().getDataType() == DataType::ElementListEnum )
		{
			bool setRefreshAttrib = false;
			const ElementList& reqAttributes = reqMsg.getAttrib().getElementList();

			while ( reqAttributes.forth() )
			{
				const ElementEntry& reqAttrib = reqAttributes.getEntry();
				const EmaString& name = reqAttrib.getName();

				if ( name == ENAME_ALLOW_SUSPECT_DATA ||
					name == ENAME_SINGLE_OPEN )
				{
					setRefreshAttrib = true;
					refreshAtributes.addUInt( name, reqAttrib.getUInt() );
				}
				else if ( name == ENAME_APP_ID  ||
					name == ENAME_POSITION )
				{
					setRefreshAttrib = true;
					refreshAtributes.addAscii( name, reqAttrib.getAscii() );
				}
			}

			if ( setRefreshAttrib ) loginRefresh.attrib( refreshAtributes.complete() );
		}

		event.getProvider().submit(loginRefresh.domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
			solicited( true ).state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted" ),
			event.getHandle() );
	}
}

void AppClient::processInvalidDomainRequest( const ReqMsg& reqMsg, const OmmProviderEvent& event )
{
	event.getProvider().submit( StatusMsg().name( reqMsg.getName() ).serviceName( reqMsg.getServiceName() ).
		domainType( reqMsg.getDomainType() ).
		state( OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Invalid domain" ),
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

int main( int argc, char* argv[] )
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
