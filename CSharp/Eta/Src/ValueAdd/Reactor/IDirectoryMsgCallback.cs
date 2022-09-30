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
    /// The RDM dictionary message callback is used 
    /// to communicate dictionary message events to the application.
    /// </summary>
    public interface IDirectoryMsgCallback
    {
        /// <summary> 
        /// A callback function that the <see cref="Reactor"/> will use to communicate 
        /// dictionary message events to the application.
        /// </summary>
        /// <param name="directoryMsgEvent">A <see cref="ReactorMsgEvent"/> containing event information. 
        /// The ReactorMsgEvent is valid only during callback</param>
        /// <returns><see cref="ReactorCallbackReturnCode"/> value that can trigger 
        /// specific Reactor behavior on the outcome of the callback function.</returns>
        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent);
    }
}
