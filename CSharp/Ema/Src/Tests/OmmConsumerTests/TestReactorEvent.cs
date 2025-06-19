/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal enum TestReactorEventType
    {
        /// <summary>
        /// ReactorChannelEvent
        /// </summary>
        CHANNEL_EVENT,

        /// <summary>
        /// RDMLoginMsgEvent
        /// </summary>
        LOGIN_MSG,

        /// <summary>
        /// RDMDirectoryMsgEvent
        /// </summary>
        DIRECTORY_MSG,

        /// <summary>
        /// RDMDictionaryMsgEvent
        /// </summary>
        DICTIONARY_MSG,

        /// <summary>
        /// ReactorMsgEvent
        /// </summary>
        MSG,
    }

    internal class TestReactorEvent
    {
        public TestReactorEventType EventType { get; private set; }
        public ReactorEvent ReactorEvent { get; private set; }

        public TestReactorEvent(TestReactorEventType eventType, ReactorEvent reactorEvent)
        {
            EventType = eventType;
            ReactorEvent = reactorEvent;

            /* Copy event, based on which type it is. */
            switch (eventType)
            {
                case TestReactorEventType.CHANNEL_EVENT:
                    {
                        ReactorEvent = new ReactorChannelEvent();
                        ((ReactorChannelEvent)ReactorEvent).EventType = ((ReactorChannelEvent)reactorEvent).EventType;
                        break;
                    }

                case TestReactorEventType.MSG:
                    {
                        ReactorEvent = new ReactorMsgEvent();
                        EmaTestUtils.CopyMsgEvent((ReactorMsgEvent)reactorEvent, (ReactorMsgEvent)ReactorEvent);
                        break;
                    }

                case TestReactorEventType.LOGIN_MSG:
                    {
                        ReactorEvent = new RDMLoginMsgEvent();
                        RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)ReactorEvent;
                        RDMLoginMsgEvent otherLoginMsgEvent = (RDMLoginMsgEvent)reactorEvent;

                        EmaTestUtils.CopyMsgEvent((ReactorMsgEvent)reactorEvent, (ReactorMsgEvent)ReactorEvent);

                        if (otherLoginMsgEvent.LoginMsg != null)
                        {
                            loginMsgEvent.LoginMsg = new();
                            EmaTestUtils.CopyLoginMsg(otherLoginMsgEvent.LoginMsg, loginMsgEvent.LoginMsg);
                        }
                        break;
                    }

                case TestReactorEventType.DIRECTORY_MSG:
                    {
                        ReactorEvent = new RDMDirectoryMsgEvent();
                        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)ReactorEvent;
                        RDMDirectoryMsgEvent otherDirectoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent;

                        EmaTestUtils.CopyMsgEvent((ReactorMsgEvent)reactorEvent, (ReactorMsgEvent)ReactorEvent);

                        if (otherDirectoryMsgEvent.DirectoryMsg != null)
                        {
                            directoryMsgEvent.DirectoryMsg = new();
                            EmaTestUtils.CopyDirectoryMsg(otherDirectoryMsgEvent.DirectoryMsg, directoryMsgEvent.DirectoryMsg);
                        }
                        break;
                    }

                case TestReactorEventType.DICTIONARY_MSG:
                    {
                        ReactorEvent = new RDMDictionaryMsgEvent();
                        RDMDictionaryMsgEvent dictionaryMsgEvent = (RDMDictionaryMsgEvent)ReactorEvent;
                        RDMDictionaryMsgEvent otherDictionaryMsgEvent = (RDMDictionaryMsgEvent)reactorEvent;

                        EmaTestUtils.CopyMsgEvent((ReactorMsgEvent)reactorEvent, (ReactorMsgEvent)ReactorEvent);

                        if (otherDictionaryMsgEvent.DictionaryMsg != null)
                        {
                            dictionaryMsgEvent.DictionaryMsg = new DictionaryMsg();
                            EmaTestUtils.CopyDictionaryMsg(otherDictionaryMsgEvent.DictionaryMsg, dictionaryMsgEvent.DictionaryMsg);
                        }
                        break;
                    }

                default:
                    {
                        EmaTestUtils.Fail("Unknown ComponentEvent type.");
                        break;
                    }
            }
        }
    }
}
