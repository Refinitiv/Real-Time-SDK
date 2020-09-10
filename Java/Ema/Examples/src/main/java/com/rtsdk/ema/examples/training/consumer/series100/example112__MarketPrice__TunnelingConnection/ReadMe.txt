Summary
=======

example112__MarketPrice__TunnelingConnection is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced
Distribution Server.

example112__MarketPrice__TunnelingConnection illustrates the abilitily of the user
to programmatically pass all http/https related configuration on OmmConsumerConfig
instance when configuring a tunneling connection. When running this application,
the user will need specify valid tunneling configurations through command line arguments.

Detailed Description
====================

example112__MarketPrice__TunnelingConnection implements the following high-level steps:
+ Passes tunneling related configuration through command line arguments
including:
if the application will attempt to make an http or encrypted
       connection, ChannelType must be set to ChannelType::RSSL_HTTP
	   or ChannelType::RSSL_ENCRYPTED in EMA configuration file
-ph Proxy host name
-pp Proxy port number
-plogin User name on proxy server
-ppasswd Password on proxy server
-pdomain Proxy Domain
-krbfile Proxy KRB file
-keyfile keystore file for encryption
-keypasswd keystore password for encryption

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the tunneling configurations
  - Sets the consumer name to "Consumer_3"
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Apache Dependent Libraries
==========================
All tunneling connections depends on some of apache libraries which are 
commons-codec-version.jar,
httpclient-version.jar,
httpcore-version.jar,
httpmime-version.jar

You can find them shipped under the product package at Ema\Libs\apache folder.

Note: If needed, you can modify these and other details to fit your local
      environment using the EmaConfig.xml file. For details on the standard 
      configuration, refer to the EMA library ReadMe.txt file or the EMA 
      Configuration Guide.
