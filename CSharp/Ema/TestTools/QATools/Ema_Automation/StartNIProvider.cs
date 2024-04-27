using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Start, "NIProvider")]
public class StartNIProvider : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmNiProviderConfig? Config { get; set; }

    [Parameter(ValueFromPipeline = true)]
    public Map? ConfigData { get; set; }

    protected override void ProcessRecord()
    {
        var provider = new OmmProvider(Config ?? new OmmNiProviderConfig());
        WriteVerbose("Provider started");
        WriteObject(provider);
    }
}
