# Elektron SDK - C/C++ Edition
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is Thomson Reuters low-level 
Transport and OMM encoder/decoder API.  It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. 

# Supported Platforms and Compilers

The Elektron-SDK has support for Linux, Windows and Solaris.  Please see the individual API README.md files for further details on supported platforms and compilers.

# Building the APIs

## Common Setup
This section shows the required setup needed before you can build any of the APIs within this package.

Firstly, obtain the source from this repository. It will contain all of the required source to build EMA and ETA libraries as detailed below.


## Building ETA

#### ETA Special Instructions
The ETA package contains transport, decoder, encoder, and cache components.  The transport, decoder and encoder components are closed source and is proprietary to Thomson Reuters and the source code is not included 
The transport, decoder, encoder, and cache components are closed source and is proprietary to Thomson Reuters and the source code is not included on GitHub. 
The `Eta\Libs` location in this package contains fully functioning libraries for the closed source portions of the product, allowing users to build and link to have a fully functional product.

####1) Build the ETA 

**For Linux/Solaris**:

Navigate to `Eta/Impl` 
-	Run `make all` to build Reactor and its dependencies.  This will link to the fully functional libraries provided in the `Libs` location.
-	Run `make stubs` to build only the Stub libraries.  This will overwrite libraries in the `Libs` location with the compiled Stub libraries.
-	Run `make rsslVA` to build only Reactor and its dependencies.  This will link to the fully functional libraries provided in the `Libs` location.  This is the same as the `make all` target.

This will build both static and shared versions of the libraries and will build Optimized libraries by default.  
If Optimized_Assert libraries are preferred, this can be modified from within the makefiles.

**NOTE:** If you are using shared libraries, you will need to run the LinuxSoLink or SolarisSoLink to properly soft link for versioned libraries. 

**For Windows**:

Navigate to `Eta/Impl` 
Select the `vcxproj` for the specific library you want to build, or use the provided solution (or `sln`) file to build in **Visual Studio**.  

When building via the solution, select the configuration combination you want (Static, Shared, Debug, Release, etc) and select `Build -> Build Solution` this will create both static and shared libraries for all targets.  


####2) Build the Transport API Examples

Navigate to `Eta/Applications`, locate the example, performance tool, or training suite you would like to build. Run the makefile or open and build the windows solution file (when applicable) or the vcxproj.

####3) Run the ETA Examples

Run the application from the command line using the appropriate execution commands.  Most applications have a help menu that can be viewed with a -? option.

## Building EMA

EMA is built upon ETA.  Before you can build EMA you must build ETA as described above. Once you have the ETA libraries in place you can then build the EMA libraries and the examples.


####1) Get or build the libxml2 library.
If your system does not already have libxml2 available, you can build the version that is contained in this release. Just navigate to `Eta/Utils/libxml2` and run the makefile or build the windows project file. 



####2) Build the EMA library

To build the EMA library, navigate to the `Ema/Src/Access` folder and run the makefile or build the windows project. Note that when building the shared object version of the EMA library, libxml2 is statically linked into it.  

####3) Build the EMA examples

After that, you can build any of the EMA examples. Navigate to the example you wish to build and you will find both a makefile and windows project file.

####4) Get access to a providing application. 

You will need a provider component to connect the EMA consumer applications to.  This can be an ADS or API provider application from ETA or RFA.

####5) Run the EMA Examples

Once the provider is running and accessible, you can run the EMA examples.  When running examples build using shared libraries you will need to make sure that the ETA libraries are local or in your path.

That should do it!  

### Windows Solution Files

As an alternative, after you build ETA as described above, you can build using the windows solution files.  Solutions files can be found in the `.../Ema/Examples/Training` directory or the `.../Ema/Src/Access`.  


# Developing 

If you have discover any issues with regards to this project, please feel free to create an Issue.

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
