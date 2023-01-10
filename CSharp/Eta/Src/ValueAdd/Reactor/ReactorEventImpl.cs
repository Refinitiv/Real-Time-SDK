/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class ReactorEventImpl : ReactorEvent
    {
        internal enum ImplType
        {
            INIT = 0,

            // sent from Reactor to Worker
            CHANNEL_INIT = 1,
            // sent from Worker to Reactor
            CHANNEL_UP = 2,
            // sent from either Reactor or Worker
            CHANNEL_DOWN = 3,
            // sent from Worker to Reactor
            CHANNEL_READY = 4,
            // sent from Reactor to Worker
            CHANNEL_CLOSE = 5,
            // sent from Worker to Reactor
            CHANNEL_CLOSE_ACK = 6,
            // sent from Worker to Reactor
            FLUSH_DONE = 7,

            WARNING = 8,

            // sent from Reactor to Worker
            FD_CHANGE = 9,
            // sent from Reactor to Worker
            FLUSH = 10,
            // sent from Reactor and possibly from Worker (in the case of an error)
            SHUTDOWN = 11,
            // sent from Reactor to Worker
            START_DISPATCH_TIMER = 12,
            // sent from Worker to Reactor
            TUNNEL_STREAM_DISPATCH_TIMEOUT = 13,
            // sent from Reactor to itself to force a tunnel stream dispatch
            TUNNEL_STREAM_DISPATCH_NOW = 14,
            // sent from Reactor to itself to force a watchlist dispatch
            WATCHLIST_DISPATCH_NOW = 15,
            // sent from Reactor to Worker
            START_WATCHLIST_TIMER = 16,
            // sent from Worker to Reactor
            WATCHLIST_TIMEOUT = 17,

            // sent from Reactor to Woker
            TOKEN_MGNT = 18,

            // sent from Reactor to itself for dispatching to the application
            TOKEN_CREDENTIAL_RENEWAL = 19
        }

        internal ImplType EventImplType { get; set; }

        internal ReactorTokenSession? TokenSession {get; set; }

        public ReactorEventImpl()
        {
            Clear();
        }

        internal void Clear()
        {
            Type = ReactorEventType.REACTOR;
            EventImplType = ImplType.INIT;
            TokenSession = null;
        }
    }
}
