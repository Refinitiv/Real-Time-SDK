/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using static LSEG.Eta.Rdm.Login;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// This is the Login handler for the ETA Consumer and NIProvider applications. 
    /// It provides methods for encoding and sending of login request, as well as 
    /// processing of responses (refresh, status, update, close). Methods are also provided 
    /// to allow setting of application name, user name and role, to be used in the login request.
    /// </summary>
    public class LoginHandler
    {
        public const int LOGIN_STREAM_ID = 1;

        public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
        public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;
        public static int TRANSPORT_BUFFER_SIZE_RTT = ChannelSession.MAX_MSG_SIZE;

        public ConsumerLoginState LoginState { get; set; } = ConsumerLoginState.PENDING_LOGIN;
        public string? UserName { get; set; }
        public string? ApplicationName { get; set; }
        public string? AuthenticationToken { get; set; }
        public string? AuthenticationExtended { get; set; }
        public string? ApplicationId { get; set; }
        public bool EnableRtt { get; set; }

        public int Role { get; set; } = RoleTypes.CONS;

        public LoginRequest LoginRequest { get; set; } = new LoginRequest();
        public LoginClose LoginClose { get; set; } = new LoginClose();
        public LoginRefresh LoginRefresh { get; set; } = new LoginRefresh();
        public LoginStatus LoginStatus { get; set; } = new LoginStatus();
        public LoginRTT LoginRtt { get; set; } = new LoginRTT();

        private EncodeIterator encIter = new EncodeIterator();

        public LoginHandler() {}

        /// <summary>
        /// Sends a login request to a channel. 
        /// This consists of getting a message buffer, setting the login request information, 
        /// encoding the login request, and sending the login request to the server.
        /// </summary>
        /// <param name="chnl">The channel to send a login request to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode SendRequest(ChannelSession chnl, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
                return TransportReturnCode.FAILURE;

            LoginRequest.Clear();
            LoginRequest.InitDefaultRequest(LOGIN_STREAM_ID);

            if (UserName != null && !UserName.Equals(string.Empty))
            {
                LoginRequest.UserName.Data(UserName);
            }

            if (AuthenticationToken != null && !AuthenticationToken.Equals(string.Empty))
            {
                LoginRequest.HasUserNameType = true;
                LoginRequest.UserNameType = UserIdTypes.TOKEN;
                LoginRequest.UserName.Data(AuthenticationToken);

                if (AuthenticationExtended != null && !AuthenticationExtended.Equals(string.Empty))
                {
                    LoginRequest.HasAuthenticationExtended = true;
                    LoginRequest.AuthenticationExtended.Data(AuthenticationExtended);
                }
            }

            if (ApplicationId != null && !ApplicationId.Equals(string.Empty))
            {
                LoginRequest.HasAttrib = true;
                LoginRequest.LoginAttrib.HasApplicationId = true;
                LoginRequest.LoginAttrib.ApplicationId.Data(ApplicationId);
            }

            if (ApplicationName != null && !ApplicationName.Equals(string.Empty))
            {
                LoginRequest.HasAttrib = true;
                LoginRequest.LoginAttrib.HasApplicationName = true;
                LoginRequest.LoginAttrib.ApplicationName.Data(ApplicationName);
            }

            LoginRequest.HasRole = true;
            LoginRequest.Role = Role;

            if (Role == RoleTypes.PROV)
            {
                LoginRequest.LoginAttrib.HasSingleOpen = true;
                LoginRequest.LoginAttrib.SingleOpen = 0;
            }
            
            if (EnableRtt && Role == RoleTypes.CONS)
            {
                LoginRequest.LoginAttrib.HasSupportRoundTripLatencyMonitoring = true;
            }

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode ret = LoginRequest.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encoding of login request failed: <" + ret + ">\n"
                };
                return TransportReturnCode.FAILURE;
            }
        
            Console.WriteLine(LoginRequest.ToString());
            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Sends RTT message to a channel if RTT feature had been enabled.
        /// </summary>
        /// <param name="channelSession">The channel to send RTT message to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode SendRttMessage(ChannelSession channelSession, out Error? error)
        {
            if (!EnableRtt)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }

            IChannel channel = channelSession.Channel!;
            ITransportBuffer? transportBuffer = channelSession.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_RTT, false, out error);
            if (transportBuffer == null)
            {
                return TransportReturnCode.FAILURE;
            }

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(transportBuffer, channel.MajorVersion, channel.MinorVersion);

            CodecReturnCode ret = LoginRtt.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encoding of login RTT failed: <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode wret = channelSession.Write(transportBuffer, out error);
            if (wret == TransportReturnCode.SUCCESS)
            {
                LogRttMessageSending(channelSession.SocketFdValue);
            }
            return wret;
        }

        /// <summary>
        /// Close the login stream. Note that closing login stream 
        /// will automatically close all other streams at the provider.
        /// </summary>
        /// <param name="chnl">The channel to send a login close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode CloseStream(ChannelSession chnl, out Error? error)
        {
            ITransportBuffer? msgBuf = chnl.GetTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);

            if (msgBuf == null)
            {
                return TransportReturnCode.FAILURE;
            }

            LoginClose.Clear();
            LoginClose.StreamId = LOGIN_STREAM_ID;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.Channel!.MajorVersion, chnl.Channel.MinorVersion);

            CodecReturnCode encRet = LoginClose.Encode(encIter);
            if (encRet != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encoding of login close failed: <" + encRet + ">\n"
                };
                return TransportReturnCode.FAILURE;
            }

            return chnl.Write(msgBuf, out error);
        }

        /// <summary>
        /// Processes login response. 
        /// This consists of looking at the msg class and decoding message into corresponding RDM login message. 
        /// For every login status and login refresh, it updates login states. 
        /// Query methods are provided to query these login states.
        /// </summary>
        /// <param name="msg">The partially decoded message</param>
        /// <param name="dIter">The decode iterator</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public TransportReturnCode ProcessResponse(Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return HandleLoginRefresh(msg, dIter, out error);
                case MsgClasses.STATUS:
                    return HandleLoginStatus(msg, dIter, out error);
                case MsgClasses.UPDATE:
                    Console.WriteLine("Received Login Update");
                    return TransportReturnCode.SUCCESS;
                case MsgClasses.GENERIC:
                    return HandleLoginRtt(msg, dIter, out error);
                case MsgClasses.CLOSE:
                    Console.WriteLine("Received Login Close");
                    LoginState = ConsumerLoginState.CLOSED;
                    error = new Error()
                    {
                        Text = "Received Login Close message",
                        ErrorId = TransportReturnCode.FAILURE
                    };
                    return TransportReturnCode.FAILURE;
                default:
                    error = new Error()
                    {
                        Text = "Received Unhandled Login Msg Class: " + msg.MsgClass
                    };                    
                    return TransportReturnCode.FAILURE;
            }
        }

        private TransportReturnCode HandleLoginStatus(Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            LoginStatus.Clear();
            CodecReturnCode ret = LoginStatus.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Decoding of login status failed: <" + ret + ">"
                };
                return TransportReturnCode.FAILURE;
            }

            Console.WriteLine("Received Login StatusMsg");
            if (!LoginStatus.HasState)
            {
                error = null;
                return TransportReturnCode.SUCCESS;
            }               

            State state = LoginStatus.State;
            Console.WriteLine("	" + state);

            if (state.StreamState() == StreamStates.CLOSED_RECOVER)
            {
                error = new Error()
                {
                    Text = "Login stream is closed recover"
                };
                LoginState = ConsumerLoginState.CLOSED_RECOVERABLE;
            }
            else if (state.StreamState() == StreamStates.CLOSED)
            {
                error = new Error()
                {
                    Text = "Login stream closed"
                };
                LoginState = ConsumerLoginState.CLOSED;
            }
            else if (state.StreamState() == StreamStates.OPEN
                    && state.DataState() == DataStates.SUSPECT)
            {
                error = new Error()
                {
                    Text = "Login stream is suspect"
                };
                LoginState = ConsumerLoginState.SUSPECT;
            }

            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode HandleLoginRefresh(Msg msg, DecodeIterator dIter, out Error? error)
        {
            error = null;
            LoginRefresh.Clear();
            CodecReturnCode ret = LoginRefresh.Decode(dIter, msg);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Decoding of login refresh failed: <" + ret + ">\n"
                };               
                return TransportReturnCode.FAILURE;
            }
            Console.WriteLine("Received Login Response for Username: " + LoginRefresh.UserName);
            Console.WriteLine(LoginRefresh.ToString());

            State state = LoginRefresh.State;
            if (state.StreamState() == StreamStates.OPEN)
            {
                if (state.DataState() == DataStates.OK)
                    LoginState = ConsumerLoginState.OK_SOLICITED;
                else if (state.DataState() == DataStates.SUSPECT)
                    LoginState = ConsumerLoginState.SUSPECT;
            }
            else if (state.StreamState() == StreamStates.CLOSED_RECOVER)
            {
                LoginState = ConsumerLoginState.CLOSED_RECOVERABLE;
            }
            else if (state.StreamState() == StreamStates.CLOSED)
            {
                LoginState = ConsumerLoginState.CLOSED;
            }
            else
            {
                LoginState = ConsumerLoginState.SUSPECT;
            }
          
            return TransportReturnCode.SUCCESS;
        }

        private TransportReturnCode HandleLoginRtt(Msg msg, DecodeIterator decodeIterator, out Error? error)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            if (EnableRtt)
            {
                LoginRtt.Clear();
                ret = LoginRtt.Decode(decodeIterator, msg);
                if (ret != CodecReturnCode.SUCCESS)
                {
                    error = new Error()
                    {
                        Text = "Decoding of login RTT failed: <" + ret + ">"
                    };
                    return TransportReturnCode.FAILURE;

                }
            }
            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private void LogRttMessageSending(long socketFdValue)
        {
            Console.WriteLine("\nReceived login RTT message from Provider " + socketFdValue + ".");
            Console.WriteLine("\tTicks: " + (LoginRtt.Ticks / 1000L));
            if (LoginRtt.HasRTLatency)
            {
                Console.WriteLine("\tLast Latency: " + LoginRtt.RTLatency);
            }
            if (LoginRtt.HasTCPRetrans)
            {
                Console.WriteLine("\tProvider side TCP Retransmissions: " + LoginRtt.TCPRetrans);
            }
        } 
    }
}
