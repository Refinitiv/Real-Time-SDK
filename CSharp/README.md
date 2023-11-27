# Refinitiv Real-Time SDK -  CSharp using .NET Core Edition
This is the Refinitiv Real-Time SDK. This SDK encompasses these Real-Time APIs writtent to .NET Core: Enterprise Message API (EMA) and the Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source Refinitiv low-level Transport and OMM encoder/decoder API. It is used by the Refinitiv Real-Time Distribution Systems and Refinitiv Real-Time for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. Applications may be written to core ETA, to ValueAdd/Reactor layer or to Watchlist layer.

Copyright (C) 2022-2023 Refinitiv. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Refinitiv Real-Time SDK - CSharp Edition. Also in CHANGELOG is a list of features and issues fixed in this release and a history per released version.

### External Dependencies

External modules used by this version of RTSDK CSharp:

        Dependency                                 Version
        ----------                                 -------
        K4os.Compression.LZ4                       1.2.16
        Microsoft.Csharp                           4.5.0
        Microsoft.IdentityModel.Abstractions       6.23.1
        Microsoft.IdentityModel.Jsonwebtokends     6.23.1
        Microsoft.IdentityModel.Logging            6.23.1
        Microsoft.IdentityModel.Tokens             6.23.1
        Microsoft.Netcore.Platforms                1.1.0
        Microsoft.Netcore.Targets                  1.1.0
        NLog                                       5.2.2
        NLog.Extensions.Logging                    5.3.2
        System.IdentityModel.Tokens.Jwt            6.23.1
        System.Runtime                             4.3.0
        System.Security.Cryptography.Cng           4.5.0 
        System.Text.Encodings                      4.3.0
        System.Text.Encodings.Web                  4.7.2
        System.Text.Json                           4.7.2


# Software Requirements
- Visual Studio 2022
- .NET Core 6.0 
- xUnit 2.4.1 or higher for unit testing.

### Platforms and Compilers used in Test

        Windows Server 2016 Enterprise Edition or later 64-bit, VS2022
        Oracle Linux Server 7.X 64-bit, .NET SDK 6.0.402
        Red Hat Enterprise Server 7.X Release 64-bit, .NET SDK 6.0.402
        Red Hat Enterprise Server 8.X Release 64-bit, .NET SDK 6.0.402
        CentOS 7.X Release 64-bit, .NET SDK 6.0.402
        Ubuntu 20.04 64-bit, .NET SDK 6.0.402

### Encryption Support

This release supports encryption using TLS 1.2 and TLS 1.3 (tested only on Linux).  

### Proxy Authentication Support
- [Free Proxy](http://www.handcraftedsoftware.org/)

Authentication Schemes:
- Basic

### Interoperability

RTSDK CSharp supports connectivity to the following platforms:

- Refinitiv Real-Time Distribution System (RSSL/RWF connections): ADS/ADH all supported versions
- Refinitiv Real-Time: Refinitiv Real-Time Deployed
- Refinitiv Real-Time Hosted
- Refinitiv Real-Time - Optimized (RTO)
- Refinitiv Direct Feed

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data. Connectivity to RTO is supported with V2 Authentication that requires Service account creation. The ability to create Service credentials is forthcoming. 

This release has been tested with the following:

- ADS 3.7.2
- ADH 3.7.2
- DACS 7.8

# Documentation
  
Please refer to top level README.md and to CSharp/Eta/README.md to find more information. In this directory, please find the test plan with test results: RTSDK-CSharp-Edition\_Test\_Plan.xlsx

# Installation & Build

Please refer to the [RTSDK Installation Guide](Eta/Docs/RTSDK_CSharp_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK

Install all prerequisites list in Software Requirements section. 

Obtain the source **from this repository** on GitHub. It will contain a solution file and all required source to build RTSDK.

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

        dotnet build --configuration <Release|Debug> RTSDK_NET6.0.sln

        NOTE: 
              - In a GitHub build, the command above builds libraries and places them into Eta/Libs or Ema/Libs and examples into Eta/Executables or Ema/Executables
              - In RRG package, it builds only libraries and places them into custom directories: Eta/Custom/Libs, Ema/Custom/Libs

        GitHub Only, to build specific example: dotnet build -t:Consumer --configuration <Release|Debug> RTSDK_NET6.0.sln

To build just libraries:

Building RTSDK using dotnet command lines is platform agnostic; i.e., it works the same way on Linux and Windows platforms. To build using Visual Studio is applicable to only Windows.

        dotnet build --configuration Release Eta/Src/Core/Core_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/ValueAdd/ValueAdd_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/Ansi/Ansi_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/AnsiPage/AnsiPage_NET6.0.csproj
        dotnet build --configuration Release Ema/Src/Core/EMA_Core_NET6.0.csproj

        NOTE: In a GitHub build, this builds libraries and places them into Eta/Libs or Ema/Libs
              In RRG package, this builds libraries and places them into custom directories: Eta/Custom/Libs, Ema/Custom/Libs

To build just examples: Each example may be built separately using the individual csproj files. Please note that the RRG package also contains a .sln file for each Eta example along with individual csproj files for each Ema example. Sample command lines to build examples:

        dotnet build --configuration Release Eta/Applications/Consumer/Consumer_NET6.0.csproj
        dotnet build --configuration Release Eta/Applications/Consumer/Consumer_NET6.0.sln
        dotnet build --configuration Release Ema/Examples/Training/Consumer/100_Series/100_MP_Streaming/Cons100_NET6.0.csproj

        NOTE: The sln and/or csproj files build examples and places them into Eta/Executables or Ema/Executables
              Solution files exist only in RRG package
              In RRG package, building examples via csproj or sln to link to pre-built libraries located in Eta/Libs and/or Ema/Libs
              In a GitHub build, example builds expect libraries in Eta/Libs and/or Ema/Libs to exist

## Running Examples 

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command to run various examples:

        dotnet [runtime-options] [path-to-application-executable] [arguments]

Sample command lines to run examples using .dll:

        dotnet Eta/Applications/Consumer/bin/Debug/net6.0/Consumer.dll [arguments] 
        dotnet Eta/Applications/Training/Consumer/Module_1a_Connect/bin/Debug/net6.0/Module_1a_Connect.dll [arguments]
        dotnet Ema/Examples/Training/Consumer/100_Series/100_MP_Streaming/obj/Release/net6.0/Cons100.dll 

- Linux: Run executable:  ./Consumer [arguments]
- Windows: Run executable:  Consumer.exe [arguments]

NOTE: Specify -? as an argument to get a list of possible arguments where applicable or specify file/programmatic configuration for EMA examples.

# Obtaining the Refinitiv Field Dictionaries

The Refinitiv `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `CSharp/etc`. In addition, the most current version can be downloaded from [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html). Search for "Service Pack" and choose the latest version of Refinitiv Template Service Pack.

# NuGet

For ease of product use, Refinitiv maintains its RTSDK CSharp libraries on NuGet.

You can download RTSDK libraries and dependencies from NuGet. Choose the appropriate set of libraries depending on the layer of RTSDK to which application is being written. Below is *sample* code to build applications.

        <dependency>
                <ItemGroup>
                    <PackageReference Include="LSEG.Eta.Core" Version="3.1.0"/>
                    <PackageReference Include="LSEG.Eta.ValueAdd" Version="3.1.0"/>
                    <PackageReference Include="LSEG.Eta.Ansi" Version="3.1.0"/>
                    <PackageReference Include="LSEG.Eta.AnsiPage" Version="3.1.0"/>
                    <PackageReference Include="LSEG.Ema.Core" Version="3.1.0"/>
                </ItemGroup/>
        </dependency>


# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Entity%20Contributor%20License%20Agreement.pdf)


Please email a signed and scanned copy to sdkagreement@refinitiv.com.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.

# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package/directory contains APIs that are subject to proprietary and open source licenses. Please make sure to read the readme files within each section for clarification.
- Please make sure to review the LICENSE.md file.
