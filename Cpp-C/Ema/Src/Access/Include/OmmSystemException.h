/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmSystemException_h
#define __rtsdk_ema_access_OmmSystemException_h

/**
	@class rtsdk::ema::access::OmmSystemException OmmSystemException.h "Access/Include/OmmSystemException.h"
	@brief OmmSystemException represents exceptions thrown by operating system.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmSystemException : public OmmException
{
public :

	///@name Accessors
	//@{	
	/** Returns ExceptionType.
		@return OmmException::OmmSystemExceptionEnum
	*/
	ExceptionType getExceptionType() const;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	const EmaString& getText() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns SystemExceptionCode.
		@return code of the system exception
	*/
	Int64 getSystemExceptionCode() const;

	/** Returns SystemExceptionAddress.
		@return address of the system exception
	*/
	void* getSystemExceptionAddress() const;
	//@}

protected :

	Int64		_exceptionCode;
	void*		_exceptionAddress;

	OmmSystemException();
	virtual ~OmmSystemException();

	OmmSystemException( const OmmSystemException& );
	OmmSystemException& operator=( const OmmSystemException& );
};

}

}

}

#endif // __rtsdk_ema_access_OmmSystemException_h
