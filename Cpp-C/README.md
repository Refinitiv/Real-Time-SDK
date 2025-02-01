# LSEG Real-Time SDK - C/C++ Edition
This is the LSEG Real-Time SDK. This SDK encompasses a couple of APIs: Enterprise Message API (EMA) and the Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source LSEG low-level Transport and OMM encoder/decoder API. It is used by the LSEG Real-Time Distribution Systems and LSEG Real-Time for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. Applications may be written to core ETA (RSSL), to ValueAdd/Reactor layer or to Watchlist layer.

Copyright (C) 2019-2025 LSEG. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Real-Time SDK - C/C++ Edition. Also in CHANGELOG is a list of issues fixed in this release and a history of features and fixes introduced per released version.

### External Dependencies

External modules used by this version of RTSDK C/C++:

     Dependency          Version
     ----------          -------
     openSSL               1.0.1e
     openSSL               1.1.1a
     openSSL               3.0.X  * 
     openSSL               3.1.X  ** 
     ccron                 2.0.0
     cJSON                 1.7.18
     curl                  8.11.0
     googletest            1.8.1 (for older than Linux7) and 1.12.1 
     l8w8jwt               2.4.0
     libxml2               2.13.5
     lz4                   1.9.4 (for older than Linux7) and 1.10.0
     zlib                  1.3.1
\* Tested on supported Linux and Windows platforms; \*\* Tested on supported Windows platform 

NOTES: 
- curl and openSSL are dynamically loaded at runtime.  
- Above mentioned version of openSSL was used in test. Please note that the RTSDK package does not build openSSL, and we recommend that all installed versions of openSSL are patched to the latest version available.  
- Default curl libraries and CMake build scripting provided in the RTSDK package are built against the default openSSL version provided by the Linux distribution (Oracle Linux 7, RedHat 8, RedHat 9). If the application is using a different version of openSSL than the distribution, one must obtain a version of Curl that links against the same major and minor version of openSSL as the application and rebuild to ensure that one version of openSSL is used. For Windows, the RTSDK package Curl build, links against the Windows schannel library, and does not have the possibility of a version incompatability issue with openSSL.
- openSSL 1.1.1 and higher are required for TLS 1.3
- Cipher suite is configurable with TLS 1.2 and it is recommended that customers use best available ciphers. With TLS 1.3, cipher suite is not yet configurable; however, default ciphers are best available.
- Check installation guide for details regarding including external dependencies for build 
   
### System Libraries Dependencies

Windows system libraries used by RTSDK C/C++:

     Dependency
     ----------
     wininet.lib
     ws2_32.lib
     crypt32.lib
     cryptui.lib
     bcrypt.lib
     Iphlpapi.lib

Linux system libraries used by RTSDK C/C++:

     Dependency
     ----------
     rt
     dl
     pthread
     m
     stdc++


### Supported Platforms, OSs, Compilers

#### Hardware/OS Requirements

- HP Intel PC or AMD Opteron (64-bit)
- CPUs must have high resolution timer frequencies greater than 1GHz.

- Oracle Linux Server 7.X, 64-bit
- Red Hat Enterprise Server 7.X, 64-bit
- Red Hat Enterprise Server 8.X, 64-bit
- Red Hat Enterprise Server 9.X, 64-bit
- Ubuntu 20.04, 64-bit
- Galaxy Kylin 10, 64-bit (TCP qualification only; multicast transport not tested)

- TCP/IP networking support installed if using TCP Socket connection types
- UDP Multicast networking support if using Reliable Multicast connection type
- Appropriate system access permissions when using Unidirectional Shared Memory connection types

* For best performance, ensure that machines and network switches are configured appropriately. See specific OS and hardware documentation for more information.

Users are welcome to migrate open source code to the platforms they prefer, however support for the included libraries is only provided on platforms listed below.

#### Supported Platforms

##### Windows

Platforms:

     Microsoft Windows Server 2016 Standard Edition or later 64-bit
     Microsoft Windows Server 2019 Standard Edition or later 64-bit
     Microsoft Windows Server 2022 Standard Edition or later 64-bit
     Microsoft Windows 10 Professional 64-bit 
     Microsoft Windows 11 Professional 64-bit 

Compilers (only on OSs supported by Microsoft): 

     Microsoft Visual Studio 14.0 (2015) 64-bit
     Microsoft Visual Studio 14.1 (2017) 64-bit 
     Microsoft Visual Studio 14.2 (2019) 64-bit 
     Microsoft Visual Studio 14.3 (2022) 64-bit 

Notes: 
- User has the option to use pre-built libraries for the compilers listed above and use them on different Windows operating systems that have support for those compilers to build their applications. User may also choose to build source and applications. 
- CMake supports VS 2013 build although libraries are no longer shipped. If closed source from BinaryPack is required to build, please use a BinaryPack [prior to Real-Time-SDK-2.0.3.L1](https://github.com/Refinitiv/Real-Time-SDK/releases/tag/Real-Time-SDK-2.0.2.G3) to build these deprecated Visual Studio versions at your own risk as changes to BinaryPacks will not be available for deprecated compilers. 
- For V2 authentication with RTO, RTSDK introduced the l8w8jwt library.  When building with Visual Studio 2019 or above, this library only works with Windows 10 SDK for October 2018 Update, version 1809 or later.

##### Linux

Platforms & Compilers:

     GCC compiler suite version 4.8.2 or higher for Oracle Linux 7.X, 64-bit, Native build
     GCC compiler suite version 7.3.0 or higher for Galaxy Kylin 10, 64-bit, qualification with RH8 library build
     GCC compiler suite version 8.3.1 or higher for Red Hat Enterprise Server 8.X, 64-bit, Native build
     Clang compiler version 9.0.1 for Linux 8 64-bit, qualification with RH8 library build 
     GCC compiler suite version 9.3.0 or higher for Ubuntu 20.04, 64-bit, qualification with RH8 library build
     GCC compiler suite version 11.4.1 or higher for Red Hat Enterprise Server 9.X, 64-bit, Native build

* Eta VACache library built 

NOTE: User has the option to use pre-built libraries or build source natively on a platform of choice. Pre-built libraries for Red Hat 9, Red Hat 8 and Oracle Linux 7 are available in release packages available on LSEG Developer Portal. 

#### Tested Versions

##### Windows
This release has been tested with supported valid OS/compiler combinations.

##### Linux
This release has been tested with the following on supported platform/OS combinations. Please note that the list of tested platforms and compiler combination below reflects test of two use cases: using pre-built libraries to build applications _and_ natively building source and using those libraries to build applications.

     OS                                           GCC Version     Use-Prebuilt Library     Use-Natively Build Library
     --------------------------------             ------------    --------------------     ----------------------------          
     Oracle Linux Server 7.7 64-bit               GCC 4.8.5       OL7_64_GCC482            OL7_64_GCC485
     Red Hat Enterprise Linux Server 7.7 64-bit   GCC 4.8.5       OL7_64_GCC482            RHEL7_64_GCC485
     Galaxy Kylin 10 64-bit                       GCC 7.3.0       RHEL8_64_GCC831          RHEL8_64_GCC740
     Oracle Linux Server 7.7 64-bit               GCC 7.4.0       n/a                      OL7_64_GCC740
     Red Hat Enterprise Linux Server 7.7 64-bit   GCC 7.4.0       n/a                      RHEL7_64_GCC740
     Red Hat Enterprise Linux Server 8.X 64-bit   GCC 8.3.1       RHEL8_64_GCC831          RHEL8_64_GCC831
     Galaxy Kylin 10 64-bit                       GCC 8.4.0       RHEL8_64_GCC831          RHEL8_64_GCC840
     Red Hat Enterprise Linux Server 8.X 64-bit   GCC 9.2.1       RHEL8_64_GCC831          RHEL8_64_GCC921
     Ubuntu 20.04 64-bit                          GCC 9.4.0       RHEL8_64_GCC831          RHEL8_64_GCC831
     Red Hat Enterprise Linux Server 9.2 64-bit   GCC 11.4.1      RHEL9_64_GCC1141         RHEL9_64_GCC1141
     Red Hat Enterprise Linux Server 9.2 64-bit   GCC 12.2.1      RHEL9_64_GCC1141         RHEL9_64_GCC1221

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

This release supports encryption using TLS 1.2 and TLS 1.3.  

NOTE: On windows TLS 1.3 is supported with OpenSSL.


### Interoperability

RTSDK Cpp-C supports connectivity to the following platforms:

- LSEG Real-Time Distribution System (RSSL/RWF connections): ADS/ADH all supported versions
- LSEG Real-Time: LSEG Real-Time Deployed
- LSEG Real-Time Hosted
- Real-Time - Optimized (RTO)
- Real-Time Direct


NOTE: Connectivity to Data Feed Direct (LDFD) is supported for Level 1 and Level 2 data.

This release has been tested with the following:

- ADS 3.8.1
- ADH 3.8.1
- DACS 7.12

# Documentation

Please refer to top level README.md and to Cpp-C/Eta/README.md or Cpp-C/Ema/README.md files to find more information. Sections Cpp-C/Eta/Docs/ and Cpp-C/Ema/Docs contain PDF/HTML documentation. In current directory, please find the test plan with test results: RTSDK-C-Edition\_Test\_Plan.xlsx

# Installation and Build

Please refer to Installation Guide for [ETA](Eta/Docs/RTSDK_C_Installation_Guide.pdf) or [EMA](Ema/Docs/RTSDK_C_Installation_Guide.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK 
There are 3 ways to install Real-Time SDK:

Obtain the source **from this repository** on GitHub. It will contain all of the required source to build RTSDK as detailed below. In addition, this repository depends on a Binary Pack found in the [release assets](https://github.com/Refinitiv/Real-Time-SDK/releases) section that is auto pulled by a build. The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product. 

Real-Time SDK packages may also be [downloaded from LSEG Developer Portal](https://developers.lseg.com/en/api-catalog/real-time-opnsrc/rt-sdk-cc/downloads). In addition, these distributions depend on a Binary Pack found in the above downloads section. This will not be automatically pulled by the build, and must be downloaded and extracted into the ../RTSDK-BinaryPack directory(Same level as the Cpp-C directory in this package). The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product.

Real-Time SDK packages are also available for download on [MyAccount](https://myaccount.lseg.com/en/downloadcenter). In addition, these distributions depend on a Binary Pack found in the above download location. This will not be automatically pulled by the build, and must be downloaded and extracted into the ../RTSDK-BinaryPack directory(Same level as the Cpp-C directory in this package). The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product. 

## Building RTSDK

**Using CMake**:

Cmake is required to create the Linux Makefile files and Windows Solution and vcxproj files. To build examples or re-build libraries, user must download [CMake](https://cmake.org) version 3.22.0 (cmake_minimum_required) or later.

In addition, Python 3 is required. This can be installed either through yum or a windows installer.

Refer to RTSDK C/C++ Installation Guide located in Cpp-C/Eta/Docs or Cpp-C/Ema/Docs for more detailed CMake build instructions than what is described below.

**For Linux**:

Note: For Linux builds with RedHat based distributions(RHEL, Oracle Linux), the CMake scripts require lsb_release to be installed.  For Red Hat Enterprise Linux and CentOS, this can be installed with the following command (this will require root access to the machine):

     yum install redhat-lsb-core

At the same directory level as the resulting RTSDK directory, issue the following command to build the optimized Makefile files:

     cmake -HsourceDir -BbuildDir
     # sourceDir is is the directory in which the top-level CMake entry point (CMakeLists.txt) resides. By default, when you build using the Solution and vcxproj files, output is sent to directory specified in sourceDir. 
     # buildDir is the directory where all build output is placed. This directory is automatically created.

Issue the following command to build debug Makefile files:

     cmake -HRTSDK -BbuildDir –DCMAKE_BUILD_TYPE=Debug

The cmake command builds all needed Makefile files (and related dependencies) in the buildDir directory. Go to the buildDir directory and type "make" or "gmake" to create the RTSDK libraries. Note that the libraries and sample application executables are sent to the RTSDK directory under sourceDir.

**For Windows**:

At the same directory level as the resulting RTSDK directory, issue the following command to build the Solution and vcxproj files:

     cmake -HsourceDir -BbuildDir -G <VisualStudioVersion>
     # sourceDir is is the directory in which the top-level CMake entry point (CMakeLists.txt) resides. By default, when you build using the Solution and vcxproj files, output is sent to directory specified in sourceDir. 
     # buildDir is the directory where all build output is placed. This directory is automatically created
     # "VisualStudioVersion" is the visual studio version to use for build on windows.
     # Valid values for VisualStudioVersion are 
          # "Visual Studio 17 2022" -A x64
          # "Visual Studio 16 2019" -A x64 
          # "Visual Studio 15 2017 Win64"
          # "Visual Studio 14 2015 Win64" 
     # Note: A list of visual studio versions can be obtained by typing "cmake -help". 
     # Note: CMake supports VS 2013 build although libraries are no longer shipped. If closed source from BinaryPack is required to build, please use a BinaryPack prior to Real-Time-SDK-2.0.3.L1 to build these deprecated Visual Studio versions at your own risk. Changes to BinaryPacks will not be available for deprecated compilers.

The cmake command builds all needed Solution and vcxproj files (and other related files) in the buildDir directory. User must open these files and build all libraries and examples in the same manner as with prior RTSDK versions. Note that the libraries and sample application executables are sent to an RTSDK directory under sourceDir.

Note that your installation of Visual Studio needs to be updated to add Microsoft Foundation Classes per Microsoft when encountering this build error: fatal error RC105: cannot open include file 'afxres.h'.

**32 bit support**:

CMake has build support for 32 bit platforms. This 32-bit support is available only for ETA core RSSL layer.

Linux: Add "-DBUILD\_32\_BIT\_ETA=ON" to the cmake build

Windows: Do not add "Win64" or "-A x64" to the "VisualStudioVersion".  Example, When specifying, "Visual Studio 14 2015", for a 64-bit build it would be "Visual Studio 14 2015 Win64". For a 32-bit build, it would be "Visual Studio 14 2015" 

NOTE: Starting with SDK version 1.3.1, DACS libraries are available for 32-bits in the BinaryPack.
NOTE: DACS libraries provided in BinaryPack for VS2019 is copied from VS 2017 as these build is not currently available.

# Obtaining the LSEG Field Dictionaries

The LSEG `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `Cpp-C/etc`. In addition, the most current version can be downloaded from [MyAccount](https://myaccount.lseg.com/en/downloadcenter). Search for "Service Pack" and choose the latest version of LSEG Real-Time Template Service Pack.


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
