using LSEG.Ema.Access;
using System.Management.Automation;
using static LSEG.Ema.Access.OmmIProviderConfig;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsCommon.New, "ProviderConfig")]
public class NewProviderConfig : Cmdlet
{
    [Parameter()]
    public string? ProviderName { get; set; }
    [Parameter()]
    public OperationModelMode? OperationModel { get; set; }
    [Parameter()]
    public AdminControlMode? AdminControlDirectory { get; set; }
    [Parameter()]
    public uint? EncryptedProtocolFlags { get; set; }
    protected override void ProcessRecord()
    {
        var config = new OmmIProviderConfig();
        if (ProviderName is not null)
            config.ProviderName(ProviderName);
        if (OperationModel is not null)
            config.OperationModel(OperationModel.Value);
        if (AdminControlDirectory is not null)
            config.AdminControlDirectory(AdminControlDirectory.Value);
        if (EncryptedProtocolFlags is not null)
            config.EncryptedProtocolFlags(EncryptedProtocolFlags.Value);
        WriteObject(config);
    }
}
