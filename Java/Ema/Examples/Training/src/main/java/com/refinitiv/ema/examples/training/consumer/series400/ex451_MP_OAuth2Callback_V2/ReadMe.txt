Summary
=======

ex451_MP_OAuth2Callback_V2 is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Real-Time - Optimized. 

ex451_MP_OAuth2Callback_V2 illustrates how to query endpoints from
Delivery Platform service discovery using the ServiceEndpointDiscovery class and use the 
location from the command line to select an endpoint. The EMA's programmatic
configuration is used to to enable session management with the retrieved endpoint
for establishing a connection with the Real-Time - Optimized service and consuming data. This 
application requires a service account(clientId) and associated clientSecret for 
authorization with the token service in order to an access token for querying 
endpoints from the Delivery Platform service discovery and sending login requests to the
service. 


Detailed Description
====================

ex451_MP_OAuth2Callback_V2 implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-clientId service account used for this login
-clientSecret associated secret for the service account
-websocket Use the WebSocket transport protocol (optional).
-keyfile optional keystore file for creating an encrypted connection(optional).
-keypasswd (mandatory if keyfile is selected) keystore password for creating an encrypted connection.
-host hostname 
-port port
Note: please refer to README.md of RTSDK Java for generating a keystore file.
-tokenV2URL URL to perform authentication to get access token (optional).

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
./gradlew runConsumer451 -PcommandLineArgs='-clientId <service account name> -clientSecret <service account secret> -keyfile <full path to the file> -keypasswd <keyfile password>'

On Windows:
gradlew.bat runConsumer451 -PcommandLineArgs='-clientId <service account name> -clientSecret <service account secret> -keyfile <full path to the file> -keypasswd <keyfile password>'

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates a CredentialStore object to store the credentials for a credential callback.  Please note that this is 
  an non-secure example credential store, and a proper production application should have a securely encrypted credential store.
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
      ReadMe.txt file and EMA COnfiguration Guide.
