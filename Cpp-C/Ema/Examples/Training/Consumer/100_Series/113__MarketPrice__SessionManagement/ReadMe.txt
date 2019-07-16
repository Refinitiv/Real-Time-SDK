Summary
=======

113__MarketPrice__SessionManagement is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Elektron Real Time in Cloud (ERT in cloud).

113__MarketPrice__SessionManagement illustrates how to use the EMA's configuration file
to enable session management and specify a location to get an endpoint for establishing
a connection with the cloud service and consume data. This application requires a user name
(machine ID) and a password for authorization with the token service in order to use
the access token for querying endpoints from the EDP service discovery and sending login 
requests to the cloud service. EMA automatically refreshes the token to keep session alive 
with the cloud service. 


Detailed Description
====================

113__MarketPrice__SessionManagement implements the following high-level steps:
+ Passes user credential through command line arguments
including:
-username machine ID to perform authorization with the token service (mandatory).
-password password to perform authorization with the token service (mandatory).
-clientId client ID to perform authorization with the token service (mandatory).
 You can generate and manage client Ids at the following URL:
 https://emea1.apps.cp.thomsonreuters.com/apps/AppkeyGenerator (you need an Eikon login
 to access this page).

Optional proxy parameters. The proxy configuration is only required if your organization requires
use of a proxy to get to the Internet. 
-ph Proxy host name (optional).
-pp Proxy port number (optional).
-plogin User name on proxy server (optional).
-ppasswd Password on proxy server (optional).
-pdomain Proxy Domain (optional).

Example command line: 
Cons113 -username <machine ID> -password <machine ID password>

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user credential
  - Sets the consumer name to "Consumer_3"
  - The Consumer_3 uses the Channel_3 channel name for using the RSSL_ENCRYPTED
	channel type and the RSSL_SOCKET encrypted protocol type for both
	Linux and Windows platforms. Both the shared version of libcurl and the openssl libraries
	are needed to run this example.
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and send login request to the endpoint of the specified location.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the ELEKTRON_DD service
+ Processes data received from the API for 900 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
