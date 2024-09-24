Transport API Non-Interactive Provider (NIP) Training Application Description
Module 1a: Establish network communication 

--------
Summary:
--------

A Non-Interactive Provider (NIP) writes a provider application that 
connects to LSEG Real-Time Distribution System and sends a specific
set (non-interactive) of information (services, domains, and capabilities).
NIPs act like clients in a client-server relationship. Multiple NIPs can
connect to the same LSEG Real-Time Distribution System and publish
the same items and content.

In this module, the OMM NIP application initializes the Transport API 
and establish a connection to an ADH server. Once connected, an OMM NIP 
can publish information into the ADH cache without needing to handle 
requests for the information. The ADH can cache the information and 
along with other LSEG Real-Time Distribution System components, 
provide the information to any NIProvider applications that indicate interest.

Detailed Descriptions:
The first step of any Transport API NIP application is to establish network 
communication with an ADH server. To do so, the OMM NIP typically creates 
an outbound connection to the well-known hostname and port of an ADH. 
The consumer uses the rsslConnect function to initiate the connection 
process and then performs connection initialization processes as needed.
 
-----------------
Application Name:
-----------------

NIProvMod1a

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the ADH Infrastructure.

-------------------
Command line usage:
-------------------  

NIProvMod1a
(runs with a default set of parameters (-h localhost -p 14003 -i ""))

or

NIProvMod1a [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
			
- NIProvMod1a -? displays command line options.  

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

