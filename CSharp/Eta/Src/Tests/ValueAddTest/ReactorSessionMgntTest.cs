/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using Xunit.Categories;
using LSEG.Eta.ValueAdd.Reactor;
using System.Net.Sockets;
using System.Collections.Generic;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Codec;
using System;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class ReactorSessionMgntTest : IReactorChannelEventCallback, IDefaultMsgCallback, 
        IReactorAuthTokenEventCallback, IReactorOAuthCredentialEventCallback, IRDMLoginMsgCallback
    {
        private string CLIENT_ID;
        private string CLIENT_ID_JWT;
        private string CLIENT_SECRET;
        private string CLIENT_JWK;
        private string TOKEN_SERVICE_URL;
        private string SERVICE_DISCOVERY_URL;
        private ReactorAuthTokenInfo reactorAuthTokenInfo = new ReactorAuthTokenInfo();
        private static readonly string TOKEN_TYPE = "Bearer";
        private const int EXPIRES_IN = 7199;
        private ReactorChannel reactorChannel;
        private List<Socket> readSockets = new List<Socket>();
        private State expectedState = new State();
        private List<ReactorChannelEventType> reactorChannelEventTypes = new List<ReactorChannelEventType>();
        private List<LoginMsgType> loginMsgTypes = new List<LoginMsgType>();
        
        private void Clear()
        {
            reactorChannelEventTypes.Clear();
            reactorAuthTokenInfo.Clear();
            readSockets.Clear();
            expectedState.Clear();
            loginMsgTypes.Clear();
        }

        public ReactorSessionMgntTest()
        {
            try
            {
                CLIENT_ID = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID", System.EnvironmentVariableTarget.Process);
                CLIENT_ID_JWT = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID_JWT", System.EnvironmentVariableTarget.Process);
                CLIENT_SECRET = Environment.GetEnvironmentVariable("ETANET_CLIENT_SECRET", System.EnvironmentVariableTarget.Process);
                CLIENT_JWK = Environment.GetEnvironmentVariable("ETANET_CLIENT_JWK", System.EnvironmentVariableTarget.Process);
                TOKEN_SERVICE_URL = Environment.GetEnvironmentVariable("ETANET_TOKEN_SERVICE_URL", System.EnvironmentVariableTarget.Process);
                SERVICE_DISCOVERY_URL = Environment.GetEnvironmentVariable("ETANET_SERVICE_DISCOVERY_URL", System.EnvironmentVariableTarget.Process);
            }
            catch(Exception)
            {
                Assert.True(false, "Failed to get OAuth credential from process environment");
            }

            Clear();
        }

        void ConnectSuccessWithOneConnection_usingDefaultLocation(bool isPingJwt, bool overrideTokenServiceURL, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if(overrideTokenServiceURL && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if(overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();
            connectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            connectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo.EnableSessionManagement = true;
            connectInfo.ReactorAuthTokenEventCallback = this;

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(connectInfo);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            if (isPingJwt == false)
            {
                credential.ClientSecret.Data(CLIENT_SECRET);
                credential.ClientId.Data(CLIENT_ID);
            }

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.ReactorOAuthCredential = credential;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;
            consumerRole.LoginMsgCallback = this;

            expectedState.StreamState(StreamStates.OPEN);
            expectedState.DataState(DataStates.OK);
            expectedState.Code(StateCodes.NONE);

            Buffer text = new Buffer();
            text.Data("Login accepted by host ads-fanout");
            expectedState.Text(text);

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);

            Assert.True(!string.IsNullOrEmpty(reactorAuthTokenInfo.AccessToken));
            Assert.Equal(TOKEN_TYPE, reactorAuthTokenInfo.TokenType);
            Assert.Equal(EXPIRES_IN, reactorAuthTokenInfo.ExpiresIn);

            Assert.Equal(ReactorReturnCode.SUCCESS, returnCode);

            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while(reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, reactorChannelEventTypes[0]);

            readSockets.Clear();
            readSockets.Add(reactorChannel.Socket);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);
            while (reactorChannel.Socket.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(LoginMsgType.REFRESH, loginMsgTypes[0]);
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, reactorChannelEventTypes[1]);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactorChannel.Close(out errorInfo));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ConnectSuccessWithOneConnection_usingDefaultLocationTest()
        {
            ConnectSuccessWithOneConnection_usingDefaultLocation(false, false, false);
        }

        void ConnectSuccessWithOneConnection_SpecifyLocation(bool isPingJwt, bool overrideTokenServiceURL, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if (overrideTokenServiceURL && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();
            connectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            connectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo.EnableSessionManagement = true;
            connectInfo.ReactorAuthTokenEventCallback = this;
            connectInfo.Location = "us-east-2"; // Specify a location

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(connectInfo);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            if (isPingJwt == false)
            {
                credential.ClientSecret.Data(CLIENT_SECRET);
                credential.ClientId.Data(CLIENT_ID);
            }

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.ReactorOAuthCredential = credential;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;
            consumerRole.RdmLoginRequest = null;

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);

            Assert.True(!string.IsNullOrEmpty(reactorAuthTokenInfo.AccessToken));
            Assert.Equal(TOKEN_TYPE, reactorAuthTokenInfo.TokenType);
            Assert.Equal(EXPIRES_IN, reactorAuthTokenInfo.ExpiresIn);

            Assert.Equal(ReactorReturnCode.SUCCESS, returnCode);

            while (reactorEventFD.Poll(10 * 1000 * 1000, SelectMode.SelectRead) != true) ;

            Assert.True(reactorEventFD.Available > 0);
            reactor.Dispatch(dispatchOpts, out errorInfo);
            Assert.True(reactorEventFD.Available == 0);

            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, reactorChannelEventTypes[0]);
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, reactorChannelEventTypes[1]);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactorChannel.Close(out errorInfo));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ConnectSuccessWithOneConnection_SpecifyLocationTest()
        {
            ConnectSuccessWithOneConnection_SpecifyLocation(false, false, false);
        }

        void ConnectSuccessWithOneConnection_InvalidLocation(bool isPingJwt, bool overrideTokenServiceURL, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if (overrideTokenServiceURL && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();
            connectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            connectInfo.EnableSessionManagement = true;
            connectInfo.ReactorAuthTokenEventCallback = this;
            connectInfo.Location = "mars-east-1"; // Specify a location

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(connectInfo);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            if (isPingJwt == false)
            {
                credential.ClientSecret.Data(CLIENT_SECRET);
                credential.ClientId.Data(CLIENT_ID);
            }

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.ReactorOAuthCredential = credential;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);

            Assert.Equal(ReactorReturnCode.PARAMETER_INVALID, returnCode);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ConnectSuccessWithOneConnection_InvalidLocationTest()
        {
            ConnectSuccessWithOneConnection_InvalidLocation(false, false, false);
        }

        void ConnectionList_FirstFailed_SecondRDPConnection(bool isPingJwt, bool overrideTokenServiceURL, bool overrideDiscoveryUrl)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if (overrideTokenServiceURL && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryUrl && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            ReactorConnectInfo firstConnectInfo = new ReactorConnectInfo();

            /* Expect to fails for the first connection */
            firstConnectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.SOCKET;
            firstConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            firstConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            firstConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            firstConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "5555";

            /* Establish a connection with RTO */
            ReactorConnectInfo secondConnectInfo = new ReactorConnectInfo();
            secondConnectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            secondConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            secondConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            secondConnectInfo.EnableSessionManagement = true;
            secondConnectInfo.ReactorAuthTokenEventCallback = this;

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(firstConnectInfo);
            connectOptions.ConnectionList.Add(secondConnectInfo);
            connectOptions.SetReconnectAttempLimit(-1);
            connectOptions.SetReconnectMaxDelay(15000);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            if (isPingJwt == false)
            {
                credential.ClientSecret.Data(CLIENT_SECRET);
                credential.ClientId.Data(CLIENT_ID);
            }

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.ReactorOAuthCredential = credential;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;
            consumerRole.LoginMsgCallback = this;

            expectedState.StreamState(StreamStates.OPEN);
            expectedState.DataState(DataStates.OK);
            expectedState.Code(StateCodes.NONE);

            Buffer text = new Buffer();
            text.Data("Login accepted by host ads-fanout");
            expectedState.Text(text);

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);

            Assert.Equal(ReactorReturnCode.SUCCESS, returnCode);

            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, reactorChannelEventTypes[0]);

            readSockets.Clear();
            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            /* Receives access token information */
            Assert.True(!string.IsNullOrEmpty(reactorAuthTokenInfo.AccessToken));
            Assert.Equal(TOKEN_TYPE, reactorAuthTokenInfo.TokenType);
            Assert.Equal(EXPIRES_IN, reactorAuthTokenInfo.ExpiresIn);

            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, reactorChannelEventTypes[1]);

            readSockets.Clear();
            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            readSockets.Clear();
            readSockets.Add(reactorEventFD);
            readSockets.Add(reactorChannel.Socket);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0 || reactorChannel.Socket.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(LoginMsgType.REFRESH, loginMsgTypes[0]);

            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, reactorChannelEventTypes[2]);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactorChannel.Close(out errorInfo));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ConnectionList_FirstFailed_SecondRDPConnection_Test()
        {
            ConnectionList_FirstFailed_SecondRDPConnection(false, false, false);
        }

        void ConnectionList_FirstFailed_SecondRDPConnection_Specify_OAuthCredentialCallback(bool isPingJwt, bool overrideTokenServiceURL, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if (overrideTokenServiceURL && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();

            ReactorConnectInfo firstConnectInfo = new ReactorConnectInfo();

            /* Expect to fails for the first connection */
            firstConnectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.SOCKET;
            firstConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            firstConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            firstConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            firstConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "5555";

            /* Establish a connection with RTO */
            ReactorConnectInfo secondConnectInfo = new ReactorConnectInfo();
            secondConnectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            secondConnectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            secondConnectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            secondConnectInfo.EnableSessionManagement = true;
            secondConnectInfo.ReactorAuthTokenEventCallback = this;

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(firstConnectInfo);
            connectOptions.ConnectionList.Add(secondConnectInfo);
            connectOptions.SetReconnectAttempLimit(-1);
            connectOptions.SetReconnectMaxDelay(15000);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            if (isPingJwt == false)
            {
                credential.ClientSecret.Data(CLIENT_SECRET);
                credential.ClientId.Data(CLIENT_ID);
            }

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.ReactorOAuthCredential = credential;
            credential.UserSpecObj = credential;
            credential.ReactorOAuthCredentialEventCallback = this;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;
            consumerRole.LoginMsgCallback = this;

            expectedState.StreamState(StreamStates.OPEN);
            expectedState.DataState(DataStates.OK);
            expectedState.Code(StateCodes.NONE);

            Buffer text = new Buffer();
            text.Data("Login accepted by host ads-fanout");
            expectedState.Text(text);

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);

            Assert.Equal(ReactorReturnCode.SUCCESS, returnCode);

            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING, reactorChannelEventTypes[0]);

            readSockets.Clear();
            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            /* Receives access token information */
            Assert.True(!string.IsNullOrEmpty(reactorAuthTokenInfo.AccessToken));
            Assert.Equal(TOKEN_TYPE, reactorAuthTokenInfo.TokenType);
            Assert.Equal(EXPIRES_IN, reactorAuthTokenInfo.ExpiresIn);

            readSockets.Clear();
            readSockets.Add(reactorEventFD);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorEventFD.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, reactorChannelEventTypes[1]);

            readSockets.Clear();
            readSockets.Add(reactorChannel.Socket);

            Socket.Select(readSockets, null, null, 10 * 1000 * 1000);

            while (reactorChannel.Socket.Available > 0)
            {
                reactor.Dispatch(dispatchOpts, out errorInfo);
            }

            Assert.Equal(LoginMsgType.REFRESH, loginMsgTypes[0]);

            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, reactorChannelEventTypes[2]);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactorChannel.Close(out errorInfo));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void ConnectionList_FirstFailed_SecondRDPConnection_Specify_OAuthCredentialCallback_Test()
        {
            ConnectionList_FirstFailed_SecondRDPConnection_Specify_OAuthCredentialCallback(false, false, false);
        }


        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void Connect_NotSpecifyClientSecretAndClientJWK_Test()
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo errorInfo);

            Assert.NotNull(reactor);

            Socket reactorEventFD = reactor.EventSocket;
            ReactorDispatchOptions dispatchOpts = new ReactorDispatchOptions();
            ReactorConnectInfo connectInfo = new ReactorConnectInfo();
            connectInfo.ConnectOptions.ConnectionType = Transports.ConnectionType.ENCRYPTED;
            connectInfo.ConnectOptions.MajorVersion = Codec.Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.Codec.MinorVersion();
            connectInfo.EnableSessionManagement = true;
            connectInfo.ReactorAuthTokenEventCallback = this;

            ReactorConnectOptions connectOptions = new ReactorConnectOptions();
            connectOptions.ConnectionList.Add(connectInfo);

            ReactorOAuthCredential credential = new ReactorOAuthCredential();

            credential.ClientId.Data(CLIENT_ID);

            ConsumerRole consumerRole = new ConsumerRole();
            consumerRole.InitDefaultRDMLoginRequest();
            consumerRole.ReactorOAuthCredential = credential;
            consumerRole.ChannelEventCallback = this;
            consumerRole.DefaultMsgCallback = this;
            consumerRole.LoginMsgCallback = this;

            ReactorReturnCode returnCode = reactor.Connect(connectOptions, consumerRole, out errorInfo);
            Assert.Equal(ReactorReturnCode.INVALID_USAGE, returnCode);
            Assert.NotNull(errorInfo);
            Assert.Equal("Failed to copy OAuth credential for enabling the session management; OAuth client secret does not exist.", errorInfo.Error.Text);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorAuthTokenEventCallback(ReactorAuthTokenEvent reactorAuthTokenEvent)
        {
            reactorAuthTokenInfo.AccessToken = reactorAuthTokenEvent.ReactorAuthTokenInfo.AccessToken;
            reactorAuthTokenInfo.TokenType = reactorAuthTokenEvent.ReactorAuthTokenInfo.TokenType;
            reactorAuthTokenInfo.ExpiresIn = reactorAuthTokenEvent.ReactorAuthTokenInfo.ExpiresIn;

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            reactorChannelEventTypes.Add(evt.EventType);

            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_UP:
                    reactorChannel = evt.ReactorChannel;
                    break;
                case ReactorChannelEventType.CHANNEL_READY:
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    break;
                case ReactorChannelEventType.CHANNEL_DOWN:
                    break;
                case ReactorChannelEventType.WARNING:
                    break;
                default:
                    Assert.True(false, "expected channel event type");
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent)
        {
            ReactorOAuthCredentialRenewalOptions renewalOptions = new ReactorOAuthCredentialRenewalOptions();
            ReactorOAuthCredentialRenewal reactorOAuthCredentialRenewal = new ReactorOAuthCredentialRenewal();
            ReactorOAuthCredential reactorOAuthCredential = (ReactorOAuthCredential)reactorOAuthCredentialEvent.UserSpecObj;

            if (reactorOAuthCredential is not null)
            {
                renewalOptions.RenewalModes = ReactorOAuthCredentialRenewalModes.CLIENT_SECRET;
                reactorOAuthCredentialRenewal.ClientSecret.Data(reactorOAuthCredential.ClientSecret.ToString());

                reactorOAuthCredentialEvent.Reactor!.SubmitOAuthCredentialRenewal(renewalOptions, reactorOAuthCredentialRenewal, out _);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            LoginMsg loginMsg = loginEvent.LoginMsg;

            if(loginMsg is not null)
            {
                loginMsgTypes.Add(loginMsg.LoginMsgType);
                if(loginMsg.LoginMsgType == LoginMsgType.REFRESH)
                {
                    LoginRefresh loginRefresh = loginMsg.LoginRefresh;

                    if(loginRefresh != null)
                    {
                        State state = loginRefresh.State;
                        Assert.Equal(expectedState.StreamState(), state.StreamState());
                        Assert.Equal(expectedState.DataState(), state.DataState());
                        Assert.Equal(expectedState.Code(), state.Code());
                        Assert.True(state.Text().ToString().IndexOf(expectedState.Text().ToString()) != -1);
                    }
                }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }
}
