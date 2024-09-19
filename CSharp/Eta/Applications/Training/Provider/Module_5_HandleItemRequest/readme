Transport API Interactive Provider Training Application Description

Module 5: Handle Item Requests

--------
Summary:
--------

In this module, OMM Interactive Provider application handles Item Requests. Once
connected, consumers can request data from the Interactive Provider. A provider can
receive a request for any domain, though this should typically be limited to the
domain capabilities indicated in the Source Directory. In this simple example, we
are sending just 1 Market Price item response message to a channel.

Detailed Descriptions:

A provider can receive a request for any domain, though this should typically be
limited to the domain capabilities indicated in the Source Directory. When a request
is received, the provider application must determine if it can satisfy the request by:

a) Comparing msgKey identification information
b) Determining whether it can provide the requested QoS
c) Ensuring that the consumer does not already have a stream open for the requested
   information

If a provider can service a request, it should send appropriate responses. However,
if the provider cannot satisfy the request, the provider should send a StatusMsg
to indicate the reason and close the stream. All requests and responses should follow
specific formatting as defined in the domain model specification. The Transport API RDM Usage
Guide defines all domains provided by LSEG.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

ProvMod5

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-p 14002 -r 300 -s DIRECT_FEED)

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
file and build the ProvMod5 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_5_HandleItemRequest.cs - The main file for the Transport API Interactive Provider Training application.
