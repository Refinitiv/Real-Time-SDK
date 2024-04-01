namespace LSEG.Ema.Access.Tests.OmmConsumerConfigTests
{
    public static class ConfigTestsUtils
    {
        public static OmmConsumerConfig LoadEmaTestConfig() => new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaTestConfig.xml");
        public static OmmConsumerConfig LoadEmaBlankConfig() => new OmmConsumerConfig("../../../OmmConsumerConfigTests/EmaBlankConfig.xml");
    }
}
