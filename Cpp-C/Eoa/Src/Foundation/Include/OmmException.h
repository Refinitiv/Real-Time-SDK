/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommexception_h
#define __thomsonreuters_eoa_foundation_ommexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmException OmmException.h "Foundation/Include/OmmException.h"
	@brief OmmException is a parent class for all exception types thrown by EOA.

	This class provides all the common functionalities and methods used by the inheriting classes.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmInvalidUsageException,
		OmmMemoryExhaustionException
*/

#define MAX_SIZE 2048
#define PADDING 256

#include "Foundation/Include/Common.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaString;

class EOA_FOUNDATION_API OmmException
{
public :

	/** @enum ExceptionType
		An enumeration representing exception type.
	*/
	enum ExceptionType
	{
		OmmInvalidUsageExceptionEnum,				/*!< Indicates invalid usage exception */

		OmmOutOfRangeExceptionEnum,					/*!< Indicates out of range exception */

		OmmMemoryExhaustionExceptionEnum,			/*!< Indicates memory exhaustion exception */

		OmmInaccessibleLogFileExceptionEnum,		/*!< Indicates inaccessible log file exception */

		OmmUnsupportedDomainTypeExceptionEnum		/*!< Indicates unsupported domain type exception */
	};

	///@name Accessors
	//@{
	/** Returns the ExceptionType value as a string format.
		@return string representation of this object's exception type as string
	*/
	const EoaString& getExceptionTypeAsString() const;

	/** Returns ExceptionType.
		@return exception type value
	*/
	virtual ExceptionType getExceptionType() const = 0;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	virtual const EoaString& getText() const = 0;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	virtual const EoaString& toString() const = 0;

	/** Operator const char* overload.
	*/
	operator const char*() const;
	//@}

protected :

	OmmException();
	virtual ~OmmException();

	OmmException& statusText( const EoaString& statusText );
	OmmException& statusText( const char* statusText );

	const EoaString& toStringInt() const;

	UInt32						_errorTextLength;
	mutable char				_errorText[MAX_SIZE];
	mutable char				_space[MAX_SIZE + PADDING];

	OmmException( const OmmException& );
	OmmException& operator=( const OmmException& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommexception_h
