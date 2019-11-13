/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

/*
 * This class represents a single Reactor.
 * It providers simple ways to connect components (such as a consumer and provider)
 * and dispatch for events. The dispatched events are copied (including any underlying data such as 
 * messages) and stored into an event queue. You can then retrieve those events and test that they are 
 * correct.
 */
#ifndef TEST_REACTOR_H
#define TEST_REACTOR_H

#include <list>

class TestReactorComponent;
class Consumer;
class Provider;

#if !defined(_WIN32)
#include <sys/select.h>
#endif

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "TestReactorEvent.h"
#include "ConsumerProviderSessionOptions.h"
#include "TestReactorComponent.h"

#ifdef __cplusplus
extern "C" {
#endif

class TestReactor
{
private:
	/* The associated reactor. */
	RsslReactor* _pReactor;
	
	/* Queue of events received from calling dispatch. */
	std::list<TestReactorEvent *> _eventQueue;
	
	/* List of components associated with this reactor. */
	std::list<TestReactorComponent *> _componentList;
	
	/* Reusable ReactorErrorInfo. */
	RsslErrorInfo _errorInfo;
	
	/* Fd sets used when dispatching. */
	fd_set readFds;
	fd_set exceptFds;
	
	/* Controls whether reactor does XML tracing. */
	bool _enableReactorXmlTracing;

public:
	/* Creates a TestReactor. */
	TestReactor();

	RsslReactor* reactor();

	/* Enables XML tracing on created reactors (convenience function for test debugging). */
	void enableReactorXmlTracing();

	/* Calls dispatch on the component's Reactor, and will store received events for later review.
	 * The test will verify that the exact number of events is received, and will fail if fewer
	 * or more are received.
	 * @param expectedEventCount The exact number of events that should be received.
	 */
	void dispatch(RsslInt expectedEventCount);
	
	/* Waits for notification on the component's Reactor, and calls dispatch when triggered. It will 
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 */
	void dispatch(RsslInt expectedEventCount, RsslUInt timeoutMsec);
	
	/* Stores the received channel event, and updates the relevant component's channel information. */ 
	RsslInt handleChannelEvent(RsslReactorChannelEvent* pEvent);
	
	/* Stores a login message event. */
	RsslInt handleLoginMsgEvent(RsslRDMLoginMsgEvent* pEvent);
	
	/* Stores a directory message event. */
	RsslInt handleDirectoryMsgEvent(RsslRDMDirectoryMsgEvent* pEvent);

	/* Stores a dictionary message event. */
	RsslInt handleDictionaryMsgEvent(RsslRDMDictionaryMsgEvent* pEvent);
	
	/* Stores a message event. */
	RsslInt handleDefaultMsgEvent(RsslMsgEvent* pEvent);
	
	/* Stores a tunnel stream message event. */
	RsslInt handleTunnelStreamMsgEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pEvent);
	
	/* Stores a tunnel stream status event. */
	RsslInt handleTunnelStreamStatusEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pEvent);
 
	/* Stores a tunnel stream request event. */
	RsslInt handleTunnelStreamRequestEvent(RsslTunnelStream* pTunnelStream, RsslTunnelStreamRequestEvent* pEvent);
	
	/* Retrieves an event from the list of events received from a dispatch call. */
	TestReactorEvent* pollEvent();
	
	/* Adds a component to the Reactor. */
	void addComponent(TestReactorComponent* pComponent);
	
	/* Adds a component's server to the selector. */
	void registerComponentServer(TestReactorComponent* pComponent);

	/* Removes a component from the Reactor. */
	void removeComponent(TestReactorComponent* pComponent);
	
	/* Associates a component with this reactor and opens a connection. */
	void connect(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent, RsslInt port);
	
	/* Associates a component with this reactor and accepts a connection. */
	void accept(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent);
	
	/* Associates a component with this reactor and accepts a connection. */
	void accept(ConsumerProviderSessionOptions* pOpts, TestReactorComponent* pComponent, RsslUInt timeoutMsec);
	
	/* Connects a Consumer and Provider component to each other. */
	static void openSession(Consumer* pConsumer, Provider* pProvider, ConsumerProviderSessionOptions* pOpts, bool recoveringChannel = false);

	/* Cleans up the TestReactor's resources (e.g. its reactor) */
	void close();
};

#ifdef __cplusplus
};
#endif

#endif
