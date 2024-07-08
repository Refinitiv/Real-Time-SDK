/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidUsageException_h
#define __refinitiv_ema_access_OmmInvalidUsageException_h

/**
	@class refinitiv::ema::access::OmmInvalidUsageException OmmInvalidUsageException.h "Access/Include/OmmInvalidUsageException.h"
	@brief OmmInvalidUsageException is thrown when application violates usage of EMA interfaces.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInvalidUsageException : public OmmException
{
public :

	/** @enum ErrorCode
		An enumeration representing error codes for handling the exception. 
	*/
	enum ErrorCode
	{
		NoErrorEnum = 0,                        /*!< No specific error code. */

		FailureEnum = -1,                       /*!< General failure. */

		NoBuffersEnum = -4,                     /*!< There are no buffers available from the buffer pool. */

		BufferTooSmallEnum = -21,               /*!< The buffer provided (or the remaining buffer space for message packing) does not have sufficient space to perform the operation. */

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

		PacketGapDetectedEnum = -61,            /*!< Multicast Transport Warning: An unrecoverable packet gap was detected and some content may have been lost. */

		SlowReaderEnum = -62,                   /*!< Multicast Transport Warning: Application is consuming more slowly than data is being provided.  Gaps are likely. */

		CongestionDetectedEnum = -63,           /*!< Multicast Transport Warning: Network congestion detected.  Gaps are likely. */

		PersistenceFullEnum = -91,              /*!< Tunnel Stream Failure: This message could not be sent because no space was available to persist it.*/

		/* EMA error codes */
		InvalidOperationEnum = -4048,           /*!< Invalid user's operation. */

		NoActiveChannelEnum = -4049,            /*!< No active channel. */

		UnSupportedChannelTypeEnum = - 4050,    /*!< Unsupported channel type. */

		UnSupportedServerTypeEnum = -4051,      /*!< Unsupported server type. */

		LoginRequestTimeOutEnum = -4052,        /*!< Login request timeout. */

		LoginRequestRejectedEnum = -4053,       /*!< Login request rejected from connected peer. */

		DirectoryRequestTimeOutEnum = -4054,    /*!< Directory request timeout. */

		DictionaryRequestTimeOutEnum = -4055,   /*!< Dictionary request timeout. */

		InternalErrorEnum = -4060               /*!< Internal Error in EMA. */
	};

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

	/** Returns an error code to describe the error case defined in the ErrorCode enum. 
		@return an error code
	*/
	Int32 getErrorCode() const;
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

	Int32 _errorCode;
};

}

}

}

#endif // __refinitiv_ema_access_OmmInvalidUsageException_h
