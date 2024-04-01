using LSEG.Eta.ValueAdd.Reactor;
using static LSEG.Ema.Access.Tests.OmmConsumerConfigTests.ConfigTestsUtils;

namespace LSEG.Ema.Access.Tests.OmmConsumerConfigTests
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
