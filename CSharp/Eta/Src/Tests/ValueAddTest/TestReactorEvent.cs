/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Eta.ValuedAdd.Tests;


/// <summary>
/// Identifies which type of ReactorEvent the ComponentEvent contains.
/// </summary>
public enum TestReactorEventType
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

    STATUS_MSG,

    /// <summary>
    /// ReactorMsgEvent
    /// </summary>
    MSG,

    AUTH_TOKEN_EVENT,
    SERVICE_DISC_ENDPOINT
}

/// <summary>
/// Represents an event received when a component calls dispatch on its reactor.
/// The event is fully copied, including such data as messages received in Reactor events.
/// </summary>
public class TestReactorEvent
{
    public TestReactorEventType EventType { get; private set; }
    public ReactorEvent ReactorEvent { get; private set; }

    /// <summary>
    /// Returns time at which the event was received, as taken by System.nanoTime()
    /// </summary>
    public System.DateTime NanoTime { get; private set; } = System.DateTime.Now;


    /** Construct an event based on its type */
    public TestReactorEvent(TestReactorEventType type, ReactorEvent evt)
    {
        EventType = type;

        /* Copy event, based on which type it is. */
        switch (type)
        {
            case TestReactorEventType.AUTH_TOKEN_EVENT:
            case TestReactorEventType.SERVICE_DISC_ENDPOINT:
                break;

            case TestReactorEventType.CHANNEL_EVENT:
                {
                    ReactorEvent = new ReactorChannelEvent();
                    ((ReactorChannelEvent)ReactorEvent).EventType = ((ReactorChannelEvent)evt).EventType;
                    break;
                }

            case TestReactorEventType.MSG:
                {
                    ReactorEvent = new ReactorMsgEvent();
                    TestUtil.CopyMsgEvent((ReactorMsgEvent)evt, (ReactorMsgEvent)ReactorEvent);
                    break;
                }

            case TestReactorEventType.LOGIN_MSG:
                {
                    ReactorEvent = new RDMLoginMsgEvent();
                    RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)ReactorEvent;
                    RDMLoginMsgEvent otherLoginMsgEvent = (RDMLoginMsgEvent)evt;

                    TestUtil.CopyMsgEvent((ReactorMsgEvent)evt, (ReactorMsgEvent)ReactorEvent);

                    if (otherLoginMsgEvent.LoginMsg != null)
                    {
                        loginMsgEvent.LoginMsg = new();
                        TestUtil.CopyLoginMsg(otherLoginMsgEvent.LoginMsg, loginMsgEvent.LoginMsg);
                    }
                    break;
                }

            case TestReactorEventType.DIRECTORY_MSG:
                {
                    ReactorEvent = new RDMDirectoryMsgEvent();
                    RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)ReactorEvent;
                    RDMDirectoryMsgEvent otherDirectoryMsgEvent = (RDMDirectoryMsgEvent)evt;

                    TestUtil.CopyMsgEvent((ReactorMsgEvent)evt, (ReactorMsgEvent)ReactorEvent);

                    if (otherDirectoryMsgEvent.DirectoryMsg != null)
                    {
                        directoryMsgEvent.DirectoryMsg = new();
                        TestUtil.CopyDirectoryMsg(otherDirectoryMsgEvent.DirectoryMsg, directoryMsgEvent.DirectoryMsg);
                    }
                    break;
                }

            case TestReactorEventType.DICTIONARY_MSG:
                {
                    ReactorEvent = new RDMDictionaryMsgEvent();
                    RDMDictionaryMsgEvent dictionaryMsgEvent = (RDMDictionaryMsgEvent)ReactorEvent;
                    RDMDictionaryMsgEvent otherDictionaryMsgEvent = (RDMDictionaryMsgEvent)evt;

                    TestUtil.CopyMsgEvent((ReactorMsgEvent)evt, (ReactorMsgEvent)ReactorEvent);

                    if (otherDictionaryMsgEvent.DictionaryMsg != null)
                    {
                        dictionaryMsgEvent.DictionaryMsg = new DictionaryMsg();
                        TestUtil.CopyDictionaryMsg(otherDictionaryMsgEvent.DictionaryMsg, dictionaryMsgEvent.DictionaryMsg);
                    }
                    break;
                }

            default:
                throw new Xunit.Sdk.XunitException("Unknown ComponentEvent type.");

        }

        TestUtil.CopyErrorInfo(evt.ReactorErrorInfo, ReactorEvent.ReactorErrorInfo);
        ReactorEvent.ReactorChannel = evt.ReactorChannel;
    }
}
