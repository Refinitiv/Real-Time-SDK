/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderConfigImpl.h"

using namespace thomsonreuters::ema::access;

OmmNiProviderConfigImpl::OmmNiProviderConfigImpl() 
{
  _userNodeName = "NiProviderGroup|NiProviderList|NiProvider.";
  modifyLoginRequest( _loginRdmReqMsg );
}

OmmNiProviderConfigImpl::~OmmNiProviderConfigImpl() {
}

void OmmNiProviderConfigImpl::modifyLoginRequest( LoginRdmReqMsg& lrrm ) {
  lrrm._rsslRdmLoginRequest.role = RDM_LOGIN_ROLE_PROV;
  lrrm._rsslRdmLoginRequest.flags = RDM_LG_RQF_HAS_ROLE;
}

EmaString OmmNiProviderConfigImpl::getUserName() const {
	EmaString retVal;

	if ( _pEmaConfig->get< EmaString >( "NiProviderGroup|DefaultNiProvider", retVal ) )
	{
		EmaString expectedName( "NiProviderGroup|NiProviderList|NiProvider." );
		expectedName.append( retVal ).append( "|Name" );
		EmaString checkValue;
		if ( _pEmaConfig->get( expectedName, checkValue ) )
			return retVal;
		else
		{
			EmaString errorMsg( "default NiProvider name [" );
			errorMsg.append( retVal ).append( "] is an non-existent NiProvider name; DefaultNiProvider specification ignored" );
			_pEmaConfig->appendErrorMessage( errorMsg, OmmLoggerClient::ErrorEnum );
		}
	}

	if ( _pEmaConfig->get< EmaString >( "NiProviderGroup|NiProviderList|NiProvider|Name", retVal ) )
		return retVal;

	return "EmaNiProvider";

}
