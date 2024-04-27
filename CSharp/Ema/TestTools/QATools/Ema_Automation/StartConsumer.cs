using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Start, "Consumer")]
public class StartConsumer : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmConsumerConfig? Config { get; set; }

    [Parameter(ValueFromPipeline = true)]
    public Map? ConfigData { get; set; }

    protected override void ProcessRecord()
    {
        var consumer = new OmmConsumer(Config ?? new OmmConsumerConfig());
        WriteVerbose("Consumer started");
        WriteObject(consumer);
    }
}
