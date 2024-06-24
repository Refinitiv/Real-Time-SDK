Transport API Non-Interactive Provider (NIP) Training Application Description
Module 2: Log in

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

In this module, applications authenticate with one another using the Login 
domain model. An OMM NIP must register with the system using a Login request 
prior to providing any content. Because this is done in an interactive manner, 
the NIP should assign a streamId with a positive value which the ADH will 
reference when sending its response.

Detailed Descriptions:
After receiving a Login request, the ADH determines whether the NIP is 
permissioned to access the system. The ADH sends a Login response, indicating 
to the NIP whether the ADH grants it access. 

a) If the application is denied, the ADH closes the Login stream and the 
NI provider application cannot perform any additional communication. 
b) If the application gains access to the ADH, the Login response informs 
the application of this. The NI provider must now provide a Source Directory.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.  
 
-----------------
Application Name:
-----------------

NIProvMod2

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the ADH Infrastructure.

-------------------
Command line usage:
-------------------  

NIProvMod2
(runs with a default set of parameters (-h localhost -p 14003 -i ""))

or

NIProvMod2 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]

- NIProvMod2 -? displays command line options.  

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

