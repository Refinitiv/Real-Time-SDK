Transport API Consumer Training Application Description

Module 1a: Establish network communication

--------
Summary:
--------

In this module, the application initializes the Transport API and
connects the client. An OMM consumer application can establish a
connection to other OMM Interactive Provider applications, including
LSEG Real-Time Distribution Systems, Data Feed Direct,
and LSEG Real-Time.

Detailed Descriptions:

The first step of any Transport API consumer application is to establish a
network connection with its peer component (i.e., another application
with which to interact). An OMM consumer typically creates an outbound
connection to the well-known hostname and port of a server (Interactive
Provider or ADS). The consumer uses the Connect() method to initiate
the connection and then uses the InitChannel() method to complete
channel initialization.

-----------------
Application Name:
-----------------

ConsMod1a

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
file and build the ConsMod1a project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_1a_Connect.cs - The main file for the Transport API Consumer Training application.
