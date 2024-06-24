/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmOpaque_h
#define __refinitiv_ema_access_OmmOpaque_h

/**
	@class refinitiv::ema::access::OmmOpaque OmmOpaque.h "Access/Include/OmmOpaque.h"
	@brief OmmOpaque represents Opaque data format in Omm.

	The following code snippet shows setting of Opaque data into FieldList;

	\code

	OmmOpaque opaque;
	opaque.set( ... );

	FieldList fList;
	fList.addOpaque( 369, opaque ).complete();

	\endcode

	The following code snippet shows extraction of Opaque data from FieldList;

	\code

	void decodeFieldList( const FieldList& fList )
	{
		while ( fList.forth() )
		{
			const FieldEntry& fEntry = fList.getEntry();

			if ( fEntry.getCode() != Data::BlankEnum )
				switch ( fEntry.getLoadType() )
				{
				case DataType::OpaqueEnum :
					const OmmOpaque& ommOpaque = fEntry.getOpaque();
					const EmaBuffer& opaqueValue = ommOpaque.getBuffer();
					break;
				}
		}
	}

	\endcode

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of Opaque and its content.
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

class OmmOpaqueDecoder;
class OmmOpaqueEncoder;

class EMA_ACCESS_API OmmOpaque : public ComplexType
{
public :

	///@name Constructor
	//@{	
	/** Constructs OmmOpaque
	*/
	OmmOpaque();
	//@}

	///@name Destructor
	//@{	
	/** Destructor.
	*/
	virtual ~OmmOpaque();
	//@}

	///@name Accessors
	//@{	
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::OpaqueEnum
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

	/** Returns Opaque string.
		@return EmaString containing the Opaque data
	*/
	const EmaString& getString() const;

	/** Returns Opaque buffer.
		@return EmaBuffer containing the Opaque data
	*/
	const EmaBuffer& getBuffer() const;
	//@}

	///@name Operations
	//@{	
	/** Clears the OmmOpaque.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmOpaque& clear();

	/** Specifies Set.
		@param[in] value specifies Opaque data using EmaString
		@return reference to this object
	*/
	OmmOpaque& set( const EmaBuffer& value );

	/** Specifies Set.
		@param[in] value specifies Opaque data using EmaBuffer
		@return reference to this object
	*/
	OmmOpaque& set( const EmaString& value );
	//@}

private :

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	Decoder& getDecoder();
	bool hasDecoder() const;

	OmmOpaque( const OmmOpaque& );
	OmmOpaque& operator=( const OmmOpaque& );

	mutable EmaString		_toString;
	OmmOpaqueDecoder*		_pDecoder;
	mutable OmmOpaqueEncoder*		_pEncoder;
	UInt64					_space[22];

};

}

}

}

#endif // __refinitiv_ema_access_OmmOpaque_h
