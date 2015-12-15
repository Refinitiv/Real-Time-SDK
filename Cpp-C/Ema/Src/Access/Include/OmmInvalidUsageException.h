/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmInvalidUsageException_h
#define __thomsonreuters_ema_access_OmmInvalidUsageException_h

/**
	@class thomsonreuters::ema::access::OmmInvalidUsageException OmmInvalidUsageException.h "Access/Include/OmmInvalidUsageException.h"
	@brief OmmInvalidUsageException is thrown when application violates usage of EMA interfaces.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient
*/

#include "Access/Include/OmmException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInvalidUsageException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmInvalidUsageExceptionEnum
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
	virtual ~OmmInvalidUsageException();
	//@}

protected :

	OmmInvalidUsageException();

	OmmInvalidUsageException( const OmmInvalidUsageException& );
	OmmInvalidUsageException& operator=( const OmmInvalidUsageException& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmInvalidUsageException_h
