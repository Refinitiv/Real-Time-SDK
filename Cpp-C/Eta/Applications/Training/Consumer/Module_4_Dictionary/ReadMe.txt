Transport API Consumer Training Application Description
Module 4: Obtain Dictionary Information

--------
Summary:
--------

Consumer applications often require a dictionary for encoding or decoding 
specific pieces of information. This dictionary typically defines type and 
formatting information. Content that uses the RsslFieldList type requires 
the use of a field dictionary (usually the LSEG RDMFieldDictionary, 
although it could also be a user-defined or user-modified field dictionary).
A consumer application can choose whether to load necessary dictionary 
information from a local file or download the information from an available 
provider.

Detailed Descriptions:
The Source Directory message should inform (from previous Module 3):
- DictionariesProvided: Which dictionaries are available for download.
- DictionariesUsed: The consumer of any dictionaries required to decode 
  the content provided on a service. (Not used in previous Module 3)

A consumer application can determine whether to load necessary dictionary 
information from a local file or download the information from the
provider if available.
 
- If loading from a file, Transport API offers several utility functions to load and 
  manage a properly-formatted field dictionary.
- If downloading information, the application issues a request using the 
  Dictionary domain model. The provider application should respond with a 
  dictionary response, typically broken into a multi-part message. Transport API 
  offers several utility functions for encoding and decoding of the
  Dictionary domain content.
 
Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ConsMod4

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------  

ConsMod4
(runs with a default set of parameters (-h localhost -p 14002 -i "" -s "DIRECT_FEED"))

or

ConsMod4 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] 
			
- ConsMod4 -? displays command line options.  

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

