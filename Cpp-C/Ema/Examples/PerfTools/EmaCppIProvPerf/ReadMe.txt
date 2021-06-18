
EmaCppIProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
in consuming Level I Market Price content directly from an
OMM provider or through a Refinitiv Real-Time Distribution System. 

The interactive provider creates two types of threads:
- A main thread, which collects and records statistical information,
- Working threads, each of which handle a connection with a consumer,
provide refreshes and updates of market data.

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

EmaCppIProvPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- RDMFieldDictionary and enumtype.def, located in the etc directory.
- 350k.xml, located in PerfTools/Common.
- MsgData.xml, located in PerfTools/Common.
- EmaCppIProvPerf includes the EMA library. For shared builds, the location of that
  library must be included in the LD_LIBRARY_PATH. The library (libema.so) will be found in
  Ema/Libs/<env>/Optimized/Shared or Ema/Libs/<env>/Debug/Shared
  where <env> is your build directory (i.e., OL7_64_GCC482).
- EmaConfig.xml, located in the Ema directory.

-------------------
Command line usage:
-------------------  

EmaCppIProvPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen. )

- EmaCppIProvPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

-----------------
Compiling Source:
-----------------

Development Tool: 

open one of the included solution files with visual studio and build.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmaCppIProvPerf.cpp - The main file for the EmaCppIProvPerf application.

IProvPerfConfig.cpp - Provides configurable options for the application.

ProviderThread.cpp - Handles consumer connections and sends content.

ProviderPerfClient.cpp - Handles messages from the consumer application.

AppUtil.cpp - Utility for use by applications and/or common classes.

CtrlBreakHandler.cpp  - Provides Contol-C handling.
 
Statistics.cpp - Provides methods for collecting and calculating statistical  information.

PerfConfig.cpp  - Common configurable options across PerfTool applications.

Mutex.cpp   - Provides Mutex handling.

ThreadAffinity.cpp - Used for printout and determination of thread affinity binding.

ThreadBinding.h - Handles Thread binding.

XmlItemParser.cpp - Used for parsing Item file (350k.xml).

xmlPerfMsgDataParser.cpp - Used for parsing Message template file (MsgData.xml).
