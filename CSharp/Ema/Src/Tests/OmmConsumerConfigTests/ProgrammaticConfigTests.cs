using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static LSEG.Ema.Access.Tests.OmmConsumerConfigTests.ConfigTestsUtils;

namespace LSEG.Ema.Access.Tests.OmmConsumerConfigTests
{
    public class ProgrammaticConfigTests
    {
        [Fact]
        public void RestProxyParamsLoadTest()
        {
            // Arrange
            var expectedProxyHost = "some-proxy.com";
            var expectedProxyPort = "1234";
            var consumerConfig = LoadEmaBlankConfig();
            var configImpl = consumerConfig.OmmConsConfigImpl;

            var emaConfigMap = new Map();
            var groupList = new ElementList();
            var innerMap = new Map();
            var objectPropertiesList = new ElementList();

            emaConfigMap
                .Clear()
                .AddKeyAscii("ConsumerGroup", MapAction.ADD,
                    groupList
                        .Clear()
                        .AddAscii("DefaultConsumer", "ProgConsumer_1")
                        .AddMap("ConsumerList",
                            innerMap
                                .Clear()
                                .AddKeyAscii("ProgConsumer_1", MapAction.ADD,
                                    objectPropertiesList
                                        .Clear()
                                        .AddAscii("RestProxyHostName", expectedProxyHost)
                                        .AddAscii("RestProxyPort", expectedProxyPort)
                                        .Complete())
                                .Complete())
                        .Complete())
                .Complete();

            // Act
            configImpl.Config(emaConfigMap);

            // Assert
            var namedConsumerConfig = configImpl.ConsumerConfigMap[configImpl.DefaultConsumer];
            Assert.Equal(expectedProxyHost, namedConsumerConfig.RestProxyHostName);
            Assert.Equal(expectedProxyPort, namedConsumerConfig.RestProxyPort);
        }
    }
}
