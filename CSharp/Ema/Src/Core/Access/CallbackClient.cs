﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System.Threading;

namespace LSEG.Ema.Access
{
    internal class OmmEventImpl<T> : IOmmConsumerEvent
    {
        private long handle;
        private object? closure;
        private OmmConsumer? m_OmmConsumer;

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

        public OmmConsumer Consumer => m_OmmConsumer!;

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

            if (ReactorChannel != null) // TODO: add handling for OmmProvider and OmmNiProvider when available
            {
                channelInformation.Set(ReactorChannel);
                channelInformation.IpAddress = "not available for OmmConsumer connections";
                channelInformation.Port = ReactorChannel.Port;
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

        public IRequestMsg RequestMsg()
        {
            if(codecRequestMsg is null)
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
            if(codecRefreshMsg is null)
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
            if(codecStatusMsg is null)
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
            if(codecCloseMsg is null)
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

        public virtual void NotifyOnAllMsg(Msg msg) { }
        public virtual void NotifyOnRefreshMsg() { }
        public virtual void NotifyOnUpdateMsg() { }
        public virtual void NotifyOnStatusMsg() { }
        public virtual void NotifyOnGenericMsg() { }
        public virtual void NotifyOnAckMsg() { }
    }

    internal class LongIdGenerator
    {
        private const long MIN_LONG_VALUE = 1;
        private const long MAX_LONG_VALUE = long.MaxValue;

        private static long _longId = MIN_LONG_VALUE;

        private static Locker LongIdLocker = new WriteLocker(new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion));

        private static void Reset()
        {
            _longId = MIN_LONG_VALUE;
        }

        public static long NextLongId()
        {
            LongIdLocker.Enter();

            if (_longId == MAX_LONG_VALUE)
                Reset();

            _longId++;

            LongIdLocker.Exit();

            return _longId;
        }
    }
}
