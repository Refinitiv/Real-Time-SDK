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
using System.Threading;
using Xunit;

using static LSEG.Eta.Tests.ValueAddTest.TestUtil;
using static LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests.TestUtil;

namespace LSEG.Eta.Tests.ValueAddTest.ReactorChannelPreferredHostTests
{
    public class IOCtlIntegrationTests
    {
        [Fact]
        public void Should_update_DetectionTimeInterval_in_preferred_host_options_when_all_preconditions_met()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { DetectionTimeInterval = 5 };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Assert
            Thread.Sleep(TimeSpan.FromSeconds(newPHOpts.DetectionTimeInterval) + TimeSpan.FromMilliseconds(100));
            provider3.Accept(opts);
            AssertInitialDataExchange(consumer, provider3, isSwitchToPreferredHost: true);
        }

        [Fact]
        public void Should_update_DetectionTimeSchedule_in_preferred_host_options_when_all_preconditions_met()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { DetectionTimeSchedule = "* * ? * *" };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Assert
            // "* * ? * *" fires at the next absolute minute boundary (:00 seconds) after Reset() is
            // called inside IOCtl.  A fixed 60-second sleep races against wall clock: if IOCtl is
            // applied near the end of a minute the cron fires almost immediately, but provider3.Accept
            // would only be called ~60 s later — long after the reconnect attempt has timed out.
            //
            // Instead, compute the next :00 boundary from the actual IOCtl call time so the sleep
            // always ends ~200 ms after the cron fires, regardless of the wall clock position.
            var ioCtlTime = DateTime.Now;
            var nextCronFire = new DateTime(
                 ioCtlTime.Year, ioCtlTime.Month, ioCtlTime.Day,
                 ioCtlTime.Hour, ioCtlTime.Minute, 0).AddMinutes(1);

            var sleepDuration = nextCronFire - DateTime.Now + TimeSpan.FromMilliseconds(100);
            Thread.Sleep(sleepDuration);

            //Thread.Sleep(TimeSpan.FromMinutes(1) + TimeSpan.FromMilliseconds(300));
            provider3.Accept(opts);
            AssertInitialDataExchange(consumer, provider3, isSwitchToPreferredHost: true);
        }

        [Fact]
        public void Should_update_ConnectionListIndex_in_preferred_host_options_when_all_preconditions_met()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { ConnectionListIndex = 1 };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Assert
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.FallbackPreferredHost(out err));
            provider2.Accept(opts);
            AssertInitialDataExchange(consumer, provider2, isSwitchToPreferredHost: true);
        }

        [Fact]
        public void Should_update_EnablePreferredHostOptions_in_preferred_host_options_when_all_preconditions_met()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { EnablePreferredHostOptions = false };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Assert
            Thread.Sleep(TimeSpan.FromSeconds(1));
            provider1.Dispose();
            consumer.TestReactor.Dispatch(1);
            provider2.Accept(opts);
            AssertInitialDataExchange(consumer, provider2, isSwitchToPreferredHost: false);
        }

        [Fact]
        public void Should_apply_preferred_host_options_update_after_fallback_completion()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { ConnectionListIndex = 0 };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.FallbackPreferredHost(out err));
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Assert
            provider3.Accept(opts);
            AssertInitialDataExchange(consumer, provider3, isSwitchToPreferredHost: true);

            provider1.TestReactor
                .Dispatch(1, channelWasClosed: true)
                .AssertReactorChannelEvent(ReactorChannelEventType.CHANNEL_DOWN);

            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.FallbackPreferredHost(out err));
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1, isSwitchToPreferredHost: true);
        }

        [Fact]
        public void Should_return_proper_error_when_preferred_host_option_is_wrong()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
            };
            var wrongPHOpts = new ReactorPreferredHostOptions(initialPHOpts) { ConnectionListIndex = -2 };

            using var provider1 = new Provider().WithDefaultRole();
            using var provider2 = new Provider().WithDefaultRole();
            using var provider3 = new Provider().WithDefaultRole();

            using var consumer = new Consumer().WithDefaultRole();

            var opts = new ConsumerProviderSessionOptions
            {
                PreferredHostOptions = initialPHOpts,
                ReconnectAttemptLimit = 1,
            };

            provider1.Bind(opts);
            provider2.Bind(opts);

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            provider3.Bind(opts);

            // Act
            var retCode = consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, wrongPHOpts, out var errorInfo);

            // Assert
            Assert.Equal(ReactorReturnCode.FAILURE, retCode);
            Assert.StartsWith("Preferred connection index cannot be negative", errorInfo.Error?.Text);
        }
    }
}
