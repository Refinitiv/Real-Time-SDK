using Xunit.Sdk;
using LSEG.Eta.Tests.Common.Xunit;

[assembly: RegisterXunitSerializer(
    typeof(JsonXunitSerializer),
    typeof(LSEG.Eta.ValueAdd.Reactor.ReactorConnectOptions),
    typeof(LSEG.Eta.ValueAdd.Reactor.ReactorPreferredHostOptions)
)]