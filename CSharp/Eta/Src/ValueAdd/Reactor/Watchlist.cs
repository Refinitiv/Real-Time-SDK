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

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal sealed class Watchlist : VaNode
    {
        EncodeIterator m_EncodeIt = new EncodeIterator();
        DecodeIterator m_DecodeIt = new DecodeIterator();
        Msg m_Msg = new Msg();
        Buffer m_TempBuffer = new Buffer();
        ByteBuffer m_TempByteBuffer = new ByteBuffer(8192);
        Buffer m_TempBuffer2 = new Buffer();
        ByteBuffer m_TempByteBuffer2 = new ByteBuffer(8192);

        // Watchlist handlers
        public IWlLoginHandler LoginHandler { get; internal set; }
        public IWlDirectoryHandler DirectoryHandler { get; internal set; }
        public IWlItemHandler ItemHandler { get; internal set; }

        /// <summary>
        /// This is used to generate consumer and provider stream IDs.
        /// </summary>
        internal WlStreamIdManager? WlStreamIdManager;

        public IDictionary<int, WlRequest>? StreamIdToWlRequestDict { get; private set; }
        public IDictionary<int, WlStream>? StreamIdToWlStreamDict { get; private set; }

        internal ReactorChannel? ReactorChannel { get; private set; }
        internal ConsumerRole? ConsumerRole { get; private set; }

        internal ReactorChannelInfo? ReactorChannelInfo { get; set; }

        internal Reactor? Reactor { get; private set; }
        internal int NumOutstandingPosts { get; set; }

        internal WlTimeoutTimerGroup? m_RequestTimeoutGroup;
        internal WlTimeoutTimerGroup? m_PostAckTimeoutGroup;


        public Watchlist(ReactorChannel reactorChannel, ConsumerRole role)
        {
            Init(reactorChannel, role);

            LoginHandler = new WlLoginHandler(this);
            DirectoryHandler = new WlDirectoryHandler(this);
            ItemHandler = new WlItemHandler(this);
        }

        public Watchlist()
        {
            LoginHandler = new WlLoginHandler(this);
            DirectoryHandler = new WlDirectoryHandler(this);
            ItemHandler = new WlItemHandler(this);
        }

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

                WlStreamIdManager = new WlStreamIdManager(countHint + 10);

                StreamIdToWlRequestDict = WlStreamIdManager.WlRequestsByIds;
                StreamIdToWlStreamDict = WlStreamIdManager.WlStreamsByIds;
            }
            else
            {
                WlStreamIdManager = new WlStreamIdManager(100);

                StreamIdToWlRequestDict = WlStreamIdManager.WlRequestsByIds;
                StreamIdToWlStreamDict = WlStreamIdManager.WlStreamsByIds;
            }

            if (ConsumerRole.WatchlistOptions.RequestTimeout > 0)
            {
                m_RequestTimeoutGroup =
                    Reactor!.m_TimeoutTimerManager.CreateTimerGroup(ConsumerRole.WatchlistOptions.RequestTimeout);
            }
            if (ConsumerRole.WatchlistOptions.PostAckTimeout > 0)
            {
                m_PostAckTimeoutGroup =
                    Reactor!.m_TimeoutTimerManager.CreateTimerGroup(ConsumerRole.WatchlistOptions.PostAckTimeout);
            }
        }

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

                if(wlRequest is null)
                {
                    if(requestMsg.CheckNoRefresh())
                    {
                        return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE,
                            "Watchlist.SubmitMsg", "Cannot submit new item request to watchlist with NO_REFRESH flag set.");
                    }

                    switch(msg.DomainType)
                    {
                        case (int)DomainType.LOGIN:
                        {
                            wlRequest = Reactor!.GetReactorPool().CreateWlRequest();
                            wlRequest.Handler = LoginHandler;
                            break;
                        }
                        case (int)DomainType.SOURCE:
                        {
                            wlRequest = Reactor!.GetReactorPool().CreateWlRequest();
                            wlRequest.Handler = DirectoryHandler;
                            break;
                        }
                        default: // all other domain types (including dictionary) handled by item handler
                        {
                            wlRequest = Reactor!.GetReactorPool().CreateWlItemRequest();
                            wlRequest.Handler = ItemHandler;
                            break;
                        }
                    }
                }
                else // stream already exists (this is a reissue)
                {
                    if((ret = ValidateReissue(wlRequest, requestMsg, submitOptions, out errorInfo)) < ReactorReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    isReissue = true;
                }

                ret = wlRequest!.Handler!.SubmitRequest(wlRequest, requestMsg, isReissue, submitOptions, out errorInfo);
                if(ret >= ReactorReturnCode.SUCCESS)
                {
                    // save submitted request message
                    wlRequest.RequestMsg.Clear();

                    // Don't copy the entire payload for the batch request as the items has been requested individually.
                    if(requestMsg.CheckHasBatch())
                    {
                        requestMsg.Copy(wlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS & (~CopyMsgFlags.DATA_BODY));
                    }
                    else
                    {
                        requestMsg.Copy(wlRequest.RequestMsg, CopyMsgFlags.ALL_FLAGS);
                    }

                    // add to watchlist request table if new request
                    if(!isReissue)
                    {
                        wlRequest.UserStreamId = requestMsg.StreamId;
                        StreamIdToWlRequestDict[requestMsg.StreamId] = wlRequest;
                    }
                }
                else // Submit failed
                {
                    if(!isReissue)
                    {
                        wlRequest.ReturnToPool();
                    }
                }

                return ret;
            }
            else // not a request message
            {
                // submit to appropriate handler if request stream already open, otherwise this is an error
                if(wlRequest is not null)
                {
                    return wlRequest!.Handler!.SubmitMsg(wlRequest, msg, submitOptions, out errorInfo);
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

        public ReactorReturnCode ReadMsg(WlStream wlStream, DecodeIterator decodeIt, Msg msg, out ReactorErrorInfo? errorInfo)
        {
            Debug.Assert(wlStream is not null);

            return wlStream.WlHandler!.ReadMsg(wlStream, decodeIt, msg, out errorInfo);
        }

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

        public void CloseWlRequest(WlRequest wlRequest)
        {
            Debug.Assert(wlRequest.ReqState != WlRequest.State.RETURN_TO_POOL);

            if (StreamIdToWlRequestDict!.TryGetValue(wlRequest.RequestMsg.StreamId, out var result))
            {
                result.ReturnToPool();
                StreamIdToWlRequestDict!.Remove(result.RequestMsg.StreamId);
            }
        }

        /// <summary>
        /// Checks whether the channel is operational
        /// </summary>
        /// <returns><c>true</c> if the channeal is up otherwise <c>false</c></returns>
        public bool IsChannelUp()
        {
            if (ReactorChannel != null)
            {
                if (ReactorChannel != null && (ReactorChannel.State == ReactorChannelState.UP
                    || ReactorChannel.State == ReactorChannelState.READY))
                {
                    return true;
                }
            }

            return false;
        }

        public void Close()
        {
            Clear();
            ReturnToPool();
        }

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
            WlStreamIdManager?.Clear();

            Reactor?.m_TimeoutTimerManager.ClearTimerGroup(m_RequestTimeoutGroup);
            Reactor?.m_TimeoutTimerManager.ClearTimerGroup(m_PostAckTimeoutGroup);
            m_RequestTimeoutGroup = null;
            m_PostAckTimeoutGroup = null;
        }

        public override void ReturnToPool()
        {
            base.ReturnToPool();

            ReactorChannel = null;
            ConsumerRole = null;
            Reactor = null;
            WlStreamIdManager?.Clear();
        }

        /* Handles channel down event. */
        internal void ChannelDown()
        {
            LoginHandler?.ChannelDown();
            DirectoryHandler?.ChannelDown(); // Delete all services (this will also trigger item status fanout)
        }

        internal void ChannelUp(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            LoginHandler?.ChannelUp(out errorInfo);
            DirectoryHandler?.ChannelUp(out errorInfo);
            ItemHandler?.ChannelUp(out errorInfo);
        }
    }
}
