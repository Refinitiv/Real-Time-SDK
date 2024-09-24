Transport API Interactive Provider Training Application Description

Module 2: Perform/Handle Login Process

--------
Summary:
--------

Applications authenticate with one another using the Login domain model.
An OMM Interactive Provider must handle the consumer's Login request messages
and supply appropriate responses.

In this module, after receiving a Login request, the Interactive Provider
can perform any necessary authentication and permissioning.

Detailed Descriptions:

After receiving a Login request, the Interactive Provider can perform any
necessary authentication and permissioning.

a) If the Interactive Provider grants access, it should send an RefreshMsg
   to convey that the user successfully connected. This message should indicate
   the feature set supported by the provider application.
b) If the Interactive Provider denies access, it should send an StatusMsg,
   closing the connection and informing the user of the reason for denial.

The login handler for this simple Interactive Provider application only allows
one login stream per channel. It provides functions for processing login requests
from consumers and sending back the responses. Functions for sending login request
reject/close status messages, initializing the login handler, and closing login streams
are also provided.

Also please note for simple training app, the interactive provider only supports
one client session from the consumer, that is, only supports one channel/client connection.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

ProvMod2

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-p 14002 -r 300)

or

dotnet run -- [-p <SrvrPortNo>] [-r <Running Time>]

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
file and build the ProvMod2 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_2_Login.cs - The main file for the Transport API Interactive Provider Training application.
