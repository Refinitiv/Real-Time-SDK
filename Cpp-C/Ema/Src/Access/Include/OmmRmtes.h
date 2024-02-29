/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmRmtes_h
#define __refinitiv_ema_access_OmmRmtes_h

/**
	@class refinitiv::ema::access::OmmRmtes OmmRmtes.h "Access/Include/OmmRmtes.h"
	@brief OmmRmtes represents Rmtes string value in Omm.

	\code

	void decodeData( const Data& rcvdData )
	{
		if ( rcvdData.getCode() != Data::BlankEnum )
			switch ( rcvdData.getDataType() )
			{
			case DataType::RmtesEnum :
				const EmaBuffer& value = static_cast< const OmmRmtes& >( rcvdData ).getRmtes();
				break;
			}
	}

	\endcode
	
	\remark OmmRmtes is a read only class.
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

class RmtesBuffer;
class OmmRmtesDecoder;

class EMA_ACCESS_API OmmRmtes : public Data
{
public :

	///@name Accessors
	//@{	
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::RmtesEnum
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

	/** Returns Rmtes.
		@return RmtesBuffer containing Rmtes data
	*/
	const RmtesBuffer& getRmtes() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	const EmaString& toString( UInt64 ) const;

	Decoder& getDecoder();
	bool hasDecoder() const;
	Decoder& setDecoder( Decoder& );
	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmRmtes();
	virtual ~OmmRmtes();
	OmmRmtes( const OmmRmtes& );
	OmmRmtes& operator=( const OmmRmtes& );

	OmmRmtesDecoder*		_pDecoder;
	UInt64					_space[34];
};

}

}

}

#endif // __refinitiv_ema_access_OmmRmtes_h
