/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "LoginRefreshImpl.h"
#include "OmmInvalidUsageException.h"
#include <new>

using namespace thomsonreuters::ema::domain::login;
using namespace thomsonreuters::ema::rdm;
using namespace thomsonreuters::ema::access;

LoginRefreshImpl::LoginRefreshImpl() :
	_pElementList(0),
	_rsslState(new RsslState())
{
	clear();
}

LoginRefreshImpl::LoginRefreshImpl(const LoginRefreshImpl& other) :
	_pElementList(0),
	_rsslState(new RsslState())
{
	*this = other;
}

LoginRefreshImpl::LoginRefreshImpl(const RefreshMsg& refreshMsg) :
	_pElementList(0),
	_rsslState(new RsslState())
{
	clear();

	decode(refreshMsg);
}

LoginRefreshImpl::~LoginRefreshImpl()
{
	if (_pElementList)
	{
		delete _pElementList;
		_pElementList = 0;
	}
}

LoginRefreshImpl& LoginRefreshImpl::clear()
{
	_changed = true;
	_allowSuspectDataSet = true;
	_allowSuspectData = true;
	_providePermissionProfileSet = true;
	_providePermissionProfile = true;
	_providePermissionExpressionsSet = true;
	_providePermissionExpressions = true;
	_singleOpenSet = true;
	_singleOpen = true;
	_supportBatchRequestsSet = false;
	_supportBatchRequests = false;
	_supportOptimizedPauseResumeSet = false;
	_supportOptimizedPauseResume = false;
	_supportProviderDictionaryDownloadSet = false;
	_supportProviderDictionaryDownload = false;
	_applicationIdSet = false;
	_applicationNameSet = false;
	_positionSet = false;
	_solicitedSet = true;
	_clearCacheSet = false;
	_supportViewRequestsSet = false;
	_supportViewRequests = false;
	_supportStandbySet = false;
	_supportStandby = false;
	_supportOMMPostSet = false;
	_supportOMMPost = false;
	_supportEnhancedSymbolListSet = false;
	_supportEnhancedSymbolList = SUPPORT_SYMBOL_LIST_NAMES_ONLY;

	_authenticationExtendedSet = false;
	_authenticationTTReissueSet = false;
	_authenticationErrorCodeSet = false;
	_authenticationErrorTextSet = false;

	_solicited = true;
	_clearCache = false;
	_nameSet = false;
	_nameTypeSet = true;
	_stateSet = false;
	_seqNumSet = false;

	rsslClearState(_rsslState);

	_nameType = USER_NAME;
	_domainType = MMT_LOGIN;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::operator=(const LoginRefreshImpl& other)
{
	_changed = true;
	_allowSuspectDataSet = other._allowSuspectDataSet;
	_providePermissionProfileSet = other._providePermissionProfileSet;
	_providePermissionExpressionsSet = other._providePermissionExpressionsSet;
	_singleOpenSet = other._singleOpenSet;
	_supportBatchRequestsSet = other._supportBatchRequestsSet;
	_supportOptimizedPauseResumeSet = other._supportOptimizedPauseResumeSet;
	_supportProviderDictionaryDownloadSet = other._supportProviderDictionaryDownloadSet;
	_applicationIdSet = other._applicationIdSet;
	_applicationNameSet = other._applicationNameSet;
	_positionSet = other._positionSet;
	_supportViewRequestsSet = other._supportViewRequestsSet;
	_supportStandbySet = other._supportStandbySet;
	_supportOMMPostSet = other._supportOMMPostSet;
	_solicitedSet = other._solicitedSet;
	_clearCacheSet = other._clearCacheSet;
	_supportEnhancedSymbolListSet = other._supportEnhancedSymbolListSet;
	_authenticationExtendedSet = other._authenticationExtendedSet;
	_authenticationTTReissueSet = other._authenticationTTReissueSet;
	_authenticationErrorCodeSet = other._authenticationErrorCodeSet;
	_authenticationErrorTextSet = other._authenticationErrorTextSet;
	_nameSet = other._nameSet;
	_nameTypeSet = other._nameTypeSet;
	_stateSet = other._stateSet;
	_seqNumSet = other._seqNumSet;


	_allowSuspectData = other._allowSuspectData;
	_providePermissionProfile = other._providePermissionProfile;
	_providePermissionExpressions = other._providePermissionExpressions;
	_singleOpen = other._singleOpen;
	_supportBatchRequests = other._supportBatchRequests;
	_supportOptimizedPauseResume = other._supportOptimizedPauseResume;
	_supportProviderDictionaryDownload = other._supportProviderDictionaryDownload;
	_applicationId = other._applicationId;
	_applicationName = other._applicationName;
	_position = other._position;
	_supportViewRequests = other._supportViewRequests;
	_supportStandby = other._supportStandby;
	_supportOMMPost = other._supportOMMPost;
	_solicited = other._solicited;
	_clearCache = other._clearCache;
	_supportEnhancedSymbolList = other._supportEnhancedSymbolList;
	_authenticationExtended = other._authenticationExtended;
	_authenticationTTReissue = other._authenticationTTReissue;
	_authenticationErrorCode = other._authenticationErrorCode;
	_authenticationErrorText = other._authenticationErrorText;
	_name = other._name;
	_nameType = other._nameType;
	_rsslState = other._rsslState;
	_rsslState->streamState = other._state.getStreamState();
	_rsslState->dataState = other._state.getDataState();
	_rsslState->code = other._state.getCode();
	_rsslState->text.data = (char*)other._state.getStatusText().c_str();
	_rsslState->text.length = other._state.getStatusText().length();
	_stateText = other._stateText;
	_seqNum = other._seqNum;
	_domainType = other._domainType;
	 
	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::message(const RefreshMsg& refreshMsg)
{
	decode(refreshMsg);

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::allowSuspectData(bool value)
{
	_changed = true;
	_allowSuspectDataSet = true;
	_allowSuspectData = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::applicationId(const EmaString& value)
{
	_changed = true;
	_applicationIdSet = true;
	_applicationId = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::applicationName(const EmaString& value)
{
	_changed = true;
	_applicationNameSet = true;
	_applicationName = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::position(const EmaString& value)
{
	_changed = true;
	_positionSet = true;
	_position = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::providePermissionExpressions(bool value)
{
	_changed = true;
	_providePermissionExpressionsSet = true;
	_providePermissionExpressions = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::providePermissionProfile(bool value)
{
	_changed = true;
	_providePermissionProfileSet = true;
	_providePermissionProfile = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::singleOpen(bool value)
{
	_changed = true;
	_singleOpenSet = true;
	_singleOpen = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportBatchRequests(UInt64 value)
{
	bool valid = false;

	if (value <= (SUPPORT_BATCH_REQUEST | SUPPORT_BATCH_REISSUE | SUPPORT_BATCH_CLOSE) )
	{
		valid = true;
	}

	if (!valid)
	{
		EmaString text("Invalid value ");
		text.append(value).append(" for the ").append(ENAME_SUPPORT_BATCH).append(" element.");
		throwIueException( text, OmmInvalidUsageException::InvalidArgumentEnum );
	}

	_changed = true;
	_supportBatchRequestsSet = true;
	_supportBatchRequests = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportEnhancedSymbolList(UInt32 value)
{
	_changed = true;
	_supportEnhancedSymbolListSet = true;
	_supportEnhancedSymbolList = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportOMMPost(bool value)
{
	_changed = true;
	_supportOMMPostSet = true;
	_supportOMMPost = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportOptimizedPauseResume(bool value)
{
	_changed = true;
	_supportOptimizedPauseResumeSet = true;
	_supportOptimizedPauseResume = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportProviderDictionaryDownload(bool value)
{
	_changed = true;
	_supportProviderDictionaryDownloadSet = true;
	_supportProviderDictionaryDownload = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportViewRequests(bool value)
{
	_changed = true;
	_supportViewRequestsSet = true;
	_supportViewRequests = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::supportStandby(bool value)
{
	_changed = true;
	_supportStandbySet = true;
	_supportStandby = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::solicited(bool value)
{
	_changed = true;
	_solicitedSet = true;
	_solicited = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::clearCache(bool value)
{
	_changed = true;
	_clearCacheSet = true;
	_clearCache = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::authenticationExtended(const EmaBuffer& value)
{
	_changed = true;
	_authenticationExtendedSet = true;
	_authenticationExtended = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::authenticationTTReissue(const UInt64& value)
{
	_changed = true;
	_authenticationTTReissueSet = true;
	_authenticationTTReissue = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::authenticationErrorCode(const UInt64& value)
{
	_changed = true;
	_authenticationErrorCodeSet = true;
	_authenticationErrorCode = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::authenticationErrorText(const EmaString& value)
{
	_changed = true;
	_authenticationErrorTextSet = true;
	_authenticationErrorText = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::name(const EmaString& value)
{
	_changed = true;
	_nameSet = true;
	_name = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::nameType(const UInt32& value)
{
	_changed = true;
	_nameTypeSet = true;
	_nameType = value;

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::state(const OmmState::StreamState& streamState, const OmmState::DataState dataState, const UInt8& statusCode, const EmaString& statusText)
{
	_changed = true;
	_stateSet = true;

	_rsslState->streamState = streamState;
	_rsslState->dataState = dataState;
	_rsslState->code = statusCode;
	_rsslState->text.data = (char*)statusText.c_str();
	_rsslState->text.length = statusText.length();
	_stateText.setInt(_rsslState->text.data, _rsslState->text.length, false);

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::state(const OmmState& state)
{
	_changed = true;
	_stateSet = true;

	_rsslState->streamState = state.getStreamState();
	_rsslState->dataState = state.getDataState();
	_rsslState->code = (UInt8)state.getStatusCode();
	_rsslState->text.data = (char*)state.getStatusText().c_str();
	_rsslState->text.length = state.getStatusText().length();
	_stateText.setInt(_rsslState->text.data, _rsslState->text.length, false);

	return *this;
}

LoginRefreshImpl& LoginRefreshImpl::seqNum(const UInt32& value)
{
	_changed = true;
	_seqNumSet = true;
	_seqNum = value;

	return *this;
}

bool LoginRefreshImpl::hasAllowSuspectData() const
{
	return _allowSuspectDataSet;
}

bool LoginRefreshImpl::hasApplicationId() const
{
	return _applicationIdSet;
}

bool LoginRefreshImpl::hasApplicationName() const
{
	return _applicationNameSet;
}

bool LoginRefreshImpl::hasPosition() const
{
	return _positionSet;
}

bool LoginRefreshImpl::hasProvidePermissionExpressions() const
{
	return _providePermissionExpressionsSet;
}

bool LoginRefreshImpl::hasProvidePermissionProfile() const
{
	return _providePermissionProfileSet;
}

bool LoginRefreshImpl::hasSingleOpen() const
{
	return _singleOpenSet;
}

bool LoginRefreshImpl::hasSupportBatchRequests() const
{
	return _supportBatchRequestsSet;
}

bool LoginRefreshImpl::hasSupportEnhancedSymbolList() const
{
	return _supportEnhancedSymbolListSet;
}

bool LoginRefreshImpl::hasSupportOMMPost() const
{
	return _supportOMMPostSet;
}

bool LoginRefreshImpl::hasSupportOptimizedPauseResume() const
{
	return _supportOptimizedPauseResumeSet;
}

bool LoginRefreshImpl::hasSupportProviderDictionaryDownload() const
{
	return _supportProviderDictionaryDownloadSet;
}

bool LoginRefreshImpl::hasSupportViewRequests() const
{
	return _supportViewRequestsSet;
}

bool LoginRefreshImpl::hasSupportStandby() const
{
	return _supportStandbySet;
}

bool LoginRefreshImpl::hasSolicited() const
{
	return _solicitedSet;
}

bool LoginRefreshImpl::hasClearCache() const
{
	return _clearCacheSet;
}

bool LoginRefreshImpl::hasAuthenticationExtended() const
{
	return _authenticationExtendedSet;
}

bool LoginRefreshImpl::hasAuthenticationTTReissue() const
{
	return _authenticationTTReissueSet;
}

bool LoginRefreshImpl::hasAuthenticationErrorCode() const
{
	return _authenticationErrorCodeSet;
}

bool LoginRefreshImpl::hasAuthenticationErrorText() const
{
	return _authenticationErrorTextSet;
}

bool LoginRefreshImpl::hasName() const
{
	return _nameSet;
}

bool LoginRefreshImpl::hasNameType() const
{
	return _nameTypeSet;
}

bool LoginRefreshImpl::hasState() const
{
	return _stateSet;
}

bool LoginRefreshImpl::hasSeqNum() const
{
	return _seqNumSet;
}

const RefreshMsg& LoginRefreshImpl::getMessage()
{
	_refreshMsg.clear();

	_refreshMsg.domainType(_domainType);
	if (_nameTypeSet)
		_refreshMsg.nameType(_nameType);
	if (_nameSet)
		_refreshMsg.name(_name);
	if (_stateSet)
		_refreshMsg.state((OmmState::StreamState)_rsslState->streamState, (OmmState::DataState)_rsslState->dataState, _rsslState->code, _stateText);
	if (_seqNumSet)
		_refreshMsg.seqNum(_seqNum);
	if (_solicitedSet)
		_refreshMsg.solicited(_solicited);
	if (_clearCacheSet)
		_refreshMsg.clearCache(_clearCache);
	_refreshMsg.complete(true);

	if (!_changed)
	{
		return _refreshMsg;
	}

	encode(_refreshMsg);

	_changed = false;

	return _refreshMsg;
}

bool LoginRefreshImpl::getAllowSuspectData() const
{
	return _allowSuspectData;
}

const EmaString& LoginRefreshImpl::getApplicationId() const
{
	if (!_applicationIdSet)
	{
		EmaString text(ENAME_APP_ID);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _applicationId;
}


const EmaString& LoginRefreshImpl::getApplicationName() const
{
	if (!_applicationNameSet)
	{
		EmaString text(ENAME_APP_NAME);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _applicationName;
}

const EmaString& LoginRefreshImpl::getPosition() const
{
	if (!_positionSet)
	{
		EmaString text(ENAME_POSITION);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _position;
}

bool LoginRefreshImpl::getProvidePermissionExpressions() const
{
	return _providePermissionExpressions;
}

bool LoginRefreshImpl::getProvidePermissionProfile() const
{
	return _providePermissionProfile;
}

bool LoginRefreshImpl::getSingleOpen() const
{
	return _singleOpen;
}

UInt32 LoginRefreshImpl::getSupportBatchRequests() const
{
	return (UInt32)_supportBatchRequests;
}

UInt32 LoginRefreshImpl::getSupportEnhancedSymbolList() const
{
	return _supportEnhancedSymbolList;
}

bool LoginRefreshImpl::getSupportOMMPost() const
{
	return _supportOMMPost;
}

bool LoginRefreshImpl::getSupportOptimizedPauseResume() const
{
	return _supportOptimizedPauseResume;
}

bool LoginRefreshImpl::getSupportProviderDictionaryDownload() const
{
	return _supportProviderDictionaryDownload;
}

bool LoginRefreshImpl::getSupportViewRequests() const
{
	return _supportViewRequests;
}

bool LoginRefreshImpl::getSupportStandby() const
{
	return _supportStandby;
}

bool LoginRefreshImpl::getSolicited() const
{
	return _solicited;
}

bool LoginRefreshImpl::getClearCache() const
{
	return _clearCache;
}

const EmaBuffer& LoginRefreshImpl::getAuthenticationExtended() const
{
	if (!_authenticationExtendedSet)
	{
		EmaString text(ENAME_AUTH_EXTENDED);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationExtended;
}

const UInt64& LoginRefreshImpl::getAuthenticationTTReissue() const
{
	if (!_authenticationTTReissueSet)
	{
		EmaString text(ENAME_AUTH_TT_REISSUE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationTTReissue;
}

const UInt64& LoginRefreshImpl::getAuthenticationErrorCode() const
{
	if (!_authenticationErrorCodeSet)
	{
		EmaString text(ENAME_AUTH_ERRORCODE);
		text.append(" element is not set");
		throwIueException (text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationErrorCode;
}

const EmaString& LoginRefreshImpl::getAuthenticationErrorText() const
{
	if (!_authenticationErrorTextSet)
	{
		EmaString text(ENAME_AUTH_ERRORTEXT);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _authenticationErrorText;
}

const EmaString& LoginRefreshImpl::getName() const
{
	if (!_nameSet)
	{
		EmaString text(ENAME_USERNAME);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _name;
}

const UInt32& LoginRefreshImpl::getNameType() const
{
	if (!_nameTypeSet)
	{
		EmaString text(ENAME_USERNAME_TYPE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _nameType;
}

const OmmState& LoginRefreshImpl::getState() const
{
	if (!_stateSet)
	{
		EmaString text(ENAME_STATE);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_state._pDecoder->setRsslData(_rsslState);

	return _state;
}

const UInt32& LoginRefreshImpl::getSeqNum() const
{
	if (!_seqNumSet)
	{
		EmaString text(ENAME_SEQ_NUM);
		text.append(" element is not set");
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _seqNum;
}


const EmaString& LoginRefreshImpl::toString() const
{
	_toString.clear();

	if (_allowSuspectDataSet)
	{
		if (_allowSuspectData)
		{
			_toString.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_ALLOW_SUSPECT_DATA).append(" : ").append("NotSupported");
		}
	}

	if (_applicationIdSet)
	{
		_toString.append("\r\n").append(ENAME_APP_ID).append(" : ").append(_applicationId);
	}

	if (_applicationNameSet)
	{
		_toString.append("\r\n").append(ENAME_APP_NAME).append(" : ").append(_applicationName);
	}

	if (_positionSet)
	{
		_toString.append("\r\n").append(ENAME_POSITION).append(" : ").append(_position);
	}

	if (_providePermissionExpressionsSet)
	{
		if (_providePermissionProfile)
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_EXP).append(" : ").append("NotSupported");
		}
	}

	if (_providePermissionProfileSet)
	{
		if (_providePermissionProfile)
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_PROV_PERM_PROF).append(" : ").append("NotSupported");
		}
	}

	if (_singleOpenSet)
	{
		if (_singleOpen)
		{
			_toString.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SINGLE_OPEN).append(" : ").append("NotSupported");
		}
	}

	if (_supportBatchRequestsSet)
	{
		_toString.append("\r\n").append(ENAME_SUPPORT_BATCH).append(" :");
		
		if (_supportBatchRequests == 0x000)
		{
			_toString.append(" NotSupported");
		}
		else
		{
			if (_supportBatchRequests & SUPPORT_BATCH_REQUEST)
			{
				_toString.append(" RequestSupported");
			}

			if (_supportBatchRequests & SUPPORT_BATCH_REISSUE)
			{
				_toString.append(" ReissueSupported");
			}

			if (_supportBatchRequests & SUPPORT_BATCH_CLOSE)
			{
				_toString.append(" CloseSupported");
			}
		}
	}

	if (_supportOMMPostSet)
	{
		if (_supportOMMPost)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_POST).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_POST).append(" : ").append("NotSupported");
		}
	}

	if (_supportProviderDictionaryDownloadSet)
	{
		if (_supportProviderDictionaryDownload)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD).append(" : ").append("NotSupported");
		}
	}

	if (_supportOptimizedPauseResumeSet)
	{
		if (_supportOptimizedPauseResume)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_OPR).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_OPR).append(" : ").append("NotSupported");
		}
	}

	if (_supportViewRequestsSet)
	{
		if (_supportViewRequests)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_VIEW).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_VIEW).append(" : ").append("NotSupported");
		}
	}

	if (_supportStandbySet)
	{
		if (_supportStandby)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_STANDBY).append(" : ").append("Supported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_STANDBY).append(" : ").append("NotSupported");
		}
	}

	if (_supportEnhancedSymbolListSet)
	{
		if (_supportEnhancedSymbolList == SUPPORT_SYMBOL_LIST_NAMES_ONLY)
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_ENH_SYMBOL_LIST).append(" : ").append("NamesOnlySupported");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SUPPORT_ENH_SYMBOL_LIST).append(" : ").append("DataStreamsSupported");
		}
	}

	if (_authenticationExtendedSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_EXTENDED_RESP).append(" : ").append(_authenticationExtended);
	}

	if (_authenticationTTReissueSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_TT_REISSUE).append(" : ").append(_authenticationTTReissue);
	}

	if (_authenticationErrorCodeSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_ERRORCODE).append(" : ").append(_authenticationErrorCode);
	}

	if (_authenticationErrorTextSet)
	{
		_toString.append("\r\n").append(ENAME_AUTH_ERRORTEXT).append(" : ").append(_authenticationErrorText);
	}

	if (_nameSet)
	{
		_toString.append("\r\n").append(ENAME_USERNAME).append(" : ").append(_name);
	}

	if (_nameTypeSet)
	{
		_toString.append("\r\n").append(ENAME_USERNAME_TYPE).append(" : ").append(_nameType);
	}

	if (_stateSet)
	{
		_toString.append("\r\n").append(ENAME_STATE).append(" : StreamState: ").append(_rsslState->streamState)
			.append(" DataState: ").append(_rsslState->dataState)
			.append(" StatusCode: ").append(_rsslState->code)
			.append(" StatusText: ").append(_stateText);
	}

	if (_seqNumSet)
	{
		_toString.append("\r\n").append(ENAME_SEQ_NUM).append(" : ").append(_seqNum);
	}

	if (_solicitedSet)
	{
		if (_solicited)
		{
			_toString.append("\r\n").append(ENAME_SOLICITED).append(" : ").append("True");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_SOLICITED).append(" : ").append("False");
		}
	}

	if (_clearCacheSet)
	{
		if (_clearCache)
		{
			_toString.append("\r\n").append(ENAME_CLEARCACHE).append(" : ").append("True");
		}
		else
		{
			_toString.append("\r\n").append(ENAME_CLEARCACHE).append(" : ").append("False");
		}
	}

	return _toString;
}

void LoginRefreshImpl::encode(RefreshMsg& refreshMsg) const
{
	if (_pElementList == 0)
	{
		try
		{
			_pElementList = new ElementList();
		}
		catch (std::bad_alloc) {}

		if (!_pElementList)
			throwMeeException("Failed to allocate memory for ElementList in LoginRefreshImpl::encode().");
	}
	else
	{
		_pElementList->clear();
	}

	if (_allowSuspectDataSet)
	{
		_pElementList->addUInt(ENAME_ALLOW_SUSPECT_DATA, _allowSuspectData);
	}

	if (_applicationIdSet)
	{
		_pElementList->addAscii(ENAME_APP_ID, _applicationId);
	}

	if (_applicationNameSet)
	{
		_pElementList->addAscii(ENAME_APP_NAME, _applicationName);
	}

	if (_positionSet)
	{
		_pElementList->addAscii(ENAME_POSITION, _position);
	}

	if (_providePermissionExpressionsSet)
	{
		_pElementList->addUInt(ENAME_PROV_PERM_EXP, _providePermissionExpressions);
	}

	if (_providePermissionProfileSet)
	{
		_pElementList->addUInt(ENAME_PROV_PERM_PROF, _providePermissionProfile);
	}

	if (_singleOpenSet)
	{
		_pElementList->addUInt(ENAME_SINGLE_OPEN, _singleOpen);
	}

	if (_supportBatchRequestsSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_BATCH, _supportBatchRequests);
	}

	if (_supportOMMPostSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_POST, _supportOMMPost);
	}

	if (_supportProviderDictionaryDownloadSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, _supportProviderDictionaryDownload);
	}

	if (_supportOptimizedPauseResumeSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_OPR, _supportOptimizedPauseResume);
	}

	if (_supportViewRequestsSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_VIEW, _supportViewRequests);
	}

	if (_supportStandbySet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_STANDBY, _supportStandby);
	}

	if (_supportEnhancedSymbolListSet)
	{
		_pElementList->addUInt(ENAME_SUPPORT_ENH_SYMBOL_LIST, _supportEnhancedSymbolList);
	}

	if (_authenticationExtendedSet)
	{
		_pElementList->addBuffer(ENAME_AUTH_EXTENDED_RESP, _authenticationExtended);
	}

	if (_authenticationTTReissueSet)
	{
		_pElementList->addUInt(ENAME_AUTH_TT_REISSUE, _authenticationTTReissue);
	}

	if (_authenticationErrorCodeSet)
	{
		_pElementList->addUInt(ENAME_AUTH_ERRORCODE, _authenticationErrorCode);
	}

	if (_authenticationErrorTextSet)
	{
		_pElementList->addAscii(ENAME_AUTH_ERRORTEXT, _authenticationErrorText);
	}

	_pElementList->complete();

	refreshMsg.attrib(*_pElementList);
}

void LoginRefreshImpl::decode(const RefreshMsg& refreshMsg)
{
	_allowSuspectDataSet = false;
	_providePermissionProfileSet = false;
	_providePermissionExpressionsSet = false;
	_singleOpenSet = false;
	_supportBatchRequestsSet = false;
	_supportOptimizedPauseResumeSet = false;
	_supportProviderDictionaryDownloadSet = false;
	_applicationIdSet = false;
	_applicationNameSet = false;
	_positionSet = false;
	_supportViewRequestsSet = false;
	_supportStandbySet = false;
	_supportOMMPostSet = false;
	_supportEnhancedSymbolListSet = false;
	_solicitedSet = false;
	_clearCacheSet = false;
	_authenticationExtendedSet = false;
	_authenticationTTReissueSet = false;
	_authenticationErrorCodeSet = false;
	_authenticationErrorTextSet = false;
	_stateSet = false;
	_nameSet = false;
	_nameTypeSet = false;
	_seqNumSet = false;

	state(refreshMsg.getState());
	if (refreshMsg.hasName())
		name(refreshMsg.getName());
	if (refreshMsg.hasNameType())
		nameType(refreshMsg.getNameType());
	if (refreshMsg.hasSeqNum())
		seqNum(refreshMsg.getSeqNum());

	if ( refreshMsg.getAttrib().getDataType() != DataType::ElementListEnum )
	  return;

	while (refreshMsg.getAttrib().getElementList().forth())
	{
		const ElementEntry& elementEntry = refreshMsg.getAttrib().getElementList().getEntry();
		const EmaString& elementName = elementEntry.getName();

		try
		{
			if ( elementName == ENAME_ALLOW_SUSPECT_DATA )
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					allowSuspectData(true);
				}
				else
				{
					allowSuspectData(false);
				}
			}
			else if (elementName == ENAME_APP_ID )
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					applicationId(elementEntry.getAscii());
			}
			else if (elementName == ENAME_APP_NAME)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					applicationName(elementEntry.getAscii());
			}
			else if (elementName == ENAME_POSITION)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					position(elementEntry.getAscii());
			}
			else if (elementName == ENAME_PROV_PERM_EXP)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					providePermissionExpressions(true);
				}
				else
				{
					providePermissionExpressions(false);
				}
			}
			else if (elementName == ENAME_PROV_PERM_PROF)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					providePermissionProfile(true);
				}
				else
				{
					providePermissionProfile(false);
				}
			}
			else if (elementName == ENAME_SINGLE_OPEN)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					singleOpen(true);
				}
				else
				{
					singleOpen(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_BATCH)
			{
				UInt64 value = elementEntry.getUInt();

				_supportBatchRequests = 0;

				_changed = true;
				_supportBatchRequestsSet = true;

				if (value == 0x000)
				{
					_supportBatchRequests = value;
				}
				else
				{
					if (value & SUPPORT_BATCH_REQUEST)
					{
						_supportBatchRequests |= SUPPORT_BATCH_REQUEST;
					}

					if (value & SUPPORT_BATCH_REISSUE)
					{
						_supportBatchRequests |= SUPPORT_BATCH_REISSUE;
					}

					if (value & SUPPORT_BATCH_CLOSE)
					{
						_supportBatchRequests |= SUPPORT_BATCH_CLOSE;
					}
				}
			}
			else if (elementName == ENAME_SUPPORT_POST)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportOMMPost(true);
				}
				else
				{
					supportOMMPost(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_VIEW)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportViewRequests(true);
				}
				else
				{
					supportViewRequests(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportProviderDictionaryDownload(true);
				}
				else
				{
					supportProviderDictionaryDownload(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_OPR)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportOptimizedPauseResume(true);
				}
				else
				{
					supportOptimizedPauseResume(false);
				}
			}
			else if (elementName == ENAME_SUPPORT_STANDBY)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportStandby(true);
				}
				else
				{
					supportStandby(false);
				}

			}
			else if (elementName == ENAME_SUPPORT_ENH_SYMBOL_LIST)
			{
				UInt64 value = elementEntry.getUInt();

				if (value > 0)
				{
					supportEnhancedSymbolList(SUPPORT_SYMBOL_LIST_DATA_STREAMS);
				}
				else
				{
					supportEnhancedSymbolList(SUPPORT_SYMBOL_LIST_NAMES_ONLY);
				}
			}
			else if (elementName == ENAME_AUTH_EXTENDED_RESP)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					authenticationExtended(elementEntry.getBuffer());
			}
			else if (elementName == ENAME_AUTH_TT_REISSUE)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					authenticationTTReissue(elementEntry.getUInt());
			}
			else if (elementName == ENAME_AUTH_ERRORCODE)
			{
				authenticationErrorCode(elementEntry.getUInt());
			}
			else if (elementName == ENAME_AUTH_ERRORTEXT)
			{
				if (elementEntry.getCode() == Data::NoCodeEnum)
					authenticationErrorText(elementEntry.getAscii());
			}
		}
		catch (const OmmInvalidUsageException& iue)
		{
			EmaString text("Decoding error for ");
			text.append(elementName).append(" element. ").append(iue.getText());
			throwIueException( text, iue.getErrorCode() );
		}
	}
}
