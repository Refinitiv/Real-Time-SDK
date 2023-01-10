# Refinitiv Real-Time SDK -  CSharp using .NET Core Edition
This is a proof of concept ETA CSharp to provide pure .NET Core solution. This preview branch includes transport and value add Reactor layers with both client and server side implemenation. This complements the current offering with Java and C languages for RTSDK API. The transport layer API supports TCP/IP transport, buffer management (such as read, write), fragmentation, packing, compression, a codec to implement open message model (OMM). In addition, the value add layer handles adminitrative messages and implements a dispatching/callback mechanism to simplify the application. The public interfaces of ETA CSharp Edition should have a similar look and feel as Java or C Edition unless there is any language specific differences. 

Copyright (C) 2022 Refinitiv. All rights reserved.

# Software Requirements
- Visual Studio 2022
- .NET Core 6.0 
- xUnit 2.4.1 or higher for unit testing.
- Microsoft.Extensions.Logging 6.0.0 or higher
- Microsoft.Extensions.Configuration 6.0.1 or higher

# Installation & Build

## Install RTSDK

Install all prerequisites list in Software Requirements section. 

Obtain the source **from this repository** on GitHub. It will contain a solution file and all required source to build RTSDK.

## Building RTSDK 

** Using Solution Files and Visual Studio **

Use the provided solution (or `sln`) file to build in **Visual Studio**. 

** Using dotnet **

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command as follows:

	dotnet build --configuration <Release|Debug> ETA.NET.sln

### Running Examples 

Navigate to `RTSDK/CSharp` and issue the appropriate dotnet command to run various examples:

	dotnet [runtime-options] [path-to-application] [arguments]

	Examples:

	* dotnet Eta/Applications/Consumer/bin/Debug/net6.0/Consumer.dll [arguments] 
	* dotnet Eta/Applications/Training/Consumer/Module_1a_Connect/bin/Debug/net6.0/Module_1a_Connect.dll [arguments]

# Notes:
- This ETA .NET is proof of concept for .NET framework implementation of Elektron Transport API and is not fully supported. 
- This section contains APIs that are subject to proprietary and open source licenses. Please make sure to read the readme files within each API flavor directory for clarification.
- Please make sure to review the LICENSE.md file.
