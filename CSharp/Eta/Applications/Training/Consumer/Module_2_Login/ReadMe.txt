Transport API Consumer Training Application Description

Module 2: Log in

--------
Summary:
--------

Applications authenticate using the Login domain model. An OMM consumer must
authenticate with a provider using a Login request prior to issuing any other
requests or opening any other streams. After receiving a Login request, an
Interactive Provider determines whether a user is permissioned to access the
system. The Interactive Provider sends back a Login response, indicating to
the consumer whether access is granted.

Detailed Descriptions:

After receiving a Login request, an Interactive Provider determines whether
a user is permissioned to access the system. The Interactive Provider sends
back a Login response, indicating to the consumer whether access is granted.

a) If the application is denied, the Login stream is closed, and the
   consumer application cannot send additional requests.

b) If the application is granted access, the Login response contains
   information about available features, such as Posting, Pause and Resume,
   and the use of Dynamic Views. The consumer application can use this
   information to tailor its interaction with the provider.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

ConsMod2

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
file and build the ConsMod2 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_2_Login.cs - The main file for the Transport API Consumer Training application.
