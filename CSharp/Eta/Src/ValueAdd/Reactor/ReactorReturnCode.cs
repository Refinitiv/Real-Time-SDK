/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Return codes associated with Reactor.
    /// </summary>
    public enum ReactorReturnCode
    {
        /// <summary>
        /// Indicates a success code. Used to inform the user of the success.
        /// </summary>
        SUCCESS = 0,

        /// <summary>
        /// A general failure has occurred. The <see cref="ReactorErrorInfo"/> contains
        /// more information about the specific error.
        /// </summary>
        FAILURE = -1,

        /// <summary>
        /// 
        /// </summary>
        WRITE_CALL_AGAIN = -2,

        /// <summary>
        /// /
        /// </summary>
        NO_BUFFERS = -3,

        /// <summary>
        /// Indicates that a parameter was out of range.
        /// </summary>
        PARAMETER_OUT_OF_RANGE = -4,

        /// <summary>
        /// Indicates that a parameter was invalid.
        /// </summary>
        PARAMETER_INVALID = -5,

        /// <summary>
        /// The interface is being improperly used.
        /// </summary>
        INVALID_USAGE = -6,

        /// <summary>
        /// An error was encountered during a channel operation.
        /// </summary>
        CHANNEL_ERROR = -7,

        /// <summary>
        /// The interface is attempting to write a message to the Reactor
        /// with an invalid encoding.
        /// </summary>
        INVALID_ENCODING = -8,

        /// <summary>
        /// Failure. Reactor is shutdown.
        /// </summary>
        SHUTDOWN = -10
    }
}
