#|-----------------------------------------------------------------------------
#|            This source code is provided under the Apache 2.0 license
#|  and is provided AS IS with no warranty or guarantee of fit for purpose.
#|                See the project's LICENSE.Md for details.
#|           Copyright (C) 2024 LSEG. All rights reserved.     
#|-----------------------------------------------------------------------------
#/

#Attaches examples into QATools solution which reference original source files allowing in-solution comparison.

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
    New-Item -ItemType Directory -Force -Path Series$series$alias$number
    Push-Location Series$series$alias$number
    $csProjContent = 
@"
<Project Sdk="Microsoft.NET.Sdk">
    <Import Project="..\$alias.props"/>
    <ItemGroup>	
        <Compile Include="..\..\..\Examples\Training\$type\$($_.Directory.Parent.Name)\$($_.Directory.Name)\${type}.cs" />
        <None Include="..\..\..\Examples\Training\$type\$($_.Directory.Parent.Name)\$($_.Directory.Name)\ReadMe.txt" />
    </ItemGroup>
</Project>
"@
    New-Item $alias$number.csproj -ItemType File -Force -Value $csProjContent
    Pop-Location
    dotnet sln QATools.sln add --solution-folder "$alias\Series $series" Series$series$alias$number\$alias$number.csproj
}