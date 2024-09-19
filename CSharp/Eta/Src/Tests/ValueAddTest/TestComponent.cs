/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;

using Xunit;

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValuedAdd.Tests
{
    internal class TestComponent : IReactorChannelEventCallback, IDictionaryMsgCallback, IDirectoryMsgCallback, IDefaultMsgCallback, IRDMLoginMsgCallback
    {
        private ReactorMsgEvent m_LastReactorMsgEvent = null;
        private RDMDictionaryMsgEvent m_LastRDMDictionaryMsgEvent = null;
        private RDMDirectoryMsgEvent m_LastRDMDirectoryMsgEvent = null;
        public int NumChannelOpenedEvent { get; private set; }
        public int NumChannelUpEvent { get; private set; }
        public int NumChannelReadyEvent { get; private set; }
        public int NumChannelDownEvent { get; private set; }
        public int NumChannelDownReconnectingEvent { get; private set; }
        public int NumChannelWarningEvent { get; private set; }
        public int NumFDChangeEvent { get; private set; }
        public int NumLoginMsgEvent { get; private set; }
        public int NumDirectoryMsgEvent { get; private set; }
        public int NumDictionaryMsgEvent { get; private set; }
        public int NumDefaultMsgEvent { get; private set; }
        public ReactorMsgEvent LastReactorMsgEvent
        {
            get
            {
                var reactorEvent = m_LastReactorMsgEvent;
                m_LastReactorMsgEvent = null;
                return reactorEvent;
            }
            private set { m_LastReactorMsgEvent = value; }
        }
        public RDMDictionaryMsgEvent LastRDMDictionaryMsgEvent
        {
            get
            {
                var reactorEvent = m_LastRDMDictionaryMsgEvent;
                m_LastRDMDictionaryMsgEvent = null;
                return reactorEvent;
            }
            private set { m_LastRDMDictionaryMsgEvent = value; }
        }
        public RDMDirectoryMsgEvent LastRDMDirectoryMsgEvent
        {
            get
            {
                var reactorEvent = m_LastRDMDirectoryMsgEvent;
                m_LastRDMDirectoryMsgEvent = null;
                return reactorEvent;
            }
            private set { m_LastRDMDirectoryMsgEvent = value; }
        }

        public DataDictionary Dictionary { get; private set; } = new DataDictionary();

        public ReactorChannel ReactorChannel { get; private set; }

        public ReactorCallbackReturnCode ChannelReturnCode { get; set; } = ReactorCallbackReturnCode.SUCCESS;

        public ReactorCallbackReturnCode MsgReturnCode { get; set; } = ReactorCallbackReturnCode.SUCCESS;


        public virtual ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            ++NumDefaultMsgEvent;
            CheckDefaultMsg(msgEvent.Msg);
            OnDefaultMsgReceived(msgEvent);
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public virtual ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            ++NumLoginMsgEvent;
            CheckLoginMsg(loginEvent.LoginMsg);
            OnLoginMsgReceived(loginEvent);
            return MsgReturnCode;
        }

        public virtual ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
        {
            ++NumDictionaryMsgEvent;
            Console.WriteLine($"DEBUG: ReactorCallbackHandler.RdmDictionaryMsgCallback: entered. dictionaryMsgEventCount={NumDictionaryMsgEvent}, event = {msgEvent.ToString()}");

            m_LastRDMDictionaryMsgEvent = new RDMDictionaryMsgEvent();
            TestUtil.CopyMsgEvent(msgEvent, m_LastRDMDictionaryMsgEvent);

            if (msgEvent.DictionaryMsg != null)
            {
                m_LastRDMDictionaryMsgEvent.DictionaryMsg = new DictionaryMsg();
                TestUtil.CopyDictionaryMsg(msgEvent.DictionaryMsg, m_LastRDMDictionaryMsgEvent.DictionaryMsg);
            }

            CheckDictionaryMsg(msgEvent.DictionaryMsg);
            OnDictionaryMsgReceived(msgEvent);

            return MsgReturnCode;
        }

        public virtual ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
        {
            ++NumDirectoryMsgEvent;

            Console.WriteLine($"DEBUG: ReactorCallbackHandler.RdmDirectoryMsgCallback: entered. DirectoryMsgEventCount={NumDirectoryMsgEvent}, event={directoryMsgEvent.ToString()}");

            LastRDMDirectoryMsgEvent = new RDMDirectoryMsgEvent();
            TestUtil.CopyMsgEvent(directoryMsgEvent, m_LastRDMDirectoryMsgEvent);

            if (directoryMsgEvent.DirectoryMsg != null)
            {
                m_LastRDMDirectoryMsgEvent.DirectoryMsg = new DirectoryMsg();
                TestUtil.CopyDirectoryMsg(directoryMsgEvent.DirectoryMsg, m_LastRDMDirectoryMsgEvent.DirectoryMsg);
            }

            CheckDirectoryMsg(directoryMsgEvent.DirectoryMsg);
            OnDirectoryMsgReceived(directoryMsgEvent);

            return MsgReturnCode;
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent channelEvent)
        {
            ReactorChannel = channelEvent.ReactorChannel;

            switch (channelEvent.EventType)
            {
                case ReactorChannelEventType.CHANNEL_OPENED:
                    ++NumChannelOpenedEvent;
                    break;
                case ReactorChannelEventType.CHANNEL_UP:
                    ++NumChannelUpEvent;
                    OnChannelUp(channelEvent);
                    break;
                case ReactorChannelEventType.CHANNEL_READY:
                    ++NumChannelReadyEvent;
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN:
                    ++NumChannelDownEvent;
                    channelEvent.ReactorChannel.Close(out var errorInfo);
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    ++NumChannelDownReconnectingEvent;
                    break;
                case ReactorChannelEventType.WARNING:
                    ++NumChannelWarningEvent;
                    break;
                case ReactorChannelEventType.FD_CHANGE:
                    ++NumFDChangeEvent;
                    break;
                default:
                    Assert.True(false); /* unexpected channel type. */
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        protected void LoadFieldDictionary(string fieldDictionary)
        {
            var tmpFile = new StreamWriter("RDMFieldDictionary");
            try {
                tmpFile.Write(fieldDictionary);
                tmpFile.Close();
                if (Dictionary.LoadFieldDictionary("RDMFieldDictionary", out CodecError error) < 0)
                {
                    Console.WriteLine($"Unable to load field dictionary. Error Text: {error.Text}");
                    Assert.True(false);
                }
            } finally
            {
                File.Delete("RDMFieldDictionary");
            }
        }
        protected void LoadEnumTypeDictionary(string enumTypeDictionary)
        {
            var tmpFile = new StreamWriter("enumtype.def");
            try
            {
                tmpFile.Write(enumTypeDictionary);
                tmpFile.Close();
                if (Dictionary.LoadEnumTypeDictionary("enumtype.def", out CodecError error) < 0)
                {
                    Console.WriteLine($"Unable to load enum dictionary. Error Text: {error.Text}");
                    Assert.True(false);
                }
            } finally
            {
                File.Delete("enumtype.def");
            }

        }


        public Action<ReactorChannelEvent> OnChannelUp = e => { };

        public Action<LoginMsg> CheckLoginMsg = msg => { };

        public Action<DirectoryMsg> CheckDirectoryMsg = msg => { };

        public Action<DictionaryMsg> CheckDictionaryMsg = msg => { };

        public Action<IMsg> CheckDefaultMsg = msg => { };


        public Action<RDMLoginMsgEvent> OnLoginMsgReceived = msg => { };

        public Action<RDMDictionaryMsgEvent> OnDictionaryMsgReceived = msg => { };

        public Action<RDMDirectoryMsgEvent> OnDirectoryMsgReceived = msg => { };

        public Action<ReactorMsgEvent> OnDefaultMsgReceived = msg => { };
    }
}
