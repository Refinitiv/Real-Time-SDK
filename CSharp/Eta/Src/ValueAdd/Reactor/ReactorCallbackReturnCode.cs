/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Information about the state of the ReactorChannel connection as well as any
    /// messages for that channel are returned to the application via a series of
    /// callback functions. There are several values that can be returned from a
    /// callback function implementation. These can trigger specific Reactor behavior
    /// based on the outcome of the callback function. This class defines the
    /// callback return codes that applications can use.
    /// </summary>
    public enum ReactorCallbackReturnCode
    {
        /// <summary>
        /// Indicates that the callback function was successful and the message or
        /// event has been handled.
        /// </summary>
        SUCCESS = 0,

        /// <summary>
        /// Indicates that the message or event has failed to be handled. Returning
        /// this code from any callback function will cause the Reactor to shutdown.
        /// </summary>
        FAILURE = -1,

        /// <summary>
        /// Can be returned from any domain-specific callback (e.g., RDMLoginMsgCallback).
        /// This will cause the Reactor to invoke the DefaultMsgCallback for this message
        /// upon the domain-specific callbacks return.
        /// </summary>
        RAISE = -2
    }
}
