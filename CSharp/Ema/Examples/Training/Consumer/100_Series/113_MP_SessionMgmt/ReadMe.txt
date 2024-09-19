Summary
=======

113_MP_SessionManagement is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from LSEG Real-Time -- Optimized.

113_MP_SessionManagement illustrates how to use the EMA's configuration file
to enable session management and specify a location to get an endpoint for establishing
a connection with a LSEG Real-Time service and consume data. This application requires 
a user name (machine ID or end-user ID) and a password or a service account(used as clientId) 
and associated client secret for authorization with the token service in order to use 
the access token for querying endpoints from Refintiv Data Platform (RDP) service discovery 
and sending login requests to the service.


Detailed Description
====================

113_MP_SessionManagement implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-clientId client ID to perform authorization with the token service (mandatory).
 For V2 client credentials this is the service account.
-clientSecret clientSecret for authorization with the token service(mandatory for V2 client credentials)
-tokenURL URL to perform authentication to get access and refresh tokens (optional).
-serviceDiscoveryURL URL for RDP service discovery to get global endpoints (optional).

Optional RIC item name parameters.
-itemName Request item name (optional). The default item name is IBM.N.

Optional proxy parameters. The proxy configuration is only required if your organization requires
use of a proxy to get to the Internet.
-ph Proxy host name (optional).
-pp Proxy port number (optional).
-ppasswd Password on proxy server (optional).

Example command to run the example from the command line folder:
On Unix:
./Cons113 -clientId <client id> -clientSecret <service account password>

On Windows:
.\Cons113 -clientId <client id> -clientSecret <service account password>

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_4"
  - The Consumer_4 uses the Channel_4 channel name for using the RSSL_ENCRYPTED
	channel type and the RSSL_SOCKET encrypted protocol type
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
