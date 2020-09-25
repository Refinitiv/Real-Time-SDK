/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidHandleException_h
#define __refinitiv_ema_access_OmmInvalidHandleException_h

/**
	@class rtsdk::ema::access::OmmInvalidHandleException OmmInvalidHandleException.h "Access/Include/OmmInvalidHandleException.h"
	@brief OmmInvalidHandleException is thrown when application passes in an invalid handle to OmmConsumer.

	OmmConsumer uses UInt64 values, called handles to identify individual item streams.
	OmmConsumer validates each passed in handle against all open and known handles.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"

namespace rtsdk {

namespace ema {

namespace access {

class EmaString;

class EMA_ACCESS_API OmmInvalidHandleException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmInvalidHandleExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	const EmaString& getText() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns the invalid handle.
		@return UInt64 value of handle causing exception
	*/
	UInt64 getHandle() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmInvalidHandleException();
	//@}

protected :

	UInt64		_handle;

	OmmInvalidHandleException( UInt64 );

	OmmInvalidHandleException( const OmmInvalidHandleException& );
	OmmInvalidHandleException& operator=( const OmmInvalidHandleException& );

private :

	OmmInvalidHandleException();
};

}

}

}

#endif // __refinitiv_ema_access_OmmInvalidHandleException_h
