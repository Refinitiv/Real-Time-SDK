/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using System.Diagnostics;
using Buffer = LSEG.Eta.Codec.Buffer;
using Array = LSEG.Eta.Codec.Array;
using LSEG.Eta.ValueAdd.Common;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    sealed internal class WlItemHandler : IWlItemHandler
    {
        #region delegate definition

        private delegate ReactorReturnCode SubmitBatchRequestHandler(ReactorSubmitOptions submitOptions, IDictionary<int, WlItemRequest> wlItemRequestDict,
            IDictionary<int, IRequestMsg> streamIdToReqMsgDict, int currentStreamId, out ReactorErrorInfo? errorInfo);

        #endregion

        Watchlist m_Watchlist;
        Qos m_DefaultQos = new ();
        Qos m_TempMatchedQos = new ();
        Reactor m_reactor;
        WlStreamAttributes m_TempWlStreamAttributes = new ();
        WlItemGroupPool m_itemGroupPool = new WlItemGroupPool(new int[] { 1, 0, 0, 0, 0, 0 }, 4);

        /// <summary>
        /// Non private stream message copy flags
        /// </summary>
        const int DEFAULT_COPY_FLAGS = CopyMsgFlags.ALL_FLAGS & ~CopyMsgFlags.EXTENDED_HEADER & ~CopyMsgFlags.DATA_BODY;

        /// <summary>
        /// Private stream copy flags
        /// </summary>
        const int PRIVATE_STREAM_COPY_FLAGS = CopyMsgFlags.ALL_FLAGS;

        Dictionary<WlStreamAttributes, WlItemStream>? WlStreamAttribToWlStreamDict;

        IRequestMsg m_TempItemAggregationRequest = new Msg();

        IStatusMsg m_StatusMsg = new Msg();

        ICloseMsg m_CloseMsg = new Msg();

        internal WlItemStreamList PendingSendMsgDlList = new ();
        internal WlItemStreamList StreamDlList = new ();

        LinkedList<IStatusMsg> StatusMsgDispatchList = new ();

        LinkedList<int> UserStreamIdListToRecover = new ();

        // List of user requests to re-submit upon dispatch that had request timeout
        readonly VaDoubleLinkList<WlItemRequest> RequestTimeoutDlList = new();

        // used for requests that are submitted when directory stream is not up
        // two tables are required - one is indexed by service id and one is indexed by service name
        Dictionary<int, LinkedList<WlItemRequest>> PendingRequestByIdDict = new ();
        Dictionary<string, LinkedList<WlItemRequest>> PendingRequestByNameDict = new ();
        
        // pool of pending request lists (to avoid GC)
        LinkedList<LinkedList<WlItemRequest>> PendingRequestListPool = new (); 

        // pool of IStatusMsg
        LinkedList<IStatusMsg> StatusMsgPool = new();

        private WlItemStream? m_CurrentFanoutStream;
        private State? m_CurrentMsgState;
        private ReactorSubmitOptions m_SubmitOptions = new ();

        // RDM dictionary message for callback
        DictionaryMsg m_RdmDictionaryMsg = new ();

        DecodeIterator m_DecodeIt = new ();
        DecodeIterator m_BatchDecodeIt = new();

        ElementList m_ElementList = new();
        ElementList m_BehaviourElementList = new();
        UInt m_DataStreamFlag = new();
        ElementEntry m_ElementEntry = new();
        ElementEntry m_BehaviourEntry = new();

        Int m_FieldId = new();

        #region View request handling
        bool m_HasViewType;
        UInt m_ViewType = new ();
        Buffer m_ViewDataElement = new();
        Buffer m_ElementName = new();
        Buffer m_ViewElementList = new();
        Array m_ViewArray = new();
        ArrayEntry m_ViewArrayEntry = new();
        WlViewHandler m_ViewHandler;
        bool m_SnapshotViewClosed;
        bool m_HasPendingViewRequest = false;
        #endregion

        // table that maps item provider request aggregation key to application
        // requests for symbol list data stream
        Dictionary<WlStreamAttributes, IRequestMsg> ProviderRequestDict = new ();
        IRequestMsg m_RequestMsg = new Msg();
        WlStreamAttributes m_SymbolListRequestKey = new();
        Map m_Map = new();
        MapEntry m_MapEntry = new ();
        Buffer m_MapKey = new Buffer();

        public enum ViewAction
        {
            SET = 1,
            MAINTAIN = 2,
            NONE = 3
        }

        public WlItemHandler(Watchlist watchlist)
        {
            m_Watchlist = watchlist;
            m_DefaultQos.Clear();
            m_DefaultQos.Timeliness(QosTimeliness.REALTIME);
            m_DefaultQos.Rate(QosRates.TICK_BY_TICK);
            m_reactor = watchlist.Reactor!;
            m_CurrentFanoutStream = null;
            m_CurrentMsgState = null;
            m_ViewHandler = new (watchlist);
        }

        public void Init(ConsumerRole role)
        {
            if (role.WatchlistOptions.ItemCountHint > 0)
            {
                WlStreamAttribToWlStreamDict = new((int)(1.5 * role.WatchlistOptions.ItemCountHint));
            }
            else
            {
                WlStreamAttribToWlStreamDict = new();
            }

            m_reactor = m_Watchlist.Reactor!;

            m_Watchlist.DirectoryHandler.ServiceCache.ServiceAddedCallback = ServiceAddedCallback;
            m_Watchlist.DirectoryHandler.ServiceCache.ServiceRemovedCallback = ServiceRemovedCallback;
            m_Watchlist.DirectoryHandler.ServiceCache.ServiceUpdatedCallback = ServiceUpdatedCallback;
        }

        public void ServiceAddedCallback(WlService wlService)
        {
            ServiceAdded(wlService, out ReactorErrorInfo? errorInfo);
        }

        public void ServiceRemovedCallback(WlService wlService, bool isChannelDown)
        {
            ServiceDeleted(wlService, isChannelDown, out ReactorErrorInfo? errorInfo);
        }

        public void ServiceUpdatedCallback(WlService wlService)
        {
            ServiceUpdated(wlService, wlService.RdmService!.HasState, out ReactorErrorInfo? errorInfo);
        }

        public void Clear()
        {
            if (WlStreamAttribToWlStreamDict != null)
            {
                WlStreamAttribToWlStreamDict.Clear();
            }

            PendingRequestByIdDict.Clear();
            PendingRequestByNameDict.Clear();
            StatusMsgDispatchList.Clear();
            if (RequestTimeoutDlList.Count() > 0)
            {
                while (RequestTimeoutDlList.Pop(WlItemRequest.TIMEOUT_ITEM_REQUEST_LINK) != null) { }
            }
            m_CurrentFanoutStream = null;
            m_CurrentMsgState = null;
            m_HasPendingViewRequest = false;
            m_ViewHandler.Clear();
#pragma warning disable CS8625 // Cannot convert null literal to non-nullable reference type.
            m_Watchlist = null;
#pragma warning restore CS8625 // Cannot convert null literal to non-nullable reference type.
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public bool IsRequestRecoverable(WlRequest wlRequest, int streamState)
        {
            return
                // A request is recoverable if:
                // - Request is not for a private stream.
                !wlRequest.RequestMsg.CheckPrivateStream()

                // - and not a dictionary request, or is a dictionary request but didn't get full dictionary yet.
                && (wlRequest.RequestMsg.DomainType != (int)DomainType.DICTIONARY || wlRequest.ReqState != WlRequest.State.OPEN)
                
                // - and the StreamState is CLOSED_RECOVER and SingleOpen is enabled.
                && m_Watchlist.LoginHandler!.SupportSingleOpen && streamState == StreamStates.CLOSED_RECOVER;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        void CopyRequestKeyReferencesToMsg(WlItemRequest wlRequest, IMsg destMsg)
        {
            IMsgKey destKey = destMsg.MsgKey;
            IMsgKey srcKey = wlRequest.RequestMsg.MsgKey;

            destKey.Flags = srcKey.Flags;
            destKey.NameType = srcKey.NameType;
            destKey.Name = srcKey.Name;
            destKey.Filter = srcKey.Filter;
            destKey.Identifier = srcKey.Identifier;
            destKey.AttribContainerType = srcKey.AttribContainerType;
            destKey.EncodedAttrib = srcKey.EncodedAttrib;

            if (wlRequest.HasServiceId)
            {
                /* Request may have requested its service by name but we know the ID, so set it */
                destKey.ApplyHasServiceId();
                destKey.ServiceId = (int)wlRequest.ServiceId;
            }
            else  /* Request may have requested its service by ID. */
                destKey.ServiceId = srcKey.ServiceId;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            int msgFlagsToReset = 0;
            WlItemRequest? wlItemRequest = wlRequest as WlItemRequest;

            /* Check if we need to add a MsgKey to the message. */
            if (wlItemRequest != null 
                && (wlItemRequest.RequestMsg.CheckMsgKeyInUpdates() /* MsgKey requested in responses. */
                        || ((wlItemRequest.ItemReqFlags & WlItemRequest.Flags.PROV_DRIVEN) != 0) 
                        && !wlItemRequest.InitialResponseReceived /* Initial response to a provider-driven stream */))
            {
                /* Message needs to have a key. If it does not, copy it from the request. */
                switch (msg.MsgClass)
                {
                    case MsgClasses.UPDATE:
                        if (!((IUpdateMsg)msg).CheckHasMsgKey())
                        {
                            ((IUpdateMsg)msg).ApplyHasMsgKey();
                            CopyRequestKeyReferencesToMsg(wlItemRequest, msg);
                            msgFlagsToReset = UpdateMsgFlags.HAS_MSG_KEY;
                        }
                        break;

                    case MsgClasses.REFRESH:
                        IRefreshMsg refreshMsg = (IRefreshMsg)msg;
                        if (!refreshMsg.CheckHasMsgKey())
                        {
                            refreshMsg.ApplyHasMsgKey();
                            CopyRequestKeyReferencesToMsg(wlItemRequest, msg);
                            msgFlagsToReset = RefreshMsgFlags.HAS_MSG_KEY;
                        }

                        if (refreshMsg.State.IsFinal())
                            wlItemRequest.UnsetServiceId();
                        break;

                    case MsgClasses.STATUS:
                        if (!((IStatusMsg)msg).CheckHasMsgKey())
                        {
                            ((IStatusMsg)msg).ApplyHasMsgKey();
                            CopyRequestKeyReferencesToMsg(wlItemRequest, msg);
                            msgFlagsToReset = StatusMsgFlags.HAS_MSG_KEY;
                        }

                        if (((IStatusMsg)msg).CheckHasState() && ((IStatusMsg)msg).State.IsFinal())
                            wlItemRequest.UnsetServiceId();
                        break;

                    case MsgClasses.GENERIC:
                        if (!((IGenericMsg)msg).CheckHasMsgKey())
                        {
                            ((IGenericMsg)msg).ApplyHasMsgKey();
                            CopyRequestKeyReferencesToMsg(wlItemRequest, msg);
                            msgFlagsToReset = GenericMsgFlags.HAS_MSG_KEY;
                        }
                        break;

                    case MsgClasses.ACK:
                        if (!((IAckMsg)msg).CheckHasMsgKey())
                        {
                            ((IAckMsg)msg).ApplyHasMsgKey();
                            CopyRequestKeyReferencesToMsg(wlItemRequest, msg);
                            msgFlagsToReset = AckMsgFlags.HAS_MSG_KEY;
                        }
                        break;

                    default:
                        /* Do nothing for classes that do not have MsgKeys. */
                        break;
                }

                wlItemRequest.InitialResponseReceived = true;
            }

            if (msg.DomainType != (int)DomainType.DICTIONARY)
            {
                ret = m_Watchlist.Reactor!.SendAndHandleDefaultMsgCallback(location,
                                                                           m_Watchlist.ReactorChannel!,
                                                                           null!,
                                                                           msg,
                                                                           wlRequest,
                                                                           out errorInfo);
            }
            else // dictionary domain
            {
                /* Decode to a DictionaryMsg. */
                m_RdmDictionaryMsg.Clear();
                switch (msg.MsgClass)
                {
                    case MsgClasses.REFRESH:
                        m_RdmDictionaryMsg.DictionaryMsgType = DictionaryMsgType.REFRESH;
                        break;
                    case MsgClasses.STATUS:
                        m_RdmDictionaryMsg.DictionaryMsgType = DictionaryMsgType.STATUS;
                        break;
                    default:
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.FAILURE, "WlItemHandler.CallbackUser",
                                $"Unknown message class for dictionary: <{MsgClasses.ToString(msg.MsgClass)}>");

                }

                m_DecodeIt.Clear();
                if (msg.EncodedDataBody.Data() != null)
                    m_DecodeIt.SetBufferAndRWFVersion(msg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                            m_Watchlist.ReactorChannel!.MinorVersion);

                CodecReturnCode codecRet;
                if ((codecRet = m_RdmDictionaryMsg.Decode(m_DecodeIt, msg)) < CodecReturnCode.SUCCESS)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE, "WlItemHandler.CallbackUser",
                            $"DictionaryMsg.Decode() failed: <{codecRet.GetAsString()}>");
                }

                ReactorCallbackReturnCode cbRet = m_Watchlist.Reactor!.SendAndHandleDictionaryMsgCallback(location,
                                                                              m_Watchlist.ReactorChannel!,
                                                                              null!,
                                                                              msg,
                                                                              m_RdmDictionaryMsg,
                                                                              out errorInfo, wlRequest);

                if (cbRet == ReactorCallbackReturnCode.RAISE)
                {
                    ret = m_Watchlist.Reactor!.SendAndHandleDefaultMsgCallback(location,
                                                                               m_Watchlist.ReactorChannel!,
                                                                               null!,
                                                                               msg,
                                                                               wlRequest,
                                                                               out errorInfo);
                }
            }

            /* If the watchlist added the MsgKey, remove it in case subsqeuent requests on this 
               stream did not ask for MsgKeys in responses. */
            if (msgFlagsToReset != 0) msg.Flags &= ~msgFlagsToReset;

            return ret;
        }

        // This interface is not needed for the item handler
        ReactorReturnCode IWlHandler.CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            throw new NotImplementedException();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator? decodeIt, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            WlItemStream wlItemStream = (WlItemStream)wlStream;
            WlService? wlService = wlItemStream.WlService;
            errorInfo = null;

            m_CurrentFanoutStream = wlItemStream;
            m_CurrentMsgState = null;

            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    {
                        ret = ReadUpdateMsg(wlItemStream, msg, out errorInfo);
                        break;
                    }
                case MsgClasses.REFRESH:
                    {
                        IRefreshMsg refreshMsg = (IRefreshMsg)msg;

                        m_CurrentMsgState = refreshMsg.State;
                        ReadRefreshMsg(wlItemStream, refreshMsg, out errorInfo);

                        break;
                    }
                case MsgClasses.STATUS:
                    {
                        IStatusMsg statusMsg = (IStatusMsg)msg;

                        if (statusMsg.CheckHasState())
                        {
                            m_CurrentMsgState = statusMsg.State;
                        }

                        ReadStatusMsg(wlItemStream, statusMsg, out errorInfo);

                        break;
                    }
                case MsgClasses.GENERIC:
                    {
                        IGenericMsg genericMsg = (IGenericMsg)msg;

                        ret = ReadGenericMsg(wlItemStream, genericMsg, out errorInfo);

                        break;
                    }
                case MsgClasses.ACK:
                    {
                        IAckMsg ackMsg = (IAckMsg)msg;

                        ret = ReadAckMsg(wlItemStream, ackMsg, out errorInfo);

                        break;
                    }
                default:
                    {
                        ret = Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "WlItemHandler.ReadMsg", $"Invalid message class({msg.MsgClass}) received by Watchlist item handler");

                        break;
                    }
            }

            if (m_CurrentFanoutStream is null)
            {
                // All requests of this item stream were closed inside callback; it it now safe to return the WlItemStream.
                wlItemStream.ReturnToPool();
            }
            else if (m_CurrentMsgState is not null)
            {
                // if m_CurrentMsgState is still set, stream is closed and may need to be recovered.
                Debug.Assert(m_CurrentMsgState.StreamState() != StreamStates.OPEN 
                    || (m_CurrentMsgState.DataState() == DataStates.SUSPECT && !m_Watchlist.LoginHandler.SupportAllowSuspectData));

                var waitingRequest = wlItemStream.WaitingRequestDlList.Pop(WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                while (waitingRequest != null)
                {
                    wlItemStream.UserRequestDlList.PushBack(waitingRequest);
                    waitingRequest = wlItemStream.WaitingRequestDlList.Pop(WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                }
                bool suspectDataOnOpenStreamForbidden = m_CurrentMsgState.StreamState() == StreamStates.OPEN
                        && m_CurrentMsgState.DataState() == DataStates.SUSPECT
                        && !m_Watchlist.LoginHandler.SupportAllowSuspectData;

                var request = wlItemStream.UserRequestDlList.Pop();
                while (request != null)
                {
                    msg.StreamId = request.RequestMsg.StreamId;
                    msg.DomainType = request.RequestMsg.DomainType;

                    int originalDataState = m_CurrentMsgState.DataState();
                    int originalStreamState = m_CurrentMsgState.StreamState();

                    if (IsRequestRecoverable(request, m_CurrentMsgState.StreamState()))
                    {
                        UserStreamIdListToRecover.AddLast(request.RequestMsg.StreamId);
                        request.ReqState = WlRequest.State.PENDING_REQUEST;

                        m_CurrentMsgState.StreamState(StreamStates.OPEN);
                        m_CurrentMsgState.DataState(DataStates.SUSPECT);

                        m_SubmitOptions.ServiceName = request.WatchlistStreamInfo.ServiceName;
                        m_SubmitOptions.RequestMsgOptions.UserSpecObj = request.WatchlistStreamInfo.UserSpec;
                        AddToPendingRequestTable((WlItemRequest)request, m_SubmitOptions);

                        if ((ret = CallbackUserWithMsg("WlItemHandler.ReadMsg", msg, request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            break;
                        }
                    }
                    else
                    {     
                        CloseWlRequest((WlItemRequest)request);
                        if (suspectDataOnOpenStreamForbidden)
                        {
                            m_CurrentMsgState.StreamState(StreamStates.CLOSED_RECOVER);
                        }
                        if ((ret = CallbackUserWithMsg("WlItemHandler.ReadMsg", msg, request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            break;
                        }
                        RepoolWlRequest((WlItemRequest)request);
                    }

                    // Restore original state on message before next fanout.
                    m_CurrentMsgState.StreamState(originalStreamState);
                    m_CurrentMsgState.DataState(originalDataState);

                    request = wlItemStream.UserRequestDlList.Pop();
                }

                if (ret < ReactorReturnCode.SUCCESS)
                {
                    wlItemStream.ReturnToPool();
                    m_CurrentFanoutStream = null;
                    return ReactorReturnCode.FAILURE;
                }

                // if Consumer doesn't tolerate SUSPECT data on open stream, send out CLOSE message since it should be closed
                if (m_CurrentMsgState.StreamState() == StreamStates.OPEN 
                    && m_CurrentMsgState.DataState() == DataStates.SUSPECT 
                    && !m_Watchlist.LoginHandler.SupportAllowSuspectData)
                {
                    m_CloseMsg.MsgClass = MsgClasses.CLOSE;
                    m_CloseMsg.DomainType = wlItemStream.StreamDomainType;
                    m_CloseMsg.StreamId = wlItemStream.StreamId;

                    ret = wlItemStream.SendMsg(m_CloseMsg, m_SubmitOptions, false, out errorInfo);
                    if (ret == ReactorReturnCode.NO_BUFFERS)
                    {
                        ret = ReactorReturnCode.SUCCESS;
                    }
                    if (ret < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }

                CloseWlItemStream(wlItemStream);
                wlItemStream.ReturnToPool();

                if (wlService != null)
                    ServiceAdded(wlService, out errorInfo);
            }

            /* send next request in service's waiting request list */
            if (wlService != null && wlService.WaitingRequestList.Count > 0 && msg.MsgClass != MsgClasses.REFRESH)
            {
                WlRequest waitingRequest = wlService.WaitingRequestList.First!.Value;
                wlService.WaitingRequestList.RemoveFirst();
                m_SubmitOptions.ServiceName = waitingRequest.WatchlistStreamInfo.ServiceName;
                m_SubmitOptions.RequestMsgOptions.UserSpecObj = waitingRequest.WatchlistStreamInfo.UserSpec;
                ret = HandleItemRequest((WlItemRequest)waitingRequest, waitingRequest.RequestMsg, m_SubmitOptions, true, out errorInfo);
            }

            m_CurrentFanoutStream = null;

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        private ReactorReturnCode ReadRefreshMsg(WlItemStream wlItemStream, IRefreshMsg msg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            m_SnapshotViewClosed = false;
            int currentViewCount = 0;
            bool isRefreshComplete = msg.CheckRefreshComplete();
            bool fanoutViewPendingRefresh = wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_VIEW_REFRESH 
                && wlItemStream.AggregateView != null 
                && wlItemStream.AggregateView.ElementCount != wlItemStream.RequestsWithViewCount;
            bool solicitedRefresh = wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_REFRESH 
                || wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE;

            WlService wlService = wlItemStream.WlService!;
            bool fanoutFirstRefresh = false;
            bool viewChangedFromRemove = false;

            // notify stream that response received if solicited
            if (msg.CheckSolicited())
            {
                fanoutFirstRefresh = wlItemStream.RequestPending;
                wlItemStream.ResponseReceived();
            }
            else
            {
                if (wlItemStream.RequestPending && (wlItemStream.RefreshState != WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE))
                {
                        m_CurrentMsgState = null;
                        return ReactorReturnCode.SUCCESS;
                }
            }

            if (msg.DomainType == (int)DomainType.SYMBOL_LIST)
            {
                HandleSymbolList(wlItemStream, msg, out errorInfo);
            }

            // copy the State from the refresh message
            msg.State.Copy(wlItemStream.State);

            // if message isn't open or non-streaming, let recovery code handles it.
            if (msg.State.StreamState() != StreamStates.OPEN && msg.State.StreamState() != StreamStates.NON_STREAMING)
            {
                return ReactorReturnCode.SUCCESS;
            }

            int usrRequestList = wlItemStream.UserRequestDlList.Count();

            // decrease number of outstanding requests on service when the request has not been removed by the user
            if (isRefreshComplete && (usrRequestList != 0))
            {
                if (wlService.NumOutstandingRequests > 0)
                {
                    wlService.NumOutstandingRequests--;
                }
            }

            // fanout refresh message to user requests associated with the stream
            var request = (WlItemRequest?)wlItemStream.UserRequestDlList.Start();
            bool removeSolicited = false;
            bool requestStreaming;
            while (request != null)
            {
                requestStreaming = (request.RequestMsg.Flags & RequestMsgFlags.STREAMING) > 0;
                request.HandlePendingViewFanout(fanoutViewPendingRefresh);

                // only fanout if refresh is desired and refresh is unsolicited or to those whose state is awaiting refresh
                if ( (fanoutFirstRefresh || !request.RequestMsg.CheckNoRefresh()) &&
                    (!msg.CheckSolicited() ||
                     request.SolicitedRefreshNeededForView(solicitedRefresh) ||
                     request.ReqState == WlRequest.State.PENDING_REFRESH ||
                     request.ReqState == WlRequest.State.PENDING_COMPLETE_REFRESH) ||
                     fanoutViewPendingRefresh)
                {
                    // check refresh complete flag and change state of user request accordingly
                    if (isRefreshComplete)
                    {
                        wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.NONE;

                        WlItemGroup? wlItemGroup;
                        if (!wlService.ItemGroupDict.TryGetValue(msg.GroupId, out wlItemGroup))
                        {
                            // add group Id as new item group in the WlService's ItemGroupDict
                            wlItemGroup = m_itemGroupPool.GetWlItemGroup(msg.GroupId.Length);
                            msg.GroupId.Copy(wlItemGroup.GroupId);
                            wlItemGroup.WlServiceId = wlService.RdmService!.ServiceId;
                            wlService.ItemGroupDict.Add(wlItemGroup.GroupId!, wlItemGroup);
                        }

                        if (wlItemStream.ItemGroup != wlItemGroup) // don't re-add stream to the same group
                        {
                            // remove item from existing group, if present.
                            RemoveStreamFromItemGroup(wlItemStream);
                            AddStreamToItemGroup(wlItemGroup!, wlItemStream);
                        }

                        if (requestStreaming && msg.State.StreamState() != StreamStates.NON_STREAMING)
                        {
                            // streaming request
                            if (request.ReqState != WlRequest.State.OPEN)
                            {
                                request.ReqState = WlRequest.State.OPEN;
                            }
                            else
                            {   /* Stream is already open so this is unsolicited refresh for the item request */
                                if (msg.CheckSolicited())
                                {
                                    msg.Flags &= ~RefreshMsgFlags.SOLICITED;
                                    removeSolicited = true;
                                }
                            }

                            if (msg.DomainType != (int)DomainType.DICTIONARY &&
                                (request.ItemReqFlags & WlItemRequest.Flags.HAS_STATIC_QOS) == 0 &&
                                !request.MatchedQos.IsDynamic)
                            {
                                request.ItemReqFlags |= WlItemRequest.Flags.HAS_STATIC_QOS;
                            }
                        }
                        else
                        {
                            // snapshot request
                            if (requestStreaming && request.RequestMsg.CheckPause())
                            {
                                WlItemStream tempWlItemStream = (WlItemStream)request.WatchlistStream!;
                                if (tempWlItemStream.NumPausedRequestsCount > 0)
                                {
                                    tempWlItemStream.NumPausedRequestsCount--;
                                }
                            }
                        }
                    }
                    else if (request.ReqState == WlRequest.State.PENDING_REFRESH) // multi-part refresh
                    {
                        request.ReqState = WlRequest.State.PENDING_COMPLETE_REFRESH;

                        // set multi-part refresh pending flag
                        wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE;

                        wlItemStream.StartRequestTimer();
                    }

                    // update stream Id in message to that of user request
                    msg.StreamId = request.RequestMsg.StreamId;

                    // For snapshot requests, change OPEN state to NON-STREAMING.
                    int tempStreamState = msg.State.StreamState();
                    if (!requestStreaming && tempStreamState == StreamStates.OPEN)
                    {
                        msg.State.StreamState(StreamStates.NON_STREAMING);
                    }

                    // if snapshot request or NON_STREAMING and the refresh complete is received, close the WlItemRequest
                    // and WlItemStream if necessary
                    if ((!requestStreaming || msg.State.StreamState() == StreamStates.NON_STREAMING) && isRefreshComplete)
                    {
                        wlItemStream.UserRequestDlList.Remove(request);
                        if (wlItemStream.RequestsWithViewCount > 0)
                        {
                            if (request.RequestMsg.CheckHasView())
                            {
                                RemoveRequestView(wlItemStream, request, out viewChangedFromRemove);
                            }

                            wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                            m_SnapshotViewClosed = true;
                            // save the current view count as callbackuser() and waiting list below can potentially add views 
                            currentViewCount = wlItemStream.RequestsWithViewCount;
                        }

                        // if no more requests in stream, close stream
                        bool allRequestedClosed = wlItemStream.UserRequestDlList.Count() == 0 && wlItemStream.WaitingRequestDlList.Count() == 0;

                        wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.NONE;
                        
                        CloseWlRequest(request);
                        // if there are no requests pending refresh completion, the stream can be closed at this point.
                        if (allRequestedClosed)
                        {
                            CloseWlItemStream(wlItemStream);
                        }

                        ret = CallbackUserWithMsg("WlItemHandler.ReadRefreshMsg", msg, request, out errorInfo);
                        request.ReturnToPool();

                        if (ret < ReactorReturnCode.SUCCESS)
                        {
                            // break out of loop for any error
                            break;
                        }

                        if (allRequestedClosed)
                        {
                            // Stream and all its requests were closed. Unset _currentFanoutStream so that the WlStream is repooled.
                            m_CurrentFanoutStream = null;
                            break;
                        }
                    }
                    else
                    {
                        WlRequest? tempWlRequest;
                        m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(msg.StreamId, out tempWlRequest);
                        if( (ret = CallbackUserWithMsg("WlItemHandler.ReadRefreshMsg", msg, tempWlRequest!, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            // break out of loop for any error
                            break;
                        }
                    }

                    msg.State.StreamState(tempStreamState);

                    /* Restore original value of the solicited flag of the refresh message */
                    if (removeSolicited)
                    {
                        msg.Flags |= RefreshMsgFlags.SOLICITED;
                        removeSolicited = false;
                    }
                }

                request = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();

            } // End while loop

            if (m_CurrentFanoutStream != null)
            {
                /* if no longer waiting for snapshot, send requests in waiting request list */
                if (wlItemStream.WaitingRequestDlList.Count() > 0 &&
                        (wlItemStream.RefreshState != WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE))
                {
                    WlItemRequest? waitingUserRequest;
                    while ((wlItemStream.RefreshState < WlItemStream.RefreshStateFlag.PENDING_REFRESH) 
                        && (waitingUserRequest = wlItemStream.WaitingRequestDlList.Pop(WlItemRequest.WAIT_ITEM_REQUEST_LINK)) != null)
                    {
                        m_SubmitOptions.ServiceName = waitingUserRequest.WatchlistStreamInfo.ServiceName;
                        m_SubmitOptions.RequestMsgOptions.UserSpecObj = waitingUserRequest.WatchlistStreamInfo.UserSpec;

                        if ( (waitingUserRequest.ItemChangeFlags & WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE) != 0)
                            ret = HandleItemReissue(waitingUserRequest, waitingUserRequest.RequestMsg, m_SubmitOptions, out errorInfo);
                        else
                            ret = HandleItemRequest(waitingUserRequest, waitingUserRequest.RequestMsg, m_SubmitOptions, true, out errorInfo);
                        if (ret < ReactorReturnCode.SUCCESS) return ret;
                    }
                }

                if (m_SnapshotViewClosed)
                {
                    m_SnapshotViewClosed = false;

                    if (currentViewCount > 0 && wlItemStream.RequestsWithViewCount == currentViewCount 
                        && wlItemStream.UserRequestDlList.Count() > 0 
                        && (viewChangedFromRemove || wlItemStream.AggregateView!.CommittedViews.Count == 0)
                        && wlItemStream.RequestsWithViewCount == wlItemStream.UserRequestDlList.Count())
                    {
                        wlItemStream.RequestMsg.Flags |= RequestMsgFlags.NO_REFRESH;
                        wlItemStream.SendMsg(wlItemStream.RequestMsg, m_SubmitOptions, false, out errorInfo);
                        wlItemStream.RequestMsg.Flags &= ~RequestMsgFlags.NO_REFRESH;
                    }
                }
            }

            /* send next request in service's waiting request list */
            if (wlService != null && wlService.WaitingRequestList.Count > 0 && isRefreshComplete)
            {
                WlItemRequest waitingItemRequest = wlService!.WaitingRequestList.First!.Value;
                wlService!.WaitingRequestList.RemoveFirst();
                m_SubmitOptions.ServiceName = waitingItemRequest.WatchlistStreamInfo.ServiceName;
                m_SubmitOptions.RequestMsgOptions.UserSpecObj = waitingItemRequest.WatchlistStreamInfo.UserSpec;

                ret = HandleItemRequest(waitingItemRequest, waitingItemRequest.RequestMsg, m_SubmitOptions, true, out errorInfo);
                if (ret < ReactorReturnCode.SUCCESS) return ret;
            }

            // No recovery needed, so set m_CurrentMsgState to null.
            m_CurrentMsgState = null;

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        private ReactorReturnCode ReadUpdateMsg(WlItemStream wlItemStream, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            if (msg.DomainType == (int)DomainType.SYMBOL_LIST)
            {
                HandleSymbolList(wlItemStream, msg, out errorInfo);
            }

            // fanout update message to user requests associated with the stream
            var request = (WlItemRequest?)wlItemStream.UserRequestDlList.Start();
            while (request != null)
            {
                // only fanout to those whose state is OPEN or PENDING_REFRESH_COMPLETE
                if (request.ReqState == WlRequest.State.OPEN || request.ReqState == WlRequest.State.PENDING_COMPLETE_REFRESH)
                {
                    msg.StreamId = request.RequestMsg.StreamId;

                    if ((ret = CallbackUserWithMsg("WlItemHandler.ReadUpdateMsg", msg, request, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        // break out of loop for any error
                        break;
                    }
                }

                request = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        private ReactorReturnCode ReadStatusMsg(WlItemStream wlItemStream, IStatusMsg statusMsg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            if(wlItemStream.WlService != null && wlItemStream.RequestPending)
            {
                wlItemStream.WlService.NumOutstandingRequests--;
            }

            // notify stream that response received
            wlItemStream.ResponseReceived();

            if(statusMsg.CheckHasState())
            {
                statusMsg.State.Copy(wlItemStream.State);
            }

            // fanout if open and tolerates suspect data; otherwise, recovery code will do it.
            if (!statusMsg.CheckHasState() 
                || (statusMsg.State.StreamState() == StreamStates.OPEN && (m_Watchlist.LoginHandler.SupportAllowSuspectData || statusMsg.State.DataState() != DataStates.SUSPECT)))
            {
                var request = (WlItemRequest?)wlItemStream.UserRequestDlList.Start();
                // fanout status message to user requests associated with the stream
                while (request != null)
                {
                    // update stream id in message to that of user request
                    statusMsg.StreamId = request.RequestMsg.StreamId;

                    // callback user
                    WlRequest? tempWlRequest;
                    if (m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(statusMsg.StreamId, out tempWlRequest))
                    {
                        if ((ret = CallbackUserWithMsg("WlItemHandler.ReadStatusMsg", statusMsg, tempWlRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            // break out of loop for any error
                            break;
                        }
                    }
                    request = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
                }

                // No recovery needed, so set m_CurrentMsgState to null.
                m_CurrentMsgState = null;
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        private ReactorReturnCode ReadGenericMsg(WlItemStream wlItemStream, IGenericMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            // fanout generic message to user requests associated with the stream
            var request = (WlItemRequest?)wlItemStream.UserRequestDlList.Start();
            while (request != null)
            {
                // only fanout to those whose state is PENDING_REFRESH or OPEN
                if (request.ReqState == WlRequest.State.PENDING_REFRESH ||
                    request.ReqState == WlRequest.State.PENDING_COMPLETE_REFRESH ||
                    request.ReqState == WlRequest.State.OPEN)
                {
                    msg.StreamId = request.RequestMsg.StreamId;

                    // callback user
                    WlRequest? tempWlRequest;
                    if (m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(msg.StreamId, out tempWlRequest))
                    {
                        if ((ret = CallbackUserWithMsg("WlItemHandler.ReadGenericMsg", msg, tempWlRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            // break out of loop for any error
                            break;
                        }
                    }
                }

                request = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ReadAckMsg(WlItemStream wlItemStream, IAckMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            if(wlItemStream.HandlePostAck(msg))
            {
                // callback user
                WlRequest? tempWlRequest;
                if (m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(msg.StreamId, out tempWlRequest))
                {
                    ret = CallbackUserWithMsg("WlItemHandler.ReadAckMsg", msg, tempWlRequest, out errorInfo);
                }
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void AddStreamToItemGroup(WlItemGroup wlItemGroup, WlItemStream wlItemStream)
        {
            if (wlItemStream.ItemGroup != wlItemGroup)
            {
                wlItemStream.ItemGroup = wlItemGroup;
                wlItemGroup.OpenStreamIdDlList.PushBack(wlItemStream.StreamIdLink[1]!);
            }
        }

        // Handles watchlist service added event
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ServiceAdded(WlService wlService, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            LinkedList<WlItemRequest>? pendingRequestList = null;
            errorInfo = null;

            // handle any pending requests
            // retrieve matching requests based on service id or service name
            PendingRequestByIdDict.Remove(wlService.RdmService!.ServiceId, out pendingRequestList);
            if (pendingRequestList is null)
            {
                PendingRequestByNameDict.Remove(wlService.RdmService!.Info.ServiceName.ToString(), out pendingRequestList);
            }

            // handle request
            if (pendingRequestList != null)
            {
                foreach(WlItemRequest wlItemRequest in pendingRequestList)
                {
                    m_SubmitOptions.ServiceName = wlItemRequest.WatchlistStreamInfo.ServiceName;
                    m_SubmitOptions.RequestMsgOptions.UserSpecObj = wlItemRequest.WatchlistStreamInfo.UserSpec;

                    if (wlItemRequest.RequestMsg.CheckNoRefresh())
                    {
                        wlItemRequest.RequestMsg.Flags &= ~RequestMsgFlags.NO_REFRESH;
                    }

                    if ((ret = HandleItemRequest(wlItemRequest, wlItemRequest.RequestMsg, m_SubmitOptions, false, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            // call SendMsg on all streams in pending stream send list
            var stream = PendingSendMsgDlList.Pop();
            while (stream != null)
            {
                stream.PendingSendMsg = false;

                if ((ret = stream.SendMsg(stream.RequestMsg, m_SubmitOptions, false, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    // No buffers means that the request was re-queued, so we can end the loop here
                    if (ret == ReactorReturnCode.NO_BUFFERS)
                    {
                        return ReactorReturnCode.SUCCESS;
                    }
                    else
                    {
                        return ret;
                    }
                }

                StreamDlList.PushBack(stream);

                if (PendingSendMsgDlList.Count() == 0)
                {
                    return ReactorReturnCode.SUCCESS;
                }

                stream = PendingSendMsgDlList.Pop();
            }

            UserStreamIdListToRecover.Clear();

            return ret;
        }

        private bool UserRequestExistsInStream(WlService service, VaDoubleLinkList<WlItemStream> list, VaDoubleLinkList<WlItemStream>.ILink<WlItemStream> link)
        {
            bool exists = false;

            var stream = list.Start(link);
            while (stream != null)
            {
                if (stream != null && stream.UserRequestDlList.Count() != 0 && stream.WlService == service)
                {
                    exists = true;
                    break;
                }
                stream = list.Forth(link);
            }

            return exists;
        }

        private ReactorReturnCode SendStatusMsgOnServiceUpdated(WlService service, 
            VaDoubleLinkList<WlItemStream> list,
            VaDoubleLinkList<WlItemStream>.ILink<WlItemStream> link, 
            out ReactorErrorInfo? errorInfo)
        {
            /* Fanout Status */
            int count = list.Count();
            var stream = list.Start(link);
            IStatusMsg statusMsg;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            while (stream != null && count > 0)
            {
                if (service == stream.WlService)
                {
                    statusMsg = GetStatusMsgFromPool();
                    statusMsg.DomainType = stream!.StreamDomainType;
                    statusMsg.StreamId = stream.StreamId;
                    statusMsg.ApplyHasState();
                    service!.RdmService!.State.Status.Copy(statusMsg.State);

                    ret = ReadMsg(stream, null, statusMsg, out errorInfo);

                    // return IStatusMsg to the its pool
                    StatusMsgPool.AddLast(statusMsg);

                    if (ret < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                count--;
                stream = list.Forth(link);
            }

            errorInfo = null;
            return ret;
        } 

        private int GetServiceStreamsCount(WlService service,
            VaDoubleLinkList<WlItemStream> list,
            VaDoubleLinkList<WlItemStream>.ILink<WlItemStream> link)
        {
            int result = 0;
            var stream = list.Start(link);
            while (stream != null)
            {
                if (stream.WlService == service)
                {
                    result++;
                }
                stream = list.Forth(link);
            }
            return result;
        }

        // Handles watchlist service updated event
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ServiceUpdated(WlService wlService, bool serviceStateUpdate, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            // Check if StreamList is empty or there are no userRequests and call ServiceAdded in these cases,
            // 	but we want to return out ServiceUpdated afterwards, ignoring further processing in these cases 
            if (wlService.StreamIdDlList.Count() == 0)
            {
                ret = ServiceAdded(wlService, out errorInfo);

                // We do not cache groupStateList
                wlService.RdmService!.GroupStateList.Clear();
                return ret;
            }
            else
            {
                bool userRequestExists = false;
                var streamLink = wlService.StreamIdDlList.Start();
                while (streamLink != null)
                {
                    WlItemStream? stream = m_Watchlist!.StreamManager.StreamsByStreamIds[streamLink.StreamId - WlStreamManager.MIN_STREAM_ID];
                    if (stream != null && stream.UserRequestDlList.Count() != 0)
                    {
                        userRequestExists = true;
                        break;
                    }
                    streamLink = wlService.StreamIdDlList.Forth();
                }

                if (!userRequestExists)
                {
                    ret = ServiceAdded(wlService, out errorInfo);

                    // We do not cache groupStateList
                    wlService.RdmService!.GroupStateList.Clear();
                    return ret;
                }
            }

            ret = ServiceAdded(wlService, out errorInfo);
            if (ret < ReactorReturnCode.SUCCESS)
            {
                return ret;
            }

            // Check if service state is available for processing
            if (serviceStateUpdate && wlService.RdmService!.HasState && wlService.RdmService!.State.HasStatus)
            {
                /* Fanout Status */
                int count = wlService.StreamIdDlList.Count();
                var streamLink = wlService.StreamIdDlList.Start();
                IStatusMsg statusMsg;
                for (int i = 0; i < count && streamLink != null; i++)
                {
                    WlItemStream? stream = m_Watchlist!.StreamManager.StreamsByStreamIds[streamLink.StreamId - WlStreamManager.MIN_STREAM_ID];
                    statusMsg = GetStatusMsgFromPool();
                    statusMsg.DomainType = stream.StreamDomainType;
                    statusMsg.StreamId = stream.StreamId;
                    statusMsg.ApplyHasState();
                    wlService.RdmService.State.Status.Copy(statusMsg.State);

                    ret = ReadMsg(stream, null, statusMsg, out errorInfo);

                    // return IStatusMsg to the its pool
                    StatusMsgPool.AddLast(statusMsg);

                    if (ret < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    streamLink = wlService.StreamIdDlList.Forth();
                }
            }

            // Check if group states are available for processing
            foreach (var serviceGroup in wlService.RdmService!.GroupStateList)
            {
                if (serviceGroup.HasStatus)
                {
                    /*	Fanout Status. */
                    WlItemGroup? wlItemGroup;
                    wlService.ItemGroupDict.TryGetValue(serviceGroup.Group, out wlItemGroup);
                    if (wlItemGroup != null)
                    {
                        WlItemStream? wlItemStream;
                        var wlItemStreamLink = wlItemGroup!.OpenStreamIdDlList!.Start();
                        while (wlItemStreamLink != null)
                        {
                            wlItemStream = m_Watchlist!.StreamManager.StreamsByStreamIds[wlItemStreamLink.StreamId - WlStreamManager.MIN_STREAM_ID];
                            IStatusMsg statusMsg = GetStatusMsgFromPool();

                            statusMsg.DomainType = wlItemStream.StreamDomainType;
                            statusMsg.StreamId = wlItemStream.StreamId;
                            statusMsg.ApplyHasState();
                            statusMsg.ApplyHasGroupId();
                            statusMsg.GroupId = wlItemGroup.GroupId;
                            serviceGroup.Status.Copy(statusMsg.State);

                            ret = ReadMsg(wlItemStream, null, statusMsg, out errorInfo);

                            // return StatusMsg to pool
                            StatusMsgPool.AddLast(statusMsg);

                            if (ret < ReactorReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            wlItemStreamLink = wlItemGroup!.OpenStreamIdDlList!.Forth();
                        }
                    }
                }
                if (serviceGroup.HasMergedToGroup)
                {
                    WlItemGroup? wlItemGroup;
                    wlService.ItemGroupDict.Remove(serviceGroup.Group, out wlItemGroup);
                    WlItemGroup? newItemGroup;
                    wlService.ItemGroupDict.TryGetValue(serviceGroup.MergedToGroup, out newItemGroup);
                    if (wlItemGroup != null)
                    {
                        if (newItemGroup != null)
                        {
                            /* Update all WlItemStream objects by removing from the existing item group and add to the new item group. */
                            WlItemStream? wlItemStream;
                            var wlItemStreamLink = wlItemGroup!.OpenStreamIdDlList!.Start();
                            while (wlItemStreamLink != null)
                            {
                                wlItemStream = m_Watchlist!.StreamManager.StreamsByStreamIds[wlItemStreamLink.StreamId - WlStreamManager.MIN_STREAM_ID];

                                if (wlItemStream != null)
                                {
                                    if (ReferenceEquals(wlItemStream.ItemGroup, wlItemGroup))
                                    {
                                        wlItemGroup.OpenStreamIdDlList.Remove(wlItemStream.StreamIdLink[1]!);

                                        if (wlItemGroup.OpenStreamIdDlList.Count() == 0 && wlItemStream.WlService != null)
                                        {
                                            if (wlItemGroup.GroupId != null)
                                            {
                                                /* wlItemGroup is already removed so it can be returned back to its pool. */
                                                m_itemGroupPool.ReturnWlItemGroup(wlItemGroup);
                                            }
                                        }
                                    }

                                    /* Update with the new item group */
                                    wlItemStream.ItemGroup = newItemGroup;

                                    newItemGroup.OpenStreamIdDlList.PushBack(wlItemStream.StreamIdLink[1]!);
                                }

                                wlItemStreamLink = wlItemGroup!.OpenStreamIdDlList!.Forth();
                            }
                        }
                        else
                        {
                            ByteBuffer destByteBuffer;
                            var groupId = wlItemGroup.GroupId;
                            int length = Math.Max(groupId!.Data().Contents.Length, WlItemGroupPool.GetMinPow2Length(serviceGroup.MergedToGroup.Length));

                            if (length <= groupId.Data().Contents.Length) { destByteBuffer = groupId.Data(); destByteBuffer.Clear(); }
                            else destByteBuffer = new ByteBuffer(length);

                            wlItemGroup.GroupId!.Clear();
                            wlItemGroup.GroupId.Data(destByteBuffer);
                            serviceGroup.MergedToGroup.Copy(groupId);
                            wlService.ItemGroupDict.Add(wlItemGroup.GroupId, wlItemGroup);
                        }
                    }
                }
            }

            // We do not cache GroupStateList. Clear after processing.
            wlService.RdmService!.GroupStateList.Clear();

            return ret;
        }

        // Handles watchlist service deleted event
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ServiceDeleted(WlService wlService, bool channelIsDown, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;
            string stateText;

            // For item recovery there is no functional difference between losing
            // all services and the channel being down. The text is just changed
            // if the cause was actually the channel going down.
            if (channelIsDown)
                stateText = "channel down.";
            else
                stateText = "Service for this item was lost.";

            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);
            m_StatusMsg.State.Text().Data(stateText);

            var streamLink = wlService.StreamIdDlList.Peek();
            while (streamLink != null)
            {
                WlItemStream stream = m_Watchlist!.StreamManager!.StreamsByStreamIds[streamLink.StreamId - WlStreamManager.MIN_STREAM_ID];
                RemoveWlItemStreamFromService(stream);
                m_StatusMsg.DomainType = stream.StreamDomainType;
                if ((ret = ReadMsg(stream, null, m_StatusMsg, out errorInfo)) != ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
                streamLink = wlService.StreamIdDlList.Peek();
            }

            WlItemRequest wlItemRequest;
            LinkedListNode<WlItemRequest>? firstRequestNode;
            while ((firstRequestNode = wlService.WaitingRequestList.First) != null)
            {
                wlItemRequest = firstRequestNode.Value;
                wlService.WaitingRequestList.RemoveFirst();

                bool isRecoverable = IsRequestRecoverable(wlItemRequest, StreamStates.CLOSED_RECOVER);

                m_StatusMsg.StreamId = wlItemRequest.RequestMsg.StreamId;
                m_StatusMsg.DomainType = wlItemRequest.RequestMsg.DomainType;

                if (isRecoverable)
                {
                    m_StatusMsg.State.StreamState(StreamStates.OPEN);
                    m_SubmitOptions.ServiceName = wlItemRequest.WatchlistStreamInfo.ServiceName;
                    m_SubmitOptions.RequestMsgOptions.UserSpecObj = wlItemRequest.WatchlistStreamInfo.UserSpec;
                    AddToPendingRequestTable(wlItemRequest, m_SubmitOptions);
                }
                else
                {
                    CloseWlRequest(wlItemRequest);
                    m_StatusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
                }

                if (CallbackUserWithMsg("WlItemHandler.ServiceDeleted", m_StatusMsg, wlItemRequest, out errorInfo) < ReactorReturnCode.SUCCESS)
                {
                    return ReactorReturnCode.FAILURE;
                }

                if (!isRecoverable)
                {
                    RepoolWlRequest(wlItemRequest);
                }                  
            }

            return ReactorReturnCode.SUCCESS;
        }

        /* Adds a user request to the pending request table. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void AddToPendingRequestTable(WlItemRequest wlItemRequest, ReactorSubmitOptions submitOptions)
        {
            // set WlStream to null when starting over
            wlItemRequest.WatchlistStream = null;

            // retrieve pending request list for this service id/name if one exists
            LinkedList<WlItemRequest>? pendingRequestList = null;
            if (submitOptions.ServiceName != null)
            {
                PendingRequestByNameDict.TryGetValue(submitOptions.ServiceName, out pendingRequestList);
            }
            else
            {
                PendingRequestByIdDict.TryGetValue(wlItemRequest.RequestMsg.MsgKey.ServiceId, out pendingRequestList);
            }

            // add to pending request list
            if (pendingRequestList != null)
            {
                // pending request list exists, just add to existing list
                pendingRequestList.AddLast(wlItemRequest);
            }
            else // pending request list doesn't exist
            {
                // create a pending request list
                pendingRequestList = new ();

                // add pending request to list
                pendingRequestList.AddLast(wlItemRequest);

                // add pending request list to table
                if (submitOptions.ServiceName != null)
                {
                    PendingRequestByNameDict.Add(submitOptions.ServiceName, pendingRequestList);
                }
                else
                {
                    PendingRequestByIdDict.Add(wlItemRequest.RequestMsg.MsgKey.ServiceId, pendingRequestList);
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void CloseWlItemStream(WlItemStream wlItemStream)
        {
            // remove the item stream from the aggregation dictionary.
            if (wlItemStream.StreamAttributes != null)
            {
                WlStreamAttribToWlStreamDict!.Remove(wlItemStream.StreamAttributes);
                wlItemStream.StreamAttributes.ReturnToPool();
                wlItemStream.StreamAttributes = null;
            }

            if (wlItemStream.PendingSendMsg)
            {
                PendingSendMsgDlList.Remove(wlItemStream);
                wlItemStream.PendingSendMsg = false;
            } 
            else
            {
                StreamDlList.Remove(wlItemStream);
            }                   

            if (wlItemStream.WlService != null)
            {
                RemoveWlItemStreamFromService(wlItemStream);
            }

            wlItemStream.CloseWlItemStream();
        }

        // remove WlItemStream from its parent service including item group in that service if any.
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void RemoveWlItemStreamFromService(WlItemStream wlItemStream)
        {
            RemoveStreamFromItemGroup(wlItemStream);
            if (wlItemStream.WlService != null)
            {
                wlItemStream.WlService.StreamIdDlList.Remove(wlItemStream.StreamIdLink[0]!);
                wlItemStream.WlService = null;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void RemoveStreamFromItemGroup(WlItemStream wlItemStream)
        {
            WlItemGroup? wlItemGroup = wlItemStream.ItemGroup;
            if (wlItemGroup != null)
            {
                wlItemGroup.OpenStreamIdDlList.Remove(wlItemStream.StreamIdLink[1]!);

                // If no streams left in group's stream list, remove item group from dictionary
                if (wlItemGroup.OpenStreamIdDlList.Count() == 0 && wlItemStream.WlService != null)
                {
                    if (wlItemGroup.GroupId != null)
                    {
                        wlItemStream.WlService.ItemGroupDict.Remove(wlItemGroup.GroupId);
                        m_itemGroupPool.ReturnWlItemGroup(wlItemGroup);
                    }
                }

                wlItemStream.ItemGroup = null;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode SubmitMsg(WlRequest wlRequest, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            switch(msg.MsgClass)
            {
                case MsgClasses.CLOSE:
                    WlItemStream? wlItemStream = (WlItemStream?)wlRequest.WatchlistStream;
                    if (wlItemStream != null)
                    {
                        if (wlItemStream.RequestPending && wlItemStream.UserRequestDlList.Count() == 1)
                        {
                            if (wlItemStream.WlService != null)
                            {
                                wlItemStream.WlService.NumOutstandingRequests--;
                            }
                        }

                        ret = RemoveUserRequestFromOpenStream((WlItemRequest)wlRequest, msg, wlItemStream, submitOptions, out errorInfo);
                    }
                    else
                    {
                        ret = RemoveUserRequestFromClosedStream((WlItemRequest)wlRequest);
                    }

                    break;

                case MsgClasses.POST:
                    if (m_Watchlist.LoginHandler.SupportPost)
                    {
                        if (wlRequest.ReqState == WlRequest.State.OPEN)
                        {
                            return HandlePost((WlItemRequest)wlRequest, msg, submitOptions, out errorInfo);
                        }
                        else
                        {
                            // cannot submit post when the stream is not open
                            return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                                "WlItemHandler.SubmitMsg", "Cannot submit IPostMsg when stream not in open state.");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                                "WlItemHandler.SubmitMsg", "Posting not supported by provider.");
                    }

                case MsgClasses.GENERIC:
                    if (wlRequest.ReqState == WlRequest.State.OPEN)
                    {
                        bool resetServiceId = false;

                        // replace service id if message submitted with service name
                        if (submitOptions.ServiceName != null)
                        {
                            if (!((IGenericMsg)msg).CheckHasMsgKey())
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                                                 ReactorReturnCode.INVALID_USAGE,
                                                                 "WlItemHandler.SubmitMsg",
                                                                 "Generic message submitted with service name but no message key.");

                            }

                            if ((ret = m_Watchlist.ChangeServiceNameToID(((IGenericMsg)msg).MsgKey, submitOptions.ServiceName, out errorInfo))
                                < ReactorReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            // set resetServiceId flag
                            resetServiceId = true;
                        }

                        WlItemStream? tempWlItemStream = (WlItemStream?)wlRequest.WatchlistStream;

                        // replace stream id with aggregated stream id
                        msg.StreamId = tempWlItemStream!.StreamId;

                        // send message
                        ret = tempWlItemStream.SendMsg(msg, submitOptions, false, out errorInfo);

                        if(ret == ReactorReturnCode.NO_BUFFERS)
                        {
                            errorInfo = null;
                            ret = ReactorReturnCode.SUCCESS;
                        }

                        // reset service id if necessary
                        if (resetServiceId)
                        {
                            msg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                            msg.MsgKey.ServiceId = 0;
                        }

                        // return if send message not successful
                        if (ret < ReactorReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                    else
                    {
                        // cannot submit generic message when stream is not open
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                                        ReactorReturnCode.INVALID_USAGE,
                                                        "WlItemHandler.SubmitMsg",
                                                        "Cannot submit GenericMsg when stream not in open state.");
                    }
                    break;

                default:

                    ret = Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                        "WlItemHandler.SubmitMsg", $"Invalid message class ({msg.MsgClass}) submitted to Watchlist item handler");
                    break;
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorReturnCode HandlePost(WlItemRequest wlItemRequest, IMsg msg, ReactorSubmitOptions submitOptions, 
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            WlItemStream wlItemStream = (WlItemStream)wlItemRequest.WatchlistStream!;
            if (m_Watchlist.NumOutstandingPosts < m_Watchlist.ConsumerRole!.WatchlistOptions.MaxOutstandingPosts)
            {
                bool resetServiceId = false;

                // validate post submit
                if ((ret = wlItemStream.ValidatePostSubmit((IPostMsg)msg, out errorInfo)) != ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }

                // replace service id if message submitted with service name
                if (submitOptions.ServiceName != null)
                {
                    if (!((IPostMsg)msg).CheckHasMsgKey())
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                                ReactorReturnCode.INVALID_USAGE,
                                                "WlItemHandler.HandlePost",
                                                "Post message submitted with service name but no message key.");

                    }

                    if ((ret = m_Watchlist.ChangeServiceNameToID(((IPostMsg)msg).MsgKey, submitOptions.ServiceName,
                        out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    // set resetServiceId flag
                    resetServiceId = true;
                }

                // send message
                // no need to replace stream id for post message here - that's done inside SendMsg()
                int userStreamId = msg.StreamId;
                msg.StreamId = wlItemStream.StreamId;

                ret = wlItemStream.SendMsg(msg, submitOptions, false, out errorInfo);
                if(ret == ReactorReturnCode.NO_BUFFERS)
                {
                    ret = ReactorReturnCode.SUCCESS;
                }

                msg.StreamId = userStreamId;

                // reset service id if CheckAck() return false
                if (resetServiceId && (!((IPostMsg)msg).CheckAck() || (ret < ReactorReturnCode.SUCCESS)))
                {
                    ((IPostMsg)msg).MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                    ((IPostMsg)msg).MsgKey.ServiceId = 0;
                    resetServiceId = false;
                }
                if (ret < ReactorReturnCode.SUCCESS)
                    return ret;
                else
                {
                    if (((IPostMsg)msg).CheckAck())
                    {
                        // increment number of outstanding post messages
                        m_Watchlist.NumOutstandingPosts++;

                        // update post tables
                        ret = wlItemStream.UpdatePostTables((IPostMsg)msg, out errorInfo);

                        // reset service id if necessary
                        if (resetServiceId)
                        {
                            ((IPostMsg)msg).MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                            ((IPostMsg)msg).MsgKey.ServiceId = 0;
                        }
                    }
                }
            }
            else
            {
                ret = Reactor.PopulateErrorInfo(out errorInfo,
                                                             ReactorReturnCode.INVALID_USAGE,
                                                             "WlItemHandler.HandlePost",
                                                             "maxOutstandingPosts limit reached.");
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorReturnCode RemoveUserRequestFromOpenStream(WlItemRequest wlItemRequest, IMsg msg, WlItemStream wlItemStream, 
            ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            var waitingRequest = wlItemStream.WaitingRequestDlList.Start(WlItemRequest.WAIT_ITEM_REQUEST_LINK);
            while (waitingRequest != null)
            {
                if (waitingRequest.RequestMsg.StreamId != wlItemRequest.RequestMsg.StreamId)
                {
                    waitingRequest = wlItemStream.WaitingRequestDlList.Forth(WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                    continue;
                }
                else
                {
                    wlItemStream.WaitingRequestDlList.Remove(waitingRequest, WlItemRequest.WAIT_ITEM_REQUEST_LINK);

                    // close watchlist request
                    CloseWlRequest(wlItemRequest);
                    RepoolWlRequest(wlItemRequest);
                    return ret;
                }
            }

            var request = wlItemStream.UserRequestDlList.Start();
            while (request != null)
            {
                if (request.RequestMsg.StreamId != wlItemRequest.RequestMsg.StreamId)
                {
                    request = wlItemStream.UserRequestDlList.Forth();
                    continue;
                }
                else
                {
                    wlItemStream.UserRequestDlList.Remove(request);

                    wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.NONE;

                    if (wlItemRequest.RequestMsg.CheckPause())
                    {
                        wlItemStream.NumPausedRequestsCount--;
                    }

                    if (wlItemRequest.RequestMsg.CheckHasView() && wlItemStream.RequestsWithViewCount > 0)
                    {
                        RemoveRequestView(wlItemStream, wlItemRequest, out _);
                        wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                    }
                    else if (wlItemStream.RequestsWithViewCount > 0)
                    {
                        wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                    }

                    if (wlItemStream.State.StreamState() == StreamStates.OPEN || wlItemStream.RequestPending)
                    {
                        // Stream is open; need to change priority or close it.
                        if (wlItemStream.UserRequestDlList.Count() == 0)
                        {
                            CloseWlItemStream((WlItemStream)wlItemRequest.WatchlistStream!);

                            msg.Copy(m_CloseMsg, CopyMsgFlags.NONE);

                            WlItemStream tempWlItemStream = (WlItemStream)wlItemRequest.WatchlistStream!;

                            m_CloseMsg.StreamId = tempWlItemStream.StreamId;

                            ret = tempWlItemStream.SendMsg(m_CloseMsg, submitOptions, false, out errorInfo);

                            if(ret == ReactorReturnCode.NO_BUFFERS)
                            {
                                ret = ReactorReturnCode.SUCCESS;
                            }

                            if (ret < ReactorReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            // If inside dispatch reading a message on this stream,
                            // don't repool it yet. The fanout will still be accessing it, e.g. iterating over its UserRequestList.
                            // If not in dispatch, however, it's safe to repool it now.
                            if (wlItemStream == m_CurrentFanoutStream)
                                m_CurrentFanoutStream = null;
                            else
                                tempWlItemStream.ReturnToPool();
                        }
                        else
                        {
                            WlItemStream tempWlItemStream = (WlItemStream)wlItemRequest.WatchlistStream!;

                            // update priority
                            // reduce stream priority count by that in user request being closed
                            int streamPriorityCount = tempWlItemStream.RequestMsg.CheckHasPriority() ?
                                    tempWlItemStream.RequestMsg.Priority.Count : 1;
                            int userRequestPriorityCount = wlItemRequest.RequestMsg.CheckHasPriority() ?
                                    wlItemRequest.RequestMsg.Priority.Count : 1;
                            tempWlItemStream.RequestMsg.Priority.Count = streamPriorityCount - userRequestPriorityCount;

                            // resend
                            bool needRefresh = ((tempWlItemStream.RequestMsg.Flags & RequestMsgFlags.NO_REFRESH) == 0);

                            if ((wlItemStream.Flags & WlItemStream.StatusFlags.PENDING_VIEW_CHANGE) == 0)
                            {
                                tempWlItemStream.RequestMsg.Flags |= RequestMsgFlags.NO_REFRESH;
                            }

                            tempWlItemStream.SendMsg(tempWlItemStream.RequestMsg, submitOptions, true, out errorInfo);

                            // Remove the RequestMsgFlags.NO_REFRESH flag if it is set temporary in the SendMsg() method.
                            if (!needRefresh)
                            {
                                tempWlItemStream.RequestMsg.Flags &= ~RequestMsgFlags.NO_REFRESH;
                            }
                        }
                    }

                    // close watchlist request
                    CloseWlRequest(wlItemRequest);
                    RepoolWlRequest(wlItemRequest);
                    break;
                }
            }

            RequestTimeoutDlList.Remove(wlItemRequest, WlItemRequest.TIMEOUT_ITEM_REQUEST_LINK);

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorReturnCode RemoveUserRequestFromClosedStream(WlItemRequest wlItemRequest)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;

            // remove from UserStreamIdListToRecover list
            LinkedListNode<int>? streamIdNode = UserStreamIdListToRecover.First;
            while (streamIdNode != null)
            {
                int listStreamId = streamIdNode.Value;

                if (listStreamId == wlItemRequest.RequestMsg.StreamId)
                {
                    UserStreamIdListToRecover.Remove(streamIdNode);
                    break;
                }

                streamIdNode = streamIdNode.Next;
            }

            // remove from StatusMsgDispatchList list
            LinkedListNode<IStatusMsg>? statusMsgNode = StatusMsgDispatchList.First;
            while (statusMsgNode != null)
            {
                IStatusMsg statusMsg = statusMsgNode.Value;

                if (statusMsg.StreamId == wlItemRequest.RequestMsg.StreamId)
                {
                    StatusMsgDispatchList.Remove(statusMsgNode);
                    StatusMsgPool.AddLast(statusMsg);
                    break;
                }
                statusMsgNode = statusMsgNode.Next;
            }

            // Remove from PendingRequestByIdDict
            foreach (var kvp in PendingRequestByIdDict)
            {
                LinkedList<WlItemRequest> pendingRequests = kvp.Value;
                LinkedListNode<WlItemRequest>? itemNode = pendingRequests.First;
                LinkedListNode<WlItemRequest>? nextNode;
                while (itemNode != null)
                {
                    WlItemRequest itemInList = itemNode.Value;
                    nextNode = itemNode.Next;
                    if (itemInList.RequestMsg.StreamId == wlItemRequest.RequestMsg.StreamId)
                    {
                        pendingRequests.Remove(itemNode);
                    }

                    itemNode = nextNode;
                }

                if (pendingRequests.Count == 0)
                {
                    PendingRequestByIdDict.Remove(kvp.Key);
                    PendingRequestListPool.AddLast(pendingRequests);
                }
            }

            // Remove from PendingRequestByNameDict
            foreach (var kvp in PendingRequestByNameDict)
            {
                LinkedList<WlItemRequest> pendingRequests = kvp.Value;
                LinkedListNode<WlItemRequest>? itemNode = pendingRequests.First;
                LinkedListNode<WlItemRequest>? nextNode;
                while (itemNode != null)
                {
                    WlItemRequest itemInList = itemNode.Value;
                    nextNode = itemNode.Next;
                    if (itemInList.RequestMsg.StreamId == wlItemRequest.RequestMsg.StreamId)
                    {
                        pendingRequests.Remove(itemNode);
                    }

                    itemNode = nextNode;
                }

                if (pendingRequests.Count == 0)
                {
                    PendingRequestByNameDict.Remove(kvp.Key);
                    PendingRequestListPool.AddLast(pendingRequests);
                }
            }

            CloseWlRequest(wlItemRequest);
            RepoolWlRequest(wlItemRequest);

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode SendStatus(int streamId, int domainType, String text, bool privateStream, out ReactorErrorInfo? errorInfo)
        {
            // populate StatusMsg
            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.StreamId = streamId;
            m_StatusMsg.DomainType = domainType;
            m_StatusMsg.ApplyHasState();

            if (!privateStream && (m_Watchlist.LoginHandler.SupportSingleOpen || m_Watchlist.LoginHandler.SupportAllowSuspectData))
            {
                m_StatusMsg.State.StreamState(StreamStates.OPEN);
            }
            else
            {
                m_StatusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            }
            m_StatusMsg.State.DataState(DataStates.SUSPECT);
            m_StatusMsg.State.Text().Data(text);

            // callback user
            WlRequest? tempWlRequest;
            m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(streamId, out tempWlRequest);
            return CallbackUserWithMsg("WlItemHandler.SendStatus", m_StatusMsg, tempWlRequest!, out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal static bool ValidateQos(Qos qos)
        {
            if(qos.Timeliness() == QosTimeliness.UNSPECIFIED || qos.Rate() == QosRates.UNSPECIFIED ||
                qos.Timeliness() > QosTimeliness.DELAYED || qos.Rate() > QosRates.TIME_CONFLATED)
            {
                return false;
            }

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode SubmitRequest(WlRequest wlRequest, IRequestMsg requestMsg, bool isReissue,ReactorSubmitOptions submitOptions, 
            out ReactorErrorInfo? errorInfo)
        {
            if (!isReissue) // This is a new request
            {
                // Validate Qos
                Qos msgQos;
                if (requestMsg.CheckHasQos() && (msgQos = requestMsg.Qos) != null)
                {
                    if (!ValidateQos(msgQos))
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID,
                            "WlItemHandler.SubmitRequest", $"Request has invalid QoS (Timeliness: {requestMsg.Qos.Timeliness()}," +
                            $" Rate: {requestMsg.Qos.Rate()}).");
                    }
                }

                // Validate WorstQos
                if (requestMsg.CheckHasWorstQos() && requestMsg.WorstQos != null)
                {
                    if (!ValidateQos(requestMsg.WorstQos))
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID,
                            "WlItemHandler.SubmitRequest", $"Request has invalid worst QoS (Timeliness: {requestMsg.WorstQos.Timeliness()}," +
                            $" Rate: {requestMsg.WorstQos.Rate()}).");
                    }
                }

                if (requestMsg.CheckHasBatch())
                {
                    // handle batch request
                    return HandleBatchRequest((WlItemRequest)wlRequest, requestMsg, submitOptions, out errorInfo);
                }
                else
                {
                    return HandleItemRequest((WlItemRequest)wlRequest, requestMsg, submitOptions, true, out errorInfo);
                }
            }
            else // This is reissue request
            {
                return HandleItemReissue((WlItemRequest)wlRequest, requestMsg, submitOptions, out errorInfo);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleBatchRequest(WlItemRequest wlRequest, IRequestMsg requestMsg, ReactorSubmitOptions submitOptions, 
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            ElementList elementList = new ElementList();
            ElementEntry elementEntry = new ElementEntry();
            Array batchArray = new Array();
            ArrayEntry batchArrayEntry = new ArrayEntry();
            bool foundBatch = false;
            int originalStreamId = requestMsg.StreamId;
            int currentStreamId = requestMsg.StreamId;
            Buffer itemName = new Buffer();
            Buffer? encodedDataBody = null;
            LinkedList<string> itemNames = new();
            CodecReturnCode retCodecVal;
            Dictionary<string, WlItemRequest> itemNameDict = new();
            SubmitBatchRequestHandler submitBatchRequestHandler = SubmitBatchRequest;

            // Make sure requestMsg does not have item name set on Msgkey
            if (requestMsg.MsgKey.CheckHasName())
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "WlItemHandler.HandleBatchRequest", "Requested batch has name in message key.");
            }

            if(requestMsg.DomainType == (int)DomainType.SYMBOL_LIST)
            {
                if ((ret = ExtractSymbolListFromMsg(wlRequest, requestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            m_BatchDecodeIt.Clear();
            m_BatchDecodeIt.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                    m_Watchlist.ReactorChannel!.MinorVersion);

            wlRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
            wlRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

            m_Watchlist.StreamIdToWlRequestDict!.Add(requestMsg.StreamId, wlRequest);

            if (requestMsg.ContainerType == DataTypes.ELEMENT_LIST)
            {
                if ((retCodecVal = elementList.Decode(m_BatchDecodeIt, null)) <= CodecReturnCode.FAILURE)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "WlItemHandler.HandleBatchRequest",
                            $"ElementList.Decode() failure with error code: {retCodecVal.GetAsString()}.");
                }

                int entryCount = 0;
                int itemListDataSize = 0;

                // check element list for itemList
                while ((retCodecVal = elementEntry.Decode(m_BatchDecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                {
                    if (retCodecVal <= CodecReturnCode.FAILURE)
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.FAILURE,
                                "WlItemHandler.HandleBatchRequest",
                                $"ElementEntry.Decode() failure with error code: {retCodecVal.GetAsString()}.");
                    }

                    entryCount++;

                    if (elementEntry.Name.ToString().Contains(ElementNames.BATCH_ITEM_LIST.ToString()))
                    {
                        itemListDataSize = ElementNames.BATCH_ITEM_LIST.Length;
                        itemListDataSize += elementEntry.EncodedData.Length;

                        foundBatch = true;

                        if ((retCodecVal = batchArray.Decode(m_BatchDecodeIt)) <= CodecReturnCode.FAILURE)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                    ReactorReturnCode.FAILURE,
                                    "WlItemHandler.HandleBatchRequest",
                                    $"Array.Decode() failure with error code: {retCodecVal.GetAsString()}.");
                        }

                        WlItemRequest? value;
                        while ((retCodecVal = batchArrayEntry.Decode(m_BatchDecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if ((retCodecVal = itemName.Decode(m_BatchDecodeIt)) == CodecReturnCode.SUCCESS)
                            {
                                string itemNameStr = itemName.ToString();
                                if (itemNameDict.TryGetValue(itemNameStr, out value))
                                {
                                    if(value.BatchRequestHelper is null)
                                    {
                                        value.BatchRequestHelper = new WlBatchRequestHelper();
                                        submitBatchRequestHandler = SubmitBatchRequestWithHelper;
                                    }

                                    WlItemRequest tempItemRequest = m_Watchlist.CreateWlItemRequest();
                                    tempItemRequest.BatchRequestHelper = value.BatchRequestHelper;

                                    value.BatchRequestHelper.WlItemRequestList.Add(tempItemRequest);
                                }
                                else
                                {
                                    itemNameDict[itemNameStr] = m_Watchlist.CreateWlItemRequest();
                                    itemNames.AddLast(itemNameStr);
                                }
                            }
                            else
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                        ReactorReturnCode.FAILURE,
                                        "WlItemHandler.HandleBatchRequest",
                                        $"Invalid BLANK_DATA while decoding :ItemList -- {retCodecVal.GetAsString()}");
                            }
                        }
                    }

                    // Found the batch item list and others entry
                    if (foundBatch && (entryCount > 1))
                    {
                        // Encodes a separate ElementList for Element names which is not itemList:
                        Buffer encodedBuffer = new Buffer();
                        encodedBuffer.Data(new ByteBuffer(requestMsg.EncodedDataBody.Length - itemListDataSize));
                        EncodeIterator encodeIter = new EncodeIterator();
                        ElementEntry elementEntryEnc = new ElementEntry();
                        ElementList elementListEnc = new ElementList();
                        DecodeIterator decodeIter = new DecodeIterator();

                        encodeIter.Clear();
                        encodeIter.SetBufferAndRWFVersion(encodedBuffer, m_Watchlist.ReactorChannel!.MajorVersion,
                            m_Watchlist.ReactorChannel!.MinorVersion);

                        elementListEnc.Clear();

                        // Copies attributes from the original ElementList
                        elementListEnc.Flags = elementList.Flags;
                        if (elementList.CheckHasInfo())
                        {
                            elementListEnc.ElementListNum = elementList.ElementListNum;
                        }

                        if (elementList.CheckHasSetData())
                        {
                            elementListEnc.EncodedSetData = elementList.EncodedSetData;
                        }

                        if (elementList.CheckHasSetId())
                        {
                            elementListEnc.SetId = elementList.SetId;
                        }

                        if ((retCodecVal = elementListEnc.EncodeInit(encodeIter, null, 0)) <= CodecReturnCode.FAILURE)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                    ReactorReturnCode.FAILURE,
                                    "WlItemHandler.HandleBatchRequest",
                                    $"ElementList.EncodeInit() failure with error code: {retCodecVal.GetAsString()}.");
                        }

                        decodeIter.Clear();
                        decodeIter.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                                m_Watchlist.ReactorChannel!.MinorVersion);

                        if ((retCodecVal = elementList.Decode(decodeIter, null)) <= CodecReturnCode.FAILURE)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                    ReactorReturnCode.FAILURE,
                                    "WlItemHandler.HandleBatchRequest",
                                    $"ElementList.Decode() failure with error code: {retCodecVal.GetAsString()}.");
                        }

                        while ((retCodecVal = elementEntry.Decode(decodeIter)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if (retCodecVal <= CodecReturnCode.FAILURE)
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                        ReactorReturnCode.FAILURE,
                                        "WlItemHandler.HandleBatchRequest",
                                        $"ElementEntry.Decode() failure with error code: {retCodecVal.GetAsString()}.");
                            }

                            if (!elementEntry.Name.ToString().Contains(ElementNames.BATCH_ITEM_LIST.ToString()))
                            {
                                elementEntryEnc.Clear();
                                elementEntryEnc.Name = elementEntry.Name;
                                elementEntryEnc.DataType = elementEntry.DataType;
                                elementEntryEnc.EncodedData = elementEntry.EncodedData;

                                retCodecVal = elementEntryEnc.Encode(encodeIter);
                                if (retCodecVal <= CodecReturnCode.FAILURE)
                                {
                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE,
                                            "WlItemHandler.HandleBatchRequest",
                                            $"ElementEntry.encode() failure with error code: {retCodecVal.GetAsString()}.");
                                }
                            }
                        }

                        if ((retCodecVal = elementListEnc.EncodeComplete(encodeIter, true)) <= CodecReturnCode.FAILURE)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                    ReactorReturnCode.FAILURE,
                                    "WlItemHandler.HandleBatchRequest",
                                    $"ElementList.EncodeComplete() failure with error code: {retCodecVal.GetAsString()}.");
                        }

                        encodedDataBody = encodeIter.Buffer();

                        break; // Breaks from the ElementList of the request message
                    }
                }

                if (!foundBatch)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "WlItemHandler.HandleBatchRequest",
                            ":ItemList not found.");
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlItemHandler.HandleBatchRequest",
                        "Unexpected container type or decoding error.");
            }

            // found itemList, thus a batch. Make individual item requests from array of requests
            /* Start at stream ID after batch request. */
            currentStreamId++;

            if (m_Watchlist.StreamIdToWlRequestDict.ContainsKey(currentStreamId) == true)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlItemHandler.HandleBatchRequest",
                        "Item in batch has same ID as existing stream.");
            }

            Dictionary<int, WlItemRequest> wlRequestDict = new (itemNames.Count);
            Dictionary<int, IRequestMsg> requestMsgDict = new (itemNames.Count);
            int possibleStreamId = currentStreamId;

            LinkedListNode<string>? currentNode = itemNames.First;
            LinkedListNode<string>? nextNode;
            while (currentNode != null)
            {
                nextNode = currentNode.Next;
                itemName.Data(currentNode.Value);
                itemNames.Remove(currentNode);

                WlItemRequest newWlRequest = itemNameDict[currentNode.Value];
                bool repeatSameItemName = false;

                do
                {
                    // Checks whether the possible stream Id is alrady existed
                    if (m_Watchlist.StreamIdToWlRequestDict.ContainsKey(possibleStreamId))
                    {
                        while (wlRequestDict.Count > 0)
                        {
                            if (wlRequestDict.Remove(currentStreamId, out WlItemRequest? removeWlRequest))
                            {
                                RepoolWlRequest(removeWlRequest);
                                requestMsgDict.Remove(currentStreamId);
                                currentStreamId++;
                            }
                        }
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.FAILURE,
                                "WlItemHandler.HandleBatchRequest",
                                "Item in batch has same ID as existing stream.");
                    }

                    if (newWlRequest.BatchRequestHelper != null)
                    {
                        repeatSameItemName = true;
                        int requestIndex = newWlRequest.BatchRequestHelper.ItemRequestIndex;
                        if (requestIndex != -1)
                        {
                            newWlRequest = newWlRequest.BatchRequestHelper.WlItemRequestList[requestIndex];
                        }

                        newWlRequest.BatchRequestHelper!.ItemRequestIndex++;

                        /* Break after iterating for every request with the same item name. */
                        if( newWlRequest.BatchRequestHelper!.ItemRequestIndex == newWlRequest.BatchRequestHelper.WlItemRequestList.Count)
                        {
                            repeatSameItemName = false;
                        }
                    }

                    IRequestMsg newRequestMsg = new Msg();

                    // Create item list request and new watchlist request based off old watchlist request
                    newWlRequest.WatchlistStream = wlRequest.WatchlistStream;


                    // Remove batch flag and do not copy the encoded data body from the batch request
                    requestMsg.Copy(newRequestMsg, CopyMsgFlags.ALL_FLAGS & (~CopyMsgFlags.DATA_BODY));
                    newRequestMsg.Flags &= ~RequestMsgFlags.HAS_BATCH;
                    newRequestMsg.StreamId = possibleStreamId;

                    newRequestMsg.MsgClass = MsgClasses.REQUEST;

                    // Set msgKey item name
                    newRequestMsg.ApplyMsgKeyInUpdates();
                    newRequestMsg.MsgKey.ApplyHasName();
                    newRequestMsg.MsgKey.Name = itemName;

                    // Set encoded data body which does not have the :ItemList entry name
                    if (encodedDataBody != null)
                    {
                        newRequestMsg.EncodedDataBody = encodedDataBody;
                    }
                    else
                    {
                        // Unset the container type when there is no encoded databody
                        newRequestMsg.ContainerType = DataTypes.NO_DATA;
                    }

                    if ((retCodecVal = newRequestMsg.Copy(newWlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS)) <= CodecReturnCode.FAILURE)
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.FAILURE,
                                "WlItemHandler.HandleBatchRequest",
                                "RequestMsg.Copy() failure.");
                    }

                    wlRequestDict.Add(possibleStreamId, newWlRequest);
                    requestMsgDict.Add(possibleStreamId, newRequestMsg);

                    possibleStreamId++;

                } while (repeatSameItemName);

                currentNode = nextNode;
            }

            if ( (ret = submitBatchRequestHandler(submitOptions, wlRequestDict, requestMsgDict, currentStreamId,
                out errorInfo)) != ReactorReturnCode.SUCCESS)
            {
                return ret;
            }

            /* Requests created. Make a request for the batch stream so it can be acknowledged. */
            IStatusMsg statusMsg = GetStatusMsgFromPool();
            statusMsg.DomainType = requestMsg.DomainType;
            statusMsg.StreamId = originalStreamId;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED);
            statusMsg.State.DataState(DataStates.OK);
            Buffer statusText = new Buffer();
            statusText.Data("Stream closed for batch");
            statusMsg.State.Text(statusText);

            StatusMsgDispatchList.AddLast(statusMsg);

            if (StatusMsgDispatchList.Count == 1)
            {
                // trigger dispatch only for first add to list
                m_reactor.SendWatchlistDispatchNowEvent(m_Watchlist.ReactorChannel!);
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode SubmitBatchRequest(ReactorSubmitOptions submitOptions, IDictionary<int, WlItemRequest> wlItemRequestDict,
            IDictionary<int, IRequestMsg> streamIdToReqMsgDict, int currentStreamId, out ReactorErrorInfo? errorInfo)
        {
            int count = wlItemRequestDict.Count;
            errorInfo = null;

            while (count > 0)
            {
                // Add watchlist request to request table
                WlItemRequest tempWlItemRequest = wlItemRequestDict[currentStreamId];
                m_Watchlist.StreamIdToWlRequestDict!.Add(currentStreamId, tempWlItemRequest);
                ReactorReturnCode ret = HandleItemRequest(tempWlItemRequest, streamIdToReqMsgDict[currentStreamId], submitOptions, 
                    true, out errorInfo);
                if (ret <= ReactorReturnCode.FAILURE)
                {
                    return ret;
                }

                count--;
                currentStreamId++;
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode SubmitBatchRequestWithHelper(ReactorSubmitOptions submitOptions, IDictionary<int, WlItemRequest> wlItemRequestDict,
            IDictionary<int, IRequestMsg> streamIdToReqMsgDict, int currentStreamId, out ReactorErrorInfo? errorInfo)
        {
            int count = wlItemRequestDict.Count;
            WlBatchRequestHelper? batchReqHelper;
            ReactorReturnCode ret;
            errorInfo = null;

            while (count > 0)
            {
                count--;

                // Add watchlist request to request table
                WlItemRequest tempWlItemRequest = wlItemRequestDict[currentStreamId];

                batchReqHelper = tempWlItemRequest.BatchRequestHelper;

                if (batchReqHelper != null)
                {
                    batchReqHelper.WlRequestDict = wlItemRequestDict;
                    batchReqHelper.RequestMsgDict = streamIdToReqMsgDict;
                    batchReqHelper.CurrentStreamId = currentStreamId;

                    /* Checks whether there is an item stream for the duplication item name. */
                    WlService? wlService;
                    IRequestMsg requestMsg = streamIdToReqMsgDict[currentStreamId];
                    if (submitOptions.ServiceName != null)
                    {
                        wlService = m_Watchlist.DirectoryHandler!.ServiceCache.Service(submitOptions.ServiceName);
                    }
                    else
                    {
                        wlService = m_Watchlist.DirectoryHandler!.ServiceCache.Service(requestMsg.MsgKey.ServiceId);
                    }

                    m_TempMatchedQos.Clear();

                    // first determine if item can be opened at this time.
                    Qos? staticQos = ((tempWlItemRequest.ItemReqFlags & WlItemRequest.Flags.HAS_STATIC_QOS) != 0)
                        ? tempWlItemRequest.MatchedQos
                        : null;
                    WlItemStream? wlItemStream = null;
                    if (wlService != null && CanItemBeOpened(requestMsg, m_TempMatchedQos, staticQos, wlService.RdmService!, out errorInfo))
                    {
                        if (requestMsg.DomainType == (int)DomainType.DICTIONARY || IsWindowOpen(wlService))
                        {
                            // find aggregation stream
                            wlItemStream = FindItemAggregationStream(requestMsg, m_TempMatchedQos, submitOptions, m_TempWlStreamAttributes);
                        }
                    }

                    m_Watchlist.StreamIdToWlRequestDict!.Add(currentStreamId, tempWlItemRequest);
                    ret = HandleItemRequest(tempWlItemRequest, requestMsg, submitOptions, true, out errorInfo);
                    if (ret <= ReactorReturnCode.FAILURE)
                    {
                        return ret;
                    }

                    if (wlItemStream != null)
                    {   /* The item stream is already existed so submit the reaming item requests individually */
                        for (int index = 0; index < batchReqHelper.WlItemRequestList.Count; index++)
                        {
                            ++currentStreamId;
                            WlItemRequest nextItemRequest = batchReqHelper.WlItemRequestList[index];
                            m_Watchlist.StreamIdToWlRequestDict!.Add(currentStreamId, nextItemRequest);
                            ret = HandleItemRequest(nextItemRequest, streamIdToReqMsgDict[currentStreamId], submitOptions, true, out errorInfo);
                            if (ret <= ReactorReturnCode.FAILURE)
                            {
                                return ret;
                            }

                            nextItemRequest.BatchRequestHelper = null;
                        }
                    }
                    else
                    {
                        currentStreamId += batchReqHelper.WlItemRequestList.Count;
                    }

                    tempWlItemRequest.BatchRequestHelper = null;
                    count -= batchReqHelper.WlItemRequestList.Count;
                    batchReqHelper.WlItemRequestList.Clear();
                }
                else
                {   /* Handles unique item names in the batch request */
                    m_Watchlist.StreamIdToWlRequestDict!.Add(currentStreamId, tempWlItemRequest);
                    ret = HandleItemRequest(tempWlItemRequest, streamIdToReqMsgDict[currentStreamId], submitOptions, true, out errorInfo);
                    if (ret <= ReactorReturnCode.FAILURE)
                    {
                        return ret;
                    }
                }
     
                currentStreamId++;
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleBatchRequests(WlItemStream wlItemStream, WlItemRequest wlItemRequest, 
            ReactorSubmitOptions submitOptions, 
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            errorInfo = null;
            WlBatchRequestHelper batchReqHelper = wlItemRequest.BatchRequestHelper!;
            int currentStreamId = batchReqHelper!.CurrentStreamId;
            for (int index = 0; index < batchReqHelper.WlItemRequestList.Count; index++)
            {
                WlItemRequest nextItemRequest = batchReqHelper.WlItemRequestList[index];
                int nextStreamId = ++currentStreamId;

                m_Watchlist.StreamIdToWlRequestDict!.Add(nextStreamId, nextItemRequest);

                nextItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                nextItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

                // set stream associated with request
                nextItemRequest.WatchlistStream = wlItemStream;

                IRequestMsg nextRequestMsg = batchReqHelper.RequestMsgDict![nextStreamId];

                if (nextRequestMsg.DomainType == (int)DomainType.SYMBOL_LIST)
                {
                    if ((ret = ExtractSymbolListFromMsg(nextItemRequest, nextRequestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        return ret;
                }
                else
                {
                    if (nextRequestMsg.CheckHasView())
                    {
                        if ((ret = ExtractViewFromMsg(nextItemRequest, nextRequestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }

                    if (nextRequestMsg.CheckHasView())
                    {
                        if ((ret = HandleViews(nextItemRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                            return ret;
                    }
                }

                // add request to stream
                wlItemStream.UserRequestDlList.PushBack(nextItemRequest);

                // handle pause request for this stream
                if (nextItemRequest.RequestMsg.CheckPause() && nextItemRequest.RequestMsg.CheckStreaming())
                {
                    wlItemStream.NumPausedRequestsCount++;
                }

                // update request state to PENDING_REFRESH
                nextItemRequest.ReqState = WlRequest.State.PENDING_REFRESH;

                if (nextRequestMsg.CheckStreaming())
                {
                    // Update priority for streaming requests only
                    if (nextRequestMsg.CheckHasPriority())
                    {
                        // use priorityClass of request if greater than existing one
                        if (nextRequestMsg.Priority.PriorityClass > wlItemStream.RequestMsg.Priority.PriorityClass)
                        {
                            wlItemStream.RequestMsg.Priority.PriorityClass = nextRequestMsg.Priority.PriorityClass;
                        }

                        // add priorityCount to that of existing one
                        if (!wlItemStream.RequestMsg.CheckStreaming())
                            wlItemStream.RequestMsg.Priority.Count = nextRequestMsg.Priority.Count;
                        else
                            wlItemStream.RequestMsg.Priority.Count += nextRequestMsg.Priority.Count;
                    }
                    else // request has no priority, assume default of 1/1
                    {
                        wlItemStream.RequestMsg.ApplyHasPriority();
                        if (!wlItemStream.RequestMsg.CheckStreaming())
                            wlItemStream.RequestMsg.Priority.Count = 1;
                        else
                            wlItemStream.RequestMsg.Priority.Count += 1;
                    }

                    if (!wlItemStream.RequestMsg.CheckStreaming())
                        wlItemStream.RequestMsg.ApplyStreaming();
                }

                nextItemRequest.BatchRequestHelper = null;
            } // End for loop

            batchReqHelper.CurrentStreamId = currentStreamId;
            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleNonAggregatedRequest(WlItemRequest wlItemRequest, 
            IRequestMsg requestMsg,
            WlStreamAttributes itemAggregationKey,
            WlService? wlService,
            bool sendNow,
            ReactorSubmitOptions submitOptions,
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            errorInfo = null;
            int requestMsgFlags = requestMsg.Flags;
            bool privateStream = (requestMsgFlags & RequestMsgFlags.PRIVATE_STREAM) > 0;
            var wlItemStream = m_Watchlist.StreamManager.GetItemStream();

            if (wlItemStream == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "WlItemHandler.HandleNonAggregatedRequest",
                    "Failed to retrieve WlStream object while opening new stream since no free StreamIDs are available");
            }

            SetNewStream(requestMsg, wlItemStream);         

            wlItemRequest.WatchlistStream = wlItemStream;
            wlItemStream!.UserRequestDlList.PushBack(wlItemRequest);
            var streamRequest = wlItemStream.RequestMsg;

            if ((requestMsgFlags & RequestMsgFlags.HAS_VIEW) > 0 && requestMsg.DomainType != (int)DomainType.SYMBOL_LIST)
            {
                if ((ret = HandleViews(wlItemRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            // copy temporary item aggregation key from FindItemAggregationStream() into new one if not private stream
            if (!privateStream)
            {
                requestMsg.Copy(streamRequest, DEFAULT_COPY_FLAGS);
                streamRequest.ContainerType = DataTypes.NO_DATA;
            }
            else
            {
                requestMsg.Copy(streamRequest, PRIVATE_STREAM_COPY_FLAGS);
                if ((requestMsgFlags & RequestMsgFlags.HAS_VIEW) > 0)
                {
                    streamRequest.ContainerType = DataTypes.ELEMENT_LIST;
                    streamRequest.ApplyHasView();
                    streamRequest.EncodedDataBody = m_ViewElementList;
                }
            }

            streamRequest.StreamId = wlItemStream.StreamId;
            streamRequest.MsgKey.ApplyHasServiceId();
            streamRequest.MsgKey.ServiceId = itemAggregationKey.MsgKey.ServiceId;

            // Qos from the item aggregation key if not the dictionary domain
            if (requestMsg.DomainType != (int)DomainType.DICTIONARY)
            {
                streamRequest.Flags |= RequestMsgFlags.HAS_QOS;
                itemAggregationKey.Qos.Copy(streamRequest.Qos);

                // clear worst Qos flag
                streamRequest.Flags &= ~RequestMsgFlags.HAS_WORST_QOS;
            }
          
            if ((requestMsgFlags & RequestMsgFlags.HAS_PRIORITY) > 0) // priority is that of request or 1/1 if not present
            {
                streamRequest.ApplyHasPriority();
                streamRequest.Priority.PriorityClass = requestMsg.Priority.PriorityClass;
                streamRequest.Priority.Count = requestMsg.Priority.Count;
            }
            else if (!privateStream) // request has no priority, use default of 1/1 if not private stream.
            {
                streamRequest.ApplyHasPriority();
                streamRequest.Priority.PriorityClass = 1;
                streamRequest.Priority.Count = 1;
            }

            /* Additional handling for the batch request with the same item name */
            if (wlItemRequest.BatchRequestHelper != null)
            {
                if ((ret = HandleBatchRequests(wlItemStream, wlItemRequest, submitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            // reset MSG_KEY_IN_UPDATES flag if set
            streamRequest.Flags &= ~RequestMsgFlags.MSG_KEY_IN_UPDATES;

            // Handles the pause flag
            if ((requestMsgFlags & RequestMsgFlags.PAUSE) > 0 && (requestMsgFlags & RequestMsgFlags.STREAMING) > 0)
            {
                wlItemStream.NumPausedRequestsCount = 1;
            }

            // send message to stream
            if (sendNow)
            {
                ret = wlItemStream.SendMsg(streamRequest, submitOptions, false, out errorInfo);

                if (ret == ReactorReturnCode.NO_BUFFERS)
                    ret = ReactorReturnCode.SUCCESS; // This request is added to pending queue.

                wlItemStream.RefreshState = streamRequest.CheckHasView() ?
                    WlItemStream.RefreshStateFlag.PENDING_VIEW_REFRESH :
                    WlItemStream.RefreshStateFlag.PENDING_REFRESH;
            }
            else // if not send now, add stream to pending send message list
            {
                ret = ReactorReturnCode.SUCCESS;
                PendingSendMsgDlList.PushBack(wlItemStream);
                wlItemStream.PendingSendMsg = true;
            }

            // increment number of outstanding requests if not dictionary domain.
            if (requestMsg.DomainType != (int)DomainType.DICTIONARY && (requestMsgFlags & RequestMsgFlags.NO_REFRESH) == 0)
            {
                wlService!.NumOutstandingRequests++;
            }

            // update tables and list if send successful
            if (ret >= ReactorReturnCode.SUCCESS)
            {
                wlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                wlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

                // save matched Qos if not dictionary
                if (requestMsg.DomainType != (int)DomainType.DICTIONARY)
                {
                    m_TempMatchedQos.Copy(wlItemRequest.MatchedQos);
                }

                // add at the end of the list of streams
                if (!wlItemStream.PendingSendMsg)
                {
                    StreamDlList.PushBack(wlItemStream);
                }
               
                // add stream to watchlist table
                // m_Watchlist.StreamIdToWlStreamDict!.Add(wlItemStream.StreamId, wlItemStream);
                wlItemStream.InUse = true;

                // add to the WlStreamAttribToWlStreamiDict and associate with item aggregation key
                // if not a private stream
                if (!privateStream)
                {
                    WlStreamAttribToWlStreamDict!.Add(itemAggregationKey, wlItemStream);
                    wlItemStream.StreamAttributes = itemAggregationKey;
                }
                else
                {
                    itemAggregationKey.ReturnToPool();
                }

                // associate stream and service
                wlItemStream.WlService = wlService;

                // add stream to service's stream list
                wlService!.StreamIdDlList.PushBack(wlItemStream.StreamIdLink[0]!);

                // update request state
                wlItemRequest.ReqState = WlRequest.State.PENDING_REFRESH;
            }
            else // fails to send the message
            {
                itemAggregationKey.ReturnToPool();
                wlItemStream.ReturnToPool();
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleAggregatedRequest(WlItemStream wlItemStream, 
            WlItemRequest wlItemRequest, 
            ReactorSubmitOptions submitOptions, 
            IRequestMsg requestMsg,
            bool sendNow,
            WlService wlService,
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            errorInfo = null;
            wlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
            wlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

            /* Validate whether the view type of the item request matches with the item stream */
            if (requestMsg.CheckHasView() && wlItemStream.AggregateView != null)
            {
                if (wlItemRequest.ViewType != wlItemStream.AggregateView.ViewType)
                {
                    IStatusMsg statusMsg = GetStatusMsgFromPool();
                    statusMsg.DomainType = requestMsg.DomainType;
                    statusMsg.StreamId = requestMsg.StreamId;
                    statusMsg.ApplyHasState();
                    statusMsg.State.StreamState(StreamStates.CLOSED);
                    statusMsg.State.DataState(DataStates.SUSPECT);
                    Buffer statusText = new Buffer();
                    statusText.Data("ViewType mismatch, cannot be aggregated.");
                    statusMsg.State.Text(statusText);
                    statusMsg.State.Code(StateCodes.USAGE_ERROR);

                    StatusMsgDispatchList.AddLast(statusMsg);

                    if (StatusMsgDispatchList.Count == 1)
                    {
                        // trigger dispatch only for first add to list
                        m_reactor.SendWatchlistDispatchNowEvent(m_Watchlist.ReactorChannel!);
                    }

                    return ReactorReturnCode.SUCCESS;
                }
            }

            // save matched Qos if not dictionary
            if (requestMsg.DomainType != (int)DomainType.DICTIONARY)
            {
                m_TempMatchedQos.Copy(wlItemRequest.MatchedQos);
            }

            // set stream associated with request
            wlItemRequest.WatchlistStream = wlItemStream;

            // Add request to stream if the stream is not waiting for a refresh or the stream is
            // streaming but it is not waiting for multi-part and view pending.
            if (!wlItemStream.RequestPending || wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.NONE ||
                (wlItemStream.RequestMsg.CheckStreaming() && wlItemStream.RefreshState != WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE &&
                 wlItemStream.RefreshState != WlItemStream.RefreshStateFlag.PENDING_VIEW_REFRESH)
                )
            {
                // Saves the current flag of the stream
                WlItemStream.StatusFlags statusFlag = wlItemStream.Flags;

                if (requestMsg.CheckHasView())
                {
                    if ((ret = HandleViews(wlItemRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        return ret;
                }

                // add request to stream
                wlItemStream.UserRequestDlList.PushBack(wlItemRequest);

                // handle pause request for this stream
                if (wlItemRequest.RequestMsg.CheckPause() && wlItemRequest.RequestMsg.CheckStreaming())
                {
                    wlItemStream.NumPausedRequestsCount++;
                }

                // update request state to PENDING_REFRESH
                wlItemRequest.ReqState = WlRequest.State.PENDING_REFRESH;

                // retrieve request from stream
                IRequestMsg streamRequestMsg = wlItemStream.RequestMsg;

                if (!requestMsg.CheckStreaming())
                {
                    // Join the existing request if the current request haven't received a refresh yet.
                    if (wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_REFRESH)
                    {
                        return ReactorReturnCode.SUCCESS;
                    }
                }
                else
                {
                    // Update priority for streaming requests only
                    if (requestMsg.CheckHasPriority())
                    {
                        // use priorityClass of request if greater than existing one
                        if (requestMsg.Priority.PriorityClass > streamRequestMsg.Priority.PriorityClass)
                        {
                            streamRequestMsg.Priority.PriorityClass = requestMsg.Priority.PriorityClass;
                        }

                        // add priorityCount to that of existing one
                        if (!streamRequestMsg.CheckStreaming())
                            streamRequestMsg.Priority.Count = requestMsg.Priority.Count;
                        else
                            streamRequestMsg.Priority.Count += requestMsg.Priority.Count;
                    }
                    else // request has no priority, assume default of 1/1
                    {
                        streamRequestMsg.ApplyHasPriority();
                        if (!streamRequestMsg.CheckStreaming())
                            streamRequestMsg.Priority.Count = 1;
                        else
                            streamRequestMsg.Priority.Count += 1;
                    }

                    if (!streamRequestMsg.CheckStreaming())
                        streamRequestMsg.ApplyStreaming();

                    // Checks that the stream is still waiting for a response and there is no change for a refresh
                    if ((wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_REFRESH) && statusFlag == wlItemStream.Flags)
                    {
                        streamRequestMsg.ApplyNoRefresh();
                    }
                }

                if (sendNow)
                {
                    // increment number of outstanding requests if not dictionary domain and a request isn't currently pending
                    if (requestMsg.DomainType != (int)DomainType.DICTIONARY && !wlItemStream.RequestPending && !requestMsg.CheckNoRefresh())
                    {
                        wlService.NumOutstandingRequests++;
                    }

                    ret = wlItemStream.SendMsg(streamRequestMsg, submitOptions, false, out errorInfo);

                    if (ret == ReactorReturnCode.NO_BUFFERS)
                        ret = ReactorReturnCode.SUCCESS; // This request is added to pending queue.

                    wlItemStream.RefreshState = streamRequestMsg.CheckHasView() ?
                        WlItemStream.RefreshStateFlag.PENDING_VIEW_REFRESH :
                        WlItemStream.RefreshStateFlag.PENDING_REFRESH;
                }
                else // add stream to pending queue if not already there
                {
                    if (!wlItemStream.PendingSendMsg)
                    {
                        StreamDlList.Remove(wlItemStream);
                        PendingSendMsgDlList.PushBack(wlItemStream);
                        wlItemStream.PendingSendMsg = true;

                        // increment number of outstanding requests if not dictionary domain and a request isn't currently pending
                        if (requestMsg.DomainType != (int)DomainType.DICTIONARY && !wlItemStream.RequestPending && !requestMsg.CheckNoRefresh())
                        {
                            wlService.NumOutstandingRequests++;
                        }
                    }
                    else // stream is pending, if this request has no pause flag set while the pending stream has it set,
                         // remove pause flag from pending stream
                    {
                        IRequestMsg pendingRequestMsg = wlItemStream.RequestMsg;
                        if (!requestMsg.CheckPause() && pendingRequestMsg.CheckPause())
                        {
                            pendingRequestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                        }
                    }
                }
            }
            else
            {
                // WlItemStream is waiting a response for snapshot request and the user's request is snapshot without view
                if (!wlItemRequest.RequestMsg.CheckStreaming() && (wlItemStream.RefreshState == WlItemStream.RefreshStateFlag.PENDING_REFRESH) &&
                    !requestMsg.CheckStreaming() && !requestMsg.CheckHasView())
                {
                    // join the snapshot request to the esisting snapshot stream
                    wlItemRequest.ReqState = WlRequest.State.PENDING_REFRESH;
                    wlItemStream.UserRequestDlList.PushBack(wlItemRequest);
                }
                else
                {
                    // currently in the middle of snapshot, multi-part refresh or pending view
                    wlItemRequest.ReqState = WlRequest.State.PENDING_REQUEST;
                    wlItemStream.WaitingRequestDlList.PushBack(wlItemRequest, WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleItemRequest(WlItemRequest wlItemRequest, IRequestMsg requestMsg, ReactorSubmitOptions submitOptions, 
            bool sendNow, 
            out ReactorErrorInfo? errorInfo)
        {
            int requestMsgFlags = requestMsg.Flags;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            if (requestMsg.DomainType == (int)DomainType.SYMBOL_LIST)
            {
                if ((ret = ExtractSymbolListFromMsg(wlItemRequest, requestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    return ret;
            }

            if ((requestMsgFlags & RequestMsgFlags.HAS_VIEW) > 0)
            {
                if ((ret = ExtractViewFromMsg(wlItemRequest, requestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
            }

            m_TempMatchedQos.Clear();
            bool privateStream = (requestMsgFlags & RequestMsgFlags.PRIVATE_STREAM) > 0;

            // retrieve service for service ID/service name.
            WlService? wlService;
            if (submitOptions.ServiceName != null)
            {
                wlService = m_Watchlist.DirectoryHandler!.ServiceCache.Service(submitOptions.ServiceName);
            }
            else
            {
                wlService = m_Watchlist.DirectoryHandler!.ServiceCache.Service(requestMsg.MsgKey.ServiceId);
            }

            // first determine if item can be opened at this time.
            Qos? staticQos = ((wlItemRequest.ItemReqFlags & WlItemRequest.Flags.HAS_STATIC_QOS) != 0) 
                ? wlItemRequest.MatchedQos 
                : null;
            if (wlService != null && CanItemBeOpened(requestMsg, m_TempMatchedQos, staticQos, wlService.RdmService!, out errorInfo))
            {
                wlItemRequest.ServiceId = (uint)wlService.RdmService!.ServiceId;
                
                if (IsWindowOpen(wlService) || requestMsg.DomainType == (int)DomainType.DICTIONARY)
                {
                    WlStreamAttributes itemAggregationKey = m_Watchlist.CreateWlStreamAttributes();                  
                    WlItemStream? wlItemStream = FindItemAggregationStream(requestMsg, m_TempMatchedQos, submitOptions, itemAggregationKey);
                    if (wlItemStream == null)
                    {
                        if ((ret = HandleNonAggregatedRequest(wlItemRequest, 
                            requestMsg, 
                            itemAggregationKey, 
                            wlService, 
                            sendNow, 
                            submitOptions, 
                            out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                    else // item can be aggregated
                    {
                        itemAggregationKey.ReturnToPool();
                        ret = HandleAggregatedRequest(wlItemStream, wlItemRequest, submitOptions, requestMsg, sendNow, wlService, out errorInfo);
                        if (ret < ReactorReturnCode.SUCCESS)
                        {
                            return ret;
                        }
                    }
                }
                else // service window not open
                {
                    wlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                    wlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;
                    wlService.WaitingRequestList.AddLast(wlItemRequest);

                    if (wlItemRequest.BatchRequestHelper != null) /* Additional handling for the batch request with the same item name */
                    {
                        WlBatchRequestHelper batchReqHelper = wlItemRequest.BatchRequestHelper;
                        for (int index = 0; index < batchReqHelper.WlItemRequestList.Count; index++)
                        {
                            WlItemRequest tempWlItemRequest = batchReqHelper.WlItemRequestList[index];
                            tempWlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                            tempWlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;
                            tempWlItemRequest.BatchRequestHelper = null;
                            wlService.WaitingRequestList.AddLast(tempWlItemRequest);
                        }
                    }
                }
            }
            else // cannot open item at this time, add to pending request table if not private stream
            {
                // save stream info
                wlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                wlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

                string statusText = wlService is null 
                    ? "Service not available" 
                    : (errorInfo != null && errorInfo.Error != null ? errorInfo.Error.Text : string.Empty);

                // queue status message to send on next dispatch call
                if (UserStreamIdListToRecover.Count == 0 || sendNow)
                {
                    /* Additional handling for the batch request with the same item name */
                    if (wlItemRequest.BatchRequestHelper != null)
                    {
                        WlBatchRequestHelper batchReqHelper = wlItemRequest.BatchRequestHelper;
                        int currentStreamId = batchReqHelper.CurrentStreamId;
                        QueueStatusForDispatch(currentStreamId, requestMsg.DomainType, statusText, privateStream);
                        for (int index = 0; index < batchReqHelper.WlItemRequestList.Count; index++)
                        {
                            ++currentStreamId;
                            WlItemRequest tempWlItemRequest = batchReqHelper.WlItemRequestList[index];
                            m_Watchlist.StreamIdToWlRequestDict![currentStreamId] = tempWlItemRequest;
                            tempWlItemRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;
                            tempWlItemRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;
                            tempWlItemRequest.BatchRequestHelper = null;
                            QueueStatusForDispatch(currentStreamId, requestMsg.DomainType, statusText, privateStream);
                        }
                    }
                    else
                    {
                        QueueStatusForDispatch(requestMsg.StreamId, requestMsg.DomainType, statusText, privateStream);
                    }
                }
                else
                {
                    // only queue status in case of close recover
                    if (UserStreamIdListToRecover.Contains(requestMsg.StreamId))
                    {
                        QueueStatusForDispatch(requestMsg.StreamId, requestMsg.DomainType, statusText, privateStream);
                    }
                }

                if (!privateStream && m_Watchlist.LoginHandler.SupportSingleOpen)
                {
                    // wlRequest.RequestMsg may not be set yet.  However, if submitOptions.ServiceName is not set,
                    // AddToPendingRequestTable uses the serviceId from wlRequst.RequestMsg.msgKey so just
                    // set it here
                    if (submitOptions.ServiceName == null)
                    {
                        wlItemRequest.RequestMsg.MsgKey.ServiceId = requestMsg.MsgKey.ServiceId;
                    }

                    if (wlItemRequest.BatchRequestHelper != null)
                    {
                        WlBatchRequestHelper batchReqHelper = wlItemRequest.BatchRequestHelper;
                        AddToPendingRequestTable(wlItemRequest, submitOptions);
                        for (int index = 0; index < batchReqHelper.WlItemRequestList.Count; index++)
                        {
                            WlItemRequest nextItemRequest = batchReqHelper.WlItemRequestList[index];
                            AddToPendingRequestTable(nextItemRequest, submitOptions);
                        }
                    }
                    else
                    {
                        AddToPendingRequestTable(wlItemRequest, submitOptions);
                    }
                }
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleItemReissue(WlItemRequest wlItemRequest, IRequestMsg requestMsg, ReactorSubmitOptions submitOptions, 
            out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;

            if (wlItemRequest.WatchlistStream == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "WlItemHandler.HandleItemReissue",
                    "Reissue not allowed on an unopen stream.");
            }

            // The streaming flag must not be chagned for item reissue
            if (wlItemRequest.RequestMsg.CheckStreaming() != requestMsg.CheckStreaming())
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "WlItemHandler.HandleItemReissue",
                    "Request reissue may not alter streaming flag.");
            }

            // Reset the reissue flags
            wlItemRequest.ItemChangeFlags = WlItemRequest.ChangeFlags.NONE;

            if(!requestMsg.CheckNoRefresh() && !requestMsg.CheckPause())
            {
                wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
            }

            if(requestMsg.DomainType == (int)DomainType.SYMBOL_LIST)
            {
                if ((ret = ExtractSymbolListFromMsg(wlItemRequest, requestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    return ret;

                wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
            }

            WlItemStream wlItemStream = (WlItemStream)wlItemRequest.WatchlistStream!;

            if (requestMsg.CheckStreaming())
            {
                if (requestMsg.CheckPause() && !wlItemRequest.RequestMsg.CheckPause())
                {
                    // increase the number of pause requested
                    wlItemStream.NumPausedRequestsCount++;
                    
                    if(wlItemStream.NumPausedRequestsCount == wlItemStream.UserRequestDlList.Count())
                    {
                        wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
                    }
                }
                else if (!requestMsg.CheckPause() && wlItemRequest.RequestMsg.CheckPause())
                {
                    // decrease the number of pause requested
                    wlItemStream.NumPausedRequestsCount--;
                    wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
                }

            }

            bool removeOldView = true;
            bool effectiveViewChange = true;

            // retrieve request message from the stream
            IRequestMsg streamRequestMsg = wlItemStream.RequestMsg;

            WlItemRequest tempWlItemRequest = m_Watchlist.CreateWlItemRequest();

            if (requestMsg.CheckHasView())
            {
                // for re-issue, in case incoming request does not have view data, re-use the cached one
                if (requestMsg.EncodedDataBody.Data() is null)
                {
                    requestMsg.EncodedDataBody = wlItemRequest.RequestMsg.EncodedDataBody;
                    requestMsg.ContainerType = wlItemRequest.RequestMsg.ContainerType;
                }

                if ((ret = ExtractViewFromMsg(tempWlItemRequest, requestMsg, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    if(!m_HasViewType && tempWlItemRequest.ViewElemCount == 0)
                    {
                        // no view type and empty view content, use the old view type and view field list,
                        // do not remove old view, still send out request
                        removeOldView = false;
                        // same view needs to go out on the wire again
                        wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;
                    }
                    else
                    {
                        return ret;
                    }
                }

                if (wlItemRequest.ViewElemCount > 0)
                {
                    if (wlItemRequest.ViewType != tempWlItemRequest.ViewType)
                    {
                        ret = Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.FAILURE,
                                "WlItemHandler.HandleItemReissue",
                                "Requested view type does not match existing stream.");
                        return ret;
                    }
                    // for only one view, aggView is that view and might be removed later
                    if (streamRequestMsg.CheckHasView() && wlItemStream.AggregateView!.ContainsView(tempWlItemRequest))
                    {
                        effectiveViewChange = false;
                    }
                }
            }

            WlView? oldView = null;
            if (wlItemRequest.ViewElemCount > 0 && removeOldView)
            {
                // has old view and can be removed
                oldView = wlItemRequest.View;
                RemoveRequestView(wlItemStream, wlItemRequest, out _);
                wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;
            }

            if (requestMsg.CheckHasView())
            {
                ExtractViewFromMsg(wlItemRequest, requestMsg, out errorInfo);
                if ((ret = HandleViews(wlItemRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    return ret;
                wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;

                // if only one view re-issue, the original aggView could be removed above, this check hence becomes no-op
                if (wlItemStream.AggregateView!.ContainsNewView() && streamRequestMsg.CheckHasView())
                {
                    wlItemRequest.ItemChangeFlags &= ~WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;
                }

                if (!streamRequestMsg.CheckHasView() && wlItemStream.RequestsWithViewCount != wlItemStream.UserRequestDlList.Count())
                {
                    wlItemRequest.ItemChangeFlags &= ~WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;
                }

                if (!effectiveViewChange)
                {
                    wlItemRequest.ItemChangeFlags &= ~WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE;
                }
            }

            if (tempWlItemRequest != null)
                RepoolWlRequest(tempWlItemRequest);

            // user requested no refresh flag, so temporarily for this message, set it and turn it off after sending it.
            if (requestMsg.CheckNoRefresh())
            {
                streamRequestMsg.ApplyNoRefresh();
            }

            // update priority only if present on reissue request
            if (requestMsg.CheckHasPriority())
            {
                if (!wlItemRequest.RequestMsg.CheckHasPriority())
                {
                    // Apply 1,1 priority if not currently present.
                    wlItemRequest.RequestMsg.ApplyHasPriority();
                    wlItemRequest.RequestMsg.Priority.PriorityClass = 1;
                    wlItemRequest.RequestMsg.Priority.Count = 1;
                }

                // update PriorityClass only if changed
                if (requestMsg.Priority.PriorityClass != wlItemRequest.RequestMsg.Priority.PriorityClass)
                {
                    // use PriorityClass of reissue request if private stream or greater than existing one 
                    if (streamRequestMsg.CheckPrivateStream() || requestMsg.Priority.PriorityClass > 
                        streamRequestMsg.Priority.PriorityClass)
                    {
                        streamRequestMsg.Priority.PriorityClass = requestMsg.Priority.PriorityClass;
                        wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
                    }
                }

                // update priorityCount only if changed
                if (requestMsg.Priority.Count != wlItemRequest.RequestMsg.Priority.Count)
                {
                    // get difference between reissue request priority count and request's previous priority count 
                    int priorityCountDiff = requestMsg.Priority.Count - wlItemRequest.RequestMsg.Priority.Count;

                    // add priorityCount difference to that of existing one
                    streamRequestMsg.Priority.Count += priorityCountDiff;
                    wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
                }
            }

            // update extended header and data body in case of private stream
            if (requestMsg.CheckPrivateStream())
            {
                if (requestMsg.CheckHasExtendedHdr())
                {
                    streamRequestMsg.ApplyHasExtendedHdr();
                    BufferHelper.CopyBuffer(requestMsg.ExtendedHeader, streamRequestMsg.ExtendedHeader);
                }
                streamRequestMsg.ContainerType = requestMsg.ContainerType;
                BufferHelper.CopyBuffer(requestMsg.EncodedDataBody, streamRequestMsg.EncodedDataBody);
            }         

            // if dictionary domain, update MsgKey filter if changed
            if (requestMsg.DomainType == (int)DomainType.DICTIONARY)
            {
                if(requestMsg.MsgKey.Filter != streamRequestMsg.MsgKey.Filter)
                {
                    streamRequestMsg.MsgKey.Filter = requestMsg.MsgKey.Filter;
                    wlItemRequest.ItemChangeFlags |= WlItemRequest.ChangeFlags.STATE_CHANGE;
                }
            }

            /* Accepts item reissues when waiting for the first response or stream is open */
            if (wlItemStream.RequestPending || wlItemStream.State.StreamState() == StreamStates.OPEN)
            {
                // handle reissue only if not in the middle of multi-part refresh
                if (wlItemStream.RefreshState != WlItemStream.RefreshStateFlag.PENDING_REFRESH_COMPLETE)
                {
                    // send message to stream for any changes.
                    if (wlItemRequest.ItemChangeFlags != WlItemRequest.ChangeFlags.NONE)
                    {
                        if ((wlItemRequest.ItemChangeFlags & WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE) != 0)
                        {
                            wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                        }

                        bool removePriority = streamRequestMsg.CheckHasPriority() && !requestMsg.CheckHasPriority();

                        /* Checks whether there is priority change with this reissue request */
                        if (removePriority)
                        {
                            streamRequestMsg.Flags &= ~RequestMsgFlags.HAS_PRIORITY;
                        }

                        ret = wlItemStream.SendMsg(streamRequestMsg, submitOptions, false, out errorInfo);
                        if (ret == ReactorReturnCode.NO_BUFFERS)
                            ret = ReactorReturnCode.SUCCESS; // This request is added to pending queue.

                        /* Restores the priority flag */
                        if (removePriority)
                        {
                            streamRequestMsg.Flags |= RequestMsgFlags.HAS_PRIORITY;
                        }

                        if (oldView != null && ((wlItemRequest.ItemChangeFlags & WlItemRequest.ChangeFlags.REISSUE_VIEW_CHANGE) != 0))
                        {
                            // this stems from when no_refresh is set on request, but later still get refresh callback from Provider,
                            // need this flag to send fan out to all
                            if (requestMsg.CheckNoRefresh())
                            {
                                wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.PENDING_VIEW_REFRESH;
                            }
                        }

                        // update request state to PENDING_REFRESH if refresh is desired
                        if (!requestMsg.CheckNoRefresh())
                        {
                            wlItemRequest.ReqState = WlRequest.State.PENDING_REFRESH;
                        }
                    }

                    // remove the temporary no refresh flag after sending the request.
                    if (streamRequestMsg.CheckNoRefresh())
                    {
                        streamRequestMsg.Flags &= ~RequestMsgFlags.NO_REFRESH;
                    }

                    if (m_HasPendingViewRequest)
                    {
                        wlItemStream.WaitingRequestDlList.PushBack(wlItemRequest, WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                        m_HasPendingViewRequest = false;
                    }
                }
                else
                {
                    // add to waiting request list
                    if( (wlItemRequest.ItemChangeFlags & WlItemRequest.ChangeFlags.STATE_CHANGE) != 0)
                    {
                        wlItemStream.WaitingRequestDlList.PushBack(wlItemRequest, WlItemRequest.WAIT_ITEM_REQUEST_LINK);
                    }
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "WlItemHandler.HandleItemReissue",
                    "Request reissue while stream state is known as open.");
            }

            errorInfo = null;
            return ret;
        }

        /* Finds an item aggregation stream for a user request. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        WlItemStream? FindItemAggregationStream(IRequestMsg requestMsg, Qos matchedQos, ReactorSubmitOptions submitOptions, WlStreamAttributes attributes)
        {
            WlItemStream? wlItemStream = null;

            // item can be aggregated only for non private stream.
            // determine if a new stream is needed or if existing stream can be used.
            attributes.Clear();
            requestMsg.MsgKey.Copy(attributes.MsgKey);
            if (submitOptions.ServiceName != null)
            {
                int serviceId = m_Watchlist.DirectoryHandler!.ServiceId(submitOptions.ServiceName);
                attributes.MsgKey.ApplyHasServiceId();
                attributes.MsgKey.ServiceId = serviceId;
            }

            attributes.DomainType = requestMsg.DomainType;
            matchedQos.Copy(attributes.Qos);

            // item can be aggregated only for non private stream.
            if (!requestMsg.CheckPrivateStream())
            {
                WlStreamAttribToWlStreamDict!.TryGetValue(attributes, out wlItemStream);
            }

            return wlItemStream;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool CanItemBeOpened(IRequestMsg requestMsg, Qos matchedQos, Qos? staticQos, Service service, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            bool ret = false;

            if (IsServiceUpAndAcceptingRequests(service))
            {
                if (IsCapabilitySupported(requestMsg.DomainType, service))
                {
                    // check Qos if not DICTIONARY domain
                    if (requestMsg.DomainType != (int)DomainType.DICTIONARY)
                    {
                        if (staticQos == null)
                        {
                            // no static qos specified, find matching Qos
                            if (requestMsg.CheckHasQos())
                            {
                                Qos? worstQos = null;
                                Qos qos = requestMsg.Qos;
                                if (requestMsg.CheckHasWorstQos())
                                {
                                    worstQos = requestMsg.WorstQos;
                                }
                                if (IsQosSupported(qos, worstQos, service, matchedQos))
                                {
                                    // qos and/or qos range is supported
                                    ret = true;
                                }
                                else
                                {
                                    errorInfo = new ReactorErrorInfo
                                    {
                                        Error =
                                        {
                                            Text = "Service does not provide a matching QoS",
                                            ErrorId = Transports.TransportReturnCode.FAILURE
                                        },

                                        Location = "WlItemHandler.CanItemBeOpened",
                                        Code = ReactorReturnCode.PARAMETER_INVALID
                                    };
                                }
                            }
                            else // no Qos specified
                            {
                                // best effort qos is supported
                                if (service.Info.QosList.Count > 0)
                                {
                                    service.Info.BestQos.Copy(matchedQos);
                                }
                                else
                                {
                                    matchedQos.Rate(QosRates.TICK_BY_TICK);
                                    matchedQos.Timeliness(QosTimeliness.REALTIME);
                                }

                                ret = true;
                            }
                        }
                        else // static qos specified
                        {
                            // use static qos for matching qos
                            if (IsQosSupported(staticQos, null, service, matchedQos))
                            {
                                // qos and/or qos range is supported
                                ret = true;
                            }
                            else
                            {
                                errorInfo = new ReactorErrorInfo
                                {
                                    Error =
                                        {
                                            Text = "Service does not provide a matching QoS",
                                            ErrorId = Transports.TransportReturnCode.FAILURE
                                        },

                                    Location = "WlItemHandler.CanItemBeOpened",
                                    Code = ReactorReturnCode.PARAMETER_INVALID
                                };
                            }
                        }
                    }
                    else // DICTIONARY domain
                    {
                        // DICTIONARY domain supported since capability supported
                        ret = true;
                    }
                }
                else
                {
                    errorInfo = new ReactorErrorInfo
                    {
                        Error =
                        {
                            Text = "Capability not supported",
                            ErrorId = Transports.TransportReturnCode.FAILURE
                        },

                        Location = "WlItemHandler.CanItemBeOpened",
                        Code = ReactorReturnCode.PARAMETER_INVALID
                    };
                }
            }
            else
            {
                errorInfo = new ReactorErrorInfo
                {
                    Error =
                    {
                        Text = "Service not up",
                        ErrorId = Transports.TransportReturnCode.FAILURE
                    },

                    Location = "WlItemHandler.CanItemBeOpened",
                    Code = ReactorReturnCode.PARAMETER_INVALID
                };
            }

            return ret;
        }

        /* Creates a new stream. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        WlItemStream? SetNewStream(IRequestMsg requestMsg, WlItemStream wlItemStream)
        {
            // Create a watchlist item stream
            
            if (wlItemStream != null)
            {
                wlItemStream.InUse = true;
                wlItemStream.StreamType = WlStreamType.ITEM;
                wlItemStream.WlHandler = this;
                wlItemStream.Watchlist = m_Watchlist;
                wlItemStream.StreamIdLink[0]!.StreamId = wlItemStream.StreamId;
                wlItemStream.StreamIdLink[1]!.StreamId = wlItemStream.StreamId;
                wlItemStream.StreamDomainType = requestMsg.DomainType;
                wlItemStream.RequestsWithViewCount = 0;
                wlItemStream.RefreshState = WlItemStream.RefreshStateFlag.NONE;
            }
                      
            return wlItemStream;
        }

        /* Determines if a service is up and accepting requests. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool IsServiceUpAndAcceptingRequests(Service service)
        {
            return (m_Watchlist.ReactorChannel!.State == ReactorChannelState.UP || m_Watchlist.ReactorChannel!.State == ReactorChannelState.READY) 
                    && service.HasState 
                    && (!service.State.HasAcceptingRequests || service.State.AcceptingRequests == 1) && service.State.ServiceStateVal == 1;
        }

        /* Determines if a service supports a capability. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        static bool IsCapabilitySupported(int domainType, Service service)
        {
            bool ret = false;

            if (service.HasInfo)
            {
                ret = service.Info.CapabilitiesList.Contains((long)domainType);
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool IsQosSupported(Qos qos, Qos? worstQos, Service service, Qos matchedQos)
        {
            bool ret = false;

            if (service.HasInfo)
            {
                if (service.Info.HasQos)
                {
                    for (int i = 0; i < service.Info.QosList.Count; i++)
                    {
                        Qos serviceQos = service.Info.QosList[i];
                        if (worstQos == null)
                        {
                            // no worst Qos, determine if request qos supported by service
                            if (serviceQos.Equals(qos))
                            {
                                ret = true;
                                serviceQos.Copy(matchedQos);
                                break;
                            } 
                        }
                        else // worstQos is specified
                        {
                            if (serviceQos.IsInRange(qos, worstQos))
                            {
                                if (serviceQos.IsBetter(matchedQos))
                                {
                                    ret = true;
                                    serviceQos.Copy(matchedQos);
                                }
                            }
                        }
                    }
                }
                else // service has no qos
                {
                    // determine if qos matches default of Realtime, Tick-By-Tick
                    if (worstQos == null)
                    {
                        ret = m_DefaultQos.Equals(qos);
                    }
                    else // worstQos specified
                    {
                        ret = m_DefaultQos.IsInRange(qos, worstQos);
                    }

                    if (ret)
                    {
                        m_DefaultQos.Copy(matchedQos);
                    }
                }
            }

            // Sets IsDynamic flag to that of requested Qos before returning
            if (ret == true)
            {
                matchedQos.IsDynamic = qos.IsDynamic;
            }

            return ret;
        }

        /* Determines if a service's window is open for another request. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool IsWindowOpen(WlService wlService)
        {
            bool ret = true;

            if (m_Watchlist.ConsumerRole!.WatchlistOptions.ObeyOpenWindow &&
                wlService.RdmService!.HasLoad && wlService.RdmService.Load.HasOpenWindow)
            {
                long openWindow = wlService.RdmService.Load.OpenWindow;
                if (openWindow == 0 || // open window of 0 means window is not open
                    wlService.NumOutstandingRequests == openWindow)
                {
                    ret = false;
                }
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode Dispatch(out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            errorInfo = null;

            // dispatch streams

            LinkedListNode<IStatusMsg>? currentNode = StatusMsgDispatchList.First;
            LinkedListNode<IStatusMsg>? nextNode;
            WlItemRequest? wlItemRequest;
            // send any queued status messages to the user
            while (currentNode != null)
            {
                IStatusMsg statusMsg = currentNode.Value;
                nextNode = currentNode.Next;
                StatusMsgDispatchList.Remove(currentNode);
                if (m_Watchlist.StreamIdToWlRequestDict!.TryGetValue(statusMsg.StreamId, out WlRequest? wlRequest))
                {
                    bool requestClosed = statusMsg.CheckHasState() && statusMsg.State.StreamState() != StreamStates.OPEN;
                    wlItemRequest = (WlItemRequest)wlRequest;

                    if (requestClosed)
                    {
                        CloseWlRequest(wlItemRequest);
                    }
                    ret = CallbackUserWithMsg("WlItemHandler.Dispatch", statusMsg, wlRequest, out errorInfo);

                    if (requestClosed)
                    {
                        RepoolWlRequest(wlItemRequest);
                    }

                    // return IStatusMsg to pool
                    StatusMsgPool.AddLast(statusMsg);

                    if (ret < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    // return IStatusMsg to pool
                    StatusMsgPool.AddLast(statusMsg);
                }

                currentNode = nextNode;
            }

            // re-submit user requests that had request timeout
            wlItemRequest = RequestTimeoutDlList.Pop(WlItemRequest.TIMEOUT_ITEM_REQUEST_LINK);
            while (wlItemRequest != null)
            {
                m_SubmitOptions.Clear();
                m_SubmitOptions.ServiceName = wlItemRequest.WatchlistStreamInfo.ServiceName;
                m_SubmitOptions.RequestMsgOptions.UserSpecObj = wlItemRequest.WatchlistStreamInfo.UserSpec;
                if((ret = HandleItemRequest(wlItemRequest, wlItemRequest.RequestMsg, m_SubmitOptions, false, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }

                wlItemRequest = RequestTimeoutDlList.Pop(WlItemRequest.TIMEOUT_ITEM_REQUEST_LINK);
            }

            // send request for all pending item stream in the list
            var stream = PendingSendMsgDlList.Pop();
            while (stream != null)
            {
                stream.PendingSendMsg = false;

                if ((ret = stream.SendMsg(stream.RequestMsg, m_SubmitOptions, false, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    // No buffers means that the request was re-queued, so we can end the loop here
                    if (ret == ReactorReturnCode.NO_BUFFERS)
                    {
                        return ReactorReturnCode.SUCCESS;
                    }
                    else
                    {
                        return ret;
                    }
                }
                else
                {
                    if (PendingSendMsgDlList.Count() == 0)
                    {
                        return ReactorReturnCode.SUCCESS;
                    }
                }
                StreamDlList.PushBack(stream);
                stream = PendingSendMsgDlList.Pop();
            }

            return ret;
        }

        /* Handles login stream open event. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode LoginStreamOpen(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /* Handles login stream closed event. 
         * If state is null, presumes it was closed by a CloseMsg. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode LoginStreamClosed(State? state)
        {
            m_StatusMsg.Clear();
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;

            if (state != null)
            {
                m_StatusMsg.State.StreamState(state.StreamState());
                m_StatusMsg.State.DataState(state.DataState());
            }
            else
            {
                // Closed via CloseMsg.
                m_StatusMsg.State.StreamState(StreamStates.CLOSED);
                m_StatusMsg.State.DataState(DataStates.SUSPECT);
            }


            m_StatusMsg.State.Text().Data("Login stream was closed.");
            FanoutToAllStreams(m_StatusMsg);

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ChannelUp(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            while (StreamDlList.Pop() != null) {}
            while (PendingSendMsgDlList.Pop() != null) { }
        }

        public void ChannelDown()
        {
            // Do nothing for the item handler.
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void FanoutToAllStreams(IMsg msg)
        {
            var stream = StreamDlList.Pop();
            while (stream != null)
            {
                ReadMsg(stream, null, msg, out _);
                stream = StreamDlList.Pop();
            }
            stream = PendingSendMsgDlList.Pop();
            while (stream != null)
            {
                ReadMsg(stream, null, msg, out _);
                stream = PendingSendMsgDlList.Pop();
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddPendingRequest(WlStream wlStream)
        {
            if (wlStream != null)
            {
                WlItemStream itemStream = (WlItemStream)wlStream;
                if (!itemStream.PendingSendMsg)
                {
                    StreamDlList.Remove(itemStream);
                    PendingSendMsgDlList.PushBack(itemStream);
                    itemStream.PendingSendMsg = true;
                }
            }
            else
            {
                //if the msg passed to stream.SentMsg() call is a request msg which has PENDING_PRIORITY_CHANGE and 
                //PENDING_VIEW_REFRESH flags, eta should not send this request out, 
                //eta will put it into the WaitingRequestList for sending it out after receiving refresh.
                m_HasPendingViewRequest = true;
            }
        }

        /* Queues a status message for sending on dispatch. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void QueueStatusForDispatch(int streamId, int domainType, String text, bool privateStream)
        {
            // get StatusMsg from pool
            IStatusMsg statusMsg = GetStatusMsgFromPool();

            // populate StatusMsg
            statusMsg.Clear();
            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = streamId;
            statusMsg.DomainType = domainType;
            statusMsg.ApplyHasState();

            if (!privateStream && m_Watchlist.LoginHandler.SupportSingleOpen)
            {
                statusMsg.State.StreamState(StreamStates.OPEN);
            }
            else
            {
                statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            }
            statusMsg.State.DataState(DataStates.SUSPECT);
            statusMsg.State.Text().Data(text);

            // add StatusMsg to dispatch list and trigger dispatch
            StatusMsgDispatchList.AddLast(statusMsg);

            if (StatusMsgDispatchList.Count == 1)
            {
                // trigger dispatch only for first add to list
                m_Watchlist.Reactor!.SendWatchlistDispatchNowEvent(m_Watchlist.ReactorChannel!);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public static void RepoolWlRequest(WlItemRequest wlItemRequest)
        {
            if (wlItemRequest.View != null)
            {
                switch (wlItemRequest.View.ViewType)
                {
                    case ViewTypes.FIELD_ID_LIST:
                        wlItemRequest.ViewFieldList = null;
                        break;
                    case ViewTypes.ELEMENT_NAME_LIST:
                        wlItemRequest.ViewFieldList = null;
                        break;
                    default:
                        break;
                }

                if (wlItemRequest.View.FieldList != null)
                {
                    wlItemRequest.View.FieldList = null;
                }

                wlItemRequest.View = null;
            }

            wlItemRequest.ReturnToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void CloseWlRequest(WlItemRequest wlItemRequest)
        {
            if ((wlItemRequest.ItemReqFlags & WlItemRequest.Flags.PROV_DRIVEN) != 0)
            {
                m_SymbolListRequestKey.Clear();
                m_SymbolListRequestKey.MsgKey = wlItemRequest.RequestMsg.MsgKey;
                m_SymbolListRequestKey.MsgKey.ServiceId = wlItemRequest.RequestMsg.MsgKey.ServiceId;
                m_SymbolListRequestKey.DomainType = wlItemRequest.RequestMsg.DomainType;
                m_SymbolListRequestKey.Qos = wlItemRequest.RequestMsg.Qos;
                ProviderRequestDict.Remove(m_SymbolListRequestKey);
            }

            m_Watchlist.CloseWlRequest(wlItemRequest);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        IStatusMsg GetStatusMsgFromPool()
        {
            IStatusMsg statusMsg;

            var node = StatusMsgPool.First;
            if(node != null)
            {
                statusMsg = node.Value;
                statusMsg.Clear();
                statusMsg.MsgClass = MsgClasses.STATUS;
                StatusMsgPool.RemoveFirst();
            }
            else
            {
                statusMsg = new Msg
                {
                    MsgClass = MsgClasses.STATUS
                };
            }

            return statusMsg;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ExtractSymbolListFromMsg(WlItemRequest wlItemRequest, IRequestMsg requestMsg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            m_ElementList.Clear();
            m_ElementEntry.Clear();
            m_BehaviourElementList.Clear();
            m_BehaviourEntry.Clear();
            m_DataStreamFlag.Clear();

            if (requestMsg.ContainerType != DataTypes.ELEMENT_LIST)
            {
                return ReactorReturnCode.SUCCESS; // nothing to extract
            }

            m_DecodeIt.Clear();
            m_DecodeIt.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                    m_Watchlist.ReactorChannel!.MinorVersion);

            CodecReturnCode ret = m_ElementList.Decode(m_DecodeIt, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE, "ItemHandler.ExtractSymbolListFromMsg",
                        $"ElementList.Decode() failed: <{ret.GetAsString()}>");
            }

            while ((ret = m_ElementEntry.Decode(m_DecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractSymbolListFromMsg",
                            $"ElementEntry.Decode() failed: <{ret.GetAsString()}>");
                }

                if (m_ElementEntry.Name.Equals(SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS))
                {
                    wlItemRequest.HasBehaviour = true;

                    if (m_ElementEntry.DataType != DataTypes.ELEMENT_LIST)
                    {
                        // Nothing to extract
                        return ReactorReturnCode.SUCCESS;
                    }

                    ret = m_BehaviourElementList.Decode(m_DecodeIt, null);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        while ((ret = m_BehaviourEntry.Decode(m_DecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if (ret < CodecReturnCode.SUCCESS)
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                                "ItemHandler.ExtractSymbolListFromMsg",
                                                "Error decoding element entry in RequestMsg.");
                            }
                            else
                            {
                                if (m_BehaviourEntry.Name.Equals(SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS))
                                {
                                    if (m_BehaviourEntry.DataType != DataTypes.UINT)
                                    {
                                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                                "ItemHandler.ExtractSymbolListFromMsg",
                                                "Error decoding Symbol List Data Streams -- Element has wrong data type.");
                                    }

                                    ret = m_DataStreamFlag.Decode(m_DecodeIt);
                                    if (ret != CodecReturnCode.SUCCESS)
                                    {
                                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                            "ItemHandler.ExtractSymbolListFromMsg",
                                            "Error decoding Symbol List Data Streams");
                                    }

                                    if (m_DataStreamFlag.ToLong() < SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_NAMES_ONLY ||
                                        m_DataStreamFlag.ToLong() > SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS)
                                    {
                                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                                "ItemHandler.ExtractSymbolListFromMsg",
                                                "Invalid symbol list request flags.");
                                    }

                                    wlItemRequest.SymbolListFlags = (int)m_DataStreamFlag.ToLong();
                                }
                            }
                        }
                    }
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleSymbolList(WlItemStream wlItemStream, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            var wlRequest = (WlItemRequest?)wlItemStream.UserRequestDlList.Start();
            while (wlRequest != null)
            {
                if (!wlRequest.HasBehaviour || wlRequest.SymbolListFlags == 0)
                {
                    wlRequest = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
                    continue;
                }
                    
                CodecReturnCode ret = CodecReturnCode.SUCCESS;
                int serviceId = -1; // Invalid service ID
                WlService? tempWlService = null;
                if (wlRequest.RequestMsg.MsgKey.CheckHasServiceId())
                {
                    // retrieve service id from request
                    serviceId = wlRequest.RequestMsg.MsgKey.ServiceId;
                }
                else if (wlRequest.WatchlistStreamInfo.ServiceName != null)
                {
                    // lookup service id by service name
                    tempWlService = m_Watchlist.DirectoryHandler.ServiceCache.Service(wlRequest.WatchlistStreamInfo.ServiceName);

                    if (tempWlService != null)
                    {
                        serviceId = tempWlService.RdmService!.ServiceId;
                    }
                }

                WlService? wlService;

                if (tempWlService != null)
                {
                    wlService = tempWlService;
                }
                else
                {
                    wlService = m_Watchlist.DirectoryHandler.ServiceCache.Service(serviceId);

                    if (wlService is null)
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                        "ItemHandler.HandleSymbolList",
                                        "Service for Symbol List stream is missing name. Cannot create data streams.");
                    }
                }

                if (msg.ContainerType != DataTypes.MAP)
                {
                    wlRequest = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
                    continue;
                }
                    
                m_RequestMsg.Clear();
                m_RequestMsg.MsgClass = MsgClasses.REQUEST;
                m_RequestMsg.DomainType = (int)DomainType.MARKET_PRICE;
                m_RequestMsg.ContainerType = DataTypes.NO_DATA;
                m_RequestMsg.ApplyHasQos();

                if ((wlRequest.SymbolListFlags & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS) > 0)
                {
                    m_RequestMsg.ApplyStreaming();
                }
                m_RequestMsg.MsgKey.ApplyHasNameType();
                m_RequestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
                // not batch
                m_RequestMsg.MsgKey.ApplyHasName();

                Qos itemQos;
                if (wlService.RdmService!.Info.QosList.Count > 0)
                {
                    itemQos = wlService.RdmService.Info.BestQos;
                    itemQos.Copy(m_RequestMsg.Qos);
                }
                else
                {
                    m_RequestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    m_RequestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                }

                m_DecodeIt.Clear();
                m_DecodeIt.SetBufferAndRWFVersion(msg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                    m_Watchlist.ReactorChannel!.MinorVersion);
                m_Map.Clear();
                m_MapEntry.Clear();
                m_MapKey.Clear();

                switch (msg.MsgClass)
                {
                    case MsgClasses.UPDATE:
                    case MsgClasses.REFRESH:
                        ret = m_Map.Decode(m_DecodeIt);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                    "ItemHandler.HandleSymbolList", $"Map.Decode() failed: <{ret.GetAsString()}>");
                        }
                        while ((ret = m_MapEntry.Decode(m_DecodeIt, m_MapKey)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if (ret != CodecReturnCode.SUCCESS)
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                        ReactorReturnCode.FAILURE, "ItemHandler.handleSymbolList",
                                        $"DecodeMapEntry() failed: <{ret.GetAsString()}>");
                            }

                            switch (m_MapEntry.Action)
                            {
                                case MapEntryActions.ADD:
                                case MapEntryActions.UPDATE:
                                    m_RequestMsg.MsgKey.Name = m_MapKey;
                                    m_SymbolListRequestKey.Clear();
                                    m_SymbolListRequestKey.MsgKey = m_RequestMsg.MsgKey;
                                    m_SymbolListRequestKey.MsgKey.ServiceId = serviceId;
                                    m_SymbolListRequestKey.DomainType = m_RequestMsg.DomainType;
                                    m_RequestMsg.Qos.Copy(m_SymbolListRequestKey.Qos);

                                    if (ProviderRequestDict.ContainsKey(m_SymbolListRequestKey))
                                        continue;

                                    int providerProvideStreamId = m_Watchlist.StreamManager!.GetProviderStreamID();
                                    m_RequestMsg.StreamId = providerProvideStreamId;
                                    WlItemRequest newWlRequest = m_Watchlist.CreateWlItemRequest();
                                    newWlRequest.ItemReqFlags |= WlItemRequest.Flags.PROV_DRIVEN;
                                    newWlRequest.WatchlistStreamInfo.ServiceName = wlRequest.WatchlistStreamInfo.ServiceName;
                                    m_SubmitOptions.ServiceName = wlRequest.WatchlistStreamInfo.ServiceName;
                                    m_SubmitOptions.RequestMsgOptions.UserSpecObj = wlRequest.WatchlistStreamInfo.UserSpec;
                                    ReactorReturnCode reactorReturnCode = HandleItemRequest(newWlRequest, m_RequestMsg, m_SubmitOptions, true, out errorInfo);

                                    if (reactorReturnCode >= ReactorReturnCode.SUCCESS)
                                    {
                                        m_RequestMsg.Copy(newWlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS);
                                        m_Watchlist.StreamIdToWlRequestDict!.Add(providerProvideStreamId, newWlRequest);
                                        if (m_RequestMsg.CheckStreaming())
                                            ProviderRequestDict.Add(m_SymbolListRequestKey, newWlRequest.RequestMsg);
                                    }
                                    else // submit failed
                                    {
                                        RepoolWlRequest(newWlRequest);
                                    }
                                    break;
                                default:
                                    break;
                            }// switch mapEntry
                        }// while mapEntry
                        break;
                    default:
                        return ReactorReturnCode.SUCCESS;
                }// switch msgClass

                wlRequest = (WlItemRequest?)wlItemStream.UserRequestDlList.Forth();
            }// wlStream loop
            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode ExtractViewFromMsg(WlItemRequest wlItemRequest, IRequestMsg requestMsg, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            wlItemRequest.ViewElemCount = 0;
            bool viewDataFound = false;
            int viewElemCount = 0;
            m_ElementList.Clear();
            m_ElementEntry.Clear();
            m_HasViewType = false;
            m_ViewType.Clear();
            m_ViewDataElement.Clear();
            m_ViewElementList = requestMsg.EncodedDataBody;
            m_DecodeIt.Clear();
            m_DecodeIt.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, m_Watchlist.ReactorChannel!.MajorVersion,
                    m_Watchlist.ReactorChannel!.MinorVersion);

            if (requestMsg.ContainerType != DataTypes.ELEMENT_LIST)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                        $"Unexpected container type <{requestMsg.ContainerType}>");
            }

            CodecReturnCode codecRet;

            if (m_ElementList.Decode(m_DecodeIt, null) != CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                        $"Unexpected container type <{requestMsg.ContainerType}>");
            }

            while ((codecRet = m_ElementEntry.Decode(m_DecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (codecRet != CodecReturnCode.SUCCESS)
                {
                    Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                            $"ElementEntry.decode() failed: <{codecRet.GetAsString()}>");
                }
                else
                {
                    if (m_ElementEntry.Name.Equals(ElementNames.VIEW_TYPE) && m_ElementEntry.DataType == DataTypes.UINT)
                    {
                        m_ViewType.Decode(m_DecodeIt);
                        m_HasViewType = true;
                    }

                    if (m_ElementEntry.Name.Equals(ElementNames.VIEW_DATA) && m_ElementEntry.DataType == DataTypes.ARRAY)
                    {
                        m_ViewDataElement = m_ElementEntry.EncodedData;
                        viewDataFound = true;
                    }
                }

            } // while

            long viewType = m_ViewType.ToLong();
            wlItemRequest.ViewType = viewType;
            wlItemRequest.ViewAction = ViewAction.SET;

            if (viewType == ViewTypes.FIELD_ID_LIST || viewType == ViewTypes.ELEMENT_NAME_LIST || !m_HasViewType)
            {
                if (requestMsg.DomainType == (int)DomainType.SYMBOL_LIST)
                {
                    errorInfo = null;
                    return ReactorReturnCode.SUCCESS;
                }

                if (!viewDataFound)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE, "ItemHandler.extractViewFromMsg",
                            $":ViewData element not found <{CodecReturnCode.INCOMPLETE_DATA}>");
                }
                else
                {
                    m_DecodeIt.Clear();
                    m_DecodeIt.SetBufferAndRWFVersion(m_ViewDataElement, m_Watchlist.ReactorChannel!.MajorVersion,
                            m_Watchlist.ReactorChannel!.MinorVersion);

                    switch (viewType)
                    {
                        case ViewTypes.FIELD_ID_LIST:
                            {
                                m_ViewArray.Clear();
                                if ((codecRet = m_ViewArray.Decode(m_DecodeIt)) == CodecReturnCode.SUCCESS)
                                {
                                    if (m_ViewArray.PrimitiveType != DataTypes.INT)
                                    {
                                        return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                            $"Unexpected primitive type in array  <{m_ViewArray.PrimitiveType}>");
                                    }

                                    List<int> fieldIdList;

                                    if (wlItemRequest.ViewFieldList is null)
                                    {
                                        fieldIdList = new List<int>();
                                        wlItemRequest.ViewFieldList = fieldIdList;
                                    }
                                    else
                                    {
                                        fieldIdList = (List<int>)wlItemRequest.ViewFieldList;
                                    }

                                    fieldIdList.Clear();

                                    while ((codecRet = m_ViewArrayEntry.Decode(m_DecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                                    {
                                        if (codecRet < CodecReturnCode.SUCCESS)
                                        {
                                            return Reactor.PopulateErrorInfo(out errorInfo,
                                                ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                                $"Error decoding array entry   <{codecRet.GetAsString()}>");
                                        }
                                        else
                                        {
                                            if ((codecRet = m_FieldId.Decode(m_DecodeIt)) == CodecReturnCode.SUCCESS)
                                            {
                                                if (m_FieldId.ToLong() < short.MinValue || m_FieldId.ToLong() > short.MaxValue)
                                                {
                                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                                            $"Field id in view request is outside the valid ID range <{m_FieldId}> ");
                                                }

                                                fieldIdList.Add((int)m_FieldId.ToLong());
                                                viewElemCount++;
                                            }
                                            else
                                            {
                                                return Reactor.PopulateErrorInfo(out errorInfo,
                                                        ReactorReturnCode.FAILURE, "ItemHandler.extractViewFromMsg",
                                                        $"Invalid BLANK_DATA or incomplete data while decoding :ViewData <{codecRet.GetAsString()}>");
                                            }
                                        }
                                    }// while
                                    wlItemRequest.ViewElemCount = viewElemCount;
                                }
                                else
                                {
                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                            $"Error decoding array  <{codecRet.GetAsString()}>");
                                }
                                break;
                            }// case
                        case ViewTypes.ELEMENT_NAME_LIST:
                            {
                                if ((codecRet = m_ViewArray.Decode(m_DecodeIt)) == CodecReturnCode.SUCCESS)
                                {
                                    if (!(m_ViewArray.PrimitiveType == DataTypes.ASCII_STRING ||
                                            m_ViewArray.PrimitiveType == DataTypes.UTF8_STRING ||
                                                    m_ViewArray.PrimitiveType == DataTypes.RMTES_STRING))
                                    {
                                        return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                            $"Unexpected primitive type in array  <{m_ViewArray.PrimitiveType}>");
                                    }

                                    List<string> elementNameList;

                                    if (wlItemRequest.ViewFieldList is null)
                                    {
                                        elementNameList = new();
                                        wlItemRequest.ViewFieldList = (object)elementNameList;
                                    }
                                    else
                                    {
                                        elementNameList = (List<string>)wlItemRequest.ViewFieldList;
                                    }

                                    elementNameList.Clear();

                                    while ((codecRet = m_ViewArrayEntry.Decode(m_DecodeIt)) != CodecReturnCode.END_OF_CONTAINER)
                                    {
                                        if (codecRet < CodecReturnCode.SUCCESS)
                                        {
                                            return Reactor.PopulateErrorInfo(out errorInfo,
                                                ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                                $"Error decoding array entry   <{codecRet.GetAsString()}>");
                                        }
                                        else
                                        {
                                            if (m_ElementName.Decode(m_DecodeIt) == CodecReturnCode.SUCCESS)
                                            {
                                                elementNameList.Add(m_ElementName.ToString());
                                                viewElemCount++;
                                            }
                                            else
                                            {
                                                return Reactor.PopulateErrorInfo(out errorInfo,
                                                        ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                                        $"Invalid BLANK_DATA or incomplete data while decoding :ViewData <{codecRet.GetAsString()}>");
                                            }
                                        }
                                    }// while			
                                    wlItemRequest.ViewElemCount = viewElemCount;
                                }
                                else
                                {
                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                            $"Error decoding array  <{codecRet.GetAsString()}>");
                                }

                                break;
                            }
                        default:
                            {
                                // non-existent viewType
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                            ReactorReturnCode.FAILURE, "ItemHandler.ExtractViewFromMsg",
                                            $"Error invalid view type  <{viewType}>"); 
                            }
                    }// switch
                }// viewDataFound	
            }// if field_id_list or element_name_list
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE, "ItemHandler.extractViewFromMsg",
                        $":Invalid :ViewType or :ViewType not found  <{viewType}>");
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode HandleViews(WlItemRequest wlItemRequest, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            switch(wlItemRequest.ViewAction)
            {
                case ViewAction.SET:
                    {
                        WlView? wlView = m_ViewHandler.CreateView(wlItemRequest.ViewFieldList!, (int)wlItemRequest.ViewType, 
                            out errorInfo);

                        if(wlView is null)
                        {
                            return ReactorReturnCode.FAILURE;
                        }

                        wlItemRequest.View = wlView;

                        break;
                    }
                case ViewAction.MAINTAIN:
                    {
                        break;
                    }
                case ViewAction.NONE:
                    {
                        break;
                    }
                default:
                    {
                        Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE, "ItemHandler",
                            $"Invalid View Action  <{wlItemRequest.ViewAction}> ");
                            return ReactorReturnCode.FAILURE;
                    }
            }

            return AddRequestView(wlItemRequest, out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        internal ReactorReturnCode AddRequestView(WlItemRequest wlItemRequest, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            WlItemStream wlItemStream = (WlItemStream)wlItemRequest.WatchlistStream!;
            if (wlItemRequest.View != null && wlItemStream.AggregateView != null &&
                wlItemRequest.View.ViewType != wlItemStream.AggregateView.ViewType)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "WlItemHandler.AddRequestView", "ViewType mismatch, cannot be aggregated.");
            }

            wlItemStream.RequestsWithViewCount++;
            wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
            if (wlItemStream.AggregateView is null)
            {
                wlItemStream.AggregateView = new();
                wlItemStream.AggregateView.ViewType = wlItemRequest.View!.ViewType;
                wlItemStream.AggregateView.ViewHandler = wlItemRequest.View!.ViewHandler;
            }

            wlItemStream.AggregateView.AddView(wlItemRequest.View!);
            wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        static void RemoveRequestView(WlItemStream wlItemStream,WlItemRequest wlItemRequest, out bool viewChangedToZero)
        {
            viewChangedToZero = false;
            if (!wlItemRequest.RequestMsg.CheckHasView())
            {
                // need to re-evaluate the wlStream request views
                // as wlStream.UserRequestList size will decrease
                wlItemStream.Flags |= WlItemStream.StatusFlags.PENDING_VIEW_CHANGE;
                return;
            }

            if (wlItemStream.RequestsWithViewCount > 0)
            {
                wlItemStream.RequestsWithViewCount--;

                wlItemStream.AggregateView!.RemoveView(wlItemRequest.View!, out viewChangedToZero);
            }
        }

        public static void AppendLinkedList<T>(LinkedList<T> first, LinkedList<T> second)
        {
            foreach(var item in second)
            {
                first.AddLast(item);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public static void AppendDictionary<TKey, TValue>(Dictionary<TKey, TValue> first, 
            Dictionary<TKey, TValue> second) where TKey: notnull
        {
            foreach(var kvp in second)
            {
                first[kvp.Key] = kvp.Value;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public static void AppendSet<T>(HashSet<T> first, HashSet<T> second) where T : notnull
        {
            foreach (var el in second)
            {
                first.Add(el);
            }
        }

        /* Handles the pause all event from the login stream.*/
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode IWlItemHandler.PauseAll()
        {
            var stream = StreamDlList.Start();
            while (stream != null) 
            {
                var request = stream.UserRequestDlList.Start();
                while (request != null)
                {
                    request.RequestMsg.ApplyPause();
                    request = stream.UserRequestDlList.Forth();
                }
                stream.NumPausedRequestsCount = stream.UserRequestDlList.Count();
                stream = StreamDlList.Forth();
            }

            stream = PendingSendMsgDlList.Start();
            while (stream != null)
            {
                var request = stream.UserRequestDlList.Start();
                while (request != null)
                {
                    request.RequestMsg.ApplyPause();
                    request = stream.UserRequestDlList.Forth();
                }
                stream.NumPausedRequestsCount = stream.UserRequestDlList.Count();
                stream = PendingSendMsgDlList.Forth();
            }

            foreach (var pendingRequestList in PendingRequestByNameDict.Values)
            {
                foreach (var userRequest in pendingRequestList)
                {
                    userRequest.RequestMsg.ApplyPause();
                }
            }

            foreach (var pendingRequestList in PendingRequestByIdDict.Values)
            {
                foreach (var userRequest in pendingRequestList)
                {
                    userRequest.RequestMsg.ApplyPause();
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        /* Handles the resume all event from the login stream.*/
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        ReactorReturnCode IWlItemHandler.ResumeAll()
        {
            var stream = StreamDlList.Start();
            while (stream != null)
            {
                if (stream.NumPausedRequestsCount > 0)
                {
                    var request = stream.UserRequestDlList.Start();
                    while (request != null)
                    {
                        request.RequestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                        request = stream.UserRequestDlList.Forth();
                    }

                    stream.NumPausedRequestsCount = 0;
                }
                stream = StreamDlList.Forth();
            }

            stream = PendingSendMsgDlList.Start();
            while (stream != null)
            {
                if (stream.NumPausedRequestsCount > 0)
                {
                    var request = stream.UserRequestDlList.Start();
                    while (request != null)
                    {
                        request.RequestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                        request = stream.UserRequestDlList.Forth();
                    }

                    stream.NumPausedRequestsCount = 0;
                }
                stream = PendingSendMsgDlList.Forth();
            }

            foreach (var pendingRequestList in PendingRequestByNameDict.Values)
            {
                foreach (var userRequest in pendingRequestList)
                {
                    userRequest.RequestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                }
            }

            foreach (var pendingRequestList in PendingRequestByIdDict.Values)
            {
                foreach (var userRequest in pendingRequestList)
                {
                    userRequest.RequestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void OnRequestTimeout(WlStream stream)
        {
            if (stream is WlItemStream wlItemStream)
            {
                CloseWlItemStream(wlItemStream);

                // fanout status to user and add requests to request timeout list
                var request = (WlItemRequest?)wlItemStream.UserRequestDlList.Pop();
                while (request != null)
                {
                    request.ReqState = WlRequest.State.PENDING_REQUEST;

                    // Add to request timeout list only if single open supported.
                    if(m_Watchlist.LoginHandler.SupportSingleOpen)
                    {
                        RequestTimeoutDlList.PushBack(request, WlItemRequest.TIMEOUT_ITEM_REQUEST_LINK);
                    }

                    SendStatus(request.RequestMsg.StreamId, request.RequestMsg.DomainType, "Request timeout",
                        request.RequestMsg.CheckPrivateStream(), out _);

                    request = (WlItemRequest?)wlItemStream.UserRequestDlList.Pop();
                }

                wlItemStream.ReturnToPool();

                m_Watchlist.Reactor!.SendWatchlistDispatchNowEvent(m_Watchlist.ReactorChannel!);
            }
        }
    }
}
