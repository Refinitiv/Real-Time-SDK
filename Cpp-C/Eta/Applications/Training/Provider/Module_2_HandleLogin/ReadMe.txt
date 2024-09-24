Transport API Interactive Provider Training Application Description
Module 2: Perform/Handle Login Process

--------
Summary:
--------

Applications authenticate with one another using the Login domain model. 
An OMM Interactive Provider must handle the consumer�s Login request messages 
and supply appropriate responses.
 
In this module, after receiving a Login request, the Interactive Provider 
can perform any necessary authentication and permissioning.

Detailed Descriptions:
After receiving a Login request, the Interactive Provider can perform any 
necessary authentication and permissioning.

a) If the Interactive Provider grants access, it should send an RsslRefreshMsg 
to convey that the user successfully connected. This message should indicate 
the feature set supported by the provider application.
b) If the Interactive Provider denies access, it should send an RsslStatusMsg, 
closing the connection and informing the user of the reason for denial.

The login handler for this simple Interactive Provider application only allows
one login stream per channel. It provides functions for processing login requests
from consumers and sending back the responses. Functions for sending login request
reject/close status messages, initializing the login handler, and closing login streams 
are also provided.

Also please note for simple training app, the interactive provider only supports 
one client session from the consumer, that is, only supports one channel/client connection.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.   
 
-----------------
Application Name:
-----------------

ProvMod2

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod2
(runs with a default set of parameters (-p 14002))

or

ProvMod2 [-p <SrvrPortNo>] 

- ProvMod2 -? displays command line options.  

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

