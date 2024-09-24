Transport API Non-Interactive Provider (NIP) Training Application Description
Module 5: Provide Content

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

In this module, after providing a Source Directory, the OMM NIP application 
can begin pushing content to the ADH. 

Detailed Descriptions:
After providing a Source Directory, the NIP application can begin 
pushing content to the ADH. Each unique information stream should 
begin with an RsslRefreshMsg, conveying all necessary identification 
information for the content. Because the provider instantiates this 
information, a negative value streamId should be used for all streams. 
The initial identifying refresh can be followed by other status or 
update messages. Some ADH functionality, such as cache rebuilding, may 
require that NIP applications publish the message key on all RsslRefreshMsgs. 
See the component specific documentation for more information.

Note:
Some components may require that NIP applications publish the msgKey in 
RsslUpdateMsgs. To avoid component or transport migration issues, NIP applications 
may want to always include this information.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

NIProvMod5

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the ADH Infrastructure.

-------------------
Command line usage:
-------------------  

NIProvMod5
(runs with a default set of parameters (-h localhost -p 14003 -i "" -s "DIRECT_FEED" -mp "TRI"))

or

NIProvMod5 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] [-mp <ItemName>] 
			
- NIProvMod5 -? displays command line options.  

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

