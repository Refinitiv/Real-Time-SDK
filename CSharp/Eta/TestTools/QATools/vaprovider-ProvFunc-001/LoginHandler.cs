/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using DateTime = System.DateTime;

namespace LSEG.Eta.ValueAdd.Provider
{
    /// <summary>
    /// This is the implementation of processing of login requests and login status messages.
    /// Only one login stream per channel is allowed by this simple provider.
    /// </summary>
    public class LoginHandler
    {
        private const int REJECT_MSG_SIZE = 512;
        private const int REFRESH_MSG_SIZE = 512;
        private const int STATUS_MSG_SIZE = 512;
        private const int RTT_MSG_SIZE = 1024;

        private const int RTT_NOTIFICATION_INTERVAL = 5;

        private LoginStatus m_LoginStatus = new LoginStatus();
        private LoginRefresh m_LoginRefresh = new LoginRefresh();
        private LoginRequest m_LoginRequest = new LoginRequest();
        private LoginRTT m_LoginRTT = new LoginRTT();
        private EncodeIterator m_EncodeIter = new EncodeIterator();
        //APIQA
        public bool SupportView { get; set; }
        //END APIQA
        private static string applicationId = "256";
        private static string applicationName = "ETA Provider";

        private LoginRequestInfoList m_LoginRequestInfoList;
        private LoginRttInfoList m_LoginRttInfoList;
        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        public bool EnableRtt { get; set; }

        public LoginHandler()
        {
            m_LoginRequestInfoList = new LoginRequestInfoList();
            m_LoginRttInfoList = new LoginRttInfoList();
        }

        /// <summary>
        /// Initializes login information fields
        /// </summary>
        public void Init()
        {
            m_LoginRequestInfoList.Init();
            m_LoginRttInfoList.Init();
        }

        /// <summary>
        /// Closes a login stream
        /// </summary>
        /// <param name="streamId">Id of the stream to close</param>
        public void CloseStream(int streamId)
        {
            /* find original request information associated with streamId */
            foreach (LoginRequestInfo loginReqInfo in m_LoginRequestInfoList)
            {
                if (loginReqInfo.LoginRequest.StreamId == streamId && loginReqInfo.IsInUse)
                {
                    // clear original request information
                    Console.WriteLine($"Closing login stream id '{loginReqInfo.LoginRequest.StreamId}' with user name: {loginReqInfo.LoginRequest.UserName}");
                    loginReqInfo.Clear();
                    break;
                }
            }
            m_LoginRttInfoList.ClearForStream(streamId);
        }

        /// <summary>
        /// Closes Login stream for ReactorChannel
        /// </summary>
        /// <param name="channel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        public void CloseStream(ReactorChannel channel)
        {
            //find original request information associated with channel
            LoginRequestInfo? loginReqInfo = FindLoginRequestInfo(channel.Channel!);
            if (loginReqInfo != null)
            {
                Console.WriteLine($"Closing login stream id '{loginReqInfo.LoginRequest.StreamId}' with user name: {loginReqInfo.LoginRequest.UserName}");
                loginReqInfo.Clear();
            }
            ClearRttInfo(channel);

        }

        /// <summary>
        /// Sends the login request reject status message for a channel
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="streamId">the id of the login stream</param>
        /// <param name="reason">the reject reason</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendRequestReject(ReactorChannel chnl, int streamId, LoginRejectReason reason, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the login request reject status */
            ITransportBuffer? msgBuf = chnl.GetBuffer(REJECT_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }
            CodecReturnCode ret = EncodeRequestReject(chnl, streamId, reason, msgBuf, errorInfo);
            if (ret != CodecReturnCode.SUCCESS)
            {
                
                return ReactorReturnCode.FAILURE;
            }
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        /// <summary>
        /// Send RTT message to a consumer connected via <paramref name="reactorChannel"/>
        /// </summary>
        /// <param name="reactorChannel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendRTT(ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            if (EnableRtt)
            {
                LoginRttInfo? loginRttInfo = CreateOrGetRttInfo(reactorChannel);
                if (loginRttInfo is not null && IsRttReadyToSend(loginRttInfo))
                {
                    m_LoginRTT = loginRttInfo.LoginRtt;
                    m_EncodeIter.Clear();
                    ITransportBuffer? msgBuf = reactorChannel.GetBuffer(RTT_MSG_SIZE, false, out errorInfo);
                    if (msgBuf == null)
                    {
                        return ReactorReturnCode.FAILURE;
                    }
                    m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);

                    m_LoginRTT.UpdateRTTActualTicks();
                    CodecReturnCode ret = m_LoginRTT.Encode(m_EncodeIter);
                    if (ret != CodecReturnCode.SUCCESS)
                    {
                        errorInfo = new ReactorErrorInfo();
                        errorInfo.Error.Text = $"Failed to encode Login RTT message, return code: {ret.GetAsString()}";
                        return ReactorReturnCode.FAILURE;
                    }
                    m_SubmitOptions.Clear();
                    return reactorChannel.Submit(msgBuf, m_SubmitOptions, out errorInfo);
                }
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Sends the login close status message for a channel.
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendCloseStatus(ReactorChannel chnl, out ReactorErrorInfo? errorInfo)
        {
            LoginRequestInfo? loginReqInfo = m_LoginRequestInfoList.Get(chnl.Channel!);
            if (loginReqInfo == null)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "Could not find login request information for the channel";
                return ReactorReturnCode.FAILURE;
            }

            // get a buffer for the login close
            ITransportBuffer? msgBuf = chnl.GetBuffer(STATUS_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

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
                Console.WriteLine($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                return ReactorReturnCode.FAILURE;
            }
            ret = m_LoginStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"LoginStatus.Encode() failed, return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="loginRequest"><see cref="LoginRequest"/> received from the client</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value that indicates the status of the operation</returns>
        public ReactorReturnCode SendRefresh(ReactorChannel chnl, LoginRequest loginRequest, out ReactorErrorInfo? errorInfo)
        {
            ITransportBuffer? msgBuf = chnl.GetBuffer(REFRESH_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            m_LoginRefresh.Clear();

            // provide login response information 

            // streamId 
            m_LoginRefresh.StreamId = loginRequest.StreamId;

            // username 
            m_LoginRefresh.HasUserName = true;
            m_LoginRefresh.UserName = loginRequest.UserName;

            m_LoginRefresh.HasUserNameType = true;
            m_LoginRefresh.UserNameType = Login.UserIdTypes.NAME;

            m_LoginRefresh.State.Code(StateCodes.NONE);
            m_LoginRefresh.State.DataState(DataStates.OK);
            m_LoginRefresh.State.StreamState(StreamStates.OPEN);
            m_LoginRefresh.State.Text().Data("Login accepted by host localhost");

            m_LoginRefresh.Solicited = true;

            m_LoginRefresh.HasAttrib = true;
            m_LoginRefresh.LoginAttrib.HasApplicationId = true;
            m_LoginRefresh.LoginAttrib.ApplicationId.Data(applicationId);

            m_LoginRefresh.LoginAttrib.HasApplicationName = true;
            m_LoginRefresh.LoginAttrib.ApplicationName.Data(applicationName);

            if (loginRequest.HasAttrib && loginRequest.LoginAttrib.HasPosition)
            {
                m_LoginRefresh.LoginAttrib.HasPosition = true;
                m_LoginRefresh.LoginAttrib.Position = loginRequest.LoginAttrib.Position;
            }

            if (EnableRtt && loginRequest.HasAttrib && loginRequest.LoginAttrib.HasSupportRoundTripLatencyMonitoring)
            {
                m_LoginRefresh.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
            }

            // this provider does not support singleOpen behavior
            m_LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginAttrib.SingleOpen = 0;


            // this provider supports batch requests
            m_LoginRefresh.HasFeatures = true;
            m_LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.SupportedFeatures.SupportBatchRequests = 1;

            //APIQA
            // this provider supports view requests
            if (SupportView)
            {
                m_LoginRefresh.HasFeatures = true;
                m_LoginRefresh.SupportedFeatures.HasSupportViewRequests = true;
                m_LoginRefresh.SupportedFeatures.SupportViewRequests = 1;
            }
            //END APIQA

            m_LoginRefresh.SupportedFeatures.HasSupportPost = true;
            m_LoginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // keep default values for all others
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine($"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}");
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"Failed to set buffer and RWF version, return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            ret = m_LoginRefresh.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"LoginRefresh.Encode() failed, return code: {ret.GetAsString()}";
                return ReactorReturnCode.FAILURE;
            }

            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        /// <summary>
        /// Finds login request information for a channel.
        /// </summary>
        /// <param name="chnl">the <see cref="IChannel"/> instance associated with the current client</param>
        /// <returns><see cref="LoginRequestInfo"/> that corresponds to the current client</returns>
        public LoginRequestInfo? FindLoginRequestInfo(IChannel chnl)
        {
            return m_LoginRequestInfoList.Get(chnl);
        }

        /// <summary>
        /// Gets login request information for the channel.
        /// </summary>
        /// <param name="reactorChannel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="loginRequest"><see cref="LoginRequest"/> received from the current client</param>
        /// <returns><see cref="LoginRequestInfo"/> that corresponds to the current client</returns>
        public LoginRequestInfo? GetLoginRequestInfo(ReactorChannel reactorChannel, LoginRequest loginRequest)
        {
            return m_LoginRequestInfoList.Get(reactorChannel.Channel!, loginRequest);
        }

        /// <summary>
        /// Tries to get <see cref="LoginRttInfo"/> instance that corresponds to the current client, 
        /// creates one if it doesn't exist
        /// </summary>
        /// <param name="reactorChannel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <returns><see cref="LoginRttInfo"/> instance associated with the current client</returns>
        public LoginRttInfo? CreateOrGetRttInfo(ReactorChannel reactorChannel)
        {
            LoginRttInfo? info = GetLoginRtt(reactorChannel);
            return info is null ? CreateLoginRtt(reactorChannel) : info;
        }

        /// <summary>
        /// Creates <see cref="LoginRttInfo"/> instance for the current ReactorChannel
        /// </summary>
        /// <param name="reactorChannel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <returns><see cref="LoginRttInfo"/> instance associated with the current client</returns>
        public LoginRttInfo? CreateLoginRtt(ReactorChannel reactorChannel)
        {
            LoginRequestInfo? loginRequestInfo = FindLoginRequestInfo(reactorChannel.Channel!);
            if ((loginRequestInfo is not null) && loginRequestInfo.IsInUse)
            {
                LoginRttInfo? loginRttInfo = m_LoginRttInfoList.CreateFromRequest(reactorChannel.Channel!, loginRequestInfo.LoginRequest);

                //could be added or deleted also another flags for demonstration different behaviour
                loginRttInfo!.LoginRtt.HasRTLatency = true;
                return loginRttInfo;
            }
            return null;
        }

        /// <summary>
        /// Fetches <see cref="LoginRttInfo"/> instance associated with the current client
        /// </summary>
        /// <param name="reactorChannel">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <returns>
        /// <see cref="LoginRttInfo"/> instance associated with the current client 
        /// (can be null if no such instance yet exists)
        /// </returns>
        public LoginRttInfo? GetLoginRtt(ReactorChannel reactorChannel)
        {
            return m_LoginRttInfoList.Get(reactorChannel.Channel!);
        }

        private bool IsRttReadyToSend(LoginRttInfo loginRttInfo)
        {
            long rttSendTime = DateTime.Now.Ticks * 100;
            if ((rttSendTime - loginRttInfo.RttLastSendNanoTime) / 1000000000 > RTT_NOTIFICATION_INTERVAL)
            {
                loginRttInfo.RttLastSendNanoTime = rttSendTime;
                return true;
            }
            return false;
        }

        private void ClearRttInfo(ReactorChannel reactorChannel)
        {
            if (EnableRtt)
            {
                LoginRttInfo? loginRttInfo = m_LoginRttInfoList.Get(reactorChannel.Channel!);
                if (loginRttInfo is not null)
                {
                    loginRttInfo.Clear();
                }
            }
        }

        /// <summary>
        /// Encodes the login request reject status. 
        /// Returns success if encoding succeeds or failure if encoding fails.
        /// </summary>
        /// <param name="chnl">the <see cref="ReactorChannel"/> instance asociated with the current client</param>
        /// <param name="streamId">the id of the login stream</param>
        /// <param name="reason">the reject reason</param>
        /// <param name="msgBuf"><see cref="ITransportBuffer"/> with reject message</param>
        /// <param name="errorInfo"><see cref="ReactorErrorInfo"/> instance that contains error information in case of failure</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation</returns>
        private CodecReturnCode EncodeRequestReject(ReactorChannel chnl, int streamId, LoginRejectReason reason, ITransportBuffer msgBuf, ReactorErrorInfo? errorInfo)
        {
            // set-up message 
            m_LoginStatus.StreamId = streamId;
            m_LoginStatus.HasState = true;
            m_LoginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_LoginStatus.State.DataState(DataStates.SUSPECT);
            switch (reason)
            {
                case LoginRejectReason.MAX_LOGIN_REQUESTS_REACHED:
                    m_LoginStatus.State.Code(StateCodes.TOO_MANY_ITEMS);
                    m_LoginStatus.State.Text().Data($"Login request rejected for stream id {streamId} - max request count reached");
                    break;
                case LoginRejectReason.NO_USER_NAME_IN_REQUEST:
                    m_LoginStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_LoginStatus.State.Text().Data($"Login request rejected for stream id  {streamId} - request does not contain user name");
                    break;
                case LoginRejectReason.LOGIN_RDM_DECODER_FAILED:
                    m_LoginStatus.State.Code(StateCodes.USAGE_ERROR);
                    m_LoginStatus.State.Text().Data($"Login request rejected for stream id  {streamId} - decoding failure: {errorInfo?.Error.Text}");
                    break;
                default:
                    break;
            }

            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIterator.SetBufferAndRWFVersion() failed with return code: {ret.GetAsString()}";
                return ret;
            }

            ret = m_LoginStatus.Encode(m_EncodeIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = "LoginStatus.Encode() failed";
                return ret;
            }

            return CodecReturnCode.SUCCESS;
        }

       
    }
}
