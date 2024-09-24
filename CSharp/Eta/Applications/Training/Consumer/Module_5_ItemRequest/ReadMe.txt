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
a FieldList. Field-value pairs contained in the field list include information
related to that item (i.e., net change, bid, ask, volume, high, low, or last price).

A Market Price request message is encoded and sent by OMM consumer applications. The
request specifies the name and attributes of an item in which the consumer is
interested. If a consumer wishes to receive updates, it can make a "streaming"
request by setting the RequestMsgFlags.STREAMING flag. If the flag is not set, the consumer
is requesting a "snapshot," and the refresh should end the request.

Market Price data is conveyed as an FieldList, where each FieldEntry
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

dotnet run

Runs with a default set of parameters (-h localhost -p 14002 -i "" -s "DIRECT_FEED" -mp "TRI")

or

dotnet run -- [-h <SrvrHostname>] [-p <SrvrPortNo>] [-i <InterfaceName>] [-s <ServiceName>] [-mp <ItemName>]

To view all command-line options, run:

dotnet run -- -?

Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the ConsMod5 project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_5_ItemRequest.cs - The main file for the Transport API Consumer Training application.
