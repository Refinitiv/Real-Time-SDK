/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.PerfTools.Common
{
    /// <summary>
    /// This is the source directory handler for the ETA consumer application. 
    /// It provides methods for sending the source directory request to a provider 
    /// and processing the response. Methods for setting the service name, 
    /// getting the service information, and closing a source directory stream are also provided.
    /// </summary>
    public class DirectoryHandler
    {
        private const int SRCDIR_STREAM_ID = 2;

        private const int FILTER_TO_REQUEST = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP;

        public const int MAX_MSG_SIZE = 1024;
        public const int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

        private Buffer m_ServiceName;     // requested service
        private Service m_Service;        // service cache for the requested service
        private State m_State;            // directory stream state

        private DirectoryRequest m_DirectoryRequest = new DirectoryRequest();
        private DirectoryRefresh m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryUpdate m_DirectoryUpdate = new DirectoryUpdate();
        private DirectoryClose m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus m_DirectoryStatus = new DirectoryStatus();
        private EncodeIterator m_EncIter = new EncodeIterator();

        /// <summary>
        /// Service name requested by the application
        /// </summary>
        public Buffer ServiceName { get => m_ServiceName; set { value.Copy(m_ServiceName); } }

        public DirectoryHandler()
        {
            m_ServiceName = new Buffer();
            m_Service = new Service();
            m_State = new State();
        }

        /// <summary>
        /// Sets the service name requested by the application.
        /// </summary>
        /// <param name="servicename">The service name requested by the application</param>
        public void SetServiceName(string servicename)
        {
            ServiceName.Data(servicename);
        }

        /// <summary>
        /// Checks if is requested service up.
        /// </summary>
        /// <returns>true if service requested by application is up, false if not.</returns>
        public bool IsRequestedServiceUp() => m_Service.HasState 
            && (!m_Service.State.HasAcceptingRequests || m_Service.State.AcceptingRequests == 1) 
            && m_Service.State.ServiceStateVal == 1;

        /// <summary>
        /// Service info.
        /// </summary>
        /// <returns>service info associated with the service name requested by application</returns>
        public Service ServiceInfo() => m_Service;

        /// <summary>
        /// Sends a source directory request to a channel. 
        /// This consists of getting a message buffer, encoding the source directory request, 
        /// and sending the source directory request to the server.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory request to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public ITransportBuffer? GetRequest(IChannel chnl, out Error error)
        {
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
            {
                return null;
            }

            //initialize directory state
            //this will be updated as refresh and status messages are received
            m_State.DataState(DataStates.NO_CHANGE);
            m_State.StreamState(StreamStates.UNSPECIFIED);

            //encode source directory request
            m_DirectoryRequest.Clear();
            m_DirectoryRequest.StreamId = SRCDIR_STREAM_ID;
            m_DirectoryRequest.Filter = FILTER_TO_REQUEST;
            m_DirectoryRequest.Streaming = true;
            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            CodecReturnCode ret = m_DirectoryRequest.Encode(m_EncIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Encode DirectoryRequest failed: code {ret.GetAsString()}.\n"
                };
                return null;
            }
            Console.WriteLine(m_DirectoryRequest.ToString());

            return msgBuf;
        }

        /// <summary>
        /// Processes a source directory response. This consists of looking at the msg class 
        /// and decoding message into corresponding RDM directory message. After decoding, 
        /// service status state (up or down) is updated from a refresh or update message for a requested service.
        /// </summary>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, out Error? error)
        {
            CodecReturnCode ret;
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    Console.WriteLine("Received Source Directory Refresh");
                    ret = m_DirectoryRefresh.Decode(dIter, msg);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"Error decoding directory refresh, code: {ret.GetAsString()}.",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }

                    Console.WriteLine(m_DirectoryRefresh.ToString());

                    m_State.DataState(m_DirectoryRefresh.State.DataState());
                    m_State.StreamState(m_DirectoryRefresh.State.StreamState());

                    ProcessServiceRefresh(m_DirectoryRefresh.ServiceList);
                    break;

                case MsgClasses.UPDATE:
                    Console.WriteLine("Received Source Directory Update");
                    ret = m_DirectoryUpdate.Decode(dIter, msg);

                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"Error decoding directory update: code {ret.GetAsString()}. ",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }

                    Console.WriteLine(m_DirectoryUpdate.ToString());
                    ProcessServiceUpdate(m_DirectoryUpdate.ServiceList);
                    break;

                case MsgClasses.STATUS:
                    ret = m_DirectoryStatus.Decode(dIter, msg);

                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"Error decoding directory status: code {ret.GetAsString()}\n",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    Console.WriteLine("Received Source Directory Status:");
                    Console.WriteLine(m_DirectoryStatus.ToString());

                    if (m_DirectoryStatus.HasState)
                    {
                        m_State.DataState(m_DirectoryStatus.State.DataState());
                        m_State.StreamState(m_DirectoryStatus.State.StreamState());
                    }
                    break;

                default:
                    error = new Error()
                    {
                        Text = "Received Unhandled Source Directory Msg Class: " + msg.MsgClass,
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private void ProcessServiceUpdate(List<Service> serviceList)
        {
            foreach (Service rdmService in serviceList)
            {
                if (rdmService.Action == MapEntryActions.DELETE && rdmService.ServiceId == m_Service.ServiceId)
                {
                    m_Service.Action = MapEntryActions.DELETE;
                }

                if (rdmService.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine($"Received serviceName: {rdmService.Info.ServiceName}");
                }

                // update service cache - assume cache is built with previous refresh message
                if (rdmService.ServiceId == m_Service.ServiceId)
                {
                    rdmService.Copy(m_Service);
                }
            }
        }

        private void ProcessServiceRefresh(List<Service> serviceList)
        {
            foreach (Service rdmService in serviceList)
            {
                if (rdmService.Action == MapEntryActions.DELETE && rdmService.ServiceId == m_Service.ServiceId)
                {
                    m_Service.Action = MapEntryActions.DELETE;
                }

                if (rdmService.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine($"Received serviceName: {rdmService.Info.ServiceName}");
                }
                // cache service requested by the application
                if (rdmService.Info.ServiceName.Equals(m_ServiceName))
                {
                    rdmService.Copy(m_Service);
                }
            }
        }

        /// <summary>
        /// Gets Close message for Source Directory stream.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public ITransportBuffer? GetCloseMsg(IChannel chnl, out Error? error)
        {
            error = null;

            // we only want to close a stream if it was not already closed (e.g.
            // rejected by provider, closed via refresh or status)
            if (m_State.IsFinal())
            {
                return null;
            }

            //get a buffer for the source directory close
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
            {
                return null;
            }

            // encode source directory close
            m_DirectoryClose.Clear();
            m_DirectoryClose.StreamId = SRCDIR_STREAM_ID;
            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            CodecReturnCode ret = m_DirectoryClose.Encode(m_EncIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encode SourceDirectory Close failed: code " + ret.GetAsString() + ".\n"
                };
                return null;
            }

            m_Service.Clear();
            return msgBuf;
        }

    }
}
