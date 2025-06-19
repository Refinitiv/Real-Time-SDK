/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// IOmmConsumerClient interface provides callback interfaces to pass received messages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Application needs to implement an application client class inheriting from IOmmConsumerClient.</b>
    /// In its own class, application needs to override callback methods it desires to use for item processing.
    /// </para>
    /// <para>
    /// <br>Application may chose to implement specific message callbacks (e.g. <see cref="OnRefreshMsg(RefreshMsg, IOmmConsumerEvent)"/>)</br>
    /// or a general callback <see cref="OnAllMsg(Msg, IOmmConsumerEvent)"/>.
    /// </para>
    /// <para>Thread safety of all IOmmConsumerClient methods depends on the user's implementation.</para>
    /// <br>The following code snippet shows basic usage of IOmmConsumerClient class to print</br>
    /// received refresh, update, and status messages to screen.
    /// <code>
    /// class AppClient : IOmmConsumerClient
    /// {
    ///     public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    ///     {
    ///         Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
    ///         Console.WriteLine(refreshMsg);
    ///     }
    ///
    ///     public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    ///     {
    ///         Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
    ///         Console.WriteLine(updateMsg);
    ///     }
    ///
    ///     public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    ///     {
    ///         Console.WriteLine($"Handle: {consumerEvent.Handle} Closure: {consumerEvent.Closure}");
    ///         Console.WriteLine(statusMsg);
    ///     }
    /// }
    /// </code>
    /// </remarks>
    /// <seealso cref="OmmConsumer"/>
    /// <seealso cref="Msg"/>
    /// <seealso cref="AckMsg"/>
    /// <seealso cref="GenericMsg"/>
    /// <seealso cref="PostMsg"/>
    /// <seealso cref="RefreshMsg"/>
    /// <seealso cref="StatusMsg"/>
    /// <seealso cref="UpdateMsg"/>
    public interface IOmmConsumerClient
    {
        /// <summary>
        /// This callback is invoked upon receiving a refresh message.
        /// Refresh message may be a start, interim or final part.
        /// </summary>
        /// <param name="refreshMsg">received <see cref="RefreshMsg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving an update message.
        /// Update messages may be interlaced within a multiple part refresh message sequence.
        /// </summary>
        /// <param name="updateMsg">received <see cref="UpdateMsg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving a status message.
        /// </summary>
        /// <param name="statusMsg">received <see cref="StatusMsg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any generic message.
        /// </summary>
        /// <param name="genericMsg">received <see cref="GenericMsg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any ack message.
        /// </summary>
        /// <param name="ackMsg">received <see cref="AckMsg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) { }

        /// <summary>
        /// This callback is invoked upon receiving any message.
        /// </summary>
        /// <param name="msg">received <see cref="Msg"/></param>
        /// <param name="consumerEvent">identifies open item for which this message is received</param>
        void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent) {}
    }
}
