///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

/**
 * DataType class provides represents Omm data types.<br>
 * DataType class is used to convert the numeric representation into string representation.
 * 
 * Code snippet:
 * <pre>
 * Payload payload = refreshMsg.payload();
 * System.out.println( "Received payload is " + payload.dataType() );
 * </pre>
 *
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
		 * A signed integer. Can currently represent a value of up to 63 bits
		 * along with a one bit sign (positive or negative).
		 */
		public final static int INT = 3;

		/**
		 * An unsigned integer. Can currently represent an unsigned value with
		 * precision of up to 64 bits.
		 */
		public final static int UINT = 4;

		/**
		 * A 4-byte value that has a range of -3.4e38 to +3.4e38
		 * with an accuracy of 6 to 7 decimal digits.<br>
		 * This value is compliant with the IEEE-754 standard.
		 */
		public final static int FLOAT = 5;

		/**
		 * An 8-byte value that has a range of -1.7e308 to +1.7e308
		 * with an accuracy of 14 to 15 digits.<br>
		 * This value is compliant with the IEEE-754 standard.
		 */
		public final static int DOUBLE = 6;

		/**
		 * An 8-byte precision (19-20 decimal places) fixed-placed
		 * representation of a numeric with either a fractional or exponential part.<br>
		 * The range of a fractional part is 1/2 through 1/256.<br>
		 * The range of an exponential part is 10-14 through 10+7.
		 */
		public final static int REAL = 8;

		/**
		 * A 4 byte value that represents a date with month, day, and year values.
		 */
		public final static int DATE = 9;

		/**
		 * A 3 or 5 byte value that includes information for hours, minutes, seconds,
		 * and optional milliseconds, microseconds and nanoseconds.
		 */
		public final static int TIME = 10;

		/**
		 * A 7 or 9 byte combination of a Date and a Time.
		 */
		public final static int DATETIME = 11;

		/**
		 * Quality Of Service, represents quality of service information
		 * such as timeliness and rate.
		 */
		public final static int QOS = 12;

		/**
		 * State, represents data and stream state information. Allows a user to
		 * send state information as part of data payload. Similar information can
		 * also be conveyed in several EMA message headers.
		 */
		public final static int STATE = 13;

		/**
		 * A 2-byte signed value that can be expanded to a language
		 * specific string in an enumerated type dictionary.
		 */
		public final static int ENUM = 14;

		/** 
		 * The EMA Array type allows users to represent a simple base primitive type
		 * list (all primitive types except OmmArray. The user can specify the
		 * whether each is of a variable or fixed-length. Because the array is a
		 * primitive type, if any primitive value in the array updates, the entire
		 * array must be resent.
		 */
		public final static int ARRAY = 15;
		
		/** 
		 * A general purpose buffer. 
		 */
		public final static int BUFFER = 16;

		/**
		 * An 8-bit characters encoding using the Reuters Basic Character Set (RBCS).<br>
		 * The first 128 characters are equivalent to the ASCII character set (ANSI X3.4-1968).
		 */
		public final static int ASCII = 17;

		/**
		 * A UTF-8 encoding of ISO 10646
		 * (specified in section 3.9 of the Unicode 4.0 standard and IETF's RFC 3629).
		 */
		public final static int UTF8 = 18;

		/**
		 * An encoding with the Reuters Multilingual Text Encoding Standard.<br>
		 * RMTES uses ISO 2022 escape sequences to select the character sets used.<br>
		 * RMTES provides support for the Reuters Basic Character Set, UTF-8,<br>
		 * Japanese Latin and Katakana (JIS C 6220 - 1969),
		 * Japanese Kanji (JIS X 0208 - 1990),<br>
		 * and Chinese National Standard (CNS 11643-1986).<br>
		 * StringRMTES also supports RREP sequences for
		 * character repetition and RHPA sequences for partial updates.
		 */
		public final static int RMTES = 19;
		
		/**
		 * No Omm data value is present.
		 */
		public final static int NO_DATA = 128;
		
		/**
		 * A buffer that should be agreed upon between Provider and Consumer.
		 */
		public final static int OPAQUE = 130;

		/**
		 * An XML buffer.
		 */
		public final static int XML = 131;

		/**
		 * A highly optimized, non-uniform type, that contains field
		 * identifier-value paired entries. fieldId refers to specific name and type
		 * information as defined in an external field dictionary (such as RDMFieldDictionary).
		 * FieldList's entry can house any data types, base primitive types, and container types.<br>
		 * If the information and entry being updated contains a primitive type, any
		 * previously stored or displayed data is replaced.<br>
		 * If the entry contains another container type, action values associated with that type
		 * specify how to update the information.
		 */
		public final static int FIELD_LIST = 132;

		/**
		 * A self-describing, non-uniform type, with each entry containing name,
		 * dataType, and a value. This type is equivalent to FieldList, but without the
		 * optimizations provided through fieldId use. ElementList's entry can house any DataType,
		 * including base primitive types, and container types.<br>
		 * If the updating information and entry contain a primitive type, any previously stored
		 * or displayed data is replaced.<br>
		 * If the entry contains another container type, action values associated with that type
		 * specify how to update the information.
		 */
		public final static int ELEMENT_LIST = 133;
		
		/**
		 * An AnsiPage buffer.
		 */
		public final static int ANSI_PAGE = 134;

		/**
		 * A non-uniform container of filterId-value paired entries. A filterId
		 * corresponds to one of 32 possible bit-value identifiers, typically
		 * defined by a domain model specification. FilterId's can be used to
		 * indicate interest or presence of specific entries through the inclusion
		 * of the filterId in the message key's filter member.<br>
		 * FilterList's entry can house only container types. Each entry has an
		 * associated action, which informs the user of how to apply the information
		 * stored in the entry.
		 */
		public final static int FILTER_LIST = 135;

		/**
		 * A container of position index-value paired entries. This container is a
		 * uniform type. Each entry's index is represented by an unsigned integer.<br>
		 * Each entry has an associated action, which informs the user of how to apply
		 * the information stored in the entry.
		 */
		public final static int VECTOR = 136;

		/**
		 * A container of key-value paired entries. Map is a uniform type for key
		 * primitive type and entry's container type. Map's entry can include only
		 * container types. Each entry's key is a base primitive type.<br> 
		 * Each entry has an associated action, which informs the user of how to apply
		 * the information stored in the entry.
		 */
		public final static int MAP = 137;

		/**
		 * A uniform type. This container is often used to represent table-based
		 * information, where no explicit indexing is present or required. As
		 * entries are received, the user should append them to any previously-received 
		 * entries.<br>
		 * Series's entry can include only container types and do not contain explicit
		 * actions; though as entries are received,the user should append them to any
		 * previously received entries.
		 */
		public final static int SERIES = 138;

		/**
		 * Indicates that the contents are another message. This allows the
		 * application to house a message within a message or a message within
		 * another container's entries. This type is typically used with posting.
		 */
		public final static int MSG = 141;

		/**
		 * A message to specify item interest.<br>
		 * A consumer sends this message.
		 */
		public final static int REQ_MSG = 256;

		/**
		 * A message to open and change an item.<br>
		 * A provider sends this message.
		 */
		public final static int REFRESH_MSG = 257;

		/**
		 * A message to change an item.<br>
		 * A provider sends this message.
		 */
		public final static int UPDATE_MSG = 258;

		/**
		 * A message to indicate the state of an item.<br>
		 * A provider sends this message.
		 */
		public final static int STATUS_MSG = 259;

		/**
		 * A message to solicit alteration of an item.<br>
		 * A consumer sends this message.
		 */
		public final static int POST_MSG = 260;

		/**
		 * A message to acknowledge the alteration of an item.<br>
		 * A provider sends this message.
		 */
		public final static int ACK_MSG = 261;

		/**
		 * A message that has no implicit market information semantics.<br>
		 * A consumer or provider may send this message.
		 */
		public final static int GENERIC_MSG = 262;

		/**
		 *  Indicates processing error.
		 */
		public final static int ERROR = 270;	
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