NIProvider Application Description

--------
Summary:
--------

The purpose of this application is to non-interactively provide Level I Market
Price and Level 2 Market By Order data to an Advanced Data Hub (ADH). It is a
single-threaded client application. First the application initializes the RSSL
transport and connects the client. After that, it attempts to load the
dictionary from a file.

The application sends a login request like a consumer to the ADH.

A source directory refresh message is published to the ADH without any request
for it.

If the dictionary could not be loaded from a file and the login response from
ADH indicates that the ADH does support the Provider Dictionary Download feature
the application will send dictionary request like a consumer and proccess the
dictionary refresh message.

If the ADH login response indicates no support for the Provider Dictionary
Download feature the application will exit.

A market price and/or market by order refresh/update messages are published to
the ADH without any request for them.

If the dictionary is found in the directory of execution, then it is loaded
directly from the file. However, the default configuration for this application
is to request the dictionary from the ADH. Hence, no link to the dictionary is
made in the execution directory by the build script. The user can change this
behavior by manually creating a link to the dictionary in the execution
directory.

Reliable multicast can be used to communicate between this application and any
ADH on the network. The non-interactive provider can then send one message to
all ADH's on the network instead of having to fan-out messages to each ADH
TCP/IP connection.

This application is intended as a basic usage example. Some of the design choices
were made to favor simplicity and readability over performance. This application
is not intended to be used for measuring performance.

-----------------
Application Name:
-----------------

NIProvider

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory can
be located in the directory of execution.

If the dictionary files cannot be found and the ADH does support the Provider
Dictionary Download feature, they are requested from the ADH via dictionary
request message.

If dictionary files can not be located in the directory of execution and ADH
does not support the Provider Dictionary Download feature the non-interactive
provider application exits.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-h localhost -p 14003 -s NI_PUB -mp TRI.N,IBM.N -runtime 600)

or

dotnet run -- -help

To view all command-line options.

- Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the NIProvider project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

NIProvider.cs - The main file for the NIProvider application.
