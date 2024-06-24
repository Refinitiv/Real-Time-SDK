Transport API Interactive Provider Training Application Description
Module 1b: Ping (heartbeat) Management

--------
Summary:
--------

In this module, after establishing a connection, ping messages might 
need to be exchanged. The negotiated ping timeout is available via 
the RsslChannel. If ping heartbeats are not sent or received within 
the expected time frame, the connection can be terminated. LSEG 
recommends sending ping messages at intervals one-third the 
size of the ping timeout.

Detailed Descriptions:
Once the connection is active, the consumer and provider applications 
might need to exchange ping messages. A negotiated ping timeout is available 
via RsslChannel corresponding to each connection (this value might differ on
a per-connection basis). A connection can be terminated if ping heartbeats 
are not sent or received within the expected time frame. LSEG 
recommends sending ping messages at intervals one-third the size of the ping timeout.
Ping or heartbeat messages are used to indicate the continued presence of 
an application. These are typically only required when no other information is 
being exchanged. Because the provider application is likely sending more frequent 
information, providing updates on any streams the consumer has requested, 
it may not need to send heartbeats as the other data is sufficient to announce 
its continued presence. It is the responsibility of each connection to manage 
the sending and receiving of heartbeat messages.
 
-----------------
Application Name:
-----------------

ProvMod1b

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod1b
(runs with a default set of parameters (-p 14002))

or

ProvMod1b [-p <SrvrPortNo>]   

- ProvMod1b -? displays command line options. 

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

Provider_Training.c - The main file for the Transport API Interactive Provider Training application.

