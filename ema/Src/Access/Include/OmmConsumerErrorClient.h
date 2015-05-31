/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerErrorClient_h
#define __thomsonreuters_ema_access_OmmConsumerErrorClient_h

/**
	@class thomsonreuters::ema::access::OmmConsumerErrorClient OmmConsumerErrorClient.h "Access/Include/OmmConsumerErrorClient.h"
	@brief OmmConsumerErrorclient class provides callback mechanism used in place of exceptions.

	By default OmmConsumer class throws exceptions if a usage error occurs. Specifying OmmConsumerErrorClient
	on the constructor of OmmConsumer ovewrites this behaviour. Instead of throwing exceptions, respective
	callback method on OmmConsumerErrorClient will be invoked.

	\remark Thread safety of all the methods in this class depends on user's implementation

	@see OmmConsumer,
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

class EMA_ACCESS_API OmmConsumerErrorClient
{
public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving an invalid handle. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] handle value of the handle that is invalid
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInvalidHandle( UInt64 handle, const EmaString& text ) {}

	/** Invoked when log file is inaccessible. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] filename identifies file name that was not able to b open
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInaccessibleLogFile( const EmaString& filename, const EmaString& text ) {}

	/** Invoked in the case of memory exhaustion. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onMemoryExhaustion( const EmaString& text) {}
	
	/** Invoked in the case of invalid usage. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onInvalidUsage( const EmaString& text) {}

	/** Invoked in the case of an underlying system error. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] code specifies system exception code
		@param[out] specifies system exception pointer
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onSystemError( Int64 code, void* ptr, const EmaString& text ) {}
	//@}

	///@name Destructor
	//@{
	virtual ~OmmConsumerErrorClient() {}
	//@}

protected:

	OmmConsumerErrorClient() {}

private:

	OmmConsumerErrorClient( const OmmConsumerErrorClient& );
	OmmConsumerErrorClient& operator=( const OmmConsumerErrorClient& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerErrorClient_h
