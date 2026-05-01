/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Tests.ValueAddTest;
using System;
using Xunit;

using static LSEG.Eta.Tests.ValueAddTest.TestUtil;
using static LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests.TestUtil;

namespace LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests
{
    public class FallbackPreferredHostIntegrationTests
    {
        [Fact]
        public void Should_not_disconnect_and_send_PREFERRED_HOST_COMPLETE_when_preferred_host_is_down()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = phOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);
            provider3.Bind(opts);
            // connect to preferred host
            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider3.ServerPort);
            provider3.Accept(opts);
            AssertInitialDataExchange(consumer, provider3, hasChannelDownReconnecting: false);
            // bring down provider # 2 and make it preferred host
            provider2.Dispose();

            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(
                    ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS,
                    new ReactorPreferredHostOptions(phOpts) { ConnectionListIndex = 1 },
                    out err));

            Thread.Sleep(1000);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.FallbackPreferredHost(out err));

            // Assert
            consumer.TestReactor
                .Dispatch(2, TimeSpan.FromSeconds(60))
                .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_STARTING_FALLBACK)
                .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_COMPLETE);
        }
    }
}
