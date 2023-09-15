/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Net;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.PerfTools.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Eta.Rdm.Login;

namespace LSEG.Eta.PerfTools.ProvPerf
{
    /// <summary>
    /// Determines the information a provider needs for accepting logins,
    /// and provides encoding of appropriate responses.
    /// </summary>
    public class LoginProvider
    {
        private const int REFRESH_MSG_SIZE = 512;

        private LoginRefresh m_LoginRefresh;
        private LoginRequest m_LoginRequest;

        public LoginRequest LoginRequest { get => m_LoginRequest; }

        private EncodeIterator m_EncodeIter;
        public string? ApplicationId { get; set; }
        public string? ApplicationName { get; set; }
        public string? Position { get; set; }

        // Use the VA Reactor instead of the ETA Channel for sending and receiving
        private ReactorSubmitOptions  m_ReactorSubmitOptions;

        public LoginProvider()
        {
            m_LoginRefresh = new LoginRefresh();
            m_LoginRequest = new LoginRequest();
            m_EncodeIter = new EncodeIterator();

            m_ReactorSubmitOptions = new();
            m_ReactorSubmitOptions.Clear();
            m_ReactorSubmitOptions.WriteArgs.Priority = WritePriorities.HIGH;
        }

        /// <summary>
        /// Processes a login request
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="msg">the partially decoded message</param>
        /// <param name="dIter">the decode iterator</param>
        /// <param name="error">Error information in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value that indicates the status of the operation</returns>
        public TransportReturnCode ProcessMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Msg msg, DecodeIterator dIter, out Error? error)
        {
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:

                    // decode login request
                    m_LoginRequest.Clear();
                    CodecReturnCode codecReturnCode = m_LoginRequest.Decode(dIter, msg);
                    if (codecReturnCode != CodecReturnCode.SUCCESS)
                    {
                        error = new Error()
                        {
                            Text = $"LoginRequest.Decode() failed with return code:  {codecReturnCode.GetAsString()}"
                        };
                        return TransportReturnCode.FAILURE;
                    }
                    //send login response
                    return SendRefresh(channelHandler, clientChannelInfo, out error);
                case MsgClasses.CLOSE:
                    Console.WriteLine($"Received Login Close for streamId {msg.StreamId}");
                    break;

                default:
                    error = new Error()
                    {
                        Text = $"Received Unhandled Login Msg Class: {msg.MsgClass}"
                    };
                    return TransportReturnCode.FAILURE;
            }

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Initializes default DACS position for login message
        /// </summary>
        public void InitDefaultPosition()
        {
            // Position
            try
            {
                string hostName = Dns.GetHostName();
                Position = Dns.GetHostAddresses(hostName)
                    .Where(ip => ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                    .FirstOrDefault()?.ToString()
                    + "/" + hostName;
            }
            catch (Exception)
            {
                Position = "1.1.1.1/net";
            }
        }

        /// <summary>
        /// Encodes and sends login refresh
        /// </summary>
        /// <param name="channelHandler">the channel handler</param>
        /// <param name="clientChannelInfo">the client channel info</param>
        /// <param name="error">Error information in case of failure</param>
        /// <returns><see cref="PerfToolsReturnCode"/> value that indicates the status of the operation</returns>
        private TransportReturnCode SendRefresh(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, out Error error)
        {
            // initialize login response info
            m_LoginRefresh.Clear();

            IChannel channel = clientChannelInfo.Channel!;

            // get a buffer for the login response
            ITransportBuffer msgBuf = channel.GetBuffer(REFRESH_MSG_SIZE, false, out error);
            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            // provide login response information

            // streamId
            m_LoginRefresh.StreamId = m_LoginRequest.StreamId;

            // username
            m_LoginRefresh.HasUserName = true;
            m_LoginRefresh.UserName = m_LoginRequest.UserName;

            m_LoginRefresh.HasUserNameType = true;
            m_LoginRefresh.UserNameType = UserIdTypes.NAME;

            m_LoginRefresh.State.Code(StateCodes.NONE);
            m_LoginRefresh.State.DataState(DataStates.OK);
            m_LoginRefresh.State.StreamState(StreamStates.OPEN);
            m_LoginRefresh.State.Text().Data("Login accepted by host localhost");

            m_LoginRefresh.Solicited = true;

            m_LoginRefresh.HasAttrib = true;
            m_LoginRefresh.LoginAttrib.HasApplicationId = true;
            m_LoginRefresh.LoginAttrib.ApplicationId.Data(ApplicationId);
            m_LoginRefresh.LoginAttrib.HasApplicationName = true;
            m_LoginRefresh.LoginAttrib.ApplicationName.Data(ApplicationName);

            if (m_LoginRequest.HasAttrib && m_LoginRequest.LoginAttrib.HasPosition)
            {
                m_LoginRefresh.LoginAttrib.HasPosition = true;
                m_LoginRefresh.LoginAttrib.Position = m_LoginRequest.LoginAttrib.Position;
            }

            // this provider does not support
            // singleOpen behavior
            m_LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginAttrib.SingleOpen = 0;

            // this provider supports batch requests
            m_LoginRefresh.HasFeatures = true;
            m_LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.SupportedFeatures.SupportBatchRequests = 1;

            m_LoginRefresh.SupportedFeatures.HasSupportPost = true;
            m_LoginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // keep default values for all others

            // encode login response
            m_EncodeIter.Clear();
            CodecReturnCode codecReturnCode = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            codecReturnCode = m_LoginRefresh.Encode(m_EncodeIter);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"LoginRefresh.Encode() failed with return code: {codecReturnCode.GetAsString()}"
                };
                return TransportReturnCode.FAILURE;
            }

            //send login response
            return channelHandler.WriteChannel(clientChannelInfo, msgBuf, 0, out error);
        }

        internal ReactorReturnCode SendRefreshReactor(ClientChannelInfo clientChannelInfo, out ReactorErrorInfo? errorInfo)
        {
            // initialize login response info
            m_LoginRefresh.Clear();

            ReactorChannel reactorChannel = clientChannelInfo.ReactorChannel!;

            // get a buffer for the login response
            ITransportBuffer? msgBuf = reactorChannel.GetBuffer(REFRESH_MSG_SIZE, false, out errorInfo);

            if (msgBuf == null)
            {
                return ReactorReturnCode.FAILURE;
            }

            // provide login response information

            // StreamId
            m_LoginRefresh.StreamId = m_LoginRequest.StreamId;

            // Username
            m_LoginRefresh.HasUserName = true;
            m_LoginRefresh.UserName.Data(m_LoginRequest.UserName.Data(), m_LoginRequest.UserName.Position, m_LoginRequest.UserName.Length);

            m_LoginRefresh.HasUserNameType = true;
            m_LoginRefresh.UserNameType = UserIdTypes.NAME;

            m_LoginRefresh.State.Code(StateCodes.NONE);
            m_LoginRefresh.State.DataState(DataStates.OK);
            m_LoginRefresh.State.StreamState(StreamStates.OPEN);
            m_LoginRefresh.State.Text().Data("Login accepted by host localhost");

            m_LoginRefresh.Solicited = true;

            m_LoginRefresh.HasAttrib = true;

            // ApplicationId
            m_LoginRefresh.LoginAttrib.HasApplicationId = true;
            m_LoginRefresh.LoginAttrib.ApplicationId.Data(ApplicationId);

            // ApplicationName
            m_LoginRefresh.LoginAttrib.HasApplicationName = true;
            m_LoginRefresh.LoginAttrib.ApplicationName.Data(ApplicationName);

            // Position
            m_LoginRefresh.LoginAttrib.HasPosition = true;
            m_LoginRefresh.LoginAttrib.Position.Data(Position);

            //
            // this provider does not support
            // SingleOpen behavior
            //
            m_LoginRefresh.LoginAttrib.HasSingleOpen = true;
            m_LoginRefresh.LoginAttrib.SingleOpen = 0;

            //
            // this provider supports
            // batch requests
            //
            m_LoginRefresh.HasFeatures = true;
            m_LoginRefresh.SupportedFeatures.HasSupportBatchRequests = true;
            m_LoginRefresh.SupportedFeatures.SupportBatchRequests = 1;

            m_LoginRefresh.SupportedFeatures.HasSupportPost = true;
            m_LoginRefresh.SupportedFeatures.SupportOMMPost = 1;

            // keep default values for all others

            // encode login response
            m_EncodeIter.Clear();
            CodecReturnCode codeRet = m_EncodeIter.SetBufferAndRWFVersion(msgBuf, reactorChannel.MajorVersion, reactorChannel.MinorVersion);
            if (codeRet != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"EncodeIter.SetBufferAndRWFVersion() failed with return code: {codeRet}";
                errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                return ReactorReturnCode.FAILURE;
            }

            codeRet = m_LoginRefresh.Encode(m_EncodeIter);
            if (codeRet != CodecReturnCode.SUCCESS)
            {
                errorInfo = new ReactorErrorInfo();
                errorInfo.Error.Text = $"LoginRefresh.Encode() failed with return code: {codeRet}";
                errorInfo.Error.ErrorId = TransportReturnCode.FAILURE;
                return ReactorReturnCode.FAILURE;
            }

            //send login response
            return reactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);
        }
    }
}
