/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "LoginCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "OmmBaseImpl.h"
#include "StaticDecoder.h"
#include "OmmState.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "PostMsgEncoder.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

const EmaString LoginCallbackClient::_clientName( "LoginCallbackClient" );
const EmaString LoginItem::_clientName( "LoginItem" );
const EmaString NiProviderLoginItem::_clientName( "NiProviderLoginItem" );

Login* Login::create( OmmBaseImpl& ommBaseImpl )
{
	Login* pLogin = 0;

	try
	{
		pLogin = new Login();
	}
	catch ( std::bad_alloc ) {}

	if ( !pLogin )
		ommBaseImpl.handleMee( "Failed to create Login." );

	return pLogin;
}

void Login::destroy( Login*& pLogin )
{
	if ( pLogin )
	{
		delete pLogin;
		pLogin = 0;
	}
}

Login::Login() :
	_username(),
	_password(),
	_position(),
	_applicationId(),
	_applicationName(),
	_instanceId(),
	_authenticationErrorText(),
	_authenticationExtended(),
	_authenticationExtendedResp(),
	_toString(),
	_pChannel( 0 ),
	_supportBatchRequest( 0 ),
	_supportEnhancedSymbolList( 0 ),
	_supportPost( 0 ),
	_singleOpen( 1 ),
	_allowSuspect( 1 ),
	_pauseResume( 0 ),
	_permissionExpressions( 1 ),
	_permissionProfile( 1 ),
	_supportViewRequest( 0 ),
	_role( 0 ),
	_authenticationErrorCode( 0 ),
	_authenticationTTReissue( 0 ),
	_userNameType( 1 ),
	_streamState( RSSL_STREAM_UNSPECIFIED ),
	_dataState( RSSL_DATA_NO_CHANGE ),
	_stateCode( RSSL_SC_NONE ),
	_toStringSet( false ),
	_usernameSet( false ),
	_passwordSet( false ),
	_positionSet( false ),
	_applicationIdSet( false ),
	_applicationNameSet( false ),
	_instanceIdSet( false ),
	_stateSet( false ),
	_authenticationExtendedSet( false ),
	_authenticationExtendedRespSet( false ),
	_authenticationErrorCodeSet( false ),
	_authenticationErrorTextSet( false ),
	_authenticationTTReissueSet( false )
{
}

Login::~Login()
{
}

Channel* Login::getChannel() const
{
	return _pChannel;
}

Login& Login::setChannel( Channel* pChannel )
{
	_toStringSet = false;
	_pChannel = pChannel;
	return *this;
}

const EmaString& Login::toString()
{
	if ( !_toStringSet )
	{
		_toStringSet = true;
		_toString.set("username ").append(_usernameSet ? _username : "<not set>").append(CR)
			.append("usernameType ").append(_userNameType).append(CR)
			.append("position ").append(_positionSet ? _position : "<not set>").append(CR)
			.append("appId ").append(_applicationIdSet ? _applicationId : "<not set>").append(CR)
			.append("applicationName ").append(_applicationNameSet ? _applicationName : "<not set>").append(CR)
			.append("instanceId ").append(_instanceIdSet ? _instanceId : "<not set>").append(CR)
			.append("singleOpen ").append(_singleOpen).append(CR)
			.append("allowSuspect ").append(_allowSuspect).append(CR)
			.append("optimizedPauseResume ").append(_pauseResume).append(CR)
			.append("permissionExpressions ").append(_permissionExpressions).append(CR)
			.append("permissionProfile ").append(_permissionProfile).append(CR)
			.append("supportBatchRequest ").append(_supportBatchRequest).append(CR)
			.append("supportEnhancedSymbolList ").append(_supportEnhancedSymbolList).append(CR)
			.append("supportPost ").append(_supportPost).append(CR)
			.append("supportViewRequest ").append(_supportViewRequest).append(CR)
			.append("role ").append(_role);

		if (_authenticationTTReissueSet)
			_toString.append(CR).append("authenticationTTReissue ").append(_authenticationTTReissue);
		if (_authenticationErrorCodeSet)
			_toString.append(CR).append("authenticationErrorCode ").append(_authenticationErrorCode);
		if (_authenticationErrorTextSet)
			_toString.append(CR).append("authenticationErrorText ").append(_authenticationErrorText);

	}

	return _toString;
}

Login& Login::set( RsslRDMLoginRefresh* pRefresh )
{
	_toStringSet = false;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA )
		_allowSuspect = pRefresh->allowSuspectData;
	else
		_allowSuspect = 1;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR )
		_permissionExpressions = pRefresh->providePermissionExpressions;
	else
		_permissionExpressions = 1;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE )
		_permissionProfile = pRefresh->providePermissionProfile;
	else
		_permissionProfile = 1;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH )
		_supportBatchRequest = pRefresh->supportBatchRequests;
	else
		_supportBatchRequest = 0;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE )
		_pauseResume = pRefresh->supportOptimizedPauseResume;
	else
		_pauseResume = 0;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SINGLE_OPEN )
		_singleOpen = pRefresh->singleOpen;
	else
		_singleOpen = 1;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_POST )
		_supportPost = pRefresh->supportOMMPost;
	else
		_supportPost = 0;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_ENH_SL )
		_supportEnhancedSymbolList = pRefresh->supportEnhancedSymbolList;
	else
		_supportEnhancedSymbolList = 0;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW )
		_supportViewRequest = pRefresh->supportViewRequests;
	else
		_supportViewRequest = 0;

	if ( pRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE )
		_userNameType = pRefresh->userNameType;
	else
		_userNameType = 1;

	if ( ( pRefresh->flags & RDM_LG_RFF_HAS_USERNAME ) && pRefresh->userName.length )
	{
		_username.set( pRefresh->userName.data, pRefresh->userName.length );
		_usernameSet = true;
	}
	else
		_usernameSet = false;

	if ( ( pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID ) && pRefresh->applicationId.length )
	{
		_applicationId.set( pRefresh->applicationId.data, pRefresh->applicationId.length );
		_applicationIdSet = true;
	}
	else
		_applicationIdSet = false;

	if ( ( pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_NAME ) && pRefresh->applicationName.length )
	{
		_applicationName.set( pRefresh->applicationName.data, pRefresh->applicationName.length );
		_applicationNameSet = true;
	}
	else
		_applicationNameSet = false;

	_instanceIdSet = false;

	if ( ( pRefresh->flags & RDM_LG_RFF_HAS_POSITION ) && pRefresh->position.length )
	{
		_position.set( pRefresh->position.data, pRefresh->position.length );
		_positionSet = true;
	}
	else
		_positionSet = false;

	if (pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE)
	{
		_authenticationErrorCode = pRefresh->authenticationErrorCode;
		_authenticationErrorCodeSet = true;
	}
	else
		_authenticationErrorCodeSet = false;

	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT) && pRefresh->authenticationErrorText.length)
	{
		_authenticationErrorText.set(pRefresh->authenticationErrorText.data, pRefresh->authenticationErrorText.length);
		_authenticationErrorTextSet = true;
	}
	else
		_authenticationErrorTextSet = false;

	if (pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE)
	{
		_authenticationTTReissue = pRefresh->authenticationTTReissue;
		_authenticationTTReissueSet = true;
	}
	else
		_authenticationTTReissueSet = false;

	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP) && pRefresh->authenticationExtendedResp.length)
	{
		_authenticationExtendedResp.setFrom(pRefresh->authenticationExtendedResp.data, pRefresh->authenticationExtendedResp.length);
		_authenticationExtendedRespSet = true;
	}
	else
		_authenticationExtendedRespSet = false;

	_passwordSet = false;

	_stateSet = true;
	_streamState = pRefresh->state.streamState;
	_dataState = pRefresh->state.dataState;
	_stateCode = pRefresh->state.code;

	return *this;
}

Login& Login::set( RsslRDMLoginRequest* pRequest )
{
	_toStringSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_ALLOW_SUSPECT_DATA )
		_allowSuspect = pRequest->allowSuspectData;
	else
		_allowSuspect = 1;

	if ( pRequest->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_EXPR )
		_permissionExpressions = pRequest->providePermissionExpressions;
	else
		_permissionExpressions = 1;

	if ( pRequest->flags & RDM_LG_RQF_HAS_PROVIDE_PERM_PROFILE )
		_permissionProfile = pRequest->providePermissionProfile;
	else
		_permissionProfile = 1;

	_supportBatchRequest = 0;

	_pauseResume = 0;

	if ( pRequest->flags & RDM_LG_RQF_HAS_SINGLE_OPEN )
		_singleOpen = pRequest->singleOpen;
	else
		_singleOpen = 1;

	_supportPost = 0;

	_supportEnhancedSymbolList = 0;

	_supportViewRequest = 0;

	if ( pRequest->flags & RDM_LG_RQF_HAS_ROLE )
		_role = pRequest->role;
	else
		_role = 0;

	if ( pRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE )
		_userNameType = pRequest->userNameType;
	else
		_userNameType = 1;

	_username.set( pRequest->userName.data, pRequest->userName.length );
	_usernameSet = true;

	if ( pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_ID && pRequest->applicationId.length )
	{
		_applicationId.set( pRequest->applicationId.data, pRequest->applicationId.length );
		_applicationIdSet = true;
	}
	else
		_applicationIdSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_NAME && pRequest->applicationName.length)
	{
		_applicationName.set( pRequest->applicationName.data, pRequest->applicationName.length );
		_applicationNameSet = true;
	}
	else
		_applicationNameSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_INSTANCE_ID && pRequest->instanceId.length)
	{
		_instanceId.set( pRequest->instanceId.data, pRequest->instanceId.length );
		_instanceIdSet = true;
	}
	else
		_instanceIdSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_POSITION && pRequest->position.length)
	{
		_position.set( pRequest->position.data, pRequest->position.length );
		_positionSet = true;
	}
	else
		_positionSet = false;

	if (pRequest->flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED && pRequest->authenticationExtended.length)
	{
		_authenticationExtended.setFrom(pRequest->authenticationExtended.data, pRequest->authenticationExtended.length);
		_authenticationExtendedSet = true;
	}
	else
		_authenticationExtendedSet = false;

	_passwordSet = false;

	_stateSet = false;

	return *this;
}

bool Login::populate( RsslRefreshMsg& refresh, RsslBuffer& buffer )
{
	rsslClearRefreshMsg( &refresh );

	RsslElementList rsslEL;
	rsslClearElementList( &rsslEL );

	rsslEL.flags = RSSL_ELF_HAS_STANDARD_DATA;

	RsslElementEntry rsslEE;
	rsslClearElementEntry( &rsslEE );

	RsslEncodeIterator eIter;
	rsslClearEncodeIterator( &eIter );

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &eIter, _pChannel->getRsslChannel()->majorVersion, _pChannel->getRsslChannel()->minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	retCode = rsslSetEncodeIteratorBuffer( &eIter, &buffer );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	retCode = rsslEncodeElementListInit( &eIter, &rsslEL, 0, 0 );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SINGLE_OPEN;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_singleOpen );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_ALLOW_SUSPECT_DATA;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_allowSuspect );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_PROV_PERM_EXP;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_permissionExpressions );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_PROV_PERM_PROF;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_permissionProfile );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SUPPORT_BATCH;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_supportBatchRequest );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SUPPORT_OPR;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_pauseResume );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SUPPORT_POST;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_supportPost );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SUPPORT_ENH_SL;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_supportEnhancedSymbolList );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	rsslEE.dataType = RSSL_DT_UINT;
	rsslEE.name = RSSL_ENAME_SUPPORT_VIEW;
	retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &_supportViewRequest );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	RsslBuffer text;
	if ( _applicationIdSet )
	{
		rsslEE.dataType = RSSL_DT_ASCII_STRING;
		rsslEE.name = RSSL_ENAME_APPID;
		text.data = ( char* )_applicationId.c_str();
		text.length = _applicationId.length();
		retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &text );
		if ( retCode != RSSL_RET_SUCCESS )
			return false;
	}

	if ( _applicationNameSet )
	{
		rsslEE.dataType = RSSL_DT_ASCII_STRING;
		rsslEE.name = RSSL_ENAME_APPNAME;
		text.data = ( char* )_applicationName.c_str();
		text.length = _applicationName.length();
		retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &text );
		if ( retCode != RSSL_RET_SUCCESS )
			return false;
	}

	if ( _positionSet )
	{
		rsslEE.dataType = RSSL_DT_ASCII_STRING;
		rsslEE.name = RSSL_ENAME_POSITION;
		text.data = ( char* )_position.c_str();
		text.length = _position.length();
		retCode = rsslEncodeElementEntry( &eIter, &rsslEE, &text );
		if ( retCode != RSSL_RET_SUCCESS )
			return false;
	}

	if (_authenticationErrorCodeSet)
	{
		rsslEE.dataType = RSSL_DT_UINT;
		rsslEE.name = RSSL_ENAME_AUTHN_ERROR_CODE;
		retCode = rsslEncodeElementEntry(&eIter, &rsslEE, &_authenticationErrorCode);
		if (retCode != RSSL_RET_SUCCESS)
			return false;
	}

	if (_authenticationErrorTextSet)
	{
		rsslEE.dataType = RSSL_DT_ASCII_STRING;
		rsslEE.name = RSSL_ENAME_AUTHN_ERROR_TEXT;
		text.data = (char*)_authenticationErrorText.c_str();
		text.length = _authenticationErrorText.length();
		retCode = rsslEncodeElementEntry(&eIter, &rsslEE, &text);
		if (retCode != RSSL_RET_SUCCESS)
			return false;
	}

	if (_authenticationTTReissueSet)
	{
		rsslEE.dataType = RSSL_DT_UINT;
		rsslEE.name = RSSL_ENAME_AUTHN_TT_REISSUE;
		retCode = rsslEncodeElementEntry(&eIter, &rsslEE, &_authenticationTTReissue);
		if (retCode != RSSL_RET_SUCCESS)
			return false;
	}

	if (_authenticationExtendedRespSet)
	{
		rsslEE.dataType = RSSL_DT_ASCII_STRING;
		rsslEE.name = RSSL_ENAME_AUTHN_EXTENDED_RESP;
		text.data = (char*)_authenticationExtendedResp.c_buf();
		text.length = _authenticationExtendedResp.length();
		retCode = rsslEncodeElementEntry(&eIter, &rsslEE, &text);
		if (retCode != RSSL_RET_SUCCESS)
			return false;
	}

	retCode = rsslEncodeElementListComplete( &eIter, RSSL_TRUE );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	buffer.length = rsslGetEncodedBufferLength( &eIter );

	refresh.msgBase.streamId = EMA_LOGIN_STREAM_ID;
	refresh.msgBase.domainType = RSSL_DMT_LOGIN;
	refresh.msgBase.containerType = RSSL_DT_NO_DATA;
	refresh.msgBase.encDataBody.data = 0;
	refresh.msgBase.encDataBody.length = 0;
	refresh.msgBase.msgKey.flags |= RSSL_MKF_HAS_ATTRIB | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
	refresh.msgBase.msgKey.nameType = _userNameType;
	refresh.msgBase.msgKey.name.data = ( char* )_username.c_str();
	refresh.msgBase.msgKey.name.length = _username.length();
	refresh.msgBase.msgKey.attribContainerType = RSSL_DT_ELEMENT_LIST;
	refresh.msgBase.msgKey.encAttrib = buffer;
	refresh.flags |= RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_SOLICITED;

	refresh.state.code = _stateCode;
	refresh.state.streamState = _streamState;
	refresh.state.dataState = _dataState;
	refresh.state.text.data = ( char* ) "Refresh Completed";
	refresh.state.text.length = 17;

	return true;
}

bool Login::populate( RsslStatusMsg& status, RsslBuffer& buffer )
{
	rsslClearStatusMsg( &status );

	status.msgBase.streamId = EMA_LOGIN_STREAM_ID;
	status.msgBase.domainType = RSSL_DMT_LOGIN;
	status.msgBase.containerType = RSSL_DT_NO_DATA;
	status.msgBase.encDataBody.data = 0;
	status.msgBase.encDataBody.length = 0;
	status.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
	status.msgBase.msgKey.nameType = _userNameType;
	status.msgBase.msgKey.name.data = ( char* )_username.c_str();
	status.msgBase.msgKey.name.length = _username.length();
	status.msgBase.msgKey.attribContainerType = RSSL_DT_NO_DATA;
	status.msgBase.msgKey.encAttrib.data = 0;
	status.msgBase.msgKey.encAttrib.length = 0;

	status.flags |= RSSL_STMF_CLEAR_CACHE | RSSL_STMF_HAS_MSG_KEY;

	return true;
}

void Login::sendLoginClose()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg( &rsslCloseMsg );

	rsslCloseMsg.msgBase.streamId = EMA_LOGIN_STREAM_ID;
	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = RSSL_DMT_LOGIN;

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = ( RsslMsg* )&rsslCloseMsg;

	submitMsgOpts.majorVersion = _pChannel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pChannel->getRsslChannel()->minorVersion;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret = rsslReactorSubmitMsg( _pChannel->getRsslReactor(),
	                                    _pChannel->getRsslChannel(),
	                                    &submitMsgOpts, &rsslErrorInfo );
}

LoginList::LoginList()
{
}

LoginList::~LoginList()
{
	Login* login = _list.pop_back();
	while ( login )
	{
		Login::destroy( login );
		login = _list.pop_back();
	}
}

void LoginList::addLogin( Login* pLogin )
{
	_list.push_back( pLogin );
}

void LoginList::removeLogin( Login* pLogin )
{
	_list.remove( pLogin );
}

void LoginList::removeLogin(RsslReactorChannel* pRsslChannel)
{
	if (pRsslChannel)
	{
		Login* prevLogin = _list.back();
		Login* login;

		while (prevLogin)
		{
			login = prevLogin;
			prevLogin = login->previous();

			if (login->getChannel()->getRsslChannel() == pRsslChannel)
			{
				_list.remove(login);
				Login::destroy(login);
			}
		}
	}
}

Login* LoginList::getLogin( Channel* pChannel )
{
	Login* login = _list.front();
	while ( login )
	{
		if ( login->getChannel() == pChannel )
			return login;
		login = login->next();
	}

	return 0;
}

UInt32 LoginList::size() const
{
	return _list.size();
}

UInt32 LoginList::sendLoginClose()
{
	UInt32 count = 0;

	Login* login = _list.front();
	while ( login )
	{
		login->sendLoginClose();
		++count;
		login = login->next();
	}

	return count;
}

Login* LoginList::operator[]( UInt32 idx ) const
{
	UInt32 index = 0;
	Login* login = _list.front();

	while ( login )
	{
		if ( index == idx )
		{
			return login;
		}
		else
		{
			index++;
			login = login->next();
		}
	}

	return 0;
}

LoginCallbackClient::LoginCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_loginItemLock(),
	_loginList(),
	_loginRequestMsg(),
	_loginRequestBuffer( 0 ),
	_loginRefreshMsg(),
	_loginRefreshBuffer( 0 ),
	_refreshMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommBaseImpl( ommBaseImpl ),
	_requestLogin( 0 ),
	_loginItems(),
	_loginFailureMsg()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created LoginCallbackClient" );
	}
}

LoginCallbackClient::~LoginCallbackClient()
{
	_loginItems.clear();

	Login::destroy( _requestLogin );

	if ( _loginRequestBuffer )
		free( _loginRequestBuffer );

	if (_loginRefreshBuffer)
		free(_loginRefreshBuffer);

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed LoginCallbackClient" );
	}
}

LoginCallbackClient* LoginCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	LoginCallbackClient* pClient = 0;

	try
	{
		pClient = new LoginCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
		ommBaseImpl.handleMee( "Failed to create LoginCallbackClient" );

	return pClient;
}

void LoginCallbackClient::destroy( LoginCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void LoginCallbackClient::initialize()
{
	rsslClearRDMLoginRequest( &_loginRequestMsg );
	rsslClearRDMLoginRefresh(&_loginRefreshMsg);
	_notifyChannelDownReconnecting = false;

	if ( !_ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->userName.length )
	{
		RsslBuffer tempBuffer;
		tempBuffer.data = _ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->defaultUsername;
		tempBuffer.length = 256;

		if ( RSSL_RET_SUCCESS != rsslGetUserName( &tempBuffer ) )
		{
			_ommBaseImpl.handleIue( "Failed to obtain name of the process owner",  OmmInvalidUsageException::FailureEnum );
			return;
		}

		_ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->userName = tempBuffer;
	}

	RsslBuffer tempLRB;

	tempLRB.length = 1000;
	tempLRB.data = ( char* ) malloc( tempLRB.length );
	if ( !tempLRB.data )
	{
		_ommBaseImpl.handleMee( "Failed to allocate memory for RsslRDMLoginRequest in LoginCallbackClient." );
		return;
	}

	_loginRequestBuffer = tempLRB.data;

	while ( RSSL_RET_BUFFER_TOO_SMALL == rsslCopyRDMLoginRequest( &_loginRequestMsg, _ommBaseImpl.getActiveConfig().pRsslRDMLoginReq, &tempLRB ) )
	{
		free( _loginRequestBuffer );

		tempLRB.length += tempLRB.length;

		tempLRB.data = ( char* ) malloc( tempLRB.length );
		if ( !tempLRB.data )
		{
			_ommBaseImpl.handleMee( "Failed to allocate memory for RsslRDMLoginRequest in LoginCallbackClient." );
			return;
		}

		_loginRequestBuffer = tempLRB.data;
	}

	_requestLogin = Login::create( _ommBaseImpl );

	_requestLogin->set( &_loginRequestMsg );

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "RDMLogin request message was populated with this info: " );
		temp.append( CR )
		.append( _requestLogin->toString() );
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}

	/* Initialize the refresh flag */
	_refreshReceived = false;
}

UInt32 LoginCallbackClient::sendLoginClose()
{
	return _loginList.sendLoginClose();
}

RsslRDMLoginRequest* LoginCallbackClient::getLoginRequest()
{
	return &_loginRequestMsg;
}

RsslRDMLoginRefresh* LoginCallbackClient::getLoginRefresh()
{
	return &_loginRefreshMsg;
}

/* This function will take in an RsslRDMLoginRequest, and overlay any string element changes to the stored request login request in the loginCallbackClient.
  In addition, this function will apply the pause flag. */
void LoginCallbackClient::overlayLoginRequest(RsslRDMLoginRequest* pRequest)
{
	RsslRDMLoginRequest tempRequest;
	bool bufferChange = false;
	/* Copy the RsslRDMLoginRequest's values */
	tempRequest = _loginRequestMsg;

	if (pRequest->userName.length != 0 && !rsslBufferIsEqual(&pRequest->userName, &_loginRequestMsg.userName))
	{
		tempRequest.userName = pRequest->userName;
		bufferChange = true;
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_ID)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_APPLICATION_ID) || !rsslBufferIsEqual(&pRequest->applicationId, &_loginRequestMsg.applicationId))
		{
			tempRequest.applicationId = pRequest->applicationId;
			tempRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_ID;
			bufferChange = true;
		}
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_PASSWORD)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_PASSWORD) || !rsslBufferIsEqual(&pRequest->password, &_loginRequestMsg.password))
		{
			tempRequest.password = pRequest->password;
			tempRequest.flags |= RDM_LG_RQF_HAS_PASSWORD;
			bufferChange = true;
		}
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_NAME)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_APPLICATION_NAME) ||  !rsslBufferIsEqual(&pRequest->applicationName, &_loginRequestMsg.applicationName))
		{
			tempRequest.applicationName = pRequest->applicationName;
			tempRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_NAME;
			bufferChange = true;
		}
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_INSTANCE_ID)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_INSTANCE_ID) || !rsslBufferIsEqual(&pRequest->instanceId, &_loginRequestMsg.instanceId))
		{
			tempRequest.instanceId = pRequest->instanceId;
			tempRequest.flags |= RDM_LG_RQF_HAS_INSTANCE_ID;
			bufferChange = true;
		}
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN) || !rsslBufferIsEqual(&pRequest->applicationAuthorizationToken, &_loginRequestMsg.applicationAuthorizationToken))
		{
			tempRequest.applicationAuthorizationToken = pRequest->applicationAuthorizationToken;
			tempRequest.flags |= RDM_LG_RQF_HAS_APPLICATION_AUTHORIZATION_TOKEN;
			bufferChange = true;
		}
	}

	if (pRequest->flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED)
	{
		if (!(_loginRequestMsg.flags & RDM_LG_RQF_HAS_AUTHN_EXTENDED) || !rsslBufferIsEqual(&pRequest->applicationAuthorizationToken, &_loginRequestMsg.applicationAuthorizationToken))
		{
			tempRequest.authenticationExtended = pRequest->authenticationExtended;
			tempRequest.flags |= RDM_LG_RQF_HAS_AUTHN_EXTENDED;
			bufferChange = true;
		}
	}

	/* Set the pause all flag.  Unset this after submitting it to the reactor */
	if (pRequest->flags & RDM_LG_RQF_PAUSE_ALL)
	{
		tempRequest.flags |= RDM_LG_RQF_PAUSE_ALL;
		bufferChange = true;
	}


	if (pRequest->flags & RDM_LG_RQF_NO_REFRESH)
	{
		tempRequest.flags |= RDM_LG_RQF_NO_REFRESH;
		bufferChange = true;
	}


	
	/* Deep copy the tempRequest onto the previous request.  Since we're potentially pulling data from 
	   the the previous memory buffer, do not free the buffer'd until the copy operation is complete. */
	if (bufferChange == true)
	{
		RsslBuffer tempLRB;
		char* tmpBuffer;
		tempLRB.length = 1000;
		tmpBuffer = (char*)malloc(tempLRB.length);
		tempLRB.data = tmpBuffer;
		if (!tempLRB.data)
		{
			_ommBaseImpl.handleMee("Failed to allocate memory for RsslRDMLoginRequest in LoginCallbackClient.");
			return;
		}

		while (RSSL_RET_BUFFER_TOO_SMALL == rsslCopyRDMLoginRequest(&_loginRequestMsg, &tempRequest, &tempLRB))
		{
			/* Buffer too small, free the data and re-malloc */
			free(tmpBuffer);
			tempLRB.length += tempLRB.length;
			tmpBuffer = (char*)malloc(tempLRB.length);
			tempLRB.data = tmpBuffer;

			if (!tempLRB.data)
			{
				_ommBaseImpl.handleMee("Failed to allocate memory for RsslRDMLoginRequest in LoginCallbackClient.");
				return;
			}
		}

		free(_loginRequestBuffer);
		_loginRequestBuffer = tmpBuffer;
	}
}

RsslReactorCallbackRet LoginCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMLoginMsgEvent* pEvent )
{
	RsslRDMLoginMsg* pLoginMsg = pEvent->pRDMLoginMsg;

	if ( !pLoginMsg )
	{
		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received an event without RDMLogin message" );
			temp.append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pLoginMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_LG_MT_REFRESH:
	{
		Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
		if ( !pLogin )
		{
			_loginList.removeLogin(pRsslReactorChannel);

			pLogin = Login::create( _ommBaseImpl );
			_loginList.addLogin( pLogin );
		}

		pLogin->set( &pLoginMsg->refresh ).setChannel( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->setLogin( pLogin );

		RsslBuffer tempLRB;

		tempLRB.length = 1000;
		tempLRB.data = (char*)malloc(tempLRB.length);
		if (!tempLRB.data)
		{
			_ommBaseImpl.handleMee("Failed to allocate memory for RsslRDMLoginRefresh in LoginCallbackClient.");
			return RSSL_RC_CRET_FAILURE;
		}

		_loginRefreshBuffer = tempLRB.data;

		while (RSSL_RET_BUFFER_TOO_SMALL == rsslCopyRDMLoginRefresh(&_loginRefreshMsg, &pLoginMsg->refresh, &tempLRB))
		{
			free(_loginRequestBuffer);

			tempLRB.length += tempLRB.length;

			tempLRB.data = (char*)malloc(tempLRB.length);
			if (!tempLRB.data)
			{
				_ommBaseImpl.handleMee("Failed to allocate memory for RsslRDMLoginRefresh in LoginCallbackClient.");
				return RSSL_RC_CRET_FAILURE;
			}

			_loginRequestBuffer = tempLRB.data;
		}

		RsslState* pState = &pLoginMsg->refresh.state;

		bool closeChannel = false;

		if ( pState->streamState != RSSL_STREAM_OPEN )
		{
			closeChannel = true;
			_ommBaseImpl.setState( OmmBaseImpl::RsslChannelUpStreamNotOpenEnum );
			stateToString( pState, _loginFailureMsg );

			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "RDMLogin stream was closed with refresh message" );
				temp.append( CR );
				Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
				if ( pLogin )
					temp.append( pLogin->toString() ).append( CR );
				temp.append( "State: " ).append( _loginFailureMsg );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
		}
		else if ( pState->dataState == RSSL_DATA_SUSPECT )
		{
			if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMLogin stream state was changed to suspect with refresh message" );
				temp.append( CR );
				Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
				if ( pLogin )
					temp.append( pLogin->toString() ).append( CR );
				temp.append( "State: " ).append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}

			_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenSuspectEnum );
		}
		else
		{
			_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenOkEnum );

			_ommBaseImpl.setActiveRsslReactorChannel( (Channel*)(pRsslReactorChannel->userSpecPtr) );
			_ommBaseImpl.reLoadDirectory();

			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMLogin stream was open with refresh message" );
				temp.append( CR )
				.append( pLogin->toString() ).append( CR )
				.append( "State: " ).append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}

		_refreshReceived = true;

		_loginItemLock.lock();

		if ( _loginItems.size() )
			processRefreshMsg( pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent );

		_loginItemLock.unlock();

		if (closeChannel)
		{
			_ommBaseImpl.unsetActiveRsslReactorChannel((Channel*)(pRsslReactorChannel->userSpecPtr));
			_ommBaseImpl.closeChannel(pRsslReactorChannel);
		}
		break;
	}
	case RDM_LG_MT_STATUS:
	{
		bool closeChannel = false;

		if ( pLoginMsg->status.flags & RDM_LG_STF_HAS_STATE )
		{
			RsslState* pState = &pLoginMsg->status.state;

			if ( pState->streamState != RSSL_STREAM_OPEN )
			{
				closeChannel = true;
				_ommBaseImpl.setState( OmmBaseImpl::RsslChannelUpStreamNotOpenEnum );
				stateToString( pState, _loginFailureMsg );

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "RDMLogin stream was closed with status message" );
					temp.append( CR );
					Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
					if ( pLogin )
						temp.append( pLogin->toString() ).append( CR );
					temp.append( "State: " ).append( _loginFailureMsg );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMLogin stream state was changed to suspect with status message" );
					temp.append( CR );
					Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
					if ( pLogin )
						temp.append( pLogin->toString() ).append( CR );
					temp.append( "State: " ).append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenSuspectEnum );
			}
			else
			{
				_ommBaseImpl.setActiveRsslReactorChannel((Channel*)(pRsslReactorChannel->userSpecPtr));
				_ommBaseImpl.reLoadDirectory();

				if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMLogin stream was open with status message" );
					temp.append( CR );
					Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
					if ( pLogin )
						temp.append( pLogin->toString() ).append( CR );
					temp.append( "State: " ).append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
				}

				_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenOkEnum );
			}
		}
		else
		{
			if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received RDMLogin status message without the state" );
				Login* pLogin = _loginList.getLogin( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
				if ( pLogin )
					temp.append( CR ).append( pLogin->toString() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}
		}

		_loginItemLock.lock();

		if ( _loginItems.size() )
			processStatusMsg( pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent );

		_loginItemLock.unlock();

		if (closeChannel)
		{
			_ommBaseImpl.unsetActiveRsslReactorChannel((Channel*)(pRsslReactorChannel->userSpecPtr));
			_ommBaseImpl.closeChannel(pRsslReactorChannel);
		}

		break;
	}
	default:
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received unknown RDMLogin message type" );
			temp.append( CR )
			.append( "Message type value " ).append( pLoginMsg->rdmMsgBase.rdmMsgType );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		break;
	}
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet LoginCallbackClient::processRefreshMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* pEvent )
{
	RsslBuffer rsslMsgBuffer;
	rsslClearBuffer( &rsslMsgBuffer );

	if ( pRsslMsg )
	{
		StaticDecoder::setRsslData( &_refreshMsg, pRsslMsg,
		                            pRsslReactorChannel->majorVersion,
		                            pRsslReactorChannel->minorVersion,
		                            0 );
	}
	else
	{
		if ( !convertRdmLoginToRsslBuffer( pRsslReactorChannel, pEvent, &rsslMsgBuffer ) )
			return RSSL_RC_CRET_SUCCESS;

		StaticDecoder::setRsslData( &_refreshMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion,
		                            pRsslReactorChannel->minorVersion,
		                            0 );
	}

	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->setEventChannel( pRsslReactorChannel );

		item->onAllMsg( _refreshMsg );
		item->onRefreshMsg( _refreshMsg );
	}

	if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
			_loginItems[idx]->remove();

		_loginItems.clear();
	}

	if ( rsslMsgBuffer.data )
		free( rsslMsgBuffer.data );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet LoginCallbackClient::processStatusMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* pEvent )
{
	RsslBuffer rsslMsgBuffer;
	rsslClearBuffer( &rsslMsgBuffer );

	if ( pRsslMsg )
	{
		StaticDecoder::setRsslData( &_statusMsg, pRsslMsg,
		                            pRsslReactorChannel->majorVersion,
		                            pRsslReactorChannel->minorVersion,
		                            0 );
	}
	else
	{
		if ( !convertRdmLoginToRsslBuffer( pRsslReactorChannel, pEvent, &rsslMsgBuffer ) )
			return RSSL_RC_CRET_SUCCESS;

		StaticDecoder::setRsslData( &_statusMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion,
		                            pRsslReactorChannel->minorVersion,
		                            0 );
	}

	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->onAllMsg( _statusMsg );
		item->onStatusMsg( _statusMsg );
	}

	if ( _statusMsg.hasState() &&
	     _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
			_loginItems[idx]->remove();

		_loginItems.clear();
	}

	if ( rsslMsgBuffer.data )
		free( rsslMsgBuffer.data );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet LoginCallbackClient::processGenericMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
	                            pRsslReactorChannel->majorVersion,
	                            pRsslReactorChannel->minorVersion,
								channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->onAllMsg( _genericMsg );
		item->onGenericMsg( _genericMsg );
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet LoginCallbackClient::processAckMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
	                            pRsslReactorChannel->majorVersion,
	                            pRsslReactorChannel->minorVersion,
								channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->onAllMsg( _ackMsg );
		item->onAckMsg( _ackMsg );
	}

	return RSSL_RC_CRET_SUCCESS;
}

void LoginCallbackClient::processChannelEvent( RsslReactorChannelEvent* pEvent )
{
	if ( !_loginList.size() )
		return;

	switch ( pEvent->channelEventType )
	{
	case RSSL_RC_CET_CHANNEL_READY:
	{
		if (!_notifyChannelDownReconnecting)
			break;

		RsslStatusMsg rsslStatusMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_loginList[0]->populate( rsslStatusMsg, temp );

		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = ( char* )"channel up";
		rsslStatusMsg.state.text.length = 10;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*) &rsslStatusMsg,
			pEvent->pReactorChannel->majorVersion,
			pEvent->pReactorChannel->minorVersion,
			0 );

		for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];

			item->setEventChannel( pEvent->pReactorChannel );
			item->onAllMsg( _statusMsg );
			item->onStatusMsg( _statusMsg );
		}

		_notifyChannelDownReconnecting = false;

	}
	break;
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING :
	{

		if (_notifyChannelDownReconnecting)
			break;
		RsslStatusMsg rsslStatusMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_loginList[0]->populate( rsslStatusMsg, temp );

		rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = (char*)"channel down";
		rsslStatusMsg.state.text.length = 12;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*)&rsslStatusMsg,
			pEvent->pReactorChannel->majorVersion,
			pEvent->pReactorChannel->minorVersion,
			0 );

		for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];

			item->setEventChannel( pEvent->pReactorChannel );
			item->onAllMsg( _statusMsg );
			item->onStatusMsg( _statusMsg );
		}

		_notifyChannelDownReconnecting = true;

	}
	break;
	case RSSL_RC_CET_CHANNEL_DOWN :
	{
		RsslStatusMsg rsslStatusMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_loginList[0]->populate( rsslStatusMsg, temp );

		rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
		rsslStatusMsg.state.streamState = RSSL_STREAM_CLOSED;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = (char*)"channel closed";
		rsslStatusMsg.state.text.length = 14;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*)&rsslStatusMsg,
			pEvent->pReactorChannel->majorVersion,
			pEvent->pReactorChannel->minorVersion,
			0 );

		for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];

			item->setEventChannel( pEvent->pReactorChannel );
			item->onAllMsg( _statusMsg );
			item->onStatusMsg( _statusMsg );
		}
	}
	break;
	default :
		break;
	}
}

bool LoginCallbackClient::convertRdmLoginToRsslBuffer( RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* pEvent, RsslBuffer* pRsslMsgBuffer )
{
	if ( !pRsslReactorChannel && !pEvent && !pRsslMsgBuffer ) return false;

	pRsslMsgBuffer->length = 4096;
	pRsslMsgBuffer->data = ( char* )malloc( sizeof( char ) * pRsslMsgBuffer->length );

	if ( !pRsslMsgBuffer->data )
	{
		_ommBaseImpl.handleMee( "Failed to allocate memory in LoginCallbackClient::convertRdmLoginToRsslBuffer()" );
		return false;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &eIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( pRsslMsgBuffer->data );
		_ommBaseImpl.handleIue( "Internal error. Failed to set encode iterator version in LoginCallbackClient::convertRdmLoginToRsslBuffer()", retCode );
		return false;
	}

	retCode = rsslSetEncodeIteratorBuffer( &eIter, pRsslMsgBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( pRsslMsgBuffer->data );
		_ommBaseImpl.handleIue( "Internal error. Failed to set encode iterator buffer in LoginCallbackClient::convertRdmLoginToRsslBuffer()", retCode);
		return false;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	retCode = rsslEncodeRDMLoginMsg( &eIter, pEvent->pRDMLoginMsg, &pRsslMsgBuffer->length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( pRsslMsgBuffer->data );

		pRsslMsgBuffer->length += pRsslMsgBuffer->length;
		pRsslMsgBuffer->data = ( char* )malloc( sizeof( char ) * pRsslMsgBuffer->length );

		if ( !pRsslMsgBuffer->data )
		{
			_ommBaseImpl.handleMee( "Failed to allocate memory in LoginCallbackClient::convertRdmLoginToRsslBuffer()" );
			return false;
		}

		retCode = rsslEncodeRDMLoginMsg( &eIter, pEvent->pRDMLoginMsg, &pRsslMsgBuffer->length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( pRsslMsgBuffer->data );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMLoginMsg in LoginCallbackClient::convertRdmLoginToRsslBuffer()" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
		return false;
	}

	return true;
}

Login* LoginCallbackClient::getLogin()
{
	return _requestLogin;
}

LoginItem* LoginCallbackClient::getLoginItem( const ReqMsg&, OmmConsumerClient& ommConsClient, void* closure )
{
	LoginItem* li = LoginItem::create( _ommBaseImpl, ommConsClient, closure, _loginList );

	if ( li )
	{
		_loginItems.push_back(li);
		if (_refreshReceived == true)
		{
			LoginItemCreationCallbackStruct* liccs(new LoginItemCreationCallbackStruct(this, li));
			TimeOut* timeOut = new TimeOut(_ommBaseImpl, 10, LoginCallbackClient::handleLoginItemCallback, liccs, true);
		}
	}

	return li;
}

NiProviderLoginItem* LoginCallbackClient::getLoginItem( const ReqMsg&, OmmProviderClient& ommProvClient, void* closure )
{
	NiProviderLoginItem* li = NiProviderLoginItem::create( _ommBaseImpl, ommProvClient, closure, _loginList );

	if ( li )
	{
		if ( getActiveChannel() )
			li->setEventChannel( getActiveChannel()->getRsslChannel() );

		_loginItems.push_back(li);
		if (_refreshReceived == true)
		{
			NiProviderLoginItemCreationCallbackStruct* liccs(new NiProviderLoginItemCreationCallbackStruct(this, li));
			TimeOut* timeOut = new TimeOut(_ommBaseImpl, 10, LoginCallbackClient::handleLoginItemCallback, liccs, true);
		}
		
	}

	return li;
}

Channel* LoginCallbackClient::getActiveChannel()
{
	UInt32 size = _loginList.size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		if ( _loginList.operator[](idx)->getChannel()->getChannelState() == Channel::ChannelReadyEnum )
			return _loginList.operator[](idx)->getChannel();
	}

	return size > 0? (_loginList.operator[](size -1)->getChannel()) : NULL;
}

void LoginCallbackClient::removeChannel(RsslReactorChannel* pRsslReactorChannel)
{
	_loginList.removeLogin(pRsslReactorChannel);
}

const EmaString& LoginCallbackClient::getLoginFailureMessage()
{
	return _loginFailureMsg;
}

LoginItem* LoginItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const LoginList& loginList )
{
	LoginItem* pItem = 0;

	try
	{
		pItem = new LoginItem( ommBaseImpl, ommConsClient, closure, loginList );
	}
	catch ( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create LoginItem" );

	return pItem;
}

LoginItem::LoginItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const LoginList& loginList ) :
	SingleItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_loginList( &loginList )
{
	_streamId = EMA_LOGIN_STREAM_ID;
}

LoginItem::~LoginItem()
{
	_loginList = 0;
}

bool LoginItem::open( RsslRDMLoginRequest* rsslRdmLoginRequest, const LoginList& loginList )
{
	bool retCode = true;

	if ( !_loginList )
		_loginList = &loginList;

	_ommBaseImpl.pipeWrite();

	return true;
}

bool LoginItem::close()
{
	remove();

	return true;
}

bool LoginItem::modify( const ReqMsg& reqMsg )
{
	RsslRDMLoginRequest tempRequest;
	rsslClearRDMLoginRequest(&tempRequest);
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator(&dIter);
	RsslBuffer tmpBuf;
	rsslClearBuffer(&tmpBuf);
	RsslErrorInfo errorInfo;
	bool ret;

	rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	if (rsslDecodeRDMLoginMsg(&dIter, (RsslMsg*)(static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder())).getRsslRequestMsg(), (RsslRDMLoginMsg*)&tempRequest, &tmpBuf, &errorInfo) != RSSL_RET_SUCCESS)
	{
		EmaString temp("Internal error: rsslDecodeRDMLoginMsg failed.");
		temp.append(CR)
			.append("Error Location ").append(errorInfo.errorLocation).append(CR)
			.append("Error Text ").append(errorInfo.rsslError.text);
		_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
	}

	_ommBaseImpl.getLoginCallbackClient().overlayLoginRequest(&tempRequest);

	ret = submit( _ommBaseImpl.getLoginCallbackClient().getLoginRequest() );
	/* Unset the Pause and No Refresh flag on the cached request */
	_ommBaseImpl.getLoginCallbackClient().getLoginRequest()->flags &= ~(RDM_LG_RQF_PAUSE_ALL | RDM_LG_RQF_NO_REFRESH);

	return ret;

}

bool LoginItem::submit( const PostMsg& postMsg )
{
	const PostMsgEncoder& postMsgEncoder = static_cast<const PostMsgEncoder&>( postMsg.getEncoder() );

	RsslPostMsg* pRsslPostMsg = postMsgEncoder.getRsslPostMsg();

	/* if the PostMsg has the Service Name */
	if ( postMsgEncoder.hasServiceName() )
	{
		EmaString serviceName = postMsgEncoder.getServiceName();
		RsslBuffer serviceNameBuffer;
		serviceNameBuffer.data = (char*) serviceName.c_str();
		serviceNameBuffer.length = serviceName.length();
			
		return submit( pRsslPostMsg, &serviceNameBuffer );
	}

	return submit( static_cast<const PostMsgEncoder&>( postMsg.getEncoder() ).getRsslPostMsg(), NULL );
}

bool LoginItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

bool LoginItem::submit( RsslRDMLoginRequest* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslRequestMsg->rdmMsgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRDMMsg = ( RsslRDMMsg* )pRsslRequestMsg;

		submitMsgOpts.majorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->majorVersion;
		submitMsgOpts.minorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
		if ( ( ret = rsslReactorSubmitMsg( _loginList->operator[]( idx )->getChannel()->getRsslReactor(),
		                                   _loginList->operator[]( idx )->getChannel()->getRsslChannel(),
		                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
				temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			EmaString text( "Failed to reissue login request. Reason: " );
			text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.handleIue( text, ret );

			return false;
		}
	}

	return true;
}

bool LoginItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslGenericMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslGenericMsg;

		submitMsgOpts.majorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->majorVersion;
		submitMsgOpts.minorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
		if ( ( ret = rsslReactorSubmitMsg( _loginList->operator[]( idx )->getChannel()->getRsslReactor(),
		                                   _loginList->operator[]( idx )->getChannel()->getRsslChannel(),
		                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslGenericMsg* )" );
				temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			EmaString text( "Failed to submit GenericMsg on login stream. Reason: " );
			text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.handleIue( text, ret );

			return false;
		}
	}

	return true;
}

bool LoginItem::submit( RsslPostMsg* pRsslPostMsg, RsslBuffer* pServiceName )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslPostMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pServiceName = pServiceName;
		submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslPostMsg;

		submitMsgOpts.majorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->majorVersion;
		submitMsgOpts.minorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
		if ( ( ret = rsslReactorSubmitMsg( _loginList->operator[]( idx )->getChannel()->getRsslReactor(),
		                                   _loginList->operator[]( idx )->getChannel()->getRsslChannel(),
		                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslPostMsg* )" );
				temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			EmaString text( "Failed to submit PostMsg on login stream. Reason: " );
			text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.handleIue( text, ret );

			return false;
		}
	}

	return true;
}

void LoginCallbackClient::sendInternalMsg( LoginItem* loginItem )
{
	RsslRefreshMsg rsslRefreshMsg;
	char tempBuffer[1000];
	RsslBuffer temp;
	temp.data = tempBuffer;
	temp.length = 1000;

	_loginList[0]->populate( rsslRefreshMsg, temp );

	RsslReactorChannel* pRsslReactorChannel = _loginList[0]->getChannel()->getRsslChannel();

	StaticDecoder::setRsslData( &_refreshMsg, reinterpret_cast< RsslMsg* >( &rsslRefreshMsg ),
	                            pRsslReactorChannel->majorVersion,
	                            pRsslReactorChannel->minorVersion,
	                            0 );

	_ommBaseImpl.msgDispatched();
	Item* item = static_cast< Item* >( loginItem );
	item->onAllMsg( _refreshMsg );
	item->onRefreshMsg( _refreshMsg );

	if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
		loginItem->remove();
}

void LoginCallbackClient::handleLoginItemCallback( void* args )
{
	LoginItemCreationCallbackStruct* arguments = reinterpret_cast< LoginItemCreationCallbackStruct* >( args );

	arguments->loginCallbackClient->sendInternalMsg( arguments->loginItem );
}

NiProviderLoginItem* NiProviderLoginItem::create( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure, const LoginList& loginList )
{
	NiProviderLoginItem* pItem = 0;

	try
	{
		pItem = new NiProviderLoginItem( ommBaseImpl, ommProvClient, closure, loginList );
	}
	catch ( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create NiProviderLoginItem" );

	return pItem;
}

NiProviderLoginItem::NiProviderLoginItem( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure, const LoginList& loginList ) :
	NiProviderSingleItem( ommBaseImpl, ommProvClient, 0, closure, 0 ),
	_loginList( &loginList )
{
	_domainType = MMT_LOGIN;
	_streamId = EMA_LOGIN_STREAM_ID;
}

NiProviderLoginItem::~NiProviderLoginItem()
{
	_loginList = 0;
}

bool NiProviderLoginItem::open( RsslRDMLoginRequest* rsslRdmLoginRequest, const LoginList& loginList )
{
	bool retCode = true;

	if ( !_loginList )
		_loginList = &loginList;

	_ommBaseImpl.pipeWrite();

	return true;
}

bool NiProviderLoginItem::close()
{
	remove();

	return true;
}

bool NiProviderLoginItem::modify( const ReqMsg& reqMsg )
{
	RsslRDMLoginRequest tempRequest;
	rsslClearRDMLoginRequest(&tempRequest);
	RsslDecodeIterator dIter;
	rsslClearDecodeIterator(&dIter);
	RsslBuffer tmpBuf;
	rsslClearBuffer(&tmpBuf);
	RsslErrorInfo errorInfo;

	bool ret;

	/* Decode the msg to the temp RDMLoginRequest*/
	rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	if (rsslDecodeRDMLoginMsg(&dIter, (RsslMsg*)(static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder())).getRsslRequestMsg(), (RsslRDMLoginMsg*)&tempRequest, &tmpBuf, &errorInfo) != RSSL_RET_SUCCESS)
	{
		EmaString temp("Internal error: rsslDecodeRDMLoginMsg failed.");
		temp.append(CR)
			.append("Error Location ").append(errorInfo.errorLocation).append(CR)
			.append("Error Text ").append(errorInfo.rsslError.text);
		_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
	}

	_ommBaseImpl.getLoginCallbackClient().overlayLoginRequest(&tempRequest);

	ret = submit(_ommBaseImpl.getLoginCallbackClient().getLoginRequest());
	/* Unset the Pause and No Refresh flag on the cached request */
	_ommBaseImpl.getLoginCallbackClient().getLoginRequest()->flags &= ~(RDM_LG_RQF_PAUSE_ALL | RDM_LG_RQF_NO_REFRESH);

	return ret;
}

bool NiProviderLoginItem::submit( const PostMsg& )
{
	return false;
}

bool NiProviderLoginItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

bool NiProviderLoginItem::submit(RsslRDMLoginRequest* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslRequestMsg->rdmMsgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRDMMsg = (RsslRDMMsg*) pRsslRequestMsg;

		submitMsgOpts.majorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->majorVersion;
		submitMsgOpts.minorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
		if ( ( ret = rsslReactorSubmitMsg( _loginList->operator[]( idx )->getChannel()->getRsslReactor(),
			_loginList->operator[]( idx )->getChannel()->getRsslChannel(),
			&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
				temp.append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
					.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			EmaString text( "Failed to reissue login request. Reason: " );
			text.append( rsslRetCodeToString( ret ) )
				.append( ". Error text: " )
				.append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.handleIue( text, ret );

			return false;
		}
	}

	return true;
}

bool NiProviderLoginItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslGenericMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslGenericMsg;

		submitMsgOpts.majorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->majorVersion;
		submitMsgOpts.minorVersion = _loginList->operator[]( idx )->getChannel()->getRsslChannel()->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
		if ( ( ret = rsslReactorSubmitMsg( _loginList->operator[]( idx )->getChannel()->getRsslReactor(),
			_loginList->operator[]( idx )->getChannel()->getRsslChannel(),
			&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslGenericMsg* )" );
				temp.append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
					.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			EmaString text( "Failed to submit GenericMsg on login stream. Reason: " );
			text.append( rsslRetCodeToString( ret ) )
				.append( ". Error text: " )
				.append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.handleIue( text, ret );

			return false;
		}
	}

	return true;
}

bool NiProviderLoginItem::submit( RsslPostMsg* )
{
	return false;
}
