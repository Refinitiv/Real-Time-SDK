/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the source directory handler for the ETA NIProvider application. 
    /// It provides methods for sending the source directory refresh to a consumer (e.g. ADH). 
    /// Methods for setting the service name, getting the service information, 
    /// and closing a source directory stream are also provided.
    /// </summary>
    public class NiProviderDirectoryHandler
    {
        private const int SRCDIR_STREAM_ID = -1;
        private const int OPEN_LIMIT = 5;
        private const string VENDOR = "Refinitiv";
        private const string LINK_NAME = "NI_PUB";

        private const long FILTER_TO_REFRESH = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.LOAD | ServiceFilterFlags.LINK;

        public static int TRANSPORT_BUFFER_SIZE_REFRESH = ChannelSession.MAX_MSG_SIZE;
        public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

        public Buffer ServiceName { get; set; } =  new Buffer();
        public int ServiceId { get; set; } = 1;
        public Service ServiceInfo { get; set; } = new Service();
        
        private EncodeIterator encIter = new EncodeIterator();
        private DirectoryRefresh directoryRefresh = new DirectoryRefresh();
        private DirectoryClose directoryClose = new DirectoryClose();

        /// <summary>
        /// Checks if the service is up. The service will be up once the refresh is sent.
        /// </summary>
        /// <returns>true if the service is up.</returns>
        public bool IsServiceUp()
        {
            if (ServiceInfo.HasState && ServiceInfo.State.HasAcceptingRequests && ServiceInfo.State.ServiceStateVal == 1 && ServiceInfo.State.AcceptingRequests == 1)
                return true;

            return false;
        }

        /// <summary>
        /// Sends a source directory refresh to a channel. 
        /// This consists of getting a message buffer, encoding the source directory refresh, 
        /// sending the source directory refresh to the server.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory refresh to.</param>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure 
        /// in case of nusuccessful operation.</param>
        /// <returns><see cref="TransportReturnCode"/> value that indicates the status of the operation.</returns>
        public TransportReturnCode SendRefresh(ChannelSession chnl, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REFRESH, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }
                
            directoryRefresh.Clear();
            directoryRefresh.StreamId = SRCDIR_STREAM_ID;

            directoryRefresh.ClearCache = true;

            Buffer text = new Buffer();
            text.Data("Source Directory Refresh Completed");
            directoryRefresh.State.Clear();
            directoryRefresh.State.StreamState(StreamStates.OPEN);
            directoryRefresh.State.DataState(DataStates.OK);
            directoryRefresh.State.Code(StateCodes.NONE);
            directoryRefresh.State.Text(text);

            directoryRefresh.Filter = FILTER_TO_REFRESH;

            ServiceInfo.Clear();
            ServiceInfo.Action = MapEntryActions.ADD;
            ServiceInfo.ServiceId = ServiceId;

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.INFO) != 0)
            {
                ServiceInfo.HasInfo = true;
                ServiceInfo.Info.Action = FilterEntryActions.SET;

                ServiceInfo.Info.HasVendor = true;
                ServiceInfo.Info.Vendor.Data(VENDOR);

                ServiceInfo.Info.ServiceName.Data(ServiceName.ToString());

                ServiceInfo.Info.HasSupportQosRange = true;
                ServiceInfo.Info.SupportsQosRange = 0;

                ServiceInfo.Info.CapabilitiesList.Add((long)DomainType.MARKET_PRICE);
                ServiceInfo.Info.CapabilitiesList.Add((long)DomainType.MARKET_BY_ORDER);

                ServiceInfo.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                ServiceInfo.Info.QosList.Add(qos);

                ServiceInfo.Info.HasDictionariesUsed = true;
                ServiceInfo.Info.DictionariesUsedList.Add(NiProviderDictionaryHandler.FIELD_DICTIONARY_DOWNLOAD_NAME);
                ServiceInfo.Info.DictionariesUsedList.Add(NiProviderDictionaryHandler.ENUM_TYPE_DOWNLOAD_NAME);

                ServiceInfo.Info.HasIsSource = true;
                ServiceInfo.Info.IsSource = 1;

                ServiceInfo.Info.HasItemList = true;
                ServiceInfo.Info.ItemList.Data("");

                ServiceInfo.Info.HasAcceptingConsStatus = true;
                ServiceInfo.Info.AcceptConsumerStatus = 0;

                ServiceInfo.Info.HasSupportOOBSnapshots = true;
                ServiceInfo.Info.SupportsOOBSnapshots = 0;
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.STATE) != 0)
            {
                ServiceInfo.HasState = true;
                ServiceInfo.State.Action = FilterEntryActions.SET;
                ServiceInfo.State.ServiceStateVal = 1;

                ServiceInfo.State.HasAcceptingRequests = true;
                ServiceInfo.State.AcceptingRequests = 1;

                ServiceInfo.State.HasStatus = true;
                ServiceInfo.State.Status.DataState(DataStates.OK);
                ServiceInfo.State.Status.StreamState(StreamStates.OPEN);
                ServiceInfo.State.Status.Code(StateCodes.NONE);
                ServiceInfo.State.Status.Text().Data("OK");
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.LOAD) != 0)
            {
                ServiceInfo.HasLoad = true;
                ServiceInfo.Load.Action = FilterEntryActions.SET;

                //open limit
                ServiceInfo.Load.HasOpenLimit = true;
                ServiceInfo.Load.OpenLimit = OPEN_LIMIT;

                //load factor
                ServiceInfo.Load.HasLoadFactor = true;
                ServiceInfo.Load.LoadFactor = 1;
            }

            if ((FILTER_TO_REFRESH & ServiceFilterFlags.LINK) != 0)
            {
                ServiceInfo.HasLink = true;
                ServiceInfo.Link.Action = FilterEntryActions.SET;

                ServiceLink serviceLink = new ServiceLink();

                //link name - Map Entry Key
                serviceLink.Name.Data(LINK_NAME);

                //link type
                serviceLink.HasType = true;
                serviceLink.Type = LinkTypes.INTERACTIVE;

                //link state
                serviceLink.LinkState = LinkStates.UP;

                //link code
                serviceLink.HasCode = true;
                serviceLink.LinkCode = LinkCodes.OK;

                //link text
                serviceLink.HasText = true;
                serviceLink.Text.Data("Link state is up");

                ServiceInfo.Link.LinkList.Add(serviceLink);
            }

            directoryRefresh.ServiceList.Add(ServiceInfo);

            //encode directory request
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            CodecReturnCode ret = directoryRefresh.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = "EncodeDirectoryRefresh failed, return code: <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Close the source directory stream.
        /// </summary>
        /// <param name="chnl">The channel to send a source directory close to.</param>
        /// <param name="error"><see cref="Error"/> instance that carries information about failure 
        /// in case of nusuccessful operation.</param>
        /// <returns><see cref="TransportReturnCode"/> value that indicates the status of the operation.</returns>
        public TransportReturnCode CloseStream(ChannelSession chnl, out Error? error)
        {
            ServiceInfo.Clear();

            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            directoryClose.Clear();
            directoryClose.StreamId = SRCDIR_STREAM_ID;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);
            CodecReturnCode ret = directoryClose.Encode(encIter);

            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encode SourceDirectory Close : Failed <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            return chnl.Write(msgBuf, out error);
        }
    }
}
