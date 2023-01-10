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
    /// The default message callback is used to communicate message events to the application.
    /// </summary>
    public interface IDefaultMsgCallback
    {
        /// <summary>
        /// A callback function that the <see cref="Reactor"/> will use to communicate message events to the application.
        /// </summary>
        /// <param name="msgEvent">A <see cref="ReactorMsgEvent"/> containing event information. 
        /// The event is valid only during callback</param>
        /// <returns>A callback return code that can trigger specific Reactor behavior 
        /// based on the outcome of the callback function</returns>
        ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent);
    }
}
