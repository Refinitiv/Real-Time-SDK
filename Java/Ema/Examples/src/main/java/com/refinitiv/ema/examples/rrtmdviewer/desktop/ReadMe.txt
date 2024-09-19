Summary
=======

The LSEG Real-Time Market Data Viewer (RRTViewer) is an OMM Consumer
GUI application that demonstrates features of the EMA library to access, 
parse and display Real-Time data from data sources such as Advanced
Distribution Server, Real-Time - Optimized (RRTO) or an 
OMM Provider. The viewer supports socket and websocket connections with 
and without encryption.

NOTE that an 'endpoint' is defined by a host and a port.
NOTE that all RRTO endpoints are encryption endpoints. 

The viewer may be started in a couple of different modes: 
+ "Specify Endpoint": User may specify an endpoint defined by a host and a port
     and a connection type.This mode does not require authentication and 
     connects to the specified endpoint.
+ "Discover Endpoint": User must specify RRTO authentication credentials and 
     choose to connect to a RRTO endpoint from a list of discovered endpoints.
     The Viewer does Service Discovery based on the type of connection requested
     by user and provides RRTO endpoints in a drop down list to choose from. 

Application uses the EMA's programmatic configuration in both cases to setup 
a connection to the appropriate endpoint. 

Once connected, user may request data on MarketPrice, MarketByOrder and/or 
MarketByPrice domains. The Viewer demonstrates EMA best practices for item
handling and demonstrates features like view, snapshot/streaming and batch 
requests on MarketPrice domain. 


How to Run Application
======================

From Java folder, run appropriate command:
  On Unix:
    export LANG='en_US.ISO-8859-1'
    ./gradlew runRRTViewerDesktop
  On Windows:
    gradlew.bat runRRTViewerDesktop 


Detailed Description
====================

RRTViewer implements the following high-level steps:
+ Choose "Discover Endpoint" or "Specify Endpoint" configuration as appropriate;
  Configure "Specify Endpoint":
    - Fill in required input text fields for specified endpoint configuration:
      host, port, connection type, connection sub-protocols;
    - Optionally fill in Server 2 endpoint information for connection recovery:
      API will failover to server next in list in a round-robin fashion; 
    - Optionally specify local dictionary OR if not specified, dictionary is 
      downloaded dictionary from Provider;
      NOTE: Local dictionary must be specified for WebSocket connection via ADS.
    - Optionally configure application options: application ID, position, username.
  Configure "Discover Endpoint" 
    - Fill in required input fields for specified endpoint configuration: username, 
      password, client ID, connection type;  Click on "Retrieve Service Endpoints"
    - Choose an endpoint from list
    - Optionally choose more than one endpoint to setup connection recovery:
      API will failover to server next in list in a round-robin fashion;
      NOTE: The order in which endpoints are selected determines failover order.
    - Optionally specify local dictionary OR if not specified, dictionary is 
      downloaded dictionary from Provider;
      NOTE: Local dictionary must be specified for WebSocket connection to RRTO
    - Optionally customize service URLS: specify custom URLS for authentication 
      and receiving service endpoint info;
    - Optionally customize encrypted configuration;
    - Optionally specify proxy server and proxy authentication data for connection;

+ Request Data on Supported Domain
 + Send MarketPrice item requests; Capabilities are as follows:
    - Sending the single request via specifying single RIC;
    - Sending the batch request via specifying multiple RICs;
    - Sending the view request via specifying FIDs;
      NOTE: view request is considered best effort since it may not be supported 
      by server side OR may be aggregated over the wire with other view/non-view
      requests. If aggregated, users will see all available or more fields 
      than requested view. 
    - Choosing to request snapshot (streaming mode is default);
    - Choosing to view debug logs;
    - If data is stale (interrupted), values will be displayed in "red" color.
 + Send MarketByPrice or MarketByOrder item requests; Capabilities are as follows:
    - Sending the single request via specifying single RIC;
    - Choosing to request snapshot (streaming mode is default);
    - Data is displayed summary data in text fields followed by with BID & ASK tables;
    - Capability to log all received item states;
    - If data is stale (interrupted), values will be displayed in "red" color.

+ Unregister existing streaming item requests or refresh snapshot data;

+ Intercept application logs and XML/JSON tracing on all stages and stream them 
  onto particular area;
    - NOTE: When debug is checked, more memory is used to display logs.

+ Streaming the client and server errors during configuration of discovered or specified 
  endpoint.

NOTE: Recommendation is to use JDK/OpenJDK 11 or higher for building and running 
this application. Prior JDK versions cause issues with displaying Item View data;

NOTE: If app is started without GRADLE using JDK/OpenJDK 11 or higher, the separate
JavaFX library must be provided;

NOTE: If needed, these and other details may be modified to fit your local
environment. For details on standard configuration, refer to the EMA library
README.md file and EMA Configuration Guide.
