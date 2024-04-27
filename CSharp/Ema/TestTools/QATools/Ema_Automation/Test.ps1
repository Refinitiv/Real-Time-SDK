#Run provider and consumer interactivelly in Powershell console
Import-Module Ema_Automation
$consumer = New-ConsumerConfig -Host 10.187.5.164:14002 -UserName user | Start-Consumer

$job = Start-Job -ScriptBlock {
	Import-Module Ema_Automation
	$provider = New-NiProviderConfig -Host 10.187.5.164:14003 -UserName user | Start-Provider
	Start-Sleep -Seconds 5
	$sentCount = 0
	foreach ($fid in 22..24)
	{
		foreach ($r in 1440..1445)
		{
			$payload = (New-Object LSEG.Ema.Access.FieldList).AddReal($fid, $r, 12).Complete()
			Submit-Item -Service NI_PUB -Item IBM.N -Provider $provider -Payload $payload
			$sentCount++
			Write-Host -NoNewline -ForegroundColor Blue "."
			Start-Sleep -Seconds 1
		}
	}
	Set-Variable -Name ItemsSentCount -Value $sentCount
}

#Process and dump consumer output
$actualCount = 0
$timeout = [timespan]::FromSeconds(20)
Request-Item -Consumer $consumer -Service NI_PUB -Item IBM.N -Timeout $timeout | ForEach-Object { Write-Host $_ }
#| Where-Object -FilterScript { $_.Payload().DataType -EQ 132 } | ForEach-Object { $_.Payload().FieldList() | ForEach-Object { Write-Host -NoNewline -ForegroundColor Green "." } }
Write-Host -ForegroundColor Green "Received: ${actualCount}"

#Dump provider output
Wait-Job -Job $job | Out-Null
$expectedCount = Get-Variable -Name ItemsSentCount
$providerOutput= Receive-Job -Job $job
Write-Host $providerOutput
Write-Host -ForegroundColor Blue "Sent: ${expectedCount}"

#Cleanup
Stop-Provider -Provider $provider
Stop-Consumer -Consumer $consumer
