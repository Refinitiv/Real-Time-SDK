Transport API Interactive Provider Training Application Description

Module 1a: Establish network communication

--------
Summary:
--------

An OMM Interactive Provider application opens a listening socket on a well-known
port allowing OMM consumer applications to connect. Once connected, consumers
can request data from the Interactive Provider.

In this module, the OMM Interactive Provider application opens a listening socket
on a well-known port allowing OMM consumer applications to connect.

Detailed Descriptions:

The first step of any Transport API Interactive Provider application is to establish
a listening socket, usually on a well-known port so that consumer applications
can easily connect. The provider uses the Bind function to open the port
and listen for incoming connection attempts.

Whenever an OMM consumer application attempts to connect, the provider uses
the Accept function to begin the connection initialization process.

For this simple training app, the interactive provider only supports a single client.

-----------------
Application Name:
-----------------

ProvMod1a

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-p 14002)

or

dotnet run -- [-p <SrvrPortNo>]

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
file and build the ProvMod1a project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_1a_Connect.cs - The main file for the Transport API Interactive Provider Training application.
