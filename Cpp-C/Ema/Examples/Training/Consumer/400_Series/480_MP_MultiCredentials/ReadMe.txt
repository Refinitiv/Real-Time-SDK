Summary
=======

480_MP_MultiCredentials illustrates the ability of the user
to specify multiple credentials when creating an OmmConsumerConfig.

Detailed Description
====================

480_MP_MultiCredentials implements the following high-level steps:

+ The application loads the default EmaConfig.xml with the configuration Consumer_2

+ The application reads the command line, setting the credentials on the config using either 
  config.addOAuth2Credential or config.addLoginMsgCredential

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates OmmOAuth2ConsumerClient and OmmLoginCredentialConsumerClient objects for each credential
  - Each credential will call back if needed for credential updates.  For oAuth credentials, EMA will not store
    any sensitive information.
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the consumer name to "Consumer_2"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment using the EmaConfig.xml file. For details on the standard 
      configuration, refer to the EMA library ReadMe.txt file or the EMA 
      Configuration Guide.
