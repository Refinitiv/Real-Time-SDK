Summary
=======

ex113_MP_SessionManagement is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from LSEG Real-Time -- Optimized.

ex113_MP_SessionManagement illustrates how to use the EMA's configuration file
to enable session management and specify a location to get an endpoint for establishing
a connection with a LSEG Real-Time service and consume data. This application requires 
a user name (machine ID or end-user ID) and a password or a service account(used as clientId) 
and associated client secret for authorization with the token service in order to use 
the access token for querying endpoints from Refintiv Data Platform (RDP) service discovery 
and sending login requests to the service. EMA automatically refreshes the token to keep 
session alive with the service for V1 connections. EMA does not need to do so for V2 connections.


Detailed Description
====================

ex113_MP_SessionManagement implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-username machine ID to perform authorization with the token service (mandatory for V1 password credentials).
-password password to perform authorization with the token service (mandatory for V1 password credentials).
-clientId client ID to perform authorization with the token service (mandatory).
 For V1 password credentials:
 You can generate and manage client Ids by using the Eikon App Key Generator.
 This is found by visiting myaccount.lseg.com, launching Eikon, and 
 searching for "App Key Generator". Eikon login is required to generate clientID.
 For V2 client credentials:
 This is the service account.
-clientSecret clientSecret for authorization with the token service(mandatory for V2 client credentials)
-takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials (optional).
 This is only used for V1 password credential logins. It is not used for V2 client credentials.
-websocket Use the WebSocket transport protocol (optional).
-tokenURL URL to perform authentication to get access and refresh tokens (optional).
-serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).

Optional RIC item name parameters.
-itemName Request item name (optional). The default item name is IBM.N.

Optional proxy parameters. The proxy configuration is only required if your organization requires
use of a proxy to get to the Internet.
-ph Proxy host name (optional).
-pp Proxy port number (optional).
-plogin User name on proxy server (optional).
-ppasswd Password on proxy server (optional).
-pdomain Proxy Domain (optional).
-krbfile KRB File location and name. Needed for Negotiate/Kerberos and Kerberos authentications (optional).

Example command to run the example from the command line from Java folder:
On Unix:
./gradlew runConsumer113 -PcommandLineArgs='-username <username> -password <password> -clientId <client id> -takeExclusiveSignOnControl <true/false> -keyfile <full path to the file> -keypasswd <keyfile password>'

On Windows:
gradlew.bat runConsumer113 -PcommandLineArgs='-username <username> -password <password> -clientId <client id> -takeExclusiveSignOnControl <true/false> -keyfile <full path to the file> -keypasswd <keyfile password>'

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_4"
  - The Consumer_4 uses the Channel_4 channel name for using the RSSL_ENCRYPTED
	channel type and the RSSL_SOCKET encrypted protocol type.
    or sets the consumer name to "Consumer_5" in case of the WebSocket transport protocol.
    The Consumer_5 uses the Channel_5 channel name for using the RSSL_ENCRYPTED
	channel type and the RSSL_WEBSOCKET encrypted protocol type.
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and send login request to the endpoint of the specified location.
+ Opens a streaming item interest
  - MarketPrice IBM.N (or optional itemName) item from the ELEKTRON_DD service
+ Processes data received from the API for 900 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
