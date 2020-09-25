/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmAnsiPage_h
#define __refinitiv_ema_access_OmmAnsiPage_h

/**
	@class rtsdk::ema::access::OmmAnsiPage OmmAnsiPage.h "Access/Include/OmmAnsiPage.h"
	@brief OmmAnsiPage represents AnsiPage data format in Omm.

	The following code snippet shows setting of AnsiPage data into FieldList;

	\code

	OmmAnsiPage ansiPage;
	ansiPage.set( ... );

	FieldList fList;
	fList.addAnsiPage( 246, ansiPage ).complete();

	\endcode

	The following code snippet shows extraction of AnsiPage data from FieldList;

	\code

	void decodeFieldList( const FieldList& fList )
	{
		while ( fList.forth() )
		{
			const FieldEntry& fEntry = fList.getEntry();

			if ( fEntry.getCode() != Data::BlankEnum )
				switch ( fEntry.getLoadType() )
				{
				case DataType::AnsiPageEnum :
					const OmmAnsiPage& ommAnsiPage = fEntry.getAnsiPage();
					const EmaBuffer& ansiPageValue = ommAnsiPage.getBuffer();
					break;
				}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of AnsiPage and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/ComplexType.h"

namespace rtsdk {

namespace ema {

namespace access {

class OmmAnsiPageDecoder;
class OmmAnsiPageEncoder;

class EMA_ACCESS_API OmmAnsiPage : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs OmmAnsiPage.
	*/
	OmmAnsiPage();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmAnsiPage();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::AnsiPageEnum
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

	/** Returns AnsiPage string.
		@return EmaString containing the AnsiPage data
	*/
	const EmaString& getString() const;

	/** Returns AnsiPage buffer.
		@return EmaBuffer containing the AnsiPage data
	*/
	const EmaBuffer& getBuffer() const;
	//@}

	///@name Operations
	//@{
	/** Clears the OmmAnsiPage.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmAnsiPage& clear();

	/** Specifies Set.
		@param[in] value specifies AnsiPage data using EmaString
		@return reference to this object
	*/
	OmmAnsiPage& set( const EmaString& value );

	/** Specifies Set.
		@param[in] value specifies AnsiPage data using EmaBuffer
		@return reference to this object
	*/
	OmmAnsiPage& set( const EmaBuffer& value );
	//@}

private :
	
	const EmaString& toString( UInt64 indent ) const;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmAnsiPage( const OmmAnsiPage& );
	OmmAnsiPage& operator=( const OmmAnsiPage& );

	mutable EmaString		_toString;
	OmmAnsiPageDecoder*		_pDecoder;
	OmmAnsiPageEncoder*		_pEncoder;
	UInt64					_space[18];
};

}

}

}

#endif // __refinitiv_ema_access_OmmAnsiPage_h
