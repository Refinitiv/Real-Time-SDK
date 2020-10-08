/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmUtf8_h
#define __refinitiv_ema_access_OmmUtf8_h

/**
	@class refinitiv::ema::access::OmmUtf8 OmmUtf8.h "Access/Include/OmmUtf8.h"
	@brief OmmUtf8 represents Utf8 string value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::Utf8Enum :
				const EmaBuffer& value = static_cast< const OmmUtf8& >( rcvdData ).getUtf8();
				break;
			}
	}

	\endcode
	
	\remark OmmUtf8 is a read only class.
	\remark The usage of this class is limited to downcast operation only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmUtf8Decoder;

class EMA_ACCESS_API OmmUtf8 : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::Utf8Enum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if received data is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the object hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance (e.g., "-1234" )
	*/
	const EmaString& toString() const;

	/** Returns Utf8.
		@return Utf8 string value contained in EmaBuffer
	*/
	const EmaBuffer& getUtf8() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmUtf8();
	virtual ~OmmUtf8();
	OmmUtf8( const OmmUtf8& );
	OmmUtf8& operator=( const OmmUtf8& );

	OmmUtf8Decoder*			_pDecoder;
	UInt64					_space[13];
};

}

}

}

#endif // __refinitiv_ema_access_OmmUtf8_h
