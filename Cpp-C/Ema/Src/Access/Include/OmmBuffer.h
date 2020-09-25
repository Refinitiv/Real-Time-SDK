/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmBuffer_h
#define __refinitiv_ema_access_OmmBuffer_h

/**
	@class refinitiv::ema::access::OmmBuffer OmmBuffer.h "Access/Include/OmmBuffer.h"
	@brief OmmBuffer represents a binary buffer value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::BufferEnum :
				const EmaBuffer& value = static_cast< const OmmBuffer& >( rcvdData ).getBuffer();
				break;
			}
	}

	\endcode
	
	\remark OmmBuffer is a read only class.
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

class OmmBufferDecoder;

class EMA_ACCESS_API OmmBuffer : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::BufferEnum
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
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns Buffer.
		@return binary buffer contained in EmaBuffer
	*/
	const EmaBuffer& getBuffer() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 indent ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmBuffer();
	virtual ~OmmBuffer();
	OmmBuffer( const OmmBuffer& );
	OmmBuffer& operator=( const OmmBuffer& );

	OmmBufferDecoder*		_pDecoder;
	UInt64					_space[13];
};

}

}

}

#endif // __refinitiv_ema_access_OmmBuffer_h
