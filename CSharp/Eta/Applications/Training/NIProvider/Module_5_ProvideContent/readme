Transport API Non-Interactive Provider (NIP) Training Application Description

Module 5: Provide Content

--------
Summary:
--------

A Non-Interactive Provider (NIP) writes a provider application that
connects to LSEG Real-Time Distribution and sends a specific set
(non-interactive) of information (services, domains, and capabilities).

NIPs act like clients in a client-server relationship. Multiple NIPs can
connect to the same LSEG Real-Time Distribution and publish the same
items and content. The NIP application sends a login request like a consumer
to the ADH and processes source directory and item requests like a provider.
A source directory refresh message and market price and/or market by order
refresh/update messages are published to the ADH without any request for them.

In this module, after providing a Source Directory, the OMM NIP application
can begin pushing content to the ADH.

Detailed Descriptions:

After providing a Source Directory, the NIP application can begin
pushing content to the ADH. Each unique information stream should
begin with an RefreshMsg, conveying all necessary identification
information for the content. Because the provider instantiates this
information, a negative value streamId should be used for all streams.
The initial identifying refresh can be followed by other status or
update messages. Some ADH functionality, such as cache rebuilding, may
require that NIP applications publish the message key on all RefreshMsgs.
See the component specific documentation for more information.

Note:

Some components may require that NIP applications publish the msgKey in
UpdateMsgs. To avoid component or transport migration issues, NIP applications
may want to always include this information.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

NIProvMod5

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the ADH Infrastructure.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1 -mp TRI)

or

dotnet run -- [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-id <Service ID>] [-mp <Market Price Item Name>]

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
file and build the NIProvMod5 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_5_ProvideContent.cs - The main file for the Transport API Non-Interactive Provider (NIP) Training application.
