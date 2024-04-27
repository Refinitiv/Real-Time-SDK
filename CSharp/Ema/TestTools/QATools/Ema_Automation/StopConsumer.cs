using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Stop, "Consumer")]
public class StopConsumer : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmConsumer? Consumer { get; set; }

    protected override void ProcessRecord() 
    {
        WriteVerbose("Consumer uninitialized");
        Consumer?.Uninitialize();
    }
}