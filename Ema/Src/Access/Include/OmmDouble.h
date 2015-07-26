/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmDouble_h
#define __thomsonreuters_ema_access_OmmDouble_h

/**
	@class thomsonreuters::ema::access::OmmDouble OmmDouble.h "Access/Include/OmmDouble.h"
	@brief OmmDouble represents double in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::DoubleEnum :
				double value = static_cast< const OmmDouble& >( rcvdData ).getDouble();
				break;
			}
	}

	\endcode
	
	\remark OmmDouble is a read only class.
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

class OmmDoubleDecoder;

class EMA_ACCESS_API OmmDouble : public Data
{
public :

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@reeturn DataType::DoubleEnum
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
		@return string representation of the class instance (e.g., "-1234.9876" )
	*/
	const EmaString& toString() const;

	/** Returns Double.
		@return value of double
	*/
	double getDouble() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;

	OmmDouble();
	virtual ~OmmDouble();
	OmmDouble( const OmmDouble& );
	OmmDouble& operator=( const OmmDouble& );

	OmmDoubleDecoder*		_pDecoder;
	UInt64					_space[12];
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmDouble_h
