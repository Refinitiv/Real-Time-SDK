Transport API Consumer Training Application Description
Module 3: Obtain Source Directory

--------
Summary:
--------

The Source Directory domain model conveys information about all available 
services in the system. An OMM consumer typically requests a Source 
Directory to retrieve information about available services and their 
capabilities. This includes information about supported domain types, the 
service�s state, the quality of service (QoS), and any item group 
information associated with the service.

Detailed Descriptions:
The Source Directory Info filter contains service name and serviceId 
information for all available services. When an appropriate service is 
discovered by the OMM consumer, it uses the serviceId associated with the 
service on all subsequent requests to that service. 

The Source Directory State filter contains status information for service, 
which informs the consumer whether the service is Up and available, or Down
and unavailable. 

The Source Directory Group filter conveys item group status information, 
including information about group states, as well as the merging of groups. 

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ConsMod3

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------  

ConsMod3
(runs with a default set of parameters (-h localhost -p 14002 -i "" -s "DIRECT_FEED"))

or

ConsMod3 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] 
			
- ConsMod3 -? displays command line options.  

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

