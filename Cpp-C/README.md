# Refinitiv Real-Time SDK - C/C++ Edition
This is the Refinitiv Real-Time SDK. This SDK encompasses a couple of APIs:  Enterprise Message API (EMA) and the Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source Refinitiv low-level Transport and OMM encoder/decoder API. It is used by the Refinitiv Real-Time Distribution Systems and Refinitiv Real-Time for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. Applications may be written to core ETA, to ValueAdd/Reactor layer or to Watchlist layer.

Copyright (C) 2019-2021 Refinitiv. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Refinitiv Real-Time SDK - C/C++ Edition. Also in CHANGELOG is a list of issues fixed in this release and a history of features and fixes introduced per released version.

### External Dependencies

External modules used by this version of RTSDK C/C++:

	Dependency		Version
	----------		-------
	openSSL 		1.0.1e
	openSSL			1.1.1a
	cJSON			v1.7.10
	curl			7.78.0
	googletest		release-1.8.1
	libxml2			2.9.12
	lz4			1.8.3
	zlib			1.2.11

- Please note that curl and openSSL are dynamically loaded at runtime.  
- Above mentioned version of openSSL was used in test. Please note that the RTSDK package does not build OpenSSL, and we recommend that all installed versions of OpenSSL are patched to the latest version available. 
- Check installation guide for details regarding including external dependencies for build 
   

### Supported Platforms, OSs, Compilers

#### Hardware/OS Requirements

- HP Intel PC or AMD Opteron (64-bit)
- CPUs must have high resolution timer frequencies greater than 1GHz.

- Red Hat Enterprise Server 6.X Release 64-bit
- Oracle Linux Server 6.X or 7.X Release 64-bit
- Red Hat Enterprise Server 8.X Release 64-bit
- CentOS 7.X Release 64-bit
- CentOS 8.X Release 64-bit

- TCP/IP networking support installed if using TCP Socket connection types
- UDP Multicast networking support if using Reliable Multicast connection type
- Appropriate system access permissions when using Unidirectional Shared Memory connection types

* For best performance, ensure that machines and network switches are configured appropriately. See specific OS and hardware documentation for more information.

Users are welcome to migrate open source code to the platforms they prefer, however support for the included libraries is only provided on platforms listed below.

#### Supported Platforms

##### Windows

Platforms:

	Microsoft Windows Server 2012 Enterprise Edition or later 64-bit
	Microsoft Windows Server 2016 Enterprise Edition or later 64-bit
	Microsoft Windows 8.1 Professional or later 64-bit
	Microsoft Windows 10 Professional 64-bit 

Compilers (only on OSs supported by Microsoft): 

	Microsoft Visual Studio 14.0 (2015) 64-bit
	Microsoft Visual Studio 14.1 (2017) 64-bit 
	Microsoft Visual Studio 14.2 (2019) 64-bit 

Notes: 
- User has the option to use pre-built libraries for the compilers listed above and use them on different Windows operating systems that have support for those compilers to build their applications. User may also choose to build source and applications. 
- CMake supports VS 2013 and VS 2012 builds although libraries are no longer shipped. If closed source from BinaryPack is required to build, please use a BinaryPack [prior to Real-Time-SDK-2.0.3.L1](https://github.com/Refinitiv/Real-Time-SDK/releases/tag/Real-Time-SDK-2.0.2.G3) to build these deprecated Visual Studio versions at your own risk as changes to BinaryPacks will not be available for deprecated compilers. 

##### Linux

Platforms & Compilers:

	GCC compiler suite version 4.4.4 or higher for Red Hat Enterprise Server 6.X, 64-bit, Native build
	GCC compiler suite version 4.8.2 or higher for Oracle Linux 7.X, 64-bit, Native build
	GCC compiler suite version 4.4.4 or higher for Oracle Linux 6.0, 64-bit, qualification with RH6 library build
	GCC compiler suite version 4.8.2 or higher for CentOS 7.0, 64-bit, qualification with OL7 library build
	GCC compiler suite version 8.3.1 or higher for Red Hat Enterprise Server 8.X, 64-bit, Native build
	GCC compiler suite version 8.3.1 or higher for CentOS 8.0, 64-bit, qualification with RH8 library build 
	Clang compiler version 9.0.1 for CentOS 8.0, 64-bit, qualification with RH8 library build 

* Eta VACache library built 

NOTE: User has the option to use pre-built libraries or build source natively on a platform of choice. Pre-built libraries for Red Hat 8, Oracle Linux 7 and Red Hat 6 are available in release packages available on Refinitiv Developer Portal. 

#### Tested Versions

##### Windows
This release has been tested with supported valid OS/compiler combinations.

##### Linux
This release has been tested with the following on supported platform/OS combinations. Please note that the list of tested platforms and compiler combination below reflects test of two use cases: using pre-built libraries to build applications _and_ natively building source and using those libraries to build applications.

	OS						GCC Version	Use-Prebuilt Library	Use-Natively Build Library
	--------------------------------		------------	----------------------	----------------------------		
	Oracle Linux Server 6.6 64-bit 			GCC 4.4.4  	RHEL6_64_GCC444		OL6_64_GCC444
	Red Hat Enterprise Linux Server 6.3 64-bit   	GCC 4.4.6	RHEL6_64_GCC444	 	RHEL6_64_GCC446	
	Red Hat Enterprise Linux Server 6.9 64-bit   	GCC 4.4.7	RHEL6_64_GCC444	 	RHEL6_64_GCC447	
	CentOS 7.0 64-bit                		GCC 4.8.2	OL7_64_GCC482		CENTOS7_64_GCC482
	Oracle Linux Server 7.7 64-bit 			GCC 4.8.5	OL7_64_GCC482		OL7_64_GCC485
	Red Hat Enterprise Linux Server 7.7 64-bit 	GCC 4.8.5 	OL7_64_GCC482		RHEL7_64_GCC485
	Oracle Linux Server 7.7 64-bit     		GCC 7.4.0	n/a           		OL7_64_GCC740
	Red Hat Enterprise Linux Server 6.10 64-bit 	GCC 7.4.0 	n/a			RHEL6_64_GCC740
	Red Hat Enterprise Linux Server 7.7 64-bit 	GCC 7.4.0 	n/a			RHEL7_64_GCC740
	Red Hat Enterprise Linux Server 8.0 64-bit 	GCC 8.3.1 	n/a			RHEL8_64_GCC831
	CentOS 8.0 64-bit                		GCC 8.3.1	RHEL8_64_GCC831 	CENTOS8_64_GCC831
	CentOS 8.0 64-bit                		Clang 9.0.1     RHEL8_64_GCC831         CENTOS8_64_Clang901	

	n/a = This is not a tested combination

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

RTSDK Cpp-C supports connectivity to the following platforms:

- Refinitiv Real-Time Distribution System (RSSL/RWF connections) : ADS version 2.6 and higher, ADH version 2.6 and higher. 
- Refinitiv Real-Time: Refinitiv Real-Time Deployed, Refinitiv Real-Time Hosted, Refinitiv Direct Feed

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data.

This release has been tested with the following:

- ADS 3.5.3
- ADH 3.5.3
- DACS 7.6

# Documentation

Please refer to top level README.md and to Cpp-C/Eta/README.md or Cpp-C/Ema/README.md files to find more information. Sections Cpp-C/Eta/Docs/ and Cpp-C/Ema/Docs contain PDF/HTML documentation. In current directory, please find the test plan with test results: RTSDK-C-Edition\_Test\_Plan.xlsx

# Installation and Build

Please refer to Installation Guide for [ETA](Eta/Docs/RTSDK_C_Installation_Guide.pdf) or [EMA](Ema/Docs/RTSDK_C_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK 
There are 3 ways to install Refinitiv Real-Time SDK:

Obtain the source **from this repository** on GitHub. It will contain all of the required source to build RTSDK as detailed below. In addition, this repository depends on a Binary Pack found in the [release assets](https://github.com/Refinitiv/Real-Time-SDK/releases) section that is auto pulled by a build. The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product. 

Refinitiv Real-Time SDK package may also be [downloaded from Refinitiv Developer Portal](https://developers.refinitiv.com/en/api-catalog/refinitiv-real-time-opnsrc/rt-sdk-cc/downloads).

Refinitiv Real-Time SDK package is also available on [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html). 

## Building RTSDK

**Using CMake**:

Cmake is required to create the Linux Makefile files and Windows Solution and vcxproj files. To build examples or re-build libraries, user must download [CMake](https://cmake.org) version 3.14 or greater. Please note that gtest version included for unit testing is not compatible with later versions of CMake such as 3.21.4; this will be addressed in a future release.

Refer to the RTSDK C/C++ Installation Guide located in Cpp-C/Eta/Docs or Cpp-C/Ema/Docs for more detailed CMake build instructions than what is described below.

**For Linux**:

Note: For Linux builds with RedHat based distributions(RHEL, CentOS, Oracle Linux), the CMake scripts require lsb_release to be installed.  For Red Hat Enterprise Linux and CentOS, this can be installed with the following command (this will require root access to the machine):

	yum install redhat-lsb-core

At the same directory level as the resulting RTSDK directory, issue the following command to build the optimized Makefile files:

	cmake -HRTSDK -BbuildDir
	# Refinitiv Real-Time SDK is the RTSDK directory 
	# buildDir is the directory where all build output is placed 
	# Note: buildDir, or anything your specify in place of it in above command, is automatically created

Issue the following command to build debug Makefile files:

	cmake -HRTSDK -BbuildDir –DCMAKE_BUILD_TYPE=Debug

The cmake command builds all needed Makefile files (and related dependencies) in the buildDir directory. 
Go to the buildDir directory and type "make" to create the RTSDK libraries. Note that the libraries are sent to the RTSDK directory (i.e., not the buildDir directory).

**For Windows**:

At the same directory level as the resulting RTSDK directory, issue the following command to build the Solution and vcxproj files:

	cmake -HRTSDK -BbuildDir -G <VisualStudioVersion>
	# RTDK is the directory with the source code 
	# buildDir is the directory where all build output is placed; this is where the built binaries are placed 
	# Note: buildDir, or anything your specify in place of it in above command, is automatically created
	# "VisualStudioVersion" is the visual studio version to use for build on windows.
	# Valid values for VisualStudioVersion are 
		# "Visual Studio 16 2019" -A x64 
		# "Visual Studio 15 2017 Win64"
		# "Visual Studio 14 2015 Win64" 
	# Note: A list of visual studio versions can be obtained by typing "cmake -help". 
	# Note: CMake supports VS 2013 and VS 2012 builds although libraries are no longer shipped. If closed source from BinaryPack is required to build, please use a BinaryPack prior to Real-Time-SDK-2.0.3.L1 to build these deprecated Visual Studio versions at your own risk. Changes to BinaryPacks will not be available for deprecated compilers.

The cmake command builds all needed Solution and vcxproj files (and other related files) in the buildDir directory. User must open these files and build all libraries and examples in the same manner as with prior RTSDK versions. Note that the build output is sent to the RTSDK directory (i.e., not the buildDir directory).

Note that your installation of Visual Studio needs to be updated to add Microsoft Foundation Classes per Microsoft when encountering this build error: fatal error RC105: cannot open include file 'afxres.h'.

**32 bit support**:

CMake has build support for 32 bit platforms.

Linux: Add "-DBUILD\_32\_BIT\_ETA=ON" to the cmake build

Windows: Do not add "Win64" or "-A x64" to the "VisualStudioVersion".  Example, When specifying, "Visual Studio 14 2015", for a 64-bit build it would be "Visual Studio 14 2015 Win64". For a 32-bit build, it would be "Visual Studio 14 2015" 

NOTE: Starting with SDK version 1.3.1, DACS libraries are available for 32-bits in the BinaryPack.
NOTE: DACS libraries provided in BinaryPack for VS2019 is copied from VS 2017 as these build is not currently available.

# Obtaining the Refinitiv Field Dictionaries

The Refinitiv `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `Cpp-C/etc`. In addition, the most current version can be downloaded from [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html). Search for "Service Pack" and choose the latest version of Refinitiv Real-Time Template Service Pack.


# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Entity%20Contributor%20License%20Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@refinitiv.com. If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.


# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each package for clarification.
- Please make sure to review the LICENSE.md file.
