rsslConsumer Application Description

--------
Summary:
--------

The purpose of this application is to consume or post content between an OMM
consumer and OMM provider. It is a single-threaded client application. The
general application flow is that it first initializes the RSSL transport and
connects the client. After that, it sends a login request, a source directory
request, a dictionary request, and one or more item request messages (snapshot
or streaming) to a provider and appropriately processes the responses. The
resulting responses from the provider are displayed onto the console.

If the dictionary is found in the directory of execution, then it is loaded
directly from the file. However, the default configuration for this application
is to request the dictionary from the provider. Hence, no link to the dictionary
is made in the execution directory by the build script. The user can change this
behavior by manually creating a link to the dictionary in the execution directory.

This application supports consuming Level I Market Price, Level II Market By
Order, Level II Market By Price, and Yield Curve information as normal or private streams.

The private stream redirect scenario is supported. In this scenario, the provider
sends a status message indicating that the requested item be opened on a private
stream rather than a normal stream.

This application can optionally perform on-stream and off-stream posting for Level I
Market Price content. The item name used for an off-stream post is "OFFPOST". For
simplicity, the off-stream post item name is not configurable, but users can modify
the code if desired.

If multiple item requests are specified on the command line for the same domain and
the provider supports batch requests, this application will send the item requests as a
single Batch request.

If supported by the provider and the application requests view use, a dynamic
view will be requested with all Level I Market Price requests. For simplicity,
this view is not configurable but users can modify the code to change the
requested view.

This application supports a symbol list request. The symbol list name is optional.
If the user does not provide a symbol list name, the name is taken from the source
directory response.

This application is intended as a basic usage example. Some of the design choices
were made to favor simplicity and readability over performance. This application 
is not intended to be used for measuring performance.
 
-----------------
Application Name:
-----------------

Consumer

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Command line usage:
-------------------  

Consumer
(runs with a default set of parameters (-h localhost -p 14002 -s DIRECT_FEED -mp TRI))

or

./Consumer [-uname <LoginUsername>] [-at <AuthenticationToken>] [-ax <AuthenticationExtended] [-h <SrvrHostname>] [-p <SrvrPortNo>] [-c <ConnType>] [-s <ServiceName>] [-aid <ApplicationId>] [-view] [-post] [-offpost] [-snapshot] [-sl [<SymbolList Name>]] [-mp|-mpps <MarketPrice ItemName>] [-mbo|-mbops <MarketByOrder ItemName>] [-mbp|-mbpps <MarketByPrice ItemName>] [-runtime <seconds>] [-td]


The user can specify multiple -mp/-mpps, -mbo/mbops, -mbp/mbpps, and -yc/ycps instances, where
each occurrence is associated with a single item. For example, specifying "-mp TRI
-mp GOOG -mbo AAPL" will issue requests for two MarketPrice items and one MarketByOrder
item. Specifying -mpps, -mbops, -mbpps, or -ycps requests the MarketPrice, MarketByOrder,
MarketByPrice, or YieldCurve items on a private stream.

Specifying the -c option configures the consumer for different connection types.
Possible connection types for the consumer are 0 (TCP/IP), 1 (HTTPS), and 2 (HTTP).
The default connection type is TCP/IP.

Specifying the -view option results in a dynamic view request for any MarketPrice
items.  If the provider does not indicate support for dynamic views, this functionality
will not be performed.

Specifying the -post option enables the consumer application to attempt on-stream
posting to the provider. When a provider supports posting, the consumer will post to
the first successfully established MarketPrice stream. If no MarketPrice items are 
requested, on-stream posting will be disabled. While on-stream posting, the application
will alternate between a Post message that contains another message and a Post message
that contains only data payload. 

Specifying the -offpost option enables the consumer application to attempt off-stream
posting to the provider. When a provider supports posting, the consumer will post a
market price item name of "OFFPOST" to the login stream. The application only sends
a Post message that contains another message for off-stream posting 

Specifying the -snapshot option results in a non-streaming or "snapshot" request for
all items.

Specifying the -sl option results in a symbol list request. The symbol list name is
optional. If the user does not provide a symbol list name, the name is taken from
the source directory response.

Specifying the -runtime option controls the time the application will run
before exiting, in seconds.

Specifying the -at option configures the token used for UserAuthn Authentication.  This should be used 
in place of a userName.  This token is retrieved from a token generator, and passed to LSEG Real-Time
Distribution, which will verify the token against a token validator.  For more information about
the UserAuthn Authentication feature, please see the Developers guide and the UserAuthn Authentication guide.

Specifying the -ax option configures the authentication extended information used for UserAuthn Authentication.

Specifying the -aid option configures the Application Id.

-td prints out additional transport details from rsslReadEx() and rsslWriteEx() function calls 

- Consumer -? displays command line options.  

- Pressing the CTRL+C buttons terminates the program.  


-----------------
Compiling Source:
-----------------

The included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the Transport API supported 
Linux platforms using the supported compilers.

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

rsslConsumer.c - The main file for the rsslConsumer application.

rsslDictionaryHandler.c - The dictionary handler for the rsslConsumer application.

rsslDirectoryEncodeDecode.c - Contains all source directory encode and decode functions.

rsslDirectoryHandler.c - The source directory handler for the rsslConsumer application.

rsslLoginConsumer.c - The login consumer for the rsslConsumer application.

rsslLoginEncodeDecode.c - Contains all login encode and decode functions.

rsslMarketPriceHandler.c - The market price domain handler for the rsslConsumer application. 

rsslMarketByOrderHandler.c - The market by order domain handler for the rsslConsumer application.

rsslMarketByPriceHandler.c - The market by price domain handler for the rsslConsumer application.

rsslYieldCurveHandler.c - The yield curve domain handler for the rsslConsumer application.

rsslSymbolListHandler.c - The symbol list domain handler for the rsslConsumer application. 

rsslPostHandler.c - This handles all post message encoding and sending when the -post option is specified.  

rsslMarketPriceItems.c - This encodes market price content inside of the Post message when posting is enabled.

rsslSendMessage.c - Utility functions for sending messages.
