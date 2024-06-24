/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_DataType_h
#define __refinitiv_ema_access_DataType_h

/**
	\class refinitiv::ema::access::DataType DataType.h "Access/DataType.h"
	\brief DataType class provides enumeration representing Omm data types.

	DataType::DataTypeEnum is a numeric and symbolic representation of Omm data type.

	DataType class is used to convert the numeric representation into string representation.

	\code

	const Payload& payload = refreshMsg.getPayload();

	std::cout << "Received payload is " << DataType( payload.getDataType() ) << endl;

	\endcode

	\remark All methods in this class are \ref SingleThreaded.

	@see EmaString,
		EmaBuffer
*/

#include "Access/Include/EmaString.h"

namespace refinitiv {
	
namespace ema {

namespace access {

class EmaBuffer;

class EMA_ACCESS_API DataType
{
public :

	/** @enum DataTypeEnum
		An enumeration representing data type.
	*/
	enum DataTypeEnum
	{
		IntEnum = 3,			/*!< A signed integer. Can currently represent a value of up to 63 bits
									along with a one bit sign (positive or negative).*/

		UIntEnum = 4,			/*!< An unsigned integer. Can currently represent an unsigned value with
									precision of up to 64 bits.*/
								 
		FloatEnum = 5,			/*!< A 4-byte value that has a range of -3.4e38 to +3.4e38 with an
									accuracy of 6 to 7 decimal digits. This value is compliant with the 
									IEEE-754 standard. */
								 
		DoubleEnum = 6,			/*!< An 8-byte value that has a range of -1.7e308 to +1.7e308 with
									an accuracy of 14 to 15 digits. This value is compliant with
									the IEEE-754 standard. */
								 
		RealEnum = 8,			/*!< An 8-byte precision (19-20 decimal places) fixed-placed representation
									of a numeric with either a fractional or exponential part. The range of
									a fractional part is 1/2 through 1/256. The range of an exponential
									part is 10-14 through 10+7. */
								
		DateEnum = 9,			/*!< A 4 byte value that represents a date with month, day, and year values. */
		
		TimeEnum = 10,			/*!< A 3 or 5 byte value that includes information for hours, minutes, 
									seconds, and optional milliseconds, microseconds and nanoseconds. */
								
		DateTimeEnum = 11,		/*!< A 7 or 9 byte combination of a Date and a Time. */
		
		QosEnum = 12,			/*!< Quality Of Service, represents quality of service information
									such as timeliness and rate */
		
		StateEnum = 13,			/*!< State, represents data and stream state information. Allows a user to send state
									information as part of data payload. Similar information can also be
									conveyed in several EMA message headers. */
								
		EnumEnum = 14,			/*!< A 2-byte signed value that can be expanded to a language specific
									string in an enumerated type dictionary. */
								
		ArrayEnum = 15,			/*!< The EMA Array type allows users to represent a simple base primitive type
									list (all primitive types except OmmArray. The user can specify the
									whether each is of a variable or fixed-length. Because the array is a
									primitive type, if any primitive value in the array updates, the entire
									array must be resent.*/
			
		BufferEnum = 16,		/*!< A general purpose buffer. */

		AsciiEnum = 17,			/*!< An 8-bit characters encoding using the LSEG Basic Character Set 
									(RBCS). The first 128 characters are equivalent to the ASCII character 
									set (ANSI X3.4-1968). */
								
		Utf8Enum = 18,			/*!< A UTF-8 encoding of ISO 10646 (specified in section 3.9 of the Unicode
									4.0 standard and IETF's RFC 3629). */
		
		RmtesEnum = 19,			/*!< An encoding with a multilingual text encoding standard.
									RMTES uses ISO 2022 escape sequences to select the character sets used. 
									RMTES provides support for the LSEG Basic Character Set, UTF-8, 
									Japanese Latin and Katakana (JIS C 6220 - 1969), Japanese Kanji (JIS X 
									0208 - 1990), and Chinese National Standard (CNS 11643-1986). StringRMTES 
									also supports RREP sequences for character repetition and RHPA sequences 
									for partial updates. */

		NoDataEnum = 128,		/*!< No Omm data value is present. */
									
		FieldListEnum = 132,	/*!< A highly optimized, non-uniform type, that contains field
									identifier-value paired entries. fieldId refers to specific name and type
									information as defined in an external field dictionary (such as 
									RDMFieldDictionary). FieldList's entry can house any data types, base primitive
									types, and container types.

									If the information and entry being updated contains a primitive type, any
									previously stored or displayed data is replaced.

									If the entry contains another container type, action values associated with that
									type specify how to update the information. */
									
		ElementListEnum = 133,	/*!< A self-describing, non-uniform type, with each entry containing name,
									dataType, and a value. This type is equivalent to FieldList, but without the
									optimizations provided through fieldId use. ElementList's entry can house any
									DataType, including base primitive types, and container types.

									If the updating information and entry contain a primitive type, any previously
									stored or displayed data is replaced.

									If the entry contains another container type, action values	associated with that
									type specify how to update the information. */
									
		FilterListEnum = 135,	/*!< A non-uniform container of filterId-value paired entries. A filterId
									corresponds to one of 32 possible bit-value identifiers, typically
									defined by a domain model specification. FilterId's can be used to
									indicate interest or presence of specific entries through the inclusion
									of the filterId in the message key's filter member.

									FilterList's entry can house only container types. Each entry has an
									associated action, which informs the user of how to apply the information
									stored in the entry. */
		
		VectorEnum = 136,		/*!< A container of position index-value paired entries. This container is a
									uniform type. Each entry's index is represented by an unsigned integer.
									Each entry has an associated action, which informs the user of how to apply
									the information stored in the entry. */
		

		MapEnum = 137,			/*!< A container of key-value paired entries. Map is a uniform type for key
									primitive type and entry's container type. Map's entry can include only
									container types. Each entry's key is a base primitive type. Each entry
									has an associated action, which informs the user of how to apply the
									information stored in the entry. */

		SeriesEnum = 138,		/*!< A uniform type. This container is often used to represent table-based
									information, where no explicit indexing is present or required. As
									entries are received, the user should append them to any previously-received
									entries.

									Series's entry can include only container types and do not contain explicit
									actions; though as entries are received,the user should append them to any
									previously received entries. */

		OpaqueEnum = 130,		/*!< A buffer that should be agreed upon between Provider and Consumer. */

		XmlEnum = 131,			/*!< An XML buffer. */

		AnsiPageEnum = 134,		/*!< An AnsiPage buffer. */
		
		MsgEnum = 141, 			/*!< Indicates that the contents are another message. This allows the
									application to house a message within a message or a message within
									another container's entries. This type is typically used with posting.*/
		
		ReqMsgEnum = 256,		/*!< A message to specify item interest. 
									A consumer sends this message. */

		RefreshMsgEnum = 257,	/*!< A message to open and change an item.
									A provider sends this message. */

		UpdateMsgEnum = 258,	/*!< A message to change an item.
									A provider sends this message. */

		StatusMsgEnum = 259,	/*!< A message to indicate the state of an item.
									A provider sends this message. */

		PostMsgEnum = 260, 		/*!< A message to solicit alteration of an item.
									A consumer sends this message. */

		AckMsgEnum = 261,		/*!< A message to acknowledge the alteration of an item.
									A provider sends this message. */

		GenericMsgEnum = 262,	/*!< A message that has no implicit market information semantics.
									A consumer or provider may send this message. */
		
		ErrorEnum = 270,		/*!< Indicates processing error. */

		LargestValue = ErrorEnum /* should be last element in DataTypeEnum and should be
								  * set to the largest enumeration value
								  */
	};

	///@name Constructor
	//@{
	/** Constructs DataType. This is used for string representation only.
		@param[in] dataType specifies data type to be converted to string
	*/
	DataType( DataTypeEnum dataType );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~DataType();
	//@}

	///@name Accessors
	//@{
	/** Returns the enum value as a string format.
		@return EmaString containing name of the data type
	*/
	const EmaString& toString() const;

	/** Operator const char* overload.
		@return const char*
	 */
	operator const char* () const;
	//@}

private :

	DataTypeEnum			_dataType;

	DataType();
	DataType( const DataType& );
	DataType& operator=( const DataType& );
};

}

}

}

#endif // __refinitiv_ema_access_DataType_h
