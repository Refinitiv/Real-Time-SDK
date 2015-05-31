# Elektron SDK
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is Thomson Reuters low-level 
Transport and OMM encoder/decoder API.  It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. 

*NOTE: This first release of the Elektron SDK contain only the EMA (Beta).  You will see that there is both an EMA and ETA directory, but the ETA directory provides only the minimum required needed for the EMA product.  It is not the full ETA release.*

# Hardware/OS Requirements

      (Linux)
      - HP Intel PC or AMD Opteron (64-bit)
      - AMD Opteron (64-bit)
      - Red Hat Enterprise Linux Advanced Server (RHEL) 6.0 64-bit 
      - Oracle Linux Server (OLS) 7.0 64-bit


# Software Requirements
      (Linux)
      - GCC compiler suite version 4.4.4 or higher for RHEL 6.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for OLS 7.0 (64-bit)



# What can you do with this release?
The intent of this particular Elektron SDK release is to provide the Beta release of EMA.
- You will be able to build the EMA libraries
- You will be able to build and run the EMA examples
- Consumer Interaction Only

# Limitations
- The ETA contained in this release is only part of the intended ETA pacakge.  The ETA folder will only contain the required parts for EMA (ValueAdd reactor). You will not be able to build ETA examples or the ETA Value add libraries. 
- You will need to get the appropriate ETA library package from customer zone to build and run EMA. Instructions on how to obtain that can be found in the following section.
- This release only supports the "static" version linux. Although the makefiles show options to build shared libraries, the shared builds will not be successful. The shared version will be available in a subsequent release.
- Windows support will be coming in a subsequent release.


# Setup
This section shows the required setup needed before you can build the EMA Beta.

To start, obtain the source from this repository which will contain all of the required source to build the EMA library and EMA examples.

Next, since EMA relies upon the ETA product which contains closed source, you will need to obtain the library package from the customer zone. This binary package will contain only the necessary lib files to allow you to build the EMA Beta. Just place the Libs directory from the package under the eta directory and you should then be able to build the ema libraries and examples.

```
Elektron-SDK1.0.0
      |
      +ema/...
      |
      +eta/Src
          /Libs  //Copy the Libs directory from the customer zone package here.
```

You can find the ETA binaries at the following:

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - API
- **Products**: TREP API Controlled Binaries

Then select the following release:

    eta3.0.0.L1.beta.linux.lib



# Building (EMA only)

1) Get the libxml2 library
If your system does not already have libxml2 available, you can build the version that is contained in this release. Just navigate to ema/Src/libxml/src and run the makefile.  The libxml2 library will be created in ema/Src/libxml/src/<platform>/Static.  Copy the resultant library to the corresponding platform ema/Src/Libs directory show below.

```
ema/Libs
      |
      OL7_64_gcc482/Optimized
                   /Optimized_Assert
```
or

```                
ema/Libs
      |
      RHEL6_64_GCC444/Optimized
                     /Optimized_Assert
  ```              


2) Build the EMA library
The makefile for building the EMA library is located within ema/Src/Access.  
Once the binaries are built you will need to copy the resultant Lib directory that is created under Access to the top level ema directory.  

3) Build the EMA examples
After that, you can build any of the EMA examples. Navigate to the example you wish to build and you will find the appropriate makefile.

4) Get access to a providing application. 
You will need a provider component to connect the EMA consumer applications to.  This can be an ADS or API provider application from UPA or RFA.

5) Run the EMA examples
Once the provider is running and accessible, you can run the EMA examples.  

That should do it!  



# Developing 

If you have discover any issues with regards to this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Notes:
- This is a BETA release.  
- Interfaces and behaviors are subject to change (based upon feedback and suggestions)
- Please make sure to review the LICENSE.md file.
