/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Access
{
    internal class ServiceDirectory
    {
        public ServiceDirectory(Service service)
        {
            Service = service;
        }

        public ServiceDirectory(string serviceName)
        {
            ServiceName = serviceName;
        }

        public Service? Service { get; set; }

        public string? ServiceName { get; set; }

        public ChannelInfo? ChannelInfo { get; set; }
    }

    internal class DirectoryCallbackClient<T> : CallbackClient<T>, IDirectoryMsgCallback
    {
        private static readonly string CLIENT_NAME = "DirectoryCallbackClient";

        private IDictionary<int, ServiceDirectory> m_ServiceByIdDict;
        private IDictionary<string, ServiceDirectory> m_ServiceByNameDict;
        private OmmBaseImpl<T> m_OmmBaseImpl;

        public DirectoryRequest? DirectoryRequest { get; private set; }

        public DirectoryCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, CLIENT_NAME)
        {
            m_OmmBaseImpl = baseImpl;

            int serviceCountHint = (int)((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.ServiceCountHint;
            int initialHashSize = (int)(serviceCountHint / 0.75 + 1);
            m_ServiceByIdDict = new Dictionary<int, ServiceDirectory>(initialHashSize);
            m_ServiceByNameDict = new Dictionary<string, ServiceDirectory>(initialHashSize);

            /* Creates these message using the EmaObjManager */
            m_RefreshMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();
            m_UpdateMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmUpdateMsg();
            m_StatusMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();
            m_GenericMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmGenericMsg();
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent dirMsgEvent)
        {
            m_OmmBaseImpl.EventReceived();

            DirectoryMsg? directoryMsg = dirMsgEvent.DirectoryMsg;
            ReactorChannel? reactorChannel = dirMsgEvent.ReactorChannel;
            ChannelInfo? channelInfo = (ChannelInfo?)reactorChannel?.UserSpecObj;

            if (directoryMsg == null)
            {
                m_OmmBaseImpl.CloseReactorChannel(reactorChannel);

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    Error error = dirMsgEvent.ReactorErrorInfo.Error;

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                    message.AppendLine("Received event without RDMDirectory message")
                        .AppendLine($"\tReactor's hash code {channelInfo?.Reactor?.GetHashCode()}")
                        .AppendLine($"\tReactorChannel's hash code {reactorChannel?.GetHashCode()}")
                        .AppendLine($"\tError Id {error.ErrorId}").AppendLine($"Internal SysError {error.SysError}")
                        .AppendLine($"\tError Location {dirMsgEvent.ReactorErrorInfo.Location}")
                        .AppendLine($"\tError Text {error.Text}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            object? item = dirMsgEvent.StreamInfo != null ? dirMsgEvent.StreamInfo.UserSpec : null;
            if (item != null)
               return ProcessCallback(dirMsgEvent, reactorChannel!, (Item<T>) item);

            switch (directoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REFRESH:
                    {
                        State state = directoryMsg.DirectoryRefresh!.State;

                        if(state.StreamState() != StreamStates.OPEN)
                        {
                            m_OmmBaseImpl.CloseReactorChannel(reactorChannel);

                            if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                                message.AppendLine("RDMDirectory stream was closed with refresh message ")
                                    .Append($"\t{state.ToString()}");

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                            }

                            ProcessDirectoryPayload(directoryMsg.DirectoryRefresh!.ServiceList, reactorChannel!);
                            break;
                        }
                        else if (state.DataState() == DataStates.SUSPECT)
                        {
                            if(m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                                message.AppendLine("RDMDirectory stream state was changed to suspect with refresh message ")
                                    .Append($"\t{state.ToString()}");

                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);

                            ProcessDirectoryPayload(directoryMsg.DirectoryRefresh!.ServiceList, reactorChannel!);
                            break;
                        }

                        m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);

                        if(m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                            message.AppendLine("RDMDirectory stream state was open with refresh message ")
                                .Append($"\t{state.ToString()}");

                            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
                        }

                        ProcessDirectoryPayload(directoryMsg.DirectoryRefresh!.ServiceList, reactorChannel!);
                        break;
                    }
                case DirectoryMsgType.STATUS:
                    {
                        if(directoryMsg.DirectoryStatus!.HasState)
                        {
                            State state = directoryMsg.DirectoryStatus!.State;

                            if(state.StreamState() != StreamStates.OPEN)
                            {
                                m_OmmBaseImpl.CloseReactorChannel(reactorChannel);

                                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                                    message.AppendLine($"RDMDirectory stream was closed with status message ")
                                        .Append($"\t{state.ToString()}");

                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                                }

                                return ReactorCallbackReturnCode.SUCCESS;
                            }
                            else if (state.DataState() == DataStates.SUSPECT)
                            {
                                if(m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                                {
                                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                                    message.AppendLine("RDMDirectory stream state was changed to suspect with status message ")
                                        .Append($"\t{state.ToString()}");

                                    m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, message.ToString());
                                }

                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_SUSPECT);
                                break;
                            }

                            if(m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                message.AppendLine("RDMDirectory stream was open with status message ")
                                    .Append($"\t{state}");

                                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.DIRECTORY_STREAM_OPEN_OK);
                        }
                        else
                        {
                            if(m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, "Received RDMDirectory status message without the state");
                            }
                        }

                        break;
                    }
                case DirectoryMsgType.UPDATE:
                    {
                        if(m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                        {
                            m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, "Received RDMDirectory update message");
                        }

                        ProcessDirectoryPayload(directoryMsg.DirectoryUpdate!.ServiceList, reactorChannel!);
                        break;
                    }
                default:
                    {
                        if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                            message.AppendLine($"Received unknown RDMDirectory message type")
                                .Append($"\tmessage type value {directoryMsg.DirectoryMsgType}");

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                        }

                        break;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        ReactorCallbackReturnCode ProcessCallback(RDMDirectoryMsgEvent dirMsgEvent, ReactorChannel reactorChannel, Item<T> item)
        {
            IMsg msg = dirMsgEvent.Msg!;
            switch (dirMsgEvent.DirectoryMsg!.DirectoryMsgType)
            {
                case DirectoryMsgType.REFRESH:
                {
                    m_RefreshMsg!.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, 
                    ((ChannelInfo)reactorChannel.UserSpecObj!).DataDictionary!);

                    EventImpl.Item = item;
                    
                    NotifyOnAllMsg(m_RefreshMsg);
                    NotifyOnRefreshMsg();

                    IRefreshMsg tempRefreshMsg = ((IRefreshMsg)msg);
                    int streamState = tempRefreshMsg.State.StreamState();
                    if (streamState == StreamStates.NON_STREAMING)
                    {
                        if (tempRefreshMsg.CheckRefreshComplete())
                        {
                            EventImpl.Item.Remove();
                        }
                    }
                    else if (streamState != StreamStates.OPEN)
                    {
                        EventImpl.Item.Remove();
                    }
                }
                break;
            case DirectoryMsgType.UPDATE :
                {
                    m_UpdateMsg!.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, 
                            ((ChannelInfo)reactorChannel.UserSpecObj!).DataDictionary!);

                    EventImpl.Item = item;
                    
                    NotifyOnAllMsg(m_UpdateMsg);
                    NotifyOnUpdateMsg();
                }
                break;
            case DirectoryMsgType.STATUS :
                {
                    m_StatusMsg!.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null!);
    
                    EventImpl.Item = item;

                    NotifyOnAllMsg(m_StatusMsg);
                    NotifyOnStatusMsg();
                    
                    IStatusMsg tempStatusMsg = ((IStatusMsg)msg);
                    if (tempStatusMsg.CheckHasState() &&
                            tempStatusMsg.State.StreamState() != StreamStates.OPEN) 
                        EventImpl.Item.Remove();
                }
                break;
            case DirectoryMsgType.CONSUMER_STATUS :
                {
                    m_GenericMsg!.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null!);
    
                    EventImpl.Item = item;
                    
                    NotifyOnAllMsg(m_GenericMsg);
                    NotifyOnGenericMsg();
                }
                break;
            default :
                {
                    if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Internal error. Received unexpected type" +
                            " of RDMDirectoryMsg in DirectoryCallbackClient.ProcessCallback()");
                    }
                    break;
                }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        void ProcessDirectoryPayload(List<Service> serviceList, ReactorChannel channel)
        {
            ChannelInfo? chnlInfo = (ChannelInfo?)channel.UserSpecObj;

            if (serviceList == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Received RDMDirectory message without a service list");
                }

                return;
            }

            if (chnlInfo == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Internal error: no ReactorChannel.UserSpecObj");
                }

                return;
            }

            foreach (Service oneService in serviceList)
            {
                switch (oneService.Action)
                {
                    case MapEntryActions.ADD:
                        {
                            if (!(oneService.HasInfo))
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Received RDMService with Add action but no Service Info");
                                }
                                break;
                            }

                            string serviceName = oneService.Info.ServiceName.ToString();
                            if (serviceName == null)
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Received RDMService with Add action but no Service name");
                                }
                                break;
                            }

                            Service? existingService = null;
                            ServiceDirectory? existingDirectory = null;
                            if (m_ServiceByNameDict.Count > 0)
                            {
                                if (m_ServiceByNameDict.TryGetValue(serviceName, out existingDirectory))
                                {
                                    existingService = existingDirectory.Service;
                                }
                            }

                            if (existingService != null)
                            {
                                if (existingService.ServiceId != oneService.ServiceId)
                                {
                                    m_ServiceByIdDict.Remove(existingService.ServiceId);
                                    existingService.ServiceId = oneService.ServiceId;
                                    m_ServiceByIdDict[existingService.ServiceId] = existingDirectory!;
                                }

                                if (existingDirectory!.ChannelInfo != chnlInfo)
                                {
                                    chnlInfo.DataDictionary = existingDirectory.ChannelInfo!.DataDictionary;
                                    existingDirectory.ChannelInfo = chnlInfo;
                                }
                            }
                            else
                            {
                                Service newService = new();
                                oneService.Copy(newService);

                                ServiceDirectory directory = new (newService);
                                directory.ChannelInfo = chnlInfo;
                                directory.ServiceName = serviceName;

                                m_ServiceByIdDict[oneService.ServiceId] =  directory;
                                m_ServiceByNameDict[serviceName] = directory;

                                if (((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).DictionaryConfig.IsLocalDictionary ||
                                    (newService.State.AcceptingRequests == 1 && newService.State.ServiceStateVal == 1))
                                {
                                    m_OmmBaseImpl.DictionaryCallbackClient!.DownloadDictionary(directory);
                                }
                            }

                            break;
                        }
                    case MapEntryActions.UPDATE:
                        {
                            Service? existingService = null;
                            ServiceDirectory? existingDirectory = null;
                            if (m_ServiceByIdDict.Count > 0)
                            {
                                if (m_ServiceByIdDict.TryGetValue(oneService.ServiceId, out existingDirectory))
                                {
                                    existingService = existingDirectory.Service;
                                }
                            }

                            if (existingService == null)
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, $"Received Update action for unknown" +
                                        $" Service with service id {oneService.ServiceId}");
                                }
                                break;
                            }
                            else if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                message.AppendLine("Received Update action for RDMService")
                                    .AppendLine($"\tService name {existingService.Info.ServiceName.ToString()}")
                                    .Append($"\tService id {existingService.ServiceId}");

                                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
                            }
                            if ((existingDirectory != null) && existingDirectory.ChannelInfo != chnlInfo)
                            {
                                chnlInfo.DataDictionary = existingDirectory.ChannelInfo!.DataDictionary;
                                existingDirectory.ChannelInfo = chnlInfo;
                            }

                            if (oneService.HasInfo)
                            {
                                ServiceInfo existingInfo = existingService.Info;
                                if (!(existingInfo.ServiceName.Equals(oneService.Info.ServiceName)))
                                {
                                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                    {
                                        StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                        message.AppendLine("Received Update action for RDMService")
                                            .AppendLine($"\tService name {existingInfo.ServiceName.ToString()}")
                                            .AppendLine($"\tService id {existingService.ServiceId}")
                                            .AppendLine($"\tattempting to change service name to {oneService.Info.ServiceName.ToString()}");

                                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                                    }
                                    break;
                                }

                                oneService.Info.Copy(existingInfo);
                            }

                            if (oneService.HasState)
                            {
                                oneService.State.Copy(existingService.State);
                                if (oneService.State.AcceptingRequests == 1 && oneService.State.ServiceStateVal == 1)
                                    m_OmmBaseImpl.DictionaryCallbackClient!.DownloadDictionary(existingDirectory!);
                            }

                            existingService.Action = MapEntryActions.UPDATE;

                            break;
                        }
                    case MapEntryActions.DELETE:
                        {
                            Service? existService = null;
                            if (m_ServiceByIdDict.Count > 0)

                            if (m_ServiceByIdDict.TryGetValue(oneService.ServiceId, out var sourceDirectory))
                            {
                                existService = sourceDirectory.Service;
                            }

                            if (existService == null)
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                    message.Append("Received Delete action for unknown RDMService with service id ")
                                        .Append(oneService.ServiceId);

                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                                }
                                break;
                            }
                            else if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                message.AppendLine("Received Delete action for RDMService")
                                    .AppendLine($"\tService name {existService.Info.ServiceName.ToString()}")
                                    .Append($"\tService id {existService.ServiceId}");

                                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
                            }

                            existService.Action = MapEntryActions.DELETE;
                            break;
                        }
                    default:
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                                message.Append($"Received unknown action for RDMService. Action value {oneService.Action}");

                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                            }
                            break;
                        }
                }
            }
        }

        internal void Initialize()
        {
            // This is only initialized for the consumer, so we can just cast it immediately here.
            DirectoryRequest? directoryRequest = ((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).AdminDirectoryRequest;

            long requestFilter = Eta.Rdm.Directory.ServiceFilterFlags.INFO |
                    Eta.Rdm.Directory.ServiceFilterFlags.STATE |
                    Eta.Rdm.Directory.ServiceFilterFlags.GROUP |
                    Eta.Rdm.Directory.ServiceFilterFlags.LOAD |
                    Eta.Rdm.Directory.ServiceFilterFlags.DATA |
                    Eta.Rdm.Directory.ServiceFilterFlags.LINK;

            if (directoryRequest == null)
            {
                directoryRequest = new();
                directoryRequest.Clear();
                directoryRequest.StreamId = 2;
                directoryRequest.Streaming = true;
                directoryRequest.Filter = requestFilter;
            }
            else
            {
                directoryRequest.StreamId = 2;
            }

            DirectoryRequest = directoryRequest;

            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();


                message.Append("RDMDirectoryRequest message was populated with Filter(s)");
                long filter = directoryRequest.Filter;

                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.INFO) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_INFO_FILTER");
                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.STATE) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_STATE_FILTER");
                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.GROUP) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_GROUP_FILTER");
                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.LOAD) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_LOAD_FILTER");
                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.DATA) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_DATA_FILTER");
                if ((filter & Eta.Rdm.Directory.ServiceFilterFlags.LINK) != 0)
                    message.AppendLine().Append("\tRDM_DIRECTORY_SERVICE_LINK_FILTER");

                if (directoryRequest.HasServiceId)
                    message.AppendLine().Append("\trequesting serviceId ").Append(directoryRequest.ServiceId);
                else
                    message.AppendLine().Append("\trequesting all services");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
            }
        }

        public ServiceDirectory? GetService(string serviceName)
        {
            m_ServiceByNameDict.TryGetValue(serviceName, out ServiceDirectory? service);
            return service;
        }

        public ServiceDirectory? GetService(int serviceId)
        {
            m_ServiceByIdDict.TryGetValue(serviceId, out ServiceDirectory? service);
            return service;
        }
    }

    internal class DirectoryCallbackClientConsumer : DirectoryCallbackClient<IOmmConsumerClient>
    {
        public DirectoryCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
        {
            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;
            EventImpl.SetOmmConsumer(ommConsumerImpl.Consumer);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
            NotifyOnUpdateMsg = NotifyOnUpdateMsgImpl;
            NotifyOnAckMsg = NotifyOnAckMsgImpl;
        }

        public void NotifyOnAllMsgImpl(Msg msg)
        {
            EventImpl.Item!.Client!.OnAllMsg(msg, EventImpl);
        }

        public void NotifyOnRefreshMsgImpl()
        {
            EventImpl.Item!.Client!.OnRefreshMsg(m_RefreshMsg!, EventImpl);
        }

        public void NotifyOnUpdateMsgImpl()
        {
            EventImpl.Item!.Client!.OnUpdateMsg(m_UpdateMsg!, EventImpl);
        }

        public void NotifyOnStatusMsgImpl()
        {
            EventImpl.Item!.Client!.OnStatusMsg(m_StatusMsg!, EventImpl);
        }

        public void NotifyOnGenericMsgImpl()
        {
            EventImpl.Item!.Client!.OnGenericMsg(m_GenericMsg!, EventImpl);
        }

        public void NotifyOnAckMsgImpl()
        {
            EventImpl.Item!.Client!.OnAckMsg(m_AckMsg!, EventImpl);
        }
    }

    internal class DirectoryItem<T> : SingleItem<T>
    {
        private static readonly string CLIENT_NAME = "DirectoryItem";
        public ChannelInfo? ChannelInfo { get; set; }

        public DirectoryItem() : base()
        {
            m_type = ItemType.DIRECTORY_ITEM;
        }

        public DirectoryItem(OmmBaseImpl<T> ommBaseImpl, T client, object? closure) :
            base(ommBaseImpl, client, closure, null)
        {
            m_type = ItemType.DIRECTORY_ITEM;
        }

        public void ResetDirectoryItem(OmmBaseImpl<T> ommBaseImpl, T client, object? closure)
        {
            Reset(ommBaseImpl, client, closure, null);
            ChannelInfo = null;
        }

        public bool OpenDirectoryItem(RequestMsg reqMsg)
        {
            ServiceDirectory? service = null;

            if (reqMsg.HasServiceName)
            {
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceName());

                if (service == null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen || m_OmmBaseImpl.LoginCallbackClient.LoginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service name of '{reqMsg.ServiceName()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.DirectoryCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                                message.ToString(), reqMsg.ServiceName());

                    return true;
                }
            }
            else if (reqMsg.HasServiceId)
            {
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceId());

                if (service == null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen
                || m_OmmBaseImpl.LoginCallbackClient.LoginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service id of '{reqMsg.ServiceId()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.DirectoryCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                      message.ToString(), null);

                    return true;
                }
            }

            m_ServiceDirectory = service;

            string? serviceName = reqMsg.HasServiceName ? reqMsg.ServiceName() : null;

            return Submit(reqMsg.m_requestMsgEncoder.m_rsslMsg, serviceName);
        }

        bool Submit(IRequestMsg requestMsg, string? serviceName)
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            submitOptions.ServiceName = null;

            if (serviceName != null)
            {
                requestMsg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                ServiceName = serviceName;
            }

            if (!requestMsg.CheckHasQos())
            {
                requestMsg.ApplyHasQos();
                requestMsg.ApplyHasWorstQos();
                requestMsg.Qos.IsDynamic = false;
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.WorstQos.Rate(QosRates.TIME_CONFLATED);
                requestMsg.WorstQos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                requestMsg.WorstQos.RateInfo(65535);
            }

            if (((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.MsgKeyInUpdates)

            {
                requestMsg.ApplyMsgKeyInUpdates();
            }

            ReactorChannel reactorChannel = ChannelInfo!.ReactorChannel!;

            submitOptions.ApplyClientChannelConfig(ChannelInfo!.ChannelConfig);
            submitOptions.ServiceName = serviceName;
            submitOptions.RequestMsgOptions.UserSpecObj = this;

            if (StreamId == 0)
            {
                requestMsg.StreamId = m_OmmBaseImpl.ItemCallbackClient!.NextStreamId(0);
                StreamId = requestMsg.StreamId;
                m_OmmBaseImpl.ItemCallbackClient.AddToMap(m_OmmBaseImpl.NextLongId(), this);
            }
            else
            {
                requestMsg.StreamId = StreamId;
            }

            if (DomainType == 0)
                DomainType = requestMsg.DomainType;
            else
                requestMsg.DomainType = DomainType;

            ReactorReturnCode ret;
            if (ReactorReturnCode.SUCCESS > (ret = reactorChannel.Submit((Eta.Codec.Msg)requestMsg, submitOptions, out var ErrorInfo)))
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    Error? error = ErrorInfo?.Error;

                    message.Append("Internal error: ReactorChannel.Submit() failed in DirectoryItem.Submit(IRequestMsg requestMsg, string serviceName)")
                    .AppendLine($"\tChannel {error?.Channel?.GetHashCode()}")
                        .AppendLine($"\tError Id {error?.ErrorId}")
                        .AppendLine($"\tInternal sysError {error?.SysError}")
                        .AppendLine($"\tError Location {ErrorInfo?.Location}")
                        .Append($"\tError Text {error?.Text}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append("Failed to open or modify directory request. Reason: ")
                    .Append(ret)
                    .Append(". Error text: ")
                    .Append(ErrorInfo?.Error.Text);

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ret);

                return false;
            }

            return true;
        }
    }
}
