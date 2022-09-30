/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.PerfTools.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.PerfTools.ProvPerf
{
    public class IProvDirectoryProvider : DirectoryProvider
    {
        // directory request received for interactive provider
        public DirectoryRequest DirectoryRequest { get; set; }

        public Service Service { get => m_Service; set => m_Service = value; }

        // Open limit
        public int OpenLimit { get => m_OpenLimit; set => m_OpenLimit = value; }

        // Service id 
        public int ServiceId { get => m_ServiceId; set => m_ServiceId = value; }

        // Qos
        public Qos Qos { get => m_Qos; set => m_Qos = value; }

        // Service name
        public string? ServiceName { get => m_ServiceName; set { m_ServiceName = value; } }

        public IProvDirectoryProvider() : base()
        {          
            DirectoryRequest = new DirectoryRequest();
        }

        /// <summary>
        /// Processes a directory request. 
        /// This consists of decoding directory request message and encoding/sending directory refresh
        /// </summary>
        /// <param name="channelHandler">handler for the channel</param>
        /// <param name="clientChannelInfo">channel information</param>
        /// <param name="msg">partially decoded message that was received</param>
        /// <param name="dIter"><see cref="DecodeIterator"/> instance</param>
        /// <param name="error"><see cref="Error"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value that indicates the status of the operation</returns>
        public TransportReturnCode ProcessMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Msg msg, DecodeIterator dIter, out Error? error)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    Console.WriteLine("Received Source Directory Request");
                    CodecReturnCode ret = DirectoryRequest.Decode(dIter, msg);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"DirectoryRequest.Decode() failed with return code: {ret.GetAsString()}"
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    // send source directory response
                    return SendRefresh(channelHandler, clientChannelInfo, out error);
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Directory Close for streamId {msg.StreamId}");
                    break;
                default:
                    error = new Error()
                    {
                        Text = $"Received unhandled Source Directory msg type: {msg.MsgClass}"
                    };
                    return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode SendRefresh(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, out Error error)
        {
            IChannel channel = clientChannelInfo.Channel!;

            // get a buffer for the source directory refresh 
            ITransportBuffer msgBuf = channel.GetBuffer(REFRESH_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            // encode source directory refresh
            m_DirectoryRefresh.Clear();
            m_DirectoryRefresh.StreamId = DirectoryRequest.StreamId;

            // clear cache
            m_DirectoryRefresh.ClearCache = true;
            m_DirectoryRefresh.Solicited = true;

            // state information for refresh message
            m_DirectoryRefresh.State.Clear();
            m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            m_DirectoryRefresh.State.DataState(DataStates.OK);
            m_DirectoryRefresh.State.Code(StateCodes.NONE);
            m_DirectoryRefresh.State.Text().Data("Source Directory Refresh Completed");

            // attribInfo information for response message
            m_DirectoryRefresh.Filter = DirectoryRequest.Filter;

            // ServiceId
            if (DirectoryRequest.HasServiceId)
            {
                m_DirectoryRefresh.HasServiceId = true;
                m_DirectoryRefresh.ServiceId = DirectoryRequest.ServiceId;
                if (DirectoryRequest.ServiceId == m_Service.ServiceId)
                {
                    m_DirectoryRefresh.ServiceList.Add(m_Service);
                }
            }
            else
            {
                m_DirectoryRefresh.ServiceList.Add(m_Service);
            }

            // encode directory refresh
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }
            ret = m_DirectoryRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"DirectoryRefresh.Encode() failed with return code: {ret.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            // send source directory refresh
            return channelHandler.WriteChannel(clientChannelInfo, msgBuf, 0, out error);
        }
    }
}
