
NIProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the Transport API, 
encoders and decoders, in providing Level I Market Price content to the 
Advanced Data Hub.

The provider creates two categories of threads:
- a main thread, which collects and records statistical information.
- provider threads, each of which connect to an ADH and provide market data.

The provider may be configured to provide updates at various rates.  To measure
latency, a timestamp is randomly placed in each burst of updates.   The 
consumer then decodes the timestamp from the update to determine the end-to-end 
latency.

This application also measures memory and CPU usage.  The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application.  The CPU usage is the total time using the CPU divided by the 
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

For more detailed information on the performance measurement applications, 
see the Transport API C Open Source Performance Tools Guide
(PerfTools/Docs/PerfToolsGuide.doc).

This application uses Libxml2, an open source C-language XML parser library.
See the readme in the provided Libxml2 source for more details.

-----------------
Application Name:
-----------------

NIProvPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- RDMFieldDictionary and enumtype.def, located in the etc directory.
- MsgData.xml, located in PerfTools/Common
- 350k.xml, located in PerfTools/Common

-------------------
Command line usage:
-------------------  

NIProvPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen. )

- NIProvPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

-----------------
Compiling Source:
-----------------

The included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the Transport API supported 
Linux platforms using the supported compilers.

The LINKTYPE value in the makefile is used to control
whether the application is built using Transport API static or
shared libraries. The default build uses Transport API static
libraries. To use Transport API shared libraries,
set LINKTYPE=Shared.

To compile, run the gmake command.

Gmake can be obtained at http://www.gnu.org/software/make/

For windows platform, using Visual Studio, open one of the included vcxproj project
files and build.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

NIProvPerf.c - The main file for the NIProvPerf application.

niProvPerfConfig.c - Provides configurable options for the application.

directoryProvider.c - Handles publishing the provider's source directory.

channelHandler.c - Provides management of connections, such as initializing,
 reading, and ping checking.

getTime.c - Provides functions for retrieving time information for use in 
  measurements.

hashTable.h - A basic hash table object.

itemEncoder.c - Encodes refresh and update messages.

latencyRandomArray.c - Provides randomization used in message bursts.

marketByOrderEncoder.c - Encodes Market By Order content (this functionality 
  is experimental).

marketPriceEncoder.c - Encodes Market Price content.

providerThreads.c - Handles the publishing of refreshes and updates for open 
  items.

rsslThread.h - Cross-platform definitions for threads and mutexes.

rsslQueue.h - A basic queue object.

statistics.c - Provides methods for collecting and calculating statistical 
  information.

testUtils.h - Contains some common test functionality.

xmlItemListParser.c - Loads the list of sample items the provider may publish.

xmlMsgDataParser.c - Loads sample message data for use in refreshes and 
  updates.

