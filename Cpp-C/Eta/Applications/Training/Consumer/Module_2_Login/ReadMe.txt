Transport API Consumer Training Application Description
Module 2: Log in

--------
Summary:
--------

Applications authenticate using the Login domain model. An OMM consumer must 
authenticate with a provider using a Login request prior to issuing any other 
requests or opening any other streams. After receiving a Login request, an 
Interactive Provider determines whether a user is permissioned to access the 
system. The Interactive Provider sends back a Login response, indicating to 
the consumer whether access is granted.

Detailed Descriptions:
After receiving a Login request, an Interactive Provider determines whether 
a user is permissioned to access the system. The Interactive Provider sends 
back a Login response, indicating to the consumer whether access is granted.

a) If the application is denied, the Login stream is closed, and the 
consumer application cannot send additional requests.
b) If the application is granted access, the Login response contains 
information about available features, such as Posting, Pause and Resume, 
and the use of Dynamic Views. The consumer application can use this 
information to tailor its interaction with the provider.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.  

-----------------
Application Name:
-----------------

ConsMod2

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------  

ConsMod2
(runs with a default set of parameters (-h localhost -p 14002 -i ""))

or

ConsMod2 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>]
			
- ConsMod2 -? displays command line options.  

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

Consumer_Training.c - The main file for the Transport API Consumer Training application.

