/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef TEST_CONSUMER_PROVIDER_SESSION_OPTIONS_H
#define TEST_CONSUMER_PROVIDER_SESSION_OPTIONS_H

#include "rtr/rsslReactor.h"
#include "rtr/rsslTunnelStream.h"

#ifdef __cplusplus
extern "C" {
#endif

class ConsumerProviderSessionOptions
{
private:
	RsslInt _connectionType;
	RsslInt32 _reconnectAttemptLimit;
	RsslInt32 _reconnectMinDelay;
	RsslInt32 _reconnectMaxDelay;
	bool _setupDefaultLoginStream;
	bool _setupDefaultDirectoryStream;
	RsslInt _numStatusEvents;
	RsslInt32 _pingTimeout;
	RsslUInt32 _consumerChannelInitTimeout;

public:
	ConsumerProviderSessionOptions()
	{
		_connectionType = 0;
		_reconnectAttemptLimit = 0;
		_reconnectMinDelay = 1000;
		_reconnectMaxDelay = 1000;
		_setupDefaultLoginStream = false;
		_setupDefaultDirectoryStream = false;
		_numStatusEvents = 0;
		_pingTimeout = 60;
		_consumerChannelInitTimeout = 60;
	}

	/* Returns the type of connection the session will use. */
	RsslInt connectionType() { return _connectionType; }
	
	/* Sets the type of connection the session will use. */
	void connectionType(RsslInt connectionType) { _connectionType = connectionType; }
	
	/* Sets whether a default login stream will be setup.
	 * If set to true, the consumer's reactor role must have a preset login request. */
	void setupDefaultLoginStream(bool setupDefaultLoginStream)
	{
		_setupDefaultLoginStream = setupDefaultLoginStream;
	}
	
	/* Returns whether a default login stream will be setup. */
	bool setupDefaultLoginStream()
	{
		return _setupDefaultLoginStream;
	}
		
	/* Sets whether a default directory stream will be setup.
	 * If set to true, either the consumer's reactor role must have a preset directory request,
	 * or its watchlist is enabled (or both). */
	void setupDefaultDirectoryStream(bool setupDefaultDirectoryStream)
	{
		_setupDefaultDirectoryStream = setupDefaultDirectoryStream;
	}
	
	/* Returns whether a default directory stream will be setup. */
	bool setupDefaultDirectoryStream()
	{
		return _setupDefaultDirectoryStream;
	}

	/* Returns the reconnectAttemptLimit. */
	RsslInt32 reconnectAttemptLimit()
	{
		return _reconnectAttemptLimit;
	}

	/* Sets the reconnectAttemptLimit. */
	void reconnectAttemptLimit(RsslInt32 reconnectAttemptLimit)
	{
		_reconnectAttemptLimit = reconnectAttemptLimit;
	}
	
	/* Returns the reconnectMinDelay. */
	RsslInt32 reconnectMinDelay()
	{
		return _reconnectMinDelay;
	}

	/* Sets the reconnectMinDelay. */
	void reconnectMinDelay(RsslInt32 reconnectMinDelay)
	{
		_reconnectMinDelay = reconnectMinDelay;
	}
	
	/* Returns the reconnectMaxDelay. */
	RsslInt32 reconnectMaxDelay()
	{
		return _reconnectMaxDelay;
	}

	/* Sets the reconnectMaxDelay. */
	void reconnectMaxDelay(RsslInt32 reconnectMaxDelay)
	{
		_reconnectMaxDelay = reconnectMaxDelay;
	}
	
	/* Returns the pingTimeout the consumer and provider will use. */
	RsslInt32 pingTimeout()
	{
		return _pingTimeout;
	}

	/* Sets the pingTimeout the consumer and provider will use. */
	void pingTimeout(RsslInt32 pingTimeout)
	{
		_pingTimeout = pingTimeout;
	}

	/* Returns the consumer channel's initialization timeout. */
	RsslUInt32 consumerChannelInitTimeout()
	{
		return _consumerChannelInitTimeout;
	}

	/* Sets the consumer channel's initializationTimeout. */
	void consumerChannelInitTimeout(RsslUInt32 consumerChannelInitTimeout)
	{
		_consumerChannelInitTimeout = consumerChannelInitTimeout;
	}

	/* Returns the number of status events received before the CHANNEL_READY event. Used for watchlist channel open callback submit status messages. */
	RsslInt numStatusEvents()
	{
		return _numStatusEvents;
	}
	
	/* Sets the number of status events received before the CHANNEL_READY event. Used for watchlist channel open callback submit status messages. */
	void numStatusEvents(RsslInt numStatusEvents)
	{
		_numStatusEvents = numStatusEvents;
	}
};

#ifdef __cplusplus
};
#endif

#endif
