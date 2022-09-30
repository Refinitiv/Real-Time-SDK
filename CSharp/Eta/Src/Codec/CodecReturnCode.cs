/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace Refinitiv.Eta.Codec
{
	/// <summary>
	/// Return codes associated with ETA Codec package.
	/// </summary>
	public enum CodecReturnCode
	{
		/* -35 through -49 reserved */

		/// <summary>
		/// Indicates that a value being encoded using a set definition exceeds the
		/// allowable range for the type as specified in the definition.
		/// </summary>
		VALUE_OUT_OF_RANGE = -34,

		/// <summary>
		/// Indicates that the application is attempting to nest more levels of
		/// content than is supported by a single <seealso cref="EncodeIterator"/>. There is a
		/// limit of 16 levels.
		/// </summary>
		ITERATOR_OVERRUN = -33,

		/// <summary>
		/// Indicates that content contains a duplicate set definition which collides
		/// with a definition already received and stored in the database.
		/// </summary>
		DUPLICATE_LOCAL_SET_DEFS = -32,

		/// <summary>
		/// Indicates that encoding exceeds the maximum number of allowed local set
		/// definitions.
		/// </summary>
		TOO_MANY_LOCAL_SET_DEFS = -31,

		/// <summary>
		/// Indicates that the setId associated with a contained definition exceeds
		/// the allowable value.
		/// </summary>
		ILLEGAL_LOCAL_SET_DEF = -30,

		/// <summary>
		/// Indicates that invalid data was provided to the invoked method </summary>
		INVALID_DATA = -29,

		/// <summary>
		/// Indicates that <seealso cref="FieldList"/> or <seealso cref="ElementList"/> encoding requires
		/// a set definition database which was not provided
		/// </summary>
		SET_DEF_NOT_PROVIDED = -27,

		/// <summary>
		/// Indicates that the <seealso cref="Buffer"/> contained on the <seealso cref="DecodeIterator"/>
		/// does not have enough data to properly decode data.
		/// </summary>
		INCOMPLETE_DATA = -26,

		/// <summary>
		/// Indicates that encoding functionality was used in an unexpected sequence
		/// or the called method is not expected in this encoding.
		/// </summary>
		UNEXPECTED_ENCODER_CALL = -25,

		/// <summary>
		/// Indicates that the type is not supported for the operation being
		/// performed. This might indicate a primitiveType is used where a
		/// containerType is expected or the opposite.
		/// </summary>
		UNSUPPORTED_DATA_TYPE = -24,

		/// <summary>
		/// Indicates that the invoked method does not contain encoding
		/// functionality for the specified type. There might be other ways to encode
		/// content or the type might be invalid in the combination being performed.
		/// </summary>
		ENCODING_UNAVAILABLE = -23,

		/// <summary>
		/// Indicates an invalid argument was provided.
		/// </summary>
		INVALID_ARGUMENT = -22,

		/// <summary>
		/// Indicates that the <seealso cref="Buffer"/> contained on the <seealso cref="EncodeIterator"/>
		/// does not contain sufficient space into which to encode
		/// </summary>
		BUFFER_TOO_SMALL = -21,

		/// <summary>
		/// Indicates that the specified version is not supported
		/// </summary>
		VERSION_NOT_SUPPORTED = -16,
		/* -20 through -17 reserved */

		/* General Failure and Success Codes */
		/// <summary>
		/// Indicates a general failure, used when no specific details are available
		/// </summary>
		FAILURE = -1,

		/// <summary>
		/// General success return code
		/// </summary>
		SUCCESS = 0,

		/* 3 through 9 reserved */

		/* Encoder/Decoder Success Return Codes */
		/// <summary>
		/// Indicates that the dictionary encoding utility method successfully
		/// encoded part of a dictionary message (because dictionary messages tend to
		/// be large, they might be segmented into a multi-part message).
		/// </summary>
		DICT_PART_ENCODED = 10,

		/// <summary>
		/// Indicates that initial message encoding was successful and now the
		/// application should encode msgKey attributes. This return occurs when
		/// calling <seealso cref="IMsg.EncodeInit(EncodeIterator, int)"/> if the application
		/// indicates that the message should include msgKey attributes without
		/// populating pre-encoded data into the <seealso cref="IMsgKey.EncodedAttrib()"/>.
		/// </summary>
		ENCODE_MSG_KEY_ATTRIB = 11,

		/// <summary>
		/// Indicates that initial message encoding (and msgKey attribute encoding)
		/// was successful, and the application should now encode extendedHeader
		/// content. This return occurs if an application indicates that the message
		/// should include extendedHeader content when calling
		/// <seealso cref="IMsg.EncodeInit(EncodeIterator, int)"/> without populating
		/// pre-encoded data into the <seealso cref="IMsg.ExtendedHeader()"/>.
		/// </summary>
		ENCODE_EXTENDED_HEADER = 12,

		/// <summary>
		/// Indicates that initial encoding succeeded and that the application should
		/// now encode the specified containerType.
		/// </summary>
		ENCODE_CONTAINER = 13,

		/// <summary>
		/// Codec Success: The end of the current container has been reached while
		/// decoding
		/// </summary>
		END_OF_CONTAINER = 14,

		/// <summary>
		/// Indicates that the decoded primitiveType is a blank value. The contents
		/// of the primitive type should be ignored, any display or calculation
		/// should treat the value as blank.
		/// </summary>
		BLANK_DATA = 15,

		/// <summary>
		/// Indicates that the containerType being decoded contains no data and was
		/// decoded from an empty payload. Informs the application not to continue to
		/// decode container entries (as none exist).
		/// </summary>
		NO_DATA = 16,

		/// <summary>
		/// Indicates that <seealso cref="FieldList"/> or <seealso cref="ElementList"/> encoding is
		/// complete. Additionally encoded entries are encoded in the standard way
		/// with no additional data optimizations
		/// </summary>
		SET_COMPLETE = 17,

		/// <summary>
		/// Indicates that <seealso cref="FieldList"/> or <seealso cref="ElementList"/> decoding skipped
		/// over contained, set-defined data because no set definition database was
		/// provided. Any standard encoded entries will still be decoded.
		/// </summary>
		SET_SKIPPED = 18,

		/// <summary>
		/// Indicates that set definition database decoding completed successfully
		/// but there was no information in the database.
		/// </summary>
		SET_DEF_DB_EMPTY = 19
	}

    /// <summary>
    /// This extenstion class converts <see cref="CodecReturnCode"/> to string.
    /// </summary>
    public static class CodecReturnCodeExtensions
    {
        /// <summary>
        /// Provides string representation for an encoder/decoder return code.
        /// </summary>
        /// <param name="retCode"> <seealso cref="CodecReturnCode"/> enumeration to convert to string
        /// </param>
        /// <returns> string representation for a encoder/decoder return code </returns>
        public static string GetAsString(this CodecReturnCode retCode)
        {
            switch (retCode)
            {
                case CodecReturnCode.VALUE_OUT_OF_RANGE:
                    return "VALUE_OUT_OF_RANGE";
                case CodecReturnCode.ITERATOR_OVERRUN:
                    return "ITERATOR_OVERRUN";
                case CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS:
                    return "DUPLICATE_LOCAL_SET_DEFS";
                case CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS:
                    return "TOO_MANY_LOCAL_SET_DEFS";
                case CodecReturnCode.ILLEGAL_LOCAL_SET_DEF:
                    return "ILLEGAL_LOCAL_SET_DEF";
                case CodecReturnCode.INVALID_DATA:
                    return "INVALID_DATA";
                case CodecReturnCode.SET_DEF_NOT_PROVIDED:
                    return "SET_DEF_NOT_PROVIDED";
                case CodecReturnCode.INCOMPLETE_DATA:
                    return "INCOMPLETE_DATA";
                case CodecReturnCode.UNEXPECTED_ENCODER_CALL:
                    return "UNEXPECTED_ENCODER_CALL";
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    return "UNSUPPORTED_DATA_TYPE";
                case CodecReturnCode.ENCODING_UNAVAILABLE:
                    return "ENCODING_UNAVAILABLE";
                case CodecReturnCode.INVALID_ARGUMENT:
                    return "INVALID_ARGUMENT";
                case CodecReturnCode.BUFFER_TOO_SMALL:
                    return "BUFFER_TOO_SMALL";
                case CodecReturnCode.FAILURE:
                    return "FAILURE";
                case CodecReturnCode.SUCCESS:
                    return "SUCCESS";
                case CodecReturnCode.DICT_PART_ENCODED:
                    return "DICT_PART_ENCODED";
                case CodecReturnCode.ENCODE_MSG_KEY_ATTRIB:
                    return "ENCODE_MSG_KEY_ATTRIB";
                case CodecReturnCode.ENCODE_EXTENDED_HEADER:
                    return "ENCODE_EXTENDED_HEADER";
                case CodecReturnCode.ENCODE_CONTAINER:
                    return "ENCODE_CONTAINER";
                case CodecReturnCode.END_OF_CONTAINER:
                    return "END_OF_CONTAINER";
                case CodecReturnCode.BLANK_DATA:
                    return "BLANK_DATA";
                case CodecReturnCode.NO_DATA:
                    return "NO_DATA";
                case CodecReturnCode.SET_COMPLETE:
                    return "SET_COMPLETE";
                case CodecReturnCode.SET_SKIPPED:
                    return "SET_SKIPPED";
                case CodecReturnCode.SET_DEF_DB_EMPTY:
                    return "SET_DEF_DB_EMPTY";
                default:
                    return Convert.ToString(retCode);
            }
        }

        /// <summary>
        /// Provides detailed information for an encoder/decoder return code.
        /// </summary>
        /// <param name="retCode"> <seealso cref="CodecReturnCode"/> enumeration to return info for
        /// </param>
        /// <returns> detailed information for a encoder/decoder return code </returns>
        public static string GetAsInfo(this CodecReturnCode retCode)
        {
            switch (retCode)
            {
                case CodecReturnCode.VALUE_OUT_OF_RANGE:
                    return "Failure: A value being encoded into a set is outside of the valid range of the type given by that set.";
                case CodecReturnCode.ITERATOR_OVERRUN:
                    return "Failure: Iterator is nested too deeply. There is a limit of 16 levels.";
                case CodecReturnCode.DUPLICATE_LOCAL_SET_DEFS:
                    return "Failure: A duplicate set definition has been received.";
                case CodecReturnCode.TOO_MANY_LOCAL_SET_DEFS:
                    return "Failure: Maximum number of set definitions has been exceeded.";
                case CodecReturnCode.ILLEGAL_LOCAL_SET_DEF:
                    return "Failure: Set definition is not valid.";
                case CodecReturnCode.INVALID_DATA:
                    return "Failure: Invalid data provided to method.";
                case CodecReturnCode.SET_DEF_NOT_PROVIDED:
                    return "Failure: A Database containing the Set Definition for encoding the desired set was not provided.";
                case CodecReturnCode.INCOMPLETE_DATA:
                    return "Failure: Not enough data was provided.";
                case CodecReturnCode.UNEXPECTED_ENCODER_CALL:
                    return "Failure: An encoder was used in an unexpected sequence.";
                case CodecReturnCode.UNSUPPORTED_DATA_TYPE:
                    return "Failure: The data type is unsupported.";
                case CodecReturnCode.ENCODING_UNAVAILABLE:
                    return "Failure: No encoder is available for the data type specified.";
                case CodecReturnCode.INVALID_ARGUMENT:
                    return "Failure: An invalid argument was provided.";
                case CodecReturnCode.BUFFER_TOO_SMALL:
                    return "Failure: The buffer provided does not have sufficient space to perform the operation.";
                case CodecReturnCode.FAILURE:
                    return "Failure: ETA general failure return code.";
                case CodecReturnCode.SUCCESS:
                    return "Success: ETA general success return code.";
                case CodecReturnCode.DICT_PART_ENCODED:
                    return "Success: Successfully encoded part of a dictionary message, returned from the  dictionary processing methods.";
                case CodecReturnCode.ENCODE_MSG_KEY_ATTRIB:
                    return "Success: The user should now encode their msgKey opaque data.";
                case CodecReturnCode.ENCODE_EXTENDED_HEADER:
                    return "Success: The user should now encode their extended header information.";
                case CodecReturnCode.ENCODE_CONTAINER:
                    return "Success: The user should encode the container type payload.";
                case CodecReturnCode.END_OF_CONTAINER:
                    return "Success: The end of the current container has been reached while decoding.";
                case CodecReturnCode.BLANK_DATA:
                    return "Success: Decoded data is a Blank.";
                case CodecReturnCode.NO_DATA:
                    return "Success: Container was decoded from an empty payload. The user should not try to decode any entries.";
                case CodecReturnCode.SET_COMPLETE:
                    return "Success: The encoded entry completed a FieldList or ElementList set. Subsequent entries will be encoded normally.";
                case CodecReturnCode.SET_SKIPPED:
                    return "Success: The FieldList or ElementList contains set data and the necessary definition was not provided. Standard entries may still be decoded.";
                case CodecReturnCode.SET_DEF_DB_EMPTY:
                    return "Success: A Set Definition Database decoded successfully but contained no definitions.";
                default:
                    return "Unknown.";
            }
        }
    }
}