/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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

        /* Link for WlItemStream queue */
        internal WlRequest? m_next, m_prev;

        internal static readonly WlRequestLink WL_REQUEST_LINK = new();

        internal GCHandle m_handle;
        internal GCHandle m_reqMsgHandle;
        internal GCHandle m_watchlistStreamInfoHandle;

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
        /// Gets stream information for the stream.
        /// </summary>
        public WatchlistStreamInfo WatchlistStreamInfo { get; private set; } = new ();

        public int WatchlistStreamId { get; set; }

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
            UserStreamId = 0;
            m_reqMsgHandle = GCHandle.Alloc(RequestMsg, GCHandleType.Normal);
            m_watchlistStreamInfoHandle = GCHandle.Alloc(WatchlistStreamInfo, GCHandleType.Normal);
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual void Clear()
        {
            ReqState = State.NONE;
            HasServiceId = false;
            m_ServiceId = 0;
            WatchlistStreamInfo.Clear();
            UserStreamId = 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void ReturnToPool()
        {
            Clear();
            base.ReturnToPool();
        }

        public void FreeWlRequest()
        {
            m_reqMsgHandle.Free();
            m_watchlistStreamInfoHandle.Free();
            m_handle.Free();
            WatchlistStream = null;
        }
    }


    internal class WlRequestLink : VaDoubleLinkList<WlRequest>.ILink<WlRequest>
    {
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlRequest? GetPrev(WlRequest thisPrev) { return thisPrev.m_prev; }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void SetPrev(WlRequest? thisPrev, WlRequest? thatPrev) { thisPrev!.m_prev = thatPrev; }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlRequest? GetNext(WlRequest thisNext) { return thisNext.m_next; }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void SetNext(WlRequest? thisNext, WlRequest? thatNext) { thisNext!.m_next = thatNext; }
    }
}
