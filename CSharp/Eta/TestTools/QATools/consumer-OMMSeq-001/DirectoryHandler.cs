/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
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
//APIQA
using LSEG.Eta.Rdm;
//END APIQA

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

        //APIQA
        private long _initialDirFilter = 0;
        private long _reissueDirFilter = 0;
        private bool _sendGenericMsg = false;

        //END APIQA

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

        //APIQA
        public void SetDirectoryFilter(long initialDirFilter, long reissueDirFilter) 
        {
            _initialDirFilter = initialDirFilter;
            _reissueDirFilter = reissueDirFilter;
        }
        //END APIQA

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
        // APIQA
        public TransportReturnCode SendRequest(ChannelSession chnl, out Error? error, bool sendGenericMsg = false)
        // END APIQA
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            //APIQA
            _sendGenericMsg = sendGenericMsg;
            //END APIQA

            //initialize directory state
            //this will be updated as refresh and status messages are received
            _state.DataState(DataStates.NO_CHANGE);
            _state.StreamState(StreamStates.UNSPECIFIED);

            //encode source directory request
            directoryRequest.Clear();
            directoryRequest.StreamId = SRCDIR_STREAM_ID;
            //APIQA
            if (_initialDirFilter == 0)
                directoryRequest.Filter = FILTER_TO_REQUEST;
            else
                directoryRequest.Filter = _initialDirFilter;
            //END APIQA
            directoryRequest.Streaming = true;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            CodecReturnCode ret = directoryRequest.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encode DirectoryRequest failed: code " + ret + ".\n"
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


        //APIQA
        private TransportReturnCode SendGenericMsgOnDirectory(IChannel chnl, out Error ? error)
        {
            IGenericMsg genMsg = (IGenericMsg)new Msg();
            Map map = new Map();
            MapEntry mEntry = new MapEntry();
            ElementList elementList = new ElementList();
            ElementEntry elementEntry = new ElementEntry();
            EncodeIterator encodeIter = new EncodeIterator();
            ITransportBuffer? msgBuf;
            TransportReturnCode retChnl;

            genMsg.MsgClass = MsgClasses.GENERIC;
            genMsg.DomainType = (int)DomainType.SOURCE;
            genMsg.ContainerType = Codec.DataTypes.MAP;
            genMsg.MsgKey.Name.Data("ConsumerConnectionStatus");
            genMsg.MsgKey.Flags = MsgKeyFlags.HAS_NAME;
            genMsg.Flags = GenericMsgFlags.HAS_MSG_KEY | GenericMsgFlags.MESSAGE_COMPLETE;

            if ((msgBuf = chnl.GetBuffer(ChannelSession.MAX_MSG_SIZE, false, out error)) == null)
            {
                Console.WriteLine("Failed chnl.GetBuffer. Error:{0}", error.ToString());
                chnl.Close(out error);
                return TransportReturnCode.FAILURE;
            }

            encodeIter.Clear();

            CodecReturnCode ret = encodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed encodeIter.SetBufferAndRWFVersion. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = genMsg.EncodeInit(encodeIter, 0)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed genMsg.EncodeInit. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            map.KeyPrimitiveType = Codec.DataTypes.BUFFER;
            map.ContainerType = Codec.DataTypes.ELEMENT_LIST;
            if ((ret = map.EncodeInit(encodeIter, 0, 0)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed map.EncodeInit. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            mEntry.Action = MapEntryActions.ADD;

            if ((ret = mEntry.Encode(encodeIter)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed mEntry.Encode. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;
            if ((ret = elementList.EncodeInit(encodeIter, null, 0)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed elementList.EncodeInit. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            elementEntry.DataType = Codec.DataTypes.UINT;
            elementEntry.Name = ElementNames.WARMSTANDBY_MODE;
            if ((ret = elementEntry.Encode(encodeIter)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed elementEntry.Encode. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = elementEntry.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed elementEntry.EncodeComplete. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = elementList.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed elementList.EncodeComplete. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = mEntry.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed mEntry.Encode. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = map.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed map.EncodeComplete. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }

            if ((ret = genMsg.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed genMsg.EncodeComplete. Code: {0}", ret.ToString());
                return (TransportReturnCode)ret;
            }
            WriteArgs writeArgs = new WriteArgs();

            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
            retChnl = chnl.Write(msgBuf, writeArgs, out error);
            if (retChnl < TransportReturnCode.SUCCESS)
            {
                chnl.Close(out error);
                chnl.ReleaseBuffer(msgBuf, out error);
                Console.WriteLine("Failed chnl.Write. Code: {0}", ret.ToString());
                return retChnl;
            }

            retChnl = chnl.Flush(out error);
            return retChnl;
        }
        //END APIQA


        //APIQA
        public CodecReturnCode EncodeSourceDirectoryRequestCustom(IChannel chnl, ITransportBuffer msgBuf, int streamId, long filter) 
        {
            IRequestMsg requestMsg = (IRequestMsg)new Msg();
            EncodeIterator encodeIter = new EncodeIterator();

            requestMsg.MsgClass = MsgClasses.REQUEST;
            requestMsg.StreamId = streamId;
            requestMsg.DomainType = (int)DomainType.SOURCE;
            requestMsg.ContainerType = Codec.DataTypes.NO_DATA;
            requestMsg.Flags = RequestMsgFlags.STREAMING | RequestMsgFlags.HAS_PRIORITY;
            requestMsg.Priority.PriorityClass = 1;
            requestMsg.Priority.Count = 1;

            requestMsg.MsgKey.Flags = MsgKeyFlags.HAS_FILTER;
            requestMsg.MsgKey.Filter = filter;

            CodecReturnCode ret = encodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Failed encodeIter.SetBufferAndRWFVersion. Code: {0}", ret.ToString());
                return ret;
            }
            if ((ret = requestMsg.EncodeInit(encodeIter, 0)) != CodecReturnCode.SUCCESS) 
            {
                Console.WriteLine("Failed requestMsg.EncodeInit. Code: {0}", ret.ToString());
                return ret;
            }
            if ((ret = requestMsg.EncodeComplete(encodeIter, true)) != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Failed requestMsg.EncodeComplete. Code: {0}", ret.ToString());
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }
        //END APIQA
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

                    // APIQA
                    TransportReturnCode transportReturnCode;
                    if (_reissueDirFilter != -1) 
                    {
                        Console.Write("\n\n*********************************\n");
                        Console.Write("Reissue directory with filter {0}*\n", _reissueDirFilter);
                        Console.Write("*********************************\n\n");

                        ITransportBuffer? msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
                        if (msgBuf == null)
                        {
                            Console.WriteLine("Failed chnl.GetBuffer.");
                            return TransportReturnCode.FAILURE;
                        }
                        if ((ret = EncodeSourceDirectoryRequestCustom(chnl, msgBuf, SRCDIR_STREAM_ID, _reissueDirFilter)) != CodecReturnCode.SUCCESS) 
                        {
                            chnl.ReleaseBuffer(msgBuf, out error);
                            Console.WriteLine("Failed chnl.GetBuffer. Code: {0}", ret.ToString());
                            return TransportReturnCode.FAILURE;
                        }
                        WriteArgs writeArgs = new WriteArgs();

                        writeArgs.Priority = WritePriorities.HIGH;
                        writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
                        
                        if ((transportReturnCode  = chnl.Write(msgBuf, writeArgs, out error))!= TransportReturnCode.SUCCESS)
                        {
                            chnl.ReleaseBuffer(msgBuf, out error);
                            Console.WriteLine("Failed chnl.Write. Code: {0}", transportReturnCode.ToString());
                            return transportReturnCode;
                        }

                        if ((transportReturnCode = chnl.Flush(out error)) != TransportReturnCode.SUCCESS) 
                        {
                            chnl.ReleaseBuffer(msgBuf, out error);
                            Console.WriteLine("Failed chnl.Flush. Code: {0}", transportReturnCode.ToString());
                            return transportReturnCode;
                        }
                    }
                    if (_sendGenericMsg)
                    {
                        transportReturnCode = SendGenericMsgOnDirectory(chnl, out error);
                        return transportReturnCode;
                    }
                //END APIQA
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
