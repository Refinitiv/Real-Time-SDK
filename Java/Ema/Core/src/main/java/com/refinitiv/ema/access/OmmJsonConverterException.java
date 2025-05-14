/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

/**
 * {@link OmmJsonConverterException} is thrown when EMA fails to perform for RWF/JSON conversion.
 */
public abstract class OmmJsonConverterException extends OmmException {

    private static final long serialVersionUID = 6659606118768100493L;

    /**
     * No specific error code.
     */
    public static int NO_ERROR_ENUM = 0;

    /**
     * General failure.
     */
    public static int FAILURE_ENUM = -1;

    /**
     * There are no buffers available from the buffer pool.
     */
    public static int NO_BUFFERS_ENUM = -4;

    /**
     * The buffer provided does not have sufficient space to perform the operation.
     */
    public static int BUFFER_TOO_SMALL = -21;

    /**
     * An invalid argument was provided.
     */
    public static int INVALID_ARGUMENT = -22;

    /**
     * No encoder is available for the data type specified.
     */
    public static int ENCODING_UNAVAILABLE = -23;

    /**
     * The data type is unsupported, may indicate invalid containerType or primitiveType specified.
     */
    public static int UNSUPPORTED_DATA_TYPE = -24;

    /**
     * An encoder was used in an unexpected sequence.
     */
    public static int UNEXPECTED_ENCODER_CALL = -25;

    /**
     * Not enough data was provided.
     */
    public static int INCOMPLETE_DATA = -26;

    /**
     * A Database containing the Set Definition for encoding the desired set was not provided.
     */
    public static int SET_DEF_NOT_PROVIDED = -27;

    /**
     * Invalid data provided to function.
     */
    public static int INVALID_DATA = -29;

    /**
     * Set definition is not valid.
     */
    public static int ILLEGAL_LOCAL_SET_DEF = -30;

    /**
     * Maximum number of set definitions has been exceeded.
     */
    public static int TOO_MANY_LOCAL_SET_DEFS = -31;

    /**
     * A duplicate set definition has been received.
     */
    public static int DUPLICATE_LOCAL_SET_DEFS = -32;

    /**
     * Iterator is nested too deeply. There is a limit of 16 levels.
     */
    public static int ITERATOR_OVERRUN = -33;

    /**
     * A value being encoded into a set is outside of the valid range of the type given by that set.
     */
    public static int VALUE_OUT_OF_RANGE = -34;

    /**
     * A display string had multiple enumerated values that correspond to it.
     */
    public static int DICT_DUPLICATE_VALUE = -35;

    /**
     * Returns an error code to describe the error case defined in the constants of {@link OmmJsonConverterException}.
     * @return an error code.
     */
    public abstract int getErrorCode();

    /**
     * Returns session information.
     * @return SessionInfo with additional session information.
     */
    public abstract SessionInfo getSessionInfo();
}
