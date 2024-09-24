Transport API Interactive Provider Training Application Description
Module 1c: Reading and Writing Data

--------
Summary:
--------

In this module, when a client or server RsslChannel.state is 
RSSL_CH_STATE_ACTIVE, it is possible for an application to receive 
data from the connection. Similarly, when a client or server 
RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is possible for an 
application to write data to the connection. Writing involves a several 
step process. 

Detailed Descriptions:
When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
possible for an application to receive data from the connection. The 
arrival of this information is often announced by the I/O notification 
mechanism that the RsslChannel.socketId is registered with. The Transport API 
reads information from the network as a byte stream, after 
which it determines RsslBuffer boundaries and returns each buffer one by 
one.

When a client or server RsslChannel.state is RSSL_CH_STATE_ACTIVE, it is 
possible for an application to write data to the connection. Writing 
involves a several step process. Because the Transport API provides 
efficient buffer management, the user is required to obtain a buffer 
from the Transport API buffer pool. This can be the guaranteed output 
buffer pool associated with an RsslChannel. After a buffer is acquired, 
the user can populate the RsslBuffer.data and set the RsslBuffer.length 
to the number of bytes referred to by data. If queued information cannot 
be passed to the network, a function is provided to allow the application 
to continue attempts to flush data to the connection. An I/O notification
mechanism can be used to help with determining when the network is able 
to accept additional bytes for writing. The Transport API can continue to
queue data, even if the network is unable to write. 
 
-----------------
Application Name:
-----------------

ProvMod1c

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod1c
(runs with a default set of parameters (-p 14002))

or

ProvMod1c [-p <SrvrPortNo>]   

- ProvMod1c -? displays command line options. 

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

