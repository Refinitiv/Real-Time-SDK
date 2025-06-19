/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class EmaTestUtils
    {
        // Copies a ReactorErrorInfo object
        public static void CopyErrorInfo(ReactorErrorInfo srcErrorInfo, ReactorErrorInfo destErrorInfo)
        {
            destErrorInfo.Location = srcErrorInfo.Location;
            destErrorInfo.Error.Channel = srcErrorInfo.Error.Channel;
            destErrorInfo.Error.ErrorId = srcErrorInfo.Error.ErrorId;
            destErrorInfo.Error.SysError = srcErrorInfo.Error.SysError;
            if (srcErrorInfo.Error.Text != null)
                destErrorInfo.Error.Text = srcErrorInfo.Error.Text;
            destErrorInfo.Code = srcErrorInfo.Code;
        }

        // Copy ReactorMsgEvent parts. 
        public static void CopyMsgEvent(ReactorMsgEvent otherEvent, ReactorMsgEvent thisEvent)
        {
            if (otherEvent.Msg != null)
            {
                thisEvent.Msg = new Eta.Codec.Msg();
                otherEvent.Msg.Copy(thisEvent.Msg, CopyMsgFlags.ALL_FLAGS);
            }

            if (otherEvent.StreamInfo != null)
            {
                thisEvent.StreamInfo.UserSpec = otherEvent.StreamInfo.UserSpec;

                if (otherEvent.StreamInfo.ServiceName != null)
                {
                    thisEvent.StreamInfo.ServiceName = otherEvent.StreamInfo.ServiceName;
                }
            }

            thisEvent.ReactorChannel = otherEvent.ReactorChannel;
            EmaTestUtils.CopyErrorInfo(otherEvent.ReactorErrorInfo, thisEvent.ReactorErrorInfo);
        }

        // Copies a DirectoryMsg
        public static void CopyDirectoryMsg(DirectoryMsg srcMsg, DirectoryMsg destMsg)
        {
            switch (srcMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    destMsg.DirectoryMsgType = DirectoryMsgType.REQUEST;
                    srcMsg.DirectoryRequest!.Copy(destMsg.DirectoryRequest!);
                    break;
                case DirectoryMsgType.CLOSE:
                    destMsg.DirectoryMsgType = DirectoryMsgType.CLOSE;
                    srcMsg.DirectoryClose!.Copy(destMsg.DirectoryClose!);
                    break;
                case DirectoryMsgType.REFRESH:
                    destMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
                    srcMsg.DirectoryRefresh!.Copy(destMsg.DirectoryRefresh!);
                    break;
                case DirectoryMsgType.UPDATE:
                    destMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                    srcMsg.DirectoryUpdate!.Copy(destMsg.DirectoryUpdate!);
                    break;
                case DirectoryMsgType.STATUS:
                    destMsg.DirectoryMsgType = DirectoryMsgType.STATUS;
                    srcMsg.DirectoryStatus!.Copy(destMsg.DirectoryStatus!);
                    break;
                case DirectoryMsgType.CONSUMER_STATUS:
                    destMsg.DirectoryMsgType = DirectoryMsgType.CONSUMER_STATUS;
                    srcMsg.DirectoryConsumerStatus!.Copy(destMsg.DirectoryConsumerStatus!);
                    break;
                default:
                    Assert.True(false);
                    break;
            }
        }

        // Copies a DictionaryMsg.
        public static void CopyDictionaryMsg(DictionaryMsg srcMsg, DictionaryMsg destMsg)
        {
            switch (srcMsg.DictionaryMsgType)
            {
                case DictionaryMsgType.REQUEST:
                    destMsg.DictionaryMsgType = DictionaryMsgType.REQUEST;
                    srcMsg.DictionaryRequest!.Copy(destMsg.DictionaryRequest!);
                    break;
                case DictionaryMsgType.CLOSE:
                    destMsg.DictionaryMsgType = DictionaryMsgType.CLOSE;
                    srcMsg.DictionaryClose!.Copy(destMsg.DictionaryClose!);
                    break;
                case DictionaryMsgType.REFRESH:
                    destMsg.DictionaryMsgType = DictionaryMsgType.REFRESH;
                    srcMsg.DictionaryRefresh!.Copy(destMsg.DictionaryRefresh!);
                    break;
                case DictionaryMsgType.STATUS:
                    destMsg.DictionaryMsgType = DictionaryMsgType.STATUS;
                    srcMsg.DictionaryStatus!.Copy(destMsg.DictionaryStatus!);
                    break;
                default:
                    Assert.True(false);
                    break;
            }
        }

        // todo: why is it here and not in the LoginMsg class?
        internal static void CopyLoginMsg(LoginMsg srcMsg, LoginMsg destMsg)
        {
            switch (srcMsg.LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    destMsg.LoginMsgType = LoginMsgType.REQUEST;
                    srcMsg.LoginRequest!.Copy(destMsg.LoginRequest!);
                    break;
                case LoginMsgType.CLOSE:
                    destMsg.LoginMsgType = LoginMsgType.CLOSE;
                    srcMsg.LoginClose!.Copy(destMsg.LoginClose!);
                    break;
                case LoginMsgType.REFRESH:
                    destMsg.LoginMsgType = LoginMsgType.REFRESH;
                    srcMsg.LoginRefresh!.Copy(destMsg.LoginRefresh!);
                    break;
                case LoginMsgType.STATUS:
                    destMsg.LoginMsgType = LoginMsgType.STATUS;
                    srcMsg.LoginStatus!.Copy(destMsg.LoginStatus!);
                    break;
                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    destMsg.LoginMsgType = LoginMsgType.CONSUMER_CONNECTION_STATUS;
                    srcMsg.LoginConsumerConnectionStatus!.Copy(destMsg.LoginConsumerConnectionStatus!);
                    break;
                case LoginMsgType.RTT:
                    destMsg.LoginMsgType = LoginMsgType.RTT;
                    srcMsg.LoginRTT!.Copy(destMsg.LoginRTT!);
                    break;
                default:
                    Fail("Unknown LoginMsgType.");
                    break;
            }
        }

        public static void Fail(string message)
        {
            throw new global::Xunit.Sdk.XunitException(message);
        }

        public static int QosRate(uint emaRate)
        {
            int rate;

            switch(emaRate)
            {
                case OmmQos.Rates.TICK_BY_TICK:
                    {
                        rate = Eta.Codec.QosRates.TICK_BY_TICK;
                        break;
                    }
                case OmmQos.Rates.JUST_IN_TIME_CONFLATED:
                    {
                        rate = Eta.Codec.QosRates.JIT_CONFLATED;
                        break;
                    }
                default:
                    {
                        rate = (int)emaRate;
                        break;
                    }
            }

            return rate;
        }

        public static int QosTimeliness(uint emaTimeliness)
        {
            int timeliness;

            switch (emaTimeliness)
            {
                case OmmQos.Timelinesses.REALTIME:
                    {
                        timeliness = Eta.Codec.QosTimeliness.REALTIME;
                        break;
                    }
                case OmmQos.Timelinesses.INEXACT_DELAYED:
                    {
                        timeliness = Eta.Codec.QosTimeliness.DELAYED_UNKNOWN;
                        break;
                    }
                default:
                    {
                        timeliness = (int)emaTimeliness;
                        break;
                    }
            }

            return timeliness;
        }
    }
}
