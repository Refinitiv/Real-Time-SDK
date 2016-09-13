# Elektron SDK - Java Edition
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is Thomson Reuters low-level 
Transport and OMM encoder/decoder API.  It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. 

# Supported Platforms and Compilers

The Elektron-SDK has support for JDK 1.7 and JDK 1.8.  Please see the individual API README.md files for further details.

# Building the APIs

## Common Setup
This section shows the required setup needed before you can build any of the APIs within this package.

Firstly, obtain the source from this repository. It will contain all of the required source to build EMA and ETA as detailed below.
In addition, this repository depends on the `Elektron-SDK-BinaryPack` (http://www.github.com/thomsonreuters/Elektron-SDK-BinaryPack) repository and pulls the ETA libraries from that location.  That repository contains fully functioning libraries for the closed source portions of the product, allowing users to build and link to have a fully functional product. 
This repository uses submodules for this cross-dependency, so users should add the `--recursive` option to their git clone command.


## Building ETA

#### ETA Special Instructions
The ETA package contains transport, decoder, encoder, and cache components.  
The transport, decoder, encoder, and cache components are closed source and is proprietary to Thomson Reuters and the source code is not included on GitHub. 
This repository depends on the `Elektron-SDK-BinaryPack` (http://www.github.com/thomsonreuters/Elektron-SDK-BinaryPack) repository and pulls the ETA libraries from that location.  That repository contains fully functioning libraries for the closed source portions of the product, allowing users to build and link to have a fully functional product.


####1) Build the ETA API 

**Using Apache Ant**:

Ant can be downloaded from http://ant.apache.org

Navigate to `Eta/Source` 
-	Run `ant all` to build Reactor and its dependencies.  This will use the fully functional JAR files and libraries provided in the `Libs` location of the `Elektron-SDK-BinaryPack` repository.
-	Run `ant build-stubs` to build only the Stub libraries.  This will overwrite libraries in the `Libs` location with the built stub libraries. 
-	Run `ant build` or `ant build-valueadd` to build only Reactor and its dependencies.  This will link to the fully functional JAR files and libraries provided in the `Libs` location.  This is the same as the `ant all` target.


####2) Build the ETA API Examples

Navigate to `Eta/Applications`, locate the example or performance tool you would like to build. 
The Example applications are located in `Eta/Applications/Examples`
The Performance Tools are located in `Eta/Applications/PerfTools`
-	Run `ant` to build all examples/performance tools or select the target from inside the build.xml file to built a specific example or performance tool.
-	Optionally, these can be built using the bat or ksh script files if preferred over ant.

####3) Run the ETA Examples

Run the application from the command line using the appropriate execution commands.  Most applications have a help menu that can be viewed with a -? option.

**NOTE** If you have built using the 'stub' libraries, the examples run but fail.  

## Building EMA

Follow the steps below to build EMA library and examples.



####1) Build the EMA library

**Using Apache Ant**:

To build EMA library (`ema.jar`), navigate to `Src` and run the `ant` command. 
The `ant` script first builds the underlying ETA libraries (`upa.jar` and `upaValueAdd.jar`) and then builds the EMA library.


####2) Build the EMA examples
To build EMA examples, navigate to `Src/examples` and run the `ant` command. The `ant` script will build all of the examples.


####4) Get access to a providing application. 

You will need a provider component to connect the EMA consumer applications to.  This can be an ADS or API provider application from ETA or RFA.

####5) Run the EMA Examples

Once the provider is running and accessible, you can run the EMA examples. 

Set the `CLASSPATH` to include the required jars.

See `.../Src/examples/example100__MarketPrice__Streaming.bat` for an example of setting the `CLASSPATH` and running an example

That should do it!  


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
