/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Rdm;
using System;
using static Refinitiv.Eta.Rdm.Login;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// This is the Login handler for the ETA ConsPerf and NIProvPerf applications. 
    /// It provides methods for encoding and sending of login request, as well as 
    /// processing of responses (refresh, status, update, close). Methods are also provided 
    /// to allow setting of application name, user name and role, to be used in the login request.
    /// </summary>
    public class LoginHandler
    {
        public const int LOGIN_STREAM_ID = 1;

        public const int MAX_MSG_SIZE = 1024;
        public const int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
        public const int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

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

        private EncodeIterator encIter = new EncodeIterator();

        public LoginHandler() { }

        /// <summary>
        /// Returns buffer with Login request encoded
        /// </summary>
        /// <param name="chnl">channel the request will be sent to</param>
        /// <param name="error"><see cref="Error"/> instance that contains the detailed information about the error</param>
        /// <returns><see cref="ITransportBuffer"/> instance that contains Login request</returns>
        public ITransportBuffer? GetRequest(IChannel chnl, out Error? error)
        {
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out error);
            if (msgBuf == null)
            {
                return null;
            }               

            LoginRequest.Clear();
            LoginRequest.InitDefaultRequest(LOGIN_STREAM_ID);

            if (UserName != null && !UserName.Equals(string.Empty))
            {
                LoginRequest.UserName.Data(UserName);
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

            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode ret = LoginRequest.Encode(encIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = $"Encoding of login request failed: <{ret.GetAsString()}>\n"
                };
                return null;
            }

            Console.WriteLine(LoginRequest.ToString());

            error = null;
            return msgBuf;
        }

        /// <summary>
        /// Close the login stream. Note that closing login stream 
        /// will automatically close all other streams at the provider.
        /// </summary>
        /// <param name="chnl">The channel to send a login close to</param>
        /// <param name="error"><see cref="Error"/> instance that holds error information in case of failure</param>
        /// <returns><see cref="TransportReturnCode"/> value</returns>
        public ITransportBuffer? GetCloseMsg(IChannel chnl, out Error error)
        {
            ITransportBuffer msgBuf = chnl.GetBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, out error);

            if (msgBuf == null)
            {
                return null;
            }

            LoginClose.Clear();
            LoginClose.StreamId = LOGIN_STREAM_ID;
            encIter.Clear();
            encIter.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);

            CodecReturnCode encRet = LoginClose.Encode(encIter);
            if (encRet != CodecReturnCode.SUCCESS)
            {
                error = new Error()
                {
                    Text = "Encoding of login close failed: <" + encRet.GetAsString() + ">\n"
                };
                return null;
            }

            return msgBuf;
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
                case MsgClasses.CLOSE:
                    Console.WriteLine("Received Login Close");
                    LoginState = ConsumerLoginState.CLOSED;
                    return TransportReturnCode.FAILURE;
                default:
                    error = new Error()
                    {
                        Text = $"Received Unhandled Login Msg Class: {msg.MsgClass}"
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
                    Text = $"Decoding of login status failed: <{ret.GetAsString()}"
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
            Console.WriteLine($"	{state}");

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
                    Text = $"Decoding of login refresh failed: {ret.GetAsString()}\n"
                };
                return TransportReturnCode.FAILURE;
            }
            Console.WriteLine($"Received Login Response for Username: {LoginRefresh.UserName}");
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

    }
}
