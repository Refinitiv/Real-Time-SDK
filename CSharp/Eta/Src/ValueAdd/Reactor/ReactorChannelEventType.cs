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
    /// Type indicating the type of RsslReactorChannelEvent.
    /// </summary>
    /// <see cref="ReactorChannelEvent"/>
    public enum ReactorChannelEventType
    {
        /// <summary>
        /// Unknown event type.
        /// </summary>
        INIT = 0,

        /// <summary>
        /// Channel has successfully initialized and can be dispatched. If the application presented any messages for setting
        /// up the session in their 
        /// </summary>
        CHANNEL_UP,

        /// <summary>
        /// ReactorChannel has failed (e.g. the connection was lost or a ping timeout expired) and can no longer send or receive data.
        /// There is no connection recovery for this event.
        /// </summary>
        CHANNEL_DOWN,

        /// <summary>
        /// ReactorChannel has failed and connection recovery has started. This only occurs on client connections since there is
        /// no connection recovery for server connections. The application should should release any resources associated with the channel.
        /// </summary>
        CHANNEL_DOWN_RECONNECTING,

        /// <summary>
        /// Channel has sent and received all messages expected for setting up the session. Normal use (such as item requests)
        /// can now be done.
        /// </summary>
        CHANNEL_READY,

        /// <summary>
        /// An event has occurred that did not result in channel failure, but may require attention by the application.
        /// </summary>
        WARNING,

        /// <summary>
        /// The <c>Socket</c> representing this channel has changed. The new and old <c>Socket</c> can be found on the ReactorChannel.
        /// </summary>
        FD_CHANGE,

        /// <summary>
        /// Channel was opened by the application and can be used (occurs when watchlist is enabled and only appears
        /// in the channelOpenCallback).
        /// </summary>
        CHANNEL_OPENED,
    }
}
