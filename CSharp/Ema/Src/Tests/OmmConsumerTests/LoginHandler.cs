/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class LoginHandler
    {
        ProviderSessionOptions m_ProviderSessionOptions;

        private const int REJECT_MSG_SIZE = 512;
        private const int REFRESH_MSG_SIZE = 512;
        private const int STATUS_MSG_SIZE = 512;
        private const int RTT_MSG_SIZE = 1024;

        private const int RTT_NOTIFICATION_INTERVAL = 5;

        private LoginStatus m_LoginStatus = new LoginStatus();
        private LoginRefresh m_LoginRefresh = new LoginRefresh();
        private LoginRequest m_LoginRequest = new LoginRequest();
        private EncodeIterator m_EncodeIter = new EncodeIterator();

        private static string applicationId = "256";
        private static string applicationName = "ETA Provider Test";

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        public LoginHandler(ProviderTest providerTest)
        {
            m_ProviderSessionOptions = providerTest.ProviderSessionOptions;
        }

        public LoginHandler(ProviderSessionOptions providerSessionOptions)
        {
            m_ProviderSessionOptions = providerSessionOptions;
        }

        public ReactorReturnCode HandleLoginMsgEvent(RDMLoginMsgEvent loginMsgEvent)
        {
            LoginMsg? loginMsg = loginMsgEvent.LoginMsg;
            ReactorChannel? reactorChannel = loginMsgEvent.ReactorChannel;

            if (loginMsg is null)
            {
                return ReactorReturnCode.FAILURE;
            }

            switch (loginMsg.LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    {
                        LoginRequest loginRequest = loginMsg.LoginRequest!;

                        if(m_ProviderSessionOptions.SendLoginReject)
                        {
                            if (SendRequestReject(reactorChannel!, loginRequest.StreamId, "Force logout by Provider", out _) != ReactorReturnCode.SUCCESS)
                            {
                                return ReactorReturnCode.FAILURE;
                            }

                            break;
                        }

                        if (SendRefresh(reactorChannel!, loginRequest, out _) != ReactorReturnCode.SUCCESS)
                        {
                            return ReactorReturnCode.FAILURE;
                        }
                        break;
                    }
                case LoginMsgType.REFRESH:
                case LoginMsgType.CLOSE:
                    break;
                case LoginMsgType.RTT:
                default:
                    Assert.Fail($"Unsupported login message type {loginMsg.LoginMsgType}");
                    break;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendRefresh(ReactorChannel reactorChannel, LoginRequest loginRequest, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(REFRESH_MSG_SIZE, false, out errorInfo);
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

            // this provider does not support singleOpen behavior
            m_LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginAttrib.SingleOpen = 0;


            // this provider supports batch requests
            m_LoginRefresh.HasFeatures = true;
            m_LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.SupportedFeatures.SupportBatchRequests = 1;

            m_LoginRefresh.SupportedFeatures.HasSupportPost = true;
            m_LoginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // keep default values for all others
            m_EncodeIter.Clear();
            CodecReturnCode ret = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
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

            return reactorChannel.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        public ReactorReturnCode SendRequestReject(ReactorChannel chnl, int streamId, string statusText, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the login request reject status */
            ITransportBuffer? msgBuf = chnl.GetBuffer(REJECT_MSG_SIZE, false, out errorInfo);
            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }
            CodecReturnCode ret = EncodeRequestReject(chnl, streamId, statusText, msgBuf, errorInfo);
            if (ret != CodecReturnCode.SUCCESS)
            {

                return ReactorReturnCode.FAILURE;
            }
            return chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo);
        }

        private CodecReturnCode EncodeRequestReject(ReactorChannel chnl, int streamId, string statusText, ITransportBuffer msgBuf, ReactorErrorInfo? errorInfo)
        {
            // set-up message 
            m_LoginStatus.StreamId = streamId;
            m_LoginStatus.HasState = true;
            m_LoginStatus.State.StreamState(StreamStates.CLOSED);
            m_LoginStatus.State.DataState(DataStates.SUSPECT);
            m_LoginStatus.State.Code(StateCodes.USAGE_ERROR);
            m_LoginStatus.State.Text().Data($"Login request rejected for stream id {streamId} - {statusText}");

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
