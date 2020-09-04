Summary
=======

The 291__Custom__DownCast application provides a custom down casting
example of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Custom data from Refinitiv Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

The 291__Custom__DownCast showcases usage of the so called downcast decoding
method. In this method, application extracts a reference to a Data class object
and casts it down to the appropriate data class type based on its data type.



Detailed Description
====================s

The 291__Custom__DownCast implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - each of the method provided in this example use the ease of use
	  data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
  - sets operationModel to UserDispatchEnum
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - Custom Domain (133) IBM.XYZ item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on main application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.

Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
