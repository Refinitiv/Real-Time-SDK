#|-----------------------------------------------------------------------------
#|            This source code is provided under the Apache 2.0 license
#|  and is provided AS IS with no warranty or guarantee of fit for purpose.
#|                See the project's LICENSE.Md for details.
#|           Copyright (C) 2024 LSEG. All rights reserved.     
#|-----------------------------------------------------------------------------
#/

#Detaches examples from QATools solution.

$examplespath = $PSScriptRoot+"\..\..\Examples\Training\"

$aliases = @{ 'IProvider' = 'Provider'}

function Get-Alias($Name)
{
    if($aliases.ContainsKey($Name))
    {
        return $aliases[$Name]
    }
    return $Name
}

Set-Location $PSScriptRoot
Get-ChildItem -Path $examplespath -Filter *.csproj -Recurse -ErrorAction SilentlyContinue -Force | ForEach-Object {
    $series = $_.Directory.Parent.Name.Substring(0,3)
    $number = $_.Directory.Name.Substring(0,3)
    $type = $_.Directory.Parent.Parent.Name
    $alias = Get-Alias -Name $type
    dotnet sln QATools.sln remove Series$series$alias$number\$alias$number.csproj
    Remove-Item -LiteralPath Series$series$alias$number -Force -Recurse
}