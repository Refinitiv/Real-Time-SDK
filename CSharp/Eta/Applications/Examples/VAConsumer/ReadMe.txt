VAConsumer Application Description

--------
Summary:
--------

The purpose of this application is to demonstrate consuming data from an OMM
Provider, ADS device, OMM Provider application or LSEG Real-Time -
Optimized using ValueAdd components.  It is a single-threaded client
application.

The consumer application implements callbacks that process information received
by the provider.  It creates the Reactor, creates the desired connections, then
dispatches from the Reactor for events and messages.  Once it has received the
event indicating that the channel is ready, it will make the desired item
requests (snapshot or streaming) to a provider and appropriately processes the
responses. The resulting decoded responses from the provider are displayed on
the console.

If the dictionary is found in the directory of execution, then it is loaded
directly from the file.  However, the default configuration for this application
is to request the dictionary from the provider.  Hence, no link to the
dictionary is made in the execution directory by the build script.  The user can
change this behavior by manually creating a link to the dictionary in the
execution directory or by copying it here.

This application supports consuming Level I Market Price, Level II Market By
Order, Level II Market By Price and Yield Curve. This application optionally
performs on-stream posting for Level I Market Price content.

This application can optionally perform on-stream and off-stream posting for
Level I Market Price content. The item name used for an off-stream post is
"OFFPOST". For simplicity, the off-stream post item name is not configurable,
but users can modify the code if desired.

If multiple item requests are specified on the command line for the same domain
and the provider supports batch requests, this application will send the item
requests as a single Batch request.

If supported by the provider and the application requests view use, a dynamic
view will be requested with all Level I Market Price requests.  For simplicity,
this view is not configurable but users can modify the code to change the
requested view.

This application supports a symbol list request. The symbol list name is
optional.  If the user does not provide a symbol list name, the name is taken
from the source directory response.

This application is intended as a basic usage example.  Some of the design
choices were made to favor simplicity and readability over performance.  This
application is not intended to be used for measuring performance.

-----------------
Application Name:
-----------------

VAConsumer

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory can
be located in the directory of execution.  If the dictionary files cannot be
found, they are requested from the provider.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of paramters (-c localhost:14002 DIRECT_FEED mp:TRI,mp:.DJI)

or

dotnet run -- [arguments]

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
file and build the VAConsumer project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

VAConsumer.cs - The main file for the VAConsumer application. Provides the default message callback.

MarketPriceHandler.cs - Provides functions for encoding market price requests and decoding responses.

MarketByOrderHandler.cs - Provides functions for encoding market by order requests and decoding responses.

MarketByPriceHandler.cs - Provides functions for encoding market by price requests and decoding responses.

YieldCurveHandler.cs - Provides functionality for encoding yield curve requests and decoding responses.

SymbolListHandler.cs - Provides functions for encoding symbol list requests and decoding responses.

PostHandler.cs - This handles all post message encoding and sending when the -post option is specified.
