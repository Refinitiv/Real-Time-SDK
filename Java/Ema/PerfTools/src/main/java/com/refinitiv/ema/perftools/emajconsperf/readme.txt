
EmajConsPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of EMA API,
decoders in consuming Level I Market Price content directly
from an OMM provider or through the Refinitiv Real-Time Distribution System. 

The consumer creates two types of threads:
- A main thread, which collects and records statistical information,
- Consumer threads, each of which create a connection to a provider and
request market data.

To measure latency, a timestamp is randomly placed in each burst of updates by 
the provider.  The consumer then decodes the timestamp from the update to
determine the end-to-end latency.  ConsPerf also supports measurement of 
posting latency.

This application also measures memory and CPU usage.  The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application.  The CPU usage is the total time using the CPU divided by the 
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

For more detailed information on the performance measurement applications, 
see the ETA Open Source Performance Tools Guide(Eta/Docs/ETAJ_PerfToolsGuide.pdf).

This application uses an open source C-language XML parser library.

-----------------
Application Name:
-----------------

emajConsPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- EmaConfig.xml in perftools directory.
- XML parser library xpp3-1.1.3_8.jar and xpp3_min-1.1.3_8.jar, in perftools directory.
- 350k.xml, located in perftools directory.
- MsgData.xml, located in perftools directory.

-------------------
Command line usage:
-------------------  

emajConsPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

emajConsPerf -? displays command line options, with a brief description of each option.  

Pressing the CTRL+C buttons terminates the program. 
 
Please see runPerfConsumer.bat(Window platform) or runPerfConsumer.ksh (Unix platform)
for more detail about how to run the application.

-----------------
Compiling Source:
-----------------

build.xml in perftools directory to build all performance applications.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

emajConsPerf.java - The main file for the emajConsPerf application.

ConsPerfConfig.java - Provides configurable options for the application.

ConsumerThread.java - Handles consumer connections and consuming content.

ItemRequest.java - Provides functions for caching and retrieving item information
for use in request message bursts.

DirectoryHandler.java - Provides functions for sending the source directory
request to a provider and processing the response.

MarketPriceDecoder.java - Decodes Market Price content.

TimeRecord.java - Stores time information for calculating latency.

ValueStatistics.java - Provides methods for collecting and calculating statistical 
information.

XmlItemInfoList.java - Loads the list of sample items that the consumer may 
request from an XML file.
