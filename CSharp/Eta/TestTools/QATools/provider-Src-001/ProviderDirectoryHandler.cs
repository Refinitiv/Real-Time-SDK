/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the source directory handler for the ETA C# Provider application.
    /// <para>
    /// Only one source directory stream per channel is allowed by this simple provider.
    /// </para>
    /// <para>
    /// It provides methods for processing source directory requests from consumers
    /// and sending back the responses.
    /// </para>
    /// <para>
    /// Methods for sending source directory request reject/close status messages,
    /// initializing the source directory handler, setting the service name,
    /// getting/setting the service id, checking if a request has minimal filter
    /// flags, and closing source directory streams are also provided.
    /// </para>
    /// </summary>
    public class ProviderDirectoryHandler
    {
        private static readonly int REJECT_MSG_SIZE = 1024;
        private static readonly int STATUS_MSG_SIZE = 1024;
        private static readonly int REFRESH_MSG_SIZE = 1024;

        private DirectoryClose m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus m_DirectoryStatus = new DirectoryStatus();
        private DirectoryRefresh m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryRequest m_DirectoryRequest = new DirectoryRequest();
        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private DirectoryRequestInfoList m_DirectoryRequestInfoList;

        private bool m_EnableGenericProvider; // used for generic provider

        // service name of provider
        public string? ServiceName { get; set; }

        // service id associated with the service name of provider
        public int ServiceId { get; set; } = 1234;

        // vendor name
        private static readonly string Vendor = "LSEG";

        // field dictionary name
        private static readonly string FieldDictionaryName = "RWFFld";

        // enumtype dictionary name
        private static readonly string EnumTypeDictionaryName = "RWFEnum";

        // link name
        private static readonly string LinkName = "ETA Provider Link";

        public static readonly int OPEN_LIMIT = 50;

        private ProviderSession m_ProviderSession;

        private Service m_Service = new Service();

        public static readonly int GENERIC_DOMAIN = 200; // used for generic provider

        /// <summary>
        /// Instantiates a new provider directory handler.
        /// </summary>
        /// <param name="providerSession">the provider sesssion</param>
        public ProviderDirectoryHandler(ProviderSession providerSession)
        {
            m_DirectoryRequestInfoList = new DirectoryRequestInfoList();
            m_ProviderSession = providerSession;
        }

        /// <summary>
        /// Enables the source directory handler for a generic provider.
        /// A generic provider does not use a dictionary and is only capable
        /// of providing the user-defined generic domain.
        /// </summary>
        public void EnableGenericProvider()
        {
            m_EnableGenericProvider = true;
        }

        /// <summary>
        /// Initializes source directory information fields.
        /// </summary>
        public void Init()
        {
            m_DirectoryRequestInfoList.Init();
        }

        /// <summary>
        /// Processes a source directory request. This consists of calling
        /// <c>DirectoryRequest.Decode()</c> to decode the request and calling
        /// <c>SendSourceDirectoryResponse()</c> to send the source directory response.
        /// </summary>
        /// <param name="chnl">The channel of the response</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error">Error information in case of any encoding or socket
        /// writing error
        /// </param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode ProcessRequest(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    // get key
                    IMsgKey msgKey = msg.MsgKey;
                    if (!KeyHasMinFilterFlags(msgKey))
                    {
                        return SendRequestReject(chnl, msg.StreamId, DirectoryRejectReason.INCORRECT_FILTER_FLAGS, out error);
                    }
                    m_DirectoryRequest.Clear();
                    if (m_DirectoryRequest.Decode(dIter, msg) != CodecReturnCode.SUCCESS)
                    {
                        return SendRequestReject(chnl, msg.StreamId, DirectoryRejectReason.INCORRECT_FILTER_FLAGS, out error);
                    }
                    DirectoryRequest? pdirectoryRequest = m_DirectoryRequestInfoList.Get(chnl, m_DirectoryRequest);
                    if (pdirectoryRequest is null)
                    {
                        return SendRequestReject(chnl, msg.StreamId, DirectoryRejectReason.MAX_SRCDIR_REQUESTS_REACHED, out error);
                    }

                    Console.WriteLine("Received Source Directory Request");

                    // send source directory response
                    //APIQA
                    //return SendRefresh(chnl, pdirectoryRequest, out error);
                    return CodecReturnCode.SUCCESS;
                    //APIQA
                case MsgClasses.CLOSE:
                    {
                        Console.WriteLine($"Received Directory Close for StreamId {msg.StreamId}");

                        // close directory stream
                        CloseStream(msg.StreamId);
                        return CodecReturnCode.SUCCESS;
                    }
                default:

                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"Received unhandled Source Directory msg type: {msg.MsgClass}"
                    };

                    return CodecReturnCode.FAILURE;
            }
        }

        /// <summary>
        /// Sends directory close status message to a channel.
        /// </summary>
        /// <param name="chnl">The channel to send close status message to</param>
        /// <param name="error">Error information in case of encoding or socket writing
        /// failure.
        /// </param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendCloseStatus(IChannel chnl, out Error? error)
        {
            error = null;
            // proceed if source directory request info found
            DirectoryRequestInfo? directoryReqInfo = FindDirectoryReqInfo(chnl);
            if (directoryReqInfo is null)
            {
                return CodecReturnCode.SUCCESS;
            }

            // get a buffer for the source directory request
            ITransportBuffer msgBuf = chnl.GetBuffer(STATUS_MSG_SIZE, false, out error);
            if (msgBuf is null)
            {
                return CodecReturnCode.FAILURE;
            }

            // encode directory close
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            m_DirectoryStatus.StreamId = directoryReqInfo.DirectoryRequest.StreamId;

            m_DirectoryStatus.HasState = true;
            m_DirectoryStatus.State.StreamState(StreamStates.CLOSED);
            m_DirectoryStatus.State.DataState(DataStates.SUSPECT);
            m_DirectoryStatus.State.Text().Data("Directory stream closed");
            ret = m_DirectoryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"DirectoryStatus.Encode failed"
                };

                return ret;
            }

            // send close status
            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        private CodecReturnCode SendRequestReject(IChannel chnl, int streamId, DirectoryRejectReason reason, out Error? error)
        {
            // get a buffer for the login request reject status
            ITransportBuffer msgBuf = chnl.GetBuffer(REJECT_MSG_SIZE, false, out error);
            if (msgBuf != null)
            {
                // encode login request reject status
                CodecReturnCode ret = EncodeRequestReject(chnl, streamId, reason, msgBuf, out error);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return ret;
                }

                // send request reject status
                return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
            }
            else
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"Channel.GetBuffer(): Failed {error.Text}"
                };

                return CodecReturnCode.FAILURE;
            }
        }

        private CodecReturnCode EncodeRequestReject(IChannel chnl, int streamId, DirectoryRejectReason reason, ITransportBuffer msgBuf, out Error? error)
        {
            error = null;
            // clear encode iterator
            m_EncodeIter.Clear();

            // set-up message
            m_DirectoryStatus.StreamId = streamId;
            m_DirectoryStatus.HasState = true;
            m_DirectoryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_DirectoryStatus.State.DataState(DataStates.SUSPECT);
            switch (reason)
            {
                case DirectoryRejectReason.MAX_SRCDIR_REQUESTS_REACHED:
                    m_DirectoryStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_DirectoryStatus.State.Text().Data($"Source directory rejected for stream id {streamId}- max request count reached");
                    break;
                case DirectoryRejectReason.INCORRECT_FILTER_FLAGS:
                    m_DirectoryStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_DirectoryStatus.State.Text().Data($"Source directory request rejected for stream id  {streamId}- request must minimally have INFO, STATE, and GROUP filter flags");
                    break;
                case DirectoryRejectReason.DIRECTORY_RDM_DECODER_FAILED:
                    m_DirectoryStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_DirectoryStatus.State.Text().Data($"Source directory request rejected for stream id  {streamId}- decoding failure");
                    break;
                default:
                    break;
            }

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };
                return ret;
            }

            ret = m_DirectoryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"DirectoryStatus.Encode() failed"
                };
            }

            return ret;
        }

        /// <summary>
        /// Closes the source directory stream for a channel.
        /// </summary>
        /// <param name="chnl">The channel to close the source directory stream for</param>
        public void CloseRequest(IChannel chnl)
        {
            //find original request information associated with chnl
            DirectoryRequestInfo? dirReqInfo = FindDirectoryReqInfo(chnl);
            if (dirReqInfo != null)
            {
                // clear original request information
                Console.WriteLine($"Closing source directory stream id '{dirReqInfo.DirectoryRequest.StreamId}' with service name: {ServiceName}");
                dirReqInfo.Clear();
            }
        }

        private DirectoryRequestInfo? FindDirectoryReqInfo(IChannel chnl)
        {
            //find original request information associated with chnl
            foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
            {
                if (Object.ReferenceEquals(sourceDirectoryReqInfo.Channel,chnl) && sourceDirectoryReqInfo.IsInUse)
                {
                    return sourceDirectoryReqInfo;
                }
            }

            return null;
        }
        private void CloseStream(int streamId)
        {
            // find original request information associated with chnl
            foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
            {
                if (sourceDirectoryReqInfo.DirectoryRequest.StreamId == streamId && sourceDirectoryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing source directory stream id '{m_DirectoryRequest.StreamId}' with service name: {ServiceName}");
                    sourceDirectoryReqInfo.Clear();
                    break;
                }
            }
        }

        /// <summary>
        /// Sends directory refresh message to a channel.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory response to</param>
        /// <param name="srcDirReqInfo">The source directory request information</param>
        /// <param name="error">Error information populated in case of encoding or socket
        /// writing
        /// </param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendRefresh(IChannel chnl, DirectoryRequest srcDirReqInfo, out Error? error)
        {
            // get a buffer for the source directory request
            ITransportBuffer msgBuf = chnl.GetBuffer(REFRESH_MSG_SIZE, false, out error);
            if (msgBuf is null)
            {
                return CodecReturnCode.FAILURE;
            }

            // encode source directory request
            m_DirectoryRefresh.Clear();
            m_DirectoryRefresh.StreamId = srcDirReqInfo.StreamId;

            // clear cache
            m_DirectoryRefresh.ClearCache = true;
            m_DirectoryRefresh.Solicited = true;

            // state information for response message
            m_DirectoryRefresh.State.Clear();
            m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
            m_DirectoryRefresh.State.DataState(DataStates.OK);
            m_DirectoryRefresh.State.Code(StateCodes.NONE);
            m_DirectoryRefresh.State.Text().Data("Source Directory Refresh Completed");

            // attribInfo information for response message
            m_DirectoryRefresh.Filter = srcDirReqInfo.Filter;

            // populate the Service
            m_Service.Clear();
            m_Service.Action = MapEntryActions.ADD;

            // set the service Id (map key)
            m_Service.ServiceId = ServiceId;

            if ((srcDirReqInfo.Filter & Rdm.Directory.ServiceFilterFlags.INFO) != 0)
            {
                m_Service.HasInfo = true;
                m_Service.Info.Action = FilterEntryActions.SET;

                // vendor
                m_Service.Info.HasVendor = true;
                m_Service.Info.Vendor.Data(Vendor);

                // service name - required
                m_Service.Info.ServiceName.Data(ServiceName);


                // Qos Range is not supported
                m_Service.Info.HasSupportQosRange = true;
                m_Service.Info.SupportsQosRange = 0;

                // capabilities - required
                if (!m_EnableGenericProvider)
                {
                    m_Service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_PRICE);
                    m_Service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_BY_ORDER);
                    m_Service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_BY_PRICE);
                    m_Service.Info.CapabilitiesList.Add((long)Rdm.DomainType.DICTIONARY);
                    m_Service.Info.CapabilitiesList.Add((long)Rdm.DomainType.SYMBOL_LIST);
                }
                else // generic provider only supports the user-defined generic domain
                {
                    m_Service.Info.CapabilitiesList.Add((long)GENERIC_DOMAIN);
                }

                // qos
                m_Service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                m_Service.Info.QosList.Add(qos);

                // dictionary used
                if (!m_EnableGenericProvider)
                {
                    m_Service.Info.HasDictionariesUsed = true;
                    m_Service.Info.DictionariesUsedList.Add(FieldDictionaryName);
                    m_Service.Info.DictionariesUsedList.Add(EnumTypeDictionaryName);
                }

                // dictionary provided
                if (!m_EnableGenericProvider)
                {
                    m_Service.Info.HasDictionariesProvided = true;
                    m_Service.Info.DictionariesProvidedList.Add(FieldDictionaryName);
                    m_Service.Info.DictionariesProvidedList.Add(EnumTypeDictionaryName);
                }

                // isSource = Service is provided directly from original publisher
                m_Service.Info.HasIsSource = true;
                m_Service.Info.IsSource = 1;

                // itemList - Name of SymbolList that includes all of the items that
                // he publisher currently provides.
                if (!m_EnableGenericProvider)
                {
                    m_Service.Info.HasItemList = true;
                    m_Service.Info.ItemList.Data("_ETA_ITEM_LIST");
                }

                // accepting customer status = no
                m_Service.Info.HasAcceptingConsStatus = true;
                m_Service.Info.AcceptConsumerStatus = 0;

                // supports out of band snapshots = no
                m_Service.Info.HasSupportOOBSnapshots = true;
                m_Service.Info.SupportsOOBSnapshots = 0;
            }

            if ((srcDirReqInfo.Filter & Rdm.Directory.ServiceFilterFlags.STATE) != 0)
            {
                m_Service.HasState = true;
                m_Service.State.Action = FilterEntryActions.SET;

                // service state
                m_Service.State.ServiceStateVal = 1;

                // accepting requests
                m_Service.State.HasAcceptingRequests = true;
                m_Service.State.AcceptingRequests = 1;
            }

            if ((srcDirReqInfo.Filter & Rdm.Directory.ServiceFilterFlags.LOAD) != 0)
            {
                m_Service.HasLoad = true;
                m_Service.Load.Action = FilterEntryActions.SET;

                // open limit
                m_Service.Load.HasOpenLimit = true;
                m_Service.Load.OpenLimit = OPEN_LIMIT;
            }

            if ((srcDirReqInfo.Filter & Rdm.Directory.ServiceFilterFlags.LINK) != 0)
            {
                m_Service.HasLink = true;
                m_Service.Link.Action = FilterEntryActions.SET;

                ServiceLink serviceLink = new ServiceLink();

                // link name - Map Entry Key
                serviceLink.Name.Data(LinkName);

                // link type
                serviceLink.HasType = true;
                serviceLink.Type = Rdm.Directory.LinkTypes.INTERACTIVE;

                // link text
                serviceLink.HasText = true;
                serviceLink.Text.Data("Link state is up");
                m_Service.Link.LinkList.Add(serviceLink);
            }

            m_DirectoryRefresh.ServiceList.Add(m_Service);

            // encode directory request
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                //error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_DirectoryRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "DirectoryRefresh.Encode() failed"
                };

                return ret;
            }

            // send source directory request
            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        private bool KeyHasMinFilterFlags(IMsgKey key)
        {
            return key.CheckHasFilter() &&
                   (key.Filter & Rdm.Directory.ServiceFilterFlags.INFO) != 0 &&
                   (key.Filter & Rdm.Directory.ServiceFilterFlags.STATE) != 0 &&
                   (key.Filter & Rdm.Directory.ServiceFilterFlags.GROUP) != 0;
        }
    }
}
