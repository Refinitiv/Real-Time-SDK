/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020, 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmException_h
#define __refinitiv_ema_access_OmmException_h

/**
	@class refinitiv::ema::access::OmmException OmmException.h "Access/Include/OmmException.h"
	@brief OmmException is a parent class for all exception types thrown by EMA.

	This class provides all the common functionalities and methods used by the inheriting classes.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmInaccessibleLogFileException,
		OmmInvalidHandleException,
		OmmInvalidUsageException,
		OmmMemoryExhaustionException,
		OmmOutOfRangeException,
		OmmSystemException,
		OmmUnsupportedDomainTypeException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#define MAX_SIZE 2048
#define PADDING 264
#define MAX_SIZE_PLUS_PADDING  MAX_SIZE + PADDING  

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class EmaString;

class EMA_ACCESS_API OmmException
{
public :

	/** @enum ExceptionType
		An enumeration representing exception type.
	*/
	enum ExceptionType
	{
		OmmInvalidUsageExceptionEnum,				/*!< Indicates invalid usage exception */

		OmmInvalidConfigurationExceptionEnum,		/*!< Indicates invalid configuration exception */

		OmmSystemExceptionEnum,						/*!< Indicates system exception */

		OmmOutOfRangeExceptionEnum,					/*!< Indicates out of range exception */

		OmmInvalidHandleExceptionEnum,				/*!< Indicates invalid handle exception */

		OmmMemoryExhaustionExceptionEnum,			/*!< Indicates memory exhaustion exception */

		OmmInaccessibleLogFileExceptionEnum,		/*!< Indicates inaccessible log file exception */

		OmmUnsupportedDomainTypeExceptionEnum,		/*!< Indicates unsupported domain type exception */

		OmmJsonConverterExceptionEnum			/*!< Indicates JSON converter exception */
	};

	///@name Accessors
	//@{
	/** Returns the ExceptionType value as a string format.
		@return string representation of this object's exception type as string
	*/
	const EmaString& getExceptionTypeAsString() const;

	/** Returns ExceptionType.
		@return exception type value
	*/
	virtual ExceptionType getExceptionType() const = 0;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	virtual const EmaString& getText() const = 0;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	virtual const EmaString& toString() const = 0;

	/** Operator const char* overload.
		\remark allows std::cout << ommException;
		@return const char* used for printing
	*/
	operator const char*() const;
	//@}

protected :

	OmmException();
	virtual ~OmmException();

	OmmException& statusText( const EmaString& statusText );
	OmmException& statusText( const char* statusText );

	const EmaString& toStringInt() const;

	UInt32						_errorTextLength;
	UInt32						_errorTextPadding; // _errorText buffer is used for placement new and needs to be 64-bit aligned
	mutable char				_errorText[MAX_SIZE];
	mutable char				_space[MAX_SIZE_PLUS_PADDING];

	OmmException( const OmmException& );
	OmmException& operator=( const OmmException& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmException_h
