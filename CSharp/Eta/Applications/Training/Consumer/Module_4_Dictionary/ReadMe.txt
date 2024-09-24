Transport API Consumer Training Application Description

Module 4: Obtain Dictionary Information

--------
Summary:
--------

Consumer applications often require a dictionary for encoding or decoding 
specific pieces of information. This dictionary typically defines type and 
formatting information. Content that uses the FieldList type requires 
the use of a field dictionary (usually the LSEG RDMFieldDictionary, 
although it could also be a user-defined or user-modified field dictionary).

A consumer application can choose whether to load necessary dictionary 
information from a local file or download the information from an available 
provider.

Detailed Descriptions:

The Source Directory message should inform (from previous Module 3):
- DictionariesProvided: Which dictionaries are available for download.
- DictionariesUsed: The consumer of any dictionaries required to decode 
  the content provided on a service. (Not used in previous Module 3)

A consumer application can determine whether to load necessary dictionary 
information from a local file or download the information from the
provider if available.
 
- If loading from a file, Transport API offers several utility functions to load and 
  manage a properly-formatted field dictionary.
- If downloading information, the application issues a request using the 
  Dictionary domain model. The provider application should respond with a 
  dictionary response, typically broken into a multi-part message. Transport API 
  offers several utility functions for encoding and decoding of the
  Dictionary domain content.
 
Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ConsMod4

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
file and build the ConsMod4 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_4_Dictionary.cs - The main file for the Transport API Consumer Training application.

