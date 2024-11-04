/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmJson_h
#define __refinitiv_ema_access_OmmJson_h

/**
	@class refinitiv::ema::access::OmmJson OmmJson.h "Access/Include/OmmJson.h"
	@brief OmmJson represents JSON data format in Omm.

	The following code snippet shows setting of JSON data into FieldList;

	\code

	OmmJson json;
	json.set( "{"json_tag":"this is json data"}" );

	FieldList fList;
	fList.addJson( 123, json ).complete();

	\endcode

	The following code snippet shows extraction of json data from FieldList;

	\code

	void decodeFieldList( const FieldList& fList )
	{
		while ( fList.forth() )
		{
			const FieldEntry& fEntry = flist.getEntry();

			if ( fEntry.getCode() != Data::BlankEnum )
				switch ( fEntry.getLoadType() )
				{
				case DataType::JsonEnum :
					const OmmJson& ommJson = fEntry.getJson();
					const EmaBuffer& jsonValue = ommJson.getBuffer();
					break;
				}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of JSON and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/ComplexType.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmJsonDecoder;
class OmmJsonEncoder;

class EMA_ACCESS_API OmmJson : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs OmmJson
	*/
	OmmJson();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmJson();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::JsonEnum
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

	/** Returns contained JSON string.
		@return EmaString containing the JSON data
	*/
	const EmaString& getString() const;

	/** Returns contained JSON buffer.
		@return EmaBuffer containing the JSON data
	*/
	const EmaBuffer& getBuffer() const;
	//@}

	///@name Operations
	//@{
	/** Clears the OmmJson.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmJson& clear();

	/** Specifies Set.
		@param[in] value specifies JSON data using EmaString
		@return reference to this object
	*/
	OmmJson& set( const EmaString& value );

	/** Specifies Set.
		@param[in] value specifies JSON data using EmaBuffer
		@return reference to this object
	*/
	OmmJson& set( const EmaBuffer& value );
	//@}

private :

	const EmaString& toString( UInt64 ) const;
	
	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmJson( const OmmJson& );
	OmmJson& operator=( const OmmJson& );

	mutable EmaString	_toString;
	OmmJsonDecoder*		_pDecoder;
	OmmJsonEncoder*		_pEncoder;
	UInt64				_space[22];
};

}

}

}

#endif // __refinitiv_ema_access_OmmJson_h
