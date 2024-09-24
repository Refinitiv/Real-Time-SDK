Transport API Consumer Training Application Description

Module 3: Obtain Source Directory

--------
Summary:
--------

The Source Directory domain model conveys information about all available
services in the system. An OMM consumer typically requests a Source
Directory to retrieve information about available services and their
capabilities. This includes information about supported domain types, the
service's state, the quality of service (QoS), and any item group
information associated with the service.

Detailed Descriptions:

The Source Directory Info filter contains service name and serviceId
information for all available services. When an appropriate service is
discovered by the OMM consumer, it uses the serviceId associated with the
service on all subsequent requests to that service.

The Source Directory State filter contains status information for service,
which informs the consumer whether the service is Up and available, or Down
and unavailable.

The Source Directory Group filter conveys item group status information,
including information about group states, as well as the merging of groups.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

ConsMod3

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-h localhost -p 14002 -i "" -s "DIRECT_FEED")

or

dotnet run -- [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>]

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
file and build the ConsMod3 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_3_Directory.cs - The main file for the Transport API Consumer Training application.
