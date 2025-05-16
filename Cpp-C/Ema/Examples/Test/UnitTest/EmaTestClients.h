/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

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

static void testSleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}

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

#endif // __EmaTestClients_h_