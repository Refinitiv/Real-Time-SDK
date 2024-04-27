using LSEG.Ema.Access;
using System.Management.Automation;
using static LSEG.Ema.Access.OmmNiProviderConfig;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsCommon.New, "NIProviderConfig")]
public class NewNIProviderConfig : Cmdlet
{
    [Parameter()]
    public string? ProviderName { get; set; }
    [Parameter()]
    public string? UserName { get; set; }
    [Parameter()]
    public string? Password { get; set; }
    [Parameter()]
    public string? Position { get; set; }
    [Parameter()]
    public string? ApplicationId { get; set; }
    [Parameter()]
    public string? InstanceId { get; set; }
    [Parameter()]
    public string? Host { get; set; }
    [Parameter()]
    public OperationModelMode? OperationModel { get; set; }
    [Parameter()]
    public AdminControlMode? AdminControlDirectory { get; set; }
    [Parameter()]
    public string? ProxyHost { get; set; }
    [Parameter()]
    public string? ProxyPort { get; set; }
    [Parameter()]
    public string? ProxyUserName { get; set; }
    [Parameter()]
    public string? ProxyPassword { get; set; }
    [Parameter()]
    public uint? EncryptedProtocolFlags { get; set; }
    protected override void ProcessRecord()
    {
        var config = new OmmNiProviderConfig();
        if (ProviderName is not null)
            config.ProviderName(ProviderName);
        if (UserName is not null)
            config.UserName(UserName);
        if (Password is not null)
            config.Password(Password);
        if (Position is not null)
            config.Position(Position);
        if (ApplicationId is not null)
            config.UserName(ApplicationId);
        if (InstanceId is not null)
            config.InstanceId(InstanceId);
        if (Host is not null)
            config.Host(Host);
        if (OperationModel is not null)
            config.OperationModel(OperationModel.Value);
        if (AdminControlDirectory is not null)
            config.AdminControlDirectory(AdminControlDirectory.Value);
        if (ProxyHost is not null)
            config.ProxyHost(ProxyHost);
        if (ProxyPort is not null)
            config.ProxyPort(ProxyPort);
        if (ProxyUserName is not null)
            config.ProxyUserName(ProxyUserName);
        if (ProxyPassword is not null)
            config.ProxyPassword(ProxyPassword);
        if (EncryptedProtocolFlags is not null)
            config.EncryptedProtocolFlags(EncryptedProtocolFlags.Value);
        WriteObject(config);
    }
}
