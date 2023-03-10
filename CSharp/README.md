# Refinitiv Real-Time SDK -  CSharp using .NET Core Edition

This is the Refinitiv Real-Time SDK. This SDK consists of the Enterprise Transport API (ETA) written to .NET Core. In this initial release of this flavor of API, ETA fully supports all OMM constructs and messages. Applications maybe written to core ETA or to ValueAdd/Reactor layer with both client and server side implementation. Please note that a client side watchlist implementation and the DACSLock library for use by server applications are forthcoming. The **Enterprise Message API (EMA)** layer is also forthcoming.

This API flavor complements the current offering with Java and C languages for RTSDK API. The transport layer API supports TCP/IP transport, buffer management (such as read, write), fragmentation, packing, compression, a codec to implement open message model (OMM). In addition, the value add layer handles adminitrative messages and implements a dispatching/callback mechanism to simplify the application. The public interfaces of ETA CSharp Edition should have a similar look and feel as Java or C Edition unless there is any language specific differences. 

Copyright (C) 2022-2023 Refinitiv. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Refinitiv Real-Time SDK - CSharp Edition. Also in CHANGELOG is a list of features and issues fixed in this release and a history per released version.

### External Dependencies

External modules used by this version of RTSDK CSharp:

        Dependency                             Version
        ----------                             -------
        K4os.Compression.LZ4                   1.2.16
        Microsoft.IdentityModel.Tokens         6.23.1
        System.IdentityModel.Tokens.Jwt        6.23.1



# Software Requirements
- Visual Studio 2022
- .NET Core 6.0 
- xUnit 2.4.1 or higher for unit testing.

### Platforms and Compilers used in Test

	Windows Server 2016 Enterprise Edition or later 64-bit
	Oracle Linux Server 7.X 64-bit, GCC 4.8.2 (JNI Libraries)
	Red Hat Enterprise Server 8.X Release 64-bit, GCC 8.3.1
	CentOS 7.X Release 64-bit Qualification 

	Microsoft Visual Studio 2022 64-bit 

### Encryption Support

This release supports encryption using TLS 1.2.  

### Proxy Authentication Support
- [Free Proxy](http://www.handcraftedsoftware.org/)

Authentication Schemes:
- Basic

### Interoperability

RTSDK CSharp supports connectivity to the following platforms:

- Refinitiv Real-Time Distribution System (RSSL/RWF connections): ADS/ADH all supported versions
- Refinitiv Real-Time: Refinitiv Real-Time Deployed
- Refinitiv Real-Time Hosted
- Refinitiv Real-Time - Optimized
- Refinitiv Direct Feed

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data.

This release has been tested with the following:

- ADS 3.6.3
- ADH 3.6.3
- DACS 7.8

# Documentation
  
Please refer to top level README.md and to CSharp/Eta/README.md to find more information. In this directory, please find the test plan with test results: RTSDK-CSharp-Edition\_Test\_Plan.xlsx

# Installation & Build

Please refer to the Installation Guide for [ETA](Eta/Docs/RTSDK_CSharp_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK

Install all prerequisites list in Software Requirements section. 

Obtain the source **from this repository** on GitHub. It will contain a solution file and all required source to build RTSDK.

## Building RTSDK 

**Using Solution Files and Visual Studio**

Use the provided solution (or `sln`) file to build in **Visual Studio**. 

**Using dotnet**

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command as follows to build libraries and/or examples:

        dotnet build --configuration <Release|Debug> ETA_NET6.0.sln

        NOTE: In a GitHub build, this builds libraries and places them into Eta/Libs and examples into Eta/Executables
              In RRG package, this builds only libraries and places them into a custom directory: Eta/Custom/Libs

To build just libraries:

        dotnet build --configuration Release Eta/Src/Core/Core_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/ValueAdd/ValueAdd_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/Ansi/Ansi_NET6.0.csproj
        dotnet build --configuration Release Eta/Src/AnsiPage/AnsiPage_NET6.0.csproj

        NOTE: In a GitHub build, this builds libraries and places them into Eta/Libs
              In RRG package, this builds libraries and places them into a custom directory: Eta/Custom/Libs

To build just examples: Each example may be built separately using the individual csproj files. Please note that the RRG package also contains a .sln file for each example. Sample command line to build examples:

        dotnet build --configuration Release Eta/Applications/Consumer/Consumer_NET6.0.csproj
        dotnet build --configuration Release Eta/Applications/Consumer/Consumer_NET6.0.sln

        NOTE: Both sln and csproj files build examples and places them into Eta/Executables. 
              Solution files exist only in RRG package
              In RRG package, building examples via csproj or sln link to pre-built libraries located in Eta/Libs
              In a GitHub build, each example expects libraries in Eta/Libs to exist

## Running Examples 

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command to run various examples:

        dotnet [runtime-options] [path-to-application] [arguments]

Sample command lines to run examples:

        dotnet Eta/Applications/Consumer/bin/Debug/net6.0/Consumer.dll [arguments] 
        dotnet Eta/Applications/Training/Consumer/Module_1a_Connect/bin/Debug/net6.0/Module_1a_Connect.dll [arguments]

- Linux: Run executable:  ./Consumer [arguments]
- Windows: Run executable:  Consumer.exe [arguments]

NOTE: Specify -? as an argument to get a list of possible arguments

# Obtaining the Refinitiv Field Dictionaries

The Refinitiv `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `CSharp/etc`. In addition, the most current version can be downloaded from [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html). Search for "Service Pack" and choose the latest version of Refinitiv Template Service Pack.

# NuGet

For ease of product use, Refinitiv maintains its RTSDK CSharp libraries on NuGet.

You can download RTSDK libraries and dependencies from NuGet. Below is *sample* code to build applications.

        <dependency>
                <ItemGroup>
                    <PackageReference Include="LSEG.Eta.Core" Version="3.0.1"/>
                    <PackageReference Include="LSEG.Eta.ValueAdd" Version="3.0.1"/>
                    <PackageReference Include="LSEG.Eta.Ansi" Version="3.0.1"/>
                    <PackageReference Include="LSEG.Eta.AnsiPage" Version="3.0.1"/>
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
- This section contains APIs that are subject to proprietary and open source licenses. Please make sure to read the readme files within each API flavor directory for clarification.
- Please make sure to review the LICENSE.md file.
