/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#pragma once

#include "TestUtilities.h"
#include "ActiveConfig.h"
#include "Ema.h"
#include "Login.h"
#include "rtr/rsslThread.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "Thread.h"

#ifndef __EmaTestClients_h_
#define __EmaTestClients_h_

using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::domain::login;

// Test option classes:
// All bool options: true means it's on, false means it's off
class ConsumerTestOptions
{
public:
	ConsumerTestOptions();

	bool getChannelInformation;
	bool getSessionChannelInfo;
};

// Map directory payload is used by the Provider test client to send directory payloads.
class ProviderTestOptions
{
public:
	ProviderTestOptions();

	virtual ~ProviderTestOptions();
	
	Map* directoryPayload;

	bool sendLoginRefresh;
	bool acceptLoginRequest;
	bool sendDirectoryRefresh;
	bool sendDictionaryRefresh;
	bool sendRefreshAttrib;
	bool sendUpdateMsg;
	bool sendGenericMsg;
	bool closeItemRequest;
	bool sendItemRefresh;
	bool supportWSB;
	EmaBuffer groupId;
};

class RequestAttributes
{
public:
	UInt64 handle;
	EmaString name;
	UInt32 serviceId;

	RequestAttributes();
	virtual ~RequestAttributes();
};

// This class will log and handle any messages/channelinfo/session channel info in the callbacks.
// Message pointers will be added to the message queues and message lists
// All "list" named EmaVectors are used for cleaning up the new'd messages/channel info/EmaVector<chanelinfo> objects
// All "queue" are used as a queue to pull the messages off of in the order that they were added.
class EmaTestClientBase
{
public:
	EmaTestClientBase();
	virtual ~EmaTestClientBase();

	void clear();

	UInt32 getMessageQueueSize();
	Msg* popMsg();

	void addMsg(ReqMsg&);
	void addMsg(RefreshMsg&);
	void addMsg(UpdateMsg&);
	void addMsg(GenericMsg&);
	void addMsg(PostMsg&);
	void addMsg(AckMsg&);

	UInt32 getChannelInfoQueueSize();
	ChannelInformation* popChannelInfo();

	UInt32 getSessionChannelInfoSize();
	EmaVector<ChannelInformation>* popSessionChannelInfo();

	UInt32 getHandleSize();
	void addHandle(UInt64);
	UInt64 popHandle();


private:
	friend class IProviderTestClientBase;
	friend class ConsumerTestClientBase;
	friend class NiProviderTestClientBase;

	EmaVector<Msg*> _messageQueue;					// The Queues are used to pull off data.  The lists are used for cleanup.
	EmaVector<Msg*> _messageList;
	EmaVector<ChannelInformation*> _channelInfoQueue;
	EmaVector<ChannelInformation*> _channelInfoList;
	EmaVector<EmaVector<ChannelInformation>*> _sessionChannelInfoQueue;
	EmaVector<EmaVector<ChannelInformation>*> _sessionChannelInfoList;
	EmaVector<UInt64> _handleVector;
	RsslMutex _poolLock;
};

// This defines the basic behavior for an interactive provider for testing
// This class base supports:
// Login, Dictionary, and Directory requests(directory payload is specified in ProviderTestOptions
// Warm standby support, with indications if this provider is an active or standby
// 
class IProviderTestClientBase : public refinitiv::ema::access::OmmProviderClient, public EmaTestClientBase
{
public:
	OmmProvider* ommProvider;
	ProviderTestOptions options;

	UInt32 serviceId = 0;

	EmaString loginUserName;
	UInt64 loginHandle;
	UInt64 clientHandle;
	UInt64 directoryHandle;
	ElementList loginRefreshAttributes;
	UInt64 wsbActiveState;

	EmaVector<RequestAttributes> activeRequests;

	DataDictionary dictionary;

	IProviderTestClientBase(ProviderTestOptions&);

	~IProviderTestClientBase();

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event);

	void onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event);

	void onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event);

	void onAllMsg(const Msg& msg, const OmmProviderEvent& event);

	void onPostMsg(const PostMsg& postMsg, const OmmProviderEvent& event);

	void onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event);

	void onReissue(const ReqMsg& reqMsg, const OmmProviderEvent& event);

	void onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event);

};

// Basic consumer test.  This class will log all messages into the EmaTestClientBase structures.
class ConsumerTestClientBase : public refinitiv::ema::access::OmmConsumerClient, public EmaTestClientBase
{
public:
	OmmConsumer* ommConsumer;
	ConsumerTestOptions options;

	ConsumerTestClientBase(ConsumerTestOptions&);

	~ConsumerTestClientBase();

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& consumerEvent);

	void onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& consumerEvent);

	void onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& consumerEvent);

	void onGenericMsg(const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent);

	void onAckMsg(const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent);

	void onAllMsg(const Msg& msg, const OmmConsumerEvent& consumerEvent);

	void setConsumer(OmmConsumer*);

	void unregisterHandles();

};

class NiProviderTestClientBase : public refinitiv::ema::access::OmmProviderClient, public EmaTestClientBase
{
public:
	OmmProvider* ommProvider;
	ProviderTestOptions options;

	EmaString loginUserName;

	NiProviderTestClientBase( ProviderTestOptions& );

	~NiProviderTestClientBase() = default;

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event);

	void onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event);

	void onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event);

	void onAllMsg(const Msg& msg, const OmmProviderEvent& event);

	void onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event);
};

// Test client for OAuth2 credential renewal interface
class OAuthClientTest : public refinitiv::ema::access::OmmOAuth2ConsumerClient
{
protected:

	void onCredentialRenewal( const refinitiv::ema::access::OmmConsumerEvent& consumerEvent ) override;
};

const unsigned BaseStartPort = 14120U;

class ConsumerProgrammaticTestConfig
{
public:
	UInt64							loginRequestTimeOut;
	UInt64							channelInitializationTimeout;
	unsigned						numChannels;
	unsigned						startPortNum;
	OmmLoggerClient::Severity		loggerSeverity;

	ConsumerProgrammaticTestConfig() :
		loginRequestTimeOut(45000),
		channelInitializationTimeout(5),
		numChannels(1),
		startPortNum(BaseStartPort),
		loggerSeverity(OmmLoggerClient::ErrorEnum)
	{
	}

	void createProgrammaticConfig( Map& configMap );
};  // class ConsumerProgrammaticTestConfig

class IProviderProgrammaticTestConfig
{
public:
	unsigned						numProviders;		// Specify number of providers to configure
	unsigned						startPortNum;		// Starting port number; each provider will get its own port number
	OmmLoggerClient::Severity		loggerSeverity;		// Logger severity level

	IProviderProgrammaticTestConfig() :
		numProviders(1),
		startPortNum(BaseStartPort),
		loggerSeverity(OmmLoggerClient::ErrorEnum)
	{
	}

	void createProgrammaticConfig( Map& configMap );
};  // class IProviderProgrammaticTestConfig

class NiProviderProgrammaticTestConfig
{
public:
	UInt64							loginRequestTimeOut;
	UInt64							channelInitializationTimeout;
	unsigned						numChannels;
	unsigned						startPortNum;		// Starting port number: connects to this port number
	OmmLoggerClient::Severity		loggerSeverity;		// Logger severity level
	bool							enableCompression;

	NiProviderProgrammaticTestConfig() :
		loginRequestTimeOut(45000),
		channelInitializationTimeout(5),
		numChannels(1),
		startPortNum(BaseStartPort),
		loggerSeverity(OmmLoggerClient::ErrorEnum),
		enableCompression(false)
	{
	}

	void createProgrammaticConfig( Map& configMap );
};  // class NiProviderProgrammaticTestConfig

class ProviderTestComponents {
public:
	ProviderTestOptions provTestOptions;
	IProviderTestClientBase* pProvClient;

	OmmIProviderConfig* pProvConfig;

	OmmProvider* pProvider;

	ProviderTestComponents() :
		pProvClient(nullptr),
		pProvConfig(nullptr),
		pProvider(nullptr)
	{
	}

	ProviderTestComponents(ProviderTestComponents* pComponents) :
		pProvClient(pComponents->pProvClient),
		pProvConfig(pComponents->pProvConfig),
		pProvider(pComponents->pProvider)
	{
	}

	ProviderTestComponents(ProviderTestComponents& components) :
		pProvClient(components.pProvClient),
		pProvConfig(components.pProvConfig),
		pProvider(components.pProvider)
	{
	}

	~ProviderTestComponents() {
		//cout << "ProviderTestComponents::~ProviderTestComponents()" << endl;
		if (pProvider != nullptr)
		{
			delete pProvider;
			pProvider = nullptr;
		}
		if (pProvConfig != nullptr)
		{
			delete pProvConfig;
			pProvConfig = nullptr;
		}
		if (pProvClient != nullptr)
		{
			delete pProvClient;
			pProvClient = nullptr;
		}
	}

	OmmProvider* createProvider(const Data& configIProvMap, const char* providerName)
	{
		pProvClient = new IProviderTestClientBase(provTestOptions);

		pProvConfig = new OmmIProviderConfig("EmaConfigTest.xml");
		pProvConfig->config(configIProvMap);
		pProvConfig->providerName(providerName);

		/* Create IProvider instance */
		pProvider = new OmmProvider(*pProvConfig, *pProvClient);
		return pProvider;
	}
};  // class ProviderTestComponents

class IProviderTestClientForGeneric : public IProviderTestClientBase
{
public:
	UInt64 genericItemHandle;

	IProviderTestClientForGeneric(ProviderTestOptions& providertTestOpts) :
		IProviderTestClientBase(providertTestOpts),
		genericItemHandle(0)
	{
	}

	virtual ~IProviderTestClientForGeneric()
	{
	}

	void onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
	{
		IProviderTestClientBase::onGenericMsg(genericMsg, event);
		genericItemHandle = event.getHandle();
	}

	UInt64 getGenericItemHandle()
	{
		return genericItemHandle;
	}
};

#endif // __EmaTestClients_h_