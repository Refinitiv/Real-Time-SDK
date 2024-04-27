using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Start, "IProvider")]
public class StartIProvider : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmIProviderConfig? Config { get; set; }

    [Parameter(ValueFromPipeline = true)]
    public Map? ConfigData { get; set; }

    protected override void ProcessRecord()
    {
        var provider = new OmmProvider(Config ?? new OmmIProviderConfig());
        WriteVerbose("Provider started");
        WriteObject(provider);
    }
}
