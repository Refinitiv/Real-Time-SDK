/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "LoginCallbackClient.h"
#include "OmmBaseImpl.h"
#include "StaticDecoder.h"
#include "OmmState.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "PostMsgEncoder.h"
#include "Utilities.h"

#include <new>

using namespace thomsonreuters::ema::access;

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
	_stateSet( false )
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
		_toString.set( "username " ).append( _usernameSet ? _username : "<not set>" ).append( CR )
			.append( "usernameType " ).append( _userNameType ).append( CR )
			.append( "position " ).append( _positionSet ? _position : "<not set>" ).append( CR )
			.append( "appId " ).append( _applicationIdSet ? _applicationId : "<not set>" ).append( CR )
			.append( "applicationName " ).append( _applicationNameSet ? _applicationName : "<not set>" ).append( CR )
			.append( "instanceId " ).append( _instanceIdSet ? _instanceId : "<not set>" ).append( CR )
			.append( "singleOpen " ).append( _singleOpen ).append( CR )
			.append( "allowSuspect " ).append( _allowSuspect ).append( CR )
			.append( "optimizedPauseResume " ).append( _pauseResume ).append( CR )
			.append( "permissionExpressions " ).append( _permissionExpressions ).append( CR )
			.append( "permissionProfile " ).append( _permissionProfile ).append( CR )
			.append( "supportBatchRequest " ).append( _supportBatchRequest ).append( CR )
			.append( "supportEnhancedSymbolList " ).append( _supportEnhancedSymbolList ).append( CR )
			.append( "supportPost " ).append( _supportPost ).append( CR )
			.append( "supportViewRequest " ).append( _supportViewRequest )
			.append( "role " ).append( _role );
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

	if ( pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_ID )
	{
		_applicationId.set( pRequest->applicationId.data, pRequest->applicationId.length );
		_applicationIdSet = true;
	}
	else
		_applicationIdSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_APPLICATION_NAME )
	{
		_applicationName.set( pRequest->applicationName.data, pRequest->applicationName.length );
		_applicationNameSet = true;
	}
	else
		_applicationNameSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_INSTANCE_ID )
	{
		_instanceId.set( pRequest->instanceId.data, pRequest->instanceId.length );
		_instanceIdSet = true;
	}
	else
		_instanceIdSet = false;

	if ( pRequest->flags & RDM_LG_RQF_HAS_POSITION )
	{
		_position.set( pRequest->position.data, pRequest->position.length );
		_positionSet = true;
	}
	else
		_positionSet = false;

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

	retCode = rsslEncodeElementListComplete( &eIter, RSSL_TRUE );
	if ( retCode != RSSL_RET_SUCCESS )
		return false;

	buffer.length = rsslGetEncodedBufferLength( &eIter );

	refresh.msgBase.streamId = 1;
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

	status.msgBase.streamId = 1;
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

	rsslCloseMsg.msgBase.streamId = 1;
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
	_refreshMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommBaseImpl( ommBaseImpl ),
	_requestLogin( 0 ),
	_loginItems(),
	_loginItemsOnChannelDown(),
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
	_loginItemsOnChannelDown.clear();

	Login::destroy( _requestLogin );

	if ( _loginRequestBuffer )
		free( _loginRequestBuffer );

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

	if ( !_ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->userName.length )
	{
		RsslBuffer tempBuffer;
		tempBuffer.data = _ommBaseImpl.getActiveConfig().pRsslRDMLoginReq->defaultUsername;
		tempBuffer.length = 256;

		if ( RSSL_RET_SUCCESS != rsslGetUserName( &tempBuffer ) )
		{
			_ommBaseImpl.handleIue( "Failed to obtain name of the process owner" );
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
}

UInt32 LoginCallbackClient::sendLoginClose()
{
	return _loginList.sendLoginClose();
}

RsslRDMLoginRequest* LoginCallbackClient::getLoginRequest()
{
	return &_loginRequestMsg;
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
			pLogin = Login::create( _ommBaseImpl );
			_loginList.addLogin( pLogin );
		}

		pLogin->set( &pLoginMsg->refresh ).setChannel( ( Channel* )( pRsslReactorChannel->userSpecPtr ) );
		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->setLogin( pLogin );

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

		_loginItemLock.lock();

		if ( _loginItems.size() )
			processRefreshMsg( pEvent->baseMsgEvent.pRsslMsg, pRsslReactorChannel, pEvent );

		_loginItemLock.unlock();

		if ( closeChannel )
			_ommBaseImpl.closeChannel( pRsslReactorChannel );

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

		if ( closeChannel )
			_ommBaseImpl.closeChannel( pRsslReactorChannel );

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
	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
	                            pRsslReactorChannel->majorVersion,
	                            pRsslReactorChannel->minorVersion,
	                            static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

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
	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
	                            pRsslReactorChannel->majorVersion,
	                            pRsslReactorChannel->minorVersion,
	                            static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

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
		_ommBaseImpl.reLoadDirectory();

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

		for ( UInt32 idx = 0; idx < _loginItemsOnChannelDown.size(); ++idx )
		{
			_ommBaseImpl.msgDispatched();
			Item* item = _loginItemsOnChannelDown[idx];
			item->onAllMsg( _statusMsg );
			item->onStatusMsg( _statusMsg );

			_loginItems.push_back( item );
		}

		_loginItemsOnChannelDown.clear();
	}
	break;
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING :
	{
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
			item->onAllMsg( _statusMsg );
			item->onStatusMsg( _statusMsg );

			_loginItemsOnChannelDown.push_back( item );
		}

		_loginItems.clear();
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
		_ommBaseImpl.handleIue( "Internal error. Failed to set encode iterator version in LoginCallbackClient::convertRdmLoginToRsslBuffer()" );
		return false;
	}

	retCode = rsslSetEncodeIteratorBuffer( &eIter, pRsslMsgBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( pRsslMsgBuffer->data );
		_ommBaseImpl.handleIue( "Internal error. Failed to set encode iterator buffer in LoginCallbackClient::convertRdmLoginToRsslBuffer()" );
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
	LoginItem* li = LoginItem::create( _ommBaseImpl, ommConsClient, closure, _loginList );

	if ( li )
	{
		LoginItemCreationCallbackStruct* liccs( new LoginItemCreationCallbackStruct( this, li ) );
		TimeOut* timeOut = new TimeOut( _ommBaseImpl, 10, LoginCallbackClient::handleLoginItemCallback, liccs, true );
		_loginItems.push_back( li );
	}

	return li;
}

NiProviderLoginItem* LoginCallbackClient::getLoginItem( const ReqMsg&, OmmProviderClient& ommProvClient, void* closure )
{
	NiProviderLoginItem* li = NiProviderLoginItem::create( _ommBaseImpl, ommProvClient, closure, _loginList );

	if ( li )
	{
		NiProviderLoginItemCreationCallbackStruct* liccs( new NiProviderLoginItemCreationCallbackStruct( this, li ) );
		TimeOut* timeOut = new TimeOut( _ommBaseImpl, 10, LoginCallbackClient::handleLoginItemCallback, liccs, true );
		_loginItems.push_back( li );
	}

	return li;
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
	_streamId = 1;
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
	return submit( static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getRsslRequestMsg() );
}

bool LoginItem::submit( const PostMsg& postMsg )
{
	return submit( static_cast<const PostMsgEncoder&>( postMsg.getEncoder() ).getRsslPostMsg() );
}

bool LoginItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

bool LoginItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslRequestMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslRequestMsg;

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

			_ommBaseImpl.handleIue( text );

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

			_ommBaseImpl.handleIue( text );

			return false;
		}
	}

	return true;
}

bool LoginItem::submit( RsslPostMsg* pRsslPostMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslPostMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

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

			_ommBaseImpl.handleIue( text );

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
	NiProviderSingleItem( ommBaseImpl, ommProvClient, closure, 0 ),
	_loginList( &loginList )
{
	_streamId = 1;
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

bool NiProviderLoginItem::modify( const ReqMsg& )
{
	return false;
}

bool NiProviderLoginItem::submit( const PostMsg& )
{
	return false;
}

bool NiProviderLoginItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

bool NiProviderLoginItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	pRsslRequestMsg->msgBase.streamId = _streamId;

	UInt32 size = _loginList->size();
	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

		submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslRequestMsg;

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

			_ommBaseImpl.handleIue( text );

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

			_ommBaseImpl.handleIue( text );

			return false;
		}
	}

	return true;
}

bool NiProviderLoginItem::submit( RsslPostMsg* )
{
	return false;
}
