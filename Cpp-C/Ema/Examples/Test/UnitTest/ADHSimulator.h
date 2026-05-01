/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#pragma once

#ifndef __ADH_SIMULATOR_H_
#define __ADH_SIMULATOR_H_

#include "gtest/gtest.h"

#include "rtr/rsslReactor.h"
#include "rtr/rsslTransport.h"
#include "EmaVector.h"

#include <thread>
#include <atomic>

using namespace refinitiv::ema::access;

class ADHSimulator;

/* client session information */
class ADHClientSessionInfo {
public:
	ADHClientSessionInfo() :
		pADHSimulator(nullptr), pReactorChannel(nullptr), isInUse(false)
	{}

	ADHClientSessionInfo(ADHSimulator* adhSim, RsslReactorChannel* pReactorChan) :
		pADHSimulator(adhSim), pReactorChannel(pReactorChan), isInUse(false)
	{}

	~ADHClientSessionInfo() {}

	void clear()
	{
		pADHSimulator = nullptr;
		pReactorChannel = nullptr;
		isInUse = false;
	}

	ADHSimulator* pADHSimulator;
	RsslReactorChannel* pReactorChannel;
	bool isInUse;
};  // class ADHClientSessionInfo

/* Counter of received messages */
struct CountMessages {
	std::atomic<unsigned> countRequest;
	std::atomic<unsigned> countRefresh;
	std::atomic<unsigned> countStatus;
	std::atomic<unsigned> countUpdate;
	std::atomic<unsigned> countClose;
	std::atomic<unsigned> countAck;
	std::atomic<unsigned> countGeneric;

	CountMessages() : countRequest(0), countRefresh(0), countStatus(0), countUpdate(0), countClose(0), countAck(0), countGeneric(0)
	{}

	void clear()
	{
		countRequest = 0;
		countRefresh = 0;
		countStatus = 0;
		countUpdate = 0;
		countClose = 0;
		countAck = 0;
		countGeneric = 0;
	}
};

/* Options for ADHSimulator */
class ADHSimulatorOptions
{
public:
	ADHSimulatorOptions() :
		pReactorOptions(nullptr), sendGenericMsg(false),
		compressionType(0), compressionLevel(0),
		supportProviderDictionaryDownload(false),
		saveMsgs(false),
		rejectLogin(false)
	{
		memset(this->portNo, 0, sizeof(this->portNo));
	}

	ADHSimulatorOptions(const char* portNo) :
		pReactorOptions(nullptr), sendGenericMsg(false),
		compressionType(0), compressionLevel(0),
		supportProviderDictionaryDownload(false),
		saveMsgs(false),
		rejectLogin(false)
	{
		memset(this->portNo, 0, sizeof(this->portNo));
		if (portNo)
		{
			strncpy(this->portNo, portNo, sizeof(this->portNo) - 1);
		}
	}

	ADHSimulatorOptions(RsslCreateReactorOptions* pReactorOpts, const char* portNo) :
		pReactorOptions(pReactorOpts), sendGenericMsg(false),
		compressionType(0), compressionLevel(0),
		supportProviderDictionaryDownload(false),
		saveMsgs(false),
		rejectLogin(false)
	{
		memset(this->portNo, 0, sizeof(this->portNo));
		if ( portNo )
		{
			strncpy(this->portNo, portNo, sizeof(this->portNo) - 1);
		}
	}

	ADHSimulatorOptions(RsslCreateReactorOptions* pReactorOpts, char* portNo, bool sendGenericMsg) :
		pReactorOptions(pReactorOpts), sendGenericMsg(sendGenericMsg),
		compressionType(0), compressionLevel(0),
		supportProviderDictionaryDownload(false),
		saveMsgs(false),
		rejectLogin(false)
	{
		memset(this->portNo, 0, sizeof(this->portNo));
		if ( portNo )
		{
			strncpy(this->portNo, portNo, sizeof(this->portNo) - 1);
		}
	}

	ADHSimulatorOptions(ADHSimulatorOptions const& adhOpts) :
		pReactorOptions(adhOpts.pReactorOptions),
		sendGenericMsg(adhOpts.sendGenericMsg.load()),
		compressionType(adhOpts.compressionType),
		compressionLevel(adhOpts.compressionLevel),
		supportProviderDictionaryDownload(adhOpts.supportProviderDictionaryDownload.load()),
		saveMsgs(adhOpts.saveMsgs.load()),
		rejectLogin(adhOpts.rejectLogin.load())
	{
		memset(this->portNo, 0, sizeof(this->portNo));
		if (adhOpts.portNo[0] != '\0')
		{
			strncpy(this->portNo, adhOpts.portNo, sizeof(this->portNo) - 1);
		}
	}

	ADHSimulatorOptions& operator=(ADHSimulatorOptions const& adhOpts)
	{
		if (this != &adhOpts)
		{
			pReactorOptions = adhOpts.pReactorOptions;
			sendGenericMsg = adhOpts.sendGenericMsg.load();
			compressionType = adhOpts.compressionType;
			compressionLevel = adhOpts.compressionLevel;
			supportProviderDictionaryDownload = adhOpts.supportProviderDictionaryDownload.load();
			saveMsgs = adhOpts.saveMsgs.load();
			rejectLogin = adhOpts.rejectLogin.load();

			memset(this->portNo, 0, sizeof(this->portNo));
			if (adhOpts.portNo[0] != '\0')
			{
				strncpy(this->portNo, adhOpts.portNo, sizeof(this->portNo) - 1);
			}
		}
		return *this;
	}

	~ADHSimulatorOptions() = default;

	void clear()
	{
		pReactorOptions = nullptr;
		*portNo = '\0';
		sendGenericMsg = false;
		compressionType = 0;
		compressionLevel = 0;
	}

	char* getPortNo() { return portNo; }
	const char* getPortNo() const { return portNo; }

	bool shouldSendGenericMsg() const { return sendGenericMsg.load(); }

	bool supportsProviderDictionaryDownload() const { return supportProviderDictionaryDownload.load(); }

	static const size_t MAX_PORTNO_LEN = 32;
public:
	/* Creation parameters. The parameters can be changed before creating ADHSimulator. */
	RsslCreateReactorOptions* pReactorOptions;
	char portNo[MAX_PORTNO_LEN];

	RsslUInt32		compressionType;  // compression types supported by the server
	RsslUInt32		compressionLevel;  // level of compression to use

	/* Behaviour options. */
	std::atomic <bool> sendGenericMsg;  // Indicates whether to send generic message
	std::atomic <bool> supportProviderDictionaryDownload;  // Indicates whether to support provider dictionary download	
	std::atomic <bool> saveMsgs;  // Indicates whether to save the messages into the ADH simulator's message queue
	std::atomic <bool> rejectLogin;		// Indicates whether to reject login requests
};

class ADHSimulator
{
public:
	ADHSimulator() = delete;

	ADHSimulator(ADHSimulatorOptions& options);

	ADHSimulator(ADHSimulator const&) = delete;

	~ADHSimulator();

	/* Start the ADH simulator thread */
	bool start();

	/* Is the ADH simulator thread running? */
	bool isRunning() const { return isRunningFlag.load(); }

	/* Stop the ADH simulator thread gracefully */
	void stop();

	/* Send a message to the Reactor channel */
	static RsslRet sendMessage(RsslReactor* pReactor, RsslReactorChannel* chnl, RsslBuffer* msgBuf);

	/* Send an RsslMsg to the Reactor channel */
	static RsslRet sendMessage(RsslReactor* pReactor, RsslReactorChannel* chnl, RsslMsg* pMsg);

	/* Callbacks */
	static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent);

	static RsslReactorCallbackRet defaultMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pRsslMsgEvent);

	static RsslReactorCallbackRet channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pChannelEvent);

	RsslMsg* popMsg();
	UInt32	 getMsgQueueSize() const { return _messageQueue.size(); }

	unsigned getCountRequest() { return counters.countRequest.load(); }
	unsigned getCountRefresh() { return counters.countRefresh.load(); }
	unsigned getCountStatus() { return counters.countStatus.load(); }
	unsigned getCountUpdate() { return counters.countUpdate.load(); }
	unsigned getCountClose() { return counters.countClose.load(); }
	unsigned getCountAck() { return counters.countAck.load(); }
	unsigned getCountGeneric() { return counters.countGeneric.load(); }

	void enableSendGenericMsg(bool enable) { options.sendGenericMsg = enable; }

	void closeChannels();

public:

	ADHSimulatorOptions options;
	RsslReactor* pReactor;
	std::vector<ADHClientSessionInfo> clientList;  // list of active clients

private:
	
	/* ADH simulator thread method. Started by the start() method */
	void runThreadADH();

	/* Get a new client session */
	ADHClientSessionInfo* getNewClientSession();

	/* Close the RsslReactorChannel. Cleans up associated resources.
	* pReactorChannel - The channel to close
	*/
	void closeChannel(RsslReactorChannel* pReactorChannel);

	/* Close a client session for a channel.
	* pReactorChannel - The channel to remove the client session for
	*/
	void closeClientSessionForChannel(RsslReactorChannel* pReactorChannel);

	/* Send generic message on the LOGIN domain */
	RsslRet sendGenericMessageLogin(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pRsslMsgEvent);

	EmaVector<RsslMsg*> _messageQueue;					// The Queues are used to pull data for verification.  The lists are used for memory cleanup
	EmaVector<RsslMsg*> _messageList;				

private:

	RsslServer* rsslSrvr;
	RsslErrorInfo rsslErrorInfo;

	std::thread tr;						// internal ADH simulator thread
	std::atomic <bool> stopThreadFlag;	// Indicates whether the ADH simulator thread should stop
	std::atomic <bool> isRunningFlag;	// Indicates whether the ADH simulator thread is running

	fd_set	readFds;
	fd_set	writeFds;
	fd_set	exceptFds;
	
	RsslDataDictionary dictionary;

	CountMessages counters;

};  // class ADHSimulator

#endif // !__ADH_SIMULATOR_H_
