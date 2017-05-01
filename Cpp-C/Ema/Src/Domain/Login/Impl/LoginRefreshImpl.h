/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2016. All rights reserved.            --
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

#ifndef __thomsonreuters_ema_domain_LoginRefreshImpl_h
#define __thomsonreuters_ema_domain_LoginRefreshImpl_h

namespace thomsonreuters {

namespace ema {

namespace domain {

namespace login {

class LoginRefreshImpl
{
public:

	LoginRefreshImpl();

	LoginRefreshImpl(const LoginRefreshImpl&);

	LoginRefreshImpl(const thomsonreuters::ema::access::RefreshMsg&);

	virtual ~LoginRefreshImpl();

	LoginRefreshImpl& clear();

	LoginRefreshImpl& operator=(const LoginRefreshImpl&);

	LoginRefreshImpl& message(const thomsonreuters::ema::access::RefreshMsg&);

	LoginRefreshImpl& allowSuspectData(bool value);

	LoginRefreshImpl& applicationId(const thomsonreuters::ema::access::EmaString&);

	LoginRefreshImpl& applicationName(const thomsonreuters::ema::access::EmaString&);

	LoginRefreshImpl& position(const thomsonreuters::ema::access::EmaString&);

	LoginRefreshImpl& providePermissionExpressions(bool value);

	LoginRefreshImpl& providePermissionProfile(bool value);

	LoginRefreshImpl& singleOpen(bool value);

	LoginRefreshImpl& supportBatchRequests(thomsonreuters::ema::access::UInt64 value);

	LoginRefreshImpl& supportEnhancedSymbolList(thomsonreuters::ema::access::UInt32 value);

	LoginRefreshImpl& supportOMMPost(bool value);

	LoginRefreshImpl& supportOptimizedPauseResume(bool value);

	LoginRefreshImpl& supportProviderDictionaryDownload(bool value);

	LoginRefreshImpl& supportViewRequests(bool value);

	LoginRefreshImpl& supportStandby(bool value);

	LoginRefreshImpl& solicited(bool value);

	LoginRefreshImpl& clearCache(bool value);

	LoginRefreshImpl& authenticationExtended(const thomsonreuters::ema::access::EmaBuffer& value);

	LoginRefreshImpl& authenticationTTReissue(const thomsonreuters::ema::access::UInt64& value);

	LoginRefreshImpl& authenticationErrorCode(const thomsonreuters::ema::access::UInt64& value);

	LoginRefreshImpl& authenticationErrorText(const thomsonreuters::ema::access::EmaString& value);

	LoginRefreshImpl& name(const thomsonreuters::ema::access::EmaString&);

	LoginRefreshImpl& nameType(const thomsonreuters::ema::access::UInt32&);

	LoginRefreshImpl& state(const thomsonreuters::ema::access::OmmState::StreamState&, const thomsonreuters::ema::access::OmmState::DataState, const thomsonreuters::ema::access::UInt8&, const thomsonreuters::ema::access::EmaString&);

	LoginRefreshImpl& state(const thomsonreuters::ema::access::OmmState&);

	LoginRefreshImpl& seqNum(const thomsonreuters::ema::access::UInt32&);

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

	const thomsonreuters::ema::access::RefreshMsg& getMessage();

	const thomsonreuters::ema::access::EmaString& getApplicationId() const;

	const thomsonreuters::ema::access::EmaString& getApplicationName() const;

	const thomsonreuters::ema::access::EmaString& getPosition() const;

	bool getProvidePermissionExpressions() const;

	bool getProvidePermissionProfile() const;

	bool getSingleOpen() const;

	thomsonreuters::ema::access::UInt32 getSupportBatchRequests() const;

	thomsonreuters::ema::access::UInt32 getSupportEnhancedSymbolList() const;

	bool getSupportOMMPost() const;

	bool getSupportOptimizedPauseResume() const;

	bool getSupportProviderDictionaryDownload() const;

	bool getSupportViewRequests() const;

	bool getSupportStandby() const;

	bool getSolicited() const;

	bool getClearCache() const;

	const thomsonreuters::ema::access::EmaBuffer& getAuthenticationExtended() const;

	const thomsonreuters::ema::access::UInt64& getAuthenticationTTReissue() const;

	const thomsonreuters::ema::access::UInt64& getAuthenticationErrorCode() const;

	const thomsonreuters::ema::access::EmaString& getAuthenticationErrorText() const;

	const thomsonreuters::ema::access::EmaString& getName() const;

	const thomsonreuters::ema::access::UInt32& getNameType() const;

	const thomsonreuters::ema::access::OmmState& getState() const;

	const thomsonreuters::ema::access::UInt32& getSeqNum() const;

	const thomsonreuters::ema::access::EmaString& toString() const;

private:

	void encode(thomsonreuters::ema::access::RefreshMsg&) const;

	void decode(const thomsonreuters::ema::access::RefreshMsg&);

	const thomsonreuters::ema::access::RefreshMsg& message() const;

	bool									_allowSuspectData;
	bool									_providePermissionProfile;
	bool									_providePermissionExpressions;
	bool									_singleOpen;
	thomsonreuters::ema::access::UInt64		_supportBatchRequests;
	bool									_supportOptimizedPauseResume;
	bool									_supportProviderDictionaryDownload;
	thomsonreuters::ema::access::EmaString	_applicationId;
	thomsonreuters::ema::access::EmaString	_applicationName;
	thomsonreuters::ema::access::EmaString	_position;
	bool									_supportViewRequests;
	bool									_supportStandby;
	bool									_supportOMMPost;
	thomsonreuters::ema::access::UInt32		_supportEnhancedSymbolList;
	bool									_solicited;
	bool									_clearCache;
	thomsonreuters::ema::access::EmaBuffer	_authenticationExtended;
	thomsonreuters::ema::access::UInt64		_authenticationTTReissue;
	thomsonreuters::ema::access::UInt64		_authenticationErrorCode;
	thomsonreuters::ema::access::EmaString	_authenticationErrorText;

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

	thomsonreuters::ema::access::UInt32					_domainType;
	thomsonreuters::ema::access::UInt32					_seqNum;
	thomsonreuters::ema::access::UInt32					_nameType;
	thomsonreuters::ema::access::EmaString				_name;

	RsslState*											_rsslState;
	thomsonreuters::ema::access::EmaStringInt			_stateText;
	thomsonreuters::ema::access::OmmState				_state;

	thomsonreuters::ema::access::RefreshMsg				_refreshMsg;

	mutable thomsonreuters::ema::access::ElementList*	_pElementList;

	mutable thomsonreuters::ema::access::EmaString       _toString;
};

}

}

}

}

#endif // __thomsonreuters_ema_domain_LoginRefreshImpl_h

