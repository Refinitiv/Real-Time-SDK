/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.IO;
using System.Text.RegularExpressions;

namespace LSEG.Eta.Tests.ValueAddTest
{
    public class ReactorQueryServiceDiscoveryTest : IReactorServiceEndpointEventCallback
    {
        private readonly string CLIENT_ID;
        private readonly string CLIENT_ID_JWT;
        private readonly string CLIENT_SECRET;
        private readonly string CLIENT_JWK;
        private readonly string TOKEN_SERVICE_URL;
        private readonly string SERVICE_DISCOVERY_URL;
        private readonly string PROXY_HOST;
        private readonly string PROXY_PORT;
        private int expectedNumOfEndpoint = 0;
        private string expectedErrorRegexFromCallback;
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
                PROXY_HOST = Environment.GetEnvironmentVariable("ETANET_PROXY_HOST", System.EnvironmentVariableTarget.Process);
                PROXY_PORT = Environment.GetEnvironmentVariable("ETANET_PROXY_PORT", System.EnvironmentVariableTarget.Process);
            }
            catch (Exception)
            {
                Assert.Fail("Failed to get OAuth credential from process environment");
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

                foreach (var endpoint in serviceEndpointEvent.ServiceEndpointInfoList)
                {
                    matchTransport = false;
                    for (int i = 0; i < expectedTransports.Length; i++)
                    {
                        if (endpoint.Transport.IndexOf(expectedTransports[i]) != -1)
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

                var actualErrorMsg = serviceEndpointEvent.ReactorErrorInfo.Error.Text;

                Assert.True(Regex.IsMatch(actualErrorMsg, expectedErrorRegexFromCallback),
                    "Pattern not found in value" + NewLine +
                    $"Regex: {expectedErrorRegexFromCallback}{NewLine}" +
                    $"Value: {actualErrorMsg}");
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private ReactorOptions CreateReactorOptions()
        {
            var options = new ReactorOptions();
            if (!string.IsNullOrEmpty(PROXY_HOST))
            {
                options.RestProxyOptions.ProxyHostName = PROXY_HOST;
                options.RestProxyOptions.ProxyPort = PROXY_PORT;
            }
            return options;
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryNotSpecifyClientSecrectAndClientJwkTest()
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.PARAMETER_INVALID, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal("Required parameter ClientSecret or ClientJwk is not set", errorInfo.Error.Text);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryUnauthorizedTest()
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data("invalidUser");
            serviceDiscoveryOptions.ClientSecret.Data("invalidSecret");
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorRegexFromCallback = "Failed to perform a REST request to the token service. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid client or client credentials.\" }";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryUnauthorized_JWT_Test()
        {
            ReactorOptions reactorOptions = CreateReactorOptions();

            if(!string.IsNullOrEmpty(TOKEN_SERVICE_URL))
            {
                reactorOptions.SetTokenServiceURL(TOKEN_SERVICE_URL);
            }

            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data("InvalidClientID");
            serviceDiscoveryOptions.ClientJwk.Data(File.ReadAllText(CLIENT_JWK));
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorRegexFromCallback = "Failed to perform a REST request to the token service. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Client not found in client database for JWT's sub claim value 'InvalidClientID'.\" }";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryInvalid_JWK_Test()
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID_JWT);
            serviceDiscoveryOptions.ClientJwk.Data("invalidClientJwk");
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.FAILURE, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, errorInfo.Code);
            string expectedText = "Failed to retrieve Json Web Key information invalidClientJwk.";
            Assert.StartsWith(expectedText, errorInfo.Error.Text);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryInvalidTokenServiceURLTest()
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
            reactorOptions.SetTokenServiceURL("https://invalid.api.refinitiv.com/auth/oauth2/v2/token");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorRegexFromCallback = @"^Failed to perform a REST request to the token service. Text: " +
                @"(No such host is known. \(invalid.api.refinitiv.com:443\))" +
                @"|(The proxy tunnel request to proxy .+ failed with status code '404')";
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
            ReactorOptions reactorOptions = CreateReactorOptions();
            reactorOptions.SetServiceDiscoveryURL("https://invalid.api.refinitiv.com/streaming/pricing/v1/");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 0;
            expectedErrorRegexFromCallback = @"^Failed to perform a REST request to the service discovery. Text: "+
                @"(No such host is known. \(invalid.api.refinitiv.com:443\))"+
                @"|(The proxy tunnel request to proxy .+ failed with status code '404')";
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.Null(errorInfo);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        void RequestQueryServiceDiscovery(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
            
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
                expectedNumOfEndpoint = 30;
            }
            else
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID_JWT);
                serviceDiscoveryOptions.ClientJwk.Data(File.ReadAllText(CLIENT_JWK));
                expectedNumOfEndpoint = 6;
            }

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
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

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_JWK_Test()
        {
            RequestQueryServiceDiscovery(true, false, false);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_JWK_Audience_Valid_Test()
        {
            RequestQueryServiceDiscovery_JWK_Audience(false, false, ReactorOAuthCredential.DEFAULT_JWT_AUDIENCE);
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_JWK_Audience_InvalidValid_Test()
        {
            RequestQueryServiceDiscovery_JWK_Audience(true, true, "InvalidAudience");
        }

        private void RequestQueryServiceDiscovery_JWK_Audience(bool overrideTokenService, bool overrideDiscoveryURL, string audience)
        {
            ReactorOptions reactorOptions = CreateReactorOptions();

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

            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID_JWT);
            serviceDiscoveryOptions.ClientJwk.Data(File.ReadAllText(CLIENT_JWK));
            serviceDiscoveryOptions.Audience.Data(audience);

            if (audience.Equals(ReactorOAuthCredential.DEFAULT_JWT_AUDIENCE))
            {
                expectedNumOfEndpoint = 6;
            }
            else // Invalid audience
            {
                expectedNumOfEndpoint = 0;
                expectedErrorRegexFromCallback = "Failed to perform a REST request to the token service. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"(Client not found in client database for JWT's sub claim value|Invalid JWT token)";
            }

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedTransports = new[] { "tcp", "websocket" };

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        void RequestQueryServiceDiscovery_TP_TCP(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = CreateReactorOptions();

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
                expectedNumOfEndpoint = 15;
            }
            else
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID_JWT);
                serviceDiscoveryOptions.ClientJwk.Data(File.ReadAllText(CLIENT_JWK));
                expectedNumOfEndpoint = 3;
            }

            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
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
        
        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_TP_TCP_JWT_Test()
        {
            RequestQueryServiceDiscovery_TP_TCP(true, false, false);
        }

        void RequestQueryServiceDiscovery_DP_JSON2(bool isPingJwt, bool overrideTokenService, bool overrideDiscoveryURL)
        {
            ReactorOptions reactorOptions = CreateReactorOptions();
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
                expectedNumOfEndpoint = 15;
            }
            else
            {
                serviceDiscoveryOptions.ClientId.Data(CLIENT_ID_JWT);
                serviceDiscoveryOptions.ClientJwk.Data(File.ReadAllText(CLIENT_JWK));
                expectedNumOfEndpoint = 3;
            }
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
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
        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_DP_JSON2_JWT_Test()
        {
            RequestQueryServiceDiscovery_DP_JSON2(true, false, false);
        }
    }
}
