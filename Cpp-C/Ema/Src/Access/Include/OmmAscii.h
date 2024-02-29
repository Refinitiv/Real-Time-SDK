/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmAscii_h
#define __refinitiv_ema_access_OmmAscii_h

/**
	@class refinitiv::ema::access::OmmAscii OmmAscii.h "Access/Include/OmmAscii.h"
	@brief OmmAscii represents Ascii string value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::AsciiEnum :
				const EmaString& value = static_cast< const OmmAscii& >( rcvdData ).getAscii();
				break;
			}
	}

	\endcode
	
	\remark OmmAscii is a read only class.
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

class OmmAsciiDecoder;

class EMA_ACCESS_API OmmAscii : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DaytaType::AsciiEnum
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
		@return string representation of the class instance (e.g., "this is ascii string" )
	*/
	const EmaString& toString() const;

	/** Returns Ascii.
		@return Ascii string value contained in EmaString
	*/
	const EmaString& getAscii() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 indent ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmAscii();
	virtual ~OmmAscii();
	OmmAscii( const OmmAscii& );
	OmmAscii& operator=( const OmmAscii& );

	OmmAsciiDecoder*		_pDecoder;
	UInt64					_space[17];
};

}

}

}

#endif // __refinitiv_ema_access_OmmAscii_h
