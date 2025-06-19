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
    /// The Reactor OAuth credential event callback is used to communicate <see cref="ReactorOAuthCredentialRenewal"/> 
    /// in order to get sensitive information from the application.
    /// </summary>
    public interface IReactorOAuthCredentialEventCallback
    {
        /// <summary>
        /// A callback function that the <see cref="Reactor"/> will use to get sensitive information from the application
        /// The ReactorSubmitOAuthCredentialRenewal() method is used to submit sensitive information.
        /// </summary>
        /// <param name="reactorOAuthCredentialEvent">A ReactorOAuthCredentialEvent containing event information.
        /// The ReactorOAuthCredentialEvent is valid only during callback.</param>
        /// <returns>A callback return code that can trigger specific Reactor behavior based on the outcome of the
        /// callback function</returns>
        /// <seealso cref="ReactorOAuthCredentialEvent"/>
        /// <seealso cref="ReactorCallbackReturnCode"/>
        public ReactorCallbackReturnCode ReactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent);
    }
}
