/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Ema.Access
{
    internal class OmmEventImpl<T> : IOmmConsumerEvent, IOmmProviderEvent
    {
        internal long handle;
        internal object? closure;
        internal OmmConsumer? m_OmmConsumer;
        internal OmmProvider? m_OmmProvider;
        internal long clientHandle;

        internal Item<T>? Item { get; set; }

        internal ReactorChannel? ReactorChannel { get; set; }

        internal ChannelInformation? channelInformation;

        internal void SetHandle(long handle)
        {
            this.handle = handle;
        }

        internal void SetClosure(object? closure)
        {
            this.closure = closure;
        }

        internal void SetOmmConsumer(OmmConsumer consumer)
        {
            m_OmmConsumer = consumer;
        }

        internal void SetOmmProvider(OmmProvider provider)
        {
            m_OmmProvider = provider;
        }

        public OmmEventImpl()
        {
        }

        public long Handle
        {
            get
            {
                if (Item != null)
                    return Item.ItemId;
                else
                    return handle;
            }
        }

        public object? Closure
        {
            get
            {
                if (Item != null)
                    return Item.Closure;
                else
                    return closure;
            }
        }

        public long ClientHandle { get => clientHandle; }

        public OmmConsumer Consumer => m_OmmConsumer!;

        public OmmProvider Provider => m_OmmProvider!;

        public ChannelInformation ChannelInformation()
        {
            if (channelInformation == null)
            {
                channelInformation = new ChannelInformation();
            }
            else
            {
                channelInformation.Clear();
            }

            if (ReactorChannel != null)
            {
                channelInformation.Set(ReactorChannel);

                if (m_OmmProvider == null)
                {
                    channelInformation.IpAddress = "not available for OmmConsumer connections";
                    channelInformation.Port = ReactorChannel.Port;
                }
                else if (m_OmmProvider.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                {
                    channelInformation.IpAddress = "not available for OmmNiProvider connections";
                }
            }

            // for NiProviders, the only channel is the login channel so we'll just use
            // the channel from the LoginCallbackClient
            if(m_OmmProvider != null && m_OmmProvider.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
            {
                if (m_OmmProvider.m_OmmProviderImpl is OmmNiProviderImpl ommNiProviderImpl)
                {
                    ChannelInfo? channelInfo = ommNiProviderImpl.LoginCallbackClient!.ActiveChannelInfo();
                    if (channelInfo != null)
                    {
                        channelInformation.Set(channelInfo.ReactorChannel!);
                        channelInformation.IpAddress = "not available for OmmNiProvider connections";
                        return channelInformation;
                    }
                }
            }

            return channelInformation!;
        }
    }

    internal abstract class CallbackClient<T>
    {
        internal RefreshMsg? m_RefreshMsg;
        internal UpdateMsg? m_UpdateMsg;
        internal StatusMsg? m_StatusMsg;
        internal GenericMsg? m_GenericMsg;
        internal AckMsg? m_AckMsg;
        protected IOmmCommonImpl commonImpl;
        
        // ETA message class types
        protected IRefreshMsg? codecRefreshMsg;
        protected ICloseMsg? codecCloseMsg;
        protected IRequestMsg? codecRequestMsg;
        protected IStatusMsg? codecStatusMsg;

        internal OmmEventImpl<T> EventImpl { get; set;  }

        public CallbackClient(OmmBaseImpl<T> baseImpl, string clientName)
        {
            commonImpl = baseImpl;
            EventImpl = new OmmEventImpl<T>();
        }

        public CallbackClient(OmmServerBaseImpl baseImpl, string clientName)
        {
            commonImpl = baseImpl;
            EventImpl = new OmmEventImpl<T>();
        }

        public IRequestMsg RequestMsg()
        {
            if(codecRequestMsg == null)
            {
                codecRequestMsg = new Eta.Codec.Msg();
            }
            else
            {
                codecRequestMsg.Clear();
            }

            codecRequestMsg.MsgClass = MsgClasses.REQUEST;

            return codecRequestMsg;
        }

        public IRefreshMsg RefreshMsg()
        {
            if (codecRefreshMsg == null)
            {
                codecRefreshMsg = new Eta.Codec.Msg();
            }
            else
            {
                codecRefreshMsg.Clear();
            }

            codecRefreshMsg.MsgClass = MsgClasses.REFRESH;

            return codecRefreshMsg;
        }

        public IStatusMsg StatusMsg()
        {
            if (codecStatusMsg == null)
            {
                codecStatusMsg = new Eta.Codec.Msg();
            }
            else
            {
                codecStatusMsg.Clear();
            }

            codecStatusMsg.MsgClass = MsgClasses.STATUS;

            return codecStatusMsg;
        }

        public ICloseMsg CloseMsg()
        {
            if (codecCloseMsg == null)
            {
                codecCloseMsg = new Eta.Codec.Msg();
            }
            else
            {
                codecCloseMsg.Clear();
            }

            codecCloseMsg.MsgClass = MsgClasses.CLOSE;

            return codecCloseMsg;
        }

        public Action<Msg> NotifyOnAllMsg = msg => { };
        public Action NotifyOnRefreshMsg = () => { };
        public Action NotifyOnUpdateMsg = () => { };
        public Action NotifyOnStatusMsg = () => { };
        public Action NotifyOnGenericMsg = () => { };
        public Action NotifyOnAckMsg = () => { };
    }

    internal class LongIdGenerator
    {
        private const long MIN_LONG_VALUE = 1;
        private const long MAX_LONG_VALUE = long.MaxValue;

        private static long _longId = MIN_LONG_VALUE;

        private static MonitorWriteLocker LongIdLocker = new MonitorWriteLocker(new object());

        private static void Reset()
        {
            _longId = MIN_LONG_VALUE;
        }

        public static long NextLongId()
        {
            LongIdLocker.Enter();

            try
            {
                if (_longId == MAX_LONG_VALUE)
                    Reset();

                return _longId++;
            }
            finally
            {
                LongIdLocker.Exit();
            }
        }
    }
}
