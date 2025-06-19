/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{

    /// <summary>
    /// The Reactor channel event callback is used to communicate ReactorChannel and
    /// connection state information to the application.
    /// </summary>
    public interface IReactorChannelEventCallback
    {

        /// <summary>
        /// A callback function that the <see cref="Reactor"/> will use to communicate
        /// <see cref="ReactorChannel"/> and connection state information to the
        /// application.
        /// </summary>
        ///
        /// <param name="evt">A ReactorChannelEvent containing event information. The
        ///            ReactorChannelEvent is valid only during callback</param>
        ///
        /// <returns>ReactorCallbackReturnCodes A callback return code that can
        ///         trigger specific Reactor behavior based on the outcome of the
        ///         callback function</returns>
        ///
        /// <seealso cref="ReactorChannelEvent"/>
        /// <seealso cref="ReactorCallbackReturnCode"/>
        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt);
    }
}
