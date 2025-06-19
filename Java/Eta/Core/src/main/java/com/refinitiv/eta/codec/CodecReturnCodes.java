/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/**
 * Return codes associated with ETA Codec package.
 */
public class CodecReturnCodes
{
    // CodecReturnCodes class cannot be instantiated
    private CodecReturnCodes()
    {
        throw new AssertionError();
    }

    /* -35 through -49 reserved */

    /**
     * Indicates that a value being encoded using a set definition exceeds the
     * allowable range for the type as specified in the definition.
     */
    public static final int VALUE_OUT_OF_RANGE = -34;

    /**
     * Indicates that the application is attempting to nest more levels of
     * content than is supported by a single {@link EncodeIterator}. There is a
     * limit of 16 levels.
     */
    public static final int ITERATOR_OVERRUN = -33;

    /**
     * Indicates that content contains a duplicate set definition which collides
     * with a definition already received and stored in the database.
     */
    public static final int DUPLICATE_LOCAL_SET_DEFS = -32;

    /**
     * Indicates that encoding exceeds the maximum number of allowed local set
     * definitions.
     */
    public static final int TOO_MANY_LOCAL_SET_DEFS = -31;

    /**
     * Indicates that the setId associated with a contained definition exceeds
     * the allowable value.
     */
    public static final int ILLEGAL_LOCAL_SET_DEF = -30;

    /** Indicates that invalid data was provided to the invoked method */
    public static final int INVALID_DATA = -29;

    /**
     * Indicates that {@link FieldList} or {@link ElementList} encoding requires
     * a set definition database which was not provided
     */
    public static final int SET_DEF_NOT_PROVIDED = -27;

    /**
     * Indicates that the {@link Buffer} contained on the {@link DecodeIterator}
     * does not have enough data to properly decode data.
     */
    public static final int INCOMPLETE_DATA = -26;

    /**
     * Indicates that encoding functionality was used in an unexpected sequence
     * or the called method is not expected in this encoding.
     */
    public static final int UNEXPECTED_ENCODER_CALL = -25;

    /**
     * Indicates that the type is not supported for the operation being
     * performed. This might indicate a primitiveType is used where a
     * containerType is expected or the opposite.
     */
    public static final int UNSUPPORTED_DATA_TYPE = -24;

    /**
     * Indicates that the invoked method does not contain encoding
     * functionality for the specified type. There might be other ways to encode
     * content or the type might be invalid in the combination being performed.
     */
    public static final int ENCODING_UNAVAILABLE = -23;

    /**
     * Indicates an invalid argument was provided.
     */
    public static final int INVALID_ARGUMENT = -22;

    /**
     * Indicates that the {@link Buffer} contained on the {@link EncodeIterator}
     * does not contain sufficient space into which to encode
     */
    public static final int BUFFER_TOO_SMALL = -21;

    /**
     * Indicates that the specified version is not supported
     */
    public static final int VERSION_NOT_SUPPORTED = -16;
    /* -20 through -17 reserved */

    /* General Failure and Success Codes */
    /**
     * Indicates a general failure, used when no specific details are available
     */
    public static final int FAILURE = -1;

    /**
     * General success return code
     */
    public static final int SUCCESS = 0;

    /* 3 through 9 reserved */

    /* Encoder/Decoder Success Return Codes */
    /**
     * Indicates that the dictionary encoding utility method successfully
     * encoded part of a dictionary message (because dictionary messages tend to
     * be large, they might be segmented into a multi-part message).
     */
    public static final int DICT_PART_ENCODED = 10;

    /**
     * Indicates that initial message encoding was successful and now the
     * application should encode msgKey attributes. This return occurs when
     * calling {@link Msg#encodeInit(EncodeIterator, int)} if the application
     * indicates that the message should include msgKey attributes without
     * populating pre-encoded data into the {@link MsgKey#encodedAttrib()}.
     */
    public static final int ENCODE_MSG_KEY_ATTRIB = 11;

    /**
     * Indicates that initial message encoding (and msgKey attribute encoding)
     * was successful, and the application should now encode extendedHeader
     * content. This return occurs if an application indicates that the message
     * should include extendedHeader content when calling
     * {@link Msg#encodeInit(EncodeIterator, int)} without populating
     * pre-encoded data into the {@link Msg#extendedHeader()}.
     */
    public static final int ENCODE_EXTENDED_HEADER = 12;

    /**
     * Indicates that initial encoding succeeded and that the application should
     * now encode the specified containerType.
     */
    public static final int ENCODE_CONTAINER = 13;

    /**
     * Codec Success: The end of the current container has been reached while
     * decoding
     */
    public static final int END_OF_CONTAINER = 14;

    /**
     * Indicates that the decoded primitiveType is a blank value. The contents
     * of the primitive type should be ignored; any display or calculation
     * should treat the value as blank.
     */
    public static final int BLANK_DATA = 15;

    /**
     * Indicates that the containerType being decoded contains no data and was
     * decoded from an empty payload. Informs the application not to continue to
     * decode container entries (as none exist).
     */
    public static final int NO_DATA = 16;

    /**
     * Indicates that {@link FieldList} or {@link ElementList} encoding is
     * complete. Additionally encoded entries are encoded in the standard way
     * with no additional data optimizations
     */
    public static final int SET_COMPLETE = 17;

    /**
     * Indicates that {@link FieldList} or {@link ElementList} decoding skipped
     * over contained, set-defined data because no set definition database was
     * provided. Any standard encoded entries will still be decoded.
     */
    public static final int SET_SKIPPED = 18;

    /**
     * Indicates that set definition database decoding completed successfully
     * but there was no information in the database.
     */
    public static final int SET_DEF_DB_EMPTY = 19;

    /**
     * Provides string representation for an encoder/decoder return code.
     * 
     * @param retCode {@link CodecReturnCodes} enumeration to convert to string
     * 
     * @return string representation for a encoder/decoder return code
     */
    public static String toString(int retCode)
    {
        switch (retCode)
        {
            case VALUE_OUT_OF_RANGE:
                return "VALUE_OUT_OF_RANGE";
            case ITERATOR_OVERRUN:
                return "ITERATOR_OVERRUN";
            case DUPLICATE_LOCAL_SET_DEFS:
                return "DUPLICATE_LOCAL_SET_DEFS";
            case TOO_MANY_LOCAL_SET_DEFS:
                return "TOO_MANY_LOCAL_SET_DEFS";
            case ILLEGAL_LOCAL_SET_DEF:
                return "ILLEGAL_LOCAL_SET_DEF";
            case INVALID_DATA:
                return "INVALID_DATA";
            case SET_DEF_NOT_PROVIDED:
                return "SET_DEF_NOT_PROVIDED";
            case INCOMPLETE_DATA:
                return "INCOMPLETE_DATA";
            case UNEXPECTED_ENCODER_CALL:
                return "UNEXPECTED_ENCODER_CALL";
            case UNSUPPORTED_DATA_TYPE:
                return "UNSUPPORTED_DATA_TYPE";
            case ENCODING_UNAVAILABLE:
                return "ENCODING_UNAVAILABLE";
            case INVALID_ARGUMENT:
                return "INVALID_ARGUMENT";
            case BUFFER_TOO_SMALL:
                return "BUFFER_TOO_SMALL";
            case FAILURE:
                return "FAILURE";
            case SUCCESS:
                return "SUCCESS";
            case DICT_PART_ENCODED:
                return "DICT_PART_ENCODED";
            case ENCODE_MSG_KEY_ATTRIB:
                return "ENCODE_MSG_KEY_ATTRIB";
            case ENCODE_EXTENDED_HEADER:
                return "ENCODE_EXTENDED_HEADER";
            case ENCODE_CONTAINER:
                return "ENCODE_CONTAINER";
            case END_OF_CONTAINER:
                return "END_OF_CONTAINER";
            case BLANK_DATA:
                return "BLANK_DATA";
            case NO_DATA:
                return "NO_DATA";
            case SET_COMPLETE:
                return "SET_COMPLETE";
            case SET_SKIPPED:
                return "SET_SKIPPED";
            case SET_DEF_DB_EMPTY:
                return "SET_DEF_DB_EMPTY";
            default:
                return Integer.toString(retCode);
        }
    }

    /**
     * Provides detailed information for an encoder/decoder return code.
     * 
     * @param retCode {@link CodecReturnCodes} enumeration to return info for
     * 
     * @return detailed information for a encoder/decoder return code
     */
    public static String info(int retCode)
    {
        switch (retCode)
        {
            case VALUE_OUT_OF_RANGE:
                return "Failure: A value being encoded into a set is outside of the valid range of the type given by that set.";
            case ITERATOR_OVERRUN:
                return "Failure: Iterator is nested too deeply. There is a limit of 16 levels.";
            case DUPLICATE_LOCAL_SET_DEFS:
                return "Failure: A duplicate set definition has been received.";
            case TOO_MANY_LOCAL_SET_DEFS:
                return "Failure: Maximum number of set definitions has been exceeded.";
            case ILLEGAL_LOCAL_SET_DEF:
                return "Failure: Set definition is not valid.";
            case INVALID_DATA:
                return "Failure: Invalid data provided to method.";
            case SET_DEF_NOT_PROVIDED:
                return "Failure: A Database containing the Set Definition for encoding the desired set was not provided.";
            case INCOMPLETE_DATA:
                return "Failure: Not enough data was provided.";
            case UNEXPECTED_ENCODER_CALL:
                return "Failure: An encoder was used in an unexpected sequence.";
            case UNSUPPORTED_DATA_TYPE:
                return "Failure: The data type is unsupported.";
            case ENCODING_UNAVAILABLE:
                return "Failure: No encoder is available for the data type specified.";
            case INVALID_ARGUMENT:
                return "Failure: An invalid argument was provided.";
            case BUFFER_TOO_SMALL:
                return "Failure: The buffer provided does not have sufficient space to perform the operation.";
            case FAILURE:
                return "Failure: ETA general failure return code.";
            case SUCCESS:
                return "Success: ETA general success return code.";
            case DICT_PART_ENCODED:
                return "Success: Successfully encoded part of a dictionary message, returned from the  dictionary processing methods.";
            case ENCODE_MSG_KEY_ATTRIB:
                return "Success: The user should now encode their msgKey opaque data.";
            case ENCODE_EXTENDED_HEADER:
                return "Success: The user should now encode their extended header information.";
            case ENCODE_CONTAINER:
                return "Success: The user should encode the container type payload.";
            case END_OF_CONTAINER:
                return "Success: The end of the current container has been reached while decoding.";
            case BLANK_DATA:
                return "Success: Decoded data is a Blank.";
            case NO_DATA:
                return "Success: Container was decoded from an empty payload. The user should not try to decode any entries.";
            case SET_COMPLETE:
                return "Success: The encoded entry completed a FieldList or ElementList set. Subsequent entries will be encoded normally.";
            case SET_SKIPPED:
                return "Success: The FieldList or ElementList contains set data and the necessary definition was not provided. Standard entries may still be decoded.";
            case SET_DEF_DB_EMPTY:
                return "Success: A Set Definition Database decoded successfully but contained no definitions.";
            default:
                return "Unknown.";
        }
    }
}
