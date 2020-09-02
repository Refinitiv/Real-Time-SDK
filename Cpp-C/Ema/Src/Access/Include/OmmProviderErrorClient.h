/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommProviderErrorClient_h
#define __thomsonreuters_ema_access_ommProviderErrorClient_h

/**
	@class rtsdk::ema::access::OmmProviderErrorClient OmmProviderErrorClient.h "Access/Include/OmmProviderErrorClient.h"
	@brief OmmProviderErrorClient class provides callback mechanism used in place of exceptions.

	By default OmmProvider class throws exceptions if usage errors occur. Specifying OmmProviderErrorClient
	on the constructor of OmmProvider overwrites this behaviour. Instead of throwing exceptions, respective
	callback methods on OmmProviderErrorClient will be invoked.

	\remark Thread safety of all the methods in this class depends on user's implementation.

	@see OmmProvider,
		OmmException,
		OmmInvalidUsageException,
		OmmInvalidHandleExeption,
		OmmMemoryExhaustionException,
		OmmInaccessibleLogFile,
		OmmSystemException,
		OmmJsonConverterException,
		ProviderSessionInfo,
		EmaString
*/

#include "Access/Include/Common.h"

namespace rtsdk {

namespace ema {

namespace access {

class EmaString;
class ProviderSessionInfo;

class EMA_ACCESS_API OmmProviderErrorClient
{
public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving an invalid handle. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		@param[out] handle value of the handle that is invalid
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInvalidHandle( UInt64 handle, const EmaString& text );

	/** Invoked when log file is inaccessible. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		@param[out] filename identifies file name that was not able to b open
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInaccessibleLogFile( const EmaString& filename, const EmaString& text );

	/** Invoked in the case of memory exhaustion. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onMemoryExhaustion( const EmaString& text );
	
	/** Invoked in the case of invalid usage. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onInvalidUsage( const EmaString& text );

	/** Invoked in the case of invalid usage. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		\remark This method provides an additional error code for applications to check and handle the error appropriately.
		\remark The applications should override only one of the onInvalidUsage() method to avoid receiving two callback calls for an invalid usage error.
		@param[out] text specifies associated error text
		@param[out] errorCode specifies associated error code
		@return void
	*/
	virtual void onInvalidUsage( const EmaString& text, Int32 errorCode );

	/** Invoked in the case of an underlying system error. Requires OmmProvider constructor to have an OmmProviderErrorClient.
		@param[out] code specifies system exception code
		@param[out] specifies system exception pointer
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onSystemError( Int64 code, void* ptr, const EmaString& text );

	/** Invoked in the case of Json converter error. Requires OmmProvider constructor to have an OmmProviderErrorClient.
	@param[out] text specifies associated error text
	@param[out] errorCode specifies associated error code
	@param[out] sessionInfo specifies associated session information
	@return void
	*/
	virtual void onJsonConverter( const EmaString& text, Int32 errorCode, const ProviderSessionInfo& sessionInfo );
	//@}

	///@name Destructor
	//@{
	virtual ~OmmProviderErrorClient();
	//@}

protected:

	OmmProviderErrorClient();

private:

	OmmProviderErrorClient( const OmmProviderErrorClient& );
	OmmProviderErrorClient& operator=( const OmmProviderErrorClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommProviderErrorClient_h
