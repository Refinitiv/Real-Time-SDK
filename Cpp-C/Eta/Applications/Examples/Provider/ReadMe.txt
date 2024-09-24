rsslProvider Application Description

--------
Summary:
--------
 
The purpose of this application is to interactively provide Level I Market Price,
Level II Market By Order, Level II Market By Price, and Symbol List data to one or
more consumers.

It is a single-threaded server application. First the application initializes the
RSSL transport and binds the server. After that, it loads dictionary information
from the RDMFieldDictionary and enumtype.def files. Finally, it processes login,
source directory, dictionary, market price, market by order, market by price,
and symbol list requests from consumers and sends the appropriate responses.

Level II Market By Price refresh messages are sent as multi-part messages. An update
message is sent between each part of the multi-part refresh message.

Batch requests are supported by this application. The login response message indicates
that batch support is present. Batch requests are accepted and a stream is opened
for each item in the batch request.

Posting requests are supported by this application for items that have already been
opened by a consumer. On-stream and off-stream posts are accepted and sent out to any 
consumer that has the item open. Off-stream posts for items that have not already
been opened by a consumer are rejected (in this example). 

Private stream requests are also supported by this application. All items requested
with the private stream flag set in the request message result in the private stream
flag set in the applicable response messages. If a request is received without the
private stream flag set for the item name of "RES-DS", this application redirects
the consumer to open the "RES-DS" item on a private stream instead of a normal stream.

Symbol List requests are expected to use a symbol list name of "_ETA_ITEM_LIST". The
symbol list name is provided in the source directory response for the consumer to use.

This application is intended as a basic usage example. Some of the design choices
were made to favor simplicity and readability over performance. This application
is not intended to be used for measuring performance.
 
-----------------
Application Name:
-----------------

Provider

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
must be located in the directory of execution. If the dictionary files
cannot be found, the provider application exits.

-------------------
Command line usage:
-------------------  

Provider
(runs with a default set of parameters (-p 14002 -s DIRECT_FEED -id 1))

or

Provider [ -p <PortNo> ] [ -s <ServiceName> ] [ -id <ServiceId> ] [-runtime <seconds>]

Specifying the -runtime option controls the time the application will run
before exiting, in seconds.

- Provider -? displays command line options.  

- Pressing the CTRL+C buttons terminates the program.  

-----------------
Compiling Source:
-----------------

The included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the ETA supported Linux
platforms using the supported compilers.

The LINKTYPE value in the makefile is used to control
whether the application is built using ETA static or
shared libraries. The default build uses ETA static
libraries. To use ETA shared libraries,
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

rsslProvider.c - The main file for the rsslProvider application.

rsslDictionaryProvider.c - The dictionary provider for the rsslProvider application.

rsslDirectoryEncodeDecode.c - Contains all source directory encode and decode functions.

rsslDirectoryHandler.c - The source directory handler for the rsslProvider application.

rsslLoginEncodeDecode.c - Contains all login encode and decode functions.

rsslLoginHandler.c - The login handler for the rsslProvider application.

rsslItemHandler.c - The handler for item requests. Supports Market Price, Market By Order,
Market By Price, and Symbol List domains.

rsslItemEncode.c - Contains non-domain-specific encoding functionality for items.

rsslMarketPriceItems.c - Contains functionality for generating and encoding Market Price data.

rsslMarketByOrderItems.c - Contains functionality for generating and encoding Market By Order data.

rsslMarketByPriceItems.c - Contains functionality for generating and encoding Market By Price data.

rsslSymbolListItems.c - Contains functionality for generating and encoding Symbol List data. 

rsslSendMessage.c - Utility functions for sending messages.
