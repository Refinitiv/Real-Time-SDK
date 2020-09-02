/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmUInt_h
#define __thomsonreuters_ema_access_OmmUInt_h

/**
	@class rtsdk::ema::access::OmmUInt OmmUInt.h "Access/Include/OmmUInt.h"
	@brief OmmUInt represents UInt64 value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::UIntEnum :
				UInt64 value = static_cast< const OmmUInt& >( rcvdData ).getUInt();
				break;
			}
	}

	\endcode
	
	\remark OmmUInt is a read only class.
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

class OmmUIntDecoder;

class EMA_ACCESS_API OmmUInt : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::UIntEnum
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
		@return string representation of the class instance (e.g., "1234" )
	*/
	const EmaString& toString() const;

	/** Returns UInt64.
		@return value of UInt64
	*/
	UInt64 getUInt() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmUInt();
	virtual ~OmmUInt();
	OmmUInt( const OmmUInt& );
	OmmUInt& operator=( const OmmUInt& );

	OmmUIntDecoder*		_pDecoder;
	UInt64				_space[12];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmUInt_h
