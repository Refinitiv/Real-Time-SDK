/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Eta.Rdm.Directory;

namespace LSEG.Eta.ValueAdd.Provider
{
    /// <summary>
    /// This is the source directory handler for the ETA .NET Provider application. 
    /// Only one source directory stream per channel is allowed by this simple provider.
    /// It provides methods for processing source directory requests from consumers 
    /// and sending back the responses.
    /// Methods for sending source directory request reject/close status messages, 
    /// initializing the source directory handler, setting the service name, 
    /// getting/setting the service id, checking if a request has minimal filter 
    /// flags, and closing source directory streams are also provided.
    /// </summary>
    internal class DirectoryHandler
    {
        private const int REJECT_MSG_SIZE = 1024;
        private const int STATUS_MSG_SIZE = 1024;
        private const int REFRESH_MSG_SIZE = 1024;

        private DirectoryClose m_DirectoryClose = new DirectoryClose();
        private DirectoryStatus m_DirectoryStatus = new DirectoryStatus();
        private DirectoryRefresh m_DirectoryRefresh = new DirectoryRefresh();
        private DirectoryRequest m_DirectoryRequest = new DirectoryRequest();
        private EncodeIterator m_EncodeIter = new EncodeIterator();
        private DirectoryRequestInfoList m_DirectoryRequestInfoList;

        private const string vendor = "Refinitiv";
        private const string m_FieldDictionaryName = "RWFFld";
        private const string m_EnumTypeDictionaryName = "RWFEnum";
        private const string m_LinkName = "ETA Provider Link";
        public const int OPEN_LIMIT = 10;

        private Service m_Service = new Service();
        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        /// <summary>
        /// Service id associated with the service name of the provider
        /// </summary>
        public int ServiceId { get; set; } = 1234;

        /// <summary>
        /// Service name of the provider
        /// </summary>
        public string? ServiceName { get; set; }

        public DirectoryHandler()
        {
            m_DirectoryRequestInfoList = new DirectoryRequestInfoList();
        }

        /// <summary>
        /// Initializes source directory information fields.
        /// </summary>
        public void Init()
        {
            m_DirectoryRequestInfoList.Init();
        }

        /// <summary>
        /// Sends directory close status message to a channel.
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendCloseStatus(ReactorChannel chnl, out ReactorErrorInfo errorInfo)
        {
            // proceed if source directory request info found
            DirectoryRequestInfo? directoryReqInfo = FindDirectoryReqInfo(chnl.Channel!);
            if (directoryReqInfo is null)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"FindDirectoryReqInfo() failed: DirectoryRequestInfo is null";
                return ReactorReturnCode.SUCCESS;
            }

            // get a buffer for the source directory request
            ITransportBuffer? msgBuf = chnl.GetBuffer(STATUS_MSG_SIZE, false, out errorInfo!);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            // encode directory close
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            m_DirectoryStatus.StreamId = directoryReqInfo.DirectoryRequest.StreamId;

            m_DirectoryStatus.HasState = true;
            m_DirectoryStatus.State.StreamState(StreamStates.CLOSED);
            m_DirectoryStatus.State.DataState(DataStates.SUSPECT);
            m_DirectoryStatus.State.Text().Data("Directory stream closed");
            ret = m_DirectoryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "DirectoryStatus.Encode failed";
                return ReactorReturnCode.FAILURE;
            }

            // send close status
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo!);
        }

        /// <summary>
        /// Sends the source directory request reject status message to a channel.
        /// </summary>
        /// <param name="chnl"><see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="streamId">the id of the directory stream</param>
        /// <param name="reason">the reject reason</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendRequestReject(ReactorChannel chnl, int streamId, DirectoryRejectReason reason, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the login request reject status
            ITransportBuffer? msgBuf = chnl.GetBuffer(REJECT_MSG_SIZE, false, out errorInfo);
            if (msgBuf != null)
            {
                // encode login request reject status
                ReactorReturnCode ret = EncodeRequestReject(chnl, streamId, reason, msgBuf, errorInfo);
                if (ret != ReactorReturnCode.SUCCESS)
                {
                    errorInfo = new ReactorErrorInfo();
                    errorInfo.Error.Text = $"EncodeRequestReject failed: {ret}";
                    return ret;
                }

                // send request reject status 
                return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
            }
            else
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "Channel.GetBuffer(): Failed " + errorInfo.Error.Text;
                return ReactorReturnCode.FAILURE;
            }
        }

        private ReactorReturnCode EncodeRequestReject(ReactorChannel chnl, int streamId, DirectoryRejectReason reason, ITransportBuffer msgBuf, ReactorErrorInfo? errorInfo)
        {
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
                    m_DirectoryStatus.State.Text().Data($"Source directory rejected for stream id {streamId} - max request count reached");
                    break;
                case DirectoryRejectReason.INCORRECT_FILTER_FLAGS:
                    m_DirectoryStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_DirectoryStatus.State.Text().Data($"Source directory request rejected for stream id {streamId} - request must minimally have INFO, STATE, and GROUP filter flags");
                    break;
                case DirectoryRejectReason.DIRECTORY_RDM_DECODER_FAILED:
                    m_DirectoryStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_DirectoryStatus.State.Text().Data($"Source directory request rejected for stream id {streamId} - decoding failure: {errorInfo?.Error.Text}");
                    break;
                default:
                    break;
            }

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_DirectoryStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"DirectoryStatus.Encode() failed with return code {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Closes the directory stream
        /// </summary>
        /// <param name="streamId">the id of the directory stream</param>
        public void CloseStream(int streamId)
        {
            // find original request information associated with chnl
            foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
            {
                if (sourceDirectoryReqInfo.DirectoryRequest.StreamId == streamId && sourceDirectoryReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing source directory stream id '{sourceDirectoryReqInfo.DirectoryRequest.StreamId}' with service name: {ServiceName}");
                    sourceDirectoryReqInfo.Clear();
                    break;
                }
            }
        }

        /// <summary>
        /// Closes the directory stream
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        public void CloseStream(ReactorChannel chnl)
        {
            //find original request information associated with chnl
            DirectoryRequestInfo? dirReqInfo = FindDirectoryReqInfo(chnl.Channel!);
            if (dirReqInfo != null)
            {
                // clear original request information 
                Console.WriteLine($"Closing source directory stream id '{dirReqInfo.DirectoryRequest.StreamId}' with service name: {ServiceName}");
                dirReqInfo.Clear();
            }
        }

        /// <summary>
        /// Finds dictionary request information for a channel.
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <returns><see cref="DirectoryRequestInfo"/> instance</returns>
        private DirectoryRequestInfo? FindDirectoryReqInfo(IChannel chnl)
        {
            //find original request information associated with chnl
            foreach (DirectoryRequestInfo sourceDirectoryReqInfo in m_DirectoryRequestInfoList)
            {
                if (sourceDirectoryReqInfo.Channel == chnl && sourceDirectoryReqInfo.IsInUse)
                {
                    return sourceDirectoryReqInfo;
                }
            }

            return null;
        }

        /// <summary>
        /// Finds dictionary request information for a channel.
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="directoryRequest">the <see cref="DirectoryRequest"/> received from the client</param>
        /// <returns><see cref="DirectoryRequest"/> instance to be used by the application</returns>
        public DirectoryRequest? GetDirectoryRequest(ReactorChannel chnl, DirectoryRequest directoryRequest)
        {
            return m_DirectoryRequestInfoList.Get(chnl.Channel!, directoryRequest);
        }

        /// <summary>
        /// Sends Directory Refresh message to the client
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance associated with the current client</param>
        /// <param name="srcDirReqInfo"><see cref="DirectoryRequest"/> instance</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendRefresh(ReactorChannel chnl, DirectoryRequest srcDirReqInfo, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the source directory request
            ITransportBuffer? msgBuf = chnl.GetBuffer(REFRESH_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
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

            if ((srcDirReqInfo.Filter & ServiceFilterFlags.INFO) != 0)
            {
                m_Service.HasInfo = true;
                m_Service.Info.Action = FilterEntryActions.SET;

                // vendor
                m_Service.Info.HasVendor = true;
                m_Service.Info.Vendor.Data(vendor);

                // service name - required
                m_Service.Info.ServiceName.Data(ServiceName);


                // Qos Range is not supported
                m_Service.Info.HasSupportQosRange = true;
                m_Service.Info.SupportsQosRange = 0;

                // capabilities - required
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_PRICE);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_ORDER);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_PRICE);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.DICTIONARY);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.SYMBOL_LIST);
                m_Service.Info.CapabilitiesList.Add((long)DomainType.SYSTEM);

                // qos
                m_Service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                m_Service.Info.QosList.Add(qos);


                // dictionary used
                m_Service.Info.HasDictionariesUsed = true;
                m_Service.Info.DictionariesUsedList.Add(m_FieldDictionaryName);
                m_Service.Info.DictionariesUsedList.Add(m_EnumTypeDictionaryName);


                // dictionary provided
                m_Service.Info.HasDictionariesProvided = true;
                m_Service.Info.DictionariesProvidedList.Add(m_FieldDictionaryName);
                m_Service.Info.DictionariesProvidedList.Add(m_EnumTypeDictionaryName);


                // isSource = Service is provided directly from original publisher
                m_Service.Info.HasIsSource = true;
                m_Service.Info.IsSource = 1;

                // itemList - Name of SymbolList that includes all of the items that
                // he publisher currently provides.
                m_Service.Info.HasItemList = true;
                m_Service.Info.ItemList.Data("_ETA_ITEM_LIST");

                // accepting customer status = no
                m_Service.Info.HasAcceptingConsStatus = true;
                m_Service.Info.AcceptConsumerStatus = 1;

                // supports out of band snapshots = no
                m_Service.Info.HasSupportOOBSnapshots = true;
                m_Service.Info.SupportsOOBSnapshots = 0;
            }

            if ((srcDirReqInfo.Filter & ServiceFilterFlags.STATE) != 0)
            {
                m_Service.HasState = true;
                m_Service.State.Action = FilterEntryActions.SET;

                // service state
                m_Service.State.ServiceStateVal = 1;

                // accepting requests
                m_Service.State.HasAcceptingRequests = true;
                m_Service.State.AcceptingRequests = 1;
            }

            if ((srcDirReqInfo.Filter & ServiceFilterFlags.LOAD) != 0)
            {
                m_Service.HasLoad = true;
                m_Service.Load.Action = FilterEntryActions.SET;

                // open limit
                m_Service.Load.HasOpenLimit = true;
                m_Service.Load.OpenLimit = OPEN_LIMIT;
            }

            if ((srcDirReqInfo.Filter & ServiceFilterFlags.LINK) != 0)
            {
                m_Service.HasLink = true;
                m_Service.Link.Action = FilterEntryActions.SET;

                ServiceLink serviceLink = new ServiceLink();

                // link name - Map Entry Key 
                serviceLink.Name.Data(m_LinkName);

                // link type
                serviceLink.HasType = true;
                serviceLink.Type = LinkTypes.INTERACTIVE;

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
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_DirectoryRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "DirectoryRefresh.Encode() failed";
                return ReactorReturnCode.FAILURE;
            }

            // send source directory request
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

    }
}
