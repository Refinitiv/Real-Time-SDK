/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// IOmmProviderClient interface provides callback interfaces to pass received messages.
    /// </summary>
    /// <remarks>
    /// Application may need to implement an application client class inheriting from IOmmProviderClient.
    /// In its own class, application needs to override callback methods it desires to use for item processing.
    /// Default empty callback methods are implemented by IOmmProviderClient interface.
    /// 
    /// <para>
    /// Application may chose to implement specific callbacks (e.g., <see cref="OnRefreshMsg(RefreshMsg, IOmmProviderEvent)"/>) 
    /// or a general callback <see cref="OnAllMsg(Msg, IOmmProviderEvent)"/>.
    /// </para>
    /// <para>
    /// Thread safety of all the methods in this class depends on the user's implementation.
    /// </para>
    /// <para>
    /// The following code snippet shows basic usage of OmmProviderClient class to print recevied messages to screen.<br/>
    /// <code>
    /// class AppClient : IOmmProviderClient
    /// {
    ///     public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    ///     {
    ///         Console.WriteLine($"Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
    ///         Console.WriteLine(refreshMsg);
    ///     }
    ///     
    ///     public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    ///     {
    ///         Console.WriteLine($"Handle: {providerEvent.Handle} Closure: {providerEvent.Closure}");
    ///         Console.WriteLine(statusMsg);
    ///     }
    /// }
    /// </code>
    /// </para>
    /// </remarks>
    public interface IOmmProviderClient
    {
        /// <summary>
        /// This callback is invoked upon receiving a refresh message.
        /// Refresh message may be a start, interim or final part.
        /// </summary>
        /// <param name="refreshMsg">received <see cref="RefreshMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving a status message.
        /// </summary>
        /// <param name="statusMsg">received <see cref="StatusMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any generic message.
        /// </summary>
        /// <param name="genericMsg">received <see cref="GenericMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any post message.
        /// </summary>
        /// <param name="postMsg">received <see cref="PostMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving an initial item request message.
        /// </summary>
        /// <param name="reqMsg">received <see cref="RequestMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving a reissue request.
        /// </summary>
        /// <param name="reqMsg">received <see cref="RequestMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnReissue(RequestMsg reqMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving a close request message.
        /// </summary>
        /// <param name="reqMsg">received <see cref="RequestMsg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any message.
        /// </summary>
        /// <param name="msg">received <see cref="Msg"/></param>
        /// <param name="providerEvent">identifies open item for which this message is received</param>
        void OnAllMsg(Msg msg, IOmmProviderEvent providerEvent) { }
    }
}
