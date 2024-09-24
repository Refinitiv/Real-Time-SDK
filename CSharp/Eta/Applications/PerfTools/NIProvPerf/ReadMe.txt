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

-----------------
Application Name:
-----------------

NIProvPerf

------------------
Setup Environment:
------------------

The following configuration files are required in the working directory:

- RDMFieldDictionary and enumtype.def, located in the etc directory.
- MsgData.xml, located in PerfTools/Common
- 350k.xml, located in PerfTools/Common

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters. The full set of configurable
parameters is printed to the screen.

or

dotnet run -- [arguments]

To view all command-line options, run:

dotnet run -- -help

Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the NIProvPerf project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

NIProvPerf.cs - The main file for the NIProvPerf application.

NIDirectoryProvider.cs - Handles publishing the provider's source directory.

NIProviderThread.cs - Handles the publishing of refreshes and updates for open items.
