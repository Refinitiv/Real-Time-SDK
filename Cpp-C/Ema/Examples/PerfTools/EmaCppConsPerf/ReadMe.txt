
EmaCppConsPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
in consuming Level I Market Price content directly from an
OMM provider or through the Thomson Reuters Enterprise Platform.

The consumer creates two types of threads:
- A main thread, which collects and records statistical information,
- Consumer threads, each of which create a connection to a provider and
request market data.

To measure latency, a timestamp is randomly placed in each burst of updates by 
the provider.  The consumer then decodes the timestamp from the update to
determine the end-to-end latency.  

This application also measures memory and CPU usage.  The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application.  The CPU usage is the total time using the CPU divided by the 
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

This application uses Libxml2, an open source  XML parser library.
See the readme in the provided Libxml2 source for more details.

-----------------
Application Name:
-----------------

EmaCppConsPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- RDMFieldDictionary and enumtype.def, located in the etc directory.
- 350k.xml, located in PerfTools/Common
- MsgData.xml, located in PerfTools/Common (only required if posting). Currently posting is not supported.
- EmaCppConsPerf includes the PerfTools library. For shared builds, the location of that
  library must be included in the LD_LIBRARY_PATH. The library (libPerfTools.so) will be found in
  PerfTools/Common/<env>/Optimized/Shared/obj or PerfTools/Common/<env>/Optimized_Assert/Shared/obj
  where <env> is your build directory (i.e., OL7_64_GCC482)

-------------------
Command line usage:
-------------------  

EmaCppConsPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen. )

- EmaCppConsPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

-----------------
Compiling Source:
-----------------

Development Tool: 

open one of the included solution files with visual studio
and build.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmaCppConsPerf.cpp - The main file for the EmaCppConsPerf application.

ConsPerfConfig.cpp - Provides configurable options for the application.

ConsumerThread.cpp - Handles consumer connections and consuming content.

AppUtil.cpp - Utility for use by applications and/or common classes.

CtrlBreakHandler.cpp  - Provides Contol-C handling
 
Statistics.cpp - Provides methods for collecting and calculating statistical  information.

PerfConfig.cpp  - Common configurable options across PerfTool applications.

Mutex.cpp   - Provides Mutex handling.

ThreadAffinity.cpp  -Used for printout and determination of thread affinity binding

ThreadBinding.h - Handles Thread binding.

 XmlItemParser.cpp  -Used for parsing Item file (350k.xml)

