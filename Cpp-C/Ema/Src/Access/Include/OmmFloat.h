/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmFloat_h
#define __refinitiv_ema_access_OmmFloat_h

/**
	@class refinitiv::ema::access::Float OmmFloat.h "Access/Include/OmmFloat.h"
	@brief OmmFloat represents float value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::FloatEnum :
				float value = static_cast< const OmmFloat& >( rcvdData ).getFloat();
				break;
			}
	}

	\endcode
	
	\remark OmmFloat is a read only class.
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

class OmmFloatDecoder;

class EMA_ACCESS_API OmmFloat : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::FloatEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if received data is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance (e.g., "1234.12" )
	*/
	const EmaString& toString() const;

	/** Returns Float.
		@return value of float
	*/
	float getFloat() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmFloat();
	virtual ~OmmFloat();
	OmmFloat( const OmmFloat& );
	OmmFloat& operator=( const OmmFloat& );

	OmmFloatDecoder*		_pDecoder;
	UInt64					_space[12];
};

}

}

}

#endif // __refinitiv_ema_access_OmmFloat_h
