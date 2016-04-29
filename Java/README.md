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

Firstly, obtain the source from this repository. It will contain all of the required source to build EMA and ETA libraries as detailed below.


## Building ETA

#### ETA Special Instructions
The ETA package contains transport, decoder, encoder, and cache components.  The transport, decoder and encoder components are closed source and is proprietary to Thomson Reuters and the source code is not included in this package. To facilitate the ability to build all APIs of this Elektron-SDK package a 'stub' library is provided for the closed portion of the ETA.   This 'stub' library will allow you to build and run, but will not provide implementation for connectivity or data handling. To get a fully functioning ETA library please see "Obtaining the ETA Binary Package" below.

####1) Build the ETA API 

**Using Apache Ant**:

Ant can be downloaded from http://ant.apache.org

Navigate to `Eta/Source` 
-	Run `ant all` to build Stub libraries and Reactor and its dependencies
-	Run `ant build-stubs` to build only the Stub libraries
-	Run `ant build` or `ant build-valueadd` to build only Reactor and its dependencies.  This is the default build to avoid clobbering any binary pack libraries.

**NOTE:** You must build the stub libraries prior to building the Reactor and its dependencies.

####2) Build the ETA API Examples

Navigate to `Eta/Applications`, locate the example or performance tool you would like to build. 
The Example applications are located in `Eta/Applications/Examples`
The Performance Tools are located in `Eta/Applications/PerfTools`
-	Run `ant` to build all examples/performance tools or select the target from inside the build.xml file to built a specific example or performance tool.
-	Optionally, these can be built using the bat or ksh script files if preferred over ant.

####3) Run the ETA Examples

If you have only built the 'stub' library from above, the examples run but fail.  
In order to get full functioning behaviour of ETA you will need to get the official binaries from the Thomson Reuters Customer Zone (http://customers.thomsonreuters.com). 
Please see "Obtaining the ETA Binary Package" below.

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



# Obtaining the ETA Binary Package

To get the full functionality of the Transport API, please get the official ETA libraries from the following.

**Developer Community:**

https://developers.thomsonreuters.com/

Then select the following options:

- **APIs by Product**: Elektron
- **APIs in this Family**: Elektron SDK - Java Edition
- **Downloads**: ETA - Java - BINARY PACKS

**Customer Zone:**

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - API
- **Products**: Elektron SDK

Then select the following release

    etaj3.0.0.L1.all-binaries.lib
	
Once you have downloaded these libraries, copy them to the corresponding directories under `Eta/Libs`
Additional information about the contents of the closed source ETA libraries are available in the README contained in that distribution package.

Your example should now be functional.


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
