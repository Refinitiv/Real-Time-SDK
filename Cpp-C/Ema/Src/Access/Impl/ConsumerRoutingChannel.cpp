/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ConsumerRoutingChannel.h"
#include "ConsumerRoutingSession.h"



using namespace refinitiv::ema::access;
 
size_t ConsumerRoutingSessionChannel::UInt16rHasher::operator()(const UInt16& value) const
{
	return value;
}

bool ConsumerRoutingSessionChannel::UInt16Equal_To::operator()(const UInt16& x, const UInt16& y) const
{
	return x == y ? true : false;
}

size_t ConsumerRoutingSessionChannel::EmaStringPtrHasher::operator()(const EmaStringPtr& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)*s++;
	return result;
}

bool ConsumerRoutingSessionChannel::EmaStringPtrEqual_To::operator()(const EmaStringPtr& x, const EmaStringPtr& y) const
{
	return *x == *y;
}

void ConsumerRoutingSessionChannelConfig::clearChannelSet()
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

void ConsumerRoutingSessionChannelConfig::clearWSBChannelSet()
{
	if (configWarmStandbySet.size() == 0)
		return;
	for (unsigned int i = 0; i < configWarmStandbySet.size(); ++i)
	{
		if (configWarmStandbySet[i] != NULL)
		{
			delete configWarmStandbySet[i];
			configWarmStandbySet[i] = NULL;
		}
	}

	configWarmStandbySet.clear();
}

void ConsumerRoutingSessionChannelConfig::clearChannelSetForWSB()
{
	if (configChannelSetForWSB.size() == 0)
		return;
	for (unsigned int i = 0; i < configChannelSetForWSB.size(); ++i)
	{
		if (configChannelSetForWSB[i] != NULL)
		{
			delete configChannelSetForWSB[i];
			configChannelSetForWSB[i] = NULL;
		}
	}

	configChannelSetForWSB.clear();
}

void ConsumerRoutingSessionChannelConfig::clearReactorChannelConnectOpts()
{
	// The memory that this points to will be from the active config cache, so we do not need to clean each one individually
	if (connectOpts.reactorConnectionList != NULL)
		delete[] connectOpts.reactorConnectionList;

	if (connectOpts.reactorWarmStandbyGroupList != NULL)
	{
		for (UInt32 i = 0; i < connectOpts.warmStandbyGroupCount; ++i)
		{
			RsslReactorWarmStandbyGroup* tmpGroup = &connectOpts.reactorWarmStandbyGroupList[i];

			// The memory that this points to will be from the active config cache, so we do not need to clean each one individually
			if (tmpGroup->startingActiveServer.perServiceBasedOptions.serviceNameList != NULL)
			{
				delete[] tmpGroup->startingActiveServer.perServiceBasedOptions.serviceNameList;
			}

			if (tmpGroup->standbyServerList != NULL)
			{
				for (UInt32 j = 0; j < tmpGroup->standbyServerCount; j++)
				{
					RsslReactorWarmStandbyServerInfo* tmpServerInfo = &tmpGroup->standbyServerList[j];

					if (tmpServerInfo->perServiceBasedOptions.serviceNameList != NULL)
					{
						delete[] tmpServerInfo->perServiceBasedOptions.serviceNameList;
					}
				}
				delete[] tmpGroup->standbyServerList;
			}
		}

		delete[] connectOpts.reactorWarmStandbyGroupList;
	}

	rsslClearReactorConnectOptions(&connectOpts);
}

// Default values are taken directly from ActiveConfig.h
ConsumerRoutingSessionChannelConfig::ConsumerRoutingSessionChannelConfig(const EmaString& channelName, ActiveConfig& activeConfig) :
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
	xmlTraceFileName(activeConfig.xmlTraceFileName),
	enablePreferredHostOptions(false),
	phDetectionTimeSchedule(activeConfig.phDetectionTimeSchedule),
	phDetectionTimeInterval(activeConfig.phDetectionTimeInterval),
	preferredChannelName(),
	preferredWSBChannelName(),
	phFallBackWithInWSBGroup(activeConfig.phFallBackWithInWSBGroup)
{
	pRoutingChannel = NULL;

	rsslClearReactorConnectOptions(&connectOpts);

	useActiveConfigLogger = true;
}

ConsumerRoutingSessionChannelConfig::~ConsumerRoutingSessionChannelConfig()
{
	clear();
}

void ConsumerRoutingSessionChannelConfig::clear()
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

	enablePreferredHostOptions = false;
	phDetectionTimeSchedule.clear();
	phDetectionTimeInterval = 0;
	preferredChannelName.clear();
	preferredWSBChannelName.clear();
	phFallBackWithInWSBGroup = false;

	rsslClearReactorConnectOptions(&connectOpts);

	clearChannelSet();
	clearWSBChannelSet();
	clearChannelSetForWSB();
	clearReactorChannelConnectOpts();
}

ConsumerRoutingSessionChannel::ConsumerRoutingSessionChannel(OmmBaseImpl& consumerBaseImpl, const EmaString& sessionChannelName, ConsumerRoutingSessionChannelConfig& sessionChannelConfig) :
	name(sessionChannelName),
	baseImpl(consumerBaseImpl),
	routingChannelConfig(sessionChannelConfig),
	channelList(),
	serviceById(),
	serviceByName(),
	serviceList(),
	pLoggerClient(0),
	pReactorChannel(0),
	pRoutingSession(consumerBaseImpl.getConsumerRoutingSession()),
	pCurrentActiveChannel(NULL),
	loginInfo(),
	routedRequestList(consumerBaseImpl),
	inPreferredHost(false),
	sentChannelUpStatus(false),
	closeOnDownReconnecting(false),
	sessionIndex(0)
{
	reconnecting = false;
	channelClosed = false;
	receivedLoginRefresh = false;
	channelState = OmmBaseImpl::NotInitializedEnum;
}

ConsumerRoutingSessionChannel::~ConsumerRoutingSessionChannel()
{
	clear();
}

void ConsumerRoutingSessionChannel::clear()
{
	receivedLoginRefresh = false;
	inPreferredHost = false;
	closeOnDownReconnecting = false;
	pReactorChannel = NULL;

	pRoutingSession = NULL;

	serviceById.clear();
	serviceByName.clear();

	channelList.removeAllChannel();

	Directory* pDirectory = serviceList.pop_front();
	while(pDirectory != NULL)
	{
		Directory::destroy(pDirectory);
		pDirectory = serviceList.pop_front();
	}

	serviceList.clear();
}

void ConsumerRoutingSessionChannel::closeReactorChannel()
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

		pReactorChannel = NULL;
		pCurrentActiveChannel = NULL;
		channelClosed = true;
	}
}