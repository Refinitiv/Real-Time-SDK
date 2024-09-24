
TransportPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the Transport API,
using the different supported connection types with variable message sizes.

The content provided by this application is intended for raw transport
measurement and does not use the OMM encoders and decoders.  The message
contains only a sequence number and random timestamp.  The remainder of
the message is padded with zeros.

The application creates two types of threads:
- a main thread, which collects and records statistical information.
- connection threads, which connect or accept connections, and pass messages
  across.

To measure latency, a timestamp is randomly placed in each burst of messages
sent. The receiver of the messages reads the timestamp and compares it to the
current time to determine the end-to-end latency.

This application also measures memory and CPU usage.  The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application.  The CPU usage is the total time using the CPU divided by the 
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

For more detailed information on the performance measurement applications, 
see the Transport API C Open Source Performance Tools Guide
(PerfTools/Docs/PerfToolsGuide.doc).

-----------------
Application Name:
-----------------

TransportPerf

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------  

To run a basic scenario, run two instances of TransportPerf:

	TransportPerf
	TransportPerf -appType client

The applications will connect to each other and begin exchanging messages.

- TransportPerf -? displays command line options, with a brief description
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

TransportPerf.c - The main file for the TransportPerf application.

transportPerfConfig.c - Providers configurable options for the application.

transportThreads.c - Handles each connection and the sending and receiving of
  messages.

channelHandler.c - Provides management of connections, such as initializing,
 reading, and ping checking.

getTime.c - Provides functions for retrieving time information for use in 
  measurements.

latencyRandomArray.c - Provides randomization used in message bursts.

rsslThread.h - Cross-platform definitions for threads and mutexes.

rsslQueue.h - A basic queue object.

statistics.c - Provides methods for collecting and calculating statistical 
  information.

testUtils.h - Contains some common test functionality.

