# Elektron SDK - C/C++ Edition
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist.

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is an open source Refinitiv low-level Transport and OMM encoder/decoder API. It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages.

Copyright (C) 2019 Refinitiv. All rights reserved,

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Elektron SDK - C/C++ Edition. Also in CHANGELOG is a list of issues fixed in this release and a history of features and fixes introduced per released version. 

### Supported Platforms, OSs, Compilers

#### Hardware/OS Requirements

- HP Intel PC or AMD Opteron (64-bit)
- CPUs must have high resolution timer frequencies greater than 1GHz.

- Red Hat Advanced Server 6.0 Release 64-bit
- Oracle Linux Server 6.0 or 7.0 Release 64-bit
- CentOS 7.0 Release 64-bit

- TCP/IP networking support installed if using TCP Socket connection types
- UDP Multicast networking support if using Reliable Multicast connection type
- Appropriate system access permissions when using Unidirectional Shared Memory connection types

* For best performance, ensure that machines and network switches are configured appropriately. See specific OS and hardware documentation for more information.

Users are welcome to migrate open source code to the platforms they prefer, however support for the included libraries is only provided on platforms listed below.

#### Supported Platforms

##### Windows

Platforms:

	Microsoft Windows Server 2008 Enterprise Edition or later 64-bit
	Microsoft Windows Server 2012 Enterprise Edition or later 64-bit
	Microsoft Windows Server 2016 Enterprise Edition or later 64-bit
	Microsoft Windows 7 Professional or later 64-bit
	Microsoft Windows 8 Professional or later 64-bit
	Microsoft Windows 8.1 Professional or later 64-bit
	Microsoft Windows 10 Professional 64-bit 

Compilers (only on OSs supported by Microsoft): 

	Microsoft Visual Studio 11.0 (2012) 64-bit
	Microsoft Visual Studio 12.0 (2013) 64-bit
	Microsoft Visual Studio 14.0 (2015) 64-bit
	Microsoft Visual Studio 15.0 (2017) 64-bit 

NOTE: User has the option to use pre-built libraries for the compilers listed above and use them on different Windows operating systems that have support for those compilers to build their applications. User may also choose to build source and applications. 

##### Linux

Platforms & Compilers:

	GCC compiler suite version 4.4.4 or higher for RHAS 6.X, 64-bit, Native build
	GCC compiler suite version 4.8.2 or higher for Oracle Linux 7.X, 64-bit, Native build
	GCC compiler suite version 4.4.4 or higher for Oracle Linux 6.0, 64-bit, qualification with RH6 library build
	GCC compiler suite version 4.8.2 or higher for CentOS 7.0, 64-bit, qualification with OL7 library build

* Eta VACache library built 

NOTE: User has the option to use pre-built libraries or build source natively on a platform of choice. Pre-built libraries for Oracle Linux 7 and Red Hat 6 are available in release packages available on Refinitiv Developer Portal. 

#### Tested Versions
External modules used by this version of ESDK C/C++:

	Dependency		Version
	----------		-------
	openSSL 		1.0.1e
	openSSL 		1.1.1a
	cJSON			v1.7.10
	curl			7.63.0
	googletest		release-1.8.1
	libxml2			2.9.9
	lz4			1.8.3
	zlib			1.2.11

##### Windows
This release has been tested with supported valid OS/compiler combinations.

##### Linux
This release has been tested with the following on supported platform/OS combinations. Please note that the list of tested platforms and compiler combination below reflects test of two use cases: using pre-built libraries to build applications _and_ natively building source and using those libraries to build applications.

	Red Hat Advanced Server 6.4 64-bit with GCC 4.4.4 (RHEL6_64_GCC444) - prebuilt libraries
	Red Hat Advanced Server 6.3 64-bit with GCC 4.4.6 (RHEL6_64_GCC446) - natively built libraries
	Red Hat Advanced Server 6.9 64-bit with GCC 4.4.7 (RHEL6_64_GCC447) - natively built libraries
	Red Hat Advanced Server 7.4 64-bit with GCC 4.8.5 (RHEL7_64_GCC485) - prebuilt libraries 
	Oracle Linux Server 6.4 64-bit with GCC 4.4.4 (RHEL6_64_GCC444) - prebuilt libraries
	Oracle Linux Server 7.0 64-bit with GCC 4.8.2 (OL7_64_GCC482) - prebuilt & natively built libraries 
	Oracle Linux Server 7.1 64-bit with GCC 4.8.3 (OL7_64_GCC483) - prebuilt & natively built libraries
	Oracle Linux Server 7.5 64-bit with GCC 4.8.5 (OL7_64_GCC485) - prebuilt & natively built libraries
	CentOS 7.0 64-bit with GCC 4.8.2 (OL7_64_GCC482) - prebuilt libraries
	Oracle Linux Server 7.6 64-bit with GCC 7.4.0 (OL7_64_GCC740) - natively built libraries
	Red Hat Linux Server 7.6 64-bit with GCC 7.4.0 (RHEL7_64_GCC740) - natively built libraries
	Red Hat Linux Server 6.10 64-bit with GCC 7.4.0 (RHEL6_64_GCC740) - natively built libraries

#### Proxy Authentication Support

Proxies:

- Microsoft's Forefront
- [Free Proxy](http://www.handcraftedsoftware.org/)

Authentication Schemes:

- Basic
- NTLM
- NEGOTIATE/Kerberos ( Windows only )

#### Encryption Support

This release supports encryption for TLS 1.2.  


### Interoperability

ESDK Java supports connectivity to the following platforms:

- Enterprise Platform for Real-Time (RSSL/RWF connections) : ADS version 2.6 and higher, ADH version 2.6 and higher. 
- Elektron: Elektron Deployed, Elektron Hosted, Elektron Direct Feed

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data.

This release has been tested with the following:

- ADS 3.3.3
- ADH 3.3.3
- DACS 7.2

# Documentation

Please refer to top level README.md and to Cpp-C/Eta/README.MD or Cpp-C/Ema/README.MD files to find more information. Sections Cpp-C/Eta/Docs/ and Cpp-C/Ema/Docs contain PDF/HTML documentation. In current directory, please find the test plan with test results: ESDK-C-Edition\_Test\_Plan.xlsx

# Installation and Build

Please refer to Installation Guide for [ETA](Cpp-C/Eta/Docs/ESDK_C_Installation_Guide.pdf) or [EMA](Cpp-C/Ema/Docs/ESDK_C_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install ESDK 
There are 3 ways to install Eletron SDK:

Obtain the source **from this repository** on GitHub. It will contain all of the required source to build ESDK as detailed below. In addition, this repository depends on the [Elektron-SDK-BinaryPack](http://www.github.com/Refinitiv/Elektron-SDK-BinaryPack) repository. The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product. Please note that the build will auto pull the appropriate BinaryPack which is also available in the [releases section on GitHub](https://github.com/Refinitiv/Elektron-SDK/releases).

Elektron SDK package may also be [downloaded from Refinitiv Developer Portal](https://developers.refinitiv.com/elektron/elektron-sdk-cc/downloads).

Elektron SDK package is also available on [Customer Zone](https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx). 

- Category: "MDS - API"
- Product: "Elektron SDK"

## Building ESDK

**Using CMake**:

Cmake is required to create the Linux Makefile files and Windows Solution and vcxproj files. To build examples or re-build libraies, user must download [CMake](https://cmake.org).

Refer to the ESDK C/C++ Installation Guide located in Cpp-C/Eta/Docs or Cpp-C/Ema/Docs for more detailed CMake build instructions than what is described below.

**For Linux**:

At the same directory level as the resulting Elektron-SDK directory, issue the following command to build the optimized Makefile files:

	cmake -HElektron-SDK -Bbuild-esdk
	# Elektron-SDK is the ESDK directory 
	# build-esdk is the directory where all build output is placed 
	# Note: build-esdk is automatically created

Issue the following command to build debug Makefile files:

	cmake -HElektron-SDK -Bbuild-esdk –DCMAKE_BUILD_TYPE=Debug

The cmake command builds all needed Makefile files (and related dependencies) in the build-esdk directory. 
Go to the build-esdk directory and type "make" to create the ESDK libraries. Note that the libraries are sent to the Elektron-SDK directory (i.e., not the build-esdk directory).

**For Windows**:

At the same directory level as the resulting Elektron-SDK directory, issue the following command to build the Solution and vcxproj files:

	cmake -HElektron-SDK -Bbuild-esdk -G "VisualStudioVersion"
	# Elektron-SDK is the ESDK directory with the source code 
	# build-esdk is the directory where all build output is placed; this is where the built binaries are placed 
	# Note: build-esdk is automatically created
	# "VisualStudioVersion" is the visual studio version to use for build on windows (e.g., "Visual Studio 14 2015 Win64")
	# Note: A list of visual studio versions can be obtained by typing "cmake -help". 

The cmake command builds all needed Solution and vcxproj files (and other related files) in the build-esdk directory. User must open these files and build all libraries and examples in the same manner as with prior ESDK versions. Note that the build output is sent to the Elektron-SDK directory (i.e., not the build-esdk directory).

Note that only the following Windows compilers are supported.

- Visual Studio 15 2017
- Visual Studio 14 2015
- Visual Studio 12 2013
- Visual Studio 11 2012

Note that your installation of Visual Studio needs to be updated to add Microsoft Foundation Classes per Microsoft when encountering this build error: fatal error RC105: cannot open include file 'afxres.h'.

**32 bit support**:

CMake has build support for 32 bit platforms.

Linux: Add "-DBUILD\_32\_BIT\_ETA=ON" to the cmake build

Windows: Do not add "Win64" to the "VisualStudioVersion".  Example, When specifying, "Visual Studio 14 2015", for a 64-bit build it would be "Visual Studio 14 2015 Win64". For a 32-bit build, it would be "Visual Studio 14 2015" 

Starting with ESDK1.3.1, DACS and ANSI libraries are available for 32-bits in the BinaryPack.

# Obtaining the Refinitiv Field Dictionaries

The Refinitiv `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `Java/etc`. In addition, the most current version can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Elektron-SDK/blob/master/Elektron%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Elektron-SDK/blob/master/Elektron%20API%20Entity%20Contributor%20License%20Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@refinitiv.com.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.


# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each package for clarification.
- Please make sure to review the LICENSE.md file.
