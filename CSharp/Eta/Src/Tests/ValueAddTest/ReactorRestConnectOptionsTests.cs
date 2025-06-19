/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using Xunit;

namespace LSEG.Eta.Tests.ValueAddTest
{
    public class ReactorRestConnectOptionsTests
    {
        [Fact]
        public void Ctor_AcceptsPassedNotEmptyProxyOptions_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions
            {
                RestProxyOptions = CreateNonEmptyProxyOptions()
            };

            // Act
            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyHostName, subject.ProxyOptions.ProxyHostName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPort, subject.ProxyOptions.ProxyPort);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyUserName, subject.ProxyOptions.ProxyUserName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPassword, subject.ProxyOptions.ProxyPassword);
        }

        [Fact]
        public void Ctor_IgnoresPassedEmptyProxyOptions_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions();

            // Act
            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyHostName, subject.ProxyOptions.ProxyHostName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPort, subject.ProxyOptions.ProxyPort);
        }

        [Fact]
        public void ApplyServiceDiscoveryOptions_IgnoresPassedProxyOptionsWhenAlreadySet_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions
            {
                RestProxyOptions = CreateNonEmptyProxyOptions()
            };

            var serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ProxyHostName.Data("yet-another-proxy.com");
            serviceDiscoveryOptions.ProxyPort.Data("10000");
            serviceDiscoveryOptions.ProxyUserName.Data("usr");
            serviceDiscoveryOptions.ProxyPassword.Data("passs");

            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Act
            subject.ApplyServiceDiscoveryOptions(serviceDiscoveryOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyHostName, subject.ProxyOptions.ProxyHostName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPort, subject.ProxyOptions.ProxyPort);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyUserName, subject.ProxyOptions.ProxyUserName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPassword, subject.ProxyOptions.ProxyPassword);
        }

        [Fact]
        public void ApplyServiceDiscoveryOptions_AcceptsPassedProxyOptionsWhenDidntSetBefore_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions();

            var serviceDiscoveryOptions = new ReactorServiceDiscoveryOptions();
            serviceDiscoveryOptions.ProxyHostName.Data("yet-another-proxy.com");
            serviceDiscoveryOptions.ProxyPort.Data("10000");
            serviceDiscoveryOptions.ProxyUserName.Data("usr");
            serviceDiscoveryOptions.ProxyPassword.Data("passs");

            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Act
            subject.ApplyServiceDiscoveryOptions(serviceDiscoveryOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(serviceDiscoveryOptions.ProxyHostName.ToString(), subject.ProxyOptions.ProxyHostName);
            Assert.Equal(serviceDiscoveryOptions.ProxyPort.ToString(), subject.ProxyOptions.ProxyPort);
            Assert.Equal(serviceDiscoveryOptions.ProxyUserName.ToString(), subject.ProxyOptions.ProxyUserName);
            Assert.Equal(serviceDiscoveryOptions.ProxyPassword.ToString(), subject.ProxyOptions.ProxyPassword);
        }

        [Fact]
        public void ApplyProxyInfo_IgnoresPassedProxyOptionsWhenAlreadySet_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions
            {
                RestProxyOptions = CreateNonEmptyProxyOptions()
            };

            var connectOptions = new ConnectOptions
            {
                ProxyOptions =
                {
                    ProxyHostName = "yet-another-proxy.com",
                    ProxyPort = "10000",
                    ProxyUserName = "usr",
                    ProxyPassword = "passs",
                }
            };

            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Act
            subject.ApplyProxyInfo(connectOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyHostName, subject.ProxyOptions.ProxyHostName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPort, subject.ProxyOptions.ProxyPort);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyUserName, subject.ProxyOptions.ProxyUserName);
            Assert.Equal(reactorOptions.RestProxyOptions.ProxyPassword, subject.ProxyOptions.ProxyPassword);
        }

        [Fact]
        public void ApplyProxyInfo_AcceptsPassedProxyOptionsWhenDidntSetBefore_Test()
        {
            // Arrange
            var reactorOptions = new ReactorOptions();

            var connectOptions = new ConnectOptions
            {
                ProxyOptions =
                {
                    ProxyHostName = "yet-another-proxy.com",
                    ProxyPort = "10000",
                    ProxyUserName = "usr",
                    ProxyPassword = "passs",
                }
            };

            var subject = new ReactorRestConnectOptions(reactorOptions);

            // Act
            subject.ApplyProxyInfo(connectOptions);

            // Assert
            Assert.NotNull(subject.ProxyOptions);
            Assert.Equal(connectOptions.ProxyOptions.ProxyHostName, subject.ProxyOptions.ProxyHostName);
            Assert.Equal(connectOptions.ProxyOptions.ProxyPort, subject.ProxyOptions.ProxyPort);
            Assert.Equal(connectOptions.ProxyOptions.ProxyUserName, subject.ProxyOptions.ProxyUserName);
            Assert.Equal(connectOptions.ProxyOptions.ProxyPassword, subject.ProxyOptions.ProxyPassword);
        }

        private ProxyOptions CreateNonEmptyProxyOptions() =>
            new()
            {
                ProxyHostName = "prrroxy.com",
                ProxyPort = "3333",
                ProxyUserName = "user",
                ProxyPassword = "password",
            };


    }
}
