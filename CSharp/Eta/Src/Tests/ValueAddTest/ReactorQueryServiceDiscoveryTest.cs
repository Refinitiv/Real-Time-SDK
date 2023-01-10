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
using System;
using Xunit.Abstractions;
using System.IO;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class ReactorQueryServiceDiscoveryTest : IReactorServiceEndpointEventCallback
    {
        private string CLIENT_ID;
        private string CLIENT_ID_JWT;
        private string CLIENT_SECRET;
        private string CLIENT_JWK;
        private string TOKEN_SERVICE_URL;
        private string SERVICE_DISCOVERY_URL;
        private int expectedNumOfEndpoint = 0;
        private string expectedErrorTextFromCallback;
        private string[] expectedTransports;

        private readonly ITestOutputHelper output;

        public ReactorQueryServiceDiscoveryTest(ITestOutputHelper output)
        {
            this.output = output;

            try
            {
                CLIENT_ID = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID", System.EnvironmentVariableTarget.Process);
                CLIENT_ID_JWT = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID_JWT", System.EnvironmentVariableTarget.Process);
                CLIENT_SECRET = Environment.GetEnvironmentVariable("ETANET_CLIENT_SECRET", System.EnvironmentVariableTarget.Process);
                CLIENT_JWK = Environment.GetEnvironmentVariable("ETANET_CLIENT_JWK", System.EnvironmentVariableTarget.Process);
                TOKEN_SERVICE_URL = Environment.GetEnvironmentVariable("ETANET_TOKEN_SERVICE_URL", System.EnvironmentVariableTarget.Process);
                SERVICE_DISCOVERY_URL = Environment.GetEnvironmentVariable("ETANET_SERVICE_DISCOVERY_URL", System.EnvironmentVariableTarget.Process);
            }
            catch (Exception)
            {
                Assert.True(false, "Failed to get OAuth credential from process environment");
            }
        }

        public ReactorCallbackReturnCode ReactorServiceEndpointEventCallback(ReactorServiceEndpointEvent serviceEndpointEvent)
        {
            Assert.NotNull(serviceEndpointEvent);

            if (expectedNumOfEndpoint != 0)
            {
                Assert.NotNull(serviceEndpointEvent.ServiceEndpointInfoList);
                Assert.True(serviceEndpointEvent.ServiceEndpointInfoList.Count >= expectedNumOfEndpoint);

                bool matchTransport;

                foreach(var endpoint in serviceEndpointEvent.ServiceEndpointInfoList)
                {
                    matchTransport = false;
                    for (int i = 0; i < expectedTransports.Length; i++)
                    {
                        if(endpoint.Transport.IndexOf(expectedTransports[i]) != -1)
                        {
                            matchTransport = true;
                        }
                    }

                    Assert.True(matchTransport);
                }
            }
            else
            {
                Assert.NotNull(serviceEndpointEvent.ReactorErrorInfo);
                Assert.Equal(expectedErrorTextFromCallback, serviceEndpointEvent.ReactorErrorInfo.Error.Text);
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryNotSpecifyClientSecrectTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.PARAMETER_INVALID, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal("Required parameter ClientSecret is not set", errorInfo.Error.Text);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryUnauthorizedTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data("invalidUser");
            serviceDiscoveryOptions.ClientSecret.Data("invalidSecret");
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorTextFromCallback = "Failed to perform a REST request to the token service. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid client or client credentials.\" }";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryInvalidTokenServiceURLTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.SetTokenServiceURL("https://invalid.api.refinitiv.com/auth/oauth2/v2/token");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorTextFromCallback = "Failed to perform a REST request to the token service. Text: No such host is known. (invalid.api.refinitiv.com:443)";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryInvalidServiceDiscoveryURLTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.SetServiceDiscoveryURL("https://invalid.api.refinitiv.com/streaming/pricing/v1/");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorTextFromCallback = "Failed to perform a REST request to the service discovery. Text: No such host is known. (invalid.api.refinitiv.com:443)";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        void RequestQueryServiceDiscovery(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            
            if(overrideTokenService && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.EnableRestLogStream = true;

            var memoryStream = new MemoryStream();

            reactorOptions.RestLogOutputStream = memoryStream;
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();

            if (isPingJwt == false)
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
                serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            }

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 30;
            expectedTransports = new[] { "tcp", "websocket" };
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            string restDumpLogs = System.Text.Encoding.Default.GetString(memoryStream.GetBuffer(),
                0, (int)memoryStream.Position);

            Assert.True(restDumpLogs.IndexOf("tcp") != -1 && restDumpLogs.IndexOf("websocket") != -1);

            output.WriteLine(restDumpLogs);

            Assert.Equal(ReactorReturnCode.SUCCESS,reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryTest()
        {
            RequestQueryServiceDiscovery(false, false, false);
        }


        void RequestQueryServiceDiscovery_TP_TCP(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();

            if (overrideTokenService && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;

            if (isPingJwt == false)
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
                serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            }

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 15;
            expectedTransports = new[] { "tcp" };

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_TP_TCP_Test()
        {
            RequestQueryServiceDiscovery_TP_TCP(false, false, false);
        }

        void RequestQueryServiceDiscovery_DP_JSON2(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            if (overrideTokenService && !string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            if (overrideDiscoveryURL && !string.IsNullOrEmpty(SERVICE_DISCOVERY_URL))
            {
                reactorOptions.SetServiceDiscoveryURL(SERVICE_DISCOVERY_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2;
            if (isPingJwt == false)
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
                serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            }
            

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 15;
            expectedTransports = new[] { "websocket" };

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_DP_JSON2_Test()
        {
            RequestQueryServiceDiscovery_DP_JSON2(false, false, false);
        }
    }
}
