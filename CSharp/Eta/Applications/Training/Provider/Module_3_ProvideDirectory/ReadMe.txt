Transport API Interactive Provider Training Application Description

Module 3: Provide Source Directory Information

--------
Summary:
--------

In this module, OMM Interactive Provider application provides Source Directory
information. The Source Directory domain model conveys information about all
available services in the system. An OMM consumer typically requests a Source Directory
to retrieve information about available services and their capabilities.

Detailed Descriptions:

The Source Directory domain model conveys information about all available services
in the system. An OMM consumer typically requests a Source Directory to retrieve
information about available services and their capabilities. This includes information
about supported domain types, the service's state, the QoS, and any item group
information associated with the service. LSEG recommends that at a minimum,
an Interactive Provider supply the Info, State, and Group filters for the Source Directory.

a) The Source Directory Info filter contains the name and serviceId for each
   available service. The Interactive Provider should populate the filter with information
   specific to the services it provides.

b) The Source Directory State filter contains status information for the service informing
   the consumer whether the service is Up (available), or Down (unavailable).

c) The Source Directory Group filter conveys item group status information, including
   information about group states, as well as the merging of groups. If a provider determines
   that a group of items is no longer available, it can convey this information by sending
   either individual item status messages (for each affected stream) or a Directory message
   containing the item group status information.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

ProvMod3

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-p 14002 -r 300 -s DIRECT_FEED )

or

dotnet run -- [-p <SrvrPortNo>] [-r <Running Time>] [-s <Service Name>]

To view all command-line options, run:

dotnet run -- -?

Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the ProvMod3 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_3_ProvideDirectory.cs - The main file for the Transport API Interactive Provider Training application.
