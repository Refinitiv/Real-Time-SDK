/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_Data_h
#define __thomsonreuters_ema_access_Data_h

/**
	@class thomsonreuters::ema::access::Data Data.h "Access/Include/Data.h"
	@brief Data class is a parent abstract class defining common interfaces for all Data type classes.

	All classes representing OMM Data inherit from this class.

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see EmaString,
		EmaBuffer
*/

#include "Access/Include/EmaBuffer.h"
#include "Access/Include/DataType.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Decoder;
class Encoder;

class EMA_ACCESS_API Data
{
public :

	/** @enum DataCode
		An enumeration representing special state of Data.
	*/
	enum DataCode
	{
		NoCodeEnum	= 0,		/*!< Indicates no special code. Application typically
									processes a valid DataType value. */

		BlankEnum	= 1			/*!< Indicates the value is unspecified. An application
									typically sets the blank code when needing to initialize or clear a field. */
	};

	///@name Accessors
	//@{
	/** Returns the DataCode value in a string format.
		@return string representation of this object's data code
	*/
	const EmaString& getCodeAsString() const;

	/** Returns the DataType, which is the type of Omm data.
		@return DataType::DataTypeEnum of this object
	*/
	virtual DataType::DataTypeEnum getDataType() const = 0;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::DataCode of this object
	*/
	virtual DataCode getCode() const = 0;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with this object's hex representation
	*/
	virtual const EmaBuffer& getAsHex() const = 0;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	virtual const EmaString& toString() const = 0;

	/** Operator const char* overload.
		\remark invokes toString().c_str()
		@return a NULL terminated character string representation of this object
	*/
	operator const char*() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Data();
	//@}

protected:

	Data();

private :

	friend class Decoder;
	friend class StaticDecoder;
	friend class MsgDecoder;

	friend class AckMsgEncoder;
	friend class GenericMsgEncoder;
	friend class PostMsgEncoder;
	friend class ReqMsgEncoder;
	friend class RefreshMsgEncoder;
	friend class StatusMsgEncoder;
	friend class UpdateMsgEncoder;

	friend class ElementListEncoder;
	friend class FieldListEncoder;
	friend class FilterListEncoder;
	friend class MapEncoder;
	friend class VectorEncoder;
	friend class SeriesEncoder;

	friend class ElementListDecoder;
	friend class FieldListDecoder;

	friend class OmmArray;
	friend class AckMsg;
	friend class GenericMsg;
	friend class PostMsg;
	friend class ReqMsg;
	friend class RefreshMsg;
	friend class StatusMsg;
	friend class UpdateMsg;

	friend class ElementList;
	friend class FieldList;
	friend class FilterList;
	friend class Map;
	friend class Series;
	friend class Vector;

	friend class FieldEntry;
	friend class ElementEntry;
	friend class MapEntry;
	friend class FilterEntry;
	friend class VectorEntry;
	friend class SeriesEntry;

	virtual Decoder& getDecoder() = 0;
	virtual bool hasDecoder() const = 0;
	virtual const EmaString& toString( UInt64 indent ) const = 0;
	virtual const Encoder& getEncoder() const = 0;
	virtual bool hasEncoder() const = 0;

	Data( const Data& );
	Data& operator=( const Data& );
};

}

}

}

#endif // __thomsonreuters_ema_access_Data_h
