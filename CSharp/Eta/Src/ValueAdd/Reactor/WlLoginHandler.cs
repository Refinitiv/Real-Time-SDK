/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The login stream handler for the watchlist.
    /// </summary>
    sealed internal class WlLoginHandler : IWlLoginHandler
    {
        Watchlist m_Watchlist;
        // Initially set to true since we have to notify the other handlers the first time
        // stream is open.
        private bool m_NotifyStreamOpen = true;

        /// <summary>
        /// Stream associated with Login handler
        /// </summary>
        public WlStream? Stream { get; set; }

        /// <summary>
        /// Returns whether or not watchlist login stream supports single open.
        /// </summary>
        public bool SupportSingleOpen
        {
            get
            {
                if (m_LoginRequest != null && m_LoginRequest.HasAttrib
                    && m_LoginRequest.LoginAttrib.HasSingleOpen)
                {
                    return (m_LoginRequest.LoginAttrib.SingleOpen > 0 ? true : false);
                }
                else
                {
                    return (m_Watchlist.ConsumerRole?.RdmLoginRequest?.LoginAttrib.SingleOpen > 0);
                }
            }
        }

        /// <summary>
        /// Returns whether or not watchlist login stream supports allow suspect
        /// data.
        /// </summary>
        public bool SupportAllowSuspectData
        {
            get
            {
                if (m_LoginRequest != null
                    && m_LoginRequest.HasAttrib
                    && m_LoginRequest.LoginAttrib.HasAllowSuspectData)
                {
                    return (m_LoginRequest.LoginAttrib.AllowSuspectData > 0);
                }
                else
                {
                    return m_Watchlist.ConsumerRole?.RdmLoginRequest?.LoginAttrib.AllowSuspectData > 0;
                }
            }

        }

        /// <summary>
        /// Returns whether or not login info is provided by the application.
        /// </summary>
        internal bool UserLoginStreamProvided
        {
            get
            {
                if (m_UserLoginStreamOpen == false)
                    return false;

                if ((m_LoginRequest != null || m_Watchlist.ConsumerRole?.RdmLoginRequest != null))
                    return true;
                else
                    return false;
            }
        }

        /// <summary>
        /// Returns whether or not watchlist login stream supports batch request.
        /// </summary>
        internal bool SupportBatchRequests
        {
            // always support batch requests
            get => true;
        }

        /// <summary>
        /// Returns whether or not watchlist login stream supports enhanced symbol list.
        /// </summary>
        internal bool SupportEnhancedSymbolList
        {
            // always support enhanced symbol list
            get => true;
        }

        /// <summary>
        /// Returns whether or not watchlist login stream supports optimized pause and resume.
        /// </summary>
        public bool SupportOptimizedPauseResume
        {
            get
            {
                if (m_LoginRefresh != null
                    && m_LoginRefresh.LoginRefresh != null
                    && m_LoginRefresh.LoginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume)
                {
                    return (m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportOptimizedPauseResume > 0);
                }
                else
                {
                    return false;
                }
            }
        }

        /// <summary>
        /// Returns whether or not watchlist login stream supports view requests.
        /// </summary>
        internal bool SupportViewRequests
        {
            get
            {
                if (m_LoginRefresh?.LoginRefresh?.SupportedFeatures.HasSupportViewRequests == true)
                {
                    return (m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportViewRequests > 0);
                }
                else
                {
                    return false;
                }
            }
        }

        /// <summary>
        /// Returns whether or not provider supports posting.
        /// </summary>
        public bool SupportPost
        {
            get
            {
                if (m_LoginRefresh?.LoginRefresh?.SupportedFeatures.HasSupportPost == true)
                {
                    return (m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportOMMPost > 0);
                }
                else
                {
                    return false;
                }
            }
        }

        

        internal int m_LoginStreamId;
        internal LoginRequest? m_LoginRequest;
        public LoginRequest? LoginRequestForEDP { get; set; }
        internal LoginRequest? m_TempLoginRequest;

        ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        Msg m_TempMsg = new Msg();

        // LoginRefresh m_LoginRefresh;
        LoginMsg m_LoginRefresh;
        // LoginStatus m_LoginStatus;
        LoginMsg m_LoginStatus;
        // LoginRTT loginRTT;
        LoginMsg m_LoginRTT;

        IStatusMsg m_StatusMsg;

        Codec.Buffer m_TempBuffer;
        internal bool m_AwaitingResumeAll;
        internal bool m_UserLoginStreamOpen;
        /// <summary>
        /// tracks pending requests so re-issues aren't sent until refresh is received
        /// </summary>
        internal int m_RequestCount;
        internal bool m_HasPendingRequest;
        public bool IsRttEnabled { get; set; } = false;

        public WlLoginHandler(Watchlist watchlist)
        {
            m_Watchlist = watchlist;

            m_LoginRefresh = new LoginMsg();
            m_LoginRefresh.LoginMsgType = LoginMsgType.REFRESH;

            m_LoginStatus = new LoginMsg();
            m_LoginStatus.LoginMsgType = LoginMsgType.STATUS;

            m_LoginRTT = new LoginMsg();
            m_LoginRTT.LoginMsgType = LoginMsgType.RTT;

            m_StatusMsg = new Msg();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)DomainType.LOGIN;

            m_TempBuffer = new Codec.Buffer();
            m_TempBuffer.Data("");
            m_TempLoginRequest = new LoginRequest();
            m_UserLoginStreamOpen = true;

            m_LoginStreamId = WlStreamIdManager.LOGIN_STREAM_ID;

            m_HasPendingRequest = false;
        }

        /* Clear state of watchlist login handler for re-use. */
        public void Clear()
        {
            // this handler is still associated with same watchlist so don't set
            // watchlist to null
            Stream?.ReturnToPool();
            Stream = null;
            m_LoginRequest = null;
            LoginRequestForEDP = null;
            m_TempLoginRequest?.Clear();

            m_LoginRefresh.Clear();
            m_LoginRefresh.LoginMsgType = LoginMsgType.REFRESH;

            m_LoginStatus.Clear();
            m_LoginStatus.LoginMsgType = LoginMsgType.STATUS;

            m_LoginRTT.Clear();
            m_LoginRTT.LoginMsgType = LoginMsgType.RTT;
            IsRttEnabled = false;

            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;

            m_TempBuffer.Clear();
            m_TempBuffer.Data("");
            m_TempMsg.Clear();
            m_AwaitingResumeAll = false;
            m_RequestCount = 0;
            m_HasPendingRequest = false;
            m_NotifyStreamOpen = true;
        }

        /// <summary>
        /// WARNING: NOT IMPLEMENTED
        /// </summary>
        /// <param name="location"></param>
        /// <param name="msg"></param>
        /// <param name="wlRequest"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        /// <exception cref="NotImplementedException"></exception>
        public ReactorReturnCode CallbackUserWithMsg(string location, IMsg msg, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            throw new NotImplementedException();
        }

        public ReactorReturnCode CallbackUserWithMsgBase(string location, IMsg msg, IRdmMsg msgBase, WlRequest wlRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode ret = m_Watchlist.Reactor!.SendAndHandleLoginMsgCallback(location,
                m_Watchlist.ReactorChannel!, null, (Msg)msg, (LoginMsg)msgBase, out errorInfo);

            if (ret == ReactorCallbackReturnCode.RAISE)
            {
                return m_Watchlist.Reactor.SendAndHandleDefaultMsgCallback(location,
                    m_Watchlist.ReactorChannel!, null, (Msg)msg, wlRequest, out errorInfo);
            }

            return (ReactorReturnCode)ret;
        }

        public void ChannelDown()
        {
            int streamId = (m_LoginRequest != null ? m_LoginRequest.StreamId : 0);

            if (Stream != null)
            {
                Stream.ChannelDown();

                m_HasPendingRequest = false;

                // set state to closed recover if current state isn't closed
                if (Stream.State.StreamState() == StreamStates.OPEN)
                {
                    Stream.State.Clear();
                    Stream.State.StreamState(StreamStates.CLOSED_RECOVER);
                    Stream.State.DataState(DataStates.SUSPECT);

                    // call back user with login status of OPEN/SUSPECT
                    m_StatusMsg.Clear();
                    m_StatusMsg.MsgClass = MsgClasses.STATUS;
                    m_StatusMsg.DomainType = (int)DomainType.LOGIN;
                    m_StatusMsg.StreamId = streamId;
                    m_StatusMsg.ApplyHasState();
                    m_StatusMsg.State.StreamState(StreamStates.OPEN);
                    m_StatusMsg.State.DataState(DataStates.SUSPECT);
                    m_StatusMsg.State.Code(StateCodes.NONE);
                    m_StatusMsg.State.Text(m_TempBuffer);

                    m_LoginStatus.Clear();
                    m_LoginStatus.LoginMsgType = LoginMsgType.STATUS;
                    m_LoginStatus.StreamId = streamId;
                    m_LoginStatus.LoginStatus!.HasState = true;
                    m_LoginStatus.LoginStatus.State.StreamState(StreamStates.OPEN);
                    m_LoginStatus.LoginStatus.State.DataState(DataStates.SUSPECT);
                    m_LoginStatus.LoginStatus.State.Code(StateCodes.NONE);
                    m_LoginStatus.LoginStatus.State.Text(m_TempBuffer);

                    m_UserLoginStreamOpen = false;
                    CallbackUserWithMsgBase("WlLoginHandler.ChannelDown", m_StatusMsg,
                        m_LoginStatus, m_Watchlist.StreamIdToWlRequestDict![streamId], out _);
                }
            }
        }

        public void ChannelUp(out ReactorErrorInfo? errorInfo)
        {
            m_NotifyStreamOpen = true;
            SendLoginRequest(false, out errorInfo);
        }

        /// <summary>
        /// Dispatch all streams for the handler.
        /// </summary>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        public ReactorReturnCode Dispatch(out ReactorErrorInfo? errorInfo)
        {
            if (Stream != null && m_HasPendingRequest)
            {
                m_HasPendingRequest = false;
                return Stream.SendMsgOutOfLoop(Stream.RequestMsg, m_SubmitOptions, out errorInfo);
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitMsg(WlRequest wlRequest, IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            switch (msg.MsgClass)
            {
                case MsgClasses.CLOSE:
                    
                    Stream!.State.StreamState(StreamStates.CLOSED);
                    Stream!.State.DataState(DataStates.SUSPECT);
                    
                    // notify other handlers that login stream is closed
                    ReactorReturnCode ret1 = m_Watchlist.ItemHandler.LoginStreamClosed(null);
                    ReactorReturnCode ret2 = m_Watchlist.DirectoryHandler.LoginStreamClosed();

                    if (ret1 < ReactorReturnCode.SUCCESS)
                    {
                        errorInfo = null;
                        return ret1;
                    }
                    else if (ret2 < ReactorReturnCode.SUCCESS)
                    {
                        errorInfo = null;
                        return ret2;
                    }

                    // send message
                    if ((ret = Stream!.SendMsgOutOfLoop(msg, submitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    // close watchlist request
                    m_Watchlist.CloseWlRequest(wlRequest);
                    wlRequest.ReturnToPool();

                    // close stream
                    Stream.Close();
                    Stream.ReturnToPool();
                    Stream = null;
                    break;
                case MsgClasses.POST:
                    if (m_LoginRefresh!.LoginRefresh!.SupportedFeatures.HasSupportPost)
                    {
                        if (Stream!.State.StreamState() == StreamStates.OPEN)
                        {
                            bool resetServiceId = false;

                            // validate post submit
                            if ((ret = Stream.ValidatePostSubmit((IPostMsg)msg, out errorInfo)) != ReactorReturnCode.SUCCESS)
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
                                        "WlLoginHandler.SubmitMsg",
                                        "Post message submitted with service name but no message key.");
                                }

                                if ((ret = m_Watchlist.ChangeServiceNameToID(((IPostMsg)msg).MsgKey, submitOptions.ServiceName, out errorInfo)) < ReactorReturnCode.SUCCESS)
                                {
                                    return ret;
                                }

                                // set resetServiceId flag
                                resetServiceId = true;
                            }

                            int userStreamId = msg.StreamId;
                            msg.StreamId = Stream.StreamId;
                            ret = Stream.SendMsgOutOfLoop(msg, submitOptions, out errorInfo);
                            msg.StreamId = userStreamId;

                            // reset service id if checkAck() return false
                            if (resetServiceId && (!((IPostMsg)msg).CheckAck() || (ret < ReactorReturnCode.SUCCESS)))
                            {
                                ((IPostMsg)msg).MsgKey.Flags = (((IPostMsg)msg).MsgKey.Flags & ~MsgKeyFlags.HAS_SERVICE_ID);
                                ((IPostMsg)msg).MsgKey.ServiceId = 0;
                                resetServiceId = false;
                            }

                            // return if send message not successful
                            if (ret < ReactorReturnCode.SUCCESS)
                                return ret;
                            else
                            {
                                if (((IPostMsg)msg).CheckAck())
                                {
                                    // increment number of outstanding post messages
                                    m_Watchlist.NumOutstandingPosts += 1;

                                    // update post tables
                                    ret = Stream.UpdatePostTables((IPostMsg)msg, out _);

                                    // reset service id if necessary
                                    if (resetServiceId)
                                    {
                                        ((IPostMsg)msg).MsgKey.Flags = (((IPostMsg)msg).MsgKey.Flags & ~MsgKeyFlags.HAS_SERVICE_ID);
                                        ((IPostMsg)msg).MsgKey.ServiceId = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            // cannot submit post when stream is not open
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.SubmitMsg",
                                "Cannot submit PostMsg when stream not in open state.");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.SubmitMsg",
                            "Posting not supported by provider");
                    }
                    break;
                case MsgClasses.GENERIC:
                    if (Stream!.State.StreamState() == StreamStates.OPEN)
                    {
                        bool resetServiceId = false;

                        // replace service id if message submitted with service name
                        if (submitOptions.ServiceName != null)
                        {
                            if (!((IGenericMsg)msg).CheckHasMsgKey())
                            {
                                return Reactor.PopulateErrorInfo(out errorInfo,
                                    ReactorReturnCode.INVALID_USAGE,
                                    "WlLoginHandler.SubmitMsg",
                                    "Generic message submitted with service name but no message key.");

                            }

                            if ((ret = m_Watchlist.ChangeServiceNameToID(((IGenericMsg)msg).MsgKey, submitOptions.ServiceName, out errorInfo)) < ReactorReturnCode.SUCCESS)
                            {
                                return ret;
                            }

                            // set resetServiceId flag
                            resetServiceId = true;
                        }

                        // send message
                        ret = Stream.SendMsgOutOfLoop(msg, submitOptions, out _);

                        // reset service id if necessary
                        if (resetServiceId)
                        {
                            ((IGenericMsg)msg).MsgKey.Flags = (((IGenericMsg)msg).MsgKey.Flags & ~MsgKeyFlags.HAS_SERVICE_ID);
                            ((IGenericMsg)msg).MsgKey.ServiceId = 0;
                        }

                        // return if send message not successful
                        if (ret < ReactorReturnCode.SUCCESS)
                        {
                            errorInfo = null;
                            return ret;
                        }
                    }
                    else
                    {
                        // cannot submit generic message when stream is not open
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.SubmitMsg",
                            "Cannot submit GenericMsg when stream not in open state.");
                    }
                    break;
                default:
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlLoginHandler.SubmitMsg",
                        $"Invalid message class ({msg.MsgClass}) submitted to Watchlist login handler");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public ReactorReturnCode SubmitRequest(WlRequest wlRequest, IRequestMsg requestMsg, bool isReissue, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            // check for different login stream id
            // user is allowed to open a different stream if login stream is closed
            if (m_LoginRequest != null
                && requestMsg.StreamId != m_LoginRequest.StreamId
                && Stream != null
                && Stream.State.StreamState() != StreamStates.CLOSED)
            {
                // cannot have more than one login stream
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.SubmitRequest",
                    "Cannot have more than one login stream with watchlist enabled.");
            }

            if (Stream == null)
            {
                if (!m_Watchlist.StreamIdToWlStreamDict!.ContainsKey(requestMsg.StreamId))
                {
                    // create stream
                    Stream = m_Watchlist.Reactor!.GetReactorPool().CreateWlStream();
                    Stream.WlHandler = this;
                    Stream.Watchlist = m_Watchlist;
                    Stream.StreamId = requestMsg.StreamId;
                    Stream.StreamDomainType = requestMsg.DomainType;
                }
                else // stream already exists with this id
                {
                    return Reactor.PopulateErrorInfo(
                        out errorInfo,
                        ReactorReturnCode.INVALID_USAGE,
                        "WlLoginHandler.SubmitRequest",
                        $"Stream already exists with id of {requestMsg.StreamId}.");
                }
            }

            if (isReissue) // subsequent login request
            {
                // convert to rdm login request
                m_TempLoginRequest!.Clear();
                m_Watchlist.ConvertCodecMsgToRDM((Msg)requestMsg, m_TempLoginRequest);
                // handle reissue
                if ((ret = HandleReissue(m_TempLoginRequest, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    return ret;
                }
                // copy to official login request
                m_LoginRequest!.Clear();

                m_TempLoginRequest.Copy(m_LoginRequest);

                // if pause not supported, remove flag from requestMsg
                if (!(m_LoginRefresh!.LoginRefresh!.HasFeatures
                      && m_LoginRefresh.LoginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume
                      && m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportOptimizedPauseResume == 1)
                    && requestMsg.CheckPause())
                {
                    requestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                }
            }
            else // first login request
            {
                // convert to rdm login request and save
                if (m_LoginRequest == null)
                {
                    m_LoginRequest = new LoginRequest();
                }
                else
                {
                    m_LoginRequest.Clear();
                }

                m_Watchlist.ConvertCodecMsgToRDM((Msg)requestMsg, m_LoginRequest);

                // if pause is set, remove pause as we do not send it in login request
                if (requestMsg.CheckPause())
                {
                    requestMsg.Flags &= ~RequestMsgFlags.PAUSE;
                }
            }

            // send message if request not pending
            if (m_RequestCount == 0)
            {
                if (m_Watchlist.IsChannelUp())
                {
                    if ((ret = Stream.SendMsgOutOfLoop(requestMsg, submitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        if (!isReissue)
                        {
                            m_LoginRequest = null;
                            Stream.ReturnToPool();
                            Stream = null;
                        }
                        return ret;
                    }
                }
            }
            if (!requestMsg.CheckNoRefresh() && m_Watchlist.IsChannelUp())
            {
                m_RequestCount++;
            }

            // save stream info
            if (!isReissue)
            {
                // only save service name when not a reissue
                wlRequest.WatchlistStreamInfo.ServiceName = submitOptions.ServiceName;

                // add stream to watchlist table
                m_Watchlist.StreamIdToWlStreamDict![requestMsg.StreamId] = Stream;
            }
            wlRequest.WatchlistStreamInfo.UserSpec = submitOptions.RequestMsgOptions.UserSpecObj;

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        public void AddPendingRequest(WlStream wlStream)
        {
            m_HasPendingRequest = true;
        }

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            Debug.Assert(Stream == wlStream);
            if (m_Watchlist.ReactorChannel!.EnableSessionManagement())
            {
                Debug.Assert(msg.StreamId == LoginRequestForEDP!.StreamId);
            }
            else
            {
                Debug.Assert(msg.StreamId == m_LoginRequest!.StreamId);
            }

            ReactorReturnCode ret1, ret2;

            ReactorReturnCode ret = msg.MsgClass switch
            {
                MsgClasses.REFRESH => ReadRefreshMsg(wlStream, dIter, msg, out errorInfo),
                MsgClasses.STATUS => ReadStatusMsg(wlStream, dIter, msg, out errorInfo),
                MsgClasses.GENERIC => ReadGenericMsg(wlStream, dIter, msg, out errorInfo),
                MsgClasses.ACK => ReadAckMsg(wlStream, dIter, msg, out errorInfo),
                _ => Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlLoginHandler.ReadMsg",
                        $"Invalid message class ({msg.MsgClass}) received by Watchlist login handler")
            };

            // If application closed the login stream while in callback, do not process further.
            if (Stream == null)
                return ret;

            // handle any state transition
            if (ret == ReactorReturnCode.SUCCESS)
            {
                switch (wlStream.State.StreamState())
                {
                    case StreamStates.CLOSED:
                    case StreamStates.CLOSED_RECOVER:
                    case StreamStates.REDIRECTED:
                        m_NotifyStreamOpen = true;
                        // notify other handlers that login stream is closed
                        ret1 = m_Watchlist.ItemHandler.LoginStreamClosed(wlStream.State);
                        ret2 = m_Watchlist.DirectoryHandler.LoginStreamClosed();

                        if (ret1 < ReactorReturnCode.SUCCESS)
                        {
                            return ret1;
                        }
                        else if (ret2 < ReactorReturnCode.SUCCESS)
                        {
                            return ret2;
                        }

                        if (wlStream.State.StreamState() == StreamStates.CLOSED
                            || wlStream.State.StreamState() == StreamStates.REDIRECTED)
                        {
                            // close watchlist request
                            WlRequest wlRequest = m_Watchlist.StreamIdToWlRequestDict![msg.StreamId];
                            m_Watchlist.CloseWlRequest(wlRequest);
                            wlRequest.ReturnToPool();

                            // close stream if state is closed
                            Stream.Close();
                            Stream.ReturnToPool();
                            Stream = null;
                            LoginRequestForEDP = null;
                        }
                        break;
                    case StreamStates.OPEN:
                        if (wlStream.State.DataState() == DataStates.OK && m_NotifyStreamOpen)
                        {
                            m_NotifyStreamOpen = false;
                            // Connection is established. Reset reconnect timeout.
                            m_Watchlist.ReactorChannel.ResetReconnectTimers();

                            // notify other handlers that login stream is open
                            ret1 = m_Watchlist.DirectoryHandler.LoginStreamOpen(out errorInfo);
                            ret2 = m_Watchlist.ItemHandler.LoginStreamOpen(out errorInfo);

                            if (ret1 < ReactorReturnCode.SUCCESS)
                            {
                                return ret1;
                            }
                            else if (ret2 < ReactorReturnCode.SUCCESS)
                            {
                                return ret2;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            return ret;
        }

        /// <summary>
        /// Reads a refresh message.
        /// </summary>
        ReactorReturnCode ReadRefreshMsg(WlStream wlStream, DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            // make sure refresh complete flag is set
            // login handler doesn't handle multi-part login refreshes
            if (!((IRefreshMsg)msg).CheckRefreshComplete())
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.FAILURE,
                    "WlLoginHandler.ReadRefreshMsg",
                    "Watchlist doesn't handle multi-part login refresh.");
            }

            // notify stream that response received if solicited
            if (((IRefreshMsg)msg).CheckSolicited())
            {
                wlStream.ResponseReceived();
            }

            // convert to rdm login refresh and save
            CodecReturnCode decodeError;
            if ((decodeError = m_LoginRefresh.Decode(dIter, msg)) != CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.FAILURE,
                    "WlLoginHandler.ReadRefreshMsg",
                    $"Could not decode message received by Watchlist login handler: {decodeError.GetAsInfo()}");
            }
            // alter login refresh for user SingleOpen and AllowSuspectData must match the
            // user's request, regardless of provider support.
            //
            // SupportOptimizedPauseResume must be passed on from the provider, but must
            // NOT pass on SupportPauseResume.
            //
            // SupportBatchRequests is supported regardless of provider support.
            //
            // Enhanced symbol list data streams are always supported.
            m_LoginRefresh.LoginRefresh!.HasAttrib = true;
            m_LoginRefresh.LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginRefresh.LoginAttrib.SingleOpen = SupportSingleOpen ? 1 : 0;
            m_LoginRefresh.LoginRefresh.LoginAttrib.HasAllowSuspectData = true;
            m_LoginRefresh.LoginRefresh.LoginAttrib.AllowSuspectData = SupportAllowSuspectData ? 1 : 0;

            m_LoginRefresh.LoginRefresh.HasFeatures = true;
            m_LoginRefresh.LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportBatchRequests = Login.BatchSupportFlags.SUPPORT_REQUESTS;
            m_LoginRefresh.LoginRefresh.SupportedFeatures.HasSupportEnhancedSymbolList = true;
            m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportEnhancedSymbolList = Login.EnhancedSymbolListSupportFlags.DATA_STREAMS;

            // set state from login refresh
            m_LoginRefresh.LoginRefresh.State.Copy(wlStream.State);

            if (m_LoginRefresh.LoginRefresh.State.StreamState() == StreamStates.CLOSED_RECOVER)
            {
                m_LoginRefresh.LoginRefresh.State.StreamState(StreamStates.OPEN);
                m_LoginRefresh.LoginRefresh.State.DataState(DataStates.SUSPECT);
                return CallbackUserAndDisconnect("WlLoginHandler.ReadRefreshMsg", msg, m_LoginRefresh, out errorInfo);
            }

            // call back user
            ret = CallbackUserWithMsgBase("WlLoginHandler.ReadRefreshMsg", msg,
                m_LoginRefresh, m_Watchlist.StreamIdToWlRequestDict![msg.StreamId], out errorInfo);

            // send pending request if necessary
            if (m_RequestCount > 0)
            {
                m_RequestCount--;
            }
            if (m_RequestCount > 0 && ret != ReactorReturnCode.FAILURE)
            {
                m_TempMsg.Clear();
                m_Watchlist.ConvertRDMToCodecMsg(m_LoginRequest!, m_TempMsg);
                if ((ret = wlStream.SendMsgOutOfLoop(m_TempMsg, m_SubmitOptions, out errorInfo)) == ReactorReturnCode.SUCCESS)
                {
                    if (!m_LoginRequest!.NoRefresh)
                    {
                        m_RequestCount = 1;
                    }
                    else
                    {
                        m_RequestCount = 0;
                    }
                }
            }

            return ret;
        }

        /// <summary>
        /// Reads a status message.
        /// </summary>
        ReactorReturnCode ReadStatusMsg(WlStream wlStream, DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            // convert to rdm login status and save
            m_LoginStatus.Decode(dIter, msg);

            // notify stream that response received
            wlStream.ResponseReceived();

            if (m_LoginStatus?.LoginStatus?.HasState == true)
            {
                m_LoginStatus.LoginStatus.State.Copy(wlStream.State);

                // if CLOSED_RECOVER, change state to OPEN/SUSPECT for call back
                if (m_LoginStatus.LoginStatus.State.StreamState() == StreamStates.CLOSED_RECOVER)
                {
                    m_LoginStatus.LoginStatus.State.StreamState(StreamStates.OPEN);
                    m_LoginStatus.LoginStatus.State.DataState(DataStates.SUSPECT);
                    return CallbackUserAndDisconnect("WlLoginHandler.ReadStatusMsg", msg, m_LoginStatus, out errorInfo);
                }
            }

            // call back user
            return CallbackUserWithMsgBase("WlLoginHandler.ReadStatusMsg", msg, m_LoginStatus!,
                m_Watchlist.StreamIdToWlRequestDict![msg.StreamId], out errorInfo);
        }

        /// <summary>
        /// Reads a generic message.
        /// </summary>
        ReactorReturnCode ReadGenericMsg(WlStream wlStream, DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            WlRequest wlRequest = m_Watchlist.StreamIdToWlRequestDict![wlStream.StreamId];

            //Redirect message to the provider.
            bool isRttMessage = (DataTypes.ELEMENT_LIST == msg.ContainerType);

            // call back user
            if (!isRttMessage)
            {
                return m_Watchlist.Reactor!.SendAndHandleDefaultMsgCallback("WlLoginHandler.ReadGenericMsg",
                    m_Watchlist.ReactorChannel!, null, (Msg)msg, wlRequest, out errorInfo);
            }
            else
            {
                if (IsRttEnabled)
                    SubmitMsg(wlRequest, msg, m_SubmitOptions, out errorInfo);

                m_LoginRTT.Clear();
                m_LoginRTT.Decode(dIter, msg);
                return (ReactorReturnCode)m_Watchlist.Reactor!.SendAndHandleLoginMsgCallback("WlLoginHandler.ReadGenericMsg",
                    m_Watchlist.ReactorChannel!, null, (Msg)msg, m_LoginRTT, out errorInfo);
            }
        }

        /// <summary>
        /// Reads an Ack message.
        /// </summary>
        ReactorReturnCode ReadAckMsg(WlStream wlStream, DecodeIterator dIter, IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            // handle the post Ack
            if (wlStream.HandlePostAck(msg))
            {
                // call back user if ACK was processed
                if (msg.DomainType != (int)DomainType.LOGIN)
                {
                    return m_Watchlist.ItemHandler.CallbackUserWithMsg(
                        "WlLoginHandler.ReadAckMsg", msg, m_Watchlist.StreamIdToWlRequestDict![msg.StreamId], out errorInfo);
                }
                else
                {
                    return CallbackUserWithMsg("WlLoginHandler.ReadAckMsg", msg, m_Watchlist.StreamIdToWlRequestDict![msg.StreamId],
                        out errorInfo);
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends login request
        /// </summary>
        void SendLoginRequest(bool noRefresh, out ReactorErrorInfo? errorInfo)
        {
            bool newStream = false;
            bool newRequest = false;

            m_UserLoginStreamOpen = true;

            LoginRequest loginRequest;

            if (m_Watchlist.ReactorChannel!.EnableSessionManagement())
            {
                string authToken = m_Watchlist.ReactorChannel.RDMLoginRequestRDP!.UserName.ToString();

                /* Don't send a login request as the access token is invalid */
                if (String.IsNullOrEmpty(authToken))
                {
                    errorInfo = null;
                    return;
                }

                if (LoginRequestForEDP == null
                    && m_Watchlist.ConsumerRole!.RdmLoginRequest != null)
                {
                    newRequest = true;
                    LoginRequestForEDP = new LoginRequest();

                    m_Watchlist.ReactorChannel.RDMLoginRequestRDP.Copy(LoginRequestForEDP);
                }
                else
                {
                    LoginRequestForEDP!.UserName.Data(authToken);
                }

                loginRequest = LoginRequestForEDP;

                if (noRefresh)
                    loginRequest.NoRefresh = true;

                // create login stream if not created yet
                if (Stream == null)
                {
                    newStream = true;

                    LoginRequestForEDP.StreamId = m_LoginStreamId;

                    // create stream
                    Stream = m_Watchlist.Reactor!.GetReactorPool().CreateWlStream();
                    Stream.WlHandler = this;
                    Stream.Watchlist = m_Watchlist;
                    Stream.StreamId = LoginRequestForEDP.StreamId;
                    Stream.StreamDomainType = LoginRequestForEDP.DomainType;
                }

                m_LoginRequest = LoginRequestForEDP;
            }
            else
            {
                // create login request if not created yet and role has one
                if (m_LoginRequest == null
                    && m_Watchlist.ConsumerRole!.RdmLoginRequest != null)
                {
                    newRequest = true;
                    m_LoginRequest = new LoginRequest();
                    m_Watchlist.ConsumerRole.RdmLoginRequest.Copy(m_LoginRequest);

                    // create login stream if not created yet
                    if (Stream == null)
                    {
                        newStream = true;

                        m_LoginRequest.StreamId = m_LoginStreamId;

                        // create stream
                        Stream = m_Watchlist.Reactor!.GetReactorPool().CreateWlStream();
                        Stream.WlHandler = this;
                        Stream.Watchlist = m_Watchlist;
                        Stream.StreamId = m_LoginRequest.StreamId;
                        Stream.StreamDomainType = m_LoginRequest.DomainType;
                    }
                }
                loginRequest = m_LoginRequest!;

                if (noRefresh)
                    loginRequest.NoRefresh = true;
            }

            // send login request via stream
            if (loginRequest != null && Stream != null)
            {
                if (loginRequest.Pause)
                {
                    loginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
                }
                if (!noRefresh && loginRequest.NoRefresh)
                {
                    loginRequest.Flags &= ~LoginRequestFlags.NO_REFRESH;
                }
                m_TempMsg.Clear();
                m_Watchlist.ConvertRDMToCodecMsg(loginRequest, m_TempMsg);

                if (Stream.SendMsgOutOfLoop(m_TempMsg, m_SubmitOptions, out errorInfo) >= ReactorReturnCode.SUCCESS)
                {
                    // if successful update tables
                    if (newRequest)
                    {
                        // add to watchlist request table
                        WlRequest wlRequest = m_Watchlist.Reactor!.GetReactorPool().CreateWlRequest();
                        m_TempMsg.Clear();
                        m_Watchlist.ConvertRDMToCodecMsg(loginRequest, m_TempMsg);
                        wlRequest.RequestMsg.Clear();
                        m_TempMsg.Copy(wlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS);
                        wlRequest.Handler = this;
                        m_Watchlist.StreamIdToWlRequestDict![loginRequest.StreamId] = wlRequest;
                    }

                    if (newStream)
                    {
                        // add stream to watchlist table
                        m_Watchlist.StreamIdToWlStreamDict![loginRequest.StreamId] = Stream;
                    }

                    m_RequestCount = 1;
                }
                else // sendMsg failed
                {
                    // if new request, set loginRequest to null
                    if (newRequest)
                    {
                        m_LoginRequest = null;
                        LoginRequestForEDP = null;
                    }

                    // if new stream, return stream to pool
                    if (newStream)
                    {
                        Stream.ReturnToPool();
                        Stream = null;
                    }
                }
            }
            errorInfo = null;
        }

        public void OnRequestTimeout(WlStream stream)
        {
            LoginRequest loginRequest;
            if (m_Watchlist.ReactorChannel!.EnableSessionManagement())
            {
                loginRequest = LoginRequestForEDP!;
            }
            else
            {
                loginRequest = m_LoginRequest!;
            }

            int streamId = (loginRequest?.StreamId ?? 0);

            // call back user with login status of OPEN/SUSPECT
            m_StatusMsg.Clear();
            m_StatusMsg.MsgClass = MsgClasses.STATUS;
            m_StatusMsg.DomainType = (int)DomainType.LOGIN;
            m_StatusMsg.StreamId = streamId;
            m_StatusMsg.ApplyHasState();
            m_StatusMsg.State.StreamState(StreamStates.OPEN);
            m_StatusMsg.State.DataState(DataStates.SUSPECT);
            m_StatusMsg.State.Code(StateCodes.TIMEOUT);
            m_StatusMsg.State.Text(m_TempBuffer);

            m_LoginStatus.Clear();
            m_LoginStatus.LoginMsgType = LoginMsgType.STATUS;
            m_LoginStatus.StreamId = streamId;
            m_LoginStatus.LoginStatus!.HasState = true;
            m_LoginStatus.LoginStatus.State.StreamState(StreamStates.OPEN);
            m_LoginStatus.LoginStatus.State.DataState(DataStates.SUSPECT);
            m_LoginStatus.LoginStatus.State.Code(StateCodes.TIMEOUT);
            m_LoginStatus.LoginStatus.State.Text(m_TempBuffer);

            CallbackUserWithMsgBase("WlLoginHandler.OnRequestTimeout", (Msg)m_StatusMsg, m_LoginStatus,
                m_Watchlist.StreamIdToWlRequestDict![m_StatusMsg.StreamId],
                    out _);

            // re-send login request
            m_TempMsg.Clear();
            m_Watchlist.ConvertRDMToCodecMsg(loginRequest!, m_TempMsg);
            stream.SendMsgOutOfLoop(m_TempMsg, m_SubmitOptions, out _);
        }

        /// <summary>
        /// Handles a login request reissue.
        /// </summary>
        /// <param name="loginRequest"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        ReactorReturnCode HandleReissue(LoginRequest loginRequest, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            // validate and handle login credentials update

            if ((ret = ValidateReissue(loginRequest, out errorInfo)) != ReactorReturnCode.SUCCESS)
            {
                return ret;
            }
            // handle pause
            if (loginRequest.Pause)
            {
                if (m_LoginRefresh!.LoginRefresh!.HasFeatures
                    && m_LoginRefresh.LoginRefresh.SupportedFeatures.HasSupportOptimizedPauseResume
                    && m_LoginRefresh.LoginRefresh.SupportedFeatures.SupportOptimizedPauseResume == 1)
                {
                    m_AwaitingResumeAll = true;

                    // notify item handler to pause all
                    ret = m_Watchlist.ItemHandler.PauseAll();
                    if (ret < ReactorReturnCode.SUCCESS)
                        return ret;

                    loginRequest.Flags |= LoginRequestFlags.PAUSE_ALL;
                }
            }
            else // not pause
            {
                // handle resume
                if (m_AwaitingResumeAll && !IsTokenChange(loginRequest))
                {
                    m_AwaitingResumeAll = false;

                    // notify item handler to resume all
                    ret = m_Watchlist.ItemHandler.ResumeAll();
                    if (ret < ReactorReturnCode.SUCCESS)
                        return ret;
                }
            }
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Validates a login request reissue.
        /// </summary>
        /// <param name="loginRequest"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        ReactorReturnCode ValidateReissue(LoginRequest loginRequest, out ReactorErrorInfo? errorInfo)
        {
            if (loginRequest.HasRole && loginRequest.Role != Login.RoleTypes.CONS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue",
                    "Login role is not consumer");
            }

            if (m_LoginRequest!.HasUserNameType == loginRequest.HasUserNameType)
            {
                if (m_LoginRequest.HasUserNameType
                    && (loginRequest.UserNameType != m_LoginRequest.UserNameType
                        || (m_LoginRequest.UserNameType != Login.UserIdTypes.TOKEN
                            && m_LoginRequest.UserNameType != Login.UserIdTypes.AUTHN_TOKEN
                            && !m_LoginRequest.UserName.Equals(loginRequest.UserName))))
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.INVALID_USAGE,
                        "WlLoginHandler.ValidateReissue",
                        "Login userNameType does not match existing request");
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue", "Login userNameType does not match existing request");
            }

            if (m_LoginRequest.HasDownloadConnectionConfig == loginRequest.HasDownloadConnectionConfig)
            {
                if (m_LoginRequest.HasDownloadConnectionConfig
                        && m_LoginRequest.DownloadConnectionConfig != loginRequest.DownloadConnectionConfig)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.INVALID_USAGE, "WlLoginHandler.ValidateReissue",
                        "Login downloadConnectionConfig does not match existing request");
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue",
                    "Login downloadConnectionConfig does not match existing request");
            }

            if (m_LoginRequest.HasInstanceId == loginRequest.HasInstanceId)
            {
                if (m_LoginRequest.HasInstanceId
                        && !m_LoginRequest.InstanceId.Equals(loginRequest.InstanceId))
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.INVALID_USAGE,
                        "WlLoginHandler.ValidateReissue",
                        "Login instanceId does not match existing request");
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue",
                    "Login instanceId does not match existing request");
            }

            if (m_LoginRequest.HasPassword == loginRequest.HasPassword)
            {
                if (m_LoginRequest.HasPassword
                    && !m_LoginRequest.Password.Equals(loginRequest.Password))
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.INVALID_USAGE,
                        "WlLoginHandler.ValidateReissue",
                        "Login password does not match existing request");
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue",
                    "Login password does not match existing request");
            }

            if (m_LoginRequest.HasAttrib == loginRequest.HasAttrib)
            {
                if (m_LoginRequest.HasAttrib)
                {
                    if (m_LoginRequest.LoginAttrib.HasApplicationId
                        == loginRequest.LoginAttrib.HasApplicationId)
                    {
                        if (m_LoginRequest.LoginAttrib.HasApplicationId
                            && !m_LoginRequest.LoginAttrib.ApplicationId.Equals(loginRequest.LoginAttrib.ApplicationId))
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login applicationId does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login applicationId does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasApplicationName
                        == loginRequest.LoginAttrib.HasApplicationName)
                    {
                        if (m_LoginRequest.LoginAttrib.HasApplicationName
                            && !m_LoginRequest.LoginAttrib.ApplicationName.Equals(loginRequest.LoginAttrib.ApplicationName))
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login applicationName does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login applicationName does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasPosition
                        == loginRequest.LoginAttrib.HasPosition)
                    {
                        if (m_LoginRequest.LoginAttrib.HasPosition
                            && !m_LoginRequest.LoginAttrib.Position.Equals(loginRequest.LoginAttrib.Position))
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login position does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login position does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasSingleOpen
                        == loginRequest.LoginAttrib.HasSingleOpen)
                    {
                        if (m_LoginRequest.LoginAttrib.HasSingleOpen
                            && m_LoginRequest.LoginAttrib.SingleOpen
                            != loginRequest.LoginAttrib.SingleOpen)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login singleOpen does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login singleOpen does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasAllowSuspectData
                        == loginRequest.LoginAttrib.HasAllowSuspectData)
                    {
                        if (m_LoginRequest.LoginAttrib.HasAllowSuspectData
                            && m_LoginRequest.LoginAttrib.AllowSuspectData
                            != loginRequest.LoginAttrib.AllowSuspectData)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login allowSuspectData does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login allowSuspectData does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasProvidePermissionExpressions
                        == loginRequest.LoginAttrib.HasProvidePermissionExpressions)
                    {
                        if (m_LoginRequest.LoginAttrib.HasProvidePermissionExpressions
                            && m_LoginRequest.LoginAttrib.ProvidePermissionExpressions
                            != loginRequest.LoginAttrib.ProvidePermissionExpressions)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login providePermissionExpressions does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login providePermissionExpressions does not match existing request");
                    }

                    if (m_LoginRequest.LoginAttrib.HasProvidePermissionProfile
                        == loginRequest.LoginAttrib.HasProvidePermissionProfile)
                    {
                        if (m_LoginRequest.LoginAttrib.HasProvidePermissionProfile
                            && m_LoginRequest.LoginAttrib.ProvidePermissionProfile
                            != loginRequest.LoginAttrib.ProvidePermissionProfile)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                ReactorReturnCode.INVALID_USAGE,
                                "WlLoginHandler.ValidateReissue",
                                "Login providePermissionProfile does not match existing request");
                        }
                    }
                    else
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlLoginHandler.ValidateReissue",
                            "Login providePermissionProfile does not match existing request");
                    }
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlLoginHandler.ValidateReissue",
                    "Login attrib does not match existing request");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Determines if there is a token change.
        /// </summary>
        /// <param name="loginRequest"></param>
        /// <returns></returns>
        bool IsTokenChange(LoginRequest loginRequest)
        {
            return (loginRequest.UserNameType == Login.UserIdTypes.TOKEN
                    || loginRequest.UserNameType == Login.UserIdTypes.AUTHN_TOKEN)
                && (!loginRequest.UserName.Equals(m_LoginRequest!.UserName)
                    || !loginRequest.AuthenticationExtended.Equals(m_LoginRequest.AuthenticationExtended));
        }

        /// <summary>
        /// Used when login Closed/Recoverable state is received.
        /// Notifies application, then disconnects channel.
        /// </summary>
        /// <param name="location"></param>
        /// <param name="msg"></param>
        /// <param name="loginMsg"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        ReactorReturnCode CallbackUserAndDisconnect(string location, IMsg msg, LoginMsg loginMsg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;

            if ((ret = CallbackUserWithMsgBase(location, msg, loginMsg, m_Watchlist.StreamIdToWlRequestDict![msg.StreamId], out errorInfo))
                != ReactorReturnCode.SUCCESS)
                return ret;

            Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location,
                "Received login response with Closed/Recover stream state. Disconnecting.");
            return m_Watchlist.Reactor!.Disconnect(m_Watchlist.ReactorChannel!, location, out _);
        }
    }
}
