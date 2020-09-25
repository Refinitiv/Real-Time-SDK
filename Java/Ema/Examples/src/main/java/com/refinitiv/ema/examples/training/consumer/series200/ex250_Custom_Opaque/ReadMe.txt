Summary
=======

The ex250_Custom_Opaque application is provided as an example of OMM Consumer
application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Custom Domain data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced
Distribution Server.

ex250_Custom_Opaque showcases opening and processing of a custom domain
OMM item whose payload is specified as Opaque. While processing the received
messages, this application simply prints out the received opaque data to the
screen in a hexadecimal format.


Detailed Description
====================

ex250_Custom_Opaque implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( OmmOpaque )
    - the method decode( OmmOpaque ) just simply prints the received opaque data
	  to the screen using hexadecimal format
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - Custom Domain IBM.XYZ item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
