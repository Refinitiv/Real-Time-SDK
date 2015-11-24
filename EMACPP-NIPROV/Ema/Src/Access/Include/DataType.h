/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_DataType_h
#define __thomsonreuters_ema_access_DataType_h

/**
	\class thomsonreuters::ema::access::DataType DataType.h "Access/DataType.h"
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

namespace thomsonreuters {
	
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
		ReqMsgEnum,			/*!< A message to specify item interest. 
								A consumer sends this message. */

		RefreshMsgEnum,		/*!< A message to open and change an item.
								A provider sends this message. */

		UpdateMsgEnum,		/*!< A message to change an item.
								A provider sends this message. */

		StatusMsgEnum,		/*!< A message to indicate the state of an item.
								A provider sends this message. */

		PostMsgEnum,		/*!< A message to solicit alteration of an item.
								A consumer sends this message. */

		AckMsgEnum,			/*!< A message to acknowledge the alteration of an item.
								A provider sends this message. */

		GenericMsgEnum,		/*!< A message that has no implicit market information semantics.
								A consumer or provider may send this message. */

		FieldListEnum,		/*!< A container of efficient, associative-referenced field identifier
								simple or complex value pairs. The field identifier references attributes
								such as name and type in a field definition dictionary. */

		ElementListEnum,	/*!< A container of flexible in-band, self-describing, associative-referenced
								named simple or complex value pairs. */

		MapEnum,			/*!< A container of associative-referenced key and complex value pairs,
								whose key is an arbitrary simple type. */

		VectorEnum,			/*!< A container of associative-referenced key and complex value pairs,
								whose key is a numeric simple type. */

		SeriesEnum,			/*!< A container of implicitly-indexed accrue-able complex values,
								typically used for repetitively structured data. */

		FilterListEnum,		/*!< A container of loosely-coupled, associative-referenced numeric and 
								complex value pairs that may be segmented. */

		OpaqueEnum,			/*!< A buffer that should be agreed upon between Provider and Consumer. */

		XmlEnum,			/*!< An XML buffer. */

		AnsiPageEnum,		/*!< An AnsiPage buffer. */

		ArrayEnum,			/*!< A container of implicitly position-oriented simple types. */

		IntEnum,			/*!< A signed integer. */

		UIntEnum,			/*!< An unsigned integer. */

		RealEnum,			/*!< An 8-byte precision (19-20 decimal places) fixed-placed representation
								of a numeric with either a fractional or exponential part. The range of
								a fractional part is 1/2 through 1/256. The range of an exponential
								part is 10-14 through 10+7. */

		FloatEnum,			/*!< A 4-byte value that has a range of -3.4e38 to +3.4e38 with an
								accuracy of 6 to 7 decimal digits. This value is compliant with the 
								IEEE-754 standard. */

		DoubleEnum,			/*!< An 8-byte value that has a range of -1.7e308 to +1.7e308 with
								an accuracy of 14 to 15 digits. This value is compliant with
								the IEEE-754 standard. */

		DateEnum,			/*!< A 4 byte value that represents a Gregorian date. */

		TimeEnum,			/*!< A 3 or 5 byte value that includes information for hours, minutes, 
								seconds, and optional milliseconds, microseconds and nanoseconds. */

		DateTimeEnum,		/*!< A 7 or 9 byte combination of a Date and a Time. */

		QosEnum,			/*!< Quality Of Service, represents quality of service information
								such as timeliness and rate */

		StateEnum,			/*!< State, represents data and stream state information */

		EnumEnum,			/*!< A 2-byte signed value that can be expanded to a language specific
								string in an EnumTable Dictionary. */

		BufferEnum,			/*!< A general purpose buffer. */

		AsciiEnum,			/*!< An 8-bit characters encoding using the Reuters Basic Character Set 
								(RBCS). The first 128 characters are equivalent to the ASCII character 
								set (ANSI X3.4-1968). */

		Utf8Enum,			/*!< A UTF-8 encoding of ISO 10646 (specified in section 3.9 of the Unicode
								4.0 standard and IETF's RFC 3629). */

		RmtesEnum,			/*!< An encoding with the Reuters Multilingual Text Encoding Standard.
								RMTES uses ISO 2022 escape sequences to select the character sets used. 
								RMTES provides support for the Reuters Basic Character Set, UTF-8, 
								Japanese Latin and Katakana (JIS C 6220 - 1969), Japanese Kanji (JIS X 
								0208 - 1990), and Chinese National Standard (CNS 11643-1986). StringRMTES 
								also supports RREP sequences for character repetition and RHPA sequences 
								for partial updates. */

		ErrorEnum,			/*!< Indicates processing error. */

		NoDataEnum			/*!< No Omm data value is present. */
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

#endif // __thomsonreuters_ema_access_DataType_h
