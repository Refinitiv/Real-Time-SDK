/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_REACTOR_COMPONENT_H
#define TEST_REACTOR_COMPONENT_H

class TestReactor;

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "TestReactorEvent.h"
#include "TestReactor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A component represents a consumer, provider, etc. on the network (note the Consumer and Provider subclasses). */
class TestReactorComponent
{
private:
	/* Reactor channel associated with this component, if connected. */
	RsslReactorChannel* _pReactorChannel;
	
	/* Whether the reactor channel associated with this component is up. */
	bool _reactorChannelIsUp;
	
	/* Server associated with this component, if any. */
	RsslServer* _pServer;
	
	RsslInt _defaultSessionLoginStreamId;
	bool _defaultSessionLoginStreamIdIsSet;
	RsslInt _defaultSessionDirectoryStreamId;
	bool _defaultSessionDirectoryStreamIdIsSet;
	
	RsslInt _portToBind; /* A port to use when binding servers. Incremented with each bind. */
	char _portToBindString[128];

protected:
	/* Reusable ReactorErrorInfo. */
	RsslErrorInfo _errorInfo;
	
	/* ReactorRole associated with this component. */
	RsslReactorChannelRole* _pReactorRole;
	
	/* Reactor associated with this component. */
	TestReactor* _pTestReactor;
	
public:
	/* Returns the port the next bind call will use. Useful for testing reconnection
	 * to servers that won't be bound until later in a test. */
	RsslInt nextServerPort();

	/* Returns the port of the component's server. */
	RsslInt serverPort();
	
	TestReactor* testReactor();
	
	void testReactor(TestReactor* pTestReactor);
	
	RsslReactorChannel* channel();
		
	TestReactorComponent(TestReactor* pTestReactor);

	RsslReactorChannelRole* reactorRole();
	
	RsslReactorChannel* reactorChannel();
	
	void reactorChannel(RsslReactorChannel* pReactorChannel);
	
	bool reactorChannelIsUp();
	
	void reactorChannelIsUp(bool reactorChannelIsUp);
	
	/* Stores the login stream ID for this component. Used if a login stream is automatically setup as part of opening a session. */
	void defaultSessionLoginStreamId(RsslInt defaultSessionLoginStreamId);
	
	/* If a login stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
	RsslInt defaultSessionLoginStreamId();
	
	/* Stores the directory stream ID for this component. Used if a directory stream is automatically setup as part of opening a session. */
	void defaultSessionDirectoryStreamId(RsslInt defaultSessionDirectoryStreamId);

	/* If a directory stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
	RsslInt defaultSessionDirectoryStreamId();
	
	RsslServer* server();
	
	void bind(ConsumerProviderSessionOptions* pOpts);
	
	/* Sends a Msg to the component's channel. */
	RsslInt submit(RsslReactorSubmitMsgOptions* pSubmitOptions);
	
	/* Sends a Msg to the component's channel, and dispatches to ensure no events are received and any internal flush events are processed. */
	RsslInt submitAndDispatch(RsslReactorSubmitMsgOptions* pSubmitOptions);
	
	/* Disconnect a consumer and provider component and clean them up. */
	static void closeSession(Consumer* pConsumer, Provider* pProvider);

	/* Closes the component's channel. */
	void closeChannel();
	
	/* Close a component and remove it from its associated TestReactor. 
	 * Closes any associated server and reactor channel. */
	void close();
};

#ifdef __cplusplus
};
#endif

#endif
