# Elektron Object API - C++ Edition (PREVIEW)
#
 
##Overview
Here is a chance to view the upcoming Elektron Object API. This directory contains preliminary example code and precompiled library to show off the upcoming EOA C++ Edition.  This is not the full API, it's a preview of the API.  We want you to be aware of where are at and where we are heading.

##Contents
- **Src** include files only for the EOA Domain and Foundation Layers
- **Examples** for the EOA Domain Layer
  - All examples contain vcproject + makefiles for the supported platforms
  - Spreadsheet file for the examples listing capabilities and used interfaces
  - EmaConfig.xml
- **Platforms** 
  - Win7, WinServer 2008 (VS2012, VS2013)
  - OL7 (gcc482)
  - Static (`Optimized` and `OptimizedAssert`)
  - 64 bit
- **Etc**
  - Dictionaries (`RdmFieldDictionary`, `enumtype.def`)
- **Docs**
  - Reference Manual
  - EMA Docs for reference only

##EOA Supported Functionality
- **Domain Layer**
  - Anytime Object
  - Lambda Expressions
  - Callback Client
  - single known service
  - request attributes specification
  - cache mode of operation
  - native and string data formats (e.g. `getDouble()` and `getDoubleAsString()`)
  - strongly typed interfaces (e.g. `getBid()`, `getAsk()`)
  - Market Price subscription (streaming and snap)
  - Market By Order subscription (streaming and snap)
  - Market By Price subscription (streaming and snap)
  - external iterators and "for-each" loop

**Note**: This EOA version does not support functionality beyond that described above.

**Note**: All libraries, source code, functionality, and examples are preliminary and subject to change. 

##List of Examples
- **Domain Layer**
  - `100__MarketPrice__Snapshot__Anytime__Dump`
  - `110__MarketByOrder__Subscription__Anytime__Refresh`
  - `120__MarketPrice__Subscription__Lambda__FieldPrimitive`
  - `130__MarketPrice__Subscription__Callback__FieldPrimitive`
  - `140__MarketByPrice__Subscription__Callback__FieldString`
  - `150__MarketPrice__Subscription__Lambda__MultiItem`


**Note**: To build and run the examples you will need to obtain the binary package.  We will eventually provide a 'build-able' EOA library, but for now, use the pre-built libraries.

##Running the Examples

####1) Build the EOA examples
You can build any of the EOA examples. Navigate to the example you wish to build and you will find both a makefile and windows vcproject file.

####2) Get access to a providing application. 
You will need a provider component to connect the EOA consumer applications to. This can be an ADS or API provider application from ETA or RFA.

####3) Running the EOA Examples
All the examples are defaulted to do the following:

- use of the TCP IP channel
- connect to localhost:14002
- use machine's user name to login into a provider
- download dictionaries from provider
- log EOA activity in a `eoaLog_<PID>.log` file in the current directory

Placing the provided EmaConfig.xml file in the current directory allows
modification of the connectivity details. Please see the EMA Config Guide
for details on the configuration parameters and their possible values.

Navigate to the build output directory containing the executable and run the example.

## Can You Provide Feedback?
Not just yet!  In the coming months we will continue to update you with more examples and prototypes. As the product matures, we will be able to accept your feedback. 

Check back over the coming months to see where we are at!
