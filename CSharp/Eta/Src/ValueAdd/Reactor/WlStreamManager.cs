/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using System.Runtime.InteropServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlStreamManager
    {
        /// <summary>
        /// Id for Login stream
        /// </summary>
        internal const int LOGIN_STREAM_ID = 1;
        /// <summary>
        /// Id for Directory stream
        /// </summary>
        internal const int DIRECTORY_STREAM_ID = 2;

        internal const int GROW_SIZE = 10_000;
        internal const int MIN_STREAM_ID = 3;
        private int m_NextProvStreamId;

        internal WlItemStream[] StreamsByStreamIds;
        private VaPool m_wlStreamPool = new VaPool(false);

        internal WlStream LoginStream;
        internal WlStream DirectoryStream;


        /// <summary>
        /// The table of requests by their ids
        /// </summary>
        internal IDictionary<int, WlRequest> WlRequestsByIds { get; }

#pragma warning disable CS8618
        public WlStreamManager(int count)
        {
            InitWlStreamPool(count + 1000);
            WlRequestsByIds = new Dictionary<int, WlRequest>((int)(1.5 * count));
            LoginStream = new WlStream();
            DirectoryStream = new WlStream();
            LoginStream.StreamId = LOGIN_STREAM_ID;
            DirectoryStream.StreamId = DIRECTORY_STREAM_ID;
            LoginStream.StreamType = WlStreamType.LOGIN;
            DirectoryStream.StreamType = WlStreamType.DIRECTORY;
            LoginStream.m_streamHandle = GCHandle.Alloc(LoginStream);
            DirectoryStream.m_streamHandle = GCHandle.Alloc(DirectoryStream);
        }
#pragma warning restore CS8618

        public WlItemStream? GetItemStream()
        {
            var stream = (WlItemStream?)m_wlStreamPool.Poll();
            if (stream == null)
            {
                if (GrowItemStreamPool(StreamsByStreamIds.Length + GROW_SIZE)) stream = (WlItemStream?)m_wlStreamPool.Poll();
                else return null;
            }
            return stream;
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

        internal void Clear()
        {
            WlRequestsByIds.Clear();
            for (int i = 0; i < StreamsByStreamIds.Length; i++)
            {
                StreamsByStreamIds[i].ReturnToPool();
            }
            m_NextProvStreamId = MIN_STREAM_ID - 1;
        }

        private void InitWlStreamPool(int size)
        {
            StreamsByStreamIds = new WlItemStream[size];
            for (int i = 0; i < size; i++)
            {
                WlItemStream wlItemStream = new WlItemStream();
                wlItemStream.m_streamHandle = GCHandle.Alloc(wlItemStream, GCHandleType.Normal);
                m_wlStreamPool.Add(wlItemStream);
                StreamsByStreamIds[i] = wlItemStream;
                wlItemStream.StreamId = i + MIN_STREAM_ID;
            }
        }

        private bool GrowItemStreamPool(int newSize)
        {
            if (StreamsByStreamIds.Length == int.MaxValue) // very unlikely to happen 
            {
                return false;
            }
            
            int resSize = newSize < int.MaxValue ? newSize : int.MaxValue;
            
            WlItemStream[] tmpStreamArray;
            try
            {
                tmpStreamArray = new WlItemStream[resSize];
                Array.Copy(StreamsByStreamIds, tmpStreamArray, StreamsByStreamIds.Length);
                for (int i = 0; i < newSize - StreamsByStreamIds.Length; i++)
                {
                    var stream = new WlItemStream();
                    stream.m_streamHandle = GCHandle.Alloc(stream, GCHandleType.Normal);
                    stream.StreamId = i + StreamsByStreamIds.Length + MIN_STREAM_ID;
                    stream.StreamType = WlStreamType.ITEM;
                    m_wlStreamPool.Add(stream);
                    tmpStreamArray[i + StreamsByStreamIds.Length] = stream;
                }
            }
            catch
            {
                return false; //unable to create more item streams
            }
            
            StreamsByStreamIds = tmpStreamArray;
            return true;
        }

        internal void Free()
        {
            for (int i = 0; i < StreamsByStreamIds.Length; i++)
            {
                StreamsByStreamIds[i].FreeWlItemStream();
            }

            LoginStream.FreeWlStream();
            DirectoryStream.FreeWlStream();

        }
    }
}
