Summary
=======

452_MP_OAuth2Callback_V2JWT is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Real-Time - Optimized. 

ex452_MP_OAuth2Callback_V2JWT illustrates how to query endpoints from
Delivery Platform service discovery using the ServiceEndpointDiscovery class and use the 
location from the command line to select an endpoint. The EMA's programmatic
configuration is used to to enable session management with the retrieved endpoint
for establishing a connection with the Real-Time - Optimized service and consuming data. This 
application requires a service account(clientId) and associated clientSecret for 
authorization with the token service in order to get an access token for sending login requests to the
service. 


Detailed Description
====================

ex452_MP_OAuth2Callback_V2JWT implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-clientId service account used for this login(mandatory)
-jwkFile File location of the JWK encoded public key(mandatory).
-websocket Use the WebSocket transport protocol (optional).
-host hostname 
-port port
-tokenV2URL URL to perform authentication to get access token (optional).
-itemName item request name(optional)
-audience audience claim for the JWT(optional)

Example command line: 
Cons451 -clientId <service account> -jwkFile <file location of the JWK> 

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates an OAuthClient object to receive credential request callbacks
  - NOTE: this application is storing the clientId and credentials as a global object for example purposes. 
    A secure credential storage should be used.
+ Instantiates a Map (configMap object) and populates it with configuration info
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_1"
  - The Consumer_1 uses the Channel_1 channel name for using the RSSL_ENCRYPTED channel type
    and either the RSSL_SOCKET or RSSL_WEBSOCKET encrypted protocol type.   
  - sets OmmConsumer configuration with data from the programmatic configuration
+ Instantiates an OmmConsumer object which initializes the connection 
  and send login request to the endpoint of the specified location.
+ Opens a streaming item interest
  - MarketPrice IBM.N (or optional itemName) item from the ELEKTRON_DD service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
