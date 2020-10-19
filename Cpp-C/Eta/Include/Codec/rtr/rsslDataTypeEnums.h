/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_DATA_TYPE_ENUMS_H
#define __RTR_RSSL_DATA_TYPE_ENUMS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"




/**
 * @addtogroup RSSLTypeEnums
 * @{
 */

typedef enum
{
	RSSL_DT_BASE_PRIMITIVE_MIN	= 0,	/*!< Minimum allowed value for primitive types - used for internal ETA range checking <BR>*/

	RSSL_DT_UNKNOWN				= 0,	/*!< (0) Unknown Data Type.  This is only valid when decoding an RsslFieldEntry type that requires a dictionary look-up.  If content is set defined, actual type enum will be present. <BR> */
								
    									/*!< (1) Reserved <BR>*/
									 	/*!< (2) Reserved <BR>*/
	RSSL_DT_INT					= 3,	/*!< (3) Signed Integer, represents a value of up to 63 bits along with a one bit sign.  See \ref RsslIntGroup for more information. <BR>*/
	RSSL_DT_UINT				= 4,	/*!< (4) Unsigned Integer, represents an unsigned value with precision of up to 64 bits. See \ref RsslUIntGroup for more information. <BR>*/
	RSSL_DT_FLOAT				= 5,	/*!< (5) 32 Bit Floating Point, represents the same range of values allowed with the system float type.  See \ref RsslFloatGroup for more information. <BR>*/
	RSSL_DT_DOUBLE				= 6,	/*!< (6) 64 Bit Floating Point, represents the same range of values allowed with the system double type.  See \ref RsslDoubleGroup for more information. <BR>*/

									 	/*!< (7) Reserved <BR>*/
	RSSL_DT_REAL				= 8,	/*!< (8) Real Numeric, a bandwidth optimized representation of a decimal or fractional value which typically requires less bytes on the wire than float or double types.  See \ref RsslRealGroup for more information. <BR>*/

	RSSL_DT_DATE				= 9,	/*!< (9) Date type, contains month, day, and year values.  See \ref RsslDateGroup for more information. <BR>*/
	RSSL_DT_TIME				= 10,	/*!< (10) Time type, contains hour, minute, second, millisecond, microsecond, and nanosecond values.  See \ref RsslTimeGroup for more information. <BR>*/
	RSSL_DT_DATETIME			= 11,	/*!< (11) DateTime type, contains all members of RSSL_DT_DATE and RSSL_DT_TIME.  See \ref RsslDateTimeGroup for more information. <BR>*/

	RSSL_DT_QOS					= 12,	/*!< (12) Quality of Service, represents quality of service information such as data timeliness (e.g. real time) and rate (e.g. tick by tick).  See \ref RsslQosGroup for more information. <BR>*/	
	RSSL_DT_STATE				= 13,	/*!< (13) State type, represents data and stream state information.  See \ref RsslStateGroup for more information. <BR>*/
	RSSL_DT_ENUM				= 14,	/*!< (14) Enumeration type, represents an enumeration type, defined as an unsigned two byte value.  See \ref RsslEnumGroup for more information. <BR>*/

  	RSSL_DT_ARRAY				= 15,	/*!< (15) Array type, represents a list of a simple primitive types that can be variable or fixed length.  See \ref RsslArrayGroup for more information. <BR>*/

	RSSL_DT_BUFFER				= 16,	/*!< (16) Buffer type, represents a raw byte buffer type with a char* and an \ref RsslUInt32 length.  See \ref RsslBufferGroup for more information. <BR>*/
	RSSL_DT_ASCII_STRING		= 17,	/*!< (17) ASCII String, uses the \ref RsslBuffer type for encoding and decoding.  This enum indicates that the content is an ASCII string and should only contain characters that are valid within the ASCII specification.  See \ref RsslBufferGroup for more information. <BR>*/
	RSSL_DT_UTF8_STRING			= 18,	/*!< (18) UTF8 String, uses the \ref RsslBuffer type for encoding and decoding. This enum indicates that the content is a UTF8 string and should only contain characters that are valid within the UTF8 encoding standard.  See \ref RsslBufferGroup for more information. <BR>*/
	RSSL_DT_RMTES_STRING		= 19,	/*!< (19) RMTES String, uses the \ref RsslBuffer type for encoding and decoding.  This enum indicates that the content is a multilingual text encoding standard string and should only contain characters that are valid within the RMTES encoding standard.  See \ref RsslBufferGroup and \ref RsslRmtesGroup for more information. <BR>*/


	RSSL_DT_BASE_PRIMITIVE_MAX	= 63,	/*!< Maximum allowed value for primitive types - used for internal ETA range checking <BR>*/
	RSSL_DT_SET_PRIMITIVE_MIN	= 64,	/*!< Minimum allowed value for set defined primitive types - used for internal ETA range checking <BR>*/

	RSSL_DT_INT_1				= 64,	/*!< (64) 1 byte signed integer, represents a value of up to 7 bits along with a one bit sign.   <BR>*/
	RSSL_DT_UINT_1				= 65,	/*!< (65) 1 byte unsigned integer, represents a value of up to 8 bits.   <BR>*/
	RSSL_DT_INT_2				= 66,	/*!< (66) 2 byte signed integer, represents a value of up to 15 bits along with a one bit sign.   <BR>*/
	RSSL_DT_UINT_2				= 67,	/*!< (67) 2 byte unsigned integer, represents a value of up to 16 bits.   <BR>*/
	RSSL_DT_INT_4				= 68,	/*!< (68) 4 byte signed integer, represents a value of up to 31 bits along with a one bit sign.   <BR>*/
	RSSL_DT_UINT_4				= 69,	/*!< (69) 4 byte unsigned integer, represents a value of up to 32 bits.  <BR>*/
	RSSL_DT_INT_8				= 70,	/*!< (70) 8 byte signed integer, represents a value of up to 63 bits along with a one bit sign.   <BR>*/
	RSSL_DT_UINT_8				= 71,	/*!< (71) 8 byte unsigned integer, represents a value of up to 64 bits.   <BR>*/
	RSSL_DT_FLOAT_4				= 72,	/*!< (72) 4 byte float value, represents the same range of values allowed with the system float type.  <BR>*/
	RSSL_DT_DOUBLE_8			= 73,	/*!< (73) 8 byte double value, represents the same range of values allowed with the system double type. <BR>*/
	RSSL_DT_REAL_4RB			= 74,	/*!< (74) Optimized variable length encoding for \ref RsslReal with value that can range up to but not exceed 31 bits and one sign bit.  <BR>*/
	RSSL_DT_REAL_8RB			= 75,	/*!< (75) Optimized variable length encoding for \ref RsslReal with value that can range up to 63 bits and one sign bit.   <BR>*/
	RSSL_DT_DATE_4				= 76,	/*!< (76) 4 byte date, includes month, day, and year.  <BR>*/
	RSSL_DT_TIME_3				= 77,	/*!< (77) 3 byte time, includes hours, minutes, and seconds.  <BR>*/
	RSSL_DT_TIME_5				= 78,	/*!< (78) 5 byte time, includes hours, minutes, seconds and milliseconds. <BR>*/
	RSSL_DT_DATETIME_7			= 79,	/*!< (79) 7 byte datetime, includes date (month, day, and year), hours, minutes, seconds.  <BR>*/
	RSSL_DT_DATETIME_9			= 80,	/*!< (80) 9 byte datetime, includes date (month, day, and year), hours, minutes, seconds, milliseconds.  <BR>*/
	RSSL_DT_DATETIME_11			= 81,	/*!< (81) 11 byte datetime, includes date (month, day, and year), hours, minutes, seconds, milliseconds, and microseconds.  <BR>*/
	RSSL_DT_DATETIME_12			= 82,	/*!< (82) 12 byte datetime, includes date (month, day, and year), hours, minutes, seconds, milliseconds, microseconds, and nanoseconds.  <BR>*/
	RSSL_DT_TIME_7				= 83,	/*!< (83) 7 byte time, includes hours, minutes, seconds, milliseconds, and microseconds. <BR>*/
	RSSL_DT_TIME_8				= 84,	/*!< (84) 8 byte time, includes hours, minutes, seconds, milliseconds, microseconds, and nanoseconds. <BR>*/
							
	RSSL_DT_SET_PRIMITIVE_MAX	= 127,	/*!< Maximum allowed value for set defined primitive types - used for internal ETA range checking <BR>*/
	RSSL_DT_CONTAINER_TYPE_MIN	= 128,	/*!< Minimum allowed value for container types - used for internal ETA range checking <BR>*/

	RSSL_DT_NO_DATA				= 128,	/*!< (128) No Data <BR>*/
										/*!< (129) Reserved <BR>*/
	RSSL_DT_OPAQUE				= 130,	/*!< (130) Opaque data, use Non-RWF type encoders.   <BR>*/
	RSSL_DT_XML					= 131,	/*!< (131) XML formatted data, use Non-RWF type encoders.  <BR>*/
	RSSL_DT_FIELD_LIST			= 132,	/*!< (132) Field List container type, used to represent content using fieldID - value pair data.  <BR>*/
	RSSL_DT_ELEMENT_LIST		= 133,	/*!< (133) Element List container type, used to represent content containing element name, dataType, and value triples.    <BR>*/
	RSSL_DT_ANSI_PAGE			= 134,	/*!< (134) ANSI page format, use Non-RWF type encoders.  <BR>*/
	RSSL_DT_FILTER_LIST			= 135,	/*!< (135) Filter List container type, used to represent content using filterID - container type paired entries.  <BR>*/
	RSSL_DT_VECTOR				= 136,	/*!< (136) Vector container type, used to represent index - container type paired entries.  <BR>*/
	RSSL_DT_MAP					= 137,	/*!< (137) Map container type, used to represent primitive type key - container type paired entries.   <BR>*/
	RSSL_DT_SERIES				= 138,	/*!< (138) Series container type, represents row based tabular information where no specific indexing is required.   <BR>*/

	RSSL_DT_MSG					= 141,	/*!< (141) RsslMsg container type.  This can be used to nest a message inside another message (e.g. RsslPostMsg containing an RsslUpdateMsg) or nesting a message inside of another container entry (e.g. RsslMapEntry contains an RsslMsg).   <BR>*/
	RSSL_DT_JSON				= 142,  /*!< (142) JSON formatted data, use Non-RWF type encoders.  <BR> */

	RSSL_DT_CONTAINER_TYPE_MAX	= 142,	/*!<  Maximum supported container type value for this release - used for internal ETA range checking <BR>*/
	RSSL_DT_MAX_RESERVED		= 224,	/*!< (224) Maximum Refinitiv reserved value.  Values beyond this can be user defined types <BR>*/

	RSSL_DT_LAST				= 255	/*!< (255) Maximum allowed enumeration value - used for internal ETA range checking. <BR>*/
} RsslDataTypes;

/** 
 * @brief General OMM strings associated with the different data types.
 * @see RsslDataTypes, rsslDataTypeToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_DT_UNKNOWN = { 7, (char*)"Unknown" };
static const RsslBuffer RSSL_OMMSTR_DT_INT = { 3, (char*)"Int" };
static const RsslBuffer RSSL_OMMSTR_DT_UINT = { 4, (char*)"UInt" };
static const RsslBuffer RSSL_OMMSTR_DT_FLOAT = { 5, (char*)"Float" };
static const RsslBuffer RSSL_OMMSTR_DT_DOUBLE = { 6, (char*)"Double" };
static const RsslBuffer RSSL_OMMSTR_DT_REAL = { 4, (char*)"Real" };
static const RsslBuffer RSSL_OMMSTR_DT_DATE = { 4, (char*)"Date" };
static const RsslBuffer RSSL_OMMSTR_DT_TIME = { 4, (char*)"Time" };
static const RsslBuffer RSSL_OMMSTR_DT_DATETIME = { 8, (char*)"DateTime" };
static const RsslBuffer RSSL_OMMSTR_DT_QOS = { 3, (char*)"Qos" };
static const RsslBuffer RSSL_OMMSTR_DT_STATE = { 5, (char*)"State" };
static const RsslBuffer RSSL_OMMSTR_DT_ENUM = { 4, (char*)"Enum" };
static const RsslBuffer RSSL_OMMSTR_DT_ARRAY = { 5, (char*)"Array" };
static const RsslBuffer RSSL_OMMSTR_DT_BUFFER = { 6, (char*)"Buffer" };
static const RsslBuffer RSSL_OMMSTR_DT_ASCII_STRING = { 11, (char*)"AsciiString" };
static const RsslBuffer RSSL_OMMSTR_DT_UTF8_STRING = { 10, (char*)"Utf8String" };
static const RsslBuffer RSSL_OMMSTR_DT_RMTES_STRING = { 11, (char*)"RmtesString" };
static const RsslBuffer RSSL_OMMSTR_DT_INT_1 = { 4, (char*)"Int1" };
static const RsslBuffer RSSL_OMMSTR_DT_UINT_1 = { 5, (char*)"UInt1" };
static const RsslBuffer RSSL_OMMSTR_DT_INT_2 = { 4, (char*)"Int2" };
static const RsslBuffer RSSL_OMMSTR_DT_UINT_2 = { 5, (char*)"UInt2" };
static const RsslBuffer RSSL_OMMSTR_DT_INT_4 = { 4, (char*)"Int4" };
static const RsslBuffer RSSL_OMMSTR_DT_UINT_4 = { 5, (char*)"UInt4" };
static const RsslBuffer RSSL_OMMSTR_DT_INT_8 = { 4, (char*)"Int8" };
static const RsslBuffer RSSL_OMMSTR_DT_UINT_8 = { 5, (char*)"UInt8" };
static const RsslBuffer RSSL_OMMSTR_DT_FLOAT_4 = { 6, (char*)"Float4" };
static const RsslBuffer RSSL_OMMSTR_DT_DOUBLE_8 = { 7, (char*)"Double8" };
static const RsslBuffer RSSL_OMMSTR_DT_REAL_4RB = { 7, (char*)"Real4RB" };
static const RsslBuffer RSSL_OMMSTR_DT_REAL_8RB = { 7, (char*)"Real8RB" };
static const RsslBuffer RSSL_OMMSTR_DT_DATE_4 = { 5, (char*)"Date4" };
static const RsslBuffer RSSL_OMMSTR_DT_TIME_3 = { 5, (char*)"Time3" };
static const RsslBuffer RSSL_OMMSTR_DT_TIME_5 = { 5, (char*)"Time5" };
static const RsslBuffer RSSL_OMMSTR_DT_DATETIME_7 = { 9, (char*)"DateTime7" };
static const RsslBuffer RSSL_OMMSTR_DT_DATETIME_9 = { 9, (char*)"DateTime9" };
static const RsslBuffer RSSL_OMMSTR_DT_DATETIME_11 = { 10, (char*)"DateTime11" };
static const RsslBuffer RSSL_OMMSTR_DT_DATETIME_12 = { 10, (char*)"DateTime12" };
static const RsslBuffer RSSL_OMMSTR_DT_TIME_7 = { 5, (char*)"Time7" };
static const RsslBuffer RSSL_OMMSTR_DT_TIME_8 = { 5, (char*)"Time8" };
static const RsslBuffer RSSL_OMMSTR_DT_NO_DATA = { 6, (char*)"NoData" };
static const RsslBuffer RSSL_OMMSTR_DT_OPAQUE = { 6, (char*)"Opaque" };
static const RsslBuffer RSSL_OMMSTR_DT_XML = { 3, (char*)"Xml" };
static const RsslBuffer RSSL_OMMSTR_DT_FIELD_LIST = { 9, (char*)"FieldList" };
static const RsslBuffer RSSL_OMMSTR_DT_ELEMENT_LIST = { 11, (char*)"ElementList" };
static const RsslBuffer RSSL_OMMSTR_DT_ANSI_PAGE = { 8, (char*)"AnsiPage" };
static const RsslBuffer RSSL_OMMSTR_DT_FILTER_LIST = { 10, (char*)"FilterList" };
static const RsslBuffer RSSL_OMMSTR_DT_VECTOR = { 6, (char*)"Vector" };
static const RsslBuffer RSSL_OMMSTR_DT_MAP = { 3, (char*)"Map" };
static const RsslBuffer RSSL_OMMSTR_DT_SERIES = { 6, (char*)"Series" };
static const RsslBuffer RSSL_OMMSTR_DT_MSG = { 3, (char*)"Msg" };
static const RsslBuffer RSSL_OMMSTR_DT_JSON = { 4, (char*)"Json" };


/** 
 * @brief An 8 bit value that uses an \ref RsslDataTypes enumerated value to specify the type of content.  This can be used to specify only container types.
 *
 * The valid container types are defined by the \ref RsslDataTypes enumeration.
 * This can only be a container type (128 - 255)
 * @see RsslDataTypes
 */
typedef RsslUInt8  RsslContainerType;

/**
 * @brief An 8 bit value that uses an \ref RsslDataTypes enumerated value to specify the type of content.  This can be used to specify only primitive types.
 * 
 * The valid primitive types are defined by the \ref RsslDataTypes enumeration.
 * This type can only be a primitive type (0 - 127).
 * @see RsslDataTypes
 */
typedef RsslUInt8 RsslPrimitiveType;

/** 
 * @brief An 8 bit value that uses an \ref RsslDataTypes enumerated value to specify the type of content.  This can be used to specify a primitive or container type.
 *
 * The valid data types are defined by the \ref RsslDataTypes enumeration.
 * This type can be a primitive type (0 - 127) or a container type (128 - 255)
 * @see RsslDataTypes
 */
typedef RsslUInt8  RsslDataType;

/**
 * @brief Provide string representation for a data type enumeration
 * @see RsslDataType, RsslDataTypes
 */
RSSL_API const char* rsslDataTypeToString(RsslDataType type);

/**
 * @brief Provide a general OMM string representation for a data type enumeration
 * @see RsslDataType, RsslDataTypes
 */
RSSL_API const char* rsslDataTypeToOmmString(RsslDataType type);


/**
 * @brief Returns maximum encoded size for primitiveTypes
 */
/**
 * @brief Checks if the passed in type is a valid primitive type (0 - 128)
 *
 * @param dataType RsslDataTypes enumerated value to return encoded size for
 * @see RsslDataTypes, RsslDataType
 * @return \ref RsslUInt32 containing the maximum encoded length when this can be determined.  255 (0xFF) is returned for types that contain buffer content and the length varies based on the \ref RsslBuffer::length.  0 is returned for invalid types.  
 */
RSSL_API RsslUInt32 rsslPrimitiveTypeSize(const RsslDataType dataType);


/**
 * @brief Checks if the passed in type is a valid primitive type (0 - 127)
 *
 * @param dataType RsslDataTypes enumerated value to check
 * @see RsslDataTypes, RsslDataType
 * @return RSSL_TRUE if passed in \ref RsslDataTypes enumeration is a valid primitive type (0 - 127) or RSSL_FALSE if not.  
 */
RTR_C_ALWAYS_INLINE RsslBool rsslIsPrimitiveType(const RsslDataType dataType)
{
	RsslInt16 minType = RSSL_DT_BASE_PRIMITIVE_MIN;
	RsslInt16 lastType = RSSL_DT_SET_PRIMITIVE_MAX;

	return(	(minType <= (RsslInt16)dataType) &&
			((RsslInt16)dataType <= lastType ) );
}


/**
 * @brief Checks if the passed in type is a valid container type (128 - 255)
 *
 * @param dataType RsslDataTypes enumerated value to check
 * @see RsslDataTypes, RsslDataType
 * @return RSSL_TRUE if passed in \ref RsslDataTypes enumeration is a valid container type (128 - 255) or RSSL_FALSE if not.  
 */
RTR_C_ALWAYS_INLINE RsslBool rsslIsContainerType(const RsslDataType dataType)
{
	RsslInt16 minType = RSSL_DT_CONTAINER_TYPE_MIN;
	RsslInt16 lastType = RSSL_DT_LAST;

	return(	(minType <= (RsslInt16)dataType) &&
			((RsslInt16)dataType <= lastType ) );
}

/**
 * @}
 */




#ifdef __cplusplus
}
#endif 

#endif
