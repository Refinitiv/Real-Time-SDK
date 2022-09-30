/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using DateTime = System.DateTime;

namespace Refinitiv.Eta.Example.Common
{
    /// <summary>
    /// This is the implementation of processing of login requests and login status
    /// messages.
    /// <para>Only one login stream per channel is allowed by this simple provider.</para>
    /// </summary>
    public class ProviderLoginHandler
    {
        private static readonly int RTT_NOTIFICATION_INTERVAL = 5;

        private static readonly int REJECT_MSG_SIZE = 512;
        private static readonly int REFRESH_MSG_SIZE = 512;
        private static readonly int STATUS_MSG_SIZE = 512;
        private static readonly int RTT_MSG_SIZE = 1024;

        private readonly LoginStatus m_LoginStatus = new LoginStatus();
        private readonly LoginRefresh m_LoginRefresh = new LoginRefresh();
        private readonly LoginRequest m_LoginRequest = new LoginRequest();
        private LoginRTT m_LoginRtt = new LoginRTT();
        private readonly EncodeIterator m_EncodeIter = new EncodeIterator();

        // application id 
        private static readonly string m_ApplicationId = "256";

        // application name
        private static readonly string m_ApplicationName = "ETA Provider";

        /// <summary>
        /// Gets or sets whether to support the RTT feature by current provider.
        /// </summary>
        public bool EnableRtt { get; set; }

        private LoginRequestInfoList m_LoginRequestInfoList;
        private LoginRttInfoList m_LoginRttInfoList;
        private ProviderSession m_ProviderSession;

        /// <summary>
        /// Default constructor
        /// </summary>
        /// <param name="providerSession">The provider session</param>
        public ProviderLoginHandler(ProviderSession providerSession)
        {
            m_ProviderSession = providerSession;
            m_LoginRequestInfoList = new LoginRequestInfoList();
            m_LoginRttInfoList = new LoginRttInfoList();
        }

        /// <summary>
        /// Initializes login information fields.
        /// </summary>
        public void Init()
        {
            m_LoginRequestInfoList.Init();
            m_LoginRttInfoList.Init();
        }

        /// <summary>
        /// Processes a login request.
        /// </summary>
        /// <param name="chnl">The channel of the request</param>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error">Error information in case of failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode ProcessRequest(IChannel chnl, Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    m_LoginRequest.Clear();
                    CodecReturnCode ret = m_LoginRequest.Decode(dIter, msg);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"LoginRequest.Decode() failed with return code: {ret.GetAsString()}"
                        };

                        return ret;
                    }

                    Console.WriteLine($"Received Login Request for Username: {m_LoginRequest.UserName}");

                    try
                    {
                        Console.WriteLine(m_LoginRequest.ToString());
                    }
                    catch (Exception)
                    {
                        return SendRequestReject(chnl, msg.StreamId, LoginRejectReason.LOGIN_RDM_DECODER_FAILED, out error);
                    }

                    LoginRequestInfo? loginRequestInfo = m_LoginRequestInfoList.Get(chnl, m_LoginRequest);
                    if (loginRequestInfo is null)
                    {
                        return SendRequestReject(chnl, msg.StreamId, LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED, out error);
                    }

                    /* check if key has user name */
                    /*
                     * user name is only login user type accepted by this
                     * application (user name is the default type)
                     */
                    if (!msg.MsgKey.CheckHasName()
                        || (msg.MsgKey.CheckHasNameType()
                            && (msg.MsgKey.NameType != (int)Login.UserIdTypes.NAME)))
                    {
                        return SendRequestReject(chnl, msg.StreamId, LoginRejectReason.NO_USER_NAME_IN_REQUEST, out error);
                    }

                    /* send login response */
                    return SendRefresh(chnl, loginRequestInfo, out error);
                case MsgClasses.GENERIC:
                    if (DataTypes.ELEMENT_LIST == msg.ContainerType)
                    {
                        m_LoginRtt.Clear();
                        ret = m_LoginRtt.Decode(dIter, msg);
                        if (ret != CodecReturnCode.SUCCESS)
                        {
                            error = new Error
                            {
                                Text = $"LoginRTT.Decode() failed with return code: {ret.GetAsString()}"
                            };

                            return ret;
                        }
                        Console.WriteLine($"Received login RTT message from Consumer {m_ProviderSession.GetClientSessionForChannel(chnl)!.SelectElement.Socket}");
                        Console.WriteLine($"\tRTT Tick value is {m_LoginRtt.Ticks / 1000}us");
                        if (m_LoginRtt.HasTCPRetrans)
                        {
                            Console.WriteLine($"\tConsumer side TCP retransmissions: {m_LoginRtt.TCPRetrans}");
                        }
                        long calculatedRtt = m_LoginRtt.CalculateRTTLatency(LoginRTT.TimeUnit.MICRO_SECONDS);
                        LoginRTT storedLoginRtt = GetLoginRttInfo(chnl)!.LoginRtt;
                        m_LoginRtt.Copy(storedLoginRtt);
                        Console.WriteLine($"\tLast RTT message latency is {calculatedRtt}us.\n");
                    }
                    break;
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Login Close for StreamId {msg.StreamId}");
                    CloseStream(msg.StreamId);
                    break;

                default:
                    //error.text("Received Unhandled Login Msg Class: " + MsgClasses.toString(msg.msgClass()));
                    error = new Error
                    {
                        Text = $"Received Unhandled Login Msg Class: {MsgClasses.ToString(msg.MsgClass)}"
                    };
                    return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        private void CloseStream(int streamId)
        {
            /* find original request information associated with streamId */
            foreach (LoginRequestInfo loginReqInfo in m_LoginRequestInfoList)
            {
                if (loginReqInfo.LoginRequest.StreamId == streamId)
                {
                    /* clear original request information */
                    Console.WriteLine($"Closing login stream id '{loginReqInfo.LoginRequest.StreamId}'  with user name: {loginReqInfo.LoginRequest.UserName}");
                    loginReqInfo.Clear();
                    break;
                }
            }
        }

        public void CloseRequestAndRtt(IChannel channel)
        {
            //find original request information associated with channel
            LoginRequestInfo? loginReqInfo = FindLoginRequestInfo(channel);
            if (loginReqInfo != null)
            {
                Console.WriteLine($"Closing login stream id '{loginReqInfo.LoginRequest.StreamId}' with user name: {loginReqInfo.LoginRequest.UserName}");
                loginReqInfo.Clear();
            }

            //find RTT information associated with channel
            ClearRttInfo(channel);
        }


        private void ClearRttInfo(IChannel channel)
        {
            if (EnableRtt)
            {
                LoginRttInfo? loginRttInfo = m_LoginRttInfoList.Get(channel);
                if (loginRttInfo != null)
                {
                    loginRttInfo.Clear();
                }
            }
        }

        /// <summary>
        /// Sends RTT message if this feature has been enabled.
        /// </summary>
        /// <param name="channel">The channel for sending Login RTT message to.</param>
        /// <param name="error">The error in case of errors</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode ProceedLoginRttMessage(IChannel channel, out Error? error)
        {
            error = null;
            if (EnableRtt)
            {
                LoginRttInfo? loginRttInfo = CreateOrGetRttInfo(channel);
                if (loginRttInfo != null && IsRttReadyToSend(loginRttInfo))
                {
                    ITransportBuffer transportBuffer = channel.GetBuffer(RTT_MSG_SIZE, false, out error);
                    if (transportBuffer is null)
                    {
                        return CodecReturnCode.FAILURE;
                    }

                    m_EncodeIter.Clear();
                    m_EncodeIter.SetBufferAndRWFVersion(transportBuffer, channel.MajorVersion, channel.MinorVersion);

                    m_LoginRtt = loginRttInfo.LoginRtt;
                    m_LoginRtt.UpdateRTTActualTicks();
                    CodecReturnCode ret = m_LoginRtt.Encode(m_EncodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"Encoding of login RTT failed: <{ret.GetAsString()}>"
                        };

                        return ret;
                    }
                    return (CodecReturnCode)m_ProviderSession.Write(channel, transportBuffer, out error);
                }
            }

            return CodecReturnCode.SUCCESS;
        }

        private static bool IsRttReadyToSend(LoginRttInfo loginRttInfo)
        {
            long rttLastSendTime = loginRttInfo.RttLastSendNanoTime;
            long rttSendTime = DateTime.Now.Ticks * 100;
            if (((rttSendTime - rttLastSendTime) / 1000000) > RTT_NOTIFICATION_INTERVAL)
            {
                loginRttInfo.RttLastSendNanoTime = rttSendTime;
                return true;
            }
            return false;
        }

        private CodecReturnCode SendRequestReject(IChannel chnl, int streamId, LoginRejectReason reason, out Error? error)
        {
            // get a buffer for the login request reject status */
            ITransportBuffer msgBuf = chnl.GetBuffer(REJECT_MSG_SIZE, false, out error);
            if (msgBuf is null)
            {
                return CodecReturnCode.FAILURE;
            }
            CodecReturnCode ret = EncodeRequestReject(chnl, streamId, reason, msgBuf, out error);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return ret;
            }
            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        private CodecReturnCode EncodeRequestReject(IChannel chnl, int streamId, LoginRejectReason reason, ITransportBuffer msgBuf, out Error? error)
        {
            error = null;

            // set-up message 
            m_LoginStatus.StreamId = streamId;
            m_LoginStatus.HasState = true;
            m_LoginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_LoginStatus.State.DataState(DataStates.SUSPECT);
            switch (reason)
            {
                case LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED:
                    m_LoginStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_LoginStatus.State.Text().Data($"Login request rejected for stream id {streamId}- max request count reached");
                    break;
                case LoginRejectReason.NO_USER_NAME_IN_REQUEST:
                    m_LoginStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_LoginStatus.State.Text().Data($"Login request rejected for stream id {streamId}- request does not contain user name");
                    m_LoginStatus.State.Code(StateCodes.USAGE_ERROR);
                    break;
                default:
                    break;
            }

            // clear encode iterator
            m_EncodeIter.Clear();

            // encode message
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    Text = $"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}"
                };

                return ret;
            }

            ret = m_LoginStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    Text = $"LoginStatus.Encode() failed"
                };

                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the login close status message for a channel.
        /// </summary>
        /// <param name="chnl">The channel to send close status message to</param>
        /// <param name="error">Error information in case of failure</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode SendCloseStatus(IChannel chnl, out Error? error)
        {
            LoginRequestInfo? loginReqInfo = m_LoginRequestInfoList.Get(chnl);
            if (loginReqInfo == null)
            {
                error = new Error
                {
                    Text = "Could not find login request information for the channel"
                };

                return CodecReturnCode.FAILURE;
            }

            // get a buffer for the login close
            ITransportBuffer msgBuf = chnl.GetBuffer(STATUS_MSG_SIZE, false, out error);
            if (msgBuf is null)
                return CodecReturnCode.FAILURE;

            m_LoginStatus.Clear();
            m_LoginStatus.StreamId = loginReqInfo.LoginRequest.StreamId;
            m_LoginStatus.HasState = true;
            m_LoginStatus.State.StreamState(StreamStates.CLOSED);
            m_LoginStatus.State.DataState(DataStates.SUSPECT);
            m_LoginStatus.State.Text().Data("Login stream closed");
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ret;
            }

            ret = m_LoginStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    Text = "LoginStatus.Encode() failed"
                };

                return ret;
            }

            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        private CodecReturnCode SendRefresh(IChannel chnl, LoginRequestInfo loginReqInfo, out Error? error)
        {
            error = null;
            // get a buffer for the login response 
            ITransportBuffer msgBuf = chnl.GetBuffer(REFRESH_MSG_SIZE, false, out error);
            if (msgBuf is null)
                return CodecReturnCode.FAILURE;

            m_LoginRefresh.Clear();

            // provide login response information 

            // streamId 
            m_LoginRefresh.StreamId = loginReqInfo.LoginRequest.StreamId;

            // username 
            m_LoginRefresh.HasUserName = true;
            m_LoginRefresh.UserName.Data(loginReqInfo.LoginRequest.UserName.Data(), loginReqInfo.LoginRequest.UserName.Position, loginReqInfo.LoginRequest.UserName.Length);

            m_LoginRefresh.HasUserNameType = true;
            m_LoginRefresh.UserNameType = Login.UserIdTypes.NAME;

            m_LoginRefresh.State.Code(StateCodes.NONE);
            m_LoginRefresh.State.DataState(DataStates.OK);
            m_LoginRefresh.State.StreamState(StreamStates.OPEN);
            m_LoginRefresh.State.Text().Data("Login accepted by host localhost");

            m_LoginRefresh.Solicited = true;

            m_LoginRefresh.HasAttrib = true;
            m_LoginRefresh.LoginAttrib.HasApplicationId = true;
            m_LoginRefresh.LoginAttrib.ApplicationId.Data(m_ApplicationId);
            m_LoginRefresh.LoginAttrib.HasApplicationName = true;
            m_LoginRefresh.LoginAttrib.ApplicationName.Data(m_ApplicationName);

            if (loginReqInfo.LoginRequest.HasAttrib && loginReqInfo.LoginRequest.LoginAttrib.HasPosition)
            {
                m_LoginRefresh.LoginAttrib.HasPosition = true;
                m_LoginRefresh.LoginAttrib.Position.Data(loginReqInfo.LoginRequest.LoginAttrib.Position.Data(), loginReqInfo.LoginRequest.LoginAttrib.Position.Position,
                    loginReqInfo.LoginRequest.LoginAttrib.Position.Length);
            }

            // this provider does not support
            // singleOpen behavior
            m_LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginAttrib.SingleOpen = 0; ;

            if (EnableRtt)
            {
                m_LoginRefresh.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
            }

            // this provider supports batch requests
            m_LoginRefresh.HasFeatures = true;
            m_LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.SupportedFeatures.SupportBatchRequests = 1;

            m_LoginRefresh.SupportedFeatures.HasSupportPost = true;
            m_LoginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // keep default values for all others
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.setBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ret;
            }

            ret = m_LoginRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error
                {
                    Text = "LoginRefresh.Encode() failed"
                };

                return ret;
            }

            return (CodecReturnCode)m_ProviderSession.Write(chnl, msgBuf, out error);
        }

        /// <summary>
        /// Finds a login request information for the channel.
        /// </summary>
        /// <param name="chnl">The channel to get the login request information for.</param>
        /// <returns><see cref="LoginRequestInfo"/></returns>
        public LoginRequestInfo? FindLoginRequestInfo(IChannel chnl)
        {
            return m_LoginRequestInfoList.Get(chnl);
        }

        /// <summary>
        /// Try to get a login rtt information for the specified channel.
        /// </summary>
        /// <param name="channel">The channel to get the login RTT information for.</param>
        /// <returns><see cref="LoginRequestInfo"/></returns>
        public LoginRttInfo? CreateOrGetRttInfo(IChannel channel)
        {
            LoginRttInfo? loginRttInfo = m_LoginRttInfoList.Get(channel);

            if (loginRttInfo is null)
            {
                loginRttInfo = CreateLoginRtt(channel);
            }

            return loginRttInfo;
        }

        /// <summary>
        /// Creates a login rtt information for the specified channel.
        /// </summary>
        /// <param name="channel">The channel to create the login RTT information for.</param>
        /// <returns><see cref="LoginRequestInfo"/></returns>
        public LoginRttInfo? CreateLoginRtt(IChannel channel)
        {
            LoginRequestInfo? loginRequestInfo = FindLoginRequestInfo(channel);
            if (loginRequestInfo != null && loginRequestInfo.IsInUse)
            {
                LoginRttInfo? loginRttInfo = m_LoginRttInfoList.CreateFromRequest(channel, loginRequestInfo.LoginRequest);
                if (loginRttInfo != null)
                {
                    loginRttInfo.LoginRtt.HasRTLatency = true;
                    return loginRttInfo;
                }
            }

            return null;
        }

        /// <summary>
        /// Finds a login rtt information for the specified channel.
        /// </summary>
        /// <param name="channel">The channel to find the login RTT information for.</param>
        /// <returns><see cref="LoginRequestInfo"/></returns>
        public LoginRttInfo? GetLoginRttInfo(IChannel channel)
        {
            return m_LoginRttInfoList.Get(channel);
        }
    }
}
