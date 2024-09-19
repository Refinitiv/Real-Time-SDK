/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */


namespace LSEG.Eta.ValueAdd.Reactor
{

    /// <summary>The RDM login message callback is used to communicate login
    /// message events to the application.</summary>
    public interface IRDMLoginMsgCallback
    {

        /// <summary>A callback function that the {@link Reactor} will use to communicate
        /// login message events to the application.</summary>
        ///
        /// <param name="loginEvent">A ReactorMsgEvent containing event information. The
        ///            ReactorMsgEvent is valid only during callback</param>
        ///
        /// <returns>ReactorCallbackReturnCodes A callback return code that can trigger
        ///         specific Reactor behavior based on the outcome of the callback
        ///         function</returns>
        ///
        /// <seealso cref="RDMLoginMsgEvent"/>
        /// <seealso cref="ReactorCallbackReturnCode"/>
        ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent);
    }
}
