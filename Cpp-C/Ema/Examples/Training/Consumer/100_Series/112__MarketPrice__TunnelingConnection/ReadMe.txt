Summary
=======

112__MarketPrice__TunnelingConnection is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Refinitiv Data Feed Direct,
directly from an OMM Provider application, or from an Advanced
Distribution Server, on specifically a Linux based operating system.

112__MarketPrice__TunnelingConnection illustrates the ability of the user
to programmatically pass all http/https related configuration on OmmConsumerConfig
instance when configuring a tunneling connection. When running this application,
the user will need specify valid tunneling configurations through commandline arguments.


Detailed Description
====================

112__MarketPrice__TunnelingConnection implements the following high-level steps:
+ Passes tunneling related configuration through commandline arguments
including:
-ph Proxy host name
-pp Proxy port number
-spTLSv1.2 enable use of cryptopgrahic protocol TLSv1.2 used with linux encrypted connections
-libsslName name of the libssl.so shared library
-libcryptoName name of the libcrypto.so shared library

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the tunneling configurations
  - Sets the consumer name to "Consumer_4"
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA COnfiguration Guide.
