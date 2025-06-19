/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2022,2024-2025 LSEG. All rights reserved.
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
#include "ConsumerRoutingSession.h"
#include "ConsumerRoutingChannel.h"
#include "OmmState.h"

#include <new>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

const EmaString LoginCallbackClient::_clientName( "LoginCallbackClient" );
const EmaString LoginItem::_clientName( "LoginItem" );
const EmaString NiProviderLoginItem::_clientName( "NiProviderLoginItem" );

LoginRdmRefreshMsgImpl* LoginRdmRefreshMsgImpl::create( OmmBaseImpl& ommBaseImpl )
{
	try
	{
		return new LoginRdmRefreshMsgImpl();
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create Login." );
	}

	return NULL;
}

void LoginRdmRefreshMsgImpl::destroy( LoginRdmRefreshMsgImpl*& pLogin )
{
	if ( pLogin )
	{
		delete pLogin;
		pLogin = 0;
	}
}

LoginRdmRefreshMsgImpl::LoginRdmRefreshMsgImpl() :
	_username(),
	_position(),
	_applicationId(),
	_applicationName(),
	_authenticationErrorText(),
	_authenticationExtendedResp(),
	_toString(),
	_stateText(),
	_pChannel( 0 )
{
	_toStringSet = false;
	_initialSet = false;
	rsslClearRDMLoginRefresh(&_refreshMsg);
}

LoginRdmRefreshMsgImpl::~LoginRdmRefreshMsgImpl()
{
}

void LoginRdmRefreshMsgImpl::clear()
{
	_initialSet = false;
	_toStringSet = false;
	_username.clear();
	_position.clear();
	_applicationId.clear();
	_applicationName.clear();
	_authenticationErrorText.clear();
	_authenticationExtendedResp.clear();
	_stateText.clear();
	_toString.clear();
	_pChannel = NULL;

	rsslClearRDMLoginRefresh(&_refreshMsg);
}

Channel* LoginRdmRefreshMsgImpl::getChannel() const
{
	return _pChannel;
}

LoginRdmRefreshMsgImpl& LoginRdmRefreshMsgImpl::setChannel( Channel* pChannel )
{
	_toStringSet = false;
	_pChannel = pChannel;
	return *this;
}

RsslRDMLoginRefresh* LoginRdmRefreshMsgImpl::getRefreshMsg()
{
	return &_refreshMsg;
}

const EmaString& LoginRdmRefreshMsgImpl::toString()
{
	if ( !_toStringSet )
	{
		_toStringSet = true;
		_toString.set("username ").append(_username).append(CR)
			.append("usernameType ").append(_refreshMsg.userNameType).append(CR)
			.append("position ").append((_refreshMsg.flags & RDM_LG_RFF_HAS_POSITION) != 0 ? _position : "<not set>").append(CR)
			.append("appId ").append((_refreshMsg.flags & RDM_LG_RFF_HAS_APPLICATION_ID) != 0 ? _applicationId : "<not set>").append(CR)
			.append("applicationName ").append((_refreshMsg.flags & RDM_LG_RFF_HAS_APPLICATION_NAME) != 0 ? _applicationName : "<not set>").append(CR)
			.append("singleOpen ").append(_refreshMsg.singleOpen).append(CR)
			.append("allowSuspect ").append(_refreshMsg.allowSuspectData).append(CR)
			.append("optimizedPauseResume ").append(_refreshMsg.supportOptimizedPauseResume).append(CR)
			.append("permissionExpressions ").append(_refreshMsg.providePermissionExpressions).append(CR)
			.append("permissionProfile ").append(_refreshMsg.providePermissionProfile).append(CR)
			.append("supportBatchRequest ").append(_refreshMsg.supportBatchRequests).append(CR)
			.append("supportEnhancedSymbolList ").append(_refreshMsg.supportEnhancedSymbolList).append(CR)
			.append("supportPost ").append(_refreshMsg.supportOMMPost).append(CR)
			.append("supportRtt ").append((_refreshMsg.flags & RDM_LG_RFF_RTT_SUPPORT) != 0 ? 1 : 0).append(CR)
			.append("supportViewRequest ").append(_refreshMsg.supportViewRequests).append(CR);

		if ((_refreshMsg.flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE) != 0)
			_toString.append(CR).append("authenticationTTReissue ").append(_refreshMsg.authenticationTTReissue);
		if ((_refreshMsg.flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE) != 0)
			_toString.append(CR).append("authenticationErrorCode ").append(_refreshMsg.authenticationErrorCode);
		if ((_refreshMsg.flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT) != 0)
			_toString.append(CR).append("authenticationErrorText ").append(_authenticationErrorText);
	}

	return _toString;
}

LoginRdmRefreshMsgImpl& LoginRdmRefreshMsgImpl::set( RsslRDMLoginRefresh* pRefresh )
{
	_toStringSet = false;

	// Shallow copy the RDM Login Refresh message here, we'll do a deep copy of the important things right after this.
	_refreshMsg = *pRefresh;

	// EMA does not use the connection config information, so we will drop it from the cached value.
	_refreshMsg.flags &= ~RDM_LG_RFF_HAS_CONN_CONFIG;
	_refreshMsg.serverCount = 0;
	_refreshMsg.serverList = NULL;

	_username.clear();

	_username.set(pRefresh->userName.data, pRefresh->userName.length);
	_refreshMsg.userName.data = (char*)_username.c_str();
	_refreshMsg.userName.length = _username.length();

	_position.clear();
	if ((pRefresh->flags & RDM_LG_RFF_HAS_POSITION) != 0)
	{
		_position.set(pRefresh->position.data, pRefresh->position.length);
		_refreshMsg.position.data = (char*)_position.c_str();
		_refreshMsg.position.length = _position.length();

	}

	_applicationId.clear();
	if((pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID) != 0)
	{
		_applicationId.set(pRefresh->applicationId.data, pRefresh->applicationId.length);
		_refreshMsg.applicationId.data = (char*)_applicationId.c_str();
		_refreshMsg.applicationId.length = _applicationId.length();
	}

	_applicationName.clear();
	if ((pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID) != 0)
	{
		_applicationName.set(pRefresh->applicationName.data, pRefresh->applicationName.length);
		_refreshMsg.applicationName.data = (char*)_applicationName.c_str();
		_refreshMsg.applicationName.length = _applicationName.length();
	}

	_authenticationErrorText.clear();
	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT) != 0)
	{
		_authenticationErrorText.set(pRefresh->authenticationErrorText.data, pRefresh->authenticationErrorText.length);
		_refreshMsg.authenticationErrorText.data = (char*)_authenticationErrorText.c_str();
		_refreshMsg.authenticationErrorText.length = _authenticationErrorText.length();
	}

	_authenticationErrorText.clear();
	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP) != 0)
	{
		_authenticationExtendedResp.setFrom(pRefresh->authenticationExtendedResp.data, pRefresh->authenticationExtendedResp.length);
		_refreshMsg.authenticationExtendedResp.data = (char*)_authenticationExtendedResp.c_buf();
		_refreshMsg.authenticationExtendedResp.length = _authenticationExtendedResp.length();
	}

	_stateText.clear();
	if (pRefresh->state.text.data != NULL && pRefresh->state.text.length != 0)
	{
		_stateText.set(pRefresh->state.text.data, pRefresh->state.text.length);
		_refreshMsg.state.text.data = (char*)_stateText.c_str();
		_refreshMsg.state.text.length = _stateText.length();
	}

	_initialSet = true;

	return *this;
}

// Aggregate the login information for request routing.  
// NOTE: If the state is set to either CLOSED or CLOSED_SUSPECT, this should not be called.  
// This will do the following:
// 1. If any of the buffer-based values are present, overwrite the current info
// 2. If any of the boolean values are set in the flags, AND both values in the both the current and pRefresh structures are set to 1, set the aggregate to 1.
//		If the new pRefresh value is set to 0, set the aggregate to 0.
// Note: This does not need to check against any of the other cases because we just need one reactor channel to have a specfic element set to 0.
// Re-aggregation will occurr whenever a channel gets closed.
bool LoginRdmRefreshMsgImpl::aggregateForRequestRouting(RsslRDMLoginRefresh* pRefresh, ConsumerRoutingSession* pConsumerSession)
{
	_toStringSet = false;
	bool changed = false;
	if (_initialSet == false)
	{
		rsslClearRDMLoginRefresh(&_refreshMsg);
		set(pRefresh);
		// Set singleOpen and allowSuspectData to 1 for all cases.
		_refreshMsg.flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
		_refreshMsg.singleOpen = 1;
		_refreshMsg.flags |= RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA;
		_refreshMsg.allowSuspectData = 1;
		return true;
	}

	// Set singleOpen and allowSuspectData to 1 for all cases.
	_refreshMsg.flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
	_refreshMsg.singleOpen = 1;

	_refreshMsg.flags |= RDM_LG_RFF_HAS_ALLOW_SUSPECT_DATA;
	_refreshMsg.allowSuspectData = 1;

	_username.set(pRefresh->userName.data, pRefresh->userName.length);
	_refreshMsg.userName.data = (char*)_username.c_str();
	_refreshMsg.userName.length = _username.length();

	if ( pRefresh->flags & RDM_LG_RFF_HAS_USERNAME_TYPE )
	{

		_refreshMsg.flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
		_refreshMsg.userNameType = pRefresh->userNameType;
	}
	else
		_refreshMsg.userNameType = 1;

	if ((pRefresh->flags & RDM_LG_RFF_HAS_POSITION) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_POSITION;
		_position.set(pRefresh->position.data, pRefresh->position.length);
		_refreshMsg.position.data = (char*)_position.c_str();
		_refreshMsg.position.length = _position.length();
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_POSITION;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_ID) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
		_applicationId.set( pRefresh->applicationId.data, pRefresh->applicationId.length );
		_refreshMsg.applicationId.data = (char*)_applicationId.c_str();
		_refreshMsg.applicationId.length = _applicationId.length();
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_APPLICATION_ID;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_APPLICATION_NAME) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
		_applicationName.set( pRefresh->applicationName.data, pRefresh->applicationName.length );
		_refreshMsg.applicationName.data = (char*)_applicationName.c_str();
		_refreshMsg.applicationName.length = _applicationName.length();
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_APPLICATION_NAME;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT;
		_authenticationErrorText.set(pRefresh->authenticationErrorText.data, pRefresh->authenticationErrorText.length);
		_refreshMsg.authenticationErrorText.data = (char*)_authenticationErrorText.c_str();
		_refreshMsg.authenticationErrorText.length = _authenticationErrorText.length();
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_ERROR_CODE) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_AUTHN_ERROR_CODE;
		_refreshMsg.authenticationErrorCode = pRefresh->authenticationErrorCode;
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_AUTHN_ERROR_CODE;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP;
		_authenticationExtendedResp.setFrom(pRefresh->authenticationExtendedResp.data, pRefresh->authenticationExtendedResp.length);
		_refreshMsg.authenticationExtendedResp.data = (char*)_authenticationExtendedResp.c_buf();
		_refreshMsg.authenticationExtendedResp.length = _authenticationExtendedResp.length();
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP;
	}


	if ((pRefresh->flags & RDM_LG_RFF_HAS_AUTHN_TT_REISSUE) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_AUTHN_TT_REISSUE;
		_refreshMsg.authenticationTTReissue = pRefresh->authenticationTTReissue;
	}
	else
	{
		_refreshMsg.flags &= ~RDM_LG_RFF_HAS_AUTHN_TT_REISSUE;
	}

	// For the following, if the flag is present on the new data, set it on the aggregation.
	// If the value of is 0 in either case(rsslClearRDMLoginRefresh sets everything to the OMM defaults), set it to 0, otherwise it should be 1.
	RsslUInt oldValue;

	if ((pRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_PROFILE;
		oldValue = _refreshMsg.providePermissionProfile;
		// if either new or current providePermissionProfile is 0, set it to 0, otherwise 1
		if (pRefresh->providePermissionProfile == 0 || _refreshMsg.providePermissionProfile == 0)
			_refreshMsg.providePermissionProfile = 0;
		else
			_refreshMsg.providePermissionProfile = 1;

		if (oldValue != _refreshMsg.providePermissionProfile)
			changed = true;
}


	if ((pRefresh->flags & RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR) != 0)
{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_PROVIDE_PERM_EXPR;

		oldValue = _refreshMsg.providePermissionExpressions;
		// if either new or current providePermissionExpressions is 0, set it to 0, otherwise 1
		if (pRefresh->providePermissionExpressions == 0 || _refreshMsg.providePermissionExpressions == 0)
			_refreshMsg.providePermissionExpressions = 0;
	else
			_refreshMsg.providePermissionExpressions = 1;

		if (oldValue != _refreshMsg.providePermissionExpressions)
			changed = true;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_BATCH) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_SUPPORT_BATCH;

		oldValue = _refreshMsg.supportBatchRequests;
		// if either new or current supportBatchRequests is 0, set it to 0, otherwise 1
		if (pRefresh->supportBatchRequests == 0 || _refreshMsg.supportBatchRequests == 0)
			_refreshMsg.supportBatchRequests = 0;
	else
			_refreshMsg.supportBatchRequests = 1;

		if (oldValue != _refreshMsg.supportBatchRequests)
			changed = true;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_POST) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_SUPPORT_POST;

		oldValue = _refreshMsg.supportOMMPost;
		// if either new or current supportOMMPost is 0, set it to 0, otherwise 1
		if (pRefresh->supportOMMPost == 0 || _refreshMsg.supportOMMPost == 0)
			_refreshMsg.supportOMMPost = 0;
	else
			_refreshMsg.supportOMMPost = 1;

		if (oldValue != _refreshMsg.supportOMMPost)
			changed = true;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_SUPPORT_OPT_PAUSE;

		oldValue = _refreshMsg.supportOptimizedPauseResume;
		// if either new or current supportOptimizedPauseResume is 0, set it to 0, otherwise 1
		if (pRefresh->supportOptimizedPauseResume == 0 || _refreshMsg.supportOptimizedPauseResume == 0)
			_refreshMsg.supportOptimizedPauseResume = 0;
		else
			_refreshMsg.supportOptimizedPauseResume = 1;

		if (oldValue != _refreshMsg.supportOptimizedPauseResume)
			changed = true;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_ENH_SL) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_SUPPORT_ENH_SL;

		oldValue = _refreshMsg.supportEnhancedSymbolList;
		// if either new or current supportEnhancedSymbolList is 0, set it to 0, otherwise 1
		if (pRefresh->supportEnhancedSymbolList == 0 || _refreshMsg.supportEnhancedSymbolList == 0)
			_refreshMsg.supportEnhancedSymbolList = 0;
	else
			_refreshMsg.supportEnhancedSymbolList = 1;

		if (oldValue != _refreshMsg.supportEnhancedSymbolList)
			changed = true;
	}

	if ((pRefresh->flags & RDM_LG_RFF_HAS_SUPPORT_VIEW) != 0)
	{
		_refreshMsg.flags |= RDM_LG_RFF_HAS_SUPPORT_VIEW;

		oldValue = _refreshMsg.supportViewRequests;
		// if either new or current supportViewRequests is 0, set it to 0, otherwise 1
		if (pRefresh->supportViewRequests == 0 || _refreshMsg.supportViewRequests == 0)
			_refreshMsg.supportViewRequests = 0;
	else
			_refreshMsg.supportViewRequests = 1;


		if (oldValue != _refreshMsg.supportViewRequests)
			changed = true;
	}

	// Remote RTT support if it is not present
	if ((pRefresh->flags & RDM_LG_RFF_RTT_SUPPORT) != (_refreshMsg.flags & RDM_LG_RFF_RTT_SUPPORT))
	{
		if ((pRefresh->flags & RDM_LG_RFF_RTT_SUPPORT) == 0)
		{
			_refreshMsg.flags &= ~RDM_LG_RFF_RTT_SUPPORT;
	}
		changed = true;
	}

	// This should always be OPEN/OK
	_refreshMsg.state.streamState = pRefresh->state.streamState;
	_refreshMsg.state.dataState = pRefresh->state.dataState;
	_refreshMsg.state.code = pRefresh->state.code;

	if (pRefresh->state.text.data != NULL && pRefresh->state.text.length != 0)
	{
		_stateText.set(pRefresh->state.text.data, pRefresh->state.text.length);
		_refreshMsg.state.text.data = (char*)_stateText.c_str();
		_refreshMsg.state.text.length = _stateText.length();
	}
	else
	{
		_stateText.clear();
		_refreshMsg.state.text.data = NULL;
		_refreshMsg.state.text.length = 0;
	}

	return changed;
}

// This is used exclusively internally, so we do not need to set a channel
bool LoginRdmRefreshMsgImpl::populate(RsslRefreshMsg& refresh, RsslBuffer& buffer, UInt8 majorVersion, UInt8 minorVersion)
{
	rsslClearRefreshMsg( &refresh );
	RsslErrorInfo errorInfo;
	RsslUInt32 bytesWritten;

	RsslElementList rsslEL;
	rsslClearElementList( &rsslEL );

	rsslEL.flags = RSSL_ELF_HAS_STANDARD_DATA;

	RsslElementEntry rsslEE;
	rsslClearElementEntry( &rsslEE );

	RsslEncodeIterator eIter;
	rsslClearEncodeIterator( &eIter );

	RsslDecodeIterator dIter;
	rsslClearDecodeIterator(&dIter);

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		if ( retCode != RSSL_RET_SUCCESS )
			return false;

	retCode = rsslSetEncodeIteratorBuffer( &eIter, &buffer );
		if ( retCode != RSSL_RET_SUCCESS )
			return false;

	if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&_refreshMsg, &bytesWritten, &errorInfo) != RSSL_RET_SUCCESS)
	{
			return false;
	}

	buffer.length = bytesWritten;

	/* set version info */
	rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	rsslSetDecodeIteratorBuffer(&dIter, &buffer);

	if (rsslDecodeMsg(&dIter, (RsslMsg*)&refresh) != RSSL_RET_SUCCESS)
	{
			return false;
	}

	refresh.state.text.data = ( char* ) "Refresh Completed";
	refresh.state.text.length = 17;

	return true;
}

bool LoginRdmRefreshMsgImpl::populate(RsslStatusMsg& status, RsslBuffer& buffer, UInt8 majorVersion, UInt8 minorVersion)
{
	rsslClearStatusMsg( &status );

	status.msgBase.streamId = EMA_LOGIN_STREAM_ID;
	status.msgBase.domainType = RSSL_DMT_LOGIN;
	status.msgBase.containerType = RSSL_DT_NO_DATA;
	status.msgBase.encDataBody.data = 0;
	status.msgBase.encDataBody.length = 0;
	status.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
	status.msgBase.msgKey.nameType = _refreshMsg.userNameType;
	status.msgBase.msgKey.name.data = ( char* )_username.c_str();
	status.msgBase.msgKey.name.length = _username.length();
	status.msgBase.msgKey.attribContainerType = RSSL_DT_NO_DATA;
	status.msgBase.msgKey.encAttrib.data = 0;
	status.msgBase.msgKey.encAttrib.length = 0;

	status.flags |= RSSL_STMF_CLEAR_CACHE | RSSL_STMF_HAS_MSG_KEY;

	return true;
}

LoginCallbackClient::LoginCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_loginItemLock(),
	_loginRefreshBuffer( 0 ),
	_refreshMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommBaseImpl( ommBaseImpl ),
	_loginItems(),
	_loginFailureMsg(),
	_loginInfo(),
	_refreshReceived(false),
	_notifyChannelDownReconnecting(false)
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created LoginCallbackClient" );
	}
}

LoginCallbackClient::~LoginCallbackClient()
{
	_loginItems.clear();

	if (_loginRefreshBuffer)
		free(_loginRefreshBuffer);

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed LoginCallbackClient" );
	}
}

LoginCallbackClient* LoginCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	try
	{
		return new LoginCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create LoginCallbackClient" );
	}

	return NULL;
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
	_notifyChannelDownReconnecting = false;

	// Set the loginInfo's starting pLoginRequestMsg here for.  This is ignored for request routing.
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		_loginInfo.pLoginRequestMsg = _ommBaseImpl.getActiveConfig().pRsslRDMLoginReq;

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "RDMLogin request message was populated with this info: " );
		temp.append( CR )
				.append(_ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->toString());
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
	}

	/* Initialize the refresh flag */
	_refreshReceived = false;
}


RsslRDMLoginRequest* LoginCallbackClient::getLoginRequest()
{
	return _loginInfo.pLoginRequestMsg->get();
}

RsslRDMLoginRefresh* LoginCallbackClient::getLoginRefresh()
{
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		return _loginInfo.loginRefreshMsg.getRefreshMsg();
	else
		return _ommBaseImpl.getConsumerRoutingSession()->aggregatedLoginInfo.loginRefreshMsg.getRefreshMsg();
}

void LoginCallbackClient::setLoginRequest(LoginRdmReqMsgImpl* newMsg)
{
	_loginInfo.pLoginRequestMsg = newMsg;
}

/* This function will take in an RsslRDMLoginRequest, and overlay any string element changes to the stored request login request in the loginCallbackClient.
  In addition, this function will apply the pause flag. */
void LoginCallbackClient::overlayLoginRequest(RsslRDMLoginRequest* pRequest)
{
	_loginInfo.pLoginRequestMsg->overlay(pRequest);
	}

void LoginCallbackClient::sendLoginClose(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel)
	{
	RsslCloseMsg rsslCloseMsg;

	rsslClearCloseMsg(&rsslCloseMsg);

	rsslCloseMsg.msgBase.streamId = EMA_LOGIN_STREAM_ID;
	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = RSSL_DMT_LOGIN;

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	RsslRet ret = rsslReactorSubmitMsg(pReactor,
		pReactorChannel,
		&submitMsgOpts, &rsslErrorInfo);
}

RsslReactorCallbackRet LoginCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMLoginMsgEvent* pEvent )
{
	RsslRDMLoginMsg* pLoginMsg = pEvent->pRDMLoginMsg;

	Channel* pChannel = ((Channel*)pRsslReactorChannel->userSpecPtr);

	bool clearAuthInfo = false;

	if (pChannel->getParentChannel() != NULL)
	{
		pChannel = pChannel->getParentChannel();
	}

	ConsumerRoutingSessionChannel* pRoutingSessionChannel = pChannel->getConsumerRoutingChannel();

	if ( !pLoginMsg )
	{
		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		{
			_ommBaseImpl.closeChannel(pRsslReactorChannel);
		}
		else
		{
			_ommBaseImpl.closeChannel( pRsslReactorChannel );

			if (pChannel->getConsumerRoutingChannel()->pRoutingSession->activeChannelCount == 0)
				_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);
		}

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
		LoginRdmRefreshMsgImpl* pLogin;
		RsslState* pState = &pLoginMsg->refresh.state;
		bool dispatchSessionLogin = false;

		if (pChannel->getConsumerRoutingChannel() == NULL)
		{
			pLogin = &_loginInfo.loginRefreshMsg;
			pLogin->set(&pLoginMsg->refresh).setChannel(pChannel);
		}
		else
		{
			pLogin = &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
			pChannel->getConsumerRoutingChannel()->receivedLoginRefresh = true;
			// Set the new refresh on the routing channel.
			pLogin->set( &pLoginMsg->refresh ).setChannel(pChannel);

			// Aggregate only if the refresh is OPEN/OK.  If it is not, we're going to either close the stream, or drop the info.
			// For a OPEN/SUSPECT, there's nothing to do as the information currently isn't valid but may come back later.
			// For CLOSED, this is considered a terminal case, so EMA will close the underlying channels, at which point the logins will get re-aggregated.
			if (pState->streamState == RSSL_STREAM_OPEN && pState->dataState == RSSL_DATA_OK)
		{
				pChannel->getConsumerRoutingChannel()->pRoutingSession->aggregateLoginRefreshInfo(&pLoginMsg->refresh);
			}
		}

		bool closeChannel = false;

		if ( pState->streamState != RSSL_STREAM_OPEN )
		{
			closeChannel = true;

			if (pChannel->getConsumerRoutingChannel() == NULL)
				_ommBaseImpl.setState(OmmBaseImpl::RsslChannelUpStreamNotOpenEnum);
			else
			{
				pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::RsslChannelUpStreamNotOpenEnum;

				int loginDownCount = 0;
				ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;

				// Check to see if all of the channels are in a RsslChannelUpStreamNotOpenEnum status or have been closed.  
				// If they are all in that status, set the ommbaseimpl state to RsslChannelUpStreamNotOpenEnum so it can transition to failure after this.
				if (_ommBaseImpl.isInitialized() == false)
				{
					for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
					{
						if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::RsslChannelUpStreamNotOpenEnum)
							loginDownCount++;
					}

					if (loginDownCount == pSession->activeChannelCount)
					{
						_ommBaseImpl.setState(OmmBaseImpl::RsslChannelUpStreamNotOpenEnum);
					}

					dispatchSessionLogin = true;
				}
			}

			stateToString( pState, _loginFailureMsg );

			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "RDMLogin stream was closed with refresh message" );
				temp.append( CR );
				LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
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
				LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
				if ( pLogin )
					temp.append( pLogin->toString() ).append( CR );
				temp.append( "State: " ).append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}
			if (pChannel->getConsumerRoutingChannel() == NULL)
				_ommBaseImpl.setState(OmmBaseImpl::LoginStreamOpenSuspectEnum);
			else
			{
				pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::LoginStreamOpenSuspectEnum;
				int loginSuspectCount = 0;
				ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;
				// Check to see if all of the channels are in a LoginStreamOpenSuspectEnum status or have been closed.  
				// If they are all in that status, set the ommbaseimpl state to RsslChannelUpStreamNotOpenEnum so it can transition to failure after this.
				if (_ommBaseImpl.isInitialized() == false)
				{
					for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
					{
						if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::LoginStreamOpenSuspectEnum)
							loginSuspectCount++;
					}

					if (loginSuspectCount == pSession->activeChannelCount)
					{
						_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenSuspectEnum );
						dispatchSessionLogin = true;
					}
				}
			}
		}
		else
		{

			if (pChannel->getConsumerRoutingChannel() == NULL)
				_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenOkEnum );
			else
			{
				pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::LoginStreamOpenOkEnum;
				int loginOkCount = 0;
				ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;

				// Check to see if all of the channels are in a LoginStreamOpenOk status.  
				// If they are all in that status, set the ommbaseimpl state to LoginStreamOpenOk so it can transition to the next step.
				if (_ommBaseImpl.isInitialized() == false)
				{
					for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
					{
						if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState >= OmmBaseImpl::LoginStreamOpenOkEnum)
							loginOkCount++;
					}

					if (loginOkCount == pSession->activeChannelCount)
					{
						_ommBaseImpl.setState(OmmBaseImpl::LoginStreamOpenOkEnum);
						dispatchSessionLogin = true;
					}
				}
				else
				{
					dispatchSessionLogin = true;
				}
			}
			
			// For Consumers, these are no-op, so we do not need to check for the request routing
			_ommBaseImpl.setActiveRsslReactorChannel(pChannel);
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
		{
			if (pChannel->getConsumerRoutingChannel() == NULL)
			{
				processRefreshMsg( pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent );
			}
			else if(dispatchSessionLogin == true)
			{
				RsslRefreshMsg rsslRefreshMsg;
				char tempBuffer[1000];
				RsslBuffer temp;
				temp.data = tempBuffer;
				temp.length = 1000;

				_ommBaseImpl.getConsumerRoutingSession()->aggregatedLoginInfo.loginRefreshMsg.populate(rsslRefreshMsg, temp, pRsslReactorChannel->pRsslChannel->majorVersion, pRsslReactorChannel->pRsslChannel->minorVersion);


				processRefreshMsg((RsslMsg*)&rsslRefreshMsg, pRsslReactorChannel, pEvent);
				_ommBaseImpl.getConsumerRoutingSession()->sentInitialLoginRefresh = true;
			}
		}

		_loginItemLock.unlock();

		if (clearAuthInfo && !closeChannel)
		{
			// unset the authentication elements in the aggregated RDMLoginResponse because this information is transient and will not apply outside of this callback.
			pLogin->getRefreshMsg()->flags &= ~(RDM_LG_RFF_HAS_AUTHN_TT_REISSUE | RDM_LG_RFF_HAS_AUTHN_EXTENDED_RESP | RDM_LG_RFF_HAS_AUTHN_ERROR_CODE | RDM_LG_RFF_HAS_AUTHN_ERROR_TEXT);
		}

		if (closeChannel)
		{
			if (pChannel->getConsumerRoutingChannel() == NULL)
			{
				_ommBaseImpl.unsetActiveRsslReactorChannel(pChannel);
				_ommBaseImpl.closeChannel(pRsslReactorChannel);
			}
			else
			{
				pChannel->getConsumerRoutingChannel()->closeOnDownReconnecting = true;

				if (pChannel->getConsumerRoutingChannel()->pRoutingSession->activeChannelCount == 0)
					_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);
			}
		}
		break;
	}
	case RDM_LG_MT_STATUS:
	{
		bool closeChannel = false;
		bool dispatchMsgToUser = true;			// Governs whether or not this status is dispatched to the user.  
												// For Request Routing, this should happen whenever a OPEN/OK happens, or ALL active channel logins are in OPEN/SUSPECT 
												// or have been explicilty CLOSED by the upstream provider.

		bool setToOpenOk = true;

		if ( pLoginMsg->status.flags & RDM_LG_STF_HAS_STATE )
		{
			RsslState* pState = &pLoginMsg->status.state;

			if ( pState->streamState != RSSL_STREAM_OPEN )
			{
				closeChannel = true;
				if (pChannel->getConsumerRoutingChannel() == NULL)
					_ommBaseImpl.setState(OmmBaseImpl::RsslChannelUpStreamNotOpenEnum);
				else
				{
					pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::RsslChannelUpStreamNotOpenEnum;

					int loginDownCount = 0;
					ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;

					// Check to see if all of the channels are in a RsslChannelUpStreamNotOpenEnum status or closed.  
					// If they are all in that status, set the ommbaseimpl state to RsslChannelUpStreamNotOpenEnum so it can transition to failure after this.
					if (_ommBaseImpl.isInitialized() == false)
					{
						for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
						{
							if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::RsslChannelUpStreamNotOpenEnum)
								loginDownCount++;
						}

						if (loginDownCount == pSession->activeChannelCount)
						{
							_ommBaseImpl.setState( OmmBaseImpl::RsslChannelUpStreamNotOpenEnum );
						}
						else
						{
							dispatchMsgToUser = false;
						}
					}
					else if (pSession->activeChannelCount == 1)
					{
						setToOpenOk = false;
					}
				}
				stateToString( pState, _loginFailureMsg );

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp;

					if (pChannel->getConsumerRoutingChannel() == NULL)
					{
						temp.append("RDMLogin stream was closed with status message");
					}
					else
					{
						temp.append("RDMLogin on channel ").append(pChannel->getName()).append(" stream was closed with status message");
					}

					temp.append( CR );
					LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
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

					EmaString temp;
					if (pChannel->getConsumerRoutingChannel() == NULL)
					{
						temp.append("RDMLogin stream state was changed to suspect with status message");
					}
					else
					{
						temp.append("RDMLogin on channel ").append(pChannel->getName()).append(" stream state was changed to suspect with status message");
					}
					temp.append( CR );
					LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
					if ( pLogin )
						temp.append( pLogin->toString() ).append( CR );
					temp.append( "State: " ).append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				if (pChannel->getConsumerRoutingChannel() == NULL)
					_ommBaseImpl.setState(OmmBaseImpl::LoginStreamOpenSuspectEnum);
				else
				{
					pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::LoginStreamOpenSuspectEnum;
					int loginSuspectCount = 0;
					ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;

					// Check to see if all of the channels are in a LoginStreamOpenSuspectEnum status or closed.  
					// If they are all in that status, set the ommbaseimpl state to LoginStreamOpenSuspectEnum so it can transition to the next step.

					for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
					{
						if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState == OmmBaseImpl::LoginStreamOpenSuspectEnum)
							loginSuspectCount++;
					}

					if (loginSuspectCount == pSession->activeChannelCount)
					{
						_ommBaseImpl.setState(OmmBaseImpl::LoginStreamOpenSuspectEnum);
						setToOpenOk = false;
					}
					else
					{
						dispatchMsgToUser = false;

						if (pSession->activeChannelCount == 1)
						{
							setToOpenOk = false;
						}
					}
				}
			}
			else
			{
				_ommBaseImpl.setActiveRsslReactorChannel(pChannel);
				_ommBaseImpl.reLoadDirectory();

				if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMLogin stream was open with status message" );
					temp.append( CR );
					LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
					if ( pLogin )
						temp.append( pLogin->toString() ).append( CR );
					temp.append( "State: " ).append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
				}

				if (pChannel->getConsumerRoutingChannel() == NULL)
					_ommBaseImpl.setState(OmmBaseImpl::LoginStreamOpenOkEnum);
				else
				{
					pChannel->getConsumerRoutingChannel()->channelState = OmmBaseImpl::LoginStreamOpenOkEnum;
					int loginOkCount = 0;
					ConsumerRoutingSession* pSession = pChannel->getConsumerRoutingChannel()->pRoutingSession;

					// Check to see if all of the channels are in a LoginStreamOpenOk status.  If they are all in that status, set the ommbaseimpl state to LoginStreamOpenOk so it can transition to the next step.
					if (_ommBaseImpl.isInitialized() == false)
					{
						for (UInt32 i = 0; i < pSession->routingChannelList.size(); i++)
						{
							// there's a potential that another routing channel has progressed past the login stream, so count that as ok
							if (pSession->routingChannelList[i] != NULL && pSession->routingChannelList[i]->channelState >= OmmBaseImpl::LoginStreamOpenOkEnum)
								loginOkCount++;
						}

						if (loginOkCount == pSession->activeChannelCount)
							_ommBaseImpl.setState( OmmBaseImpl::LoginStreamOpenOkEnum );
					}
				}
			}
		}
		else
		{
			if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received RDMLogin status message without the state" );
				LoginRdmRefreshMsgImpl* pLogin = (pChannel->getConsumerRoutingChannel() == NULL) ? &_loginInfo.loginRefreshMsg : &pChannel->getConsumerRoutingChannel()->loginInfo.loginRefreshMsg;
				if ( pLogin )
					temp.append( CR ).append( pLogin->toString() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}
		}

		if (setToOpenOk == true)
		{
			pLoginMsg->status.state.streamState = RSSL_STREAM_OPEN;
			pLoginMsg->status.state.dataState = RSSL_DATA_OK;
		}

		_loginItemLock.lock();

		if (dispatchMsgToUser && _loginItems.size() )
			processStatusMsg( pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent );

		_loginItemLock.unlock();

		if (closeChannel)
		{
			if (pChannel->getConsumerRoutingChannel() == NULL)
			{
				_ommBaseImpl.unsetActiveRsslReactorChannel(pChannel);
				_ommBaseImpl.closeChannel(pRsslReactorChannel);
			}
			else
			{
				pChannel->getConsumerRoutingChannel()->closeOnDownReconnecting = true;

				if (pChannel->getConsumerRoutingChannel()->pRoutingSession->activeChannelCount == 0)
					_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);
			}
		}

		break;
	}
	case RDM_LG_MT_RTT:
	{
		_loginItemLock.lock();

		if (_loginItems.size())
			processGenericMsg(pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent);

		_loginItemLock.unlock();

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

	RsslUInt32 majorVersion = RSSL_RWF_MAJOR_VERSION;
	RsslUInt32 minorVersion = RSSL_RWF_MINOR_VERSION;

	if (pRsslReactorChannel->majorVersion != 0)
	{
		majorVersion = pRsslReactorChannel->majorVersion;
	}

	if (pRsslReactorChannel->minorVersion != 0)
	{
		minorVersion = pRsslReactorChannel->minorVersion;
	}

	if ( pRsslMsg )
	{
		StaticDecoder::setRsslData( &_refreshMsg, pRsslMsg,
		                            majorVersion,
		                            minorVersion,
		                            0 );
	}
	else
	{
		if ( !convertRdmLoginToRsslBuffer( pRsslReactorChannel, pEvent, &rsslMsgBuffer ) )
			return RSSL_RC_CRET_SUCCESS;

		StaticDecoder::setRsslData( &_refreshMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            majorVersion,
		                            minorVersion,
		                            0 );
	}

	// Fan out the refresh to all registered handles
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

	RsslUInt32 majorVersion = RSSL_RWF_MAJOR_VERSION;
	RsslUInt32 minorVersion = RSSL_RWF_MINOR_VERSION;

	if (pRsslReactorChannel->majorVersion != 0)
	{
		majorVersion = pRsslReactorChannel->majorVersion;
	}

	if (pRsslReactorChannel->minorVersion != 0)
	{
		minorVersion = pRsslReactorChannel->minorVersion;
	}

	if ( pRsslMsg )
	{
		StaticDecoder::setRsslData( &_statusMsg, pRsslMsg,
									majorVersion,
		                            minorVersion,
		                            0 );
	}
	else
	{
		if ( !convertRdmLoginToRsslBuffer( pRsslReactorChannel, pEvent, &rsslMsgBuffer ) )
			return RSSL_RC_CRET_SUCCESS;

		StaticDecoder::setRsslData( &_statusMsg, &rsslMsgBuffer, RSSL_DT_MSG,
									majorVersion,
		                            minorVersion,
		                            0 );
	}

	// Fan out the status msg to all registered handles
	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->setEventChannel((void*)pRsslReactorChannel);
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

	RsslUInt32 majorVersion = RSSL_RWF_MAJOR_VERSION;
	RsslUInt32 minorVersion = RSSL_RWF_MINOR_VERSION;

	if (pRsslReactorChannel->majorVersion != 0)
	{
		majorVersion = pRsslReactorChannel->majorVersion;
	}

	if (pRsslReactorChannel->minorVersion != 0)
	{
		minorVersion = pRsslReactorChannel->minorVersion;
	}

	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
	                            majorVersion,
	                            minorVersion,
								channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	// Fan out the generic msg to all registered handles
	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->setEventChannel((void*)pRsslReactorChannel);
		item->onAllMsg( _genericMsg );
		item->onGenericMsg( _genericMsg );
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet LoginCallbackClient::processAckMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	RsslUInt32 majorVersion = RSSL_RWF_MAJOR_VERSION;
	RsslUInt32 minorVersion = RSSL_RWF_MINOR_VERSION;

	if (pRsslReactorChannel->majorVersion != 0)
	{
		majorVersion = pRsslReactorChannel->majorVersion;
	}

	if (pRsslReactorChannel->minorVersion != 0)
	{
		minorVersion = pRsslReactorChannel->minorVersion;
	}

	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
								majorVersion,
	                            minorVersion,
								channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	// Sets serviceName on received AckMsg assuming serviceId exists
	if ( pRsslMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) 
	{
		if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
		{
			Directory** pDirectoryPtr = channel->getConsumerRoutingChannel()->serviceById.find(pRsslMsg->msgBase.msgKey.serviceId);

			if (pDirectoryPtr != NULL)
			{
				Directory* pDirectory = *pDirectoryPtr;
				_ackMsg.getDecoder().setServiceName(pDirectory->getName().c_str(), pDirectory->getName().length());

				if (pDirectory->hasGeneratedServiceId())
				{
					_ackMsg.getDecoder().setServiceId((UInt16)pDirectory->getGeneratedServiceId());
				}
			}
		}
		else
		{
			const Directory* pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory(pRsslMsg->msgBase.msgKey.serviceId);
			if (pDirectory)
			{
				_ackMsg.getDecoder().setServiceName(pDirectory->getName().c_str(), pDirectory->getName().length());
			}
		}
	}

	for ( UInt32 idx = 0; idx < _loginItems.size(); ++idx )
	{
		_ommBaseImpl.msgDispatched();
		Item* item = _loginItems[idx];
		item->setEventChannel((void*)pRsslReactorChannel);
		item->onAllMsg( _ackMsg );
		item->onAckMsg( _ackMsg );
	}

	return RSSL_RC_CRET_SUCCESS;
}

void LoginCallbackClient::processChannelEvent( RsslReactorChannelEvent* pEvent )
{
	if (_refreshReceived == false)
		return;

	RsslReactorChannel* pReactorChannel = pEvent->pReactorChannel;
	Channel* pChannel = (Channel*)pEvent->pReactorChannel->userSpecPtr;

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

		_loginInfo.loginRefreshMsg.populate(rsslStatusMsg, temp);

		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = ( char* )"channel up";
		rsslStatusMsg.state.text.length = 10;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*) &rsslStatusMsg,
			RSSL_RWF_MAJOR_VERSION,
			RSSL_RWF_MINOR_VERSION,
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

		_loginInfo.loginRefreshMsg.populate(rsslStatusMsg, temp);

		rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = (char*)"channel down";
		rsslStatusMsg.state.text.length = 12;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*)&rsslStatusMsg,
			RSSL_RWF_MAJOR_VERSION,
			RSSL_RWF_MINOR_VERSION,
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

		_loginInfo.loginRefreshMsg.populate(rsslStatusMsg, temp);

		rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
		rsslStatusMsg.state.streamState = RSSL_STREAM_CLOSED;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
		rsslStatusMsg.state.text.data = (char*)"channel closed";
		rsslStatusMsg.state.text.length = 14;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData( &_statusMsg, (RsslMsg*)&rsslStatusMsg,
			RSSL_RWF_MAJOR_VERSION, 
			RSSL_RWF_MINOR_VERSION,
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
	case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
	{
		RsslStatusMsg rsslStatusMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_loginInfo.loginRefreshMsg.populate(rsslStatusMsg, temp);

		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = OmmState::StatusCode::SocketPHComplete;
		rsslStatusMsg.state.text.data = (char*)"Preferred host complete";
		rsslStatusMsg.state.text.length = 23;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData(&_statusMsg, (RsslMsg*)&rsslStatusMsg,
			RSSL_RWF_MAJOR_VERSION,
			RSSL_RWF_MINOR_VERSION,
			0);

		for (UInt32 idx = 0; idx < _loginItems.size(); ++idx)
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];

			item->setEventChannel(pEvent->pReactorChannel);
			item->onAllMsg(_statusMsg);
			item->onStatusMsg(_statusMsg);
		}
	}
	break;
	case RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK:
	{
		RsslStatusMsg rsslStatusMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_loginInfo.loginRefreshMsg.populate(rsslStatusMsg, temp);

		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.streamState = RSSL_STREAM_OPEN;
		rsslStatusMsg.state.code = OmmState::StatusCode::SocketPHStartingFallback;
		rsslStatusMsg.state.text.data = (char*)"Preferred host starting fallback";
		rsslStatusMsg.state.text.length = 32;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;

		StaticDecoder::setRsslData(&_statusMsg, (RsslMsg*)&rsslStatusMsg,
			RSSL_RWF_MAJOR_VERSION,
			RSSL_RWF_MINOR_VERSION,
			0);

		for (UInt32 idx = 0; idx < _loginItems.size(); ++idx)
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];

			item->setEventChannel(pEvent->pReactorChannel);
			item->onAllMsg(_statusMsg);
			item->onStatusMsg(_statusMsg);
		}
	}
	break;
	default :
		break;
	}
}

bool LoginCallbackClient::convertRdmLoginToRsslBuffer( RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* pEvent, RsslBuffer* pRsslMsgBuffer )
{
	if ( !pRsslReactorChannel || !pEvent || !pRsslMsgBuffer ) return false;

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

LoginItem* LoginCallbackClient::getLoginItem( const ReqMsg&, OmmConsumerClient& ommConsClient, void* closure )
{
	LoginItem* li = LoginItem::create( _ommBaseImpl, ommConsClient, closure );

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
	NiProviderLoginItem* li = NiProviderLoginItem::create( _ommBaseImpl, ommProvClient, closure );

	if ( li )
	{
		li->setEventChannel(_ommBaseImpl.getRsslReactorChannel());

		_loginItems.push_back(li);
		if (_refreshReceived == true)
		{
			NiProviderLoginItemCreationCallbackStruct* liccs(new NiProviderLoginItemCreationCallbackStruct(this, li));
			TimeOut* timeOut = new TimeOut(_ommBaseImpl, 10, LoginCallbackClient::handleLoginItemCallback, liccs, true);
		}
		
	}

	return li;
}

LoginInfo& LoginCallbackClient::getLoginInfo()
{
	return _loginInfo;
}

const EmaString& LoginCallbackClient::getLoginFailureMessage()
{
	return _loginFailureMsg;
}

EmaVector< Item* >& LoginCallbackClient::getLoginItems()
{
	return _loginItems;
}

LoginItem* LoginItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure)
{
	try
	{
		return new LoginItem( ommBaseImpl, ommConsClient, closure );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create LoginItem" );
	}

	return NULL;
}

LoginItem::LoginItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure) :
	SingleItem( ommBaseImpl, ommConsClient, closure, 0 )
{
	_streamId = EMA_LOGIN_STREAM_ID;
}

LoginItem::~LoginItem()
{
}

bool LoginItem::open( RsslRDMLoginRequest* rsslRdmLoginRequest)
{
	bool retCode = true;

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
	bool ret = true;

	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{

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

		ret = submit(_ommBaseImpl.getLoginCallbackClient().getLoginRequest(), _ommBaseImpl.getRsslReactorChannel());
	/* Unset the Pause and No Refresh flag on the cached request */
	_ommBaseImpl.getLoginCallbackClient().getLoginRequest()->flags &= ~(RDM_LG_RQF_PAUSE_ALL | RDM_LG_RQF_NO_REFRESH);
	}
	else
	{
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		if (rsslDecodeRDMLoginMsg(&dIter, (RsslMsg*)(static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder())).getRsslRequestMsg(), (RsslRDMLoginMsg*)&tempRequest, &tmpBuf, &errorInfo) != RSSL_RET_SUCCESS)
		{
			EmaString temp("Internal error: rsslDecodeRDMLoginMsg failed.");
			temp.append(CR).append("Error Location ").append(errorInfo.errorLocation).append(CR)
				.append("Error Text ").append(errorInfo.rsslError.text);
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		// If the multi credential logins have been set, we will check if there is a PAUSE_ALL or interestAfterRefresh set indicating Pause/Resume all.
		// In this case, we will just send the current request out to the wire with the pause message.
		if (_ommBaseImpl.hasMultiCredentialLogins() == true)
		{
			if (tempRequest.flags & RDM_LG_RQF_PAUSE_ALL)
			{
				for (UInt32 i = 0; i < _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++i)
				{
					ConsumerRoutingSessionChannel* routingChannel = _ommBaseImpl.getConsumerRoutingSession()->routingChannelList[i];
					// routingChannel should never be null
					if (routingChannel->pReactorChannel != NULL)
					{
						RsslRDMLoginRequest* pLoginRdmMsg = routingChannel->loginInfo.pLoginRequestMsg->get();

						pLoginRdmMsg->flags |= RDM_LG_RQF_PAUSE_ALL;

						ret = submit(pLoginRdmMsg, routingChannel->pReactorChannel);
					}
				}
			}
			else if (reqMsg.getInterestAfterRefresh() == true)
			{
				for (UInt32 i = 0; i < _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++i)
				{
					ConsumerRoutingSessionChannel* routingChannel = _ommBaseImpl.getConsumerRoutingSession()->routingChannelList[i];
					// routingChannel should never be null
					if (routingChannel->pReactorChannel != NULL)
					{
						LoginInfo& loginInfo = routingChannel->loginInfo;

						loginInfo.pLoginRequestMsg->overlay((RsslRDMLoginRequest*)&tempRequest);

						ret = submit(loginInfo.pLoginRequestMsg->get(), routingChannel->pReactorChannel);
					}
				}
			}
			else
			{
				EmaString text("Unsupported login message submitted with Multicredential login messages configured.  Only supported login messages are pause and resume.");
				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}
		}
		else
		{
			// When multicredentials are not turned on, fanout the request to each channel after overlaying the incoming request to the channel
			for (UInt32 i = 0; i < _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++i)
			{
				ConsumerRoutingSessionChannel* routingChannel = _ommBaseImpl.getConsumerRoutingSession()->routingChannelList[i];
				// routingChannel should never be null
				if (routingChannel->pReactorChannel != NULL)
				{
					LoginInfo& loginInfo = routingChannel->loginInfo;

					loginInfo.pLoginRequestMsg->overlay((RsslRDMLoginRequest*)&tempRequest);

					ret = submit(loginInfo.pLoginRequestMsg->get(), routingChannel->pReactorChannel);
				}
			}
		}
	}

	return ret;

}

bool LoginItem::submit( const PostMsg& postMsg )
{
	const PostMsgEncoder& postMsgEncoder = static_cast<const PostMsgEncoder&>( postMsg.getEncoder() );

	RsslPostMsg* pRsslPostMsg = postMsgEncoder.getRsslPostMsg();

	RsslBuffer serviceNameBuffer;
	const Directory* pDirectory = NULL;

	if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
	{
		if ( postMsgEncoder.hasServiceName() )
		{
			const EmaString& serviceName = postMsgEncoder.getServiceName();

			serviceNameBuffer.data = (char*)serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();
			// Request routing is turned on, so find the matching service name and route it to there
			ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceByName.find(&postMsgEncoder.getServiceName());
			if (!pRoutingServicePtr)
			{
				EmaString text("Failed to submit PostMsg on login stream. Reason: No available service");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}

			ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

			if (pRoutingService->isDeleted())
			{
				EmaString text("Failed to submit PostMsg on login stream. Reason: No available service");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}

			for (UInt32 i = 0; i < pRoutingService->routingChannelList.size(); i++)
			{
				if (pRoutingService->routingChannelList[i] != NULL && pRoutingService->routingChannelList[i]->pReactorChannel != NULL)
				{
					if (submit(pRsslPostMsg, &serviceNameBuffer, pRoutingService->routingChannelList[i]->pReactorChannel) == false)
					{
						return false;
					}
				}
			}

			return true;
		}
		else
		{
			// Off stream post requires a service id otherwise.
			if ((pRsslPostMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID) == 0)
			{
				EmaString text("Failed to submit PostMsg on login stream. Reason: no specified service name or service id");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}


			// Request routing is turned on, so find the matching service name and route it to there
			ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(pRsslPostMsg->msgBase.msgKey.serviceId);
			if (!pRoutingServicePtr)
			{
				EmaString text("Failed to submit PostMsg on login stream. Reason: invalid service id");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}

			UInt16 oldServiceId = pRsslPostMsg->msgBase.msgKey.serviceId;

			ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

			if (pRoutingService->isDeleted())
			{
				EmaString text("Failed to submit PostMsg on login stream. Reason: No available service");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}

			const EmaString& serviceName = pRoutingService->getName();

			serviceNameBuffer.data = (char*)serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();

			pRsslPostMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

			// Submit the request with the service name specified.  This will overwrite any serviceId set in the request.
			for (UInt32 i = 0; i < pRoutingService->routingChannelList.size(); i++)
			{
				if (pRoutingService->routingChannelList[i] != NULL && pRoutingService->routingChannelList[i]->pReactorChannel != NULL)
				{
					if (submit(pRsslPostMsg, &serviceNameBuffer, pRoutingService->routingChannelList[i]->pReactorChannel) == false)
					{
						return false;
					}
				}
			}

			return true;
		}
	}
	/* if the PostMsg has the Service Name */
	else if (postMsgEncoder.hasServiceName())
	{
		pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory(postMsgEncoder.getServiceName());
		if (!pDirectory)
		{
			EmaString temp("Failed to submit PostMsg on item stream. Reason: Service name of '");
			temp.append(postMsgEncoder.getServiceName()).append("' is not found.");

			_ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);

			return false;
		}

		const EmaString& serviceName = postMsgEncoder.getServiceName();
			
		serviceNameBuffer.data = (char*) serviceName.c_str();
		serviceNameBuffer.length = serviceName.length();
			
		return submit(pRsslPostMsg, &serviceNameBuffer, _ommBaseImpl.getRsslReactorChannel());

	}

	return submit( static_cast<const PostMsgEncoder&>( postMsg.getEncoder() ).getRsslPostMsg(), NULL, _ommBaseImpl.getRsslReactorChannel());
}

bool LoginItem::submit( const GenericMsg& genMsg )
{
	const GenericMsgEncoder& genericMsgEncoder = static_cast<const GenericMsgEncoder&>(genMsg.getEncoder());

	// For request routing, fan this out to all the connected providers.  These currently should not be seent by EMA, as the Reactor will handle sending RTT responses automatically.
	if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
	{
		ConsumerRoutingService** pRoutingServicePtr = NULL;
		if (genericMsgEncoder.hasServiceId())
		{
			pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(genericMsgEncoder.getRsslGenericMsg()->msgBase.msgKey.serviceId);
		}

		for (UInt32 i = 0; i < _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++i)
		{
			ConsumerRoutingSessionChannel* routingChannel = _ommBaseImpl.getConsumerRoutingSession()->routingChannelList[i];
			// routingChannel should never be null
			if (routingChannel->pReactorChannel != NULL)
			{	
				if (genericMsgEncoder.hasServiceId())
				{
					if (!pRoutingServicePtr)
					{
						return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), routingChannel->pReactorChannel);
					}

					ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

					// If the channel is in the routing service list, substitute the found serviceId here.
					if (pRoutingService->routingChannelList[sessionChannel->sessionIndex] != NULL)
					{
						Directory** pDirectoryPtr = sessionChannel->serviceByName.find(&pRoutingService->getName());

						if (pDirectoryPtr == NULL)
						{
							return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), routingChannel->pReactorChannel);
						}

						Directory* pDirectory = *pDirectoryPtr;

						genericMsgEncoder.getRsslGenericMsg()->msgBase.msgKey.serviceId = (RsslUInt16)pDirectory->getService()->serviceId;

						return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), routingChannel->pReactorChannel);
					}
					else
					{
						return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), routingChannel->pReactorChannel);
					}
				}
				else
					return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), routingChannel->pReactorChannel);

			}
}

		return true;
	}
	else
	{
		return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg(), _ommBaseImpl.getRsslReactorChannel());
	}
}

bool LoginItem::submit( RsslRDMLoginRequest* pRsslRequestMsg, RsslReactorChannel* pChannel )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslRequestMsg->rdmMsgBase.streamId = _streamId;

		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRDMMsg = ( RsslRDMMsg* )pRsslRequestMsg;

	submitMsgOpts.majorVersion = pChannel->majorVersion;
	submitMsgOpts.minorVersion = pChannel->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
										pChannel,
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

	return true;
}

bool LoginItem::submit( RsslGenericMsg* pRsslGenericMsg, RsslReactorChannel* pReactorChannel )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslGenericMsg->msgBase.streamId = _streamId;

	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslGenericMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;

	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
			pReactorChannel, &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
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

	return true;
}

bool LoginItem::submit( RsslPostMsg* pRsslPostMsg, RsslBuffer* pServiceName, RsslReactorChannel* pReactorChannel )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslPostMsg->msgBase.streamId = _streamId;

		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pServiceName = pServiceName;
		submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslPostMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;

	if ((ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
		pReactorChannel, &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
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

	return true;
}

void LoginCallbackClient::sendInternalMsg( LoginItem* loginItem )
{
	RsslRefreshMsg rsslRefreshMsg;
	char tempBuffer[1000];
	RsslBuffer temp;
	temp.data = tempBuffer;
	temp.length = 1000;

	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		_loginInfo.loginRefreshMsg.populate(rsslRefreshMsg, temp);
	}
	else
	{
		_ommBaseImpl.getConsumerRoutingSession()->aggregatedLoginInfo.loginRefreshMsg.populate(rsslRefreshMsg, temp);
	}

	StaticDecoder::setRsslData( &_refreshMsg, reinterpret_cast< RsslMsg* >( &rsslRefreshMsg ),
								RSSL_RWF_MAJOR_VERSION,
								RSSL_RWF_MINOR_VERSION,
	                            0 );

	_ommBaseImpl.msgDispatched();
	Item* item = static_cast< Item* >( loginItem );
	// This is an internally generated event, so there may not be an associated reactor channel
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		item->setEventChannel(_ommBaseImpl.getRsslReactorChannel());
	}
	else
	{
		item->setEventChannel(NULL);
	}

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

// This will re-aggregate logins after close, and if necessary, fan out the appropriate refresh or status messages.
void LoginCallbackClient::aggregateLoginsAfterClose()
{
	// Do nothing if there aren't any active channels or if all channels are currently reconnecting
	if (_ommBaseImpl.getConsumerRoutingSession()->activeChannelCount == 0)
		return;

	int reconnectionCount = 0;
	for (UInt32 i = 0; i < _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++i)
	{
		if (_ommBaseImpl.getConsumerRoutingSession()->routingChannelList[i]->reconnecting == true)
			reconnectionCount++;
	}

	if (reconnectionCount == _ommBaseImpl.getConsumerRoutingSession()->activeChannelCount)
	{
		return;
	}

	// First, aggregate the requests for all channels with a currently active login
	bool loginAggregateChanged = _ommBaseImpl.getConsumerRoutingSession()->aggregateLoginRefreshInfo(NULL);

	// If the aggregation changed, submit it to the user.
	if (loginAggregateChanged == true)
	{
		RsslRefreshMsg rsslRefreshMsg;
		char tempBuffer[1000];
		RsslBuffer temp;
		temp.data = tempBuffer;
		temp.length = 1000;

		_ommBaseImpl.getConsumerRoutingSession()->aggregatedLoginInfo.loginRefreshMsg.populate(rsslRefreshMsg, temp, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		StaticDecoder::setRsslData(&_refreshMsg, reinterpret_cast<RsslMsg*>(&rsslRefreshMsg),
			RSSL_RWF_MAJOR_VERSION,
			RSSL_RWF_MINOR_VERSION,
			0);

		// Fan out the refresh to all registered handles
		for (UInt32 idx = 0; idx < _loginItems.size(); ++idx)
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItems[idx];
			item->setEventChannel(NULL);

			item->onAllMsg(_refreshMsg);
			item->onRefreshMsg(_refreshMsg);
		}

		if (_refreshMsg.getState().getStreamState() != OmmState::OpenEnum)
		{
			for (UInt32 idx = 0; idx < _loginItems.size(); ++idx)
				_loginItems[idx]->remove();

			_loginItems.clear();
		}
	}
}

NiProviderLoginItem* NiProviderLoginItem::create( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure )
{
	try
	{
		return new NiProviderLoginItem( ommBaseImpl, ommProvClient, closure );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create NiProviderLoginItem" );
	}

	return NULL;
}

NiProviderLoginItem::NiProviderLoginItem( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure ) :
	NiProviderSingleItem( ommBaseImpl, ommProvClient, 0, closure, 0 )
{
	_domainType = MMT_LOGIN;
	_streamId = EMA_LOGIN_STREAM_ID;
}

NiProviderLoginItem::~NiProviderLoginItem()
{
}

bool NiProviderLoginItem::open( RsslRDMLoginRequest* rsslRdmLoginRequest)
{
	bool retCode = true;

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
	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRDMMsg = (RsslRDMMsg*) pRsslRequestMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _ommBaseImpl.getRsslReactor(),
		pReactorChannel,
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

	return true;
}

bool NiProviderLoginItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslGenericMsg->msgBase.streamId = _streamId;
	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslGenericMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet ret;
	if ((ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
		pReactorChannel,
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

	return true;
}

bool NiProviderLoginItem::submit( RsslPostMsg* )
{
	return false;
}


LoginInfo::LoginInfo() :
	pLoginRequestMsg(0),
	loginRefreshMsg()
{

}

LoginInfo::~LoginInfo()
{

}