/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmError represents received Omm data who fails to process properly.
/// </summary>
///
/// Objects of OmmError class are returned when an error is detected while processing
/// received data. These objects are used for debugging purposes only.
///
/// <example>
/// void DecodeData(Data rcvdData)
/// {
/// 	if ( rcvdData.Code != Data.DataCode.BLANK )
/// 		switch ( rcvdData.DataType )
/// 		{
/// 		case DataTypes.UINT:
/// 			ulong val = ((OmmUInt)rcvdData).Value;
/// 			break;
///
/// 		...
///
/// 		case DataTypes.ERROR:
/// 			Console.WriteLine($"Failed to decode data. Error code is: {((OmmError)rcvdData).ErrorCodeAsString()}");
/// 			Console.WriteLine($"Received data is: {rcvdData.AsHex()}");
/// 			break;
/// 		}
/// }
/// </example>
///
/// OmmError is a read only class.
///
/// The usage of this class is limited to downcast operation only.
///
/// All methods in this class are \ref SingleThreaded.
///
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
///
public sealed class OmmError : Data
{

    #region Public members

    /// <summary>
    /// Represents error codes.
    /// </summary>
    public enum ErrorCodes : int
    {
        /// <summary>
        /// Indicates no error.
        /// </summary>
        NO_ERROR = 0,

        /// <summary>
        /// Indicates missing dictionary.
        /// </summary>
        NO_DICTIONARY = 1,

        /// <summary>
        /// Indicates internal iterator set failure.
        /// </summary>
        ITERATOR_SET_FAILURE = 2,

        /// <summary>
        /// Indicates internal iterator overrun failure.
        /// </summary>
        ITERATOR_OVERRUN = 3,

        /// <summary>
        /// Indicates field id was not found in dictionary.
        /// </summary>
        FIELD_ID_NOT_FOUND = 4,

        /// <summary>
        /// Indicates incomplete data.
        /// </summary>
        INCOMPLETE_DATA = 5,

        /// <summary>
        /// Indicates unsupported data type.
        /// </summary>
        UNSUPPORTED_DATA_TYPE = 6,

        /// <summary>
        /// Indicates set defined data is not present.
        /// </summary>
        NO_SET_DEFINITION = 7,

        /// <summary>
        /// Indicates unknown error.
        /// </summary>
        UNKNOWN_ERROR = 8
    }

    /// <summary>
    ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
    /// </summary>
    public override int DataType { get => Access.DataType.DataTypes.ERROR; }

    /// <summary>
    /// Returns the <see cref="ErrorCode"/> value as a string format.
    /// </summary>
    ///
    /// <returns>string representation of error code (e.g. "ITERATOR_SETFAILURE")</returns>
    public string ErrorCodeAsString()
    {
        return ErrorCode switch
        {
            ErrorCodes.NO_ERROR => NO_ERROR_STRING,
            ErrorCodes.NO_DICTIONARY => NO_DICTIONARY_STRING,
            ErrorCodes.FIELD_ID_NOT_FOUND => FIELD_ID_NOT_FOUND_STRING,
            ErrorCodes.ITERATOR_OVERRUN => ITERATOR_OVERRUN_STRING,
            ErrorCodes.ITERATOR_SET_FAILURE => ITERATOR_SET_FAILURE_STRING,
            ErrorCodes.INCOMPLETE_DATA => INCOMPLETE_DATA_STRING,
            ErrorCodes.NO_SET_DEFINITION => NO_SET_DEFINITION_STING,
            ErrorCodes.UNSUPPORTED_DATA_TYPE => UNSUPPORTED_DATA_TYPE_STRING,
            ErrorCodes.UNKNOWN_ERROR => UNKNOWN_ERROR_STRING,
            _ => DEFAULT_STRING + ErrorCode
        };
    }

    /// <summary>
    /// Returns ErrorCode.
    /// </summary>
    ///
    /// <value>error code</value>
    public ErrorCodes ErrorCode { get; internal set; }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmError"/> instance.</returns>
    public override string ToString()
    {
        return ToString(0);
    }

    #endregion

    #region Implementation details

    private const string NO_ERROR_STRING = "NoError";
    private const string NO_DICTIONARY_STRING = "NoDictionary";
    private const string ITERATOR_SET_FAILURE_STRING = "IteratorSetFailure";
    private const string ITERATOR_OVERRUN_STRING = "IteratorOverrun";
    private const string FIELD_ID_NOT_FOUND_STRING = "FieldIdNotFound";
    private const string INCOMPLETE_DATA_STRING = "IncompleteData";
    private const string UNSUPPORTED_DATA_TYPE_STRING = "UnsupportedDataType";
    private const string NO_SET_DEFINITION_STING = "NoSetDefinition";
    private const string UNKNOWN_ERROR_STRING = "UnknownError";
    private const string DEFAULT_STRING = "Unrecognized ErrorCode value ";

    internal OmmError()
    {
        Code = Data.DataCode.NO_CODE;
    }

    internal override CodecReturnCode Decode(DecodeIterator dIter)
    {
        throw new OmmInvalidUsageException("Unsupported attempt to Decode an OmmError()");
    }

    internal override string ToString(int indent)
    {
        m_ToString.Clear();

        Utilities.AddIndent(m_ToString, indent).Append("OmmError");
        ++indent;
        Utilities.AddIndent(m_ToString, indent, true).Append("  ErrorCode=\"").Append(ErrorCodeAsString()).Append('"');
        --indent;

        Utilities.AddIndent(m_ToString, indent, true).Append("OmmErrorEnd\n");

        return m_ToString.ToString();
    }

    #endregion
}
