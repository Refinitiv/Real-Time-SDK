/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
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
    public class InfoIntegrationTests
    {
        [Fact]
        public void Should_return_initial_PreferredHostInfo_after_connect()
        {
            // Arrange
            var phOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
                DetectionTimeInterval = 60,
                DetectionTimeSchedule = GetCronExpressionFromNow(minutes: 10),
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

            consumer.Connect(opts, provider1.ServerPort, provider2.ServerPort, provider2.ServerPort + 1);
            provider1.Accept(opts);
            AssertInitialDataExchange(consumer, provider1);

            // Act
            var info = new ReactorChannelInfo();
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.Info(info, out err));

            // Assert
            AssertPreferredHostInfoEqual(phOpts, info.PreferredHostInfo);
        }

        [Fact]
        public void Should_return_updated_PreferredHostInfo_after_IOCtl_applied()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
                DetectionTimeInterval = 60,
                DetectionTimeSchedule = GetCronExpressionFromNow(minutes: 15),
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts)
            {
                EnablePreferredHostOptions = false,
                ConnectionListIndex = 1,
                DetectionTimeInterval = 90,
                DetectionTimeSchedule = GetCronExpressionFromNow(hours: 2),
            };

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

            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));
            consumer.TestReactor.Dispatch(-1, TimeSpan.FromSeconds(10));

            // Act
            var info = new ReactorChannelInfo();
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.Info(info, out err));

            // Assert
            AssertPreferredHostInfoEqual(newPHOpts, info.PreferredHostInfo);
        }

        [Fact]
        public void Should_return_initial_PreferredHostInfo_before_IOCtl_applied_during_fallback()
        {
            // Arrange
            var initialPHOpts = new ReactorPreferredHostOptions
            {
                EnablePreferredHostOptions = true,
                ConnectionListIndex = 2,
                DetectionTimeInterval = 60,
                DetectionTimeSchedule = GetCronExpressionFromNow(minutes: 15),
            };
            var newPHOpts = new ReactorPreferredHostOptions(initialPHOpts)
            {
                EnablePreferredHostOptions = false,
                ConnectionListIndex = 1,
                DetectionTimeInterval = 90,
                DetectionTimeSchedule = GetCronExpressionFromNow(hours: 2),
            };

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

            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.FallbackPreferredHost(out err));
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.IOCtl(ReactorChannelIOCtlCode.PREFERRED_HOST_OPTIONS, newPHOpts, out err));

            // Act
            var infoBefore = new ReactorChannelInfo();
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.Info(infoBefore, out err));

            // wait for fallback completion & new PH options to be applied
            consumer.TestReactor
                .Dispatch(-1, TimeSpan.FromSeconds(10))
                .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_STARTING_FALLBACK)
                .AssertReactorChannelEvent(ReactorChannelEventType.PREFERRED_HOST_COMPLETE);

            Thread.Sleep(TimeSpan.FromSeconds(1));
            var infoAfter = new ReactorChannelInfo();
            AssertSuccess((out ReactorErrorInfo err) =>
                consumer.ReactorChannel.Info(infoAfter, out err));

            // Assert
            AssertPreferredHostInfoEqual(initialPHOpts, infoBefore.PreferredHostInfo);
            AssertPreferredHostInfoEqual(newPHOpts, infoAfter.PreferredHostInfo);
        }

        private static void AssertPreferredHostInfoEqual(ReactorPreferredHostOptions expectedPreferredHostOptions, ReactorPreferredHostInfo actualPreferredHostInfo)
        {
            Assert.Equal(expectedPreferredHostOptions.EnablePreferredHostOptions, actualPreferredHostInfo.IsPreferredHostEnabled);
            Assert.Equal(expectedPreferredHostOptions.ConnectionListIndex, actualPreferredHostInfo.ConnectionListIndex);
            Assert.Equal(expectedPreferredHostOptions.DetectionTimeInterval, actualPreferredHostInfo.DetectionTimeInterval);
            Assert.Equal(expectedPreferredHostOptions.DetectionTimeSchedule, actualPreferredHostInfo.DetectionTimeSchedule);
        }
    }
}
