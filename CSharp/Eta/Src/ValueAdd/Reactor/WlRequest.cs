/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// This is base user request information.
    /// </summary>
    internal class WlRequest : VaNode
    {
        public enum State
        {
            NONE,
            RETURN_TO_POOL,
            PENDING_REQUEST,            // Waiting to send a request message via WlStream
            PENDING_REFRESH,            // Waiting to receive a refresh message
            PENDING_COMPLETE_REFRESH,   // Waiting to receive the complete refresh message for multi-part
            OPEN 						// Request has been completed	
        }

        /// <summary>
        /// Gets or sets the state of user request.
        /// </summary>
        public State ReqState { get; set; }

        /// <summary>
        /// Gets the request message.
        /// </summary>
        public IRequestMsg RequestMsg { get; private set; } = new Msg();

        /// <summary>
        /// Indicates whether the request knows its service Id.
        /// </summary>
        public bool HasServiceId { get; private set; }

        private uint m_ServiceId;

        /// <summary>
        /// Gets or sets service ID fo the request.
        /// </summary>
        public uint ServiceId
        {
            get => m_ServiceId;

            set
            {
                m_ServiceId = value;
                HasServiceId = true;
            }
        }

        /// <summary>
        /// Gets or sets handler associated with request.
        /// </summary>
        public IWlHandler? Handler { get; set; }

        /// <summary>
        /// Gets stream information for the stream.
        /// </summary>
        public WatchlistStreamInfo WatchlistStreamInfo { get; private set; } = new ();

        /// <summary>
        /// Gets or sets the stream associated with request. 
        /// </summary>
        public WlStream? WatchlistStream { get; set; }

        /// <summary>
        /// Gets or sets user stream Id.
        /// </summary>
        public int UserStreamId { get; set; }

        /// <summary>
        /// Unsets the service Id
        /// </summary>
        public void UnsetServiceId()
        {
            HasServiceId = false;
            m_ServiceId = 0;
        }

        public WlRequest()
        {
            ReqState = State.NONE;
            RequestMsg.MsgClass = MsgClasses.REQUEST;
            HasServiceId = false;
            m_ServiceId = 0;
            Handler = null;
            UserStreamId = 0;
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public virtual void Clear()
        {
            ReqState = State.NONE;
            HasServiceId = false;
            m_ServiceId = 0;
            WatchlistStreamInfo.Clear();
            Handler = null;
            UserStreamId = 0;
        }
    }
}
