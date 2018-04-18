# Elektron SDK - C/C++ Edition
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is Thomson Reuters low-level 
Transport and OMM encoder/decoder API.  It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. 

# Supported Platforms and Compilers

The Elektron-SDK has support for Linux and Windows.  Please see the individual API README.md files for further details on supported platforms and compilers.

# Building the APIs

## Common Setup
This section shows the required setup needed before you can build any of the APIs within this package.

Firstly, obtain the source from this repository. It will contain all of the required source to build ESDK as detailed below.  
In addition, this repository depends on the "Elektron-SDK-BinaryPack" (http://www.github.com/thomsonreuters/Elektron-SDK-BinaryPack) repository and pulls the ETA libraries from that location.  That repository contains fully functioning libraries for the closed source portions of the product, allowing users to build and link to have a fully functional product. 

## Building ESDK

**Using CMake**:

CMake can be downloaded from https://cmake.org

**For Linux**:

At the same directory level as the resulting Elektron-SDK directory, issue the following command to build the optimized Makefile files:

cmake -HElektron-SDK -Bbuild-esdk
where Elektron-SDK is the ESDK directory and build-esdk is the directory where all build output is placed (note that build-esdk is automatically created)

Issue the following command to build debug Makefile files:

cmake -HElektron-SDK -Bbuild-esdk –DCMAKE_BUILD_TYPE=Debug

The cmake command builds all needed Makefile files (and related dependencies) in the build-esdk directory. 

Go to the build-esdk directory and type "make" to create the ESDK libraries. Note that the libraries are sent to the Elektron-SDK directory (i.e., not the build-esdk directory).

**For Windows**:

Note: The following details regarding Windows and CMake require the use of Cygwin. Cygwin can be downloaded from https://cygwin.com

At the same directory level as the resulting Elektron-SDK directory, issue the following command to build the Solution and vcxproj files:

cmake -HElektron-SDK -Bbuild-esdk -G "VisualStudioVersion"
where Elektron-SDK is the ESDK directory and build-esdk is the directory where all build output is placed (note that build-esdk is automatically created)

"VisualStudioVersion" is the visual studio version (e.g., "Visual Studio 14 2015 Win64"). A list of visual studio versions can be obtained by typing "cmake -help". 

The cmake command builds all needed Solution and vcxproj files (and other related files) in the build-esdk directory. You open these files and build all libraries and examples in the same fashion as you did with prior ESDKs.
Note that the build output is sent to the Elektron-SDK directory (i.e., not the build-esdk directory).

Note that only the following Windows versions are supported.

Visual Studio 15 2017
Visual Studio 14 2015
Visual Studio 12 2013
Visual Studio 11 2012

**32 bit support**:

CMake has build support for 32 bit platforms.

Linux: Add "-DBUILD_32_BIT_ETA=ON" to the cmake build

Windows: Don't add Win64 to the "VisualStudioVersion" (i.e., use "Visual Studio 14 2015" vs "Visual Studio 14 2015 Win64")

# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](../Elektron API Individual Contributor License Agreement.pdf)
- [Entity Contributor License Agreement](../Elektron API Entity Contributor License Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@thomsonreuters.com.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.


# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each package for clarification.
- Please make sure to review the LICENSE.md file.
