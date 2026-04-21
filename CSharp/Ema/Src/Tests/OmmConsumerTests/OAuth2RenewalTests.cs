/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access.Tests.Utils;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests;

public class OAuth2RenewalTests
{
    internal readonly string? CLIENT_ID;
    internal readonly string? CLIENT_SECRET;
    internal readonly string? PROXY_HOST;
    internal readonly string? PROXY_PORT;
    internal readonly string? LOCATION;
    internal readonly string? SERVICE_DISCOVERY_URL;
    internal readonly string? TOKEN_SERVICE_URL;

    readonly ITestOutputHelper output;


    public OAuth2RenewalTests(ITestOutputHelper output)
    {
        this.output = output;

        CLIENT_ID = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID", System.EnvironmentVariableTarget.Process);
        CLIENT_SECRET = Environment.GetEnvironmentVariable("ETANET_CLIENT_SECRET", System.EnvironmentVariableTarget.Process);
        PROXY_HOST = Environment.GetEnvironmentVariable("ETANET_PROXY_HOST", System.EnvironmentVariableTarget.Process);
        PROXY_PORT = Environment.GetEnvironmentVariable("ETANET_PROXY_PORT", System.EnvironmentVariableTarget.Process);
        LOCATION = Environment.GetEnvironmentVariable("ETANET_LOCATION", System.EnvironmentVariableTarget.Process);
        SERVICE_DISCOVERY_URL = Environment.GetEnvironmentVariable("ETANET_SERVICE_DISCOVERY_URL", System.EnvironmentVariableTarget.Process);
        TOKEN_SERVICE_URL = Environment.GetEnvironmentVariable("ETANET_TOKEN_SERVICE_URL", System.EnvironmentVariableTarget.Process);
    }

    private Map CreateProgConfig(bool debug = false)
    {
        var configMap = new Map()
            .AddKeyAscii("ConsumerGroup", MapAction.ADD, new ElementList()
                .AddAscii("DefaultConsumer", "Consumer_5")
                .AddMap("ConsumerList", new Map()
                    .AddKeyAscii("Consumer_5", MapAction.ADD, new ElementList()
                        .AddAscii("ChannelSet", "Channel_1, Channel_2")
                        .AddAscii("Dictionary", "Dictionary_1")
                        .AddUInt("XmlTraceToStdout", debug ? 1ul : 0ul)
                        .Tap(_ =>
                        {
                            if (debug)
                                _
                                    .AddUInt("RestEnableLog", 1)
                                    .AddAscii("RestLogFileName", "rest.log")
                                    .AddAscii("Logger", "Logger_2");
                        })
                        .Complete())
                    .Complete())
                .Complete())

            .AddKeyAscii("ChannelGroup", MapAction.ADD, new ElementList()
                .AddMap("ChannelList", new Map()
                    .AddKeyAscii("Channel_1", MapAction.ADD, new ElementList()
                        .AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET)
                        .AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.NONE)
                        .AddUInt("GuaranteedOutputBuffers", 5000)
                        .AddUInt("ConnectionPingTimeout", 30000)
                        .AddUInt("TcpNodelay", 1)
                        .AddAscii("Host", "localhost")
                        .AddAscii("Port", "14002")
                        .Complete())
                    .AddKeyAscii("Channel_2", MapAction.ADD, new ElementList()
                        .AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.ENCRYPTED)
                        .AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.NONE)
                        .AddUInt("GuaranteedOutputBuffers", 5000)
                        .AddAscii("Location", !string.IsNullOrEmpty(LOCATION) ? LOCATION : "us-east-1")
                        .AddUInt("EnableSessionManagement", 1)
                        .AddEnum("EncryptedProtocolType", EmaConfig.EncryptedProtocolTypeEnum.SOCKET)
                        .Tap(_ => {
                            if (!string.IsNullOrEmpty(PROXY_HOST))
                                _.AddAscii("ProxyHost", PROXY_HOST);
                            if (!string.IsNullOrEmpty(PROXY_PORT))
                                _.AddAscii("ProxyPort", PROXY_PORT);
                        })
                        .Complete())
                    .Complete())
                .Complete())

            .AddKeyAscii("LoggerGroup", MapAction.ADD, new ElementList()
                .AddMap("LoggerList", new Map()
                    .AddKeyAscii("Logger_1", MapAction.ADD, new ElementList()
                        .AddEnum("LoggerType", EmaConfig.LoggerTypeEnum.STDOUT)
                        .AddEnum("LoggerSeverity", EmaConfig.LoggerLevelEnum.INFO)
                        .Complete())
                    .AddKeyAscii("Logger_2", MapAction.ADD, new ElementList()
                        .AddEnum("LoggerType", EmaConfig.LoggerTypeEnum.FILE)
                        .AddEnum("LoggerSeverity", (ushort)(debug ? EmaConfig.LoggerLevelEnum.TRACE : EmaConfig.LoggerLevelEnum.INFO))
                        .Complete())
                    .Complete())
                .Complete())

            .AddKeyAscii("DictionaryGroup", MapAction.ADD, new ElementList()
                .AddMap("DictionaryList", new Map()
                    .AddKeyAscii("Dictionary_1", MapAction.ADD, new ElementList()
                        .AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.CHANNEL)
                        .Complete())
                    .Complete())
                .Complete())

            .Complete();

        return configMap;
    }

    public class AppClient : IOmmConsumerClient
    {
        ITestOutputHelper output;

        public AppClient(ITestOutputHelper output)
        {
            this.output = output;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent evt)
        {
            output.WriteLine(refreshMsg.ToString());
        }

        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent evt)
        {
            output.WriteLine(updateMsg.ToString());
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent evt)
        {
            output.WriteLine(statusMsg.ToString());
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent) { }
        public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) { }
        public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent) { }
    }

    class CredentialStore
    {
        public string ClientSecret = string.Empty;
        public string ClientId = string.Empty;
        // use it as a counter for test Assertion
        public int Renewals = 0;
    }

    // this the callback responsible for refreshing OAuth2 credentials for
    // EMA/Reactor. Application would implement a real secure credential store, but in
    // this test it is basically a mock up
    class OAuthCallback : IOmmOAuth2ConsumerClient
    {
        private readonly EventWaitHandle _executedEvent;

        public OAuthCallback(EventWaitHandle executedEvent)
        {
            _executedEvent = executedEvent;
        }

        public void OnOAuth2CredentialRenewal(IOmmConsumerEvent evt, OAuth2CredentialRenewal creds)
        {
            CredentialStore credentials = (CredentialStore)evt.Closure!;
            
            _executedEvent.Set();

            Assert.Equal(credentials.ClientId, creds.ClientId());

            OAuth2CredentialRenewal renewal = new OAuth2CredentialRenewal();

            renewal.ClientId(credentials.ClientId)
                .ClientSecret(credentials.ClientSecret);

            evt.Consumer.RenewOAuthCredentials(renewal);

            ++credentials.Renewals;
        }
    }

    /// <summary>
    /// Verify that the library invokes credential renewal callback and that the renewed
    /// credentials are accepted.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Test scenario is as follows.</para>
    ///
    /// <para>
    /// A custom configuration defines a consumer with two channels:</para>
    ///
    /// <para>
    /// Channel_1 is an invalid channel that the library tries to connect to.</para>
    ///
    /// <para>
    /// Channel_2 is a valid connection to "us-east-1" Location.</para>
    ///
    /// <para>
    /// <see cref="Reactor"/> first tries to establish connection via the first Channel_1,
    /// but fails. And switches to the Channel_2 and before doing that invokes callback to
    /// renew credentials.</para>
    ///
    /// <para>
    /// This test requires valid OAuth2 credentials to be supplies via environment
    /// variables (<see cref="CLIENT_ID"/> and <see cref="CLIENT_SECRET"/>) and being able
    /// to establish a network connection to the servers.</para>
    ///
    /// </remarks>
    [Fact]
    public void SimpleRenewal_Test()
    {
        output.WriteLine("SimpleRenewal_Test() BEGIN");

        // first of all ensure that required environment variables were set up at all
        Assert.NotNull(CLIENT_ID);
        Assert.NotNull(CLIENT_SECRET);
        Assert.NotEmpty(CLIENT_ID!);
        Assert.NotEmpty(CLIENT_SECRET!);

        CredentialStore credentials = new()
        {
            ClientId = CLIENT_ID!,
            ClientSecret = CLIENT_SECRET!
        };

        OmmConsumerConfig config = new();
        config
            .Config(CreateProgConfig(debug: false).MarkForClear())
            .ConsumerName("Consumer_5")
            .ClientId(CLIENT_ID!)
            .ClientSecret(CLIENT_SECRET!)
            .ServiceDiscoveryUrl(SERVICE_DISCOVERY_URL!)
            .TokenUrlV2(TOKEN_SERVICE_URL!);

        AppClient consumerClient = new AppClient(output);

        using var oauthExecEvent = new AutoResetEvent(false);
        OAuthCallback oAuthCallback = new OAuthCallback(oauthExecEvent);
        OmmConsumer? consumer = null;
        try
        {
            consumer = new OmmConsumer(config, consumerClient, oAuthCallback, credentials);

            string itemName = "ItemName";

            RequestMsg requestMsg = new();
            requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE)
                .ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
                .Name(itemName).InterestAfterRefresh(true);

            long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

            Assert.True(handle > 0);

            oauthExecEvent.WaitOne(TimeSpan.FromSeconds(30));

            Assert.NotEqual(0, credentials.Renewals);

            {
                OAuth2CredentialRenewal renewal = new OAuth2CredentialRenewal();

                renewal.ClientId(credentials.ClientId)
                    .ClientSecret(credentials.ClientSecret);

                // attempt to renew credentials outside of the callback method
                Assert.Throws<OmmInvalidUsageException>(() => consumer.RenewOAuthCredentials(renewal));
            }
        }
        finally
        {
            config.Clear();
            consumer?.Uninitialize();
            output.WriteLine("SimpleRenewal_Test() END");
        }
    }

}
