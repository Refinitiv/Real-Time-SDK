/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmJsonConverterException_h
#define __rtsdk_ema_access_OmmJsonConverterException_h

 /**
	 @class rtsdk::ema::access::OmmJsonConverterException OmmJsonConverterException.h "Access/Include/OmmJsonConverterException.h"
	 @brief OmmJsonConverterException is thrown when EMA fails to perform for RWF/JSON conversion.
	 
	 \remark All methods in this class are \ref SingleThreaded.

	 @see OmmException,
		 SessionInfo,
		 OmmConsumerErrorClient,
		 OmmProviderErrorClient
 */

#include "Access/Include/OmmException.h"
#include "Access/Include/SessionInfo.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmJsonConverterException : public OmmException
{
public:

	/** @enum ErrorCode
		An enumeration representing error codes for handling the exception.
	*/
	enum ErrorCode
	{
		NoErrorEnum = 0,                        /*!< No specific error code. */

		FailureEnum = -1,                       /*!< General failure. */

		NoBuffersEnum = -4,                     /*!< There are no buffers available from the buffer pool. */

		BufferTooSmallEnum = -21,               /*!< The buffer provided does not have sufficient space to perform the operation. */

		InvalidArgumentEnum = -22,              /*!< An invalid argument was provided. */

		EncodingUnavaliableEnum = -23,          /*!< No encoder is available for the data type specified. */

		UnsupportedDataTypeEnum = -24,          /*!< The data type is unsupported, may indicate invalid containerType or primitiveType specified. */

		UnexpectedEncoderCallEnum = -25,        /*!< An encoder was used in an unexpected sequence. */

		IncompleteDataEnum = -26,               /*!< Not enough data was provided. */

		SetDefNotProvidedEnum = -27,            /*!< A Database containing the Set Definition for encoding the desired set was not provided. */

		InvalidDataEnum = -29,                  /*!< Invalid data provided to function. */

		IllegalLocalSetDefEnum = -30,           /*!< Set definition is not valid. */

		TooManyLocalSetDefsEnum = -31,          /*!< Maximum number of set definitions has been exceeded. */

		DuplicateLocalSetDefsEnum = -32,        /*!< A duplicate set definition has been received. */

		IteratorOverrunEnum = -33,              /*!< Iterator is nested too deeply. There is a limit of 16 levels. */

		ValueOutOfRangeEnum = -34,              /*!< A value being encoded into a set is outside of the valid range of the type given by that set. */

		DictDuplicateEnumValueEnum = -35,       /*!< A display string had multiple enumerated values that correspond to it. */
	};

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmJsonConverterExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	const EmaString& getText() const;

	/** Returns an error code to describe the error case defined in the ErrorCode enum.
	@return an error code
	*/
	Int32 getErrorCode() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns session information.
	@return SessionInfo with additional session information
	*/
	virtual const SessionInfo& getSessionInfo() const = 0;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmJsonConverterException();
	//@}

protected:

	OmmJsonConverterException();

	OmmJsonConverterException(const OmmJsonConverterException&);
	OmmJsonConverterException& operator=(const OmmJsonConverterException&);

	Int32 _errorCode;
};

}

}

}

#endif // __rtsdk_ema_access_OmmJsonConverterException_h

