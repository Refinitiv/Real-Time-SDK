Summary
=======

example113__MarketPrice__SessionManagement is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Elektron Real Time in Cloud (ERT in cloud).

example113__MarketPrice__SessionManagement illustrates how to use the EMA's configuration file
to enable session management and specify a location to get an endpoint for establishing
a connection with ERT in cloud and consuming data. This application requires a username
and a password for authorization with the token service in order to an access token for
querying endpoints from the EDP service discovery and sending login requests to ERT in
cloud. EMA automatically refreshes the token to keep session alive with the connecting
provider


Detailed Description
====================

example113__MarketPrice__SessionManagement implements the following high-level steps:
+ Passes user credential through command line arguments
including:
The ChannelType must be set to ChannelType::RSSL_ENCRYPTED in EMA configuration file.
-username user name to perform authorization with the token service.
-password password to perform authorization with the token service.
-clientId client ID to perform authorization with the token service.
-keyfile keystore file for encryption.
-keypasswd keystore password for encryption. 
-ph Proxy host name.
-pp Proxy port number.
-plogin User name on proxy server.
-ppasswd Password on proxy server.
-pdomain Proxy Domain.
-krbfile KRB File location and name. Needed for Negotiate/Kerberos and Kerberos authentications.

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_3"
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and send login request to the endpoint of the specified location.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the ELEKTRON_DD service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA COnfiguration Guide.
