/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Manages StreamIDs for Watchlist
    /// </summary>
    internal class WlStreamIdManager
    {
        private readonly int MIN_STREAM_ID = 3;
        private int m_NextStreamId;
        private int m_NextProvStreamId;

        /// <summary>
        /// Id for Login stream
        /// </summary>
        internal readonly static int LOGIN_STREAM_ID = 1;
        /// <summary>
        /// Id for Directory stream
        /// </summary>
        internal readonly static int DIRECTORY_STREAM_ID = 2;

        /// <summary>
        /// The table of open streams by their ids
        /// </summary>
        internal IDictionary<int, WlStream> WlStreamsByIds { get; }

        /// <summary>
        /// The table of requests by their ids
        /// </summary>
        internal IDictionary<int, WlRequest> WlRequestsByIds { get; }

        /// <summary>
        /// Constructor for StreamID management component
        /// </summary>
        /// <param name="itemCountHint">the expected number of items to be requested</param>
        internal WlStreamIdManager(int itemCountHint)
        {
            m_NextStreamId = MIN_STREAM_ID - 1;
            m_NextProvStreamId = MIN_STREAM_ID - 1;
            WlStreamsByIds = new Dictionary<int, WlStream>(itemCountHint);
            WlRequestsByIds = new Dictionary<int, WlRequest>(itemCountHint);
        }

        /// <summary>
        /// Gets next free streamId
        /// </summary>
        /// <returns>free streamId</returns>
        internal int GetStreamID()
        {
            do
            {
                if (++m_NextStreamId == int.MaxValue) m_NextStreamId = MIN_STREAM_ID - 1;
            } while (WlStreamsByIds.ContainsKey(m_NextStreamId));

            return m_NextStreamId;
        }

        /// <summary>
        /// Gets next free streamId for provider-driven request
        /// </summary>
        /// <returns>free streamId</returns>
        internal int GetProviderStreamID()
        {
            do
            {
                if (++m_NextProvStreamId == int.MaxValue) m_NextProvStreamId = MIN_STREAM_ID - 1;
            } while (WlRequestsByIds.ContainsKey(-m_NextProvStreamId));
            return -m_NextProvStreamId;
        }

        /// <summary>
        /// Clears dictionaries and resets streamId incrementors
        /// </summary>
        internal void Clear()
        {
            WlStreamsByIds.Clear();
            WlRequestsByIds.Clear();
            m_NextStreamId = MIN_STREAM_ID - 1;
            m_NextProvStreamId = MIN_STREAM_ID - 1;
        }
    }
}
