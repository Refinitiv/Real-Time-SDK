Transport API Interactive Provider Training Application Description
Module 5: Handle Item Requests

--------
Summary:
--------

In this module, OMM Interactive Provider application handles Item Requests. Once 
connected, consumers can request data from the Interactive Provider. A provider can 
receive a request for any domain, though this should typically be limited to the 
domain capabilities indicated in the Source Directory. In this simple example, we 
are sending just 1 Market Price item response message to a channel. 

Detailed Descriptions:
A provider can receive a request for any domain, though this should typically be 
limited to the domain capabilities indicated in the Source Directory. When a request 
is received, the provider application must determine if it can satisfy the request by:

a) Comparing msgKey identification information
b) Determining whether it can provide the requested QoS
c) Ensuring that the consumer does not already have a stream open for the requested 
information

If a provider can service a request, it should send appropriate responses. However, 
if the provider cannot satisfy the request, the provider should send an RsslStatusMsg 
to indicate the reason and close the stream. All requests and responses should follow 
specific formatting as defined in the domain model specification. The Transport API RDM Usage
Guide defines all domains provided by LSEG.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ProvMod5

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod5
(runs with a default set of parameters (-p 14002 -s "DIRECT_FEED"))

or

ProvMod5 [-p <SrvrPortNo>] [-s <ServiceName>] 
			
- ProvMod5 -? displays command line options.  

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

