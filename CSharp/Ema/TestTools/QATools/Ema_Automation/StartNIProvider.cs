/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

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
