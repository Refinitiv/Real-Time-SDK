/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Microsoft.VisualBasic;
using static LSEG.Ema.Access.OmmQos;
using System.Collections.Generic;
using System.Net;
using System.Text.Unicode;
using static LSEG.Ema.Access.RequestMsg;
using static LSEG.Eta.Rdm.Dictionary;
using static System.Formats.Asn1.AsnWriter;
using System.Xml.Linq;
using System;
using static System.Net.Mime.MediaTypeNames;

namespace LSEG.Ema.Access;


/// <summary>
/// DataType class provides represents Omm data types.
/// </summary>
/// <remarks>
/// DataType class is used to convert the numeric representation into string representation.
/// <example>
/// Code snippet:
/// <code>
/// ComplexTypeData payload = refreshMsg.Payload();
/// Console.WriteLine( $"Received payload is {payload.DataType}" );
/// </code>
/// </example>
/// </remarks>
public sealed class DataType
{
    private DataType()
    {
    }

    private const string REQMSG_STRING = "ReqMsg";
    private const string REFRESHMSG_STRING = "RefreshMsg";
    private const string UPDATEMSG_STRING = "UpdateMsg";
    private const string STATUSMSG_STRING = "StatusMsg";
    private const string POSTMSG_STRING = "PostMsg";
    private const string ACKMSG_STRING = "AckMsg";
    private const string GENERICMSG_STRING = "GenericMsg";
    private const string FIELDLIST_STRING = "FieldList";
    private const string ELEMENTLIST_STRING = "ElementList";
    private const string MAP_STRING = "Map";
    private const string VECTOR_STRING = "Vector";
    private const string SERIES_STRING = "Series";
    private const string FILTERLIST_STRING = "FilterList";
    private const string OPAQUE_STRING = "Opaque";
    private const string XML_STRING = "Xml";
    private const string JSON_STRING = "Json";
    private const string ANSIPAGE_STRING = "AnsiPage";
    private const string OMMARRAY_STRING = "OmmArray";
    private const string INT_STRING = "Int";
    private const string UINT_STRING = "UInt";
    private const string REAL_STRING = "Real";
    private const string FLOAT_STRING = "Float";
    private const string DOUBLE_STRING = "Double";
    private const string DATE_STRING = "Date";
    private const string TIME_STRING = "Time";
    private const string DATETIME_STRING = "DateTime";
    private const string QOS_STRING = "Qos";
    private const string STATE_STRING = "State";
    private const string ENUM_STRING = "Enum";
    private const string BUFFER_STRING = "Buffer";
    private const string ASCII_STRING = "Ascii";
    private const string UTF8_STRING = "Utf8";
    private const string RMTES_STRING = "Rmtes";
    private const string ERROR_STRING = "Error";
    private const string NODATA_STRING = "NoData";
    private const string DEFAULTDT_STRING = "Unknown DataType value ";


    /// <summary>
    /// Represents data type enumerations.
    /// </summary>
    public static class DataTypes
    {
        
        /// <summary>
        /// A signed integer. Can currently represent a value of up to 63 bits  along with a one bit sign(positive or negative).
        /// </summary>
        public const int INT = 3;

        /// <summary>
        /// An unsigned integer. Can currently represent an unsigned value with precision of up to 64 bits.
        /// </summary>
        public const int UINT = 4;

        /// <summary>
        /// A 4-byte value that has a range of -3.4e38 to +3.4e38  with an accuracy of 6 to 7 decimal digits.<br/>
        /// This value is compliant with the IEEE-754 standard.
        /// </summary>
        public const int FLOAT = 5;

        /// <summary>
        /// An 8-byte value that has a range of -1.7e308 to +1.7e308 with an accuracy of 14 to 15 digits.<br/>
        /// This value is compliant with the IEEE-754 standard.
        /// </summary>
        public const int DOUBLE = 6;

        /// <summary>
        ///  An 8-byte precision (19-20 decimal places) fixed-placed representation of a numeric with either a fractional or exponential part.<br/>
        ///  The range of a fractional part is 1/2 through 1/256.<br/>
        ///  The range of an exponential part is 10-14 through 10+7.
        /// </summary>
        public const int REAL = 8;

        /// <summary>
        /// A 4 byte value that represents a date with month, day, and year values.
        /// </summary>
        public const int DATE = 9;

        /// <summary>
        /// A 3 or 5 byte value that includes information for hours, minutes, seconds,
        /// and optional milliseconds, microseconds and nanoseconds.
        /// </summary>
        public const int TIME = 10;

        /// <summary>
        /// A 7 or 9 byte combination of a Date and a Time.
        /// </summary>
        public const int DATETIME = 11;

        /// <summary>
        /// Quality Of Service, represents quality of service information such as timeliness and rate.
        /// </summary>
        public const int QOS = 12;

        /// <summary>
        /// State, represents data and stream state information. Allows a user to  send state information 
        /// as part of data payload.Similar information can also be conveyed in several EMA message headers.
        /// </summary>
        public const int STATE = 13;

        /// <summary>
        /// A 2-byte signed value that can be expanded to a language specific string in an enumerated type dictionary.
        /// </summary>
        public const int ENUM = 14;

        /// <summary>
        /// The EMA Array type allows users to represent a simple base primitive type list of all primitive types 
        /// except OmmArray. The user can specify whether each member of the array is a variable or fixed-length.
        /// Because the array is a primitive type, if any value in the array updates, the entire array must be resent.
        /// </summary>
        public const int ARRAY = 15;

        /// <summary>
        /// A general purpose buffer.
        /// /// </summary>
        public const int BUFFER = 16;

        /// <summary>
        /// An 8-bit characters encoding using the Reuters Basic Character Set (RBCS).<br/>
        /// The first 128 characters are equivalent to the ASCII character set(ANSI X3.4-1968).
        /// </summary>
        public const int ASCII = 17;

        /// <summary>
        /// A UTF-8 encoding of ISO 10646 (specified in section 3.9 of the Unicode 4.0 standard and IETF's RFC 3629).
        /// </summary>
        public const int UTF8 = 18;
         
        /// <summary>
        /// An encoding with a multilingual text encoding standard.<br/>
        /// RMTES uses ISO 2022 escape sequences to select the character sets used.<br/>
        /// RMTES provides support for the Reuters Basic Character Set, UTF-8,<br/>
        /// Japanese Latin and Katakana (JIS C 6220 - 1969), Japanese Kanji(JIS X 0208 - 1990),<br/>
        /// and Chinese National Standard(CNS 11643-1986).<br/>
        /// RMTES also supports RREP sequences for character repetition and RHPA sequences for partial updates.
        /// </summary>
        public const int RMTES = 19;
 
        /// <summary>
        /// No Omm data value is present.
        /// </summary>
        public const int NO_DATA = 128;

        /// <summary>
        /// A buffer containing data that is formatted according to an external agreement between the Provider and Consumer.
        /// </summary>
        public const int OPAQUE = 130;

        /// <summary>
        /// An XML buffer.
        /// </summary>
        public const int XML = 131;

        /// <summary>
        /// A highly optimized, non-uniform container type that contains field identifier-value paired entries.<br/>
        /// FieldId refers to specific name and type information as defined in an external field 
        /// dictionary(such as RDMFieldDictionary).<br/>
        /// FieldList's entry can house any data types, base primitive types, and container types.<br/>
        /// If the information and entry being updated contains a primitive type, any previously 
        /// stored or displayed data is replaced.<br/>
        /// If the entry contains another container type, action values associated with that type
        /// specify how to update the information.
        /// </summary>
        public const int FIELD_LIST = 132;

        /// <summary>
        /// A self-describing, non-uniform container type with each entry containing name, dataType, and 
        /// a value. This type is equivalent to FieldList, but without the optimizations provided through 
        /// fieldId use. ElementList's entry can house any DataType, including base primitive types, and container 
        /// types.<br/>
        /// If the updating information and entry contain a primitive type, any previously stored or displayed 
        /// data is replaced.<br/>
        /// If the entry contains another container type, action values associated with that type specify how to 
        /// update the information.
        /// </summary>
        public const int ELEMENT_LIST = 133;

        /// <summary>
        /// An AnsiPage buffer.
        /// </summary>
        public const int ANSI_PAGE = 134;

        /// <summary>
        /// A non-uniform container of filterId-value paired entries. A filterId corresponds to one of 32 
        /// possible bit-value identifiers, typically defined by a domain model specification.<br/>
        /// FilterId's can be used to indicate interest or presence of specific entries through the inclusion 
        /// of the filterId in the message key's filter member.<br/>
        /// FilterList's entry can house only container types. Each entry has an associated action, which 
        /// informs the user of how to Apply the information stored in the entry.
        /// </summary>
        public const int FILTER_LIST = 135;

        /// <summary>
        /// A uniform container of position index-value paired entries.<br/>
        /// Each entry's index is represented by an unsigned integer.<br/>
        /// Each entry has an associated action, which informs the user of how to apply the information stored in the entry.
        /// </summary>
        public const int VECTOR = 136;

        /// <summary>
        /// A container of key-value paired entries. Map is a uniform type for key primitive type and the entry's 
        /// container type. Map's entry can include only containers.<br/>
        /// Each entry's key is a base primitive type.<br/>
        /// Each entry has an associated action, which informs the user of how to apply the information stored in the entry.
        /// </summary>
        public const int MAP = 137;

        /// <summary>
        /// A uniform container that is often used to represent table-based information, where no explicit 
        /// indexing is present or required. As entries are received, the user should append them to any 
        /// previously-received entries.<br/>
        /// Series's entry can include only container types and do not contain explicit actions; though as entries 
        /// are received,the user should append them to any previously received entries.
        /// </summary>
        public const int SERIES = 138;

        /// <summary>
        /// Indicates that the contents are another message. This allows the application to house a 
        /// message within a message or a message within another container's entries.<br/>
        /// This type is typically used with posting.
        /// </summary>
        public const int MSG = 141;

        /// <summary>
        /// An JSON buffer.
        /// </summary>
        public const int JSON = 142;

        /// <summary>
        /// A message to specify item interest.<br/>
        /// A consumer sends this message.
        /// </summary>
        public const int REQ_MSG = 256;

        /// <summary>
        /// A message to open and change an item.<br/>
        /// A provider sends this message.
        /// </summary>
        public const int REFRESH_MSG = 257;

        /// <summary>
        /// A message to change an item.<br/>
        /// A provider sends this message.
        /// </summary>        
        public const int UPDATE_MSG = 258;

        /// <summary>
        /// A message to indicate the state of an item.<br/>
        /// A provider sends this message.
        /// </summary>
        public const int STATUS_MSG = 259;

         /// <summary>
         /// A message to solicit alteration of an item.<br/>
         /// A consumer sends this message.
         /// </summary>
        public const int POST_MSG = 260;

        /// <summary>
        /// A message to acknowledge the alteration of an item.<br/>
        /// A provider sends this message.
        /// </summary>
        public const int ACK_MSG = 261;

        /// <summary>
        /// A message that has no implicit market information semantics.<br/>
        /// A consumer or provider may send this message.
        /// </summary>
        public const int GENERIC_MSG = 262;

        /// <summary>
        /// Indicates processing error.
        /// </summary>
        public const int ERROR = 270;
    }

    /// <summary>
    /// Returns OMM data type as string value
    /// </summary>
    /// <param name="dataType">numeric value of data type <see cref="DataTypes"/></param>
    /// <returns>string containing name of the data type</returns>
    public static string AsString(int dataType)
    {
        return dataType switch
        {
            DataTypes.REQ_MSG => REQMSG_STRING,
            DataTypes.REFRESH_MSG => REFRESHMSG_STRING,
            DataTypes.STATUS_MSG => STATUSMSG_STRING,
            DataTypes.UPDATE_MSG => UPDATEMSG_STRING,
            DataTypes.POST_MSG => POSTMSG_STRING,
            DataTypes.ACK_MSG => ACKMSG_STRING,
            DataTypes.GENERIC_MSG => GENERICMSG_STRING,
            DataTypes.FIELD_LIST => FIELDLIST_STRING,
            DataTypes.ELEMENT_LIST => ELEMENTLIST_STRING,
            DataTypes.MAP => MAP_STRING,
            DataTypes.VECTOR => VECTOR_STRING,
            DataTypes.SERIES => SERIES_STRING,
            DataTypes.FILTER_LIST => FILTERLIST_STRING,
            DataTypes.OPAQUE => OPAQUE_STRING,
            DataTypes.XML => XML_STRING,
            DataTypes.JSON => JSON_STRING,
            DataTypes.ANSI_PAGE => ANSIPAGE_STRING,
            DataTypes.ARRAY => OMMARRAY_STRING,
            DataTypes.INT => INT_STRING,
            DataTypes.UINT => UINT_STRING,
            DataTypes.REAL => REAL_STRING,
            DataTypes.FLOAT => FLOAT_STRING,
            DataTypes.DOUBLE => DOUBLE_STRING,
            DataTypes.DATE => DATE_STRING,
            DataTypes.TIME => TIME_STRING,
            DataTypes.DATETIME => DATETIME_STRING,
            DataTypes.QOS => QOS_STRING,
            DataTypes.STATE => STATE_STRING,
            DataTypes.ENUM => ENUM_STRING,
            DataTypes.BUFFER => BUFFER_STRING,
            DataTypes.ASCII => ASCII_STRING,
            DataTypes.UTF8 => UTF8_STRING,
            DataTypes.RMTES => RMTES_STRING,
            DataTypes.NO_DATA => NODATA_STRING,
            DataTypes.ERROR => ERROR_STRING,
            _ => DEFAULTDT_STRING + dataType,
        };
    }
}
