/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Watchlist options for enabling the watchlist feature on <see cref="ConsumerRole"/>.
    /// </summary>
    sealed public class ConsumerWatchlistOptions
    {
        internal ConsumerWatchlistOptions()
        {
            Clear();
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            EnableWatchlist = false;
            ChannelOpenEventCallback = null;
            ItemCountHint = 0;
            ObeyOpenWindow = true;
            MaxOutstandingPosts = 100_000;
            PostAckTimeout = 15_000;
            RequestTimeout = 15_000;
        }

        /// <summary>
        /// Gets or sets whether the watchlist feature is enabled.
        /// </summary>
        public bool EnableWatchlist { get; set; }

        /// <summary>
        /// Gets or sets a callback function that is provided when a channel is first opened 
        /// by <see cref="Reactor.Connect(ReactorConnectOptions, ReactorRole, out ReactorErrorInfo?)"/>.
        /// </summary>
        /// <remarks>This is only allowed when the watchlist feature is enabled and this is optional property.</remarks>
        public IReactorChannelEventCallback? ChannelOpenEventCallback { get; set; }

        /// <summary>
        /// Gets or sets the number of items the application expects to request.
        /// </summary>
        public uint ItemCountHint { get; set; } 

        /// <summary>
        /// Gets or sets whether item requests obey the OpenWindow provided by a service.
        /// </summary>
        public bool ObeyOpenWindow { get; set; }

        /// <summary>
        /// Gets or sets the maximum number of on-stream post acknowledgements that may be outstanding for the channel.
        /// </summary>
        public uint MaxOutstandingPosts { get; set; }

        /// <summary>
        /// Gets or sets time a stream will wait for acknowledgement of an on-stream post, in milliseconds. 
        /// </summary>
        public uint PostAckTimeout { get; set; }

        /// <summary>
        /// Gets or sets time a requested stream will wait for a response from the provider, in milliseconds. 
        /// </summary>
        public uint RequestTimeout { get; set; }

        /// <summary>
        /// Performs a deep copy from a specified ConsumerWatchlistOptions into this ConsumerWatchlistOptions.
        /// </summary>
        /// <param name="watchlistOptions">The watchlist options to copy from</param>
        internal void Copy(ConsumerWatchlistOptions watchlistOptions)
        {
            EnableWatchlist = watchlistOptions.EnableWatchlist;
            ChannelOpenEventCallback = watchlistOptions.ChannelOpenEventCallback;
            ItemCountHint = watchlistOptions.ItemCountHint;
            ObeyOpenWindow = watchlistOptions.ObeyOpenWindow;
            MaxOutstandingPosts = watchlistOptions.MaxOutstandingPosts;
            PostAckTimeout = watchlistOptions.PostAckTimeout;
            RequestTimeout = watchlistOptions.RequestTimeout;
        }
    }
}
