/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the source directory handler for the ETA consumer application. 
    /// It provides methods for sending the source directory request 
    /// to a provider and processing the response. Methods for setting the service name, 
    /// getting the service information, and closing a source directory stream are also provided.
    /// </summary>
    public class DirectoryHandler
    {
        private const int SRCDIR_STREAM_ID = 2;

        private const int FILTER_TO_REQUEST = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP;

        public const int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

        private Buffer _serviceName;    // requested service
        private Service _service;        // service cache for the requested service
        private State _state;            // directory stream state

        private DirectoryRequest directoryRequest = new DirectoryRequest();
        private DirectoryRefresh directoryRefresh = new DirectoryRefresh();
        private DirectoryUpdate directoryUpdate = new DirectoryUpdate();
        private DirectoryClose directoryClose = new DirectoryClose();
        private DirectoryStatus directoryStatus = new DirectoryStatus();
        private EncodeIterator encIter = new EncodeIterator();

        /// <summary>
        /// Service name requested by the application
        /// </summary>
        public Buffer ServiceName { get => _serviceName; set { _serviceName = value; } }

        public DirectoryHandler()
        {
            _serviceName = new Buffer();
            _service = new Service();
            _state = new State();
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
        public bool IsRequestedServiceUp()
        {
            return _service.HasState && (!_service.State.HasAcceptingRequests || _service.State.AcceptingRequests == 1) && _service.State.ServiceStateVal == 1;
        }

        /// <summary>
        /// Service info.
        /// </summary>
        /// <returns>service info associated with the service name requested by application</returns>
        public Service ServiceInfo()
        {
            return _service;
        }
        
        /// <summary>
        /// Sends a source directory request to a channel. 
        /// This consists of getting a message buffer, encoding the source directory request, 
        /// and sending the source directory request to the server.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory request to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode SendRequest(ChannelSession chnl, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }
                
            //initialize directory state
            //this will be updated as refresh and status messages are received
            _state.DataState(DataStates.NO_CHANGE);
            _state.StreamState(StreamStates.UNSPECIFIED);

            //encode source directory request
            directoryRequest.Clear();
            directoryRequest.StreamId = SRCDIR_STREAM_ID;
            directoryRequest.Filter = FILTER_TO_REQUEST;
            directoryRequest.Streaming = true;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            CodecReturnCode ret = directoryRequest.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Encode DirectoryRequest failed, return code {ret.GetAsString()}\n"
                };
                return TransportReturnCode.FAILURE;
            }
            Console.WriteLine(directoryRequest.ToString());

            //send source directory request
            TransportReturnCode wRet = chnl.Write(msgBuf, out error);
            if (wRet != TransportReturnCode.SUCCESS)
            {
                return wRet;
            }

            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Processes a source directory response. This consists of looking at the msg class 
        /// and decoding message into corresponding RDM directory message. After decoding, 
        /// service status state (up or down) is updated from a refresh or update message for a requested service.
        /// </summary>
        /// <param name="chnl">The channel that received the response</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    Console.WriteLine("Received Source Directory Refresh");
                    ret = directoryRefresh.Decode(dIter, msg);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = "Error decoding directory refresh, code: " + ret + ".",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }

                    Console.WriteLine(directoryRefresh.ToString());

                    _state.DataState(directoryRefresh.State.DataState());
                    _state.StreamState(directoryRefresh.State.StreamState());

                    ProcessServiceRefresh(directoryRefresh.ServiceList, out error);
                    if (_service.Action == MapEntryActions.DELETE)
                    {
                        error = new Error()
                        {
                            Text = "Process Response failed: directory service is deleted",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    break;

                case MsgClasses.UPDATE:
                    Console.WriteLine("Received Source Directory Update");
                    ret = directoryUpdate.Decode(dIter, msg);

                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = "Error decoding directory update: code " + ret + ". ",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }

                    Console.WriteLine(directoryUpdate.ToString());
                    ProcessServiceUpdate(directoryUpdate.ServiceList, out error);
                    if (_service.Action == MapEntryActions.DELETE)
                    {
                        error = new Error()
                        {
                            Text = "ProcessResponse() Failed: directory service is deleted",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    break;

                case MsgClasses.STATUS:
                    ret = directoryStatus.Decode(dIter, msg);

                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = "Error decoding directory status: code " + ret + ".\n",
                            ErrorId = TransportReturnCode.FAILURE
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    Console.WriteLine("Received Source Directory Status:");
                    Console.WriteLine(directoryStatus.ToString());

                    if (directoryStatus.HasState)
                    {
                        _state.DataState(directoryStatus.State.DataState());
                        _state.StreamState(directoryStatus.State.StreamState());
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

        private void ProcessServiceUpdate(List<Service> serviceList, out Error? error)
        {
            error = null;
            foreach (Service rdmService in serviceList)
            {
                if (rdmService.Action == MapEntryActions.DELETE && rdmService.ServiceId == _service.ServiceId)
                {
                    _service.Action = MapEntryActions.DELETE;
                }

                if (rdmService.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine("Received serviceName: " + rdmService.Info.ServiceName);
                }

                // update service cache - assume cache is built with previous refresh message
                if (rdmService.ServiceId == _service.ServiceId)
                {
                    rdmService.Copy(_service);
                }
            }
        }

        private void ProcessServiceRefresh(List<Service> serviceList, out Error? error)
        {
            error = null;
            foreach (Service rdmService in serviceList)
            {
                if (rdmService.Action == MapEntryActions.DELETE && rdmService.ServiceId == _service.ServiceId)
                {
                    _service.Action = MapEntryActions.DELETE;
                }

                if (rdmService.Info.ServiceName.ToString() != null)
                {
                    Console.WriteLine("Received serviceName: " + rdmService.Info.ServiceName);
                }
                // cache service requested by the application
                if (rdmService.Info.ServiceName.Equals(_serviceName))
                {
                    rdmService.Copy(_service);
                }
            }
        }
        
        /// <summary>
        /// Closes the source directory stream.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode CloseStream(ChannelSession chnl, out Error? error)
        {
            error = null;
             // we only want to close a stream if it was not already closed (e.g.
             // rejected by provider, closed via refresh or status)
            if (_state.IsFinal())
                return TransportReturnCode.SUCCESS;

            //get a buffer for the source directory close
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            // encode source directory close
            directoryClose.Clear();
            directoryClose.StreamId = SRCDIR_STREAM_ID;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            CodecReturnCode ret = directoryClose.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encode SourceDirectory Close failed: code " + ret + ".\n"
                };
                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode wRet = chnl.Write(msgBuf, out error);
            if (wRet != TransportReturnCode.SUCCESS)
            {
                return wRet;
            }

            _service.Clear(); //stream is closed, cached service information is invalid.

            return TransportReturnCode.SUCCESS;
        }
       
    }
}
