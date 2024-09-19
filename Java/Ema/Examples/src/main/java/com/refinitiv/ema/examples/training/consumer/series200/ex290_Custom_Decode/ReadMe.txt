Summary
=======

The ex290_Custom_Decode application is provided as an example of OMM Consumer
application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Custom data from Data Feed Direct (LDFD), directly
from an OMM Provider application, or from an Advanced Distribution Server.

ex290_Custom_Decode showcases decoding of items on custom OMM domain.
Messages of a custom OMM domain may contain any container at any level of encoding.

 
Detailed Description
====================

ex290_Custom_Decode implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
  - each of the methods provided in this example use the ease of use
	data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - Custom Domain (133) IBM.XYZ item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.

Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
