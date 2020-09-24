/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmUnsupportedDomainTypeException_h
#define __rtsdk_ema_access_OmmUnsupportedDomainTypeException_h

/**
	@class rtsdk::ema::access::OmmUnsupportedDomainTypeException OmmUnsupportedDomainTypeException.h "Access/Include/OmmUnsupportedDomainTypeException.h"
	@brief OmmUnsupportedDomainTypeException is thrown when a domain type value is greater than 255.

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

class EMA_ACCESS_API OmmUnsupportedDomainTypeException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmUnsupportedDomainTypeExceptionEnum
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

	/** Returns the invalid DomainType. DomainTypes must be less than 256.
		@return domain type value that caused the exception
	*/
	UInt16 getDomainType() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmUnsupportedDomainTypeException();
	//@}

protected :

	OmmUnsupportedDomainTypeException();

	UInt16			_domainType;

	OmmUnsupportedDomainTypeException( const OmmUnsupportedDomainTypeException& );
	OmmUnsupportedDomainTypeException& operator=( const OmmUnsupportedDomainTypeException& );
};

}

}

}

#endif // __rtsdk_ema_access_OmmUnsupportedDomainTypeException_h
