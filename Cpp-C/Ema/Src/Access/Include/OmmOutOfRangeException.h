/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmOutOfRangeException_h
#define __refinitiv_ema_access_OmmOutOfRangeException_h

/**
	@class refinitiv::ema::access::OmmOutOfRangeException OmmOutOfRangeException.h "Access/Include/OmmOutOfRangeException.h"
	@brief OmmOutOfRangeException is thrown when a passed in method argument is out of range.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmOutOfRangeException : public OmmException
{
public :
	
	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmOutOfRangeExceptionEnum
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
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmOutOfRangeException();
	//@}

protected :

	OmmOutOfRangeException();

	OmmOutOfRangeException( const OmmOutOfRangeException& );
	OmmOutOfRangeException& operator=( const OmmOutOfRangeException& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmOutOfRangeException_h
