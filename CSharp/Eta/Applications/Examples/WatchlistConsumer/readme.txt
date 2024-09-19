WatchlistConsumer Application Description

--------
Summary:
--------

The purpose of this application is to demonstrate consuming data from an 
ADS device, OMM Provider application or Real-Time - Optimized 
using ValueAdd components.  It is a single-threaded client application.

This application leverages the consumer watchlist feature provided by the
RsslReactor to provide recovery and aggregation of items. Using the 
consumer watchlist feature also enables this application to consume
from either an OMM Provider or ADS over a socket-based connection, or from
an ADS over a multicast network.  It requests items within the 
channelOpenCallback function, which are automatically requested by the
RsslReactor on its behalf when the channel connects to the providing component.

If the dictionary is found in the directory of execution, then it is loaded
directly from the file.  However, the default configuration for this application
is to request the dictionary from the provider.  Hence, no link to the dictionary
is made in the execution directory by the build script.  The user can change this
behavior by manually creating a link to the dictionary in the execution directory.

This application supports consuming Level I Market Price, Level II Market 
By Order, Level II Market By Price, and Yield Curve. This application 
optionally performs on-stream posting for Level I Market Price content.  

This application supports a symbol list request. When requesting a symbol
list, it also requests that the RsslReactor automatically open streams
using the item names that appear in the list.

This application can optionally perform on-stream and off-stream posting for Level I
Market Price content. The item name used for an off-stream post is "OFFPOST". For
simplicity, the off-stream post item name is not configurable, but users can modify
the code if desired.

This application can open tunnel streams to a provider that supports them (via the -tunnel option),
such as the included provider example.

This application is intended as a basic usage example.  Some of the design 
choices were made to favor simplicity and readability over performance.  
This application is not intended to be used for measuring performance.

 
-----------------
Application Name:
-----------------

WatchlistConsumer

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.Def files located in the etc directory
can be located in the directory of execution.  If the dictionary files
cannot be found, they are requested from the provider.

Both the shared version of libcurl and the openssl libraries are needed to run this example for 
connecting and consuming data from Real-Time - Optimized. 

-------------------
Command line usage:
-------------------  

WatchlistConsumer

(runs with a default set of parameters (-h localhost -p 14002 -s DIRECT_FEED -mp TRI))

or

WatchlistConsumer -c encrypted -ec socket -s <service name> -u <machine ID> -passwd <password> -clientId <client ID> -sessionMgnt

The above example command line is used to create an encrypted connection with the socket encrypted protocol type
using the default "us-east-1" region and enable session management to login and subscribe data from Real-Time - Optimized. 

or

WatchlistConsumer or WatchlistConsumer [ -c <Connection Type> ] [ -ec <encrypted protocol> ]
  [ -if <Interface Name> ] [ -u <Login UserName> ] [ -passwd <Login password> ] [ -clientId <Client ID> ]
  [ -sessionMgnt ] [ -takeExclusiveSignOnControl <true/false> ] [ -l <Location Name> ] [ -query ] 
  [ -s <ServiceName>] [ -mp <MarketPrice ItemName> ] [ -mbo <MarketByOrder ItemName> ]
  [ -mbp <MarketByPrice ItemName> ] [ -yc <YieldCurve ItemName> ] [ -sl <SymbolList ItemName> ]
  [ -sld <SymbolList ItemName> ] [ -view ] [ -runTime <TimeToRun> ] [-at <AuthenticationToken>] 
  [-ax <AuthenticationExtended>] [-aid <ApplicationId>] [ -restEnableLog ] [ -restLogFileName <FileName>]

  Connection options for socket and encrypted connection types:
    [ -h <Server Hostname> ] [ -p <Port> ]

  Connection options for the reliable multicast connection type; all must be specified:
    [ -sa <Send Address> ] [ -ra <Receive Address> ] [ -sp <Send Port> ] [ -rp <Receive Port> ]
	[ -up <Unicast Port> ]

  Options for publishing Host Stat Message options on reliable multicast connections; -hsmAddr and -hsmPort must be specified to enable:
    [ -hsmAddr <Address> ] [ -hsmPort <Port> ] [ -hsmInterface <Interface Name> ] [ -hsmInterval <seconds> ]


The -c option specifies the connection type. Valid arguments are socket, and encrypted.  
When using a socket or encrypted connection, use the -h and -p options to configure the 
address and port for connecting. 

The -ec option specifies the encrypted transport protocol. Valid arguments are socket.  

The -clientId option specifies an unique ID for authenticating with the RDP token service (mandatory).
You can generate and manage client Ids by using the Eikon App Key Generator. 
This is found by visiting my.Refinitiv.Com, launching Eikon (need valid login), and 
searching for "App Key Generator". The App Key is the Client ID. 

Specifying the -sessionMgnt option enables session management in the Reactor to perform
authentication token management and will query RDP service discovery(if -query is not specified)
on behalf of users if configured to do so.

When -sessionMgnt is specified, if a host and port is specified, the Consumer will not use service discovery.
In addition, the following options are used to retrieve a token from the authentication token management system.

For a client credentials grant(Service Account) V2 login: -clientId <service account> -clientSecret <client secret>. 

The -l option specifies a location name that is used to get an endpoint from the
RDP service discovery information. Defaults to "us-east-1".

Specifying the -query option to query the RDP service discovery to get an endpoint
according to the specified connection type and location name.

The user can specify multiple instances of -mp, -mbo, -mbp, -yc, -sl and -sld,
where each occurrence is associated with a single item. For example, 
specifying "-mp TRI -mp GOOG -mbo AAPL" will issue requests for two 
MarketPrice items and one MarketByOrder item.  The -sld option is like the -sl 
option, but will request that data streams be opened for items present in the 
symbol list response.

Specifying the -view option results in a dynamic view request for any MarketPrice
items.  If the provider does not indicate support for dynamic views, the Reactor will forward
the item request without the view.

Specifying the -post option enables the consumer application to attempt on-stream
posting to the provider. When a provider supports posting, the consumer will post to
the first successfully established MarketPrice stream. If no MarketPrice items are 
requested, on-stream posting will be disabled. While on-stream posting, the application
will alternate between a Post message that contains another message and a Post message
that contains only data payload.

Specifying the -at option configures the token used for UserAuthn Authentication. This should be used 
in place of a userName.  This token is retrieved from a token generator, and passed to
LSEG Real-Time Distribution, which will verify the token against a token validator.
For more information about the UserAuthn Authentication feature, please see the Developers guide and
the UserAuthn Authentication guide.

Specifying the -ax option configures the authentication extended information used for UserAuthn Authentication.

Specifying the -aid option configures the Application Id.

Specifying the -runTime option controls the time the application will run
before exiting, in seconds.

- WatchlistConsumer -? displays command line options.  