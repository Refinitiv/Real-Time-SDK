/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmInvalidUsageException is thrown when application violates usage of EMA interfaces.
/// </summary>
public sealed class OmmInvalidUsageException : OmmException
{
    /// <summary>
    /// Gets exception type
    /// </summary>
    public override ExceptionType Type { get => ExceptionType.OmmInvalidUsageException; }

    /// <summary>
    /// Gets the error code to describe the error case defined in the <see cref="ErrorCodes"/>.
    /// </summary>
    /// <value>an error code causing exception</value>
    public int ErrorCode { get; private set; } = ErrorCodes.NONE;

    /// <summary>
    /// Represents error codes for handling the exception.
    /// </summary>
    public static class ErrorCodes
    {
        /// <summary>
        /// No specific error code.
        /// </summary>
        public const int NONE = 0;

        /// <summary>
        /// General failure.
        /// </summary>
        public const int FAILURE = -1;

        /// <summary>
        /// This indicates that Write was unable to send all fragments with the current call and must continue fragmenting.
        /// </summary>
        public const int WRITE_CALL_AGAIN = -2;

        /// <summary>
        /// There are no buffers available from the buffer pool.
        /// </summary>
        public const int NO_BUFFERS = -3;

        /// <summary>
        /// Indicates that a parameter was out of range.
        /// </summary>
        public const int PARAMETER_OUT_OF_RANGE = -4;

        /// <summary>
        /// Indicates that a parameter was invalid.
        /// </summary>
        public const int PARAMETER_INVALID = -5;

        /// <summary>
        /// The interface is being improperly used.
        /// </summary>
        public const int INVALID_USAGE = -6;

        /// <summary>
        /// An error was encountered during a channel operation.
        /// </summary>
        public const int CHANNEL_ERROR = -7;

        /// <summary>
        /// The interface is attempting to write a message to the Reactor
        /// with an invalid encoding.
        /// </summary>
        public const int INVALID_ENCODING = -8;

        /// <summary>
        /// The interface is attempting to write a message to the TunnelStream,
        /// but the persistence file is full.
        /// </summary>
        public const int PERSISTENCE_FULL = -9;

        /// <summary>
        /// Indicates that the specified version is not supported
        /// </summary>
        public const int VERSION_NOT_SUPPORTED = -16;

        /// <summary>
        /// The buffer provided (or the remaining buffer space for message packing) does not have sufficient space to perform the operation.
        /// </summary>
        public const int BUFFER_TOO_SMALL = -21;

        /// <summary>
        /// An invalid argument was provided.
        /// </summary>
        public const int INVALID_ARGUMENT = -22;

        /// <summary>
        /// No encoder is available for the data type specified.
        /// </summary>
        public const int ENCODING_UNAVALIABLE = -23;

        /// <summary>
        /// The data type is unsupported, may indicate invalid containerType or primitiveType specified.
        /// </summary>
        public const int UNSUPPORTED_DATA_TYPE = -24;

        /// <summary>
        /// An encoder was used in an unexpected sequence.
        /// </summary>
        public const int UNEXPECTED_ENCODER_CALL = -25;

        /// <summary>
        /// Not enough data was provided.
        /// </summary>
        public const int INCOMPLETE_DATA = -26;

        /// <summary>
        /// A Database containing the Set Definition for encoding the desired set was not provided.
        /// </summary>
        public const int SET_DEF_NOT_PROVIDED = -27;

        /// <summary>
        /// Invalid data provided to function.
        /// </summary>
        public const int INVALID_DATA = -29;

        /// <summary>
        /// Set definition is not valid.
        /// </summary>
        public const int ILLEGAL_LOCAL_SET_DEF = -30;

        /// <summary>
        /// Maximum number of set definitions has been exceeded.
        /// </summary>
        public const int TOO_MANY_LOCAL_SET_DEFS = -31;

        /// <summary>
        /// A duplicate set definition has been received.
        /// </summary>
        public const int DUPLICATE_LOCAL_SET_DEFS = -32;

        /// <summary>
        /// Iterator is nested too deeply. There is a limit of 16 levels.
        /// </summary>
        public const int ITERATOR_OVERRUN = -33;

        /// <summary>
        /// A value being encoded into a set is outside of the valid range of the type
        /// given by that set.
        /// </summary>
        public const int VALUE_OUT_OF_RANGE = -34;

        /// <summary>
        /// A display string had multiple enumerated values that correspond to it.
        /// </summary>
        public const int DICT_DUPLICATE_ENUM_VALUE = -35;

        /// <summary>
        /// Multicast Transport Warning: An unrecoverable packet gap was detected and some
        /// content may have been lost.
        /// </summary>
        public const int PACKET_GAP_DETECTED = -61;

        /// <summary>
        /// Multicast Transport Warning: Application is consuming more slowly than data is
        /// being provided.  Gaps are likely.
        /// </summary>
        public const int SLOW_READER = -62;

        /// <summary>
        /// Multicast Transport Warning: Network congestion detected.  Gaps are likely.
        /// </summary>
        public const int CONGESTION_DETECTED = -63;

        /// <summary>
        /// Invalid user's operation.
        /// </summary>
        public const int INVALID_OPERATION = -4048;

        /// <summary>
        /// No active channel.
        /// </summary>
        public const int NO_ACTIVE_CHANNEL = -4049;

        /// <summary>
        /// Unsupported channel type.
        /// </summary>
        public const int UNSUPPORTED_CHANNEL_TYPE = -4050;

        /// <summary>
        /// Unsupported server type.
        /// </summary>
        public const int UNSUPPORTED_SERVER_TYPE = -4051;

        /// <summary>
        /// Login request timeout.
        /// </summary>
        public const int LOGIN_REQUEST_TIME_OUT = -4052;

        /// <summary>
        /// Login request rejected from connected peer.
        /// </summary>
        public const int LOGIN_REQUEST_REJECTED = -4053;

        /// <summary>
        /// Directory request timeout.
        /// </summary>
        public const int DIRECTORY_REQUEST_TIME_OUT = -4054;

        /// <summary>
        /// Dictionary request timeout.
        /// </summary>
        public const int DICTIONARY_REQUEST_TIME_OUT = -4055;

        /// <summary>
        /// Internal Error in EMA.
        /// </summary>
        public const int INTERNAL_ERROR = -4060;
    }

    internal OmmInvalidUsageException()
        : base()
    {
    }

    internal OmmInvalidUsageException(string message)
        : base(message)
    {
    }

    internal OmmInvalidUsageException(string message, int errorCode)
        : base(message)
    {
        ErrorCode = errorCode;
    }

    /// <summary>
    /// Returns a string representation of the class instance.
    /// </summary>
    ///
    /// <returns>string representation of the class object.</returns>
    public override string ToString()
    {
        return $"Exception Type='OmmInvalidUsageException', Text='{Message}', Error Code='{ErrorCode}'";
    }
}
