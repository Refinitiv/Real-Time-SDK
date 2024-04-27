OVERVIEW
========

The 430_MP_Authentication application is a simple application written to demonstrate
usage of the UserAuthn Authentication Feature.

LIMITATIONS
===========

To demonstrate the UserAuthn Authentication Feature the 430_MP_Authentication application
must be connected to an ADH that supports the feature.

USAGE
=====

The 430_MP_Authentication utilizes command line options as well as the xml configuration.

Options:

-at <authentication token> 		Required authentication token.
-ax <authentication extended> 	Optional authentication extended information
-aid <application Id>			Optional application Id.  If not present, this defaults to "256".

To learn the command line options, please specify "-?" command line option so that the
usage help can be printed out.


Detailed Description
====================

430_MP_Authentication implements the following high level steps:

+ Implements OmmProviderClient class in AppLoginClient
  - overrides desired methods for the login domain
  - When the Login response message is received, the appLoginClient stores the 
    handle and Authentication TTReissue.  This handle is used to reissue the 
	token once the Authentication TTReissue time is reached.
  - provides own methods as needed  
+ Instantiates AppLoginClient object that receives and processes login item messages
+ Instantiates and modifies OmmProviderConfig object
  - sets request message for Login domain to override default
  - This request message contains the authentication token, the application 
    Id, and the optional authentication extended information
+ Instantiates OmmProvider object which initializes connection and logins into
  the specified server
  - This also registers the interest in the Login stream using the AppLoginClient
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice IBM.N item on the NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ When the current time(in Unix Epoch) is equal to the Authentication TTReissue
  - Send a Login reissue on the login handle containing the authentication token 
    and authentication extended information
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
