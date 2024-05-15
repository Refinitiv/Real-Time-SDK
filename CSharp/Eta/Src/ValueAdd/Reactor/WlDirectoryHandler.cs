/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using System.Diagnostics;
using System.Net;
using static LSEG.Eta.Rdm.Directory;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Reactor
{
    sealed internal class WlDirectoryHandler : IWlDirectoryHandler
    {
        const long ALL_FILTERS = ServiceFilterFlags.INFO | ServiceFilterFlags.STATE | ServiceFilterFlags.GROUP |
            ServiceFilterFlags.DATA | ServiceFilterFlags.LINK | ServiceFilterFlags.LOAD;

        const int m_DirectoryStreamId = 2;

        Watchlist m_Watchlist;

        DirectoryMsg m_DirectoryMsg = new DirectoryMsg();

        DirectoryRequest m_DirectoryRequest = new DirectoryRequest();  // Directory Request message sent over the network to ADS/Provider
        DirectoryRefresh m_DirectoryRefresh = new DirectoryRefresh();  // will hold last received Directory Refresh message
        DirectoryRefresh m_DirectoryRefresh2 = new DirectoryRefresh(); // helper Directory Refresh message
        DirectoryUpdate m_DirectoryUpdate = new DirectoryUpdate();     // will hold last received Directory Update message
        DirectoryStatus m_DirectoryStatus = new DirectoryStatus();     // will hold the last Directory Status message received

        ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        Msg m_TempMsg = new Msg();
        IRefreshMsg m_TempRefreshMsg = new Msg();
        IUpdateMsg m_TempUpdateMsg = new Msg();
        IStatusMsg m_StatusMsg = new Msg();

        Buffer m_TempBuffer = new Buffer();

        bool m_RoleDirectoryRequestAdded = false;
        bool m_HasPendingRequest = false;
        bool m_RequestDispatchFlag = false;

        internal WlServiceCache m_ServiceCache = new WlServiceCache();

        public Func<ReactorReturnCode>? OnDirectoryStreamOpen;
        public WlServiceCache ServiceCache { get => m_ServiceCache; set { m_ServiceCache = value; } }
        internal WlStream m_DirectoryStream { get; set; }

        private bool m_ReceivedRefresh = false;

#pragma warning disable CS8618
        /// <summary>
        /// Constructor for Directory handler instance
        /// </summary>
        /// <param name="watchlist">parent <see cref="Watchlist"/> instance</param>
        public WlDirectoryHandler(Watchlist watchlist)
        {
            Debug.Assert(watchlist != null);

            m_Watchlist = watchlist;

            m_DirectoryRequest.Filter = ALL_FILTERS;
            m_DirectoryRequest.Streaming = true;

            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)DomainType.SOURCE;
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.State.Code(StateCodes.NONE);
            m_StatusMsg.State.Text(m_TempBuffer);
        }
#pragma warning restore CS8618

        public void Init()
        {
            m_DirectoryStream = m_Watchlist.StreamManager.DirectoryStream;
        }

        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode ret = m_Watchlist!.Reactor!.SendAndHandleDirectoryMsgCallback(location,
                                                                     m_Watchlist!.ReactorChannel!,
                                                                     null,
                                                                     msg,
                                                                     null,
                                                                     out errorInfo,
                                                                     wlRequest);

            if (ret == ReactorCallbackReturnCode.RAISE)
            {
                return m_Watchlist.Reactor.SendAndHandleDefaultMsgCallback(location,
                                                                           m_Watchlist!.ReactorChannel!,
                                                                           null,
                                                                           msg,
                                                                           wlRequest,
                                                                           out errorInfo);
            }

            return ret == ReactorCallbackReturnCode.SUCCESS ? ReactorReturnCode.SUCCESS : ReactorReturnCode.FAILURE;
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode ret = m_Watchlist!.Reactor!.SendAndHandleDirectoryMsgCallback(location,
                                                                     m_Watchlist!.ReactorChannel!,
                                                                     null,
                                                                     msg,
                                                                     (DirectoryMsg)msgBase,
                                                                     out errorInfo,
                                                                     wlRequest);

            if (ret == ReactorCallbackReturnCode.RAISE)
            {
                return m_Watchlist.Reactor.SendAndHandleDefaultMsgCallback(location,
                                                                           m_Watchlist!.ReactorChannel!,
                                                                           null,
                                                                           msg,
                                                                           wlRequest,
                                                                           out errorInfo);
            }

            return ret == ReactorCallbackReturnCode.SUCCESS ? ReactorReturnCode.SUCCESS : ReactorReturnCode.FAILURE;
        }

        public void ChannelDown()
        {
            DeleteAllServices(true, out _);
        }

        public void ChannelUp(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            return;
        }

        public ReactorReturnCode Dispatch(out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode;
            CodecReturnCode codecReturnCode;
            if (m_DirectoryStream != null && m_HasPendingRequest)
            {
                m_HasPendingRequest = false;
                m_TempMsg.Clear();
                if ((codecReturnCode = m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryRequest, m_TempMsg)) != CodecReturnCode.SUCCESS)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlDictionaryHandler.Dispatch",
                        $"Failure converting from Rdm to codec message, return code: {codecReturnCode.GetAsString()}");
                }
                if ((reactorReturnCode = m_DirectoryStream.SendMessage(m_TempMsg, m_SubmitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return reactorReturnCode;
                }
            }

            if (m_ServiceCache.ServiceList.Count > 0)
            {
                ReactorReturnCode ret;
                m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
                var request = m_DirectoryStream!.UserRequestDlList.Start();
                while (request != null)
                {
                    if (request.ReqState == WlRequest.State.PENDING_REFRESH)
                    {
                        request.ReqState = WlRequest.State.OPEN;
                        if ((ret = SendRefreshMsg(request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                    request = m_DirectoryStream!.UserRequestDlList.Forth();
                }
            }

            m_RequestDispatchFlag = false;

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            Debug.Assert(m_DirectoryStream == wlStream);
            Debug.Assert(msg.StreamId == m_DirectoryRequest.StreamId);

            ReactorReturnCode ret;

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    ret = ReadRefreshMsg(decodeIt, (IRefreshMsg)msg, out errorInfo);
                    break;
                case MsgClasses.STATUS:
                    ret = ReadStatusMsg(decodeIt, (IStatusMsg)msg, out errorInfo);
                    break;
                case MsgClasses.UPDATE:
                    ret = ReadUpdateMsg(decodeIt, msg, out errorInfo);
                    break;
                case MsgClasses.GENERIC:
                    ret = ReadGenericMsg(msg, out errorInfo);
                    break;
                default:
                    ret = Reactor.PopulateErrorInfo(out errorInfo,
                                                    ReactorReturnCode.FAILURE,
                                                    "WlDirectoryHandler.ReadMsg",
                                                    $"Invalid message class ({msg.MsgClass}) received by Watchlist directory handler");
                    break;
            }

            // handle any state transition
            if (ret == ReactorReturnCode.SUCCESS)
            {
                switch (m_DirectoryStream.State.StreamState())
                {
                    case StreamStates.CLOSED_RECOVER:
                        HandleClose(out errorInfo, false);
                        SendDirectoryRequestMessage();
                        break;
                    case StreamStates.CLOSED:
                    case StreamStates.REDIRECTED:
                        HandleClose(out errorInfo);
                        break;
                    case StreamStates.OPEN:
                        if (m_DirectoryStream.State.DataState() == DataStates.OK && msg.MsgClass == MsgClasses.REFRESH)
                        {
                            if (m_Watchlist!.ReactorChannel!.State != ReactorChannelState.READY)
                            {
                                ret = m_Watchlist.Reactor!.SendAndHandleChannelEventCallback("WlDirectoryHandler.ReadMsg",
                                                                                             ReactorChannelEventType.CHANNEL_READY,
                                                                                             m_Watchlist.ReactorChannel, errorInfo);

                                if (ret == ReactorReturnCode.SUCCESS)
                                {
                                    m_Watchlist.ReactorChannel.State = ReactorChannelState.READY;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            else if (ret < ReactorReturnCode.SUCCESS)
            {
                if ((ret = m_Watchlist.Reactor!.Disconnect(m_Watchlist.ReactorChannel!, "WlDirectoryHandler.ReadMsg", out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            return ret;
        }

        public ReactorReturnCode SubmitMsg(WlRequest request, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            switch (msg.MsgClass)
            {
                case MsgClasses.CLOSE:
                    m_DirectoryStream.UserRequestDlList.Remove(request);
                    m_Watchlist!.CloseWlRequest(request);
                    request.Clear();
                    request.ReturnToPool();
                    break;
                case MsgClasses.GENERIC:
                    if (m_DirectoryStream.State.StreamState() == StreamStates.OPEN)
                    {
                        bool resetServiceId = false;

                        // replace service id if message submitted with service name
                        if (submitOptions.ServiceName != null)
                        {
                            if (!((IGenericMsg)msg).CheckHasMsgKey())
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                                                 ReactorReturnCode.INVALID_USAGE,
                                                                 "WlDirectoryHandler.SubmitMsg",
                                                                 "Generic message submitted with service name but no message key.");

                            }

                            if ((ret = m_Watchlist!.ChangeServiceNameToID(msg.MsgKey, submitOptions.ServiceName, out errorInfo)) < ReactorReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            resetServiceId = true;
                        }

                        msg.StreamId = m_DirectoryStreamId;
                        ret = m_DirectoryStream.SendMessage(msg, submitOptions, out errorInfo);

                        if (resetServiceId)
                        {
                            msg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                            msg.MsgKey.ServiceId = 0;
                        }

                        if (ret < ReactorReturnCode.SUCCESS && ret != ReactorReturnCode.NO_BUFFERS)
                        {
                            return ret;
                        }
                    }
                    else
                    {
                        // cannot submit generic message when stream is not open
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                                         ReactorReturnCode.INVALID_USAGE,
                                                         "WlDirectoryHandler.SubmitMsg",
                                                         "Cannot submit GenericMsg when stream not in open state.");
                    }
                    break;
                default:
                    return Reactor.PopulateErrorInfo(out errorInfo,
                                                     ReactorReturnCode.FAILURE,
                                                     "WlDirectoryHandler.SubmitMsg",
                                                     $"Invalid message class {msg.MsgClass} submitted to Watchlist Directory handler");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitRequest(WlRequest request, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            request.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
            request.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

            if (!isReissue)
            {
                m_DirectoryStream.UserRequestDlList.PushBack(request);
                request.WatchlistStream = m_DirectoryStream;
            }

            // Queue request message for assembly and dispatch only if the requestMsg wants a refresh and we have a refresh message
            if (!request.RequestMsg.CheckNoRefresh() && m_ServiceCache.ServiceList.Count != 0)
            {
                if (!m_RequestDispatchFlag)
                {
                    // trigger dispatch only for first add to list, and only if the directory refresh was received
                    m_Watchlist!.Reactor!.SendWatchlistDispatchNowEvent(m_Watchlist!.ReactorChannel!);
                    m_RequestDispatchFlag = true;
                }
            }

            request.ReqState = WlRequest.State.PENDING_REFRESH;

            return ret;
        }

        internal ReactorReturnCode ReadRefreshMsg(DecodeIterator dIter, IRefreshMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            // make sure refresh complete flag is set
            // directory handler doesn't handle multi-part directory refreshes
            if (!msg.CheckRefreshComplete())
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                                                 ReactorReturnCode.FAILURE,
                                                 "WlDirectoryHandler.ReadRefreshMsg",
                                                 "Watchlist doesn't handle multi-part directory refresh.");
            }

            // notify stream that response received if solicited
            if (msg.CheckSolicited())
            {
                m_DirectoryStream.ResponseReceived();
            }

            m_DirectoryRefresh.Clear();
            m_DirectoryRefresh.Decode(dIter, (Msg)msg);

            if (msg.CheckClearCache())
            {
                if (!msg.CheckSolicited())
                {
                    DeleteAllServices(false, out errorInfo); // clears cache and notifies the subscribers that services should be deleted
                } else
                {
                    m_ServiceCache.ClearCache(false);
                }
            }

            // set state from directory refresh
            m_DirectoryRefresh.State.Copy(m_DirectoryStream.State);
            if (m_DirectoryRefresh.State.StreamState() == StreamStates.CLOSED_RECOVER)
            {
                m_DirectoryRefresh.State.StreamState(StreamStates.OPEN);
                m_DirectoryRefresh.State.DataState(DataStates.SUSPECT);
                msg.State.StreamState(StreamStates.OPEN);
                msg.State.DataState(DataStates.SUSPECT);
            }

            if ((ret = m_ServiceCache.ProcessServiceList(m_DirectoryRefresh.ServiceList, out errorInfo)) != ReactorReturnCode.SUCCESS)
            {
                return ret;
            }

            var request = m_DirectoryStream.UserRequestDlList.Start();
            while (request != null)
            {
                if (request.ReqState == WlRequest.State.PENDING_REFRESH)
                {
                    request.ReqState = WlRequest.State.OPEN;

                    if (!m_ReceivedRefresh)
                    {
                        m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
                        if ((ret = SendRefreshMsg(request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            break;
                        }
                    }
                    else
                    {
                        m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                        if ((ret = SendRefreshAsUpdateMsg(request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            break;
                        }
                    }
                }
                else if (request.ReqState == WlRequest.State.OPEN)
                {
                    if (msg.CheckClearCache() && m_ServiceCache.ServiceList.Count == 0)
                    {
                        break; // Update messages with DELETE action have already been sent and there are no services in the list, so do nothing
                    }
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                    if ((ret = SendRefreshAsUpdateMsg(request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        break;
                    }
                }
                request = m_DirectoryStream.UserRequestDlList.Forth();
            }

            m_ReceivedRefresh = true;

            return ret;
        }

        internal ReactorReturnCode ReadStatusMsg(DecodeIterator dIter, IStatusMsg msg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            CodecReturnCode codecReturnCode;
            if ((codecReturnCode = m_DirectoryStatus.Decode(dIter, (Msg)msg)) < CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.FAILURE,
                    "WlDirectoryHandler.ReadStatusMsg",
                    $"Failed to decode status message, return code: {codecReturnCode.GetAsString()}");
            }

            // notify stream that response received
            m_DirectoryStream.ResponseReceived();

            if (msg.CheckClearCache())
            {
                // if stream state is OPEN, notify item handler all services deleted
                if (m_DirectoryStatus.State.StreamState() == StreamStates.OPEN)
                {
                    DeleteAllServices(false, out errorInfo);
                } else
                {
                    m_ServiceCache.ClearCache(false); // clear cache anyway
                }
            }

            if (m_DirectoryStatus.HasState)
            {
                m_DirectoryStatus.State.Copy(m_DirectoryStream.State);
            }

            return ReactorReturnCode.SUCCESS;
        }

        internal ReactorReturnCode ReadUpdateMsg(DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
            m_DirectoryMsg.Clear();

            m_DirectoryUpdate.Clear();
            CodecReturnCode codecReturnCode = m_DirectoryUpdate.Decode(dIter, (Msg)msg);

            if (codecReturnCode < CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.FAILURE,
                    "WlDictionaryHandler.ReadUpdateMsg",
                    $"Failed to decode incoming update message, return code : {codecReturnCode.GetAsString()}");
            }

            if ((ret = m_ServiceCache.ProcessServiceList(m_DirectoryUpdate.ServiceList, out errorInfo)) < ReactorReturnCode.SUCCESS)
            {
                return ret;
            }

            var request = m_DirectoryStream.UserRequestDlList.Start();
            while (request != null)
            {
                if ((ret = SendUpdateMsg(request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
                request = m_DirectoryStream.UserRequestDlList.Forth();
            }

            errorInfo = null;
            return ret;
        }

        internal ReactorReturnCode ReadGenericMsg(IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;

            var request = m_DirectoryStream.UserRequestDlList.Start();
            while (request != null)
            {
                msg.StreamId = request.RequestMsg.StreamId;
                if ((ret = m_Watchlist!.Reactor!.SendAndHandleDefaultMsgCallback("WlDirectoryHandler.ReadGenericMsg",
                    m_Watchlist!.ReactorChannel!,
                    null,
                    msg,
                    request,
                    out errorInfo))
                    < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
                request = m_DirectoryStream.UserRequestDlList.Forth();
            }

            return ret;
        }

        internal ReactorReturnCode SendRefreshMsg(WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            m_DirectoryMsg.Clear();

            FillDirectoryRefreshFromRequestMsg(m_DirectoryMsg!.DirectoryRefresh!, wlRequest.RequestMsg);
            m_ServiceCache.FillDirectoryRefreshServiceListFromCache(m_DirectoryMsg!.DirectoryRefresh!, wlRequest!.WatchlistStreamInfo!.ServiceName!);

            m_TempRefreshMsg.Clear();
            m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryMsg!.DirectoryRefresh!, (Msg)m_TempRefreshMsg);

            // use filter from user request
            long returnFilter = GetResultingFilter(wlRequest.RequestMsg.MsgKey.Filter, m_DirectoryMsg!.DirectoryRefresh!.Filter);
            if (m_TempRefreshMsg.CheckHasMsgKey())
                m_TempRefreshMsg.MsgKey.Filter = returnFilter;
            m_DirectoryMsg!.DirectoryRefresh!.Filter = returnFilter;

            // callback user
            ret = CallbackUserWithMsgBase("WlDirectoryHandler.FanoutRefreshMsg",
                m_TempRefreshMsg,
                m_DirectoryMsg!,
                wlRequest,
                out errorInfo);

            m_ServiceCache.ReturnServicesToPool(m_DirectoryMsg!.DirectoryRefresh!.ServiceList);

            return ret;
        }

        internal ReactorReturnCode SendRefreshAsUpdateMsg(WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            m_DirectoryMsg.Clear();
            m_DirectoryRefresh2.Clear();

            FillDirectoryRefreshFromRequestMsg(m_DirectoryRefresh2, wlRequest.RequestMsg);
            m_ServiceCache.FillDirectoryRefreshServiceListFromCache(m_DirectoryRefresh2, wlRequest!.WatchlistStreamInfo!.ServiceName!);

            if (m_DirectoryRefresh2.ServiceList.Count == 0) // no updated services for this user request, don't send empty update
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            m_DirectoryMsg!.DirectoryUpdate!.ServiceList.AddRange(m_DirectoryRefresh2.ServiceList);
            m_DirectoryMsg!.DirectoryUpdate!.HasFilter = true;
            m_DirectoryMsg!.DirectoryUpdate!.Filter = m_DirectoryRefresh2.Filter;
            m_DirectoryMsg!.StreamId = m_DirectoryRefresh2.StreamId;
            m_DirectoryMsg!.DirectoryUpdate!.HasServiceId = true;
            m_DirectoryMsg!.DirectoryUpdate!.ServiceId = m_DirectoryRefresh2.ServiceId;
            m_DirectoryMsg!.Flags = (int)m_DirectoryRefresh2.Flags;

            m_TempUpdateMsg.Clear();
            m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryMsg!.DirectoryUpdate!, (Msg)m_TempUpdateMsg);

            // use filter from user request
            long returnFilter = GetResultingFilter(wlRequest.RequestMsg.MsgKey.Filter, m_DirectoryRefresh2.Filter);
            if (m_TempUpdateMsg.CheckHasMsgKey())
                m_TempUpdateMsg.MsgKey.Filter = returnFilter;
            m_DirectoryMsg!.DirectoryUpdate!.Filter = returnFilter;


            // callback user
            ret = CallbackUserWithMsgBase("WlDirectoryHandler.FanoutRefreshAsUpdateMsg",
                m_TempUpdateMsg,
                m_DirectoryMsg!,
                wlRequest,
                out errorInfo);

            m_ServiceCache.ReturnServicesToPool(m_DirectoryRefresh2.ServiceList);

            return ret;
        }

        internal ReactorReturnCode SendUpdateMsg(WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;
            m_DirectoryMsg.Clear();

            FillDirectoryUpdateFromRequestMsg(m_DirectoryMsg!.DirectoryUpdate!, wlRequest);
            m_ServiceCache.FillDirectoryUpdateServiceListFromUpdateMsgServices(m_DirectoryMsg!.DirectoryUpdate!, m_DirectoryUpdate.ServiceList);

            // fanout only if we have a service in the directoryUpdate
            if (m_DirectoryMsg!.DirectoryUpdate!.ServiceList.Count > 0)
            {
                long returnFilter = GetResultingFilter(m_DirectoryMsg!.DirectoryUpdate!.Filter, m_DirectoryUpdate.Filter);
                m_DirectoryMsg!.DirectoryUpdate!.Filter = returnFilter;

                m_TempUpdateMsg.Clear();
                m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryMsg!.DirectoryUpdate!, (Msg)m_TempUpdateMsg);

                // callback user
                ret = CallbackUserWithMsgBase("WlDirectoryHandler.ReadUpdateMsg",
                                                m_TempUpdateMsg,
                                                m_DirectoryMsg,
                                                wlRequest,
                                                out errorInfo);

                m_ServiceCache.ReturnServicesToPool(m_DirectoryMsg!.DirectoryUpdate!.ServiceList);
            }

            return ret;
        }

        public int ServiceId(string serviceName)
        {
            return m_ServiceCache.ServiceId(serviceName);
        }

        internal void FillDirectoryRefreshFromRequestMsg(DirectoryRefresh directoryRefresh, IRequestMsg requestMsg)
        {
            directoryRefresh.StreamId = requestMsg.StreamId;
            if (requestMsg.MsgKey.CheckHasFilter())
                directoryRefresh.Filter = requestMsg.MsgKey.Filter;
            else
                directoryRefresh.Filter = m_DirectoryRefresh.Filter;

            directoryRefresh.Solicited = true;
            directoryRefresh.State.DataState(m_DirectoryRefresh.State.DataState());
            directoryRefresh.State.StreamState(m_DirectoryRefresh.State.StreamState());

            if (requestMsg.MsgKey.CheckHasServiceId())
            {
                directoryRefresh.HasServiceId = true;
                directoryRefresh.ServiceId = requestMsg.MsgKey.ServiceId;
            }
        }

        internal void FillDirectoryUpdateFromRequestMsg(DirectoryUpdate directoryUpdate, WlRequest wlRequest)
        {
            directoryUpdate.StreamId = wlRequest.RequestMsg.StreamId;
            directoryUpdate.HasFilter = true;
            if (wlRequest.RequestMsg.MsgKey.CheckHasFilter())
            {
                long filter = wlRequest.RequestMsg.MsgKey.Filter;
                directoryUpdate.Filter = filter == 0 ? ALL_FILTERS : filter;
            }
            else
            {
                directoryUpdate.Filter = m_DirectoryUpdate.Filter != 0 ? m_DirectoryUpdate.Filter : ALL_FILTERS;
            }

            if (wlRequest.RequestMsg.MsgKey.CheckHasServiceId())
            {
                directoryUpdate.HasServiceId = true;
                directoryUpdate.ServiceId = wlRequest.RequestMsg.MsgKey.ServiceId;
            }
            else if (wlRequest.WatchlistStreamInfo.ServiceName != null)
            {
                int serviceId = m_ServiceCache.ServiceId(wlRequest.WatchlistStreamInfo.ServiceName);
                if (serviceId >= 0)
                {
                    directoryUpdate.HasServiceId = true;
                    directoryUpdate.ServiceId = serviceId;
                }
            }
        }

        internal ReactorReturnCode FanoutStatus()
        {
            ReactorReturnCode reactorReturnCode = ReactorReturnCode.SUCCESS;
            m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.STATUS;
            m_DirectoryMsg.Clear();
            // set state to closed recover if current state isn't closed
            if (m_DirectoryStream.State.StreamState() != StreamStates.CLOSED)
            {
                // call back user with directory status of OPEN/SUSPECT
                m_StatusMsg.State.StreamState(StreamStates.OPEN);
                m_StatusMsg.State.DataState(DataStates.SUSPECT);

                m_DirectoryMsg.DirectoryStatus!.State.StreamState(StreamStates.OPEN);
                m_DirectoryMsg.DirectoryStatus!.State.DataState(DataStates.SUSPECT);
            }
            else // closed, call back user with directory status of CLOSED/SUSPECT
            {
                m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                m_StatusMsg.State.DataState(DataStates.SUSPECT);

                m_DirectoryMsg.DirectoryStatus!.State.StreamState(StreamStates.CLOSED);
                m_DirectoryMsg.DirectoryStatus!.State.DataState(DataStates.SUSPECT);
            }

            if (m_DirectoryStream.UserRequestDlList.Count() > 0)
            {
                // fanout status message to user requests associated with the stream
                var request = m_DirectoryStream.UserRequestDlList.Start();
                while (request != null)
                {
                    if ((reactorReturnCode = SendStatusMessageForUserRequest(request)) < ReactorReturnCode.SUCCESS)
                    {
                        return reactorReturnCode;
                    }
                    request = m_DirectoryStream.UserRequestDlList.Forth();
                }
            }

            return reactorReturnCode;
        }

        internal ReactorReturnCode SendStatusMessageForUserRequest(WlRequest wlRequest)
        {
            // Set streamId and filter to that of the userRequest
            m_StatusMsg.StreamId = wlRequest.RequestMsg.StreamId;
            m_DirectoryMsg.DirectoryStatus!.StreamId = wlRequest.RequestMsg.StreamId;

            // use filter from user request
            m_StatusMsg.ApplyHasMsgKey();
            m_StatusMsg.MsgKey.Filter = wlRequest.RequestMsg.MsgKey.Filter;
            m_DirectoryMsg.DirectoryStatus!.HasFilter = true;
            m_DirectoryMsg.DirectoryStatus!.Filter = wlRequest.RequestMsg.MsgKey.Filter;

            return CallbackUserWithMsgBase("WlDirectoryHandler.SendStatusMessageForUserRequest",
                    (Msg)m_StatusMsg,
                    m_DirectoryMsg,
                    wlRequest,
                    out _);
        }

        public void DeleteAllServices(bool isChannelDown, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            Service rdmService;

            m_DirectoryStream.ChannelDown();
            if (m_DirectoryStream.State.StreamState() == StreamStates.OPEN)
            {
                if (isChannelDown)
                {
                    m_DirectoryStream.m_stateTextBuffer.Clear();
                    m_DirectoryStream.State.Clear();
                    m_DirectoryStream.State.Text().Data(m_DirectoryStream.m_stateTextBuffer);
                    m_DirectoryStream.State.StreamState(StreamStates.CLOSED_RECOVER);
                    m_DirectoryStream.State.DataState(DataStates.SUSPECT);
                    m_DirectoryStream.InUse = false;
                }

                m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                m_DirectoryMsg.Clear();

                var request = m_DirectoryStream.UserRequestDlList.Start();
                while (request != null)
                {
                    if (isChannelDown)
                    {
                        request.ReqState = WlRequest.State.PENDING_REFRESH;
                    }
                    m_DirectoryMsg.StreamId = request.RequestMsg.StreamId;
                    foreach (var service in m_ServiceCache.ServiceList)
                    {
                        // Only add services that the user requested (or all services if zero) to the serviceList of the update
                        if (service!.RdmService!.ServiceId == request.RequestMsg.MsgKey.ServiceId || request.RequestMsg.MsgKey.ServiceId == 0)
                        {
                            rdmService = m_ServiceCache.GetRdmServiceFormPool();
                            service!.RdmService!.Copy(rdmService);
                            rdmService.Action = MapEntryActions.DELETE;

                            m_DirectoryMsg!.DirectoryUpdate!.ServiceList.Add(rdmService);
                        }
                    }

                    m_TempUpdateMsg.Clear();
                    m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryMsg!.DirectoryUpdate!, (Msg)m_TempUpdateMsg);
                    long returnFilter = GetResultingFilter(request.RequestMsg.MsgKey.Filter, m_DirectoryMsg!.DirectoryUpdate!.Filter);
                    if (m_TempUpdateMsg.CheckHasMsgKey())
                        m_TempUpdateMsg.MsgKey.Filter = returnFilter;
                    if (m_DirectoryMsg!.DirectoryUpdate!.HasFilter)
                        m_DirectoryMsg!.DirectoryUpdate!.Filter = returnFilter;

                    if (CallbackUserWithMsgBase("WlDirectoryHandler.DeleteAllServices",
                        (Msg)m_TempUpdateMsg,
                        m_DirectoryMsg,
                        m_Watchlist!.StreamIdToWlRequestDict![m_TempUpdateMsg.StreamId],
                        out errorInfo)
                        < ReactorReturnCode.SUCCESS)
                    {
                        break;
                    }

                    m_DirectoryMsg.Clear();
                    request = m_DirectoryStream.UserRequestDlList.Forth();
                }

                m_ServiceCache.ReturnServicesToPool(m_DirectoryMsg!.DirectoryUpdate!.ServiceList);
                m_ServiceCache.ClearCache(isChannelDown);
            }
        }

        internal void HandleClose(out ReactorErrorInfo? errorInfo, bool closeStream = true)
        {
            errorInfo = null;
            if (closeStream)
                CloseDirectoryStream();

            m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
            m_DirectoryMsg.Clear();
            var request = m_DirectoryStream.UserRequestDlList.Start();
            while (request != null)
            {
                if (SendActionDeleteUpdateMsg(request, out errorInfo) < ReactorReturnCode.SUCCESS)
                {
                    return;
                }
                request = m_DirectoryStream.UserRequestDlList.Forth();
            }

            m_ServiceCache.ClearCache(false);
        }

        private ReactorReturnCode SendActionDeleteUpdateMsg(WlRequest request, out ReactorErrorInfo? errorInfo)
        {
            m_DirectoryMsg!.DirectoryUpdate!.StreamId = request.RequestMsg.StreamId;

            if (request.RequestMsg.MsgKey.ServiceId == 0)
            {
                foreach (var service in m_ServiceCache.ServiceList)
                {
                    var s = m_ServiceCache.GetRdmServiceFormPool();
                    service!.RdmService!.Copy(s);
                    s.Action = MapEntryActions.DELETE;
                    m_DirectoryMsg!.DirectoryUpdate!.ServiceList.Add(s);
                }
            }
            else
            {
                foreach (var service in m_ServiceCache.ServiceList)
                {
                    if (request.RequestMsg.MsgKey.ServiceId == service!.RdmService!.ServiceId)
                    {
                        var s = m_ServiceCache.GetRdmServiceFormPool();
                        service!.RdmService!.Copy(s);
                        s.Action = MapEntryActions.DELETE;
                        m_DirectoryMsg!.DirectoryUpdate!.ServiceList.Add(s);
                    }
                }
            }

            m_TempUpdateMsg.Clear();
            m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryMsg!.DirectoryUpdate!, (Msg)m_TempUpdateMsg);

            // use filter from user request
            if (m_DirectoryMsg!.DirectoryUpdate!.HasFilter)
            {
                long returnFilter = GetResultingFilter(request.RequestMsg.MsgKey.Filter, m_DirectoryMsg!.DirectoryUpdate!.Filter);
                m_TempUpdateMsg.MsgKey.Filter = returnFilter;
                m_DirectoryMsg!.DirectoryUpdate!.Filter = returnFilter;
            }

            var ret = CallbackUserWithMsgBase("WlDirectoryHandler.HandleClose",
                                            (Msg)m_TempUpdateMsg,
                                            m_DirectoryMsg,
                                            m_Watchlist!.StreamIdToWlRequestDict![m_TempUpdateMsg.StreamId],
                                            out errorInfo);

            m_ServiceCache.ReturnServicesToPool(m_DirectoryMsg!.DirectoryUpdate!.ServiceList);

            return ret;
        }

        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo? errorInfo)
        {
            m_DirectoryStream.WlHandler = this;
            m_DirectoryStream.Watchlist = m_Watchlist;
            m_DirectoryStream.StreamDomainType = (int)DomainType.SOURCE;
            m_DirectoryStream.InUse = true;

            m_DirectoryRequest.StreamId = m_DirectoryStream.StreamId;

            if (m_Watchlist!.ConsumerRole!.RdmDirectoryRequest != null
                && !m_RoleDirectoryRequestAdded
                && !m_Watchlist!.StreamIdToWlRequestDict!.ContainsKey(m_Watchlist!.ConsumerRole!.RdmDirectoryRequest.StreamId))
            {
                // User has enabled InitDefaultRDMDirectoryRequest
                WlRequest wlRequest = m_Watchlist.CreateWlRequest();
                m_TempMsg.Clear();
                m_Watchlist.ConvertRDMToCodecMsg(m_Watchlist!.ConsumerRole!.RdmDirectoryRequest, (Msg)wlRequest.RequestMsg);
                m_Watchlist.StreamIdToWlRequestDict.Add(m_Watchlist!.ConsumerRole!.RdmDirectoryRequest.StreamId, wlRequest);
                wlRequest.ReqState = WlRequest.State.PENDING_REFRESH;
                m_DirectoryStream.UserRequestDlList.PushBack(wlRequest);
                wlRequest.WatchlistStream = m_DirectoryStream;
                m_RoleDirectoryRequestAdded = true;
            }

            // send directory request for all services and filters
            if (m_DirectoryStream.State.StreamState() != StreamStates.OPEN
                && !m_DirectoryStream.RequestPending)
            {
                m_TempMsg.Clear();
                m_Watchlist.ConvertRDMToCodecMsg(m_DirectoryRequest, m_TempMsg);
                ReactorReturnCode ret;

                if ((ret = m_DirectoryStream.SendMessage(m_TempMsg, m_SubmitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode LoginStreamClosed()
        {
            m_DirectoryStream.ResponseReceived();
            if (m_DirectoryStream.State.StreamState() == StreamStates.OPEN)
            {
                if (m_Watchlist!.LoginHandler.Stream!.State.StreamState() == StreamStates.CLOSED_RECOVER
                    || m_Watchlist!.ReactorChannel!.EnableSessionManagement())
                {
                    // login stream in close recover state
                    m_DirectoryStream.m_stateTextBuffer.Clear();
                    m_DirectoryStream.State.Clear();
                    m_DirectoryStream.State.Text().Data(m_DirectoryStream.m_stateTextBuffer);
                    m_DirectoryStream.State.StreamState(StreamStates.CLOSED_RECOVER);
                    m_DirectoryStream.State.DataState(DataStates.SUSPECT);

                    // clear service cache
                    m_ServiceCache.ClearCache(false);
                }

                // close stream if login stream is closed
                else if (m_Watchlist.LoginHandler.Stream.State.StreamState() == StreamStates.CLOSED)
                {
                    // delete all services and close directory stream
                    DeleteAllServices(false, out _);
                    CloseDirectoryStream();
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        private void CloseDirectoryStream()
        {
            // Close directory stream (but don't repool it so that we keep the application's requests)
            m_DirectoryStream.m_stateTextBuffer.Clear();
            m_DirectoryStream.State.Clear();
            m_DirectoryStream.State.Text().Data(m_DirectoryStream.m_stateTextBuffer);
            m_DirectoryStream.State.StreamState(StreamStates.CLOSED);
            m_DirectoryStream.State.DataState(DataStates.SUSPECT);
            m_DirectoryStream.InUse = false;
        }

        private long GetResultingFilter(long userFilter, long responseFilter)
        {
            return responseFilter != 0 ? userFilter & responseFilter : userFilter;
        }

        public void AddPendingRequest(WlStream wlStream)
        {
            m_HasPendingRequest = true;
        }

        public void Clear()
        {
            var request = m_DirectoryStream.UserRequestDlList.Pop();
            while (request != null)
            {
                request.FreeWlRequest();
                request = m_DirectoryStream.UserRequestDlList.Pop();
            }

            // this handler is still associated with same watchlist so don't set watchlist to null
            m_DirectoryStream.ClearWlStream();
            m_DirectoryRefresh.Clear();
            m_DirectoryRefresh2.Clear();
            m_DirectoryStatus.Clear();
            m_DirectoryStatus.HasState = true;
            m_DirectoryStatus.State.Code(StateCodes.NONE);
            m_DirectoryStatus.State.Text(m_TempBuffer);
            m_DirectoryUpdate.Clear();
            m_SubmitOptions.Clear();
            m_TempMsg.Clear();
            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)DomainType.SOURCE;
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.State.Code(StateCodes.NONE);
            m_StatusMsg.State.Text(m_TempBuffer);
            m_TempBuffer.Clear();
            m_TempBuffer.Data("");
            m_TempMsg.Clear();
            m_ServiceCache.Clear();
            m_TempRefreshMsg.Clear();
            m_TempUpdateMsg.Clear();
            m_RequestDispatchFlag = false;
            m_RoleDirectoryRequestAdded = false;
            m_HasPendingRequest = false;
            m_ReceivedRefresh = false;
        }

        public void OnRequestTimeout(WlStream stream)
        {
            FanoutStatus();
            SendDirectoryRequestMessage();
        }

        private void SendDirectoryRequestMessage()
        {
            m_TempMsg.Clear();
            m_Watchlist!.ConvertRDMToCodecMsg(m_DirectoryRequest, m_TempMsg);
            m_DirectoryStream.SendMessage(m_TempMsg, m_SubmitOptions, out _);
        }
    }
}
