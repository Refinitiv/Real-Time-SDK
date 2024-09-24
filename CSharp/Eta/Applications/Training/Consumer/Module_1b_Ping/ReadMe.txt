Transport API Consumer Training Application Description

Module 1b: Ping (heartbeat) Management

--------
Summary:
--------

Ping or heartbeat messages indicate the continued presence of an application.
After the consumer's connection is active, ping messages must be exchanged.
The negotiated ping timeout is retrieved using the IChannel.Info() method.
The connection will be terminated if ping heartbeats are not sent or received
within the expected time frame.

Detailed Descriptions:

Ping or heartbeat messages are used to indicate the continued presence of
an application. These are typically only required when no other information
is being exchanged. For example, there may be long periods of time that
elapse between requests made from an OMM consumer application. In this
situation, the consumer would send periodic heartbeat messages to inform
the providing application that it is still alive. Because the provider
application is likely sending more frequent information, providing updates
on any streams the consumer has requested, it may not need to send
heartbeats as the other data is sufficient to announce its continued
presence. It is the responsibility of each connection to manage the sending
and receiving of heartbeat messages.

-----------------
Application Name:
-----------------

ConsMod1b

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-h localhost -p 14002 -i "")

or

dotnet run -- [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]

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
file and build the ConsMod1b project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_1b_Ping.cs - The main file for the Transport API Consumer Training application.
