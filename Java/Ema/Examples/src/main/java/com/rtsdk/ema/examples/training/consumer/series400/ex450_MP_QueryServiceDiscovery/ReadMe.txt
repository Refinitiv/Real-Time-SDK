Summary
=======

ex450_MP_QueryServiceDiscovery is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Refinitiv Real-Time - Optimized. 

ex450_MP_QueryServiceDiscovery illustrates how to query endpoints from
Refinitiv Data Platform service discovery using the ServiceEndpointDiscovery class and use the 
location from the command line to select an endpoint. The EMA's programmatic
configuration is used to to enable session management with the retrieved endpoint
for establishing a connection with the Refinitiv Real-Time Optimized service and consuming data. This 
application requires a username (Machine ID), password, and client ID for 
authorization with the token service in order to an access token for querying 
endpoints from the Refinitiv Data Platform service discovery and sending login requests to the
service. EMA automatically refreshes the token to keep session alive with the service.


Detailed Description
====================

ex450_MP_QueryServiceDiscovery implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-username machine ID to perform authorization with the token service (mandatory).
-password password to perform authorization with the token service (mandatory). 
-clientId client ID for application making the request to Refinitiv Data Platform token service, 
also known as AppKey generated using an AppGenerator (mandatory). You can 
generate and manage client IDs at the following URL:
https://emea1.apps.cp.thomsonreuters.com/apps/AppkeyGenerator (you need an Eikon login
to access this page).
-takeExclusiveSignOnControl <true/false> the exclusive sign on control to force sign-out for the same credentials(optional).
-location location to get an endpoint from Refinitiv Data Platform service discovery. Now, it is either
 "us-east" by default or "eu-west" (optional).
-keyfile keystore file for creating an encrypted connection.
-keypasswd keystore password for creating an encrypted connection.
Note: please refer to README.md of RTSDK Java for generating a keystore file.

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
./gradlew runConsumer450 -PcommandLineArgs='-username <username> -password <password> -clientId <client id> -takeExclusiveSignOnControl <true/false> -keyfile <full path to the file> -keypasswd <keyfile password>'

On Windows:
gradlew.bat runConsumer450 -PcommandLineArgs='-username <username> -password <password> -clientId <client id> -takeExclusiveSignOnControl <true/false> -keyfile <full path to the file> -keypasswd <keyfile password>'

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates a Map (configMap object) and populates it with configuration info
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_1"
  - The Consumer_1 uses the Channel_1 channel name for using the RSSL_ENCRYPTED channel type.
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
