using LSEG.Ema.Access;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsCommon.New, "ProgrammaticConfigData")]
public class NewProgrammaticConfigData : Cmdlet
{
    protected override void ProcessRecord() =>
        WriteObject(new Map());
}
