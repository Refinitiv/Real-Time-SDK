/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using Refinitiv.Eta.Codec;
using System.Diagnostics;

namespace Refinitiv.Eta.ValueAdd.Rdm
{

    /// <summary>The RDM Login Base Message.</summary>
    ///
    /// <remarks>This RDM dictionary messages may be reused or pooled in a single collection via
    /// their common <c>LoginMsg</c> base class and re-used as a different
    /// <see cref="LoginMsgType"/>.</remarks>
    ///
    /// <seealso cref="LoginClose"/>
    /// <seealso cref="LoginRefresh"/>
    /// <seealso cref="LoginRequest"/>
    /// <seealso cref="LoginStatus"/>
    /// <seealso cref="LoginConsumerConnectionStatus"/>
    public class LoginMsg
    {
        private LoginClose? m_LoginClose;
        private LoginRefresh? m_LoginRefresh;
        private LoginRequest? m_LoginRequest;
        private LoginStatus? m_LoginStatus;
        private LoginConsumerConnectionStatus? m_LoginConsumerConnectionStatus;
        private LoginRTT? m_LoginRTT;

        private LoginMsgType m_LoginMsgType = LoginMsgType.UNKNOWN;

        /// Login message type. These are defined per-message class basis for login
        public LoginMsgType LoginMsgType
        {
            get => m_LoginMsgType;
            set
            {
                m_LoginMsgType = value;
                switch (value)
                {
                    case LoginMsgType.CLOSE:
                        if (m_LoginClose == null)
                            m_LoginClose = new LoginClose();
                        break;
                    case LoginMsgType.REFRESH:
                        if (m_LoginRefresh == null)
                            m_LoginRefresh = new LoginRefresh();
                        break;
                    case LoginMsgType.REQUEST:
                        if (m_LoginRequest == null)
                            m_LoginRequest = new LoginRequest();
                        break;
                    case LoginMsgType.STATUS:
                        if (m_LoginStatus == null)
                            m_LoginStatus = new LoginStatus();
                        break;
                    case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                        if (m_LoginConsumerConnectionStatus == null)
                            m_LoginConsumerConnectionStatus = new LoginConsumerConnectionStatus();
                        break;
                    case LoginMsgType.RTT:
                        if (m_LoginRTT == null)
                            m_LoginRTT = new LoginRTT();
                        break;
                    default:
                        break;
                }
            }
        }

        public LoginClose? LoginClose
        {
            get => (LoginMsgType == LoginMsgType.CLOSE) ? m_LoginClose : null;
        }

        public LoginRefresh? LoginRefresh
        {
            get => (LoginMsgType == LoginMsgType.REFRESH) ? m_LoginRefresh : null;
        }

        public LoginRequest? LoginRequest
        {
            get => (LoginMsgType == LoginMsgType.REQUEST) ? m_LoginRequest : null;
        }

        public LoginStatus? LoginStatus
        {
            get => (LoginMsgType == LoginMsgType.STATUS) ? m_LoginStatus : null;
        }

        public LoginConsumerConnectionStatus? LoginConsumerConnectionStatus
        {
            get => (LoginMsgType == LoginMsgType.CONSUMER_CONNECTION_STATUS)
                ? m_LoginConsumerConnectionStatus
                : null;
        }

        public LoginRTT? LoginRTT
        {
            get => (LoginMsgType == LoginMsgType.RTT)
                ? m_LoginRTT
                : null;
        }

        private MsgBase? GetMsg()
        {
            switch (m_LoginMsgType)
            {
                case LoginMsgType.CLOSE:
                    return m_LoginClose;

                case LoginMsgType.REFRESH:
                    return m_LoginRefresh;

                case LoginMsgType.REQUEST:
                    return m_LoginRequest;

                case LoginMsgType.STATUS:
                    return m_LoginStatus;

                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    return m_LoginConsumerConnectionStatus;

                case LoginMsgType.RTT:
                    return m_LoginRTT;

                default:
                    return null;
            }
        }

        public override string ToString()
        {
            return GetMsg()!.ToString() ?? String.Empty;
        }

        public CodecReturnCode Decode(DecodeIterator dIter, Msg msg)
        {
            switch (LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    return LoginRequest!.Decode(dIter, msg);
                case LoginMsgType.REFRESH:
                    return LoginRefresh!.Decode(dIter, msg);
                case LoginMsgType.STATUS:
                    return LoginStatus!.Decode(dIter, msg);
                case LoginMsgType.CLOSE:
                    return LoginClose!.Decode(dIter, msg);
                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    return LoginStatus!.Decode(dIter, msg);
                case LoginMsgType.RTT:
                    return LoginRTT!.Decode(dIter, msg);
                default:
                    Debug.Assert (false); // not supported on this message class
                    return CodecReturnCode.FAILURE;
            }
        }
    }
}
