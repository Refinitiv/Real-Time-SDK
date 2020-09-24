/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmConsumerErrorClient_h
#define __rtsdk_ema_access_OmmConsumerErrorClient_h

/**
	@class rtsdk::ema::access::OmmConsumerErrorClient OmmConsumerErrorClient.h "Access/Include/OmmConsumerErrorClient.h"
	@brief OmmConsumerErrorclient class provides callback mechanism used in place of exceptions.

	By default OmmConsumer class throws exceptions if a usage error occurs. Specifying OmmConsumerErrorClient
	on the constructor of OmmConsumer overwrites this behaviour. Instead of throwing exceptions, respective
	callback method on OmmConsumerErrorClient will be invoked.

	\remark Thread safety of all the methods in this class depends on user's implementation

	@see OmmConsumer,
		OmmException,
		OmmInvalidUsageException,
		OmmInvalidHandleExeption,
		OmmMemoryExhaustionException,
		OmmInaccessibleLogFile,
		OmmSystemException,
		OmmJsonConverterException,
		ConsumerSessionInfo,
		EmaString
*/

#include "Access/Include/Common.h"

namespace rtsdk {

namespace ema {

namespace access {

class EmaString;
class ConsumerSessionInfo;

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
	virtual void onInvalidHandle( UInt64 handle, const EmaString& text );

	/** Invoked when log file is inaccessible. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] filename identifies file name that was not able to b open
		@param[out] text specifies associated error text
		@return void
	*/
	virtual void onInaccessibleLogFile( const EmaString& filename, const EmaString& text );

	/** Invoked in the case of memory exhaustion. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onMemoryExhaustion( const EmaString& text);
	
	/** Invoked in the case of invalid usage. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onInvalidUsage( const EmaString& text);

	/** Invoked in the case of invalid usage. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		\remark This method provides an additional error code for applications to check and handle the error appropriately.
		\remark The applications should override only one of the onInvalidUsage() method to avoid receiving two callback calls for an invalid usage error.
		@param[out] text specifies associated error text
		@param[out] errorCode specifies associated error code
		@return void
	*/
	virtual void onInvalidUsage( const EmaString& text, Int32 errorCode );

	/** Invoked in the case of an underlying system error. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] code specifies system exception code
		@param[out] specifies system exception pointer
		@param[out] text specifies associated error text
		@return void
	*/ 
	virtual void onSystemError( Int64 code, void* ptr, const EmaString& text );

	/** Invoked in the case of Json converter error. Requires OmmConsumer constructor to have an OmmConsumerErrorClient.
		@param[out] text specifies associated error text
		@param[out] errorCode specifies associated error code
		@param[out] sessionInfo specifies associated session information
		@return void
	*/
	virtual void onJsonConverter( const EmaString& text, Int32 errorCode, const ConsumerSessionInfo& sessionInfo );
	//@}

	///@name Destructor
	//@{
	virtual ~OmmConsumerErrorClient();
	//@}

protected:

	OmmConsumerErrorClient();

private:

	OmmConsumerErrorClient( const OmmConsumerErrorClient& );
	OmmConsumerErrorClient& operator=( const OmmConsumerErrorClient& );
};

}

}

}

#endif // __rtsdk_ema_access_OmmConsumerErrorClient_h
