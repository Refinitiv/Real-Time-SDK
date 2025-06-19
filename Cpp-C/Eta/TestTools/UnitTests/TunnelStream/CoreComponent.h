/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_CORE_COMPONENT_H
#define TEST_CORE_COMPONENT_H

class ReadEvent;
class TestReactorEvent;

#include <list>

#if !defined(_WIN32)
#include <sys/select.h>
#endif

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"
#include "rtr/rsslGetTime.h"
#include "ConsumerProviderSessionOptions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A component that uses ETA directly, instead of a Reactor. */
class CoreComponent
{
private:
	fd_set readFds, writeFds, exceptFds;
	RsslErrorInfo _errorInfo;
	RsslServer* _pServer;
	RsslReadInArgs _readInArgs;
	RsslReadOutArgs _readOutArgs;
	RsslInProgInfo _inProg;
	std::list<ReadEvent *> _eventQueue;
	RsslInt _maxFragmentSize;
	RsslWriteInArgs _writeInArgs;
	RsslWriteOutArgs _writeOutArgs;
	RsslChannelInfo _channelInfo;
	
	RsslInt _defaultSessionLoginStreamId;
	bool _defaultSessionLoginStreamIdIsSet;
	RsslInt _defaultSessionDirectoryStreamId;
	bool _defaultSessionDirectoryStreamIdIsSet;
	
	RsslInt _portToBind; /* A port to use when binding servers. Incremented with each bind. */
	char _portToBindString[128];

	/* Helper to send a domain rep message or Codec pMsg to a channel. */
	void submit(RsslRDMMsgBase* pMsgBase, RsslMsg* pMsg);

protected:
	RsslChannel* _pChannel;
	RsslError _error;
	RsslEncodeIterator _eIter;
	RsslDecodeIterator _dIter;

public:
	CoreComponent();
	
	/* Returns component's server, if any. */
	RsslServer* server();

	/* Returns the port of the component's server. */
	RsslInt serverPort();

	/* Closes the component's channel. */
	void closeChannel();
	
	/* Closes a component. */
	void close();
	
	/* Sets up a server. */
	void bind(ConsumerProviderSessionOptions* pOpts);

	/* Accepts a channel. Used by acceptAndInitChannel. Can be run alone to
	 * avoid initializing a channel. */
	void accept(RsslInt connectionType);
	
	/* Accepts and initializes a RsslChannel.
	 * This is intended to be run against a Reactor-based client, where initialization
	 * is performed by an internal thread, so we shouldn't need to do anything with the
	 * other side to get an active channel. */
	void acceptAndInitChannel(RsslInt connectionType);
	
	/* Waits for notification on the component's channel, and reads when triggered. It will 
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 */
	void dispatch(RsslUInt expectedEventCount);
	
	/* Waits for notification on the component's channel, and reads when triggered. It will 
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 */
	void dispatch(RsslUInt expectedEventCount, RsslUInt timeoutMsec);
	
	/* Submit a Codec RsslMsg to the channel. */
	void submit(RsslMsg* pMsg);
	
	/* Submit a Domain Representation message to the channel. */
	void submit(RsslRDMMsgBase* pMsgBase);
		
	void writeBuffer(RsslBuffer* pBuffer);
	
	/* Retrieves an event from the list of events received from a dispatch call. */
	ReadEvent* pollEvent();
	
	/* Stores the directory stream ID for this component. Used if a directory stream is automatically setup as part of opening a session. */
	void defaultSessionDirectoryStreamId(RsslInt defaultSessionDirectoryStreamId);

	/* If a directory stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
	RsslInt defaultSessionDirectoryStreamId();
};

#ifdef __cplusplus
};
#endif

#endif
