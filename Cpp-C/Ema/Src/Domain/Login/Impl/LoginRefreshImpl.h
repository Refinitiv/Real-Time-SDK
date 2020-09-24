/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "ElementList.h"
#include "EmaBuffer.h"
#include "EmaString.h"
#include "RefreshMsg.h"
#include "ExceptionTranslator.h"
#include "rtr/rsslDataPackage.h"
#include "EmaStringInt.h"
#include "OmmState.h"
#include "OmmStateDecoder.h"

#ifndef __rtsdk_ema_domain_LoginRefreshImpl_h
#define __rtsdk_ema_domain_LoginRefreshImpl_h

namespace rtsdk {

namespace ema {

namespace domain {

namespace login {

class LoginRefreshImpl
{
public:

	LoginRefreshImpl();

	LoginRefreshImpl(const LoginRefreshImpl&);

	LoginRefreshImpl(const rtsdk::ema::access::RefreshMsg&);

	virtual ~LoginRefreshImpl();

	LoginRefreshImpl& clear();

	LoginRefreshImpl& operator=(const LoginRefreshImpl&);

	LoginRefreshImpl& message(const rtsdk::ema::access::RefreshMsg&);

	LoginRefreshImpl& allowSuspectData(bool value);

	LoginRefreshImpl& applicationId(const rtsdk::ema::access::EmaString&);

	LoginRefreshImpl& applicationName(const rtsdk::ema::access::EmaString&);

	LoginRefreshImpl& position(const rtsdk::ema::access::EmaString&);

	LoginRefreshImpl& providePermissionExpressions(bool value);

	LoginRefreshImpl& providePermissionProfile(bool value);

	LoginRefreshImpl& singleOpen(bool value);

	LoginRefreshImpl& supportBatchRequests(rtsdk::ema::access::UInt64 value);

	LoginRefreshImpl& supportEnhancedSymbolList(rtsdk::ema::access::UInt32 value);

	LoginRefreshImpl& supportOMMPost(bool value);

	LoginRefreshImpl& supportOptimizedPauseResume(bool value);

	LoginRefreshImpl& supportProviderDictionaryDownload(bool value);

	LoginRefreshImpl& supportViewRequests(bool value);

	LoginRefreshImpl& supportStandby(bool value);

	LoginRefreshImpl& solicited(bool value);

	LoginRefreshImpl& clearCache(bool value);

	LoginRefreshImpl& authenticationExtended(const rtsdk::ema::access::EmaBuffer& value);

	LoginRefreshImpl& authenticationTTReissue(const rtsdk::ema::access::UInt64& value);

	LoginRefreshImpl& authenticationErrorCode(const rtsdk::ema::access::UInt64& value);

	LoginRefreshImpl& authenticationErrorText(const rtsdk::ema::access::EmaString& value);

	LoginRefreshImpl& name(const rtsdk::ema::access::EmaString&);

	LoginRefreshImpl& nameType(const rtsdk::ema::access::UInt32&);

	LoginRefreshImpl& state(const rtsdk::ema::access::OmmState::StreamState&, const rtsdk::ema::access::OmmState::DataState, const rtsdk::ema::access::UInt8&, const rtsdk::ema::access::EmaString&);

	LoginRefreshImpl& state(const rtsdk::ema::access::OmmState&);

	LoginRefreshImpl& seqNum(const rtsdk::ema::access::UInt32&);

	bool hasAllowSuspectData() const;

	bool hasApplicationId() const;

	bool hasApplicationName() const;

	bool hasPosition() const;

	bool hasProvidePermissionExpressions() const;

	bool hasProvidePermissionProfile() const;

	bool hasSingleOpen() const;

	bool hasSupportBatchRequests() const;

	bool hasSupportEnhancedSymbolList() const;

	bool hasSupportOMMPost() const;

	bool hasSupportOptimizedPauseResume() const;

	bool hasSupportProviderDictionaryDownload() const;

	bool hasSupportViewRequests() const;

	bool hasSupportStandby() const;

	bool hasSolicited() const;

	bool hasClearCache() const;

	bool hasAuthenticationExtended() const;

	bool hasAuthenticationTTReissue() const;

	bool hasAuthenticationErrorCode() const;

	bool hasAuthenticationErrorText() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasState() const;

	bool hasSeqNum() const;

	bool getAllowSuspectData() const;

	const rtsdk::ema::access::RefreshMsg& getMessage();

	const rtsdk::ema::access::EmaString& getApplicationId() const;

	const rtsdk::ema::access::EmaString& getApplicationName() const;

	const rtsdk::ema::access::EmaString& getPosition() const;

	bool getProvidePermissionExpressions() const;

	bool getProvidePermissionProfile() const;

	bool getSingleOpen() const;

	rtsdk::ema::access::UInt32 getSupportBatchRequests() const;

	rtsdk::ema::access::UInt32 getSupportEnhancedSymbolList() const;

	bool getSupportOMMPost() const;

	bool getSupportOptimizedPauseResume() const;

	bool getSupportProviderDictionaryDownload() const;

	bool getSupportViewRequests() const;

	bool getSupportStandby() const;

	bool getSolicited() const;

	bool getClearCache() const;

	const rtsdk::ema::access::EmaBuffer& getAuthenticationExtended() const;

	const rtsdk::ema::access::UInt64& getAuthenticationTTReissue() const;

	const rtsdk::ema::access::UInt64& getAuthenticationErrorCode() const;

	const rtsdk::ema::access::EmaString& getAuthenticationErrorText() const;

	const rtsdk::ema::access::EmaString& getName() const;

	const rtsdk::ema::access::UInt32& getNameType() const;

	const rtsdk::ema::access::OmmState& getState() const;

	const rtsdk::ema::access::UInt32& getSeqNum() const;

	const rtsdk::ema::access::EmaString& toString() const;

private:

	void encode(rtsdk::ema::access::RefreshMsg&) const;

	void decode(const rtsdk::ema::access::RefreshMsg&);

	const rtsdk::ema::access::RefreshMsg& message() const;

	bool									_allowSuspectData;
	bool									_providePermissionProfile;
	bool									_providePermissionExpressions;
	bool									_singleOpen;
	rtsdk::ema::access::UInt64		_supportBatchRequests;
	bool									_supportOptimizedPauseResume;
	bool									_supportProviderDictionaryDownload;
	rtsdk::ema::access::EmaString	_applicationId;
	rtsdk::ema::access::EmaString	_applicationName;
	rtsdk::ema::access::EmaString	_position;
	bool									_supportViewRequests;
	bool									_supportStandby;
	bool									_supportOMMPost;
	rtsdk::ema::access::UInt32		_supportEnhancedSymbolList;
	bool									_solicited;
	bool									_clearCache;
	rtsdk::ema::access::EmaBuffer	_authenticationExtended;
	rtsdk::ema::access::UInt64		_authenticationTTReissue;
	rtsdk::ema::access::UInt64		_authenticationErrorCode;
	rtsdk::ema::access::EmaString	_authenticationErrorText;

	mutable bool			_changed;
	bool					_allowSuspectDataSet;
	bool					_providePermissionProfileSet;
	bool					_providePermissionExpressionsSet;
	bool					_singleOpenSet;
	bool					_supportBatchRequestsSet;
	bool					_supportOptimizedPauseResumeSet;
	bool					_supportProviderDictionaryDownloadSet;
	bool					_applicationIdSet;
	bool					_applicationNameSet;
	bool					_positionSet;
	bool					_supportViewRequestsSet;
	bool					_supportStandbySet;
	bool					_supportOMMPostSet;
	bool					_supportEnhancedSymbolListSet;
	bool					_solicitedSet;
	bool					_clearCacheSet;
	bool					_authenticationExtendedSet;
	bool					_authenticationTTReissueSet;
	bool					_authenticationErrorCodeSet;
	bool					_authenticationErrorTextSet;
	bool					_stateSet;
	bool					_nameSet;
	bool					_nameTypeSet;
	bool					_seqNumSet;

	rtsdk::ema::access::UInt32					_domainType;
	rtsdk::ema::access::UInt32					_seqNum;
	rtsdk::ema::access::UInt32					_nameType;
	rtsdk::ema::access::EmaString				_name;

	RsslState*											_rsslState;
	rtsdk::ema::access::EmaStringInt			_stateText;
	rtsdk::ema::access::OmmState				_state;

	rtsdk::ema::access::RefreshMsg				_refreshMsg;

	mutable rtsdk::ema::access::ElementList*	_pElementList;

	mutable rtsdk::ema::access::EmaString       _toString;

#ifdef __EMA_COPY_ON_SET__
	rtsdk::ema::access::EmaString				_statusText;
#endif
};

}

}

}

}

#endif // __rtsdk_ema_domain_LoginRefreshImpl_h

