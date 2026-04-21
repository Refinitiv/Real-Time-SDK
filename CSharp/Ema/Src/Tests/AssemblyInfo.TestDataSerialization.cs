using Xunit.Sdk;
using LSEG.Eta.Tests.Common.Xunit;

[assembly: RegisterXunitSerializer(
    typeof(JsonXunitSerializer),
    typeof(LSEG.Ema.Access.OmmConsumerConfigImpl),
    typeof(LSEG.Ema.Access.Tests.OmmConsumerTests.ModifyIOCtlTest.IOCtlSetting),
    typeof(LSEG.Ema.Access.Tests.OmmIProviderTests.ModifyIOCtlTest.IOCtlSetting),
    typeof(LSEG.Ema.Access.OmmConsumerConfigImpl)
)]