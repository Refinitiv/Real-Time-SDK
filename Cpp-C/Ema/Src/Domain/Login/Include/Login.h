/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2016. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_domain_Login_h
#define __thomsonreuters_ema_domain_Login_h

namespace thomsonreuters {

namespace ema {

namespace domain {

namespace login {

class LoginReqImpl;
class LoginRefreshImpl;
class LoginStatusImpl;

class EMA_ACCESS_API Login {

public:

/**
@class thomsonreuters::ema::domain::login::LoginReq Login.h "Domain/Login/Include/Login.h"
@brief LoginReq is a helper class to provide ease of use for encoding and decoding of RDM defined login request attributes.

@see EmaString,
ElementList
*/

class EMA_ACCESS_API LoginReq
{
public :

	///@name Constructor
	//@{
	/** Default constructor setting everything to the RDM specified defaults
	*/
	LoginReq();

	/** Copy constructor
	*/
	LoginReq( const LoginReq& );

	/** Constructor initialized with ReqMsg
	*/
	LoginReq( const thomsonreuters::ema::access::ReqMsg& );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~LoginReq();
	//@}

	///@name Operations
	//@{
	/** Sets all values to defaults defined in RDM Usage Guide.
		@return reference to this object
	*/
	LoginReq& clear();

	/** Assignment operator
		@return reference to this object
	*/
	LoginReq& operator=( const LoginReq& );

	/** Sets LoginReq with ReqMsg
		\remark allows easy decoding of login request attributes
		@return reference to this object
	*/
	LoginReq& message(const thomsonreuters::ema::access::ReqMsg&);

	/** method to set requested AllowSuspectData support type
		@return reference to this object
	*/
	LoginReq& allowSuspectData(bool value = true);

	/** method to set requested DownloadConnectionConfig support type
		@return reference to this object
	*/
	LoginReq& downloadConnectionConfig(bool value = false);

	/** method to set ApplicationId
		@return reference to this object
	*/
	LoginReq& applicationId( const thomsonreuters::ema::access::EmaString& );

	/** method to set ApplicationName
		@return reference to this object
	*/
	LoginReq& applicationName( const thomsonreuters::ema::access::EmaString& );

	/** method to set ApplicationAuthorizationToken
		@return reference to this object
	*/
	LoginReq& applicationAuthorizationToken(const thomsonreuters::ema::access::EmaString&);

	/** method to set application's InstanceId
		@return reference to this object
	*/
	LoginReq& instanceId( const thomsonreuters::ema::access::EmaString& );

	/** method to set application's Password
		@return reference to this object
	*/
	LoginReq& password( const thomsonreuters::ema::access::EmaString& );

	/** method to set application's Position
		@return reference to this object
	*/
	LoginReq& position( const thomsonreuters::ema::access::EmaString& );

	/** method to set requested ProvidePermissionExpressions support type
		@return reference to this object
	*/
	LoginReq& providePermissionExpressions(bool value = true);

	/** method to set requested ProvidePermissionProfile support type
		@return reference to this object
	*/
	LoginReq& providePermissionProfile(bool value = true);

	/** method to set application's Role
		@return reference to this object
	*/
	LoginReq& role(thomsonreuters::ema::access::UInt32 value = thomsonreuters::ema::rdm::LOGIN_ROLE_CONS);

	/** method to set requested SingleOpen support type
		@return reference to this object
	*/
	LoginReq& singleOpen(bool value = true);

	/** method to set requested SupportProviderDictionaryDownload support type
		@return reference to this object
	*/
	LoginReq& supportProviderDictionaryDownload(bool value = false);

	/** method to set pause flag on LoginReq
		@return reference to this object
	*/
	LoginReq& pause(bool value = false);

	/** method to set authentication extended buffer
		@return reference to this object
	*/
	LoginReq& authenticationExtended(const thomsonreuters::ema::access::EmaBuffer&);

	/** method to set name
	@return reference to this object
	*/
	LoginReq& name(const thomsonreuters::ema::access::EmaString&);

	/** method to set name type
	@return reference to this object
	*/
	LoginReq& nameType(const thomsonreuters::ema::access::UInt32&);
	//@}

	///@name Accessors
	//@{

	/** method to check if AllowSuspectData support type is set
		@return true if AllowSuspectData support type is set
	*/
	bool hasAllowSuspectData() const;

	/** method to check if DownloadConnectionConfig support type is set
		@return true if DownloadConnectionConfig support type is set
	*/
	bool hasDownloadConnectionConfig() const;

	/** method to check if ApplicationId is set
		@return true if ApplicationId is set
	*/
	bool hasApplicationId() const;

	/** method to check if ApplicationName is set
		@return true if ApplicationName is set
	*/
	bool hasApplicationName() const;

	/** method to check if ApplicationAuthorizationToken is set
		@return true if ApplicationAuthorizationToken is set
	*/
	bool hasApplicationAuthorizationToken() const;

	/** method to check if InstanceId is set
		@return true if InstanceId is set
	*/
	bool hasInstanceId() const;

	/** method to check if Password is set
		@return true if Password is set
	*/
	bool hasPassword() const;

	/** method to check if Position is set
		@return true if Position is set
	*/
	bool hasPosition() const;

	/** method to check if ProvidePermissionExpressions support type is set
		@return true if ProvidePermissionExpressions support type is set
	*/
	bool hasProvidePermissionExpressions() const;

	/** method to check if ProvidePermissionProfile support type is set
		@return true if ProvidePermissionProfile support type is set
	*/
	bool hasProvidePermissionProfile() const;

	/** method to check if Role is set
		@return true if Role is set
	*/
	bool hasRole() const;

	/** method to check if SingleOpen support type is set
		@return true if SingleOpen support type is set
	*/
	bool hasSingleOpen() const;

	/** method to check if SupportProviderDictionaryDownload support type is set
		@return true if SupportProviderDictionaryDownload support type is set
	*/
	bool hasSupportProviderDictionaryDownload() const;

	/** method to check if Pause element was set on message
		@return true if Pause element was set
	*/
	bool hasPause() const;

	/** method to check if authentication extended buffer is set
		@return true if authentication extended buffer is set
	*/
	bool hasAuthenticationExtended() const;

	/** method to check if name is set
	@return true if name is set
	*/
	bool hasName() const;

	/** method to check if nameType is set
	@return true if nameType is set
	*/
	bool hasNameType() const;

	/** method to obtain ReqMsg out of LoginReq
		@return ReqMsg version of LoginReq
	*/
	const thomsonreuters::ema::access::ReqMsg& getMessage() const;

	/** method to obtain requested AllowSuspectData support type
	*/
	bool getAllowSuspectData() const;

	/** method to obtain requested DownloadConnectionConfig support type
	*/
	bool getDownloadConnectionConfig() const;

	/** method to obtain ApplicationId
		@throw OmmInvalidUsageException if hasApplicationId() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getApplicationId() const;

	/** method to obtain ApplicationName
		@throw OmmInvalidUsageException if hasApplicationName() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getApplicationName() const;

	/** method to obtain ApplicationAuthorizationToken
		@throw OmmInvalidUsageException if hasApplicationAuthorizationToken() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getApplicationAuthorizationToken() const;

	/** method to obtain InstanceId
		@throw OmmInvalidUsageException if hasInstanceId() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getInstanceId() const;

	/** method to obtain Password
		@throw OmmInvalidUsageException if hasPassword() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getPassword() const;

	/** method to obtain Position
		@throw OmmInvalidUsageException if hasPosition() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getPosition() const;

	/** method to obtain requested ProvidePermissionExpressions support type
	*/
	bool getProvidePermissionExpressions() const;

	/** method to obtain requested ProvidePermissionProfile support type
	*/
	bool getProvidePermissionProfile() const;

	/** method to obtain Role 
	*/
	thomsonreuters::ema::access::UInt32 getRole() const;

	/** method to obtain requested SingleOpen support type
	*/
	bool getSingleOpen() const;

	/** method to obtain requested SupportProviderDictionaryDownload support type
	*/
	bool getSupportProviderDictionaryDownload() const;

	/** method to obtain if Pause flag is set
	*/
	bool getPause() const;

	/** method to obtain authentication extended
	*/
	const thomsonreuters::ema::access::EmaBuffer& getAuthenticationExtended() const;

	/** method to obtain name
	*/
	const thomsonreuters::ema::access::EmaString& getName() const;

	/** method to obtain nameType
	*/
	const thomsonreuters::ema::access::UInt32& getNameType() const;

	/** Returns a string representation of the class instance.
	@return string representation of the class instance
	*/
	const thomsonreuters::ema::access::EmaString& toString() const;
	//@}

private :

	LoginReqImpl* _pLoginReqImpl;
};

/**
@class thomsonreuters::ema::domain::login::LoginRefresh Login.h "Domain/Login/Include/Login.h"
@brief LoginRefresh is a helper class to provide ease of use for encoding and decoding of RDM defined login refresh attributes.

@see EmaString,
ElementList
*/

class EMA_ACCESS_API LoginRefresh
{
public :

	///@name Constructor
	//@{
	/** Default constructor setting everything to the RDM specified defaults
	*/
	LoginRefresh();

	/** Copy constructor
	*/
	LoginRefresh( const LoginRefresh& );

	/** Constructor initialized with RefreshMsg
		\remark allows easy decoding and encoding of login refresh attributes
	*/
	LoginRefresh( const thomsonreuters::ema::access::RefreshMsg& );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~LoginRefresh();
	//@}

	///@name Operations
	//@{
	/** Sets all values to defaults defined in RDM Usage Guide.
		@return reference to this object
	*/
	LoginRefresh& clear();

	/** Assignment operator
		@return reference to this object
	*/
	LoginRefresh& operator=( const LoginRefresh& );

	/** Sets LoginRefresh with RefreshMsg
		\remark allows easy decoding of login refresh attributes
		@return reference to this object
	*/
	LoginRefresh& message(const thomsonreuters::ema::access::RefreshMsg&);

	/** method to set requested AllowSuspectData support type
		@return reference to this object
	*/
	LoginRefresh& allowSuspectData(bool value = true);

	/** method to set ApplicationId
		@return reference to this object
	*/
	LoginRefresh& applicationId( const thomsonreuters::ema::access::EmaString& );

	/** method to set ApplicationName
		@return reference to this object
	*/
	LoginRefresh& applicationName( const thomsonreuters::ema::access::EmaString& );

	/** method to set application's Position
		@return reference to this object
	*/
	LoginRefresh& position( const thomsonreuters::ema::access::EmaString& );

	/** method to set requested ProvidePermissionExpressions support type
		@return reference to this object
	*/
	LoginRefresh& providePermissionExpressions(bool value = true);

	/** method to set requested ProvidePermissionProfile support type
		@return reference to this object
	*/
	LoginRefresh& providePermissionProfile(bool value = true);

	/** method to set requested SingleOpen support type
		@return reference to this object
	*/
	LoginRefresh& singleOpen(bool value = true);

	/** method to set requested SupportBatchRequests support type
		@return reference to this object
	*/
	LoginRefresh& supportBatchRequests( thomsonreuters::ema::access::UInt32 value = 0x000);

	/** method to set requested SupportEnhancedSymbolList support type
		@return reference to this object
	*/
	LoginRefresh& supportEnhancedSymbolList(thomsonreuters::ema::access::UInt32 value = thomsonreuters::ema::rdm::SUPPORT_SYMBOL_LIST_NAMES_ONLY);

	/** method to set requested SupportOMMPost support type
		@return reference to this object
	*/
	LoginRefresh& supportOMMPost(bool value = false);

	/** method to set requested SupportOptimizedPauseResume support type
		@return reference to this object
	*/
	LoginRefresh& supportOptimizedPauseResume(bool value = false);

	/** method to set requested SupportProviderDictionaryDownload support type
		@return reference to this object
	*/
	LoginRefresh& supportProviderDictionaryDownload(bool value = false);

	/** method to set requested SupportViewRequests support type
		@return reference to this object
	*/
	LoginRefresh& supportViewRequests(bool value = false);

	/** method to set requested SupportStandby support type
		@return reference to this object
	*/
	LoginRefresh& supportStandby(bool value = false);

	/** method to set whether refresh message is solicited
	@return reference to this object
	*/
	LoginRefresh& solicited(bool value = false);

	/** method to set clearCache flag on message
	@return reference to this object
	*/
	LoginRefresh& clearCache(bool value = false);

	/** method to set authentication extended buffer
	@return reference to this object
	*/
	LoginRefresh& authenticationExtended(const thomsonreuters::ema::access::EmaBuffer& value);

	/** method to set authenticationTTReissue
	@return reference to this object
	*/
	LoginRefresh& authenticationTTReissue(const thomsonreuters::ema::access::UInt64& value);

	/** method to set authenticationErrorCode
	@return reference to this object
	*/
	LoginRefresh& authenticationErrorCode(const thomsonreuters::ema::access::UInt64& value);
	
	/** method to set authenticationErrorText
	@return reference to this object
	*/
	LoginRefresh& authenticationErrorText(const thomsonreuters::ema::access::EmaString& value);

	/** method to set name
	@return reference to this object
	*/
	LoginRefresh& name(const thomsonreuters::ema::access::EmaString&);

	/** method to set name type
	@return reference to this object
	*/
	LoginRefresh& nameType(const thomsonreuters::ema::access::UInt32&);

	/** method to set state
	@return reference to this object
	*/
	LoginRefresh& state(const thomsonreuters::ema::access::OmmState::StreamState&, const thomsonreuters::ema::access::OmmState::DataState, const thomsonreuters::ema::access::UInt8&, const thomsonreuters::ema::access::EmaString&);

	/** method to set sequence number
	@return reference to this object
	*/
	LoginRefresh& seqNum(const thomsonreuters::ema::access::UInt32&);
	//@}

	///@name Accessors
	//@{

	/** method to check if AllowSuspectData support type is set
		@return true if AllowSuspectData support type is set
	*/
	bool hasAllowSuspectData() const;

	/** method to check if ApplicationId is set
		@return true if ApplicationId is set
	*/
	bool hasApplicationId() const;

	/** method to check if ApplicationName is set
		@return true if ApplicationName is set
	*/
	bool hasApplicationName() const;

	/** method to check if Position is set
		@return true if Position is set
	*/
	bool hasPosition() const;

	/** method to check if ProvidePermissionExpressions support type is set
		@return true if ProvidePermissionExpressions support type is set
	*/
	bool hasProvidePermissionExpressions() const;

	/** method to check if ProvidePermissionProfile support type is set
		@return true if ProvidePermissionProfile support type is set
	*/
	bool hasProvidePermissionProfile() const;

	/** method to check if SingleOpen support type is set
		@return true if SingleOpen support type is set
	*/
	bool hasSingleOpen() const;

	/** method to check if SupportBatchRequests support type is set
		@return true if SupportBatchRequests support type is set
	*/
	bool hasSupportBatchRequests() const;

	/** method to check if SupportEnhancedSymbolList support type is set
		@return true if SupportEnhancedSymbolList support type is set
	*/
	bool hasSupportEnhancedSymbolList() const;

	/** method to check if SupportOMMPost support type is set
		@return true if SupportOMMPost support type is set
	*/
	bool hasSupportOMMPost() const;

	/** method to check if SupportOptimizedPauseResume support type is set
		@return true if SupportOptimizedPauseResume support type is set
	*/
	bool hasSupportOptimizedPauseResume() const;

	/** method to check if SupportProviderDictionaryDownload support type is set
		@return true if SupportProviderDictionaryDownload support type is set
	*/
	bool hasSupportProviderDictionaryDownload() const;

	/** method to check if SupportViewRequests support type is set
		@return true if SupportViewRequests support type is set
	*/
	bool hasSupportViewRequests() const;

	/** method to check if SupportStandby support type is set
		@return true if SupportStandby support type is set
	*/
	bool hasSupportStandby() const;

	/** method to check if Solicited is set
	@return true if Solicited is set
	*/
	bool hasSolicited() const;

	/** method to check if clearCache is set
	@return true if clearCache is set
	*/
	bool hasClearCache() const;

	/** method to check if authentication extended buffer is set
		@return true if authentication extended buffer is set
	*/
	bool hasAuthenticationExtended() const;

	/** method to check if authenticationTTReissue is set
		@return true if authenticationTTReissue is set
	*/
	bool hasAuthenticationTTReissue() const;

	/** method to check if authenticationErrorCode is set
		@return true if authenticationErrorCode is set
	*/
	bool hasAuthenticationErrorCode() const;

	/** method to check if authenticationErrorText is set
		@return true if authenticationErrorText is set
	*/
	bool hasAuthenticationErrorText() const;

	/** method to check if name is set
	@return true if name is set
	*/
	bool hasName() const;

	/** method to check if nameType is set
	@return true if nameType is set
	*/
	bool hasNameType() const;

	/** method to check if state is set
	@return true if state is set
	*/
	bool hasState() const;

	/** method to check if seqNum is set
	@return true if seqNum is set
	*/
	bool hasSeqNum() const;

	/** method to obtain RefreshMsg out of LoginRefresh
	@return RefreshMsg version of LoginRefresh
	*/
	const thomsonreuters::ema::access::RefreshMsg& getMessage() const;

	/** method to obtain requested AllowSuspectData support type
	*/
	bool getAllowSuspectData() const;

	/** method to obtain ApplicationId
		@throw OmmInvalidUsageException if hasApplicationId() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getApplicationId() const;

	/** method to obtain ApplicationName
		@throw OmmInvalidUsageException if hasApplicationName() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getApplicationName() const;

	/** method to obtain Position
		@throw OmmInvalidUsageException if hasPosition() returns false
	*/
	const thomsonreuters::ema::access::EmaString& getPosition() const;

	/** method to obtain requested ProvidePermissionExpressions support type
	*/
	bool getProvidePermissionExpressions() const;

	/** method to obtain requested ProvidePermissionProfile support type
	*/
	bool getProvidePermissionProfile() const;

	/** method to obtain requested SingleOpen support type
	*/
	bool getSingleOpen() const;

	/** method to obtain requested SupportBatchRequests support type
	*/
	thomsonreuters::ema::access::UInt32 getSupportBatchRequests() const;

	/** method to obtain requested SupportEnhancedSymbolList support type
	*/
	thomsonreuters::ema::access::UInt32 getSupportEnhancedSymbolList() const;

	/** method to obtain requested SupportOMMPost support type
	*/
	bool getSupportOMMPost() const;

	/** method to obtain requested SupportOptimizedPauseResume support type
	*/
	bool getSupportOptimizedPauseResume() const;

	/** method to obtain requested SupportProviderDictionaryDownload support type
	*/
	bool getSupportProviderDictionaryDownload() const;

	/** method to obtain requested SupportViewRequests support type
	*/
	bool getSupportViewRequests() const;

	/** method to obtain requested SupportStandby support type
	*/
	bool getSupportStandby() const;

	/** method to obtain if message is set with Solicited
	*/
	bool getSolicited() const;

	/** method to obtain if message is set with clearCache
	*/
	bool getClearCache() const;

	/** method to obtain authentication extended buffer
	*/
	const thomsonreuters::ema::access::EmaBuffer& getAuthenticationExtended() const;

	/** method to obtain authenticationTTReissue
	*/
	const thomsonreuters::ema::access::UInt64& getAuthenticationTTReissue() const;

	/** method to obtain authenticationErrorCode
	*/
	const thomsonreuters::ema::access::UInt64& getAuthenticationErrorCode() const;

	/** method to obtain authenticationErrorText
	*/
	const thomsonreuters::ema::access::EmaString& getAuthenticationErrorText() const;

	/** method to obtain name
	*/
	const thomsonreuters::ema::access::EmaString& getName() const;

	/** method to obtain nameType
	*/
	const thomsonreuters::ema::access::UInt32& getNameType() const;

	/** method to obtain state
	*/
	const thomsonreuters::ema::access::OmmState& getState() const;

	/** method to obtain seqNum
	*/
	const thomsonreuters::ema::access::UInt32& getSeqNum() const;

	/** Returns a string representation of the class instance.
	@return string representation of the class instance
	*/
	const thomsonreuters::ema::access::EmaString& toString() const;
	//@}

private:

	LoginRefreshImpl* _pLoginRefreshImpl;
};


/**
@class thomsonreuters::ema::domain::login::LoginStatus Login.h "Domain/Login/Include/Login.h"
@brief LoginStatus is a helper class to provide ease of use for encoding and decoding of RDM defined login Status attributes.

@see EmaString,
ElementList
*/

class EMA_ACCESS_API LoginStatus
{
public:

	///@name Constructor
	//@{
	/** Default constructor setting everything to the RDM specified defaults
	*/
	LoginStatus();

	/** Copy constructor
	*/
	LoginStatus(const LoginStatus&);

	/** Constructor initialized with StatusMsg
	\remark allows easy decoding and encoding of login Status attributes
	*/
	LoginStatus(const thomsonreuters::ema::access::StatusMsg&);
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~LoginStatus();
	//@}

	///@name Operations
	//@{
	/** Sets all values to defaults defined in RDM Usage Guide.
	@return reference to this object
	*/
	LoginStatus& clear();

	/** Assignment operator
	@return reference to this object
	*/
	LoginStatus& operator=(const LoginStatus&);

	/** Sets LoginStatus with StatusMsg
	\remark allows easy decoding of login Status attributes
	@return reference to this object
	*/
	LoginStatus& message(const thomsonreuters::ema::access::StatusMsg&);

	/** method to set authenticationErrorCode
	@return reference to this object
	*/
	LoginStatus& authenticationErrorCode(const thomsonreuters::ema::access::UInt64& value);

	/** method to set authenticationErrorText
	@return reference to this object
	*/
	LoginStatus& authenticationErrorText(const thomsonreuters::ema::access::EmaString& value);

	/** method to set name
	@return reference to this object
	*/
	LoginStatus& name(const thomsonreuters::ema::access::EmaString&);

	/** method to set name type
	@return reference to this object
	*/
	LoginStatus& nameType(const thomsonreuters::ema::access::UInt32&);

	/** method to set state
	@return reference to this object
	*/
	LoginStatus& state(const thomsonreuters::ema::access::OmmState::StreamState&, const thomsonreuters::ema::access::OmmState::DataState, const thomsonreuters::ema::access::UInt8&, const thomsonreuters::ema::access::EmaString&);

	//@}

	///@name Accessors
	//@{

	/** method to check if authenticationErrorCode is set
	@return true if authenticationErrorCode is set
	*/
	bool hasAuthenticationErrorCode() const;

	/** method to check if authenticationErrorText is set
	@return true if authenticationErrorText is set
	*/
	bool hasAuthenticationErrorText() const;

	/** method to check if name is set
	@return true if name is set
	*/
	bool hasName() const;

	/** method to check if nameType is set
	@return true if nameType is set
	*/
	bool hasNameType() const;

	/** method to check if state is set
	@return true if state is set
	*/
	bool hasState() const;

	/** method to obtain StatusMsg out of LoginStatus
	@return StatusMsg version of LoginStatus
	*/
	const thomsonreuters::ema::access::StatusMsg& getMessage() const;

	/** method to obtain authenticationErrorCode
	*/
	const thomsonreuters::ema::access::UInt64& getAuthenticationErrorCode() const;

	/** method to obtain authenticationErrorText
	*/
	const thomsonreuters::ema::access::EmaString& getAuthenticationErrorText() const;

	/** method to obtain name
	*/
	const thomsonreuters::ema::access::EmaString& getName() const;

	/** method to obtain nameType
	*/
	const thomsonreuters::ema::access::UInt32& getNameType() const;

	/** method to obtain state
	*/
	const thomsonreuters::ema::access::OmmState& getState() const;

	/** Returns a string representation of the class instance.
	@return string representation of the class instance
	*/
	const thomsonreuters::ema::access::EmaString& toString() const;
	//@}

private:

	LoginStatusImpl* _pLoginStatusImpl;
};

};

}

}

}

}

#endif // __thomsonreuters_ema_domain_Login_h
