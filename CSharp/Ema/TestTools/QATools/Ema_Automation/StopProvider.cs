using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Stop, "Provider")]
public class StopProvider : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmProvider? Provider { get; set; }

    protected override void ProcessRecord() 
    {
        WriteVerbose("Provider uninitialized");
        Provider?.Uninitialize();
    }
}