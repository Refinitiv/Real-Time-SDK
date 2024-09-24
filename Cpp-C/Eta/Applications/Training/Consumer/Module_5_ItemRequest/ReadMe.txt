Transport API Consumer Training Application Description
Module 5: Issue Item Requests 

--------
Summary:
--------

After the consumer application successfully logs in and obtains Source 
Directory and Dictionary information, it can request additional content. 
When issuing the request, the consuming application specifies the serviceId 
of the desired service along with a streamId. Requests can be sent for any 
domain using the formats defined in that domain model specification. In this 
simple example, we show how to make a Market Price level I data Item request 
to obtain the data from a provider.

Detailed Descriptions:
The Market Price domain provides access to Level I market information such as 
trades, indicative quotes, and top-of-book quotes. All information is sent as 
an RsslFieldList. Field-value pairs contained in the field list include information 
related to that item (i.e., net change, bid, ask, volume, high, low, or last price).

A Market Price request message is encoded and sent by OMM consumer applications. The 
request specifies the name and attributes of an item in which the consumer is 
interested. If a consumer wishes to receive updates, it can make a "streaming"
request by setting the RSSL_RQMF_STREAMING flag. If the flag is not set, the consumer 
is requesting a "snapshot," and the refresh should end the request.

Market Price data is conveyed as an RsslFieldList, where each RsslFieldEntry 
corresponds to a piece of information and its current value. The field list should be 
decoded using its associated Field Dictionary, indicated by the dictionaryId present 
in the field list.
 
Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.

-----------------
Application Name:
-----------------

ConsMod5

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------  

ConsMod5
(runs with a default set of parameters (-h localhost -p 14002 -i "" -s "DIRECT_FEED" -mp "TRI"))

or

ConsMod5 [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] [-mp <ItemName>] 
			
- ConsMod5 -? displays command line options.  

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

