/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using Xunit.Categories;
using Refinitiv.Eta.ValueAdd.Reactor;
using System;

namespace Refinitiv.Eta.ValuedAdd.Tests
{
    public class ReactorQueryServiceDiscoveryTest : IReactorServiceEndpointEventCallback
    {
        private string CLIENT_ID;
        private string CLIENT_SECRET;
        private int expectedNumOfEndpoint = 0;
        private string[] expectedTransports;

        public ReactorQueryServiceDiscoveryTest()
        {
            try
            {
                CLIENT_ID = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID", System.EnvironmentVariableTarget.Process);
                CLIENT_SECRET = Environment.GetEnvironmentVariable("ETANET_CLIENT_SECRET", System.EnvironmentVariableTarget.Process);
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

            return ReactorCallbackReturnCode.SUCCESS;
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
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, errorInfo.Code);
            Assert.Equal("Failed to get access token information from the token service. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid client or client credentials.\" }", errorInfo.Error.Text);

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
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, errorInfo.Code);
            Assert.Equal("Failed to perform a REST request to the token service. Text: No such host is known. (invalid.api.refinitiv.com:443)", errorInfo.Error.Text);

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
            ReactorErrorInfo errorInfo;
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out errorInfo));
            Assert.NotNull(errorInfo);
            Assert.Equal(ReactorReturnCode.FAILURE, errorInfo.Code);
            Assert.Equal("Failed to perform a REST request to the service discovery. Text: No such host is known. (invalid.api.refinitiv.com:443)", errorInfo.Error.Text);

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscoveryTest()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 30;
            expectedTransports = new[] { "tcp", "websocket" };
            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS,reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_TP_TCP_Test()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.SetTokenServiceURL("https://api.refinitiv.com/auth/oauth2/v2/token");
            reactorOptions.SetServiceDiscoveryURL("https://api.refinitiv.com/streaming/pricing/v1/");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 15;
            expectedTransports = new[] { "tcp" };

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }

        [Fact]
        [Category("Unit")]
        [Category("Reactor")]
        public void RequestQueryServiceDiscovery_DP_JSON2_Test()
        {
            ReactorOptions reactorOptions = new ReactorOptions();
            reactorOptions.SetTokenServiceURL("https://api.refinitiv.com/auth/oauth2/v2/token");
            reactorOptions.SetServiceDiscoveryURL("https://api.refinitiv.com/streaming/pricing/v1/");
            reactorOptions.UserSpecObj = this;
            Reactor reactor = Reactor.CreateReactor(reactorOptions, out _);

            Assert.NotNull(reactor);

            ReactorServiceDiscoveryOptions serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2;
            serviceDiscoveryOptions.ClientId.Data(CLIENT_ID);
            serviceDiscoveryOptions.ClientSecret.Data(CLIENT_SECRET);
            serviceDiscoveryOptions.ReactorServiceEndpointEventCallback = this;
            expectedNumOfEndpoint = 15;
            expectedTransports = new[] { "websocket" };

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.QueryServiceDiscovery(serviceDiscoveryOptions, out _));

            Assert.Equal(ReactorReturnCode.SUCCESS, reactor.Shutdown(out _));
        }
    }
}
