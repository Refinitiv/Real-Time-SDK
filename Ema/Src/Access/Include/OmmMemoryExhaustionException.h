/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmMemoryExhaustionException_h
#define __thomsonreuters_ema_access_OmmMemoryExhaustionException_h

/**
	@class thomsonreuters::ema::access::OmmMemoryExhaustionException OmmMemoryExhaustionException.h "Access/Include/OmmMemoryExhaustionException.h"
	@brief OmmMemoryExhaustionException represents out of memory exceptions.

	OmmMemoryExhaustionException are thrown when malloc() returns a null pointer,
	operator new throws std::bad_alloc exception.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
		OmmConsumerErrorClient
*/

#include "Access/Include/OmmException.h"

namespace thomsonreuters {

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

#endif // __thomsonreuters_ema_access_OmmMemoryExhaustionException_h
