/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmEnum_h
#define __thomsonreuters_ema_access_OmmEnum_h

/**
	@class thomsonreuters::ema::access::OmmEnum OmmEnum.h "Access/Include/OmmEnum.h"
	@brief OmmEnum represents UInt16 value in Omm. The enumeration is the meaning of the UInt16 value.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::EnumEnum :
				UInt16 value = static_cast< const OmmEnum& >( rcvdData ).getEnum();
				break;
			}
	}

	\endcode
	
	\remark OmmEnum is a read only class.
	\remark The usage of this class is limited to downcast operation only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmEnumDecoder;

class EMA_ACCESS_API OmmEnum : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::EnumEnum
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
		@return string representation of the class instance (e.g., "255" )
	*/
	const EmaString& toString() const;

	/** Returns Enum.
		@throw OmmInvalidUsageException if getCode returns Data::BlankEnum
		@return value of UInt16
	*/
	UInt16 getEnum() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;

	OmmEnum();
	virtual ~OmmEnum();
	OmmEnum( const OmmEnum& );
	OmmEnum& operator=( const OmmEnum& );

	OmmEnumDecoder*		_pDecoder;
	UInt64				_space[12];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmEnum_h
