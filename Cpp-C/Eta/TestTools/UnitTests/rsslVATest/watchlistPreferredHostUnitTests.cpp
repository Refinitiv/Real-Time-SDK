/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "watchlistTestFramework.h"
#include "gtest/gtest.h"


#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;


class PreferredHostTestParameters
{
public:
	RsslBool multiLoginMsg;

	PreferredHostTestParameters(RsslBool multiLogin)
	{

		multiLoginMsg = multiLogin;
	}

	friend ostream& operator<<(ostream& out, const PreferredHostTestParameters& params)
	{
		out << "Multiple Login mode = " << (params.multiLoginMsg ? "T" : "F") << endl;
		return out;
	}
};

// For all tests, 4 listening sockets are opened on localhost: 14011(server index 0), 14012(server index 1), 14013(server index 2), and 14014(server index 3).

/* This test does the following:
 * Initial setup: 
 *  3 WSB groups, all set for SERVICE_BASED
 *  WSB index 0 and 1 point to two invalid ports each
 *  WSB index 2 points to server 0 and server 1
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2
 * 
 * Test behaviors:
 *  This test will immediately connect on WSB group 3 to server 0 and server 1(using wtfSetupWarmStandbyConnection), and after connection, server 0 will be set to STANDBY on service1, server 0 will be set to ACTIVE
 *  The test will request an item, which will go to server 0 and server 1, and server 0 will repond with a partial refresh(no payload), and server 1 will respond with a full refresh(including payload)
 *  Verify that the consumer receives the full refresh with payload.
 */
void preferredHost_WSBService_ConnectionUp(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  1 WSB group, set for SERVICE_BASED, infinite reconnections
 *  WSB index 1 points to server 0 and server 1
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 0 and fallbackWithinWSBGroup set to TRUE
 *
 * Test behaviors:
 *  This test will immediately connect on WSB group 1 to server 0 and server 1(using wtfSetupWarmStandbyConnection), and after connection, server 0 will be set to STANDBY on service1, server 0 will be set to ACTIVE
 *  Call fallbackToPreferredHost, see that the consumer immediately gets PREFERRED_HOST_COMPLETE
 *  Disconnect server 1(the WSB active), see that server 0 gets an directory message stating that service1 is now active on that server
 *  Reconnect server 1, see that it connects and that the watchlist assigns server1 as standby
 *  Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Call fallbackToPreferredHost, see that the consumer swaps the active and standby, so for service1, sever 1 is now standby, server 2 is now active   
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 */
void preferredHost_WSBService_FallbackWithinWSBGroup(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED ith reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).  
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  Restart server 2 and server 3 
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Call fallbackToPreferredHost
 *  While the fallback is happening, have both server 0 and server 1 send an update, see that the consumer only gets one
 *  Perform the service based WSB connection, with server 3 set to active for service1, and server 2 set to standby for service1
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  Make sure that the watchlist sends requests to both server 2 and server 3
 */
void preferredHost_WSBService_FallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED ith reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the preferred host timer set to 3 seconds
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  Restart server 2 and server 3
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Wait for the preferred host operation to start, accepting a connection on server 2
 *  While the fallback is happening, have both server 0 and server 1 send an update, see that the consumer only gets one
 *  Perform the service based WSB connection, with server 3 set to active for service1, and server 2 set to standby for service1
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  Make sure that the watchlist sends requests to both server 2 and server 3
 */
void preferredHost_WSBService_FallbackTimer(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  A channel list of 3 invalid servers
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the preferred host timer set to 2 seconds
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *	Request an item, and server 1 will repond with a full refresh(including payload), and server 0 will respond with a partial refresh(no payload)
 *  Call fallbackToPreferredHost, since there is server 2 is still shutdown, there will be a PREFERRED_HOST_COMPLETE event without any changes
 */
void preferredHost_WSBService_InvalidWSBGroupFallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list of index 0 pointing to server 2, and the other channels pointing to invalid ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *	Request an item, and server 2 will repond with a full refresh(including payload), and server 0 will respond with a partial refresh(no payload)
 *  Shutdown server 0 and server 1
 *  Disconnect channel server 0, see that nothing happens(since it is the standby)
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Watch that the reconnection is handled until it gets to channel list 0(server 2)
 *  Finish the connection for server 2, see that the item is requested.
 *  Call fallbackToPreferredHost 
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  See that both servers get the request
 */
void preferredHost_WSBService_FallbackFunctionCallFromChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list of index 0 pointing to server 2, and the other channels pointing to invalid ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *	Request an item, and server 1 will repond with a full refresh(including payload), and server 0 will respond with a partial refresh(no payload)
 *  Disconnect channel server 0, see that nothing happens(since it is the standby)
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Watch that the reconnection is handled until it gets to channel list 0(server 2)
 *  Finish the connection for server 2, see that the item is requested.
 *	Restart server 0 and server 1
 *  Disconnect server 2, see that the disconnection is handled by the consumer
 *  See that Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  See that both servers get the request
 */
void preferredHost_WSBService_FallbackFromChannelListAfterDisconnect(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list with all channels pointing to invalid server ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *	Request an item, and server 1 will repond with a full refresh(including payload), and server 0 will respond with a partial refresh(no payload)
 *  Disconnect channel server 0, see that nothing happens(since it is the standby)
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Ensure that the disconnection gets to the channel list
 *	Restart server 0 and server 1
 *  See that Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  See that both servers get the request
 */
void preferredHost_WSBService_ReconnectToPreferredAfterChannelListReconnectAttempt(PreferredHostTestParameters parameters);


/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED
 *  WSB index 0 and 1 point to two invalid ports each
 *  WSB index 2 points to server 0 and server 1
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list(this should be ignored)
 *  PH config is set to true, with the WSB group index set to 2
 *
 * Test behaviors:
 *  This test will immediately connect on WSB group 3 to server 0 and server 1(using wtfSetupWarmStandbyConnection), and after connection, server 0 will be set to ACTIVE on the login, server 1 will be set to STANDBY on the login
 *  The test will request an item, which will go to server 0 and server 1, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Verify that the consumer receives the full refresh payload
 */
void preferredHost_WSBLogin_ConnectionUp(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  1 WSB group, set for LOGIN_BASED, infinite reconnections
 *  WSB index 1 points to server 0 and server 1
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 0 and fallbackWithinWSBGroup set to TRUE
 *
 * Test behaviors:
 *  This test will immediately connect on WSB group 1 to server 0 and server 1(using wtfSetupWarmStandbyConnection), and after connection, server 0 will be set to ACTIVE on the login, server 1 will be set to STANDBY on the login
 *  Call fallbackToPreferredHost, see that the consumer immediately gets PREFERRED_HOST_COMPLETE
 *  Disconnect server 0(the WSB active), see that server 1 gets an login message stating that it is now the active 
 *  Reconnect server 0, see that it connects and that the watchlist assigns it as standby
 *  Request an item, and server 1 will repond with a full refresh(including payload), and server 0 will respond with a partial refresh(no payload)
 *  Call fallbackToPreferredHost, see that the consumer swaps the active and standby, meaning that sever 1 is now standby, server 0 is now active
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 */
void preferredHost_WSBLogin_FallbackWithinWSBGroup(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  A channel list of 3 invalid servers
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.(not used)
 *  PH config is set to true, with the WSB group index set to 2, the preferred host timer set to 2 seconds
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active, server 1 set as standby
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Restart server 2 and server 3
 *  Call fallbackToPreferredhost
 *  Server 2 and 3 will get connected and setup the login based WSB, with server 2 as the active, server 3 set as standby
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  Make sure that both providers get the item request
 */
void preferredHost_WSBLogin_FallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for SERVICE_BASED ith reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.
 *  PH config is set to true, with the WSB group index set to 2, the preferred host timer set to 3 seconds
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 1 as the active for service1 and server 0 set as standby
 *  Restart server 2 and server 3
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Wait for the preferred host operation to start, accepting a connection on server 2
 *  While the fallback is happening, have both server 0 and server 1 send an update, see that the consumer only gets one
 *  Perform the service based WSB connection, with server 3 set to active for service1, and server 2 set to standby for service1
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  Make sure that the watchlist sends requests to both server 2 and server 3
 */
void preferredHost_WSBLogin_FallbackTimer(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  A channel list of 3 invalid servers
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.(this is not used)
 *  PH config is set to true, with the WSB group index set to 2, the preferred host timer set to 2 seconds
 *
 * Test behaviors:
 *	Shut down sever 2 and server 3
 *  Startup the connection(using wtfSetupWarmStandbyConnection with connOpts.accept set to FALSE).
 *	The connection should attempt to connect to server 2, this will fail, and it will then connect to server 0
 *  Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active and server 1 set as standby
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Call fallbackToPreferredHost, since there is server 2 is still shutdown, there will be a PREFERRED_HOST_COMPLETE event without any changes
 */
void preferredHost_WSBLogin_InvalidWSBGroupFallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list of index 0 pointing to server 2, and the other channels pointing to invalid ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.(this is not used)
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active and server 1 set as standby
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Shutdown server 0 and server 1
 *  Disconnect channel server 0, see that the active is switched to server 1
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Watch that the reconnection is handled until it gets to channel list 0(server 2)
 *  Finish the connection for server 2, see that the item is requested.
 *  Call fallbackToPreferredHost
 *  Server 0 and 1 will get connected and setup the service based WSB, with server 0 as the active and server 1 set as standby
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 *  See that both servers get the request
 */
void preferredHost_WSBLogin_FallbackFunctionCallFromChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list of index 0 pointing to server 2, and the other channels pointing to invalid ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.(this will not be used)
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active and server 1 set as standby
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Disconnect channel server 0, see that the active gets switched to server 1
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Watch that the reconnection is handled until it gets to channel list 0(server 2)
 *  Finish the connection for server 2, see that the item is requested.
 *	Restart server 0 and server 1
 *  Disconnect server 2, see that the disconnection is handled by the consumer
 *  See that Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active and server 1 set as standby
 *  See that both servers get the request
 */
void preferredHost_WSBLogin_FallbackFromChannelListAfterDisconnect(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *  A size 3 channel list with all channels pointing to invalid server ports
 *  The starting connection does not have any services defined
 *  The secondary connection has the service "service1" defined in it's service list.(this is not used)
 *  PH config is set to true, with the WSB group index set to 2, the channelset index set to 0
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupWarmStandbyConnection), connecting to server 0 and server 1.
 *  Server 0 and 1 will get connected and setup the login based WSB, with server 1 as the active and server 1 set as standby
 *	Request an item, and server 0 will repond with a full refresh(including payload), and server 1 will respond with a partial refresh(no payload)
 *  Disconnect channel server 0, see that the active changes to server 1
 *  Disconnect channel server 1, see that the full disconnection is handled
 *  Ensure that the disconnection gets to the channel list
 *	Restart server 0 and server 1
 *  See that Server 0 and 1 will get connected and setup the login based WSB, with server 0 as the active and server 1 set as standby
 *  See that both servers get the request
 */
void preferredHost_WSBLogin_ReconnectToPreferredAfterChannelListReconnectAttempt(PreferredHostTestParameters parameters);



/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G0*, G1, G0*, G2, G0*, C0*,
 *	G0*, G1, G0*, G2, G0*, C1,
 *	G0*, G1, G0*, G2, G0*, C0*,
 *	G0*, G1, G0*, G2, G0*, C2
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 *	C0 is port 10000
 *	C1 is port 11000
 *	C2 is port 12000
 */
void preferredHost_WSB_ReconnectOrder_FirstGroupPreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G1*, G0, G1*, G2, G1*, C1*,
 *	G1*, G0, G1*, G2, G1*, C0,
 *	G1*, G0, G1*, G2, G1*, C1*,
 *	G1*, G0, G1*, G2, G1*, C2
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 *	C0 is port 10000
 *	C1 is port 11000
 *	C2 is port 12000
 */
void preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G2*, G0, G2*, G1, G2*, C2*,
 *	G2*, G0, G2*, G1, G2*, C0,
 *	G2*, G0, G2*, G1, G2*, C2*,
 *	G2*, G0, G2*, G1, G2*, C1
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 *	C0 is port 10000
 *	C1 is port 11000
 *	C2 is port 12000
 */
void preferredHost_WSB_ReconnectOrder_LastGroupPreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  No channel list
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G0*, G1, G0*, G2, ...
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 */
void preferredHost_WSB_ReconnectOrder_FirstGroupPreferred_NoChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  No channel list
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G1*, G0, G1*, G2, ...
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 */
void preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred_NoChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A 3 WSB group list
 *  WSB group 0 is set to invalid server ports
 *  WSB group 1 is set to invalid server ports
 *  WSB group 2 is set to invalid server ports
 *  No channel list
 *  PH config is set to true, the channelset index set to 0
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *	G2*, G0, G2*, G1, ...
 *
 *	G0 is port 15000
 *	G1 is port 16000
 *	G2 is port 17000
 */
void preferredHost_WSB_ReconnectOrder_LastGroupPreferred_NoChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *	No channel list
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection, connecting to wsb group 2
 *	Request an item
 *  Call rsslReactorChannelIOCTL and verify that the following failure conditions occur:
 *  A call with an invalid WSB group fails
 *  Invalid channel index fails
 *  Invalid cron string fails
 *  Invalid cron string length fails
 */
void preferredHost_WSB_IOCTL_Fail(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to invalid server ports
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 0 and server 1
 *	Channel list of 3 channels set to invalid server ports
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection, connecting to wsb group 2
 *	Request an item
 *  Call rsslReactorChannelIOCTL and verify that the following failure conditions occur:
 *  A call with an invalid WSB group fails
 *  Invalid channel index succeeds
 *  Invalid cron string fails
 *  Invalid cron string length fails
 */
void preferredHost_WSB_IOCTL_FailWithChannelList(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  3 WSB groups, all set for LOGIN_BASED with reconnect attempt limit set to 1
 *  WSB index 0 points to server 0 and server 1
 *  WSB index 1 points to invalid server ports
 *  WSB index 2 points to server 2 and server 3
 *  A size 3 channel list with all channels pointing to invalid server ports
 *
 * Test behaviors:
 *  Startup the WSB connection to group 2, request an item
 *  Call rsslReactorChannelIOCTL and verify that the change takes with rsslReactorGetChannelInfo by changing the following;
 *  Set preferredHostEnabled to FALSE, and call fallbackToPreferredHost and see that it correctly fails
 *  Change the preferred wsb group to 0
 *  Change the preferred channel index to 0
 *  Change detection time to 3600 seconds
 *  Change detection index to 5AM, Jan 1st("0 5 1 1 *")
 *  Change the channel index back to 2
 *  Change the wsb index to 0, call fallbackToPreferredHost
 *  Change the wsb index back to 2 after calling fallbackToPreferredHost
 *  call rsslReactorGetChannelInfo during the preferred host operation, make sure that it has not changed
 *  Finish the preferred host operation, see that the options have changed after the connection succeeds
 */
void preferredHost_WSB_IOCTL(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 0
 * 
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *  C0*, C1, C0*, C2...
 *	C0 is port 10000
 *	C1 is port 11000
 *  C2 is port 12000
 */
void preferredHost_ChannelList_ReconnectOrder_FirstPreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 1
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *  C1*, C0, C1*, C2...
 *	C0 is port 10000
 *	C1 is port 11000
 *  C2 is port 12000
 */
void preferredHost_ChannelList_ReconnectOrder_MiddlePreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to invalid server port
 *  Channel 1 set to invalid server port
 *  Channel 2 set to invalid server port
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Starup the connection, see that the reconnection order is the following(* indicates preferred):
 *  C2*, C0, C2*, C1...
 *	C0 is port 10000
 *	C1 is port 11000
 *  C2 is port 12000
 */
void preferredHost_ChannelList_ReconnectOrder_LastPreferred(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set to server 1
 *  Channel 2 set to server 2
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 */
void preferredHost_ChannelList_ConnectionUp(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set to server 1
 *  Channel 2 set to server 2
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 *  Disconnect server 2
 *  Accept a new incomming connection on server 0, and see that the item request goes through
 *  Call fallbackToPreferredHost
 *  During the PH operation, send an update from server 0 and see that the consumer gets it
 *  Accept a connection on server 2, see that the channel gets initialsed
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 */
void preferredHost_ChannelList_FallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set to server 1
 *  Channel 2 set to server 2
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 *  Disconnect server 2
 *  Accept a new incomming connection on server 0, and see that the item request goes through
 *  Wait for the PH operation to start(accepting sever 2)
 *  During the PH operation, send an update from server 0 and see that the consumer gets it
 *  Accept a connection on server 2, see that the channel gets initialsed
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event
 */
void preferredHost_ChannelList_FallbackTimer(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set an invalid server port
 *  Channel 2 set an invalid server port
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 *  Disconnect server 2
 *  Accept a new incomming connection on server 0, and see that the item request goes through
 *  Call fallbackToPreferredHost
 *  During the PH operation, send an update from server 0 and see that the consumer gets it
 *  Make sure that the consumer gets the PREFERRED_HOST_COMPLETE event without any changes
 */
void preferredHost_ChannelList_InvalidServerFallbackFunctionCall(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set an invalid server port
 *  Channel 2 set an invalid server port
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 *  Call rsslReactorChannelIOCTL and verify that the change takes with rsslReactorGetChannelInfo by changing the following;
 *  Set preferredHostEnabled to FALSE, and call fallbackToPreferredHost and see that it correctly fails
 *  Change the preferred channel index to 0
 *  Change detection time to 3600 seconds
 *  Change detection index to 5AM, Jan 1st("0 5 1 1 *")
 *  Change the channel index back to 2
 *  Change the index to 0, call fallbackToPreferredHost
 *  Change the channel index back to 2 after calling fallbackToPreferredHost
 *  call rsslReactorGetChannelInfo during the preferred host operation, make sure that it has not changed
 *  Finish the preferred host operation, see that the options have changed after the connection succeeds
 */
void preferredHost_ChannelList_IOCTL(PreferredHostTestParameters parameters);

/* This test does the following:
 * Initial setup:
 *  A size 3 channel list
 *  Channel 0 set to server 0
 *  Channel 1 set an invalid server port
 *  Channel 2 set an invalid server port
 *  PH config is set to true, the channelset index set to 2
 *
 * Test behaviors:
 *  Startup the connection(using wtfSetupConnectionList, setting server 2), connecting to server 2
 *  Server 2 will get connected
 *	Request an item, and server 2 will repond with a full refresh
 *  Call rsslReactorChannelIOCTL and verify that the following failure conditions occur:
 *  A call with an invalid WSB group succeeds(this is a don't care) with no changes
 *  Invalid channel index fails
 *  Invalid cron string fails
 *  Invalid cron string length fails
 */
void preferredHost_ChannelList_IOCTL_Fail(PreferredHostTestParameters parameters);


class PreferredHostUnitTest : public ::testing::TestWithParam<PreferredHostTestParameters> {
public:

	virtual void SetUp()
	{
		WtfInitOpts wtfInitOpts;
		wtfClearInitOpts(&wtfInitOpts);
		wtfInitOpts.numberOfServer = 4;
		wtfInit(&wtfInitOpts, 0);
		wtfsleep(200);

		/* Creates four servers by default. */
		wtfBindServer(RSSL_CONN_TYPE_SOCKET, const_cast<char*>("14011"));
		wtfBindServer(RSSL_CONN_TYPE_SOCKET, const_cast<char*>("14012"));
		wtfBindServer(RSSL_CONN_TYPE_SOCKET, const_cast<char*>("14013"));
		wtfBindServer(RSSL_CONN_TYPE_SOCKET, const_cast<char*>("14014"));

		wtfsleep(200);
	}

	virtual void TearDown()
	{
		wtfCloseServer(0);
		wtfCloseServer(1);
		wtfCloseServer(2);
		wtfCloseServer(3);
		wtfsleep(200);

		wtfCleanup();

		wtfsleep(200);
	}
};

INSTANTIATE_TEST_SUITE_P(
	TestingPreferredHostUnitTests,
	PreferredHostUnitTest,
	::testing::Values(

		PreferredHostTestParameters(RSSL_FALSE),
		PreferredHostTestParameters(RSSL_TRUE)

	));

#include <ctime>

#ifdef __cplusplus
extern "C" {
#endif
	RSSL_VA_API RsslInt64 getNextFallbackPreferredHostTime(RsslPreferredHostOptions* pPreferredHostOpts, RsslInt64 lastRecordedTimeMs);
#ifdef __cplusplus
};
#endif

void test_cron_minutes(const char* detectionTimeSchedule, RsslInt64 minutes)
{
	RsslInt64 lastRecordedTimeMs = 0;
	RsslInt64 next = 0;

	//RsslInt64 getNextFallbackPreferredHostTime(RsslPreferredHostOptions* pPreferredHostOpts, RsslInt64 lastRecordedTimeMs);
	RsslPreferredHostOptions preferredHostOpts{};
	preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;

	preferredHostOpts.detectionTimeSchedule.data = (char*)detectionTimeSchedule;
	preferredHostOpts.detectionTimeSchedule.length = (RsslUInt32)strlen(detectionTimeSchedule);
	lastRecordedTimeMs = 0;
	next = getNextFallbackPreferredHostTime(&preferredHostOpts, lastRecordedTimeMs);

	auto now = std::time(nullptr);
	auto t = std::localtime(&now);
	next /= 1000;
	RsslInt64 wait;
	if (t->tm_min >= minutes)
	{
		wait = ((60LL - t->tm_min) + (minutes - 1)) * 60 + (60LL - t->tm_sec);
	}
	else
	{
		wait = ((minutes - 1) - t->tm_min) * 60 + (60LL - t->tm_sec);
	}
	RsslInt64 diff = next - wait;
	EXPECT_EQ(next, wait);
}

TEST_F(PreferredHostUnitTest, Cron)
{
	test_cron_minutes("15 * * * *", 15);
	test_cron_minutes("05 * * * *", 05);
	test_cron_minutes("25 * * * *", 25);
	test_cron_minutes("55 * * * *", 55);
	test_cron_minutes("00 * * * *", 00);
}

TEST_P(PreferredHostUnitTest, PHChannelListFunctionCall)
{
	preferredHost_ChannelList_FallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListTimer)
{
	preferredHost_ChannelList_FallbackTimer(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListInvalidServerFunctionCall)
{
	preferredHost_ChannelList_InvalidServerFallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListConnectionUp)
{
	preferredHost_ChannelList_ConnectionUp(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListIoctl)
{
	preferredHost_ChannelList_IOCTL(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListIoctl_Failure)
{
	preferredHost_ChannelList_IOCTL_Fail(GetParam());
}


TEST_P(PreferredHostUnitTest, PHChannelListReconnectOrder_FirstPreferred)
{
	preferredHost_ChannelList_ReconnectOrder_FirstPreferred(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListReconnectOrder_MiddlePreferred)
{
	preferredHost_ChannelList_ReconnectOrder_MiddlePreferred(GetParam());
}

TEST_P(PreferredHostUnitTest, PHChannelListReconnectOrder_LastPreferred)
{
	preferredHost_ChannelList_ReconnectOrder_LastPreferred(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceBasedConnectionUp)
{
	preferredHost_WSBService_ConnectionUp(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceFallbackFunctionCall)
{
	preferredHost_WSBService_FallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceFallbackTimer)
{
	preferredHost_WSBService_FallbackTimer(GetParam());
}

TEST_P(PreferredHostUnitTest, PFWSBServiceBasedFallbackWithinWSBGroup)
{
	preferredHost_WSBService_FallbackWithinWSBGroup(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceFallbackFunctionCallFromChannelList)
{
	preferredHost_WSBService_FallbackFunctionCallFromChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceFallbackFromChannelListAfterDisconnect)
{
	preferredHost_WSBService_FallbackFromChannelListAfterDisconnect(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBServiceReconnectToPreferredAfterChannelListReconnectAttempt)
{
	preferredHost_WSBService_ReconnectToPreferredAfterChannelListReconnectAttempt(GetParam());
}


TEST_P(PreferredHostUnitTest, PHWSBServiceInvalidServerFunctionCall)
{
	preferredHost_WSBService_InvalidWSBGroupFallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginBasedConnectionUp)
{
	preferredHost_WSBLogin_ConnectionUp(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginFallbackFunctionCall)
{
	preferredHost_WSBLogin_FallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginFallbackTimer)
{
	preferredHost_WSBLogin_FallbackTimer(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginBasedFallbackWithinWSBGroup)
{
	preferredHost_WSBLogin_FallbackWithinWSBGroup(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginFallbackFunctionCallFromChannelList)
{
	preferredHost_WSBLogin_FallbackFunctionCallFromChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginFallbackFromChannelListAfterDisconnect)
{
	preferredHost_WSBLogin_FallbackFromChannelListAfterDisconnect(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginReconnectToPreferredAfterChannelListReconnectAttempt)
{
	preferredHost_WSBLogin_ReconnectToPreferredAfterChannelListReconnectAttempt(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBLoginInvalidServerFunctionCall)
{
	preferredHost_WSBLogin_InvalidWSBGroupFallbackFunctionCall(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBIoctl)
{
	preferredHost_WSB_IOCTL(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBIoctl_Failure)
{
	preferredHost_WSB_IOCTL_Fail(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBIoctl_Failure_WithChannelList)
{
	preferredHost_WSB_IOCTL_FailWithChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_FirstGroupPreferred_NoChannelList)
{
	preferredHost_WSB_ReconnectOrder_FirstGroupPreferred_NoChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_MiddleGroupPreferred_NoChannelList)
{
	preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred_NoChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_LastGroupPreferred_NoChannelList)
{
	preferredHost_WSB_ReconnectOrder_LastGroupPreferred_NoChannelList(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_FirstGroupPreferred)
{
	preferredHost_WSB_ReconnectOrder_FirstGroupPreferred(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_MiddleGroupPreferred)
{
	preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred(GetParam());
}

TEST_P(PreferredHostUnitTest, PHWSBReconnectOrder_LastGroupPreferred)
{
	preferredHost_WSB_ReconnectOrder_LastGroupPreferred(GetParam());
}


void preferredHost_WSBService_ConnectionUp(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void preferredHost_WSBLogin_ConnectionUp(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfFinishTest();
}

void preferredHost_WSBService_FallbackWithinWSBGroup(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslRDMLoginRefresh loginRefresh;
	RsslReactorSubmitMsgOptions submitOpts;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer.perServiceBasedOptions.serviceNameCount = 1;
	standbyServer.perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	// Check that the fallback does nothing
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));
	// Dispatch to make sure all the events and the user gets a preferred host complete
	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_ACTIVE;

	/* Provider should now accept. */
	wtfAccept(1);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 500, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);
	
	pEvent = wtfGetEvent();
	if (pEvent == NULL)
	{
		wtfDispatch(WTF_TC_PROVIDER, 100, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	wtfSendDefaultSourceDirectory(&connOpts, &pEvent->rdmMsg.pRdmMsg->directoryMsg.request, 1);

	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_STANDBY;

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_SERVICE_BASED_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());
	
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));
	// Dispatch to make sure all the events and the user gets a preferred host complete
	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);


	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_STANDBY;

	// There's a chance that the dispatch won't read both messages across the two connections, so dispatch again if pEvent is null.
	if ((pEvent = wtfGetEvent()) == NULL)
	{
		wtfDispatch(WTF_TC_PROVIDER, 400, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == RDM_DIRECTORY_SERVICE_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_ACTIVE;

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());


	wtfFinishTest();
}

void preferredHost_ChannelList_FallbackFunctionCall(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, * pRefreshMsg;
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService service;

	RsslUpdateMsg	updateMsg, * pUpdateMsg;

	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslStatusMsg* pStatusMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	// Close the provider channel.
	wtfCloseChannel(WTF_TC_PROVIDER, 2);
	
	// Dispatch consumer, wait for down_reconnecting
	wtfDispatch(WTF_TC_CONSUMER, 500);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags& RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	// Should attempt server index 0 next.
	wtfAccept(0);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 0);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Start the fallback
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));
	
	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);
	
	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}
	}

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags& RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// Get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;
	
	wtfFinishTest();
}

void preferredHost_ChannelList_FallbackTimer(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, * pRefreshMsg;
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService service;

	RsslUpdateMsg	updateMsg, * pUpdateMsg;

	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslStatusMsg* pStatusMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;
	connOpts.preferredHostOpts.detectionTimeInterval = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));


	if ((pEvent = wtfGetEvent()) == NULL)
	{
		wtfDispatch(WTF_TC_CONSUMER, 400);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	// Get PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	// Close the provider channel.
	wtfCloseChannel(WTF_TC_PROVIDER, 2);

	// Dispatch consumer, wait for down_reconnecting
	wtfDispatch(WTF_TC_CONSUMER, 500);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	// Should attempt server index 0 next.
	wtfAccept(0);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 0);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 3000);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}
	}

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// Get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_FallbackFunctionCall(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfShutdownServer(2);
	wtfShutdownServer(3);

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	while(!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	/* Provider should now accept. */
	wtfAccept(0);

	// re-start the two servers closed earlier. This is to give enough time for Windows to startup the listening socket
	wtfRestartServer(2);
	wtfRestartServer(3);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));
	}

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());


	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receive requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives several messages from the active service and channel events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	unsigned iRsslMsg = 0;
	unsigned iRdmMsg = 0;
	unsigned iChnlEvnt = 0;

	while ((pEvent = wtfGetEvent()) != NULL)
	{
		WtfEventType eventType = pEvent->base.type;
		switch (eventType) {
		case WTF_DE_RSSL_MSG:
		{
			RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
			ASSERT_TRUE(pMsg != NULL);
			if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)pMsg);
				ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
				ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
				ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
				ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
				ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
				ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));
			}
			else if (pMsg->msgBase.msgClass == RSSL_MC_STATUS)
			{
				/* Consumer receives item status. */
				ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)pMsg);
				ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
				ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
				ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
				ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
				ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
				ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
				ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
			}
			else
			{
				/* We do not expect additional RsslMsgs. */
				ASSERT_TRUE(0) << "We do not expect additional RsslMsgs. iRsslMsg: " << iRsslMsg;
			}
			++iRsslMsg;
			break;
		}
		case WTF_DE_RDM_MSG:
		{
			ASSERT_EQ(0, iRdmMsg) << "We do not expect additional RdmMsgs. iRdmMsg: " << iRdmMsg;

			/* Consumer receives Open/Suspect login status. */
			ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
			ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
			ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
			ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

			++iRdmMsg;
			break;
		}
		case WTF_DE_CHNL:
		{
			/* Get the channel events in order, step by step. */
			switch (iChnlEvnt) {
			case 0:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);
				break;
			case 1:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);
				break;
			case 2:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
				break;
			case 3:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
				break;
			case 4:
				ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));
				break;
			case 5:
				ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));
				break;
			default:
				/* We do not expect additional Channel events. */
				ASSERT_TRUE(0) << "We do not expect additional Channel events. iChnlEvnt: " << iChnlEvnt;
			}

			++iChnlEvnt;
			break;
		}
		default:  break;
		}
	}

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

 	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 3);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_FallbackFunctionCallFromChannelList(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags& RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14012) << " actual port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14012) << " actual port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000) << " actual port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14011) << " actual port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000) << " actual port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14011) << " actual port: " << pEvent->channelEvent.port;

	wtfAccept(2);
	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 2);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Accept the incomming connection
	wtfAccept(0);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 2);

	/* Consumer receives several messages from the active service and channel events. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	unsigned iRsslMsg = 0;
	unsigned iRdmMsg = 0;
	unsigned iChnlEvnt = 0;

	while ((pEvent = wtfGetEvent()) != NULL)
	{
		WtfEventType eventType = pEvent->base.type;
		switch (eventType) {
		case WTF_DE_RSSL_MSG:
		{
			RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
			ASSERT_TRUE(pMsg != NULL);
			if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)pMsg);
				ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
				ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
				ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
				ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
				ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
				ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));
			}
			else if (pMsg->msgBase.msgClass == RSSL_MC_STATUS)
			{
				/* Consumer receives item status. */
				ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)pMsg);
				ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
				ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
				ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
				ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
				ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
				ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
				ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
			}
			else
			{
				/* We do not expect additional RsslMsgs. */
				ASSERT_TRUE(0) << "We do not expect additional RsslMsgs. iRsslMsg: " << iRsslMsg;
			}
			++iRsslMsg;
			break;
		}
		case WTF_DE_RDM_MSG:
		{
			ASSERT_EQ(0, iRdmMsg) << "We do not expect additional RdmMsgs. iRdmMsg: " << iRdmMsg;

			/* Consumer receives Open/Suspect login status. */
			ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
			ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
			ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
			ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
			ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

			++iRdmMsg;
			break;
		}
		case WTF_DE_CHNL:
		{
			/* Get the channel events in order, step by step. */
			switch (iChnlEvnt) {
			case 0:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);
				break;
			case 1:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
				ASSERT_TRUE(pEvent->channelEvent.port == 14013) << " actual port: " << pEvent->channelEvent.port;
				break;
			case 2:
				ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);
				break;
			case 3:
				ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));
				break;
			case 4:
				ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));
				break;
			default:
				/* We do not expect additional Channel events. */
				ASSERT_TRUE(0) << "We do not expect additional Channel events. iChnlEvnt: " << iChnlEvnt;
			}

			++iChnlEvnt;
			break;
		}
		default:  break;
		}
	}

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_FallbackFromChannelListAfterDisconnect(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	wtfAccept(2);
	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 2);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 2);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	wtfCloseChannel(WTF_TC_PROVIDER, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags& RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);


	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// Accept the incomming connection
	wtfAccept(0);

	// get CHANNEL_UP
	wtfDispatch(WTF_TC_CONSUMER, 100);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_ReconnectToPreferredAfterChannelListReconnectAttempt(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RSSL_MSG);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 10000);

	// Accept the start of the WSB group.
	wtfAccept(0);

	// get CHANNEL_UP
	wtfDispatch(WTF_TC_CONSUMER, 100);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_FallbackTimer(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfShutdownServer(2);
	wtfShutdownServer(3);

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;
	connOpts.preferredHostOpts.detectionTimeInterval = 3;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	/* Provider should now accept. */
	wtfAccept(0);

	/* Established connection on the non-preferred host. */
	/* ReactorWorker starts timer: fallback by timeout. */

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));
	}

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 500);
	}
	// Get PREFERRED_HOST_COMPLETE

	ASSERT_TRUE(pEvent != NULL);
	/* Consumer should the STARTING_FALLBACK and HOST_COMPLETE event. */
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 500);
	}

	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	// re-start the two servers closed earlier. This is to give enough time for Windows to startup the listening socket
	wtfRestartServer(2);
	wtfRestartServer(3);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receive requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 3500);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	// Consumer might receive a update msg
	ASSERT_TRUE(pEvent != NULL);
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
		ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
		ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

		// Get the next event
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// Get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);


	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 500);
	}

	// Consumer gets PREFERRED_HOST_COMPLETE
	if (pEvent != NULL)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);
	}

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 3);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBService_InvalidWSBGroupFallbackFunctionCall(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	RsslInt32		providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;
	RsslRDMDirectoryRefresh directoryRefresh;


	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */
	WtfTestServer* startingServer;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	while(!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	/* Provider should now accept. */
	wtfAccept();

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);
	

	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	startingServer = getWtfTestServer(0);
	startingServer->warmStandbyMode = WTF_WSBM_SERVICE_BASED_ACTIVE;

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);
	

	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());


	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	// Get PREFERRED_HOST_COMPLETE
	pEvent = wtfGetEvent();
	if (pEvent == NULL)
	{
		wtfDispatch(WTF_TC_CONSUMER, 10000);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);


	wtfFinishTest();
}

void preferredHost_WSBLogin_FallbackWithinWSBGroup(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup;
	RsslReactorWarmStandbyServerInfo standbyServer;
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslRDMLoginRefresh loginRefresh;
	RsslReactorSubmitMsgOptions submitOpts;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup);

	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer);

	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		standbyServer.reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup.standbyServerCount = 1;
	reactorWarmstandByGroup.standbyServerList = &standbyServer;
	reactorWarmstandByGroup.warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;


	connOpts.warmStandbyGroupCount = 1;
	connOpts.reactorWarmStandbyGroupList = &reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));
	// Dispatch to make sure all the events and the user gets a preferred host complete
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Dispatch to make sure all the events and the user gets a preferred host complete
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE;

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Dispatch to make sure all the events and the user gets a preferred host complete
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	wtfDispatch(WTF_TC_CONSUMER, 400);

	// Dispatch to make sure all the events and the user gets a preferred host complete
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept. */
	wtfAccept(0);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	pEvent = wtfGetEvent();
	if (pEvent == NULL)
	{
		wtfDispatch(WTF_TC_PROVIDER, 100, 0);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);
	pWtfTestServer = getWtfTestServer(pEvent->rdmMsg.serverIndex);
	pWtfTestServer->warmStandbyMode = WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY;

	// There's a chance that the dispatch won't read both messages across the two connections, so dispatch again if pEvent is null.
	if ((pEvent = wtfGetEvent()) == NULL)
	{
		wtfDispatch(WTF_TC_PROVIDER, 400, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	wtfSendDefaultSourceDirectory(&connOpts, &pEvent->rdmMsg.pRdmMsg->directoryMsg.request, 0);

	wtfDispatch(WTF_TC_CONSUMER, 200);
	wtfDispatch(WTF_TC_CONSUMER, 200);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup.startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));
	
	wtfDispatch(WTF_TC_CONSUMER, 400);

	// Dispatch to make sure all the events and the user gets a preferred host complete
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	if(pEvent->rdmMsg.serverIndex == 0)
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	else
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_STANDBY);

	// There's a chance that the dispatch won't read both messages across the two connections, so dispatch again if pEvent is null.
	if ((pEvent = wtfGetEvent()) == NULL)
	{
		wtfDispatch(WTF_TC_PROVIDER, 400, 1);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	if (pEvent->rdmMsg.serverIndex == 0)
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	else
		ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_STANDBY);

	/* Provider should receive no more message. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* The active provider sends a update message with payload. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer doesn't receive an update message from the standby service. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_FALSE(pEvent = wtfGetEvent());


	wtfFinishTest();
}

void preferredHost_WSBService_FallbackFunctionCall(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg *pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus *pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfShutdownServer(2);
	wtfShutdownServer(3);

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	/* Provider should now accept. */
	wtfAccept(0);

	// re-start the two servers closed earlier. This is to give enough time for Windows to startup the listening socket
	wtfRestartServer(2);
	wtfRestartServer(3);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));
	}

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);


	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receive requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags& RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags& RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer should the STARTING_FALLBACK event. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}
	}

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags& RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags& RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE, depending on timing.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) || (pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));


	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE, depending on timing.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY) || (pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old providers.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change(Consumer should not get the above response, as it's for a secondary). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);


	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_CONSUMER, 100);

	wtfDispatch(WTF_TC_PROVIDER, 400, 3, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBService_FallbackFunctionCallFromChannelList(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	wtfAccept(2);
	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	/* Consumer channel up. */
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 100);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 2);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 400);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	// Accept the incomming connection
	wtfAccept(0);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 200);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);



	/* Consumer may receive a update message from the active service. */
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
		ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
		ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
		ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
		ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
		ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

		// Get the next event
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider receives request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_CONSUMER, 100);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	/* Provider receives request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBService_FallbackFromChannelListAfterDisconnect(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	wtfAccept(2);
	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 2);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 400);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 2);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

	wtfCloseChannel(WTF_TC_PROVIDER, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);


	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// Accept the incomming connection
	wtfAccept(0);

	// get CHANNEL_UP
	wtfDispatch(WTF_TC_CONSUMER, 100);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags& RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_CONSUMER, 100);

	wtfDispatch(WTF_TC_PROVIDER, 400, 1, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);


	/* Provider receives request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBService_ReconnectToPreferredAfterChannelListReconnectAttempt(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_TRUE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	wtfShutdownServer(0);
	wtfShutdownServer(1);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Disconnect the two providers.

	/* Closes the channel of the secondary server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 0);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Closes the channel of the active server. */
	wtfCloseChannel(WTF_TC_PROVIDER, 1);

	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14012);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 14011);

	// startup the servers here so there's time to initialize them in the OS
	wtfRestartServer(0);
	wtfRestartServer(1);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_EQ(pEvent->channelEvent.port, 10000);

	// Accept the start of the WSB group.
	wtfAccept(0);

	// get CHANNEL_UP
	wtfDispatch(WTF_TC_CONSUMER, 100);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags& RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags& RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags& RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	/* Provider receives login request. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 100);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_CONSUMER, 100);


	wtfDispatch(WTF_TC_PROVIDER, 400, 1, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	/* Provider receives request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBService_FallbackTimer(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_STANDBY;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;

	ASSERT_TRUE(wtfStartTest());

	wtfShutdownServer(2);
	wtfShutdownServer(3);

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_SERVICE_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;
	connOpts.preferredHostOpts.detectionTimeInterval = 3;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING || pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);
	if (pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING)
		ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	/* Provider should now accept. */
	wtfAccept(0);

	/* Established connection on the non-preferred host. */
	/* ReactorWorker starts timer: fallback by timeout. */

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));
	}

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 400);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	/* Consumer should the STARTING_FALLBACK event. */
	RsslUInt timeoutMs = 0;

	pEvent = wtfGetEvent();
	while (pEvent == NULL && timeoutMs < 2000)
	{
		wtfDispatch(WTF_TC_CONSUMER, 500);
		timeoutMs += 500;

		pEvent = wtfGetEvent();
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 500);
	}

	// Get PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);
	}

	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	wtfDispatch(WTF_TC_CONSUMER, 500);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	// re-start the two servers closed earlier. This is to give enough time for Windows to startup the listening socket
	wtfRestartServer(2);
	wtfRestartServer(3);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receive requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Accept the incomming connection for the fallback
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 1) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 1);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer should the STARTING_FALLBACK event. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}
	}

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// Get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old providers.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 400, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[0].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change(Consumer should not get the above response, as it's for a secondary). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	pEvent = wtfGetEvent();
	if (pEvent != NULL)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);
	}

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	// Dispatch to send requests.  Due to timing, the PREFERRED_HOST_COMPLETE event may fire here.
	wtfDispatch(WTF_TC_CONSUMER, 200);
	pEvent = wtfGetEvent();
	if(pEvent != NULL)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);
	}

	wtfDispatch(WTF_TC_CONSUMER, 200);
	pEvent = wtfGetEvent();
	if (pEvent != NULL)
	{
		ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
		ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);
	}

	wtfDispatch(WTF_TC_PROVIDER, 400, 3, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_CONSUMER_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].flags == RDM_DR_CSSF_HAS_WARM_STANDBY_MODE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].serviceId == rdmService[1].serviceId);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->directoryMsg.consumerStatus.consumerServiceStatusList[0].warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	wtfFinishTest();
}

void preferredHost_WSBLogin_InvalidWSBGroupFallbackFunctionCall(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	RsslInt32		providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;
	RsslRDMDirectoryRefresh directoryRefresh;


	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	/* Provider should now accept. */
	wtfAccept();

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());


	wtfDispatch(WTF_TC_CONSUMER, 400);


	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Get PREFERRED_HOST_COMPLETE
	pEvent = wtfGetEvent();
	if (pEvent == NULL)
	{
		wtfDispatch(WTF_TC_CONSUMER, 10000);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	/* Consumer should the STARTING_FALLBACK event. */
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);
	
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);


	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_FirstGroupPreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// G0*, G1, G0*, G2, G0*, C0*,
	// G0*, G1, G0*, G2, G0*, C1,
	// G0*, G1, G0*, G2, G0*, C0*,
	// G0*, G1, G0*, G2, G0*, C2

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000
	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 1;
	connOpts.preferredHostOpts.connectionListIndex = 1;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configurd preferred):
	// G1*, G0, G1*, G2, G1*, C1*,
	// G1*, G0, G1*, G2, G1*, C0,
	// G1*, G0, G1*, G2, G1*, C1*,
	// G1*, G0, G1*, G2, G1*, C2

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000
	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_LastGroupPreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// G2*, G0, G2*, G1, G2*, C2*,
	// G2*, G0, G2*, G1, G2*, C0,
	// G2*, G0, G2*, G1, G2*, C2*,
	// G2*, G0, G2*, G1, G2*, C1

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000
	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);
	
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_FirstGroupPreferred_NoChannelList(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// G0*, G1, G0*, G2...

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_MiddleGroupPreferred_NoChannelList(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 1;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configurd preferred):
	// G1*, G0, G1*, G2...

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_WSB_ReconnectOrder_LastGroupPreferred_NoChannelList(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17000");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("17001");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// G2*, G0, G2*, G1, G2*...

	// G0 is port 15000
	// G1 is port 16000
	// G2 is port 17000
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 15000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 16000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 17000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_ChannelList_IOCTL(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, * pRefreshMsg;
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService service;

	RsslUpdateMsg	updateMsg, * pUpdateMsg;

	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslStatusMsg* pStatusMsg;

	RsslPreferredHostOptions preferredHostOptions;
	RsslReactorChannelInfo channelInfo;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	// Close the provider channel.
	wtfCloseChannel(WTF_TC_PROVIDER, 2);

	// Dispatch consumer, wait for down_reconnecting
	wtfDispatch(WTF_TC_CONSUMER, 500);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	// Should attempt server index 0 next.
	wtfAccept(0);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 0);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Start testing the ioctl calls
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	// Change the connectionListIndex value
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_FALSE;
	preferredHostOptions.connectionListIndex = 0;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify the changes
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Make sure fallbacktopreferredhost fails now
	ASSERT_TRUE(rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	// Change the connectionListIndex value
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 0;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify the changes
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Change the detectionTimeInterval to 3600(1 hour)
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.detectionTimeInterval = 3600;
	preferredHostOptions.connectionListIndex = 2;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 3600);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the detectionTimeInterval to 0 and the detection time schedule to 5:00 AM local
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.detectionTimeInterval = 0;
	preferredHostOptions.connectionListIndex = 2;
	preferredHostOptions.detectionTimeSchedule = { 9 , const_cast<char*>("0 5 1 1 *") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify what's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 9);
	ASSERT_TRUE(strcmp(const_cast<char*>("0 5 1 1 *"), channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.data) == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the options back.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.connectionListIndex = 2;


	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify what's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Change the options to index 0.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.connectionListIndex = 0;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	// Accept the incomming connection
	wtfAccept(2);

	// Verify that the preferred host index has not changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);


	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());

	/* Consumer should the STARTING_FALLBACK event. */
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			
		}

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// Get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	// Verify that the preferred host index has not changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);


	wtfFinishTest();
}

void preferredHost_WSB_IOCTL_Fail(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslPreferredHostOptions preferredHostOptions;
	RsslReactorChannelInfo channelInfo;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo		errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	// Setup a WSB connection so we can call ioctl calls on it.
	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;
	
	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* We're connected, we have info, start to call ioctl failure cases */
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	// Set invalid channel list index, this should succeed and not change the wsb group.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 5;
	preferredHostOptions.warmStandbyGroupListIndex = 2;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set invalid wsb group index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.warmStandbyGroupListIndex = 5;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set a bad detection time schedule list index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.detectionTimeSchedule = {20,  const_cast<char*>("this is a bad string")};

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);


	// Set a bad too long time schedule list index string
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	// Just need to set the length here to more than 255.
	preferredHostOptions.detectionTimeSchedule = { 300,  const_cast<char*>("this is a bad string") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	wtfFinishTest();
}

void preferredHost_WSB_IOCTL_FailWithChannelList(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	WtfTestServer* pWtfTestServer;
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslPreferredHostOptions preferredHostOptions;
	RsslReactorChannelInfo channelInfo;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo		errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	// Setup a WSB connection so we can call ioctl calls on it.
	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;


	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_TRUE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);


	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	pWtfTestServer = getWtfTestServer(0);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	pWtfTestServer = getWtfTestServer(1);

	ASSERT_TRUE(pWtfTestServer->warmStandbyMode == WTF_WSBM_LOGIN_BASED_SERVER_TYPE_STANDBY);
	ASSERT_TRUE(pWtfTestServer->pServer->portNumber == atoi(standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName));

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* We're connected, we have info, start to call ioctl failure cases */
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	// Set invalid channel list index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 5;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set invalid wsb group index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.warmStandbyGroupListIndex = 5;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set a bad detection time schedule list index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.detectionTimeSchedule = { 20,  const_cast<char*>("this is a bad string") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);


	// Set a bad too long time schedule list index string
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	// Just need to set the length here to more than 255.
	preferredHostOptions.detectionTimeSchedule = { 300,  const_cast<char*>("this is a bad string") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	wtfFinishTest();
}

void preferredHost_WSB_IOCTL(PreferredHostTestParameters parameters)
{
	RsslReactorSubmitMsgOptions opts;
	WtfEvent* pEvent;
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, refreshMsg2, * pRefreshMsg;
	RsslUpdateMsg	updateMsg, * pUpdateMsg;
	RsslStatusMsg* pStatusMsg;
	WtfSetupWarmStandbyOpts connOpts;
	RsslReactorWarmStandbyGroup		reactorWarmstandByGroup[3];
	RsslReactorWarmStandbyServerInfo standbyServer[3];
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslUInt32		providerDirectoryFilter;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	WtfWarmStandbyExpectedMode warmStandbyExpectedMode[2];
	RsslRDMService rdmService[2]; /* index 0 is service for active server and 1 for standby server. */

	RsslPreferredHostOptions preferredHostOptions;
	RsslReactorChannelInfo channelInfo;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;

	warmStandbyExpectedMode[0].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_ACTIVE;
	warmStandbyExpectedMode[1].warmStandbyMode = RDM_LOGIN_SERVER_TYPE_STANDBY;

	ASSERT_TRUE(wtfStartTest());

	wtfShutdownServer(2);
	wtfShutdownServer(3);

	wtfClearSetupWarmStandbyConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[0]);

	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[0].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[0]);

	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[0].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[0].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[0].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[0].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[0].standbyServerCount = 1;
	reactorWarmstandByGroup[0].standbyServerList = &standbyServer[0];
	reactorWarmstandByGroup[0].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[1]);

	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16000");
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[1].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[1]);

	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("16001");
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[1].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[1].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[1].perServiceBasedOptions.serviceNameList = &selectServiceName;

	if (parameters.multiLoginMsg)
	{
		standbyServer[1].reactorConnectInfo.loginReqIndex = 1;
	}


	reactorWarmstandByGroup[1].standbyServerCount = 1;
	reactorWarmstandByGroup[1].standbyServerList = &standbyServer[1];
	reactorWarmstandByGroup[1].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	/* Set warm standby configuration with one active server and one stand by server. This will be the one we connect to */
	rsslClearReactorWarmStandbyGroup(&reactorWarmstandByGroup[2]);

	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	if (parameters.multiLoginMsg)
	{
		reactorWarmstandByGroup[2].startingActiveServer.reactorConnectInfo.loginReqIndex = 0;
	}

	rsslClearReactorWarmStandbyServerInfo(&standbyServer[2]);

	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14014");
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	standbyServer[2].reactorConnectInfo.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	standbyServer[2].perServiceBasedOptions.serviceNameCount = 1;
	standbyServer[2].perServiceBasedOptions.serviceNameList = &selectServiceName;


	if (parameters.multiLoginMsg)
	{
		standbyServer[2].reactorConnectInfo.loginReqIndex = 1;
	}

	reactorWarmstandByGroup[2].standbyServerCount = 1;
	reactorWarmstandByGroup[2].standbyServerList = &standbyServer[2];
	reactorWarmstandByGroup[2].warmStandbyMode = RSSL_RWSB_MODE_LOGIN_BASED;

	connOpts.warmStandbyGroupCount = 3;
	connOpts.reactorWarmStandbyGroupList = reactorWarmstandByGroup;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = 1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 2;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	wtfSetService1Info(&rdmService[0]);
	wtfSetService1Info(&rdmService[1]);

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupWarmStandbyConnection(&connOpts, &warmStandbyExpectedMode[0], &rdmService[0], &rdmService[1], RSSL_FALSE, RSSL_CONN_TYPE_SOCKET, parameters.multiLoginMsg);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	}
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	/* Provider should now accept. */
	wtfAccept(0);

	// re-start the two servers closed earlier. This is to give enough time for Windows to startup the listening socket
	wtfRestartServer(2);
	wtfRestartServer(3);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 0);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	/* Consumer channel ready (reactor sends login request). */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &activeUserName));
	}

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh, true);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	if (parameters.multiLoginMsg == RSSL_FALSE)
	{
		ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);
	}

	/* Provider receives a generic message on the login domain to indicate warm standby mode. */
	wtfDispatch(WTF_TC_PROVIDER, 200, 0);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[0].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 0);

	/* Provider receives source directory request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);


	/* Dispatch to make a connection to secondary server. */
	wtfDispatch(WTF_TC_CONSUMER, 800);
	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 1);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 1);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider receives login request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 1);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 1, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 1);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());


	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Both Providers receive requests. */
	wtfDispatch(WTF_TC_PROVIDER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* The standby server sends a refresh message with no payload. */
	rsslClearRefreshMsg(&refreshMsg2);
	refreshMsg2.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg2.msgBase.streamId = providerStreamId;
	refreshMsg2.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg2.msgBase.containerType = RSSL_DT_NO_DATA;
	refreshMsg2.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg2.msgBase.msgKey.serviceId = service1Id;
	refreshMsg2.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg2.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg2.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg2.state.dataState = RSSL_DATA_OK;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg2;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 1);

	/* Consumer receives a refresh message from the active server only. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more message from the standby server. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	// Start testing the ioctl here
	
	// Change the enablePreferredHostOptions value
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_FALSE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.warmStandbyGroupListIndex = 2;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Make sure fallbacktopreferredhost fails now
	ASSERT_TRUE(rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	// Change the fallbackwithinwsbgroup value
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.warmStandbyGroupListIndex = 2;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the connectionListIndex value
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.connectionListIndex = 2;
	preferredHostOptions.warmStandbyGroupListIndex = 2;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify the changes
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the warmStandbyGroupListIndex value to 0
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.warmStandbyGroupListIndex = 0;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify the changes
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Change the detectionTimeInterval to 3600(1 hour)
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.detectionTimeInterval = 3600;
	preferredHostOptions.warmStandbyGroupListIndex = 2;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 3600);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the detectionTimeInterval to 0 and the detection time schedule to 5:00 AM local
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.detectionTimeInterval = 0;
	preferredHostOptions.warmStandbyGroupListIndex = 2;
	preferredHostOptions.detectionTimeSchedule = {9 , const_cast<char*>("0 5 * * *") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify what's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 9);
	ASSERT_TRUE(strcmp(const_cast<char*>("0 5 * * *"), channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.data) == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Change the options back.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.warmStandbyGroupListIndex = 2;


	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify what's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Call fallbacktopreferredhost
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	// Change the options again.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_FALSE;
	preferredHostOptions.warmStandbyGroupListIndex = 0;


	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);


	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// These should not change
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	// Accept the incomming connection
	wtfAccept(2);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Consumer receives a update message from the active service. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	if (pEvent->base.type == WTF_DE_RSSL_MSG)
	{
		RsslMsg* pMsg = (RsslMsg*)wtfGetRsslMsg(pEvent);
		ASSERT_TRUE(pMsg != NULL);
		if (pMsg->msgBase.msgClass == RSSL_MC_UPDATE)
		{
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));

			// Get the next event
			ASSERT_TRUE(pEvent = wtfGetEvent());
		}
	}

	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	// get down_reconnecting
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);

	// get CHANNEL_UP
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	// get CHANNEL_READY or PREFERRED_HOST_COMPLETE
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE((pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY || pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE));

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	// Receive channel down for the old provider.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 2);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);

	wtfDispatch(WTF_TC_PROVIDER, 200, 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == RDM_LOGIN_SERVER_TYPE_ACTIVE);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 2);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&rdmService[0]);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	// Dispatch consumer to receive directory.
	wtfDispatch(WTF_TC_CONSUMER, 100);

	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider should now accept for the secondary server. */
	wtfAcceptWithTime(1000, 3);

	wtfDispatch(WTF_TC_CONSUMER, 100);

	/* Consumer channel FD change. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_FD_CHANGE);

	/* Provider channel up & ready
	 * (must be checked before consumer; in the case of raw providers,
	 * this call will initialize the channel). */
	wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_PROVIDER, 200, 3);
	}
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		ASSERT_TRUE(rsslBufferIsEqual(&pEvent->rdmMsg.pRdmMsg->loginMsg.request.userName, &standbyUserName));
	}

	wtfInitDefaultLoginRefresh(&loginRefresh, true);
	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 200);

	wtfDispatch(WTF_TC_PROVIDER, 200, 3, 1);

	// Secondary server gets the STANDBY call.
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_CONSUMER_CONNECTION_STATUS);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.flags == RDM_LG_CCSF_HAS_WARM_STANDBY_INFO);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->loginMsg.consumerConnectionStatus.warmStandbyInfo.warmStandbyMode == warmStandbyExpectedMode[1].warmStandbyMode);
	ASSERT_TRUE(pEvent->rdmMsg.serverIndex == 3);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &rdmService[0];
	directoryRefresh.serviceCount = 1;

	rdmService[0].flags |= RDM_SVCF_HAS_LOAD;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_LIMIT;
	rdmService[0].load.openLimit = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_OPEN_WINDOW;
	rdmService[0].load.openWindow = 0xffffffffffffffffULL;
	rdmService[0].load.flags |= RDM_SVC_LDF_HAS_LOAD_FACTOR;
	rdmService[0].load.loadFactor = 65535;

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 3);


	wtfDispatch(WTF_TC_CONSUMER, 1000);

	/* Consumer should receive no more messages. */
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 3);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;


	// This should be changed to the new value
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeInterval == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_FALSE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_FALSE);

	wtfFinishTest();
}

void preferredHost_ChannelList_ReconnectOrder_FirstPreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;
	connOpts.requestTimeout = 50000;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 0;

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// C0*, C1, C0*, C2...

	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000) << "Expected port is 10000. port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000) << "Expected port is 11000. port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000) << "Expected port is 10000. port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000) << "Expected port is 12000. port: " << pEvent->channelEvent.port;

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000) << "Expected port is 10000. port: " << pEvent->channelEvent.port;

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_ChannelList_ReconnectOrder_MiddlePreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;
	connOpts.requestTimeout = 50000;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 1;

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// C1*, C0, C1*, C2...

	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_ChannelList_ReconnectOrder_LastPreferred(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("10000");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("11000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("12000");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;
	connOpts.requestTimeout = 50000;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	connOpts.accept = RSSL_FALSE;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_FALSE);

	// Expected connection order(* is configured preferred):
	// C2*, C0, C2*, C1...

	// C0 is port 10000
	// C1 is port 11000
	// C2 is port 12000

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 10000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 11000);

	while (!(pEvent = wtfGetEvent()))
	{
		wtfDispatch(WTF_TC_CONSUMER, 200);
	};
	ASSERT_TRUE(pEvent != NULL);
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 12000);

	// Get all of the new events before finish test.
	// Under Linux the channel reconnection procedure performed really fast when the open operation returns fail.
	while ((pEvent = wtfGetEvent()) != NULL)
	{
	};

	wtfFinishTest();
}

void preferredHost_ChannelList_ConnectionUp(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));

	wtfFinishTest();
}

void preferredHost_ChannelList_ConnectionUp_NoWatchlist(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));

	wtfFinishTest();
}

void preferredHost_ChannelList_InvalidServerFallbackFunctionCall(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;
	RsslRefreshMsg	refreshMsg, * pRefreshMsg;
	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;
	WtfMarketPriceItem marketPriceItem;
	RsslInt32		providerStreamId, providerLoginStreamId, providerDirectoryStreamId;
	RsslUInt32		providerDirectoryFilter;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo errorInfo;
	RsslRDMDirectoryRefresh directoryRefresh;
	RsslRDMService service;

	RsslUpdateMsg	updateMsg, * pUpdateMsg;

	RsslReactorSubmitMsgOptions submitOpts;
	RsslRDMLoginRefresh loginRefresh;
	RsslRDMLoginStatus* pLoginStatus;
	RsslStatusMsg* pStatusMsg;
	RsslPreferredHostOptions preferredHostOptions;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}


	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("15000");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 2) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 2);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	// Close the provider channel.
	wtfCloseChannel(WTF_TC_PROVIDER, 2);

	// Dispatch consumer, wait for down_reconnecting
	wtfDispatch(WTF_TC_CONSUMER, 500);

	/* Consumer receives Open/Suspect login status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pLoginStatus = (RsslRDMLoginStatus*)wtfGetRdmMsg(pEvent));
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.streamId == WTF_DEFAULT_CONSUMER_LOGIN_STREAM_ID);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pLoginStatus->rdmMsgBase.rdmMsgType == RDM_LG_MT_STATUS);
	ASSERT_TRUE(pLoginStatus->flags & RDM_LG_STF_HAS_STATE);
	ASSERT_TRUE(pLoginStatus->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pLoginStatus->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pLoginStatus->state.code == RSSL_SC_NONE);

	/* Consumer receives item status. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pStatusMsg = (RsslStatusMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pStatusMsg->msgBase.msgClass == RSSL_MC_STATUS);
	ASSERT_TRUE(pStatusMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pStatusMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pStatusMsg->flags & RSSL_STMF_HAS_STATE);
	ASSERT_TRUE(pStatusMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pStatusMsg->state.dataState == RSSL_DATA_SUSPECT);
	ASSERT_TRUE(pStatusMsg->state.code == RSSL_SC_NONE);
	ASSERT_TRUE(pStatusMsg->msgBase.containerType == RSSL_DT_NO_DATA);

	/* Consumer receives channel event. */
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING);
	ASSERT_TRUE(pEvent->channelEvent.port == 14013);

	// Should attempt server index 0 next.
	wtfAccept(0);

	/* Consumer channel up. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider channel up & ready */
	wtfDispatch(WTF_TC_PROVIDER, 500, 0);
	ASSERT_TRUE((pEvent = wtfGetEvent()));
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_UP);

	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_CHANNEL_READY);

	/* Provider receives login request. */
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_LOGIN);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST);
	providerLoginStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	/* Provider sends login response. */
	wtfInitDefaultLoginRefresh(&loginRefresh);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&loginRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives login response. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REFRESH);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId == 1);
	ASSERT_TRUE(pEvent->rdmMsg.pUserSpec == (void*)0x55557777);

	/* Provider receives source directory request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_RDM_MSG);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.domainType == RSSL_DMT_SOURCE);
	ASSERT_TRUE(pEvent->rdmMsg.pRdmMsg->rdmMsgBase.rdmMsgType == RDM_DR_MT_REQUEST);

	/* Filter should contain at least Info, State, and Group filters. */
	providerDirectoryFilter =
		pEvent->rdmMsg.pRdmMsg->directoryMsg.request.filter;
	ASSERT_TRUE(providerDirectoryFilter ==
		(RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER));

	/* Request should not have service ID. */
	ASSERT_TRUE(!(pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_HAS_SERVICE_ID));

	/* Request should be streaming. */
	ASSERT_TRUE((pEvent->rdmMsg.pRdmMsg->directoryMsg.request.flags
		& RDM_DR_RQF_STREAMING));

	providerDirectoryStreamId = pEvent->rdmMsg.pRdmMsg->rdmMsgBase.streamId;

	rsslClearRDMDirectoryRefresh(&directoryRefresh);
	directoryRefresh.rdmMsgBase.streamId = providerDirectoryStreamId;
	directoryRefresh.filter = providerDirectoryFilter;
	directoryRefresh.flags = RDM_DR_RFF_SOLICITED | RDM_DR_RFF_CLEAR_CACHE;

	directoryRefresh.serviceList = &service;
	directoryRefresh.serviceCount = 1;

	wtfSetService1Info(&service);

	rsslClearReactorSubmitMsgOptions(&submitOpts);
	submitOpts.pRDMMsg = (RsslRDMMsg*)&directoryRefresh;
	wtfSubmitMsg(&submitOpts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 0);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 0);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));
	providerStreamId = pRequestMsg->msgBase.streamId;

	/* The active provider sends a refresh message with payload. */
	rsslClearRefreshMsg(&refreshMsg);
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_HAS_QOS
		| RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE;
	refreshMsg.msgBase.streamId = providerStreamId;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	refreshMsg.msgBase.msgKey.serviceId = service1Id;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	mpDataBody.length = mpDataBodyLen;

	wtfInitMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	refreshMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&refreshMsg;
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_TRUE, 0);

	/* Consumer receives a refresh message from the provider. */
	wtfDispatch(WTF_TC_CONSUMER, 400);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRefreshMsg = (RsslRefreshMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRefreshMsg->msgBase.streamId == consumerStreamId);
	ASSERT_TRUE(pRefreshMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(pRefreshMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
	ASSERT_TRUE(pRefreshMsg->msgBase.msgKey.serviceId == service1Id);
	ASSERT_TRUE(pRefreshMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRefreshMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRefreshMsg->state.streamState == RSSL_STREAM_OPEN);
	ASSERT_TRUE(pRefreshMsg->state.dataState == RSSL_DATA_OK);
	ASSERT_TRUE(rsslBufferIsEqual(&pRefreshMsg->msgBase.encDataBody, &mpDataBody));

	/* Consumer should receive no more messages. */
	wtfDispatch(WTF_TC_CONSUMER, 100);

	// Set ioctl to a index 1(bad server
	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	rsslClearRsslPreferredHostOptions(&preferredHostOptions);

	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 1;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS,
		(void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);


	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReactorFallbackToPreferredHost(consumerChannel, &errorInfo));

	/* Consumer should the STARTING_FALLBACK event. */
	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pEvent->base.type == WTF_DE_CHNL);
	ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK);

	/* The provider sends an update message with payload on the old consumer. */
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.flags = RSSL_UPMF_HAS_MSG_KEY;
	updateMsg.msgBase.streamId = providerStreamId;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID;
	updateMsg.msgBase.msgKey.serviceId = service1Id;
	mpDataBody.length = mpDataBodyLen;

	wtfUpdateMarketPriceItemFields(&marketPriceItem);
	ASSERT_TRUE(wtfProviderEncodeMarketPriceDataBody(&mpDataBody, &marketPriceItem, 0) == RSSL_RET_SUCCESS);
	updateMsg.msgBase.encDataBody = mpDataBody;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&updateMsg;
	// Events are expected to be queued here for the initialization of the provider 2 channel.
	wtfSubmitMsg(&opts, WTF_TC_PROVIDER, NULL, RSSL_FALSE, 0);

	/* Expects 2 events: a update message event and PREFERRED_HOST_COMPLETE channel event */
	RsslInt32 k = 0;
	RsslBool updMsgEvent = RSSL_FALSE;
	RsslBool prefHostEvent = RSSL_FALSE;

	if ((pEvent = wtfGetEvent()) == NULL)
	{
		wtfDispatch(WTF_TC_CONSUMER, 100);
		ASSERT_TRUE(pEvent = wtfGetEvent());
	}

	do {
		ASSERT_TRUE(pEvent != NULL);
		WtfEventType eventType = pEvent->base.type;

		if (eventType == WTF_DE_RSSL_MSG && !updMsgEvent)
		{
			updMsgEvent = RSSL_TRUE;
			ASSERT_TRUE(pUpdateMsg = (RsslUpdateMsg*)wtfGetRsslMsg(pEvent));
			ASSERT_TRUE(pUpdateMsg->msgBase.streamId == consumerStreamId);
			ASSERT_TRUE(pUpdateMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
			ASSERT_TRUE(pUpdateMsg->msgBase.containerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.flags == RSSL_MKF_HAS_SERVICE_ID);
			ASSERT_TRUE(pUpdateMsg->msgBase.msgKey.serviceId == service1Id);
			ASSERT_TRUE(rsslBufferIsEqual(&pUpdateMsg->msgBase.encDataBody, &mpDataBody));
		}
		else if (eventType == WTF_DE_CHNL && !prefHostEvent)
		{
			// Get PREFERRED_HOST_COMPLETE
			prefHostEvent = RSSL_TRUE;
			ASSERT_TRUE(pEvent->channelEvent.channelEventType == RSSL_RC_CET_PREFERRED_HOST_COMPLETE);
			ASSERT_NE(pEvent->channelEvent.rsslErrorId, RSSL_RET_SUCCESS) << "Expected an error as a result of the fallback operation";
		}
		else
		{
			// If we are here, so this is an error.
			ASSERT_TRUE(0) << "eventType: " << eventType << "; updMsgEvent: " << (updMsgEvent ? "T" : "F") << "; prefHostEvent: " << (prefHostEvent ? "T" : "F");
		}

		// When the both expected events were received - exit from the loop
		if (updMsgEvent && prefHostEvent)
			break;

		// Waits for the next (2-nd) event
		RsslUInt timeoutMs = 0;

		pEvent = wtfGetEvent();
		while (pEvent == NULL && timeoutMs < 10000)
		{
			wtfDispatch(WTF_TC_CONSUMER, 500);			
			timeoutMs += 500;

			pEvent = wtfGetEvent();
		}
	} while (++k < 2);

	wtfFinishTest();
}

void preferredHost_ChannelList_IOCTL_Fail(PreferredHostTestParameters parameters)
{
	WtfEvent* pEvent;
	RsslReactorSubmitMsgOptions opts;
	WtfSetupConnectionOpts connOpts;
	RsslReactorConnectInfo connectionServer[3];
	RsslBuffer selectServiceName = { 8, const_cast<char*>("service1") };
	RsslRequestMsg	requestMsg, * pRequestMsg;

	RsslBuffer		itemName1 = { 7, const_cast<char*>("ITEM1.N") };
	RsslInt32		consumerStreamId = 5;

	char			mpDataBodyBuf[256];
	RsslBuffer		mpDataBody = { 256, mpDataBodyBuf };
	RsslUInt32		mpDataBodyLen = 256;

	RsslPreferredHostOptions preferredHostOptions;
	RsslReactorChannelInfo channelInfo;
	RsslReactorChannel* consumerChannel;
	RsslErrorInfo		errorInfo;

	if (parameters.multiLoginMsg == RSSL_TRUE)
	{
		cout << "\tSkip testing multi-login." << endl;
		return;
	}

	ASSERT_TRUE(wtfStartTest());
	wtfClearSetupConnectionOpts(&connOpts);
	connOpts.provideDefaultServiceLoad = RSSL_TRUE;

	rsslClearReactorConnectInfo(&connectionServer[0]);
	connectionServer[0].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[0].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14011");
	connectionServer[0].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[0].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[0].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[1]);
	connectionServer[1].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[1].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14012");
	connectionServer[1].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[1].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[1].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	rsslClearReactorConnectInfo(&connectionServer[2]);
	connectionServer[2].rsslConnectOptions.connectionType = RSSL_CONN_TYPE_SOCKET;;
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.address = const_cast<char*>("localhost");
	connectionServer[2].rsslConnectOptions.connectionInfo.unified.serviceName = const_cast<char*>("14013");
	connectionServer[2].rsslConnectOptions.tcpOpts.tcp_nodelay = RSSL_TRUE;
	connectionServer[2].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	connectionServer[2].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;

	connOpts.connectionCount = 3;
	connOpts.reactorConnectionList = connectionServer;

	connOpts.reconnectAttemptLimit = -1;
	connOpts.reconnectMinDelay = 100;
	connOpts.reconnectMaxDelay = 100;

	// Set preferred host to enabled, with fallback to wsb set to true.  This is a singlular WSB group test.
	connOpts.preferredHostOpts.enablePreferredHostOptions = RSSL_TRUE;
	connOpts.preferredHostOpts.fallBackWithInWSBGroup = RSSL_FALSE;
	connOpts.preferredHostOpts.warmStandbyGroupListIndex = 0;
	connOpts.preferredHostOpts.connectionListIndex = 2;

	/* Setup warm standby connections and source directory information. */
	wtfSetupConnectionList(&connOpts, RSSL_CONN_TYPE_SOCKET, RSSL_TRUE, 2);

	/* Request a market price request */
	rsslClearRequestMsg(&requestMsg);
	requestMsg.msgBase.streamId = consumerStreamId;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.flags = RSSL_RQMF_STREAMING;
	requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	requestMsg.msgBase.msgKey.name = itemName1;

	rsslClearReactorSubmitMsgOptions(&opts);
	opts.pRsslMsg = (RsslMsg*)&requestMsg;
	opts.pServiceName = &service1Name;
	wtfSubmitMsg(&opts, WTF_TC_CONSUMER, NULL, RSSL_TRUE);

	/* Provider receives request. */
	wtfDispatch(WTF_TC_PROVIDER, 100, 2);
	ASSERT_TRUE(pEvent = wtfGetEvent());
	ASSERT_TRUE(pRequestMsg = (RsslRequestMsg*)wtfGetRsslMsg(pEvent));
	ASSERT_TRUE(pRequestMsg->msgBase.msgClass == RSSL_MC_REQUEST);
	ASSERT_TRUE(pRequestMsg->msgBase.domainType == RSSL_DMT_MARKET_PRICE);
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY));
	ASSERT_TRUE(!(pRequestMsg->flags & RSSL_RQMF_NO_REFRESH));
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_STREAMING);
	ASSERT_TRUE(pRequestMsg->flags & RSSL_RQMF_HAS_QOS);
	ASSERT_TRUE(pRequestMsg->qos.timeliness == RSSL_QOS_TIME_REALTIME);
	ASSERT_TRUE(pRequestMsg->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK);
	ASSERT_TRUE(pRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME);
	ASSERT_TRUE(pEvent->rsslMsg.serverIndex == 2);
	ASSERT_TRUE(rsslBufferIsEqual(&pRequestMsg->msgBase.msgKey.name, &itemName1));

	consumerChannel = wtfGetChannel(WTF_TC_CONSUMER);

	// Set invalid wsb list index, this should succeed and not change the wsb group.
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 2;
	preferredHostOptions.warmStandbyGroupListIndex = 5;
	preferredHostOptions.fallBackWithInWSBGroup = RSSL_TRUE;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_SUCCESS);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set invalid connection list index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.connectionListIndex = 5;

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	// Set a bad detection time schedule list index
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	preferredHostOptions.detectionTimeSchedule = { 20,  const_cast<char*>("this is a bad string") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);


	// Set a bad too long time schedule list index string
	rsslClearRsslPreferredHostOptions(&preferredHostOptions);
	preferredHostOptions.enablePreferredHostOptions = RSSL_TRUE;
	// Just need to set the length here to more than 255.
	preferredHostOptions.detectionTimeSchedule = { 300, const_cast<char*>("this is a bad string") };

	ASSERT_TRUE(rsslReactorChannelIoctl(consumerChannel, RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS, (void*)&preferredHostOptions, &errorInfo) == RSSL_RET_INVALID_ARGUMENT);

	wtfDispatch(WTF_TC_CONSUMER, 100);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	wtfDispatch(WTF_TC_PROVIDER, 0);
	ASSERT_FALSE(pEvent = wtfGetEvent());

	// Verify that nothing's changed
	ASSERT_TRUE(rsslReactorGetChannelInfo(consumerChannel, &channelInfo, &errorInfo) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isPreferredHostEnabled == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.connectionListIndex == 2);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.warmStandbyGroupListIndex == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.detectionTimeSchedule.length == 0);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.fallBackWithInWSBGroup == RSSL_TRUE);
	ASSERT_TRUE(channelInfo.rsslPreferredHostInfo.isChannelPreferred == RSSL_TRUE);

	wtfFinishTest();
}
