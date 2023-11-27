/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Rdm;

using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests;

public class OAuth2RenewalTests
{
    private const string EMA_FILE_PATH = "../../../OmmConsumerTests/EmaOAuthTest.xml";

    internal readonly string? CLIENT_ID;
    internal readonly string? CLIENT_SECRET;

    readonly ITestOutputHelper output;


    public OAuth2RenewalTests(ITestOutputHelper output)
    {
        this.output = output;

        CLIENT_ID = Environment.GetEnvironmentVariable("ETANET_CLIENT_ID", System.EnvironmentVariableTarget.Process);
        CLIENT_SECRET = Environment.GetEnvironmentVariable("ETANET_CLIENT_SECRET", System.EnvironmentVariableTarget.Process);
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
        public void OnOAuth2CredentialRenewal(IOmmConsumerEvent evt, OAuth2CredentialRenewal creds)
        {
            CredentialStore credentials = (CredentialStore)evt.Closure!;

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

        CredentialStore credentials = new CredentialStore()
        {
            ClientId = CLIENT_ID!,
            ClientSecret = CLIENT_SECRET!
        };

        OmmConsumerConfig config = new(EMA_FILE_PATH);
        config.ConsumerName("Consumer_5")
            .ClientId(CLIENT_ID!)
            .ClientSecret(CLIENT_SECRET!);

        AppClient consumerClient = new AppClient(output);

        OAuthCallback oAuthCallback = new OAuthCallback();
        OmmConsumer consumer = new OmmConsumer(config, consumerClient, oAuthCallback, credentials);

        string itemName = "ItemName";

        RequestMsg requestMsg = new();
        requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE)
            .ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
            .Name(itemName).InterestAfterRefresh(true);

        long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

        Assert.True(handle > 0);

        Thread.Sleep(5_000);

        Assert.NotEqual(0, credentials.Renewals);

        {
            OAuth2CredentialRenewal renewal = new OAuth2CredentialRenewal();

            renewal.ClientId(credentials.ClientId)
                .ClientSecret(credentials.ClientSecret);

            // attempt to renew credentials outside of the callback method
            Assert.Throws<OmmInvalidUsageException>(() => consumer.RenewOAuthCredentials(renewal));
        }

        consumer.Uninitialize();

        output.WriteLine("SimpleRenewal_Test() END");
    }

}
