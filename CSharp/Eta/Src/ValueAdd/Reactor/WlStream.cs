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
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Watchlist base structure for streams.
    /// </summary>
    internal class WlStream : VaNode
    {
        /// Post messages are identified by the PostId and an optional SeqNum (sequence number).
        private Dictionary<Tuple<long, long>, WlPostTimeoutInfo> m_PostIdToMsgTable = new ();
        /// list to track post ACK timeouts
        private LinkedList<WlPostTimeoutInfo> m_PostTimeoutInfoPool = new LinkedList<WlPostTimeoutInfo>();
        private WlTimeoutTimer? m_RequestTimeoutTimer;
        private Watchlist? m_Watchlist;
        internal ByteBuffer m_stateTextBuffer = new ByteBuffer(256);

        internal GCHandle m_streamHandle;
        internal GCHandle m_reqMsgHandle;
        internal GCHandle m_userReqListHandle;

        internal Action<WlTimeoutTimer> m_requestTimeoutAction;

        /// <summary>
        /// Gets or sets a Watchlist
        /// </summary>
        public Watchlist? Watchlist
        {
            get => m_Watchlist;
            set
            {
                m_Watchlist = value;
                if (m_Watchlist != null)
                {
                    m_RequestTimeoutTimer = (m_Watchlist.m_RequestTimeoutGroup != null)
                    ? m_Watchlist!.Reactor!.GetReactorPool().CreateWlTimeoutTimer(m_Watchlist!.m_RequestTimeoutGroup!, m_requestTimeoutAction)
                    : null;
                }           
            }
        }
        /// <summary>
        /// Gets or sets stream Id.
        /// </summary>
        public int StreamId { get; set; }

        /// <summary>
        /// Gets or sets domain type.
        /// </summary>
        public int StreamDomainType { get; set; }

        /// <summary>
        /// Gets or sets the watchlist handler for this request.
        /// </summary>
        public IWlHandler? WlHandler { get; set; }

        /// <summary>
        /// Returns the state of this stream.
        /// </summary>
        public State State { get; private set; } = new State();

        /// <summary>
        /// Codec Request message associated with this stream
        /// </summary>
        public IRequestMsg RequestMsg { get; private set; } = new Msg();

        /// <summary>
        /// Determines whehter this stream has a request pending response
        /// </summary>
        public bool RequestPending { get; set; } = false;
        /// <summary>
        /// Determines whether the stream is currently in use.
        /// </summary>
        public bool InUse = false;
        /// <summary>
        /// Determines the domain the stream is used for.
        /// </summary>
        public WlStreamType StreamType = WlStreamType.NONE;

        /// <summary>
        /// Closes the item stream.
        /// </summary>
        public void Close()
        {
            // set state to closed
            m_stateTextBuffer.Clear();
            State.Clear();
            State.Text().Data(m_stateTextBuffer);
            State.StreamState(StreamStates.CLOSED);
            State.DataState(DataStates.SUSPECT);

            if (StreamId != 0)
            {
                InUse = false;
            }

            CloseAllOutStandingPost();
        }

        /// <summary>
        /// User request list associated with this stream used for fanout
        /// </summary>
        public WlRequestList UserRequestDlList { get; private set; } = new WlRequestList();

        /// <summary>
        /// Default constructor
        /// </summary>
        public WlStream()
        {
            State.Text().Data(m_stateTextBuffer);
            m_userReqListHandle = GCHandle.Alloc(UserRequestDlList, GCHandleType.Normal);
            m_reqMsgHandle = GCHandle.Alloc(RequestMsg, GCHandleType.Normal);

            m_requestTimeoutAction = this.OnRequestTimeout;
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void ClearWlStream()
        {
            RequestPending = false;
            WlHandler = null;
            Watchlist = null;
            if (UserRequestDlList.Count() > 0) { while (UserRequestDlList.Pop() != null) { } }
            m_RequestTimeoutTimer?.ReturnToPool();
            m_RequestTimeoutTimer = null;
            m_stateTextBuffer.Clear();
            State.Clear();
            State.Text().Data(m_stateTextBuffer);
            // outstanding post messages
            if (m_PostTimeoutInfoPool.Count > 0)
            {
                foreach (var timeoutInfo in m_PostTimeoutInfoPool)
                {
                    timeoutInfo.Clear();
                }
                m_PostTimeoutInfoPool.Clear();
            }
            
            if (m_PostIdToMsgTable.Values.Count > 0)
            {
                foreach (var timeoutInfo in m_PostIdToMsgTable.Values)
                {
                    timeoutInfo.Clear();
                }
                m_PostIdToMsgTable.Clear();
            }
        }

        public override void ReturnToPool()
        {
            ClearWlStream();
            base.ReturnToPool();
        }

        /// <summary>
        /// Encodes a ETA message into buffer and writes to channel.
        /// </summary>
        /// <param name="msg">the Codec message to encode</param>
        /// <param name="submitOptions"><see cref="ReactorSubmitOptions"/> instance</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that carries
        /// error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        internal ReactorReturnCode EncodeIntoBufferAndWrite(IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode;
            CodecReturnCode codecReturnCode;

            // lazily initialize channel info to get maxFragmentSize
            if (Watchlist!.ReactorChannelInfo!.ChannelInfo.MaxFragmentSize == 0)
            {
                if ((reactorReturnCode = Watchlist.ReactorChannel!.Info(Watchlist.ReactorChannelInfo, out errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    if (reactorReturnCode == ReactorReturnCode.FAILURE && Watchlist.ReactorChannel!.Channel!.State != Transports.ChannelState.ACTIVE)
                        return ReactorReturnCode.SUCCESS;
                    else
                        return reactorReturnCode;
                }
            }

            int bufferSize = Watchlist!.ReactorChannelInfo.ChannelInfo.MaxFragmentSize;
            ITransportBuffer? buffer = Watchlist.ReactorChannel!.GetBuffer(bufferSize, false, out errorInfo);
            
            if (buffer != null)
            {
                var encodeIterator = Watchlist!.m_wlStreamEncodeIterator;
                do
                {
                    encodeIterator!.Clear();
                    encodeIterator.SetBufferAndRWFVersion(buffer, Watchlist!.ReactorChannel!.MajorVersion, Watchlist!.ReactorChannel!.MinorVersion);

                    codecReturnCode = msg.Encode(encodeIterator);

                    if(codecReturnCode == CodecReturnCode.BUFFER_TOO_SMALL)
                    {
                        /* Release the old buffer */
                        if (Watchlist.ReactorChannel!.ReleaseBuffer(buffer, out errorInfo) < ReactorReturnCode.SUCCESS)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                               ReactorReturnCode.FAILURE,
                               "WlStream.EncodeIntoBufferAndWrite",
                               $"Failed to release buffer with error text: {errorInfo?.Error.Text}");
                        }

                        bufferSize *= 2;

                        /* Get a new buffer with the increased buffer size */
                        buffer = Watchlist.ReactorChannel!.GetBuffer(bufferSize, false, out errorInfo);

                        if(buffer is null)
                        {
                            return Reactor.PopulateErrorInfo(out errorInfo,
                                                 ReactorReturnCode.NO_BUFFERS,
                                                 "WlStream.EncodeIntoBufferAndWrite",
                                                 $"Channel out of buffers, errorId = {errorInfo!.Error.ErrorId}, errorText: {errorInfo!.Error.Text}");
                        }
                    }

                } while (codecReturnCode == CodecReturnCode.BUFFER_TOO_SMALL);

                /* Checks if there are other failures apart from the CodecReturnCode.BUFFER_TOO_SMALL */
                if (codecReturnCode < CodecReturnCode.SUCCESS)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "WlStream.EncodeIntoBufferAndWrite",
                        $"Failed encoding message, return code: {codecReturnCode.GetAsString()}");
                }

                reactorReturnCode = Watchlist.Reactor!.SubmitChannel(Watchlist.ReactorChannel, buffer, submitOptions, out errorInfo);
                switch (reactorReturnCode)
                {
                    case ReactorReturnCode.SUCCESS:
                        // don't release buffer here
                        break;
                    case ReactorReturnCode.WRITE_CALL_AGAIN:
                        // call submit again until it passes
                        while (reactorReturnCode == ReactorReturnCode.WRITE_CALL_AGAIN)
                        {
                            reactorReturnCode = Watchlist.Reactor.SubmitChannel(Watchlist.ReactorChannel, buffer, submitOptions, out errorInfo);
                        }
                        break;
                    default:
                        Watchlist.ReactorChannel.ReleaseBuffer(buffer, out errorInfo);
                        break;
                }
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo,
                                                  ReactorReturnCode.NO_BUFFERS,
                                                  "WlStream.EncodeIntoBufferAndWrite",
                                                  $"Channel out of buffers, errorId = {errorInfo?.Error.ErrorId}, errorText: {errorInfo?.Error.Text}");
            }

            return reactorReturnCode;
        }

        /// <summary>
        /// Sends close message for the stream.
        /// </summary>
        /// <param name="msg">the Close message</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that carries
        /// error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        internal ReactorReturnCode SendCloseMsg(IMsg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode;

            // if close message cannot be sent keep trying until close message is sent
            do
            {
                reactorReturnCode = EncodeIntoBufferAndWrite(msg, m_Watchlist!.m_streamSubmitOptions, out errorInfo);
                if (reactorReturnCode == ReactorReturnCode.NO_BUFFERS)
                {
                    // increase buffers to move things along
                    ReactorReturnCode reactorReturnCode2;
                    Watchlist!.ReactorChannelInfo!.Clear();
                    if ((reactorReturnCode2 = Watchlist!.ReactorChannel!.Info(Watchlist!.ReactorChannelInfo!, out errorInfo)) >= ReactorReturnCode.SUCCESS)
                    {
                        int newNumberOfBuffers = Watchlist!.ReactorChannelInfo!.ChannelInfo.GuaranteedOutputBuffers + 2;
                        if ((reactorReturnCode2 = Watchlist!.ReactorChannel!.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, newNumberOfBuffers, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            return reactorReturnCode2;
                        }
                    }
                    else
                        return reactorReturnCode2;
                }
            } while (reactorReturnCode == ReactorReturnCode.NO_BUFFERS);

            return reactorReturnCode;
        }

        ///<summary>
        /// Wrapper for sendMsg, used when just sending a message. Can return SUCCESS, or FAILURE
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="submitOptions"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode SendMsgOutOfLoop(IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret = SendMessage(msg, submitOptions, out errorInfo);

            if (ret == ReactorReturnCode.NO_BUFFERS)
                return ReactorReturnCode.SUCCESS;
            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode SendMessage(IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            if (Watchlist!.IsChannelUp())
            {
                if (msg.MsgClass != MsgClasses.CLOSE)
                {
                    ReactorReturnCode reactorReturnCode = EncodeIntoBufferAndWrite(msg, submitOptions, out errorInfo);
                    if (reactorReturnCode >= ReactorReturnCode.SUCCESS)
                    {
                        if (msg.MsgClass == MsgClasses.REQUEST)
                        {
                            if (msg != RequestMsg)
                            {
                                CodecReturnCode codecReturnCode;
                                if ((codecReturnCode = msg.Copy(RequestMsg, CopyMsgFlags.ALL_FLAGS)) < CodecReturnCode.SUCCESS)
                                {
                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                        ReactorReturnCode.FAILURE,
                                        "WlStream.SendMessage",
                                        $"Failed to copy request message, return code: {codecReturnCode.GetAsString()}");
                                }
                            }

                            if (!RequestPending && !((IRequestMsg)msg).CheckNoRefresh())
                            {
                                if (StartRequestTimer() != ReactorReturnCode.SUCCESS)
                                {
                                    return ReactorReturnCode.FAILURE;
                                }
                            }
                        }
                    }
                    else if (reactorReturnCode == ReactorReturnCode.NO_BUFFERS && msg.MsgClass != MsgClasses.POST)
                    {
                        if (msg.MsgClass == MsgClasses.REQUEST)
                        {
                            if (msg != RequestMsg)
                            {
                                CodecReturnCode codecReturnCode;
                                if ((codecReturnCode = msg.Copy(RequestMsg, CopyMsgFlags.ALL_FLAGS)) < CodecReturnCode.SUCCESS)
                                {
                                    return Reactor.PopulateErrorInfo(out errorInfo,
                                        ReactorReturnCode.FAILURE,
                                        "WlStream.SendMessage",
                                        $"Failed to copy request message, return code: {codecReturnCode.GetAsString()}");
                                }
                            }
                            WlHandler!.AddPendingRequest(this);
                        }
                        return reactorReturnCode;
                    }
                    else return reactorReturnCode;
                }
                else
                {
                    return SendCloseMsg(msg, out errorInfo);
                }
            }
            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /* Sends a NAK message to the application. */
        ReactorReturnCode SendNak(IPostMsg postMsg, out ReactorErrorInfo? errorInfo)
        {
            var AckMsg = Watchlist!.AckMsg;
            AckMsg.Clear();
            AckMsg.MsgClass = MsgClasses.ACK;
            AckMsg.StreamId = postMsg.StreamId;
            AckMsg.DomainType = postMsg.DomainType;
            AckMsg.ContainerType = DataTypes.NO_DATA;
            AckMsg.Flags = AckMsgFlags.NONE;
            AckMsg.ApplyHasNakCode();
            AckMsg.NakCode = NakCodes.NO_RESPONSE;
            AckMsg.ApplyHasText();
            AckMsg.Text.Data("No Ack received for PostMsg with postId = " + postMsg.PostId);
            AckMsg.AckId = postMsg.PostId;
            AckMsg.SeqNum = postMsg.SeqNum;

            if (postMsg.CheckHasMsgKey())
            {
                if (postMsg.MsgKey.CheckHasName())
                {
                    AckMsg.ApplyHasMsgKey();
                    AckMsg.MsgKey.ApplyHasName();
                    AckMsg.MsgKey.Name = postMsg.MsgKey.Name;
                }

                if (postMsg.MsgKey.CheckHasServiceId())
                {
                    AckMsg.ApplyHasMsgKey();
                    AckMsg.MsgKey.ApplyHasServiceId();
                    AckMsg.MsgKey.ServiceId = postMsg.MsgKey.ServiceId;
                }
            }

            if (postMsg.CheckHasSeqNum())
                AckMsg.ApplyHasSeqNum();

            // call back item handler with NAK message
            return Watchlist!.ItemHandler.CallbackUserWithMsg("WlStream.SendNak", AckMsg, Watchlist!.StreamIdToWlRequestDict![AckMsg.StreamId], out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode StartRequestTimer()
        {
            RequestPending = true;
            m_RequestTimeoutTimer?.Stop();
            m_RequestTimeoutTimer?.Start();
            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal virtual void ChannelDown()
        {
            RequestPending = false;
            m_RequestTimeoutTimer?.Stop();
        }

        /// <summary>
        /// Response received for this stream.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ResponseReceived()
        {
            RequestPending = false;
            m_RequestTimeoutTimer?.Stop();
        }

        /// <summary>
        /// Update the applicable post tables after successfully sending a post message.
        /// </summary>
        /// <param name="postMsg"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode UpdatePostTables(IPostMsg postMsg, out ReactorErrorInfo? errorInfo)
        {
            // insert PostMsg into postSeqNumToMsgTable by sequence number
            long seqNum = postMsg.CheckHasSeqNum() ? postMsg.SeqNum : 0;

            var postKey = Tuple.Create(postMsg.PostId, seqNum);
            if (m_PostIdToMsgTable.ContainsKey(postKey)) // timer for this key is already added
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }

            WlPostTimeoutInfo postTimeoutInfo = PostTimeoutInfoPoolPoll(postMsg);

            m_PostIdToMsgTable[postKey] = postTimeoutInfo;

            // always start post ACK timer even for multi-part since each part gets ACK
            /* this has the dual purpose of aging out entries in the _postIdToMsgTable
               and NAKing when there's no response to the post message */
            postTimeoutInfo.Timer?.Start();

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Validates a post message submit.
        /// </summary>
        /// <param name="postMsg"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode ValidatePostSubmit(IPostMsg postMsg, out ReactorErrorInfo? errorInfo)
        {
            if (StreamDomainType != (int)DomainType.LOGIN || postMsg.CheckHasMsgKey())
            {
                // make sure post message domain isn't an administrative domain
                if (postMsg.DomainType == (int)DomainType.LOGIN
                    || postMsg.DomainType == (int)DomainType.SOURCE
                    || postMsg.DomainType == (int)DomainType.DICTIONARY)
                {
                    // cannot submit post message with login domain type
                    return Reactor.PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.INVALID_USAGE,
                        "WlStream.SubmitMsg",
                        "Cannot submit PostMsg with administrative domain type.");
                }

                // check post id and sequence number if ACK required
                if (postMsg.CheckAck())
                {
                    // make sure post message has post id if ACK required
                    if (!postMsg.CheckHasPostId())
                    {
                        // cannot submit post requiring ack with no post id
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlStream.SubmitMsg",
                            "Cannot submit PostMsg requiring ack with no post id.");
                    }

                    // make sure multi-part post message has sequence number
                    if (!postMsg.CheckPostComplete() && !postMsg.CheckHasSeqNum())
                    {
                        // cannot submit multi-part post message with no sequence number
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlStream.SubmitMsg",
                            "Cannot submit multi-part PostMsg with no sequence number.");
                    }

                    // check for multi-part post and return error if sequence number invalid
                    if (postMsg.CheckHasSeqNum()
                        && m_PostIdToMsgTable.ContainsKey(Tuple.Create(postMsg.SeqNum, postMsg.SeqNum)))
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlStream.HandlePostSubmit",
                            $"Cannot submit multi-part PostMsg with duplicate sequence number {postMsg.SeqNum}, postId = {postMsg.PostId}");
                    }
                    else if (!postMsg.CheckHasSeqNum()
                            && m_PostIdToMsgTable.ContainsKey(Tuple.Create(postMsg.SeqNum, 0L)))
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.INVALID_USAGE,
                            "WlStream.HandlePostSubmit",
                            $"Cannot submit PostMsg with duplicate postId of {postMsg.PostId}");
                    }
                }
            }
            else
            {
                // cannot submit post with no MsgKey
                return Reactor.PopulateErrorInfo(out errorInfo,
                    ReactorReturnCode.INVALID_USAGE,
                    "WlStream.SubmitMsg",
                    "Cannot submit PostMsg with no MsgKey.");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Handles a post ACK message.
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool HandlePostAck(IMsg msg)
        {
            // return false if post id was already removed from table, true if still in table
            bool ret = false;

            // retrieve postSeqNumToMsgTable from postIdToMsgTable by ACK id which is the same as post id
            long seqNum = ((IAckMsg)msg).CheckHasSeqNum() ? ((IAckMsg)msg).SeqNum : 0;
            Tuple<long, long> postKey = Tuple.Create(((IAckMsg)msg).AckId, seqNum);
            if (m_PostIdToMsgTable.TryGetValue(postKey, out var timeoutInfo))
            {
                // decrement number of outstanding post messages
                Watchlist!.NumOutstandingPosts -= 1;

                // remove PostMsg from postSeqNumToMsgTable and add back to pool

                // set message's stream id to that of post message for fanout
                msg.StreamId = timeoutInfo.PostMsg.StreamId;

                timeoutInfo.Timer?.Stop();

                // remove PostMsg from postSeqNumToMsgTable and add back to pool
                PostTimeoutInfoReturnToPool(timeoutInfo);
                m_PostIdToMsgTable.Remove(postKey);

                ret = true;
            }
            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void OnRequestTimeout(WlTimeoutTimer timer)
        {
            // if stream is CLOSED then do nothing
            if (State.StreamState() == StreamStates.CLOSED)
            {
                return;
            }

            // handle request timeout
            // if request pending, resend request
            if (RequestPending)
            {
                // encode into buffer and send out
                if (Watchlist?.IsChannelUp() == true)
                {
                    // send close message first
                    var CloseMsg = Watchlist.CloseMsg;
                    CloseMsg.MsgClass = MsgClasses.CLOSE;
                    CloseMsg.StreamId = StreamId;
                    CloseMsg.DomainType = StreamDomainType;
                    if (SendCloseMsg(CloseMsg, out _) >= ReactorReturnCode.SUCCESS)
                    {
                        // reset request pending flag
                        RequestPending = false;

                        // notify handler of request timeout
                        WlHandler!.OnRequestTimeout(this);
                    }
                }
            }
        }

        /// <summary>
        /// Callback invoked to handle post messages that did not receive ACK on time.
        /// </summary>
        /// <param name="timer">the timer that timed out, the
        ///   <see cref="WlTimeoutTimer.UserSpecObject"/> is used to find out the particular
        ///   post message that timed out</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void OnPostTimeout(WlTimeoutTimer timer)
        {
            IPostMsg? postMsg = timer.UserSpecObject as IPostMsg;
            if (postMsg != null)
            {
                // PostMsg hasn't been ACKed, send NAK to user if ACK requested
                if (postMsg.CheckAck())
                {
                    SendNak(postMsg, out _);
                }

                long seqNum = postMsg.CheckHasSeqNum() ? postMsg.SeqNum : 0;
                var postKey = Tuple.Create(postMsg.PostId, seqNum);

                if (m_PostIdToMsgTable.TryGetValue(postKey, out var timeoutInfo))
                {
                    // timed out post message is no longer outstanding
                    Watchlist!.NumOutstandingPosts -= 1;

                    // remove PostMsg from postSeqNumToMsgTable and add back to pool
                    PostTimeoutInfoReturnToPool(timeoutInfo);
                    m_PostIdToMsgTable.Remove(postKey);
                }
            }

            return;
        }

        /// <summary>
        /// Either create a new or poll a cached post timeout info instance from the pool.
        /// </summary>
        /// <param name="postMsg">The Post message that will be associated with this timeout info</param>
        /// <returns>a post timeout info instance</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private WlPostTimeoutInfo PostTimeoutInfoPoolPoll(IPostMsg postMsg)
        {
            WlPostTimeoutInfo timeoutInfo;
            if (m_PostTimeoutInfoPool.Count > 0)
            {
                timeoutInfo = m_PostTimeoutInfoPool.First!.Value;
                timeoutInfo.Clear();
                m_PostTimeoutInfoPool.RemoveFirst();
            }
            else
            {
                timeoutInfo = new WlPostTimeoutInfo();
            }
            timeoutInfo.PostMsg = postMsg;
            if (m_Watchlist?.m_PostAckTimeoutGroup != null)
            {
                timeoutInfo.Timer = m_Watchlist!.Reactor!.GetReactorPool().CreateWlTimeoutTimer(m_Watchlist.m_PostAckTimeoutGroup,
                    this.OnPostTimeout);

                timeoutInfo.Timer.UserSpecObject = postMsg;
            }
            return timeoutInfo;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void PostTimeoutInfoReturnToPool(WlPostTimeoutInfo timeoutInfo)
        {
            m_PostTimeoutInfoPool.AddLast(timeoutInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void CloseAllOutStandingPost()
        {
            foreach (var postTimeoutInfo in m_PostIdToMsgTable.Values)
            {
                if (postTimeoutInfo != null && postTimeoutInfo.Timer != null)
                {
                    /* Removes the number of outstanding posts */
                    Watchlist!.NumOutstandingPosts -= 1;

                    postTimeoutInfo.Timer.Stop();
                    PostTimeoutInfoReturnToPool(postTimeoutInfo);
                }
            }

            m_PostIdToMsgTable.Clear();
        }

        internal void FreeWlStream()
        {
            m_Watchlist = null;
            m_reqMsgHandle.Free();
            m_userReqListHandle.Free();
            m_streamHandle.Free();
        }
    }
}
