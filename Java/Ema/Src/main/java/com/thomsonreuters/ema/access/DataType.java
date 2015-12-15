///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * DataType class provides represents Omm data types.
 * DataType class is used to convert the numeric representation into string representation.
 */
public class DataType
{
	private DataType()
	{
		throw new AssertionError();
	}

	private final static String REQMSG_STRING = "ReqMsg";
	private final static String REFRESHMSG_STRING = "RefreshMsg";
	private final static String UPDATEMSG_STRING = "UpdateMsg";
	private final static String STATUSMSG_STRING = "StatusMsg";
	private final static String POSTMSG_STRING = "PostMsg";
	private final static String ACKMSG_STRING = "AckMsg";
	private final static String GENERICMSG_STRING = "GenericMsg";
	private final static String FIELDLIST_STRING = "FieldList";
	private final static String ELEMENTLIST_STRING = "ElementList";
	private final static String MAP_STRING = "Map";
	private final static String VECTOR_STRING = "Vector";
	private final static String SERIES_STRING = "Series";
	private final static String FILTERLIST_STRING = "FilterList";
	private final static String OPAQUE_STRING = "Opaque";
	private final static String XML_STRING = "Xml";
	private final static String ANSIPAGE_STRING = "AnsiPage";
	private final static String OMMARRAY_STRING = "OmmArray";
	private final static String INT_STRING = "Int";
	private final static String UINT_STRING = "UInt";
	private final static String REAL_STRING = "Real";
	private final static String FLOAT_STRING = "Float";
	private final static String DOUBLE_STRING = "Double";
	private final static String DATE_STRING = "Date";
	private final static String TIME_STRING = "Time";
	private final static String DATETIME_STRING = "DateTime";
	private final static String QOS_STRING = "Qos";
	private final static String STATE_STRING = "State";
	private final static String ENUM_STRING = "Enum";
	private final static String BUFFER_STRING = "Buffer";
	private final static String ASCII_STRING = "Ascii";
	private final static String UTF8_STRING = "Utf8";
	private final static String RMTES_STRING = "Rmtes";
	private final static String ERROR_STRING = "Error";
	private final static String NODATA_STRING = "NoData";
	private final static String DEFAULTDT_STRING = "Unknown DataType value ";
	
	/**
	 * Represents data type.
	 */
	public static class DataTypes
	{
		/**
		 * A message to specify item interest.
		 * A consumer sends this message.
		 */
		public final static int REQ_MSG = 0;

		/**
		 * A message to open and change an item.
		 * A provider sends this message.
		 */
		public final static int REFRESH_MSG = 1;

		/**
		 * A message to change an item.
		 * A provider sends this message.
		 */
		public final static int UPDATE_MSG = 2;

		/**
		 * A message to indicate the state of an item.
		 * A provider sends this message.
		 */
		public final static int STATUS_MSG = 3;

		/**
		 * A message to solicit alteration of an item.
		 * A consumer sends this message.
		 */
		public final static int POST_MSG = 4;

		/**
		 * A message to acknowledge the alteration of an item.
		 * A provider sends this message.
		 */
		public final static int ACK_MSG = 5;

		/**
		 * A message that has no implicit market information semantics.
		 * A consumer or provider may send this message.
		 */
		public final static int GENERIC_MSG = 6;

		/**
		 * A container of efficient, associative-referenced field identifier,
		 * simple or complex value pairs.
		 * The field identifier references attributes
		 * such as name and type in a field definition dictionary.
		 */
		public final static int FIELD_LIST = 7;
		
		/**
		 * A container of flexible in-band, self-describing,
		 * associative-referenced named simple or complex value pairs.
		 */
		public final static int ELEMENT_LIST = 8;

		/**
		 * A container of associative-referenced key
		 * and complex value pairs, whose key is an arbitrary simple type.
		 */
		public final static int MAP = 9;

		/**
		 * A container of associative-referenced key
		 * and complex value pairs, whose key is a numeric simple type.
		 */
		public final static int VECTOR = 10;
		
		/**
		 * A container of implicitly-indexed accruable,
		 * complex values, typically used for repetitively structured data.
		 */
		public final static int SERIES = 11;

		/**
		 * A container of loosely-coupled, associative-referenced numeric
		 * and complex value pairs that may be segmented.
		 */
		public final static int FILTER_LIST = 12; 

		/**
		 * A buffer that should be agreed upon between Provider and Consumer.
		 */
		public final static int OPAQUE = 13;

		/**
		 * An XML buffer.
		 */
		public final static int XML = 14;

		/**
		 * An AnsiPage buffer.
		 */
		public final static int ANSI_PAGE = 15;

		/**
		 * A container of implicitly position-oriented simple types.
		 */
		public final static int ARRAY = 16;
		
		/**
		 * A signed integer.
		 */
		public final static int INT = 17;

		/**
		 * An unsigned integer.
		 */
		public final static int UINT = 18;

		/**
		 * An 8-byte precision (19-20 decimal places) fixed-placed
		 * representation of a numeric with either a fractional or exponential part.
		 * The range of a fractional part is 1/2 through 1/256.
		 * The range of an exponential part is 10-14 through 10+7.
		 */
		public final static int REAL = 19;
		
		/**
		 * A 4-byte value that has a range of -3.4e38 to +3.4e38
		 * with an accuracy of 6 to 7 decimal digits.
		 * This value is compliant with the IEEE-754 standard.
		 */
		public final static int FLOAT = 20;

		/**
		 * An 8-byte value that has a range of -1.7e308 to +1.7e308
		 * with an accuracy of 14 to 15 digits.
		 * This value is compliant with the IEEE-754 standard.
		 */
		public final static int DOUBLE = 21;

		 /**
		  * A 4 byte value that represents a Gregorian date.
		  */
		public final static int DATE = 22;
		
		/**
		 * A 3 or 5 byte value that includes information for hours, minutes, seconds,
		 * and optional milliseconds, microseconds and nanoseconds.
		 */
		public final static int TIME = 23;

		/**
		 * A 7 or 9 byte combination of a Date and a Time.
		 */
		public final static int DATETIME = 24;
		
		/**
		 * Quality Of Service, represents quality of service information
		 * such as timeliness and rate.
		 */
		public final static int QOS = 25;

		/**
		 * State, represents data and stream state information.
		 */
		public final static int STATE = 26;

		/**
		 * A 2-byte signed value that can be expanded to a language
		 * specific string in a Table Dictionary.
		 */
		public final static int ENUM = 27;

		public final static int BUFFER = 28; /** A general purpose buffer. */

		/**
		 * An 8-bit characters encoding using the Reuters Basic Character Set (RBCS).
		 * The first 128 characters are equivalent to the ASCII character set (ANSI X3.4-1968).
		 */
		public final static int ASCII = 29;

		/**
		 * A UTF-8 encoding of ISO 10646
		 * (specified in section 3.9 of the Unicode 4.0 standard and IETF's RFC 3629).
		 */
		public final static int UTF8 = 30;

		/**
		 * An encoding with the Reuters Multilingual Text Encoding Standard.
		 * RMTES uses ISO 2022 escape sequences to select the character sets used.
		 * RMTES provides support for the Reuters Basic Character Set, UTF-8,
		 * Japanese Latin and Katakana (JIS C 6220 - 1969),
		 * Japanese Kanji (JIS X 0208 - 1990), 
		 * and Chinese National Standard (CNS 11643-1986).
		 * StringRMTES also supports RREP sequences for
		 * character repetition and RHPA sequences for partial updates.
		 */
		public final static int RMTES = 31;
		
		/**
		 *  Indicates processing error.
		 */
		public final static int ERROR = 32;

		/**
		 * No Omm data value is present.
		 */
		public final static int NO_DATA = 33;
	}

	/**
	 * @param dataType numeric value of data type (see {@link com.thomsonreuters.ema.access.DataType.DataTypes})
	 * @return String containing name of the data type
	 */
	public static String asString(int dataType)
	{
		switch (dataType)
		{
			case DataTypes.REQ_MSG :
				return REQMSG_STRING;
			case DataTypes.REFRESH_MSG :
				return REFRESHMSG_STRING;
			case DataTypes.STATUS_MSG :
				return STATUSMSG_STRING;
			case DataTypes.UPDATE_MSG :
				return UPDATEMSG_STRING;
			case DataTypes.POST_MSG :
				return POSTMSG_STRING;
			case DataTypes.ACK_MSG :
				return ACKMSG_STRING;
			case DataTypes.GENERIC_MSG :
				return GENERICMSG_STRING;
			case DataTypes.FIELD_LIST :
				return FIELDLIST_STRING;
			case DataTypes.ELEMENT_LIST :
				return ELEMENTLIST_STRING;
			case DataTypes.MAP :
				return MAP_STRING;
			case DataTypes.VECTOR :
				return VECTOR_STRING;
			case DataTypes.SERIES :
				return SERIES_STRING;
			case DataTypes.FILTER_LIST :
				return FILTERLIST_STRING;
			case DataTypes.OPAQUE :
				return OPAQUE_STRING;
			case DataTypes.XML :
				return XML_STRING;
			case DataTypes.ANSI_PAGE :
				return ANSIPAGE_STRING;
			case DataTypes.ARRAY :
				return OMMARRAY_STRING;
			case DataTypes.INT :
				return INT_STRING;
			case DataTypes.UINT :
				return UINT_STRING;
			case DataTypes.REAL :
				return REAL_STRING;
			case DataTypes.FLOAT :
				return FLOAT_STRING;
			case DataTypes.DOUBLE :
				return DOUBLE_STRING;
			case DataTypes.DATE :
				return DATE_STRING;
			case DataTypes.TIME :
				return TIME_STRING;
			case DataTypes.DATETIME :
				return DATETIME_STRING;
			case DataTypes.QOS :
				return QOS_STRING;
			case DataTypes.STATE :
				return STATE_STRING;
			case DataTypes.ENUM :
				return ENUM_STRING;
			case DataTypes.BUFFER :
				return BUFFER_STRING;
			case DataTypes.ASCII :
				return ASCII_STRING;
			case DataTypes.UTF8 :
				return UTF8_STRING;
			case DataTypes.RMTES :
				return RMTES_STRING;
			case DataTypes.NO_DATA :
				return NODATA_STRING;
			case DataTypes.ERROR :
				return ERROR_STRING;
			default :
				return DEFAULTDT_STRING + dataType;
		}
	}
}