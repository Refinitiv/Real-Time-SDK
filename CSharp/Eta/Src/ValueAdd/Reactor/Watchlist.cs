/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using Buffer = LSEG.Eta.Codec.Buffer;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class Watchlist : VaNode
    {
        internal const int DEFAULT_INIT_WATCHLIST_ITEM_POOLS = 1000;

        #region Watchlist pools

        private VaLimitedPool m_WlRequestPool = new VaLimitedPool(false);
        private VaLimitedPool m_WlItemRequestPool = new VaLimitedPool(false);
        private VaLimitedPool m_WlStreamAttributesPool = new VaLimitedPool(false);
        private VaLimitedPool m_WlStreamPool = new VaLimitedPool(false);

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void InitWlRequestPool<T>(int size, VaLimitedPool pool) where T : WlRequest, new()
        {
            for (int i = 0; i < size; i++)
            {
                T item = new T();
                item.m_handle = GCHandle.Alloc(item, GCHandleType.Normal);
                pool.Add(item);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void InitWlStreamPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                WlStream item = new WlStream();
                item.m_streamHandle = GCHandle.Alloc(item, GCHandleType.Normal);
                m_WlStreamPool.Add(item);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public T CreatePooledRequest<T>(VaLimitedPool pool) where T : WlRequest, new()
        {
            T? itemRequest = (T?)pool.Poll();
            if (itemRequest == null)
            {
                itemRequest = new T();
                itemRequest.m_handle = GCHandle.Alloc(itemRequest, GCHandleType.Normal);
                pool.UpdatePool(itemRequest);
            }

            return itemRequest;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void GrowWlRequestPool(int size)
        {
            InitWlRequestPool<WlRequest>(size, m_WlRequestPool);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlRequest CreateWlRequest()
        {
            return CreatePooledRequest<WlRequest>(m_WlRequestPool);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void GrowWlItemRequestPool(int size)
        {
            InitWlRequestPool<WlItemRequest>(size, m_WlItemRequestPool);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlItemRequest CreateWlItemRequest()
        {
            return CreatePooledRequest<WlItemRequest>(m_WlItemRequestPool);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void GrowWlStreamPool(int size)
        {
            InitWlStreamPool(size);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlStream CreateWlStream()
        {
            WlStream? wlStream = (WlStream?)m_WlStreamPool.Poll();
            if (wlStream == null)
            {
                wlStream = new WlStream();
                wlStream.m_streamHandle = GCHandle.Alloc(wlStream, GCHandleType.Normal);
                m_WlStreamPool.UpdatePool(wlStream);
            }

            return wlStream;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void GrowWlStreamAttributesPool(int size)
        {
            for (int i = 0; i < size; i++)
            {
                WlStreamAttributes wlStreamAttributes = new WlStreamAttributes();
                wlStreamAttributes.m_handle = GCHandle.Alloc(wlStreamAttributes, GCHandleType.Normal);
                m_WlStreamAttributesPool.Add(wlStreamAttributes);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public WlStreamAttributes CreateWlStreamAttributes()
        {
            WlStreamAttributes? wlStreamAttributes = (WlStreamAttributes?)m_WlStreamAttributesPool.Poll();
            if (wlStreamAttributes == null)
            {
                wlStreamAttributes = new WlStreamAttributes();
                wlStreamAttributes.m_handle = GCHandle.Alloc(wlStreamAttributes, GCHandleType.Normal);
                m_WlStreamAttributesPool.UpdatePool(wlStreamAttributes);
            }

            return wlStreamAttributes;
        }

        public void GrowWatchlistObjectPools(int diff)
        {
            GrowWlItemRequestPool(diff);
            GrowWlStreamAttributesPool(diff);
        }

        #endregion

        private EncodeIterator m_EncodeIt = new EncodeIterator();
        private DecodeIterator m_DecodeIt = new DecodeIterator();
        private Msg m_Msg = new Msg();
        private Buffer m_TempBuffer = new Buffer();
        private ByteBuffer m_TempByteBuffer = new ByteBuffer(8192);
        private Buffer m_TempBuffer2 = new Buffer();
        private ByteBuffer m_TempByteBuffer2 = new ByteBuffer(8192);
        
        internal ByteBuffer m_ViewByteBuffer = new ByteBuffer(2048);
        internal EncodeIterator m_wlStreamEncodeIterator = new EncodeIterator();
        internal ReactorSubmitOptions m_streamSubmitOptions = new ReactorSubmitOptions();

        // Watchlist handlers
        public IWlLoginHandler LoginHandler { get; internal set; }
        public IWlDirectoryHandler DirectoryHandler { get; internal set; }
        public IWlItemHandler ItemHandler { get; internal set; }

        internal ICloseMsg CloseMsg { get; private set; } = new Msg();

        internal IAckMsg AckMsg { get; private set; } = new Msg();

        public IDictionary<int, WlRequest>? StreamIdToWlRequestDict { get; private set; }

        internal ReactorChannel? ReactorChannel { get; private set; }
        internal ConsumerRole? ConsumerRole { get; private set; }

        internal ReactorChannelInfo? ReactorChannelInfo { get; set; }

        internal Reactor? Reactor { get; private set; }
        internal int NumOutstandingPosts { get; set; }

        internal WlTimeoutTimerGroup? m_RequestTimeoutGroup;
        internal WlTimeoutTimerGroup? m_PostAckTimeoutGroup;

        internal WlStreamManager StreamManager;

#pragma warning disable CS8618
        public Watchlist(ReactorChannel reactorChannel, ConsumerRole role)
        {
            Init(reactorChannel, role);

            GrowWlRequestPool(10);
            GrowWlStreamPool(10);
            GrowWlItemRequestPool(DEFAULT_INIT_WATCHLIST_ITEM_POOLS);
            GrowWlStreamAttributesPool(DEFAULT_INIT_WATCHLIST_ITEM_POOLS);

            LoginHandler = new WlLoginHandler(this);
            DirectoryHandler = new WlDirectoryHandler(this);
            ItemHandler = new WlItemHandler(this);
        }

        public Watchlist()
        {
            GrowWlRequestPool(10);
            GrowWlStreamPool(10);
            GrowWlItemRequestPool(DEFAULT_INIT_WATCHLIST_ITEM_POOLS);
            GrowWlStreamAttributesPool(DEFAULT_INIT_WATCHLIST_ITEM_POOLS);

            LoginHandler = new WlLoginHandler(this);
            DirectoryHandler = new WlDirectoryHandler(this);
            ItemHandler = new WlItemHandler(this);
        }
#pragma warning restore CS8618

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Init(ReactorChannel reactorChannel, ConsumerRole role)
        {
            ReactorChannel = reactorChannel;
            ConsumerRole = role;
            Reactor = reactorChannel.Reactor;
            ReactorChannelInfo = new ReactorChannelInfo();

            ItemHandler.Init(role);

            if (ConsumerRole.WatchlistOptions.ItemCountHint != 0)
            {
                int countHint = ConsumerRole.WatchlistOptions.ItemCountHint >= int.MaxValue ?
                    int.MaxValue : (int)ConsumerRole.WatchlistOptions.ItemCountHint;

                StreamManager = new WlStreamManager(countHint);
                StreamIdToWlRequestDict = StreamManager.WlRequestsByIds;
            }
            else
            {
                StreamManager = new WlStreamManager(1000);
                StreamIdToWlRequestDict = StreamManager.WlRequestsByIds;
            }

            if (ConsumerRole.WatchlistOptions.RequestTimeout > 0)
            {
                m_RequestTimeoutGroup = Reactor!.m_TimeoutTimerManager.CreateTimerGroup(ConsumerRole.WatchlistOptions.RequestTimeout);
            }
            if (ConsumerRole.WatchlistOptions.PostAckTimeout > 0)
            {
                m_PostAckTimeoutGroup = Reactor!.m_TimeoutTimerManager.CreateTimerGroup(ConsumerRole.WatchlistOptions.PostAckTimeout);
            }

            DirectoryHandler.Init();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode SubmitMsg(Msg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            StreamIdToWlRequestDict!.TryGetValue(msg.StreamId, out WlRequest? wlRequest);
            bool isReissue = false;

            if (msg.MsgClass == MsgClasses.REQUEST)
            {
                IRequestMsg requestMsg = msg;
                if (!string.IsNullOrEmpty(submitOptions.ServiceName) && requestMsg.MsgKey.CheckHasServiceId())
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                       "Watchlist.SubmitMsg", "Cannot submit request with both service name and service id specified.");
                }

                if (wlRequest == null)
                {
                    if (requestMsg.CheckNoRefresh())
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                            "Watchlist.SubmitMsg", "Cannot submit new item request to watchlist with NO_REFRESH flag set.");
                    }

                    if (msg.DomainType != (int)DomainType.LOGIN &&  msg.DomainType != (int)DomainType.SOURCE)
                    {
                        wlRequest = CreateWlItemRequest();
                        ret = ItemHandler!.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                    }
                    else
                    {
                        wlRequest = CreateWlRequest();
                        if (msg.DomainType == (int)DomainType.LOGIN)
                        {
                            ret = LoginHandler.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                        }
                        else
                        {
                            ret = DirectoryHandler.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                        }
                    }
                }
                else // stream already exists (this is a reissue)
                {
                    if ((ret = ValidateReissue(wlRequest, requestMsg, submitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    isReissue = true;

                    if (msg.DomainType != (int)DomainType.LOGIN && msg.DomainType != (int)DomainType.SOURCE)
                    {
                        ret = ItemHandler.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                    }
                    else
                    {
                        if (msg.DomainType == (int)DomainType.LOGIN)
                        {
                            ret = LoginHandler.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                        }
                        else
                        {
                            ret = DirectoryHandler.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                        }
                    }
                }

                
                if (ret >= ReactorReturnCode.SUCCESS)
                {
                    // save submitted request message

                    // Don't copy the entire payload for the batch request as the items has been requested individually.
                    if (requestMsg.CheckHasBatch())
                    {
                        requestMsg.Copy(wlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS & (~CopyMsgFlags.DATA_BODY));
                    }
                    else
                    {
                        requestMsg.Copy(wlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS);
                    }

                    // add to watchlist request table if new request
                    if (!isReissue)
                    {
                        wlRequest.UserStreamId = requestMsg.StreamId;
                        StreamIdToWlRequestDict[requestMsg.StreamId] = wlRequest;
                    }
                }
                else // Submit failed
                {
                    if (!isReissue)
                    {
                        wlRequest.ReturnToPool();
                    }
                }

                return ret;
            }
            else // not a request message
            {
                // submit to appropriate handler if request stream already open, otherwise this is an error
                if (wlRequest != null)
                {
                    switch (wlRequest.RequestMsg.DomainType)
                    {
                        case (int)DomainType.LOGIN:
                            {
                                return LoginHandler!.SubmitMsg(wlRequest, msg, submitOptions, out errorInfo);
                            }
                        case (int)DomainType.SOURCE:
                            {
                                return DirectoryHandler!.SubmitMsg(wlRequest, msg, submitOptions, out errorInfo);
                            }
                        default: // all other domain types (including dictionary) handled by item handler
                            {
                                return ItemHandler!.SubmitMsg(wlRequest, msg, submitOptions, out errorInfo);
                            }
                    }
                }
                else
                {
                    if (msg.MsgClass != MsgClasses.CLOSE)
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                            "Watchlist.SubmitMsg", $"Cannot submit message class {MsgClasses.ToString(msg.MsgClass)} " +
                            $"to watchlist before stream is opened with a REQUEST.");
                    }
                    else
                    {
                        return ReactorReturnCode.SUCCESS;
                    }
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode SubmitMsg(MsgBase rdmMsg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            if (rdmMsg.DomainType != (int)DomainType.LOGIN &&
                rdmMsg.DomainType != (int)DomainType.SOURCE &&
                rdmMsg.DomainType != (int)DomainType.DICTIONARY)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                            "Watchlist.SubmitMsg", $"Cannot submit domain type {DomainTypes.ToString(rdmMsg.DomainType)} " +
                            $"to watchlist as RDM message.");
            }

            // Converts RDM to Codec message
            m_Msg.Clear();
            CodecReturnCode codeRet;
            if( (codeRet = ConvertRDMToCodecMsg(rdmMsg, m_Msg)) != CodecReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                            "Watchlist.SubmitMsg", $"Failed to convert RDM to Codec message: {CodecReturnCodeExtensions.GetAsInfo(codeRet)}");
            }

            if((rdmMsg.DomainType == (int)DomainType.DICTIONARY) && (rdmMsg.MsgClass == MsgClasses.REQUEST))
            {
                // DictionaryRequest.ServiceId is not optional. If a service name was provided, let that take precedence over
                // ServiceId -- remove the ServiceId from this message so that we don't get an error for having both.
                m_Msg.MsgKey.Flags = (m_Msg.MsgKey.Flags & ~MsgKeyFlags.HAS_SERVICE_ID);
            }

            return SubmitMsg(m_Msg, submitOptions, out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        ReactorReturnCode ValidateReissue(WlRequest wlRequest, IRequestMsg requestMsg, ReactorSubmitOptions submitOptions,
            out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;

            if(requestMsg.MsgKey.CheckHasServiceId())
            {
                // reissue request has service Id
                if(!wlRequest.RequestMsg.MsgKey.CheckHasServiceId() ||
                    requestMsg.MsgKey.ServiceId != wlRequest.RequestMsg.MsgKey.ServiceId)
                {
                    ret = ReactorReturnCode.INVALID_USAGE;
                }
            }
            else // reissue request doesn't have service Id
            {
                if(wlRequest.RequestMsg.MsgKey.CheckHasServiceId())
                {
                    ret = ReactorReturnCode.INVALID_USAGE;
                }
                else if(submitOptions.ServiceName is null)
                {
                    if(wlRequest.WatchlistStreamInfo.ServiceName is not null)
                    {
                        ret = ReactorReturnCode.INVALID_USAGE;
                    }
                }
                else
                {
                    if (wlRequest.RequestMsg.DomainType != (int)DomainType.LOGIN && (wlRequest.WatchlistStreamInfo.ServiceName is null ||
                        !submitOptions.ServiceName!.Equals(wlRequest.WatchlistStreamInfo.ServiceName)))
                    {
                        ret = ReactorReturnCode.INVALID_USAGE;
                    }
                }
            }

            if(ret < ReactorReturnCode.SUCCESS)
            {
                ret = Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                    "Watchlist.ValidateReissue", "Cannot change ServiceId or ServiceName on reissue.");
            }

            return ret;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode ConvertRDMToCodecMsg(MsgBase rdmMsg, Msg msg)
        {
            // Clears temp buffer
            m_TempBuffer.Clear();
            m_TempByteBuffer.Clear();
            m_TempBuffer.Data(m_TempByteBuffer);

            // Encodes RDM message to buffer
            m_EncodeIt.Clear();
            m_EncodeIt.SetBufferAndRWFVersion(m_TempBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            CodecReturnCode codecRet = rdmMsg.Encode(m_EncodeIt);

            while( codecRet == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_TempByteBuffer = new ByteBuffer(m_TempByteBuffer.Capacity * 2);
                m_TempBuffer.Clear();
                m_TempBuffer.Data(m_TempByteBuffer);
                m_EncodeIt.Clear();
                m_EncodeIt.SetBufferAndRWFVersion(m_TempBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                codecRet = rdmMsg.Encode(m_EncodeIt);
            }

            if(codecRet >= CodecReturnCode.SUCCESS)
            {
                // Decodes encoded RDM message into Codec message
                m_DecodeIt.Clear();
                m_DecodeIt.SetBufferAndRWFVersion(m_TempBuffer, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                codecRet = msg.Decode(m_DecodeIt);
            }

            return codecRet;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal CodecReturnCode ConvertCodecMsgToRDM(Msg msg, MsgBase rdmMsg)
        {
            CodecReturnCode codecRet = CodecReturnCode.SUCCESS;

            // Clears temp buffer
            m_TempBuffer2.Clear();
            m_TempByteBuffer2.Clear();
            m_TempBuffer2.Data(m_TempByteBuffer2);

            // Encodes Codec message into buffer
            m_EncodeIt.Clear();
            m_EncodeIt.SetBufferAndRWFVersion(m_TempBuffer2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

            codecRet = msg.Encode(m_EncodeIt);

            while(codecRet == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_TempByteBuffer2 = new ByteBuffer(m_TempByteBuffer2.Capacity * 2);
                m_TempBuffer2.Clear();
                m_TempBuffer2.Data(m_TempByteBuffer2);
                m_EncodeIt.Clear();
                m_EncodeIt.SetBufferAndRWFVersion(m_TempBuffer2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                codecRet = msg.Encode(m_EncodeIt);
            }

            if (codecRet >= CodecReturnCode.SUCCESS)
            {
                // Decodes encoded Codec message into RDM message
                m_DecodeIt.Clear();
                m_DecodeIt.SetBufferAndRWFVersion(m_TempBuffer2, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());
                m_Msg.Clear();
                m_Msg.Decode(m_DecodeIt);
                codecRet = rdmMsg.Decode(m_DecodeIt, m_Msg);
            }

            return codecRet;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, Msg msg, out ReactorErrorInfo? errorInfo)
        {
            Debug.Assert(wlStream is not null);

            return wlStream.WlHandler!.ReadMsg(wlStream, decodeIt, msg, out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode Dispatch(out ReactorErrorInfo? errorInfo)
        {
            // dispatch all of the handlers
            ReactorReturnCode ret1 = LoginHandler!.Dispatch(out var loginErrorInfo);
            ReactorReturnCode ret2 = DirectoryHandler!.Dispatch(out var directoryErrorInfo);
            ReactorReturnCode ret3 = ItemHandler!.Dispatch(out var itemErrorInfo);

            if (ret1 < ReactorReturnCode.SUCCESS)
            {
                errorInfo = loginErrorInfo;
                return ret1;
            }
            else if (ret2 < ReactorReturnCode.SUCCESS)
            {
                errorInfo = directoryErrorInfo;
                return ret2;
            }
            else if (ret3 < ReactorReturnCode.SUCCESS)
            {
                errorInfo = itemErrorInfo;
                return ret3;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode ChangeServiceNameToID(IMsgKey msgKey, string serviceName, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            if(msgKey.CheckHasServiceId())
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                    "Watchlist.ChangeServiceNameToID", "Message submitted with both service name and service ID.");
            }

            int serviceId = DirectoryHandler.ServiceId(serviceName);
            if(serviceId < 0)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                    "Watchlist.ChangeServiceNameToID", $"Message submitted with unknown service name {serviceName}.");
            }
            else
            {
                msgKey.ApplyHasServiceId();
                msgKey.ServiceId = serviceId;
            }

            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void CloseWlRequest(WlRequest wlRequest)
        {
            Debug.Assert(wlRequest.ReqState != WlRequest.State.RETURN_TO_POOL);

            if (StreamIdToWlRequestDict!.TryGetValue(wlRequest.RequestMsg.StreamId, out var result))
            {
                StreamIdToWlRequestDict!.Remove(result.RequestMsg.StreamId);
            }
        }

        /// <summary>
        /// Checks whether the channel is operational
        /// </summary>
        /// <returns><c>true</c> if the channeal is up otherwise <c>false</c></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool IsChannelUp()
        {
            return ReactorChannel != null && (ReactorChannel.State == ReactorChannelState.UP || ReactorChannel.State == ReactorChannelState.READY);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Close()
        {
            Clear();
            ReturnToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            ReactorChannel = null;
            ConsumerRole = null;
            Reactor = null;
            LoginHandler.Clear();
            DirectoryHandler.Clear();
            ItemHandler.Clear();
            m_EncodeIt.Clear();
            m_DecodeIt.Clear();
            m_Msg.Clear();
            StreamManager?.Clear();

            Reactor?.m_TimeoutTimerManager.ClearTimerGroup(m_RequestTimeoutGroup);
            Reactor?.m_TimeoutTimerManager.ClearTimerGroup(m_PostAckTimeoutGroup);
            m_RequestTimeoutGroup = null;
            m_PostAckTimeoutGroup = null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void ReturnToPool()
        {
            base.ReturnToPool();

            ReactorChannel = null;
            ConsumerRole = null;
            Reactor = null;
            StreamManager?.Clear();
        }

        /* Handles channel down event. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ChannelDown()
        {
            LoginHandler?.ChannelDown();
            DirectoryHandler?.ChannelDown(); // Delete all services (this will also trigger item status fanout)
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ChannelUp(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            LoginHandler?.ChannelUp(out errorInfo);
            DirectoryHandler?.ChannelUp(out errorInfo);
            ItemHandler?.ChannelUp(out errorInfo);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool TryGetActiveStream(int streamId,  out WlStream? stream)
        {
            stream = null;
            if (streamId > 0 && streamId < StreamManager.StreamsByStreamIds.Length + WlStreamManager.MIN_STREAM_ID)
            {
                if (streamId >= WlStreamManager.MIN_STREAM_ID)
                {
                    var tmpStream = StreamManager.StreamsByStreamIds[streamId - WlStreamManager.MIN_STREAM_ID];
                    if (tmpStream.InUse)
                    {
                        stream = tmpStream;
                        return true;
                    }
                    else
                        return false;
                }
                else
                {
                    switch (streamId)
                    {
                        case WlStreamManager.LOGIN_STREAM_ID:
                            if (StreamManager.LoginStream.InUse)
                            {
                                stream = StreamManager.LoginStream;
                                return true;
                            }
                            else
                                return false;
                        case WlStreamManager.DIRECTORY_STREAM_ID:
                            if (StreamManager.DirectoryStream.InUse)
                            {
                                stream = StreamManager.DirectoryStream;
                                return true;
                            }
                            else
                                return false;
                        default:
                            return false;
                    }
                }              
            }

            return false;
        }

        ~Watchlist()
        {
            if (StreamManager != null)
            {
                StreamManager.Free();
            }

            WlStream? stream;
            while ((stream = (WlStream?)m_WlStreamPool.Poll()) != null) stream.FreeWlStream();
            WlRequest? request;
            while ((request = (WlRequest?)m_WlRequestPool.Poll()) != null) request.FreeWlRequest();
            WlItemRequest? itemRequest;
            while ((itemRequest = (WlItemRequest?)m_WlItemRequestPool.Poll()) != null) itemRequest.FreeWlItemRequest();
            WlStreamAttributes? attributes;
            while ((attributes = (WlStreamAttributes?)m_WlStreamAttributesPool.Poll()) != null) attributes.m_handle.Free();
        }
    }
}
