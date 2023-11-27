Provider Application Description

--------
Summary:
--------

The purpose of this application is to interactively provide Level I Market
Price, Level II Market By Order, Level II Market By Price, and Symbol List data
to one or more consumers.

It is a single-threaded server application. First the application initializes
the RSSL transport and binds the server. After that, it loads dictionary
information from the RDMFieldDictionary and enumtype.def files. Finally, it
processes login, source directory, dictionary, market price, market by order,
market by price, and symbol list requests from consumers and sends the
appropriate responses.

Level II Market By Price refresh messages are sent as multi-part messages. An
update message is sent between each part of the multi-part refresh message.

Batch requests are supported by this application. The login response message
indicates that batch support is present. Batch requests are accepted and a
stream is opened for each item in the batch request.

Posting requests are supported by this application for items that have already
been opened by a consumer. On-stream and off-stream posts are accepted and sent
out to any consumer that has the item open. Off-stream posts for items that have
not already been opened by a consumer are rejected (in this example).

Private stream requests are also supported by this application. All items
requested with the private stream flag set in the request message result in the
private stream flag set in the applicable response messages. If a request is
received without the private stream flag set for the item name of "RES-DS", this
application redirects the consumer to open the "RES-DS" item on a private stream
instead of a normal stream.

Symbol List requests are expected to use a symbol list name of
"_ETA_ITEM_LIST". The symbol list name is provided in the source directory
response for the consumer to use.

This application is intended as a basic usage example. Some of the design
choices were made to favor simplicity and readability over performance. This
application is not intended to be used for measuring performance.

-----------------
Application Name:
-----------------

Provider

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory must
be located in the directory of execution. If the dictionary files cannot be
found, the provider application exits.

-------------------
Command line usage:
-------------------

dotnet run

(runs with a default set of parameters (-p 14002 -s DIRECT_FEED -id 1))

or

dotnet run -- [ -p <PortNo> ] [ -s <ServiceName> ] [ -id <ServiceId> ] [-runtime <seconds>]

Specifying the -runtime option controls the time the application will run
before exiting, in seconds.

To view all command-line options, run:

dotnet run -- -help

- Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the Provider project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Provider.cs - The main file for the Provider application.

ItemHandler.cs - The handler for item requests. Supports Market Price, Market By Order,
  Market By Price, and Symbol List domains.
