/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmMemoryExhaustionException_h
#define __refinitiv_ema_access_OmmMemoryExhaustionException_h

/**
	@class refinitiv::ema::access::OmmMemoryExhaustionException OmmMemoryExhaustionException.h "Access/Include/OmmMemoryExhaustionException.h"
	@brief OmmMemoryExhaustionException represents out of memory exceptions.

	OmmMemoryExhaustionException are thrown when malloc() returns a null pointer,
	operator new throws std::bad_alloc exception.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmMemoryExhaustionException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmMemoryExhaustionExceptionEnum
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
	virtual ~OmmMemoryExhaustionException();
	//@}

protected :

	OmmMemoryExhaustionException();

	OmmMemoryExhaustionException(const OmmMemoryExhaustionException&);
	OmmMemoryExhaustionException& operator=(const OmmMemoryExhaustionException&);
};

}

}

}

#endif // __refinitiv_ema_access_OmmMemoryExhaustionException_h
