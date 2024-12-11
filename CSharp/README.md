# LSEG Real-Time SDK -  CSharp using .NET Core Edition
This is the Real-Time SDK. This SDK encompasses these Real-Time APIs writtent to .NET Core: Enterprise Message API (EMA) and the Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source low-level Transport and OMM encoder/decoder API. It is used for optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. Applications may be written to core ETA, to ValueAdd/Reactor layer or to Watchlist layer.

Copyright (C) 2022-2024 LSEG. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Real-Time SDK - CSharp Edition. Also in CHANGELOG is a list of features and issues fixed in this release and a history per released version.

### External Dependencies

External modules used by this version of RTSDK CSharp:

        Dependency                                 Version
        ----------                                 -------
        K4os.Compression.LZ4                       1.3.8
        Microsoft.Csharp                           4.5.0
        Microsoft.IdentityModel.Abstractions       8.1.2 
        Microsoft.IdentityModel.Jsonwebtokends     8.1.2 
        Microsoft.IdentityModel.Logging            8.1.2 
        Microsoft.IdentityModel.Tokens             8.1.2 
        Microsoft.Netcore.Platforms                5.0.0
        NLog                                       5.3.4
        NLog.Extensions.Logging                    5.3.14
        System.IdentityModel.Tokens.Jwt            8.1.2


# Software Requirements
- Visual Studio 2022
- .NET Core 6 or .NET Core 8. NOTE: .NET 8 is used in default build
- xUnit 2.9.2 or higher for unit testing.

### Platforms and Compilers used in Test

        Windows Server 2019 Standard Edition or later 64-bit, .NET SDK 6.0.421
        Windows Server 2022 Standard Edition or later 64-bit, .NET SDK 8.0.403
	Windows 11 64-bit, .NET SDK 8.0.403
        Oracle Linux Server 7.X 64-bit, .NET SDK 6.0.421
        Red Hat Enterprise Server 7.X Release 64-bit, .NET SDK 6.0.421
        Red Hat Enterprise Server 8.X Release 64-bit, .NET SDK 6.0.421
        Red Hat Enterprise Server 8.X Release 64-bit, .NET SDK 8.0.401
        Red Hat Enterprise Server 9.X Release 64-bit, .NET SDK 8.0.401
        Ubuntu 20.04 64-bit, .NET SDK 6.0.421

### Encryption Support

This release supports encryption using TLS 1.2 and TLS 1.3 (tested only on Linux).  

### Proxy Authentication Support
- [Free Proxy](http://www.handcraftedsoftware.org/)

Authentication Schemes:
- Basic

### Interoperability

RTSDK CSharp supports connectivity to the following platforms:

- LSEG Real-Time Distribution System (RSSL/RWF connections): ADS/ADH all supported versions
- LSEG Real-Time Deployed
- LSEG Real-Time Hosted
- LSEG Real-Time - Optimized (RTO)
- Real Time Direct 

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data. Connectivity to RTO is supported with V2 Authentication that requires Service account creation. The ability to create Service credentials is forthcoming. 

This release has been tested with the following:

- ADS 3.8.1
- ADH 3.8.1
- DACS 7.12

# Documentation
  
Please refer to top level README.md and to CSharp/Eta/README.md to find more information. In this directory, please find the test plan with test results: RTSDK-CSharp-Edition\_Test\_Plan.xlsx

# Installation & Build

Please refer to the [RTSDK Installation Guide](Eta/Docs/RTSDK_CSharp_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK

Install all prerequisites list in Software Requirements section. 

Obtain the source **from this repository** on GitHub. It will contain a solution file and all required source to build RTSDK.

Real-Time SDK package may also be [downloaded from LSEG Developer Portal](https://developers.lseg.com/en/api-catalog/real-time-opnsrc/rt-sdk-csharp/downloads). In addition, these distributions depend on a Binary Pack found in the above downloads section. This will not be automatically pulled by the build, and must be downloaded and extracted into the ../RTSDK-BinaryPack directory(Same level as the CSharp directory in this package). The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product.

Real-Time SDK package is also available on [MyAccount](https://myaccount.lseg.com/content/mytr/en/downloadcenter.html). In addition, these distributions depend on a Binary Pack found in the above downloads section. This will not be automatically pulled by the build, and must be downloaded and extracted into the ../RTSDK-BinaryPack directory(Same level as the CSharp directory in this package). The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product.

## Building RTSDK 

**Using Solution Files and Visual Studio**

Use the provided solution (or `sln`) file to build in **Visual Studio**. 

**Using dotnet**

The RRG package contains all required external dependencies in the CSharp/NuGetPackages directory. In an environment without internet access, you must add this directory as a nuget source and disable other nuget sources for a build to succeed. Here are some dotnet commands to do so:

        To check existing NuGet sources:   
               dotnet nuget list source

        To add a new NuGet source:
              dotnet nuget add source <full path to your RRG package/CSharp/NuGetPackages>.

        To disable certain NuGet sources:
              dotnet nuget disable source <specify a source show in the list>.
              Example: dotnet nuget disable source "nuget.org"


To build, navigate to `RTSDK/CSharp` and issue the appropriate dotnet command as follows to build libraries and/or examples:

        dotnet build --configuration <Release|Debug> -p:EsdkTargetFramework=<all|net6.0|net8.0> RTSDK.sln

        NOTE: 
              - In a GitHub build, the command above builds libraries and places them into Eta/Libs or Ema/Libs and examples into Eta/Executables or Ema/Executables
              - In RRG package, it builds only libraries and places them into custom directories: Eta/Custom/Libs, Ema/Custom/Libs

        GitHub Only, to build specific example: dotnet build -t:Consumer --configuration <Release|Debug> -p:EsdkTargetFramework=<all|net6.0|net8.0> RTSDK.sln

To build just libraries:

Building RTSDK using dotnet command lines is platform agnostic; i.e., it works the same way on Linux and Windows platforms. To build using Visual Studio is applicable to only Windows. Sample commands:

        dotnet build --configuration Release -p:EsdkTargetFramework=net8.0 Eta/Src/Core/Core.csproj
        dotnet build --configuration Release -p:EsdkTargetFramework=all Eta/Src/ValueAdd/ValueAdd.csproj
        dotnet build --configuration Release -p:EsdkTargetFramework=net6.0 Eta/Src/Ansi/Ansi.csproj
        dotnet build --configuration Release -p:EsdkTargetFramework=all Eta/Src/AnsiPage/AnsiPage.csproj
        dotnet build --configuration Release -p:EsdkTargetFramework=all Ema/Src/Core/EMA_Core.csproj

        NOTE: In a GitHub build, this builds libraries and places them into Eta/Libs or Ema/Libs
              In RRG package, this builds libraries and places them into custom directories: Eta/Custom/Libs, Ema/Custom/Libs

To build just examples: Each example may be built separately using the individual csproj files. Please note that the RRG package also contains a .sln file for each Eta example along with individual csproj files for each Ema example. Sample command lines to build examples:

        dotnet build --configuration Release -p:EsdkTargetFramework=net8.0 Eta/Applications/Consumer/Consumer.csproj
        dotnet build --configuration Release -p:EsdkTargetFramework=all Eta/Applications/Consumer/Consumer.sln
        dotnet build --configuration Release -p:EsdkTargetFramework=net6.0 Ema/Examples/Training/Consumer/100_Series/100_MP_Streaming/Cons100.csproj

        NOTE: The sln and/or csproj files build examples and places them into Eta/Executables or Ema/Executables
              Solution files exist only in RRG package
              In RRG package, building examples via csproj or sln to link to pre-built libraries located in Eta/Libs and/or Ema/Libs
              In a GitHub build, example builds expect libraries in Eta/Libs and/or Ema/Libs to exist

## Running Examples 

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command to run various examples:

        dotnet [runtime-options] [path-to-application-executable] [arguments]

Sample command lines to run examples using .dll:

        dotnet Eta/Applications/Consumer/bin/Debug/net8.0/Consumer.dll [arguments] 
        dotnet Eta/Applications/Training/Consumer/Module_1a_Connect/bin/Debug/net8.0/Module_1a_Connect.dll [arguments]
        dotnet Ema/Examples/Training/Consumer/100_Series/100_MP_Streaming/obj/Release/net8.0/Cons100.dll 

- Linux: Run executable:  ./Consumer [arguments]
- Windows: Run executable:  Consumer.exe [arguments]

NOTE: Specify -? as an argument to get a list of possible arguments where applicable or specify file/programmatic configuration for EMA examples.

# Obtaining the Field Dictionaries

The `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `CSharp/etc`. In addition, the most current version can be downloaded from [MyAccount](https://myaccount.lseg.com/en/downloadcenter). Search for "MDS - General" and "Real-Time Service Pack" and choose the latest version of Real-Time Template Service Pack.

# NuGet

For ease of product use, LSEG maintains its RTSDK CSharp libraries on NuGet.

You can download RTSDK libraries and dependencies from NuGet. Choose the appropriate set of libraries depending on the layer of RTSDK to which application is being written. Below is *sample* code to build applications.

        <dependency>
                <ItemGroup>
                    <PackageReference Include="LSEG.Eta.Core" Version="3.3.1"/>
                    <PackageReference Include="LSEG.Eta.ValueAdd" Version="3.3.1"/>
                    <PackageReference Include="LSEG.Eta.Ansi" Version="3.3.1"/>
                    <PackageReference Include="LSEG.Eta.AnsiPage" Version="3.3.1"/>
                    <PackageReference Include="LSEG.Ema.Core" Version="3.3.1"/>
                </ItemGroup/>
        </dependency>


# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Real-Time%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Real-Time%20API%20Entity%20Contributor%20License%20Agreement.pdf)


Please email a signed and scanned copy to sdkagreement@lseg.com. If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.

# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package/directory contains APIs that are subject to proprietary and open source licenses. Please make sure to read the readme files within each section for clarification.
- Please make sure to review the LICENSE.md file.
