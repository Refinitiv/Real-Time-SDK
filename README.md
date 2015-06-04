# Elektron SDK 1.0.0
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


      (Windows)
      - Intel compatible PC and AMD Opteron for 64-bit
      - CPUs must have high resolution timer frequencies greater than 1GHz.
      - Microsoft Windows Server 2008 (SP1 or greater) 64-bit 
      - Microsoft Windows 7 Professional 64-bit
      - Microsoft Windows 8 Professional 64-bit
      - Microsoft Windows 8.1 Professional 64-bit 
     
      
# Software Requirements
      (Linux)
      - GCC compiler suite version 4.4.4 or higher for RHEL 6.0 (64-bit)
      - GCC compiler suite version 4.4.4 or higher for OLS 6.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for OLS 7.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for CentOS 7.0 (64-bit)

      (Windows)
      - Microsoft Visual C++ 10.0 64-bit (visual Studio 2010)
      - Microsoft Visual C++ 11.0 64-bit (Visual Studio 2012)
      - Microsoft Visual C++ 12.0 64-bit (Visual Studio 2013)
      

# What can you do with this release?
The intent of this particular Elektron SDK release is to provide the Beta release of EMA.
- You will be able to build the EMA libraries
- You will be able to build and run the EMA examples
- Consumer Interaction Only

# Limitations
- The ETA contained in this release is only part of the intended ETA pacakge.  The ETA folder will only contain the required parts for EMA (ValueAdd reactor). You will not be able to build ETA examples or the ETA Value add libraries. 
- You will need to get the appropriate ETA library package from customer zone to build and run EMA. Instructions on how to obtain that can be found in the following section.


# Setup
This section shows the required setup needed before you can build the EMA Beta.

To start, obtain the source from this repository which will contain all of the required source to build the EMA library and EMA examples.

Next, since EMA relies upon the ETA product which contains closed source, you will need to obtain the library package from the customer zone. This binary package will contain only the necessary lib files to allow you to build the EMA Beta. Just place the Libs directory from the package under the eta directory and you should then be able to build the ema libraries and examples.

```
Elektron-SDK1.0.0
      |
      +ema/...
      |
      +eta
          |
          +Src
          |
          +Libs  //Copy the Libs directory from the customer zone package here.
```

You can find the ETA binaries at the following:

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - API
- **Products**: TREP API Controlled Binaries

Then select the following release:

    eta3.0.0.L1.beta.<platform>.lib



# Building (EMA only)

Once you have done the Setup step above, follow these steps below: 

###1) Get or build the libxml2 library.

If your system does not already have libxml2 available, you can build the version that is contained in this release. Just navigate to `ema/Src/libxml/src` and run the makefile or windows project file. 

**For Windows**:
The *libxml2* library will be created in `ema/Src/libxml/src/Libs`.  Copy the resultant `Libs` directory to the corresponding platform `ema/Libs` directory.

**For Linux**: (This will be fixed in a subsequent release to be consistent with Windows)
The *libxml2* library will be created in `ema/Src/libxml/src/<platform>` where `<platform>` is `OL7_64_gcc482` or `RHEL6_64_GCC444`.

For "shared" libraries, create the following directories (if they don't exist):
```
ema/Libs/<platform>/Optimized/Shared
ema/Libs/<platform>/Optimized_Assert/Shared
```

Then, copy the `libxml2.so` to both of the directories listed above.


For "static" libraries, create the following directories (if they don't exist):
```
ema/Libs/<platform>/Optimized
ema/Libs/<platform>/Optimized_Assert
```

Then, copy the `libxml2.a` to both of the directories listed above.


###2) Build the EMA library

To build the EMA library, navigate to the `ema/Src/Access` folder and run the makefiles/windows project.  
Once the binaries are built you will need to copy the resultant `Lib` directory that is created under `Access` to the top level `ema` directory, merging it with previously created libxml2 library.  

####3) Build the EMA examples

After that, you can build any of the EMA examples. Navigate to the example you wish to build and you will find both a makefile and windows project file.

####4) Get access to a providing application. 

You will need a provider component to connect the EMA consumer applications to.  This can be an ADS or API provider application from UPA or RFA.

####5) Run the EMA examples

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
