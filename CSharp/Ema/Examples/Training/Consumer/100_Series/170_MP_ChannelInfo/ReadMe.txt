Summary
=======

170_MP_ChannelInfo is an example of an OMM Consumer application
that demonstrates basic usage of the EMA library to access and parse
channel information on the channel used to connect to the source of
data received by the application.

Detailed Description
====================

170_MP_ChannelInfo implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient:
   - Overrides desired methods.
+ Instantiates AppClient object that receives and processes item messages.
+ Instantiates an OmmConsumer object which initializes the connection 
   and logs into the specified server.
+ retrieves the channel information on the OmmConsumer object and illustrates how to
   print it
+ Illustrates how to retrieve and print channel information on refresh, update, and status message
   events.
+ Exits.

Note: If needed, you can modify these and other details to fit your local environment.
      For details on standard configuration, refer to the EMA library ReadMe.txt file
      or to the EMA Configuration Guide.
