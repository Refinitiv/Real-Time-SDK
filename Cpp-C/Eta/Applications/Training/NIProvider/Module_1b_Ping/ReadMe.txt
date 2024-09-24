Transport API Non-Interactive Provider (NIP) Training Application Description
Module 1b: Ping (heartbeat) Management

--------
Summary:
--------

A Non-Interactive Provider (NIP) writes a provider application that 
connects to LSEG Real-Time Distribution System and sends a specific
set (non-interactive) of information (services, domains, and capabilities).
NIPs act like clients in a client-server relationship. Multiple NIPs can
connect to the same LSEG Real-Time Distribution System and publish
the same items and content.

In this module, after establishing a connection, ping messages might 
need to be exchanged. The negotiated ping timeout is available via 
the RsslChannel. If ping heartbeats are not sent or received within 
the expected time frame, the connection can be terminated. LSEG 
recommends sending ping messages at intervals one-third the 
size of the ping timeout.

Detailed Descriptions:
Ping or heartbeat messages are used to indicate the continued presence of 
an application. These are typically only required when no other information 
is being exchanged. For example, there may be long periods of time that 
elapse between requests made from an OMM NIP application to ADH Infrastructure.
In this situation, the NIP would send periodic heartbeat messages to inform 
the ADH Infrastructure that it is still alive. Because the ADH Infrastructure 
is likely sending more frequent information, providing updates 
on any streams the NIP has requested, it may not need to send 
heartbeats as the other data is sufficient to announce its continued 
presence. It is the responsibility of each connection to manage the sending
and receiving of heartbeat messages.
 
-----------------
Application Name:
-----------------

NIProvMod1b

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the ADH Infrastructure.

-------------------
Command line usage:
-------------------  

NIProvMod1b
(runs with a default set of parameters (-h localhost -p 14003 -i ""))

or

NIProvMod1b [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
			
- NIProvMod1b -? displays command line options.  

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

