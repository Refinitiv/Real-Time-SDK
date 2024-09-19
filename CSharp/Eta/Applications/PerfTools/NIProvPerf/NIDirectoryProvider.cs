/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;

using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.Perftools.NIProvPerf
{
    /// <summary>
    /// The directory handler for the NIProvPerf. 
    /// Configures a single service and provides encoding and sending of a directory message.
    /// </summary>
    public class NIDirectoryProvider : DirectoryProvider
    {
        // Service
        public Service Service { get => m_Service; set => m_Service = value; }

        // Open limit
        public int OpenLimit { get => m_OpenLimit; set => m_OpenLimit = value; }

        // Service id 
        public int ServiceId { get => m_ServiceId; set => m_ServiceId = value; }

        // Service name
        public string? ServiceName { get => m_ServiceName; set { m_ServiceName = value; } }

        /// <summary>
        /// Initializes the directory refresh
        /// </summary>
        /// <param name="streamId">stream id of the refresh</param>
        internal void InitRefresh(int streamId)
        {
            m_DirectoryRefresh.Clear();

            m_DirectoryRefresh.ClearCache = true;
            m_DirectoryRefresh.Filter = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP;

            // streamId
            m_DirectoryRefresh.StreamId = streamId;

            m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            m_DirectoryRefresh.State.DataState(DataStates.OK);
            m_DirectoryRefresh.State.Code(StateCodes.NONE);
            m_DirectoryRefresh.State.Text().Data("Source Directory Refresh Completed");

            m_DirectoryRefresh.ServiceList.Add(m_Service);
        }
       
        /// <summary>
        /// Encodes the directory refresh
        /// </summary>
        /// <param name="channel">the current channel</param>
        /// <param name="streamId">stream id of refresh</param>
        /// <param name="error">detailed error information in case of failure</param>
        /// <returns>transport buffer with the encoded message</returns>
        public ITransportBuffer? EncodeRefresh(IChannel channel, int streamId, out Error error)
        {
            // get a buffer for the source directory response
            ITransportBuffer msgBuf = channel.GetBuffer(REFRESH_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return null;
            }

            // set buffer on encode iterator
            m_EncodeIter.Clear();
            var ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return null;
            }

            // initialize source directory refresh
            InitRefresh(streamId);

            // encode source directory refresh
            ret = m_DirectoryRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return null;
            }

            // return encoded source directory refresh
            return msgBuf;
        }
    }
}
