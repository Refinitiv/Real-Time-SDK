/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "BaseRoutingChannel.h"
#include "BaseRoutingSession.h"



using namespace refinitiv::ema::access;
 
size_t BaseRoutingSessionChannel::UInt16rHasher::operator()(const UInt16& value) const
{
	return value;
}

bool BaseRoutingSessionChannel::UInt16Equal_To::operator()(const UInt16& x, const UInt16& y) const
{
	return x == y ? true : false;
}

size_t BaseRoutingSessionChannel::EmaStringPtrHasher::operator()(const EmaStringPtr& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)*s++;
	return result;
}

bool BaseRoutingSessionChannel::EmaStringPtrEqual_To::operator()(const EmaStringPtr& x, const EmaStringPtr& y) const
{
	return *x == *y;
}

void BaseRoutingSessionChannelConfig::clearChannelSet()
{
	if (configChannelSet.size() == 0)
		return;
	for (unsigned int i = 0; i < configChannelSet.size(); ++i)
	{
		if (configChannelSet[i] != NULL)
		{
			delete configChannelSet[i];
			configChannelSet[i] = NULL;
		}
	}

	configChannelSet.clear();
}


void BaseRoutingSessionChannelConfig::clearReactorChannelConnectOpts()
{
	// The memory that this points to will be from the active config cache, so we do not need to clean each one individually
	if (connectOpts.reactorConnectionList != NULL)
		delete[] connectOpts.reactorConnectionList;

	rsslClearReactorConnectOptions(&connectOpts);
}

// Default values are taken directly from ActiveConfig.h
BaseRoutingSessionChannelConfig::BaseRoutingSessionChannelConfig(const EmaString& channelName, ActiveConfig& activeConfig) :
	name(channelName),
	activeConfig(activeConfig),
	loggerConfig(activeConfig.loggerConfig),
	reconnectAttemptLimit(activeConfig.reconnectAttemptLimit),
	reconnectMinDelay(activeConfig.reconnectMinDelay),
	reconnectMaxDelay(activeConfig.reconnectMaxDelay),
	xmlTraceMaxFileSize(activeConfig.xmlTraceMaxFileSize),
	xmlTraceToFile(activeConfig.xmlTraceToFile),
	xmlTraceToStdout(activeConfig.xmlTraceToStdout),
	xmlTraceToMultipleFiles(activeConfig.xmlTraceToMultipleFiles),
	xmlTraceWrite(activeConfig.xmlTraceWrite),
	xmlTraceRead(activeConfig.xmlTraceRead),
	xmlTracePing(activeConfig.xmlTracePing),
	xmlTracePingOnly(activeConfig.xmlTracePingOnly),
	xmlTraceHex(activeConfig.xmlTraceHex),
	xmlTraceDump(activeConfig.xmlTraceDump),
	xmlTraceFileName(activeConfig.xmlTraceFileName)
{
	pRoutingChannel = NULL;

	rsslClearReactorConnectOptions(&connectOpts);

	useActiveConfigLogger = true;
}

BaseRoutingSessionChannelConfig::~BaseRoutingSessionChannelConfig()
{
	clear();
}

void BaseRoutingSessionChannelConfig::clear()
{
	reconnectAttemptLimit = DEFAULT_RECONNECT_ATTEMPT_LIMIT;
	reconnectMinDelay = DEFAULT_RECONNECT_MIN_DELAY;
	reconnectMaxDelay = DEFAULT_RECONNECT_MAX_DELAY;
	xmlTraceMaxFileSize = DEFAULT_XML_TRACE_MAX_FILE_SIZE;
	xmlTraceToFile = DEFAULT_XML_TRACE_TO_FILE;
	xmlTraceToStdout = DEFAULT_XML_TRACE_TO_STDOUT;
	xmlTraceToMultipleFiles = DEFAULT_XML_TRACE_TO_MULTIPLE_FILE;
	xmlTraceWrite = DEFAULT_XML_TRACE_WRITE;
	xmlTraceRead = DEFAULT_XML_TRACE_READ;
	xmlTracePing = DEFAULT_XML_TRACE_PING;
	xmlTracePingOnly = DEFAULT_XML_TRACE_PING_ONLY;
	xmlTraceHex = DEFAULT_XML_TRACE_HEX;
	xmlTraceDump = DEFAULT_XML_TRACE_DUMP;
	xmlTraceFileName = DEFAULT_XML_TRACE_FILE_NAME;
	pRoutingChannel = NULL;
	loggerConfig.clear();

	useActiveConfigLogger = true;

	if (connectOpts.reactorConnectionList != NULL)
	{
		delete[] connectOpts.reactorConnectionList;
	}


	rsslClearReactorConnectOptions(&connectOpts);

	clearChannelSet();
	clearReactorChannelConnectOpts();
}

BaseRoutingSessionChannel::BaseRoutingSessionChannel(OmmBaseImpl& baseImpl, const EmaString& sessionChannelName, BaseRoutingSessionChannelConfig& sessionChannelConfig) :
	name(sessionChannelName),
	baseImpl(baseImpl),
	channelList(),
	pLoggerClient(0),
	pReactorChannel(0),
	pCurrentActiveChannel(NULL),
	loginInfo(),
	inPreferredHost(false),
	sentChannelUpStatus(false),
	closeOnDownReconnecting(false),
	sessionIndex(0),
	routingChannelConfig(sessionChannelConfig),
	pRoutingSession(baseImpl.getRoutingSession())
{
	reconnecting = false;
	channelClosed = false;
	receivedLoginRefresh = false;
	channelState = OmmBaseImpl::NotInitializedEnum;
}

BaseRoutingSessionChannel::~BaseRoutingSessionChannel()
{
	clear();
}

void BaseRoutingSessionChannel::clear()
{
	receivedLoginRefresh = false;
	inPreferredHost = false;
	closeOnDownReconnecting = false;
	pReactorChannel = NULL;

	channelList.removeAllChannel();
}

void BaseRoutingSessionChannel::closeReactorChannel()
{
	RsslErrorInfo errorInfo;
	if (pReactorChannel)
	{
		if (rsslReactorCloseChannel(baseImpl._pRsslReactor, pReactorChannel, &errorInfo) != RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= baseImpl._activeConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to close reactor channel (rsslReactorCloseChannel).");
				temp.append("' RsslChannel='").append((UInt64)errorInfo.rsslError.channel)
					.append("' Error Id='").append(errorInfo.rsslError.rsslErrorId)
					.append("' Internal sysError='").append(errorInfo.rsslError.sysError)
					.append("' Error Location='").append(errorInfo.errorLocation)
					.append("' Error Text='").append(errorInfo.rsslError.text).append("'. ");

				baseImpl._userLock.lock();

				if (baseImpl._pLoggerClient) baseImpl._pLoggerClient->log(baseImpl._activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);

				baseImpl._userLock.unlock();
			}
		}
		
		channelState = OmmBaseImpl::RsslChannelDownEnum;
		pReactorChannel = NULL;
		pCurrentActiveChannel = NULL;
		channelClosed = true;
	}
}