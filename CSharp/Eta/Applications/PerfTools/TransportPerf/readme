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

dotnet run
dotnet run -- -appType client

The applications will connect to each other and begin exchanging messages.

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
file and build the TransportPerf project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

TransportPerf.cs - The main file for the TransportPerf application.

TransportPerfConfig.cs - Providers configurable options for the application.

TransportThread.cs - Handles each connection and the sending and receiving of
  messages.
