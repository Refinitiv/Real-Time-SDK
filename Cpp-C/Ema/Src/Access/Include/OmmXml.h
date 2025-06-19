/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmXml_h
#define __refinitiv_ema_access_OmmXml_h

/**
	@class refinitiv::ema::access::OmmXml OmmXml.h "Access/Include/OmmXml.h"
	@brief OmmXml represents XML data format in Omm.

	The following code snippet shows setting of XML data into FieldList;

	\code

	OmmXml xml;
	xml.set( "<xml tag>this is xml data</xml tag>" );

	FieldList fList;
	fList.addXml( 123, xml ).complete();

	\endcode

	The following code snippet shows extraction of xml data from FieldList;

	\code

	void decodeFieldList( const FieldList& fList )
	{
		while ( fList.forth() )
		{
			const FieldEntry& fEntry = flist.getEntry();

			if ( fEntry.getCode() != Data::BlankEnum )
				switch ( fEntry.getLoadType() )
				{
				case DataType::XmlEnum :
					const OmmXml& ommXml = fEntry.getXml();
					const EmaBuffer& xmlValue = ommXml.getBuffer();
					break;
				}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of XML and its content.
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

class OmmXmlDecoder;
class OmmXmlEncoder;

class EMA_ACCESS_API OmmXml : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs OmmXml
	*/
	OmmXml();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmXml();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::XmlEnum
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

	/** Returns contained XML string.
		@return EmaString containing the XML data
	*/
	const EmaString& getString() const;

	/** Returns contained XML buffer.
		@return EmaBuffer containing the XML data
	*/
	const EmaBuffer& getBuffer() const;
	//@}

	///@name Operations
	//@{
	/** Clears the OmmXml.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmXml& clear();

	/** Specifies Set.
		@param[in] value specifies XML data using EmaString
		@return reference to this object
	*/
	OmmXml& set( const EmaString& value );

	/** Specifies Set.
		@param[in] value specifies XML data using EmaBuffer
		@return reference to this object
	*/
	OmmXml& set( const EmaBuffer& value );
	//@}

private :

	const EmaString& toString( UInt64 ) const;
	
	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmXml( const OmmXml& );
	OmmXml& operator=( const OmmXml& );

	mutable EmaString	_toString;
	OmmXmlDecoder*		_pDecoder;
	OmmXmlEncoder*		_pEncoder;
	UInt64				_space[22];
};

}

}

}

#endif // __refinitiv_ema_access_OmmXml_h
