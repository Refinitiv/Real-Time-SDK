﻿using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System.Management.Automation;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Submit, "Item")]
public class SubmitItem : Cmdlet
{
    [Parameter(ValueFromPipeline = true)]
    public OmmProvider? Provider { get; set; }

    [Parameter(Mandatory = true)]
    public string? Item { get; set; }

    [Parameter(Mandatory = true)]
    public string? Service { get; set; }

    [Parameter(Mandatory = true, ValueFromPipeline = true)]
    public FieldList? Payload { get; set; }

    [Parameter()]
    public long Handle { get; set; }

    [Parameter()]
    public int DomainType { get; set; } = EmaRdm.MMT_MARKET_BY_ORDER;

    protected override void ProcessRecord()
    {
        Provider?.Submit(new RefreshMsg().DomainType(DomainType).ServiceName(Service).Name(Item)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE)
            .Payload(Payload).Complete(true),
            Handle);
    }
}
