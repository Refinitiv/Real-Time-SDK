/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Example.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Concurrent;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// Used by the ChannelHandler object. Maintains information about a channel,
    /// such as ping time information and whether flushing is being done.
    /// </summary>
    public class ClientChannelInfo
    {
        /// <summary>
        /// Gets an unique ID for this object.
        /// </summary>
        public Guid ID { get; private set; } = Guid.NewGuid();

        /// <summary>
        /// Gets or sets the ETA Channel associated with this info.
        /// </summary>
        public IChannel? Channel { get; set; }

        /// <summary>
        /// Gets or sets a reference to user-specified data associated with this channel.
        /// </summary>
        public object? UserSpec { get; set; }

        /// <summary>
        /// Gets or sets whether this channel needs to have data flushed.
        /// </summary>
        public bool NeedFlush { get; set; }

        /// <summary>
        /// Gets or sets whether this channel has additional data to read.
        /// </summary>
        public bool NeedRead { get; set; }

        /// <summary>
        /// Gets or sets whether a ping or messages have been received since the last ping check.
        /// </summary>
        public bool ReceivedMsg { get; set; }

        /// <summary>
        /// Gets or sets whether a ping or messages have been sent since the last ping check.
        /// </summary>
        public bool SentMsg { get; set; }

        /// <summary>
        /// Gets or sets whether ping handling is done for this channel.
        /// </summary>
        public bool CheckPings { get; set; }

        /// <summary>
        /// Gets or sets time before which this channel should receive a ping.
        /// </summary>
        public long NextReceivePingTime { get; set; }

        /// <summary>
        /// Gets or sets time before which a ping should be sent for this channel.
        /// </summary>
        public long NextSendPingTime { get; set; }

        /// <summary>
        /// Gets or sets a reference back to the list this channel is an element of.
        /// </summary>
        public volatile ConcurrentDictionary<Guid, ClientChannelInfo>? ParentQueue;

        /// <summary>
        /// Use the VA Reactor instead of the ETA Channel for sending and receiving.
        /// </summary>
        public ReactorChannel? ReactorChannel { get; set; }

        /// <summary>
        /// Gets or set a <see cref="SelectElement"/> associated with this channel if any.
        /// </summary>
        public SelectElement? SelectElement { get; set; }
    }
}
