/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmError_h
#define __thomsonreuters_ema_access_OmmError_h

/**
	@class rtsdk::ema::access::OmmError OmmError.h "Access/Include/OmmError.h"
	@brief OmmError represents received Omm data who fails to process properly.

	Objects of OmmError class are returned when an error is detected while processing 
	received data. These objects are used for debugging purposes only.
	
	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::UIntEnum :
				UInt64 value = static_cast< const OmmUInt& >( rcvdData ).getUInt();
				break;

			...

			case DataType::ErrorEnum :
				cout << "Failed to decode data. Error code is: "
					<< static_cast< OmmError& >( rcvdData ).getErrorCodeAsString() << "\n"
					<< "Received data is: "
					<< rcvdData.getAsHex() << "\n";
				break;
			}
	}

	\endcode

	\remark OmmError is a read only class.
	\remark The usage of this class is limited to downcast operation only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmErrorDecoder;

class EMA_ACCESS_API OmmError : public Data
{
public :

	/** @enum ErrorCode
		An enumeration representing decoding error condition.
	*/
	enum ErrorCode
	{
		NoErrorEnum				= 0,		/*!< Indicates no error */

		NoDictionaryEnum		= 1,		/*!< Indicates missing dictionary */

		IteratorSetFailureEnum	= 2,		/*!< Indicates internal iterator set failure */

		IteratorOverrunEnum		= 3,		/*!< Indicates internal iterator overrun failure */

		FieldIdNotFoundEnum		= 4,		/*!< Indicates field id was not found in dictionary */

		IncompleteDataEnum		= 5,		/*!< Indicates incomplete data */

		UnsupportedDataTypeEnum	= 6,		/*!< Indicates unsupported data type */

		NoSetDefinitionEnum		= 7,		/*!< Indicates set defined data is not present */

		UnknownErrorEnum		= 8			/*!< Indicates unknown error */
	};

	///@name Accessors
	//@{
	/** Returns the ErrorCode value as a string format.
		@return string representation of error code (e.g., "IteratorSetFailure" )
	*/
	const EmaString& getErrorCodeAsString() const;
		
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::ErrorEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::NoCodeEnum
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns ErrorCode.
		@return error code
	*/
	ErrorCode getErrorCode() const;
	//@}

private:

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmError();
	virtual ~OmmError();
	OmmError( const OmmError& );
	OmmError& operator=( const OmmError& );

	mutable EmaString		_toString;
	OmmErrorDecoder*		_pDecoder;
	UInt64					_space[9];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmError_h
