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
Content that uses the RsslFieldList type requires the use of a field dictionary 
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

NIProvMod4
(runs with a default set of parameters (-h localhost -p 14003 -i "" -s "DIRECT_FEED"))

or

NIProvMod4 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] 
			
- NIProvMod4 -? displays command line options.  

- Pressing the CTRL+C buttons terminates the program.  

-----------------
Compiling Source:
-----------------

The included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the Transport API supported 
Linux platforms using the supported compilers.

The COMPILE_BITS value in the makefile is used to control
whether 32 or 64 bit version of the application is built.  
If a 64 bit application build is desired, COMPILE_BITS should
be set to 64 (which is the default setting).  If 32 bit 
application build is desired, COMPILE_BITS should be set
to 32.  

The LINKTYPE value in the makefile is used to control
whether the application is built using Transport API static or 
shared libraries. The default build uses Transport API static 
libraries. To use Transport API shared libraries, 
set LINKTYPE=Shared.

To compile, run the gmake command.

Gmake can be obtained at http://www.gnu.org/software/make/
For windows platform, using Visual Studio, open one of the included vcxproj project
files and build.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

NIProvider_Training.c - The main file for the Transport API Non-Interactive Provider (NIP) Training application.

