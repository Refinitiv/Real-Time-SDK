/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Ema.Access.Tests.OmmConfigTests.ConfigTestsUtils;

namespace LSEG.Ema.Access.Tests.OmmConfigTests
{
    public class PopulateReactorOptionsFromConfigTests
    {
        [Fact]
        public void OverrideRestProxyHostInXmlConfigTest()
        {
            // Arrange
            var overridingValue = "overriden.proxy";

            var reactorOptions = new ReactorOptions();
            var config = LoadEmaTestConfig()
                .RestProxyHostName(overridingValue);

            // Act
            config.OmmConsConfigImpl.PopulateReactorOptions(reactorOptions);

            // Assert
            var restProxyOpts = reactorOptions.RestProxyOptions;
            Assert.Equal(overridingValue, restProxyOpts.ProxyHostName);
        }

        [Fact]
        public void OverrideRestProxyPortInXmlConfigTest()
        {
            // Arrange
            var overridingValue = "8888";

            var reactorOptions = new ReactorOptions();
            var config = LoadEmaTestConfig()
                .RestProxyPort(overridingValue);

            // Act
            config.OmmConsConfigImpl.PopulateReactorOptions(reactorOptions);

            // Assert
            var restProxyOpts = reactorOptions.RestProxyOptions;
            Assert.Equal(overridingValue.ToString(), restProxyOpts.ProxyPort);
        }

        [Fact]
        public void SetRestProxyUserNameTest()
        {
            // Arrange
            var valueToSet = "lsjkdflsksklf";
            var reactorOptions = new ReactorOptions();
            var config = new OmmConsumerConfig()
                .RestProxyUserName(valueToSet);

            // Act
            config.OmmConsConfigImpl.PopulateReactorOptions(reactorOptions);

            // Assert
            var restProxyOpts = reactorOptions.RestProxyOptions;
            Assert.Equal(valueToSet, restProxyOpts.ProxyUserName);
        }

        [Fact]
        public void SetRestProxyPasswordTest()
        {
            // Arrange
            var valueToSet = "lsjkdflsksklf";
            var reactorOptions = new ReactorOptions();
            var config = new OmmConsumerConfig()
                .RestProxyPassword(valueToSet);

            // Act
            config.OmmConsConfigImpl.PopulateReactorOptions(reactorOptions);

            // Assert
            var restProxyOpts = reactorOptions.RestProxyOptions;
            Assert.Equal(valueToSet, restProxyOpts.ProxyPassword);
        }
    }
}
