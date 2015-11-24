/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommNiProviderErrorClient_h
#define __thomsonreuters_ema_access_ommNiProviderErrorClient_h

/**
	@class thomsonreuters::ema::access::OmmNiProviderErrorClient OmmNiProviderErrorClient.h "Access/Include/OmmNiProviderErrorClient.h"
	@brief OmmNiProviderErrorclient class provides callback mechanism used in place of exceptions.

	By default OmmNiProvider class throws exceptions if usage errors occur. Specifying OmmNiProviderErrorClient
	on the constructor of OmmNiProvider overwrites this behaviour. Instead of throwing exceptions, respective
	callback methods on OmmNiProviderErrorClient will be invoked.

	\remark Thread safety of all the methods in this class depends on user's implementation

	@see OmmNiProvider,
		OmmException,
		OmmInvalidUsageException,
		OmmInvalidHandleExeption,
		OmmMemoryExhaustionException,
		OmmInaccessibleLogFile,
		OmmSystemException,
		EmaString
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EmaString;

class EMA_ACCESS_API OmmNiProviderErrorClient
{
public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving an invalid handle. Requires OmmNiProvider constructor to have an OmmNiProviderErrorClient.
		@param[out] handle value of the handle that is invalid
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInvalidHandle( UInt64 handle, const EmaString& text ) {}

	/** Invoked when log file is inaccessible. Requires OmmNiProvider constructor to have an OmmNiProviderErrorClient.
		@param[out] filename identifies file name that was not able to b open
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInaccessibleLogFile( const EmaString& filename, const EmaString& text ) {}

	/** Invoked in the case of memory exhaustion. Requires OmmNiProvider constructor to have an OmmNiProviderErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onMemoryExhaustion( const EmaString& text) {}
	
	/** Invoked in the case of invalid usage. Requires OmmNiProvider constructor to have an OmmNiProviderErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onInvalidUsage( const EmaString& text) {}

	/** Invoked in the case of an underlying system error. Requires OmmNiProvider constructor to have an OmmNiProviderErrorClient.
		@param[out] code specifies system exception code
		@param[out] specifies system exception pointer
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onSystemError( Int64 code, void* ptr, const EmaString& text ) {}
	
	//@}

	///@name Destructor
	//@{
	virtual ~OmmNiProviderErrorClient() {}
	//@}

protected:

	OmmNiProviderErrorClient() {}

private:

	OmmNiProviderErrorClient( const OmmNiProviderErrorClient& );
	OmmNiProviderErrorClient& operator=( const OmmNiProviderErrorClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommNiProviderErrorClient_h
