Transport API Interactive Provider Training Application Description
Module 3: Provide Source Directory Information

--------
Summary:
--------

In this module, OMM Interactive Provider application provides Source Directory 
information. The Source Directory domain model conveys information about all 
available services in the system. An OMM consumer typically requests a Source Directory 
to retrieve information about available services and their capabilities. 
 
Detailed Descriptions:
The Source Directory domain model conveys information about all available services 
in the system. An OMM consumer typically requests a Source Directory to retrieve 
information about available services and their capabilities. This includes information 
about supported domain types, the service�s state, the QoS, and any item group 
information associated with the service. LSEG recommends that at a minimum, 
an Interactive Provider supply the Info, State, and Group filters for the Source Directory.
 
a) The Source Directory Info filter contains the name and serviceId for each 
available service. The Interactive Provider should populate the filter with information 
specific to the services it provides.

b) The Source Directory State filter contains status information for the service informing 
the consumer whether the service is Up (available), or Down (unavailable).

c) The Source Directory Group filter conveys item group status information, including 
information about group states, as well as the merging of groups. If a provider determines 
that a group of items is no longer available, it can convey this information by sending 
either individual item status messages (for each affected stream) or a Directory message 
containing the item group status information. 
 
Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ProvMod3

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, we will exit the interactive provider application.

-------------------
Command line usage:
-------------------  

ProvMod3
(runs with a default set of parameters (-p 14002 -s "DIRECT_FEED"))

or

ProvMod3 [-p <SrvrPortNo>] [-s <ServiceName>] 
			
- ProvMod3 -? displays command line options.  

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

