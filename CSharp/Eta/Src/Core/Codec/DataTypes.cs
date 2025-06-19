/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// Defines enumerations for ETA primitive and container types.
	/// </summary>
	sealed public class DataTypes
	{
		private const int __RSZVAR = 0xFF; // Variable length size

		private const int __RSZI8 = 1;
		private const int __RSZUI8 = 1;
		private const int __RSZI16 = 2;
		private const int __RSZUI16 = 2;
		private const int __RSZI32 = 4;
		private const int __RSZUI32 = 4;
		private const int __RSZI64 = 8;
		private const int __RSZUI64 = 8;
		private const int __RSZFLT = 4;
		private const int __RSZDBL = 8;
		private const int __RSZDT = 4;
		private const int __RSZTM = 5;
		private static readonly int __RSZDTM = __RSZDT + __RSZTM;
		private const int __RSZTM3 = 3;
		private static readonly int __RSZDTM7 = __RSZDT + __RSZTM3;
		private const int __RSZQOS = 5;
		private const int __RSZRL32 = 5;
		private const int __RSZRL64 = 9;

		// DataTypes class cannot be instantiated
		private DataTypes()
		{
		}

		/* Minimum allowed value for primitive types - used for internal ETA range checking */
		internal const int PRIMITIVE_MIN = 0;

		/// <summary>
		/// Indicates that type is unknown. Only valid when decoding a Field List
		/// type where a dictionary look-up is required to determine the type). This
		/// type cannot be passed into encoding or decoding methods.
		/// </summary>
		public const int UNKNOWN = 0;

		/* Reserved */
		internal const int RESERVED1 = 1;
		/* Reserved */
		internal const int RESERVED2 = 2;

		/// <summary>
		/// A signed integer type. Can currently represent a value of up to 63 bits
		/// along with a one bit sign (positive or negative).
		/// </summary>
		public const int INT = 3;

		/// <summary>
		/// An unsigned integer type. Can currently represent an unsigned value with
		/// precision of up to 64 bits.
		/// </summary>
		public const int UINT = 4;

		/// <summary>
		/// A four-byte floating point type. Can represent the same range of values
		/// allowed with the system float type. Follows IEEE 754 specification.
		/// </summary>
		public const int FLOAT = 5;

		/// <summary>
		/// An eight-byte floating point type. Can represent the same range of values
		/// allowed with the system double type. Follows IEEE 754 specification.
		/// </summary>
		public const int DOUBLE = 6;

		/* Reserved */
		internal const int RESERVED7 = 7;

		/// <summary>
		/// An optimized RWF representation of a decimal or fractional value which
		/// typically requires less bytes on the wire than float or double types. The
		/// user specifies a value with a hint for converting to decimal or
		/// fractional representation.
		/// </summary>
		public const int REAL = 8;

		/// <summary>
		/// Defines a date with month, day, and year values.
		/// </summary>
		public const int DATE = 9;

		/// <summary>
		/// Defines a time with hour, minute, second, and millisecond values.
		/// </summary>
		public const int TIME = 10;

        /// <summary>
        /// Combined representation of date and time. Contains all members of
        /// <see cref="DataTypes.DATE"/> and <see cref="DataTypes.TIME"/>.
        /// </summary>
        public const int DATETIME = 11;

		/// <summary>
		/// Defines QoS information such as data timeliness (e.g. real time) and rate
		/// (e.g. tick-by-tick). Allows a user to send QoS information as part of the
		/// data payload. Similar information can also be conveyed using multiple ETA
		/// message headers.
		/// </summary>
		public const int QOS = 12;

		/// <summary>
		/// Represents data and stream state information. Allows a user to send state
		/// information as part of data payload. Similar information can also be
		/// conveyed in several ETA message headers.
		/// </summary>
		public const int STATE = 13;

		/// <summary>
		/// Represents an enumeration type, defined as an unsigned two-byte value.
		/// Many times, this enumeration value is cross-referenced with an
		/// enumeration dictionary (e.g., enumtype.def) or a well-known enumeration
		/// definition.
		/// </summary>
		public const int ENUM = 14;

		/// <summary>
		/// The ETA Array type allows users to represent a simple base primitive type
		/// list (all primitive types except <see cref="Array"/>). The user can specify the
		/// base primitive type that an array carries and whether each is of a
		/// variable or fixed-length. Because the array is a primitive type, if any
		/// primitive value in the array updates, the entire array must be resent.
		/// </summary>
		public const int ARRAY = 15;

		/// <summary>
		/// Represents a raw byte buffer type. Any semantics associated with the data
		/// in this buffer is provided from outside of ETA, either via a field
		/// dictionary (e.g., RDMFieldDictionary), or a DMM definition
		/// </summary>
		public const int BUFFER = 16;

		/// <summary>
		/// Represents an ASCII string which should contain only characters that are
		/// valid in ASCII specification. Because this might be NULL terminated, use
		/// the provided length when accessing content. ETA does not enforce or
		/// validate any encoding standard, and this is the responsibility of the
		/// encoding or decoding user.
		/// </summary>
		public const int ASCII_STRING = 17;

		/// <summary>
		/// Represents a UTF8 string which should follow the UTF8 encoding standard
		/// and contain only characters valid within that set. Because this might be
		/// NULL terminated, use the provided length when accessing content. ETA does
		/// not enforce or validate any encoding standard, and this is the
		/// responsibility of the encoding or decoding user.
		/// </summary>
		public const int UTF8_STRING = 18;

		/// <summary>
		/// Represents an RMTES (Multilingual Text Encoding Standard) string
		/// which should follow the RMTES encoding standard and contain only
		/// characters valid within that set. ETA does not enforce or validate
		/// encoding standards, so this is the responsibility of the encoding or
		/// decoding user.
		/// </summary>
		public const int RMTES_STRING = 19;

		/// <summary>
		/// Maximum allowed value for base primitive types </summary>
		public const int BASE_PRIMITIVE_MAX = 63;
		/// <summary>
		/// Minimum allowed value for set defined primitive types </summary>
		public const int SET_PRIMITIVE_MIN = 64;

		/// <summary>
		/// A signed, one-byte integer type. Can currently represent a value of up to
		/// 7 bits along with a one-bit sign (positive or negative). This set-defined
		/// primitive type cannot be represented as blank.
		/// </summary>
		public const int INT_1 = 64;

		/// <summary>
		/// An unsigned, one-byte integer type. Can currently represent an unsigned
		/// value with precision of up to 8 bits. This set-defined primitive type
		/// cannot be represented as blank.
		/// </summary>
		public const int UINT_1 = 65;

		/// <summary>
		/// A signed, two-byte integer type. Can currently represent a value of up to
		/// 15 bits along with a one-bit sign (positive or negative). This
		/// set-defined primitive type cannot be represented as blank.
		/// </summary>
		public const int INT_2 = 66;

		/// <summary>
		/// An unsigned, two-byte integer type. Can currently represent an unsigned
		/// value with precision of up to 16 bits. This set-defined primitive type
		/// cannot be represented as blank.
		/// </summary>
		public const int UINT_2 = 67;

		/// <summary>
		/// A signed, four-byte integer type. Can currently represent a value of up
		/// to 31 bits along with a one-bit sign (positive or negative). This
		/// set-defined primitive type cannot be represented as blank.
		/// </summary>
		public const int INT_4 = 68;

		/// <summary>
		/// An unsigned, four-byte integer type. Can currently represent an unsigned
		/// value with precision of up to 32 bits. This set-defined primitive type
		/// cannot be represented as blank.
		/// </summary>
		public const int UINT_4 = 69;

		/// <summary>
		/// A signed, eight-byte integer type. Can currently represent a value of up
		/// to 63 bits along with a one-bit sign (positive or negative). This
		/// set-defined primitive type cannot be represented as blank.
		/// </summary>
		public const int INT_8 = 70;

		/// <summary>
		/// An unsigned, eight-byte integer type. Can currently represent an unsigned
		/// value with precision of up to 64 bits. This set-defined primitive type
		/// cannot be represented as blank.
		/// </summary>
		public const int UINT_8 = 71;

		/// <summary>
		/// A four-byte, floating point type. Can represent the same range of values
		/// allowed with the system float type. Follows the IEEE 754 specification.
		/// This set-defined primitive type cannot be represented as blank.
		/// </summary>
		public const int FLOAT_4 = 72;

		/// <summary>
		/// An eight-byte, floating point type. Can represent the same range of
		/// values allowed with the system double type. Follows the IEEE 754
		/// specification. This set-defined primitive type cannot be represented as
		/// blank.
		/// </summary>
		public const int DOUBLE_8 = 73;

		/// <summary>
		/// An optimized RWF representation of a decimal or fractional value which
		/// typically requires less bytes on the wire than float or double types.
		/// This type allows for up to a four-byte value with a hint for converting
		/// to decimal or fractional representation. This set-defined primitive type
		/// can be represented as blank.
		/// </summary>
		public const int REAL_4RB = 74;

		/// <summary>
		/// An optimized RWF representation of a decimal or fractional value which
		/// typically requires less bytes on the wire than float or double types.
		/// This type allows for up to eight bytes of value with a hint for
		/// converting to decimal or fractional representation. This set-defined
		/// primitive type can be represented as blank.
		/// </summary>
		public const int REAL_8RB = 75;

		/// <summary>
		/// 4-byte representation of a date containing month, day, and year values.
		/// This value can be represented as blank
		/// </summary>
		public const int DATE_4 = 76;

		/// <summary>
		/// 3 byte representation of a time containing hour, minute, and second
		/// values. This value can be represented as blank
		/// </summary>
		public const int TIME_3 = 77;

		/// <summary>
		/// 5 byte Representation of a time containing hour, minute, second, and
		/// millisecond values.  This value can be represented as blank.
		/// </summary>
		public const int TIME_5 = 78;

		/// <summary>
		/// 7 byte combined representation of date and time. Contains all members of
		/// <see cref="Date"/> and hour, minute, and second from <see cref="Time"/>. This value
		/// can be represented as blank.
		/// </summary>
		public const int DATETIME_7 = 79;

		/// <summary>
		/// 9 byte combined representation of date and time. Contains all members of
		/// <see cref="Date"/> and hour, minute, second, and millisecond from <see cref="Time"/>. This value
		/// can be represented as blank.
		/// </summary>
		public const int DATETIME_9 = 80;

		/// <summary>
		/// 11 byte combined representation of date and time. Contains all members of
		/// <see cref="Date"/> and hour, minute, second, millisecond, and microsecond from <see cref="Time"/>. 
		/// This value can be represented as blank.
		/// </summary>
		public const int DATETIME_11 = 81;

		/// <summary>
		/// 12 byte combined representation of date and time. Contains all members of
		/// <see cref="Date"/> and hour, minute, second, millisecond, microsecond, and
		/// nanosecond from <see cref="Time"/>.   
		/// This value can be represented as blank.
		/// </summary>
		public const int DATETIME_12 = 82;

		/// <summary>
		/// 7 byte Representation of a time containing hour, minute, second, 
		/// millisecond, and microsecond values.  This value can be represented as blank.
		/// </summary>
		public const int TIME_7 = 83;

		/// <summary>
		/// 8 byte Representation of a time containing hour, minute, second, 
		/// millisecond, microsecond, and nanosecond values.  This value can be represented 
		/// as blank.
		/// </summary>
		public const int TIME_8 = 84;

		/// <summary>
		/// Maximum allowed value for set defined primitive types </summary>
		public const int SET_PRIMITIVE_MAX = 127;

		/// <summary>
		/// Minimum allowed value for container types </summary>
		public const int CONTAINER_TYPE_MIN = 128;

		/// <summary>
		/// No Data </summary>
		public const int NO_DATA = 128;

		/* Reserved */
		internal const int RESERVED129 = 129;

		/// <summary>
		/// Opaque data, use Non-RWF type encoders </summary>
		public const int OPAQUE = 130;

		/// <summary>
		/// XML formatted data, use Non-RWF type encoders </summary>
		public const int XML = 131;

		/// <summary>
		/// A highly optimized, non-uniform type, that contains field
		/// identifier-value paired entries. fieldId refers to specific name and type
		/// information as defined in an external field dictionary (such as
		/// RDMFieldDictionary).
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="FieldEntry"/>, which can
		/// house any <see cref="DataTypes"/>, including set-defined data, base primitive
		/// types, and container types.
		/// <ul>
		/// <li>
		/// If the information and entry being updated contains a primitive type, any
		/// previously stored or displayed data is replaced.</li>
		/// <li>If the entry contains another container type, action values
		/// associated with that type specify how to update the information.</li>
		/// </ul>
		/// </para>
		/// </remarks>
        /// <seealso cref="FieldEntry"/>
		public const int FIELD_LIST = 132;

		/// <summary>
		/// A self-describing, non-uniform type, with each entry containing name,
		/// dataType, and a value. This type is equivalent to <see cref="FieldList"/>, but
		/// without the optimizations provided through fieldId use.
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="ElementEntry"/>, which can
		/// house any <see cref="DataTypes"/>, including set-defined data, base primitive
		/// types, and container types.
		/// <ul>
		/// <li>If the updating information and entry contain a primitive type, any
		/// previously stored or displayed data is replaced.</li>
		/// <li>If the entry contains another container type, action values
		/// associated with that type specify how to update the information.</li>
		/// </ul>
		/// </para>
		/// </remarks>
		/// <seealso cref="ElementEntry"/>
		public const int ELEMENT_LIST = 133;

		/// <summary>
		/// Indicates that contents are ANSI Page format, use Non-RWF type encoders
		/// </summary>
		public const int ANSI_PAGE = 134;

		/// <summary>
		/// A non-uniform container of filterId-value paired entries. A filterId
		/// corresponds to one of 32 possible bit-value identifiers, typically
		/// defined by a domain model specification. FilterId's can be used to
		/// indicate interest or presence of specific entries through the inclusion
		/// of the filterId in the message key's filter member.
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="FilterEntry"/>, which can
		/// house only container types. Though the <see cref="FilterList"/> can specify a
		/// containerType, each entry can override this specification to house a
		/// different type. Each entry has an associated action, which informs the
		/// user of how to apply the information stored in the entry.
		/// </para>
		/// </remarks>
		/// <seealso cref="FilterEntry"/>
		/// <seealso cref="FilterList"/>
		public const int FILTER_LIST = 135;

		/// <summary>
		/// A container of position index-value paired entries. This container is a
		/// uniform type, where the containerType of each entry's payload is
		/// specified on the <see cref="Vector"/>. Each entry's index is represented by an
		/// unsigned integer.
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="VectorEntry"/>, which can
		/// house only container types, as specified on the <see cref="Vector"/>. Each
		/// entry's index is an unsigned integer. Each entry has an associated
		/// action, which informs the user of how to apply the information stored in
		/// the entry.
		/// </para>
		/// </remarks>
		/// <seealso cref="Vector"/>
		/// <seealso cref="VectorEntry"/>
		public const int VECTOR = 136;

		/// <summary>
		/// A container of key-value paired entries. <see cref="Map"/> is a uniform type,
		/// where the base primitive type of each entry's key and the containerType
		/// of each entry's payload are specified on the <see cref="Map"/>.
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="MapEntry"/>, which can
		/// include only container types, as specified on the <see cref="Map"/>. Each
		/// entry's key is a base primitive type, as specified on the <see cref="Map"/>.
		/// Each entry has an associated action, which informs the user of how to
		/// apply the information stored in the entry.
		/// </para>
		/// </remarks>
		/// <seealso cref="Map"/>
		/// <seealso cref="MapEntry"/>
		public const int MAP = 137;

		/// <summary>
		/// A uniform type, where the containerType of each entry is specified on the
		/// <see cref="Series"/>. This container is often used to represent table-based
		/// information, where no explicit indexing is present or required. As
		/// entries are received, the user should append them to any
		/// previously-received entries.
		/// </summary>
		/// <remarks>
		/// <para>
		/// <b>Entry Information</b>: Entry type is <see cref="SeriesEntry"/>, which can
		/// include only container types, as specified on the <see cref="Series"/>.
		/// <see cref="SeriesEntry"/> types do not contain explicit actions; though as
		/// entries are received, the user should append them to any previously
		/// received entries.
		/// </para>
		/// </remarks>
		/// <seealso cref="Series"/>
		/// <seealso cref="SeriesEntry"/>
		public const int SERIES = 138;

		/// <summary>
		/// Indicates that the contents are another message. This allows the
		/// application to house a message within a message or a message within
		/// another container's entries. This type is typically used with posting.
		/// <para>
		/// <b>Entry Information</b>: None
		/// </para>
		/// </summary>
		public const int MSG = 141;

		/// <summary>
		/// Indicates that the contents are JSON encoded. Use Non-RWF encoders and
		/// decoders to process content, likely a JSON encoding/decoding library.
		/// <para>
		/// <b>Entry Information</b>: None
		/// </para>
		/// </summary>
		public const int JSON = 142;

		/// <summary>
		/// Maximum supported container type value for this release. </summary>
		public const int CONTAINER_TYPE_MAX = 142;

		/* Maximum reserved value. Values beyond this can be user defined types */
		internal const int MAX_RESERVED = 224;

		/* Last Value */
		internal const int LAST = 255;

		/// <summary>
		/// Provides string representation for a data type value.
		/// </summary>
		/// <param name="dataType"> <see cref="DataTypes"/> enumeration to convert to string
		/// </param>
		/// <returns> string representation for a data type value </returns>
		public static string ToString(int dataType)
		{
			string ret = "";

			switch (dataType)
			{
				case UNKNOWN:
					ret = "UNKNOWN";
					break;
				case INT:
					ret = "INT";
					break;
				case UINT:
					ret = "UINT";
					break;
				case FLOAT:
					ret = "FLOAT";
					break;
				case DOUBLE:
					ret = "DOUBLE";
					break;
				case REAL:
					ret = "REAL";
					break;
				case DATE:
					ret = "DATE";
					break;
				case TIME:
					ret = "TIME";
					break;
				case DATETIME:
					ret = "DATETIME";
					break;
				case QOS:
					ret = "QOS";
					break;
				case STATE:
					ret = "STATE";
					break;
				case ENUM:
					ret = "ENUM";
					break;
				case ARRAY:
					ret = "ARRAY";
					break;
				case BUFFER:
					ret = "BUFFER";
					break;
				case ASCII_STRING:
					ret = "ASCII_STRING";
					break;
				case UTF8_STRING:
					ret = "UTF8_STRING";
					break;
				case RMTES_STRING:
					ret = "RMTES_STRING";
					break;
				case INT_1:
					ret = "INT_1";
					break;
				case UINT_1:
					ret = "UINT_1";
					break;
				case INT_2:
					ret = "INT_2";
					break;
				case UINT_2:
					ret = "UINT_2";
					break;
				case INT_4:
					ret = "INT_4";
					break;
				case UINT_4:
					ret = "UINT_4";
					break;
				case INT_8:
					ret = "INT_8";
					break;
				case UINT_8:
					ret = "UINT_8";
					break;
				case FLOAT_4:
					ret = "FLOAT_4";
					break;
				case DOUBLE_8:
					ret = "DOUBLE_8";
					break;
				case REAL_4RB:
					ret = "REAL_4RB";
					break;
				case REAL_8RB:
					ret = "REAL_8RB";
					break;
				case DATE_4:
					ret = "DATE_4";
					break;
				case TIME_3:
					ret = "TIME_3";
					break;
				case TIME_5:
					ret = "TIME_5";
					break;
				case DATETIME_7:
					ret = "DATETIME_7";
					break;
				case DATETIME_9:
					ret = "DATETIME_9";
					break;
				case NO_DATA:
					ret = "NO_DATA";
					break;
				case OPAQUE:
					ret = "OPAQUE";
					break;
				case XML:
					ret = "XML";
					break;
				case FIELD_LIST:
					ret = "FIELD_LIST";
					break;
				case ELEMENT_LIST:
					ret = "ELEMENT_LIST";
					break;
				case ANSI_PAGE:
					ret = "ANSI_PAGE";
					break;
				case FILTER_LIST:
					ret = "FILTER_LIST";
					break;
				case VECTOR:
					ret = "VECTOR";
					break;
				case MAP:
					ret = "MAP";
					break;
				case SERIES:
					ret = "SERIES";
					break;
				case MSG:
					ret = "MSG";
					break;
				case JSON:
					ret = "JSON";
					break;
				default:
					break;
			}

			return ret;
		}

		/// <summary>
		/// Checks if the passed in type is a valid primitive type.
		/// </summary>
		/// <param name="dataType"> <see cref="DataTypes"/> enumerated value to check
		/// </param>
		/// <returns> <c>true</c> if passed in type is a valid primitive type or <c>false</c> if not.
		/// </returns>
		public static bool IsPrimitiveType(int dataType)
		{
			return ((PRIMITIVE_MIN <= dataType) && (dataType <= SET_PRIMITIVE_MAX));
		}

		/// <summary>
		/// Checks if the passed in type is a valid container type.
		/// </summary>
		/// <param name="dataType"> <see cref="DataTypes"/> enumerated value to check
		/// </param>
		/// <returns> <c>true</c> if passed in type is a valid container type or <c>false</c> if not.
		/// </returns>
		public static bool IsContainerType(int dataType)
		{
			return ((CONTAINER_TYPE_MIN <= dataType) && (dataType <= LAST));
		}

		/// <summary>
		/// Returns maximum encoded size for primitiveTypes.
		/// </summary>
		/// <param name="dataType"> <see cref="DataTypes"/> enumerated value to return encoded size for
		/// </param>
		/// <returns> maximum encoded size when this can be determined, 255 (0xFF) for types
		///         that contain buffer content whose length varies based on <see cref="Buffer.Length"/>,
		///         or 0 for invalid types.
		/// </returns>
		public static int PrimitiveTypeSize(int dataType)
		{
			int retVal = 0;

			switch (dataType)
			{
				case INT:
					retVal = __RSZI64 + 1;
					break;
				case UINT:
					retVal = __RSZUI64 + 1;
					break;
				case FLOAT:
					retVal = __RSZFLT + 1;
					break;
				case DOUBLE:
					retVal = __RSZDBL + 1;
					break;
				case REAL:
					retVal = __RSZRL64 + 1;
					break;
				case DATE:
					retVal = __RSZDT + 1;
					break;
				case TIME:
					retVal = __RSZTM + 1;
					break;
				case DATETIME:
					retVal = __RSZDTM + 1;
					break;
				case QOS:
					retVal = __RSZQOS + 1;
					break;
				case ENUM:
					retVal = __RSZUI16 + 1;
					break;
				case INT_1:
					retVal = __RSZI8;
					break;
				case UINT_1:
					retVal = __RSZUI8;
					break;
				case INT_2:
					retVal = __RSZI16;
					break;
				case UINT_2:
					retVal = __RSZUI16;
					break;
				case INT_4:
					retVal = __RSZI32;
					break;
				case UINT_4:
					retVal = __RSZUI32;
					break;
				case INT_8:
					retVal = __RSZI64;
					break;
				case UINT_8:
					retVal = __RSZUI64;
					break;
				case FLOAT_4:
					retVal = __RSZFLT;
					break;
				case DOUBLE_8:
					retVal = __RSZDBL;
					break;
				case REAL_4RB:
					retVal = __RSZRL32;
					break;
				case REAL_8RB:
					retVal = __RSZRL64;
					break;
				case DATE_4:
					retVal = __RSZDT;
					break;
				case TIME_3:
					retVal = __RSZTM3;
					break;
				case TIME_5:
					retVal = __RSZTM;
					break;
				case DATETIME_7:
					retVal = __RSZDTM7;
					break;
				case DATETIME_9:
					retVal = __RSZDTM;
					break;
				case STATE:
				case ARRAY:
				case BUFFER:
				case ASCII_STRING:
				case UTF8_STRING:
				case RMTES_STRING:
					retVal = __RSZVAR;
					break;
				default:
					break;
			}

			return retVal;
		}
	}

}
