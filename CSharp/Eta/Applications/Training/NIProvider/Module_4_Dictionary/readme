Transport API Non-Interactive Provider (NIP) Training Application Description

Module 4: Load Dictionary Information

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

Some data requires the use of a dictionary for encoding or decoding. This
dictionary typically defines type and formatting information and directs
the application as to how to encode or decode specific pieces of information.
Content that uses the FieldList type requires the use of a field dictionary
(usually the LSEG RDMFieldDictionary, though it could also be a
user-defined or modified field dictionary).

Dictionaries may be available locally in a file for an OMM NIP appliation. In
this Training example Module 4, the OMM NIP will use dictionaries that are
available locally in a file.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

-----------------
Application Name:
-----------------

NIProvMod4

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

Runs with a default set of parameters (-h localhost -p 14003 -i "" -r 300 -s DIRECT_FEED -id 1)

or

dotnet run -- [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-r <Running Time>] [-id <Service ID>]

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
file and build the NIProvMod4 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_4_Dictionary.cs - The main file for the Transport API Non-Interactive Provider (NIP) Training application.
