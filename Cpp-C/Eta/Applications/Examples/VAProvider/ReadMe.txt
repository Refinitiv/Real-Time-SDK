rsslVAProvider Application Description

--------
Summary:
--------

The purpose of this application is to demonstrate interactively providing
data to OMM Consumer applications using ValueAdd components. It is a 
single-threaded server application.

The provider application implements callbacks that process requests
received by the consumer. It creates an RsslServer and an RsslReactor and
waits for connections. It accepts these connections into the RsslReactor
and responds to messages through its Login, Directory, Dictionary, and 
default message callbacks.

This provider supports providing Level I Market Price, Level II Market By
Order, Level II Market By Price, and Symbol List data.

Level II Market By Price refresh messages are sent as multi-part messages. An update
message is sent between each part of the multi-part refresh message.

Batch requests are supported by this application. The login response message indicates
that batch support is present. Batch requests are accepted and a stream is opened
for each item in the batch request.

Posting requests are supported by this application for items that have already been
opened by a consumer. On-stream and off-stream posts are accepted and sent out to any
consumer that has the item open. Off-stream posts for items that have not already
been opened by a consumer are rejected (in this example).

Symbol List requests are expected to use a symbol list name of "_ETA_ITEM_LIST". The
symbol list name is provided in the source directory response for the consumer to use.

This application can optionally use the Value Add payload cache component. If the cache 
option is enabled, the OMM payload data published with refresh and update
responses will be applied to the cache entry for the item. Once an item has been cached, 
any refresh responses to be sent for the item will be encoded from the cache entry. The
cache is demonstrated with domains Market Price, Market By Order, and Market By Price.

This provider supports accepting tunnel streams and exchanging simple messages
between the provider and consumer.

This application is intended as a basic usage example. Some of the design choices
were made to favor simplicity and readability over performance. This application
is not intended to be used for measuring performance.
 
-----------------
Application Name:
-----------------

VAProvider

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
must be located in the directory of execution. If the dictionary files
cannot be found, the provider application exits.

-------------------
Command line usage:
-------------------  

VAProvider
(runs with a default set of parameters (-p 14002 -s DIRECT_FEED -id 1))

or

VAProvider [ -p <PortNo> ] [ -s <ServiceName> ] [ -id <ServiceId> ] [-runtime <seconds>] [-cache]

Specifying the -runtime option controls the time the application will run
before exiting, in seconds.

- VAProvider -? displays command line options.  

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

rsslProvider.c - The main file for the rsslVAProvider application. Provides the default message callback.

rsslDictionaryProvider.c - Provides the Dictionary domain message callback function and handles responding to requests.

rsslDirectoryHandler.c - Provides the Directory domain message callback function and and handles responding to requests.

rsslLoginHandler.c - Provides the Login domain message callback function and handles responding to requests.

rsslItemHandler.c - The handler for item requests. Supports Market Price, Market By Order,
Market By Price, and Symbol List domains.

rsslVAItemEncode.c - Contains non-domain-specific encoding functionality for items.

rsslVAMarketPriceItems.c - Contains functionality for generating and encoding Market Price data.

rsslVAMarketByOrderItems.c - Contains functionality for generating and encoding Market By Order data.

rsslVAMarketByPriceItems.c - Contains functionality for generating and encoding Market By Price data.

rsslVASymbolListItems.c - Contains functionality for generating and encoding Symbol List data. 

rsslVASendMessage.c - Utility functions for sending messages.

rsslVACacheHandler.c - Utility functions for applying and retrieving cache payload data.

tunnelStreamHandler.c - Provides tunnel stream management, used by the SimpleTunnelStreamMsgHandler.

simpleTunnelMsgHandler.c - Provides functionality for sending and receiving basic messages via a tunnel stream.

