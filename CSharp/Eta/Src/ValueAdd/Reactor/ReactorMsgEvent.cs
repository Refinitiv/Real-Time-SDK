/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// ReactorMsgEvent base class. Used by all other message event classes.
    /// </summary>
    public class ReactorMsgEvent : ReactorEvent
    {
        /// <summary>
        /// Gets <see cref="ITransportBuffer"/> associated with this message event.
        /// </summary>
        public ITransportBuffer? TransportBuffer { get; internal set; } = null;

        /// <summary>
        /// Gets <see cref="IMsg"/> associated with this message event.
        /// </summary>
        public IMsg? Msg { get; internal set; } = null;

        /// <summary>
        /// Gets <see cref="WatchlistStreamInfo"/> associated with this message event.
        /// </summary>
        /// <remarks>Only used when the Watchlist is enabled</remarks>
        public WatchlistStreamInfo StreamInfo { get; private set; } = new WatchlistStreamInfo();

        /// <summary>
        /// Clears to default values
        /// </summary>
        public virtual void Clear()
        {
            TransportBuffer = null;
            Msg = null;
            StreamInfo.Clear();
        }

        /// <summary>
        /// Returns this object back to the pool.
        /// </summary>
        public override void ReturnToPool()
        {
            TransportBuffer = null;
            Msg = null;

            base.ReturnToPool();
        }

        /// <summary>
        /// Gets the string representation of this object. 
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"{base.ToString()}, {(TransportBuffer != null ? "TransportBuffer present" : "TransportBuffer null")}, {(Msg != null ? "Msg present" : "Msg null")}";
        }
    }

}
