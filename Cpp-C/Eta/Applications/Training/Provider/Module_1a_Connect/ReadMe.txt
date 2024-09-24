Transport API Interactive Provider Training Application Description
Module 1a: Establish network communication

--------
Summary:
--------

An OMM Interactive Provider application opens a listening socket on a well-known 
port allowing OMM consumer applications to connect. Once connected, consumers 
can request data from the Interactive Provider.

In this module, the OMM Interactive Provider application opens a listening socket 
on a well-known port allowing OMM consumer applications to connect.

Detailed Descriptions:
The first step of any Transport API Interactive Provider application is to establish 
a listening socket, usually on a well-known port so that consumer applications 
can easily connect. The provider uses the rsslBind function to open the port 
and listen for incoming connection attempts.
Whenever an OMM consumer application attempts to connect, the provider uses 
the rsslAccept function to begin the connection initialization process.

For this simple training app, the interactive provider only supports a single client. 
 
-----------------
Application Name:
-----------------

ProvMod1a

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod1a
(runs with a default set of parameters (-p 14002))

or

ProvMod1a [-p <SrvrPortNo>]   

- ProvMod1a -? displays command line options. 

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

