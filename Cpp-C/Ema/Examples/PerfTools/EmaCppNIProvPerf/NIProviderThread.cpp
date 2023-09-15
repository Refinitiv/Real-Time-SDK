///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|        Copyright (C) 2021-2022 Refinitiv.    All rights reserved.         --
///*|-----------------------------------------------------------------------------

#include "NIProviderThread.h"
#include "MessageDataUtil.h"
#include "PerfUtils.h"
#include <map>

#include <iostream>
#include <fstream>

using namespace std;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

using namespace perftool::common;


ProviderThreadState::ProviderThreadState(const NIProvPerfConfig& config) :
	niProvConfig(config), currentTick(0)
{
	/* Determine update rates on per-tick basis */
	updatesPerTick = niProvConfig.updatesPerSec / niProvConfig.ticksPerSec;
	updatesPerTickRemainder = niProvConfig.updatesPerSec % niProvConfig.ticksPerSec;

	/* Reset current pointers to lists of update items */
	reset();
}


NIProviderThread::NIProviderThread(NIProvPerfConfig& config, PerfMessageData* perfData, Int32 provIndex)
	:	stopThread(false), running(false),
		providerThreadIndex(provIndex),
		niProvPerfConfig(config), perfMessageData(perfData),
		niProviderClient(NULL),
		provider(NULL),
#if defined(WIN32)
		_handle(NULL), _threadId(0),
#else
		_threadId(0),
#endif
		provThreadState(config),
		statsFile(NULL),
		latencyLogFile(NULL),
		latencyUpdateRandomArray(NULL)
{
}

NIProviderThread::~NIProviderThread()
{
	stop();
	clean();
}

void NIProviderThread::clean()
{
	if (provider != NULL)
		delete provider;
	provider = NULL;

	if (niProviderClient != NULL)
		delete niProviderClient;
	niProviderClient = NULL;

	if (statsFile != NULL)
		fclose(statsFile);
	statsFile = NULL;

	if (latencyLogFile != NULL)
		fclose(latencyLogFile);
	latencyLogFile = NULL;

	if (latencyUpdateRandomArray != NULL)
		delete latencyUpdateRandomArray;
	latencyUpdateRandomArray = NULL;

	ProvItemInfo* itemInfo = updateItems.front();
	while (itemInfo != NULL)
	{
		ProvItemInfo* itemNext = itemInfo->next();
		delete itemInfo;
		itemInfo = itemNext;
	}
	updateItems.clear();

	itemInfo = refreshItems.front();
	while (itemInfo != NULL)
	{
		ProvItemInfo* itemNext = itemInfo->next();
		delete itemInfo;
		itemInfo = itemNext;
	}
	refreshItems.clear();
}

void NIProviderThread::providerThreadInit()
{
	EmaString tmpFilename;

	tmpFilename = niProvPerfConfig.statsFilename;
	tmpFilename.append(providerThreadIndex).append(".csv");

	if (!(statsFile = fopen(tmpFilename.c_str(), "w")))
	{
		EmaString text("Error: Failed to open file '");
		text += tmpFilename;
		text += "'.\n";
		printf("%s", text.c_str());
		AppUtil::logError(text);
		exit(-1);
	}

	latencyLogFile = NULL;
	if (niProvPerfConfig.logLatencyToFile)
	{
		tmpFilename = niProvPerfConfig.latencyLogFilename;
		tmpFilename.append(providerThreadIndex).append(".csv");

		/* Open latency log file. */
		latencyLogFile = fopen(tmpFilename.c_str(), "w");
		if (!latencyLogFile)
		{
			EmaString text("Failed to open latency log file: ");
			text += tmpFilename;
			text += "\n";
			printf("%s", text.c_str());
			AppUtil::logError(text);
			exit(-1);
		}

		fprintf(latencyLogFile, "Message type, Send time, Receive time, Latency (usec)\n");
	}


//		case PROVIDER_NONINTERACTIVE:
	fprintf(statsFile, "UTC, Images sent, Updates sent, CPU usage (%%), Memory (MB)\n");

	/* Determine latency ticks number when sending UpdateMsg */
	if (niProvPerfConfig.updatesPerSec != 0 && niProvPerfConfig.latencyUpdatesPerSec > 0)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		randomArrayOpts.totalMsgsPerSec = niProvPerfConfig.updatesPerSec;
		randomArrayOpts.latencyMsgsPerSec = niProvPerfConfig.latencyUpdatesPerSec;
		randomArrayOpts.ticksPerSec = niProvPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		latencyUpdateRandomArray = new LatencyRandomArray(randomArrayOpts);
	}
	return;
}

void NIProviderThread::providerThreadInitItems(UInt32 itemListCount, UInt32 itemListStartIndex)
{
	if (!PerfUtils::initializeItems(niProvPerfConfig, refreshItems, itemListCount, itemListStartIndex))
		exit(-1);
	return;
}

#if defined(WIN32)
unsigned __stdcall NIProviderThread::ThreadFunc(void* pArguments)
{
	((NIProviderThread*)pArguments)->run();

	return 0;
}

#else
extern "C"
{
	void* NIProviderThread::ThreadFunc(void* pArguments)
	{
		((NIProviderThread*)pArguments)->run();

		return 0;
	}
}
#endif

void NIProviderThread::start()
{
//	cout << "NIProviderThread.MainThread. start." << endl;
#if defined(WIN32)
	_handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &_threadId);
	assert(_handle != 0);

	SetThreadPriority(_handle, THREAD_PRIORITY_NORMAL);
#else
	pthread_create(&_threadId, NULL, ThreadFunc, this);
	assert(_threadId != 0);
#endif

	running = true;
//	cout << "NIProviderThread.MainThread. start.  running = true" << endl;
	return;
}

void NIProviderThread::stop()
{
	cout << "NIProviderThread. stop. #" << providerThreadIndex << endl;

#if defined(WIN32)
	WaitForSingleObject(_handle, INFINITE);
	CloseHandle(_handle);
	_handle = 0;
	_threadId = 0;
#else
	pthread_join(_threadId, NULL);
	_threadId = 0;
#endif
	running = false;
	return;
}

void NIProviderThread::run()
{
	const PerfTimeValue microSecPerTick = 1000000 / niProvPerfConfig.ticksPerSec;
	PerfTimeValue currentTime = 0, nextTickTime = 0;  // in microseconds

	cout << "NIProviderThread.MainThread. run. #" << providerThreadIndex << endl;
	try {
		niProviderClient = new NIProviderPerfClient(this, niProvPerfConfig);
		
		//cout << "NIProviderThread.MainThread. run.1" << endl;
		OmmNiProviderConfig providerConfig;

		providerConfig.operationModel((niProvPerfConfig.useUserDispatch ? OmmNiProviderConfig::UserDispatchEnum : OmmNiProviderConfig::ApiDispatchEnum));

		EmaString sProviderName;
		if (!niProvPerfConfig.providerName.empty())
		{
			sProviderName = niProvPerfConfig.providerName;
			if (providerThreadIndex > 1)
				sProviderName += providerThreadIndex;
		}
		else
		{
			sProviderName = providerThreadNameBase;
			sProviderName += providerThreadIndex;
		}

		providerConfig.providerName(sProviderName);

		providerConfig.username("user");

		if ( !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
			providerConfig.apiThreadBind(apiThreadCpuId);
		if ( !workerThreadCpuId.empty() && !workerThreadCpuId.caseInsensitiveCompare("-1") )
			providerConfig.workerThreadBind(workerThreadCpuId);

		provider = new OmmProvider(providerConfig, *niProviderClient);

		cout << endl << "NIProviderThread.MainThread. run.2" << endl;
		if (!stopThread)
		{
			ChannelInformation ci;
			provider->getChannelInformation(ci);
			cout << "channel info (ni-provider)" << endl << ci << endl;
		}

		while (!running && !stopThread)
		{
			cout << "NIProviderThread. run. Waiting initilization... #" << providerThreadIndex << endl;
			AppUtil::sleep(200);
		}

		if ( !cpuId.empty() && !cpuId.caseInsensitiveCompare("-1") )
		{
			EmaString provThreadName("EmaThread for ");
			provThreadName.append(sProviderName);

			// bind cpuId for the ni provider thread
			if (!bindThisThread(provThreadName, cpuId))
			{
				cout << "NIProviderThread::run() #" << providerThreadIndex << " bindThisThread failed!"
					<< " cpuId[" << cpuId << "] " << endl;
				running = false;
			}
		}
		if ( !workerThreadCpuId.empty() && !workerThreadCpuId.caseInsensitiveCompare("-1") )
		{
			EmaString workerThreadName(sProviderName);
			workerThreadName.append("_RW");
			// Add information about Reactor worker thread
			addThisThread(workerThreadName, workerThreadCpuId, 0);
		}
		if ( !niProvPerfConfig.useUserDispatch
			&& !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
		{
			EmaString provApiThreadName(sProviderName);
			provApiThreadName.append("_Api");
			// Add information about API internal thread
			addThisThread(provApiThreadName, apiThreadCpuId, 0);
		}

		if ( !cpuId.empty() && !cpuId.caseInsensitiveCompare("-1")
			|| !workerThreadCpuId.empty() && !workerThreadCpuId.caseInsensitiveCompare("-1")
			|| !niProvPerfConfig.useUserDispatch && !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
		{
			printAllThreadBinding();
		}

		currentTime = perftool::common::GetTime::getTimeMicro();
		nextTickTime = currentTime + microSecPerTick;

		cout << "NIProviderThread.MainThread. sending all the Refreshes. #" << providerThreadIndex << endl;

		// Main loop: tick by tick
		while (running && !stopThread)
		{
			//cout << "NIProviderThread. run. MainThread..." << endl;
			if (currentTime >= nextTickTime)
			{
				nextTickTime += microSecPerTick;

				// send burst messages
				if (niProviderClient->isConnectionUp())
				{
					sendBurstMessages();
				}

				if (niProvPerfConfig.useUserDispatch)
				{
					provider->dispatch(OmmConsumer::NoWaitEnum); // Dispatch few messages
				}
			}
			else  // if (currentTime < nextTickTime)
			{
				if (niProvPerfConfig.useUserDispatch)
				{
					PerfTimeValue sleepTime = (nextTickTime - currentTime);  // in microseconds
					provider->dispatch(sleepTime); // Dispatch either sleeps or works (dispatching msgs) till next tick time;
				}
				else  // ApiDispatch
				{
					PerfTimeValue sleepTimeMs = (nextTickTime - currentTime) / 1000;  // in milliseconds
					AppUtil::sleep(sleepTimeMs);
				}
			}

			currentTime = perftool::common::GetTime::getTimeMicro();
		}
	}
	catch (const OmmException& excp)
	{
		cout << "NIProviderThread. run. #" << providerThreadIndex << " Exception: " << excp.toString() << endl;
	}
	catch (...)
	{
		cout << "NIProviderThread. run. #" << providerThreadIndex << " GeneralException" << endl;
	}
	cout << "NIProviderThread. run. #" << providerThreadIndex << " FINISH!!!" << endl;
	running = false;

	// Remove the omm provider instance to stop the server
	if (provider != NULL)
		delete provider;
	provider = NULL;

	return;
}


void NIProviderThread::prepareRefreshMessageMarketPrice(RefreshMsg& refreshMsg)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	FieldList& fieldListPreEncoded = perfMessageData->getRefreshFieldList();
	FieldList fieldList;

	if (!niProvPerfConfig.preEncItems)
	{
		// fills up FieldList by template for RefreshMsg
		msgDataUtil->fillMarketPriceFieldListRefreshMsg(fieldList);

		refreshMsg.payload(fieldList);
	}
	else
	{
		refreshMsg.payload(fieldListPreEncoded);
	}
}

void NIProviderThread::prepareRefreshMessageMarketByOrder(RefreshMsg& refreshMsg)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Map& mapOrdersPreEncoded = perfMessageData->getRefreshMboMapOrders();
	Map mapOrders;

	if (!niProvPerfConfig.preEncItems)
	{
		// fills up Map (of orders) by template for RefreshMsg
		msgDataUtil->fillMarketByOrderMapRefreshMsg(mapOrders);

		refreshMsg.payload(mapOrders);
	}
	else
	{
		refreshMsg.payload(mapOrdersPreEncoded);
	}
}

void NIProviderThread::sendRefreshMessages()
{
	EmaList< ProvItemInfo* >& refreshList = refreshItems;
	if (refreshList.empty())
		return;

	RefreshMsg refreshMsg;

	for (ProvItemInfo* itemInfo = refreshList.front(); itemInfo != NULL && !stopThread; itemInfo = itemInfo->next())
	{
		refreshMsg.clear();

		if (niProvPerfConfig.useServiceId)
			refreshMsg.serviceId(niProvPerfConfig.serviceId);
		else
			refreshMsg.serviceName(niProvPerfConfig.serviceName);

		refreshMsg.name(itemInfo->getName());
		refreshMsg.domainType(itemInfo->getDomain());
		if (itemInfo->getFlags() & ITEM_IS_SOLICITED)
			refreshMsg.solicited(true);
		if (itemInfo->getFlags() & ITEM_IS_PRIVATE)
			refreshMsg.privateStream(true);

		refreshMsg.state(((itemInfo->getFlags() & ITEM_IS_STREAMING_REQ) ? OmmState::OpenEnum : OmmState::NonStreamingEnum),
			OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed");
		refreshMsg.qos();  // default values for qos(OmmQos::RealTimeEnum, OmmQos::TickByTickEnum)

		switch (itemInfo->getDomain())
		{
		case RSSL_DMT_MARKET_PRICE:
			prepareRefreshMessageMarketPrice(refreshMsg);
			break;
		case RSSL_DMT_MARKET_BY_ORDER:
			prepareRefreshMessageMarketByOrder(refreshMsg);
			break;
		default:
			break;
		}

		refreshMsg.complete();

		provider->submit(refreshMsg, itemInfo->getHandle());

		stats.refreshMsgCount.countStatIncr();

		// move the item-info to the list for sending updates
		updateItems.push_back(itemInfo);

	}  // for-each (refreshList)

	refreshList.clear();

	return;
}

void NIProviderThread::prepareUpdateMessageMarketPrice(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	FieldList fieldList;

	msgDataUtil->fillMarketPriceFieldListUpdateMsg(fieldList, latencyStartTime);

	updateMsg.payload(fieldList);
}

void NIProviderThread::prepareUpdateMessageMarketByOrder(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Map mapOrders;

	msgDataUtil->fillMarketByOrderMapUpdateMsg(mapOrders, latencyStartTime);

	updateMsg.payload(mapOrders);
}

void NIProviderThread::sendUpdateMessages()
{
	EmaList< ProvItemInfo* >& updateList = updateItems;
	if (updateList.empty() || niProvPerfConfig.updatesPerSec == 0)
		return;

	/* Determine updates to send out. Spread the remainder out over the first ticks */
	UInt32 updatesLeft = provThreadState.getUpdatesPerTick();
	if (provThreadState.getUpdatesPerTickRemainder() > provThreadState.getCurrentTick())
		++updatesLeft;

	// templates for messages (from MsgData.xml)
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Int32 latencyUpdateNumber = (niProvPerfConfig.latencyUpdatesPerSec > 0) ? latencyUpdateRandomArray->getNext() : -1;
	PerfTimeValue latencyStartTime;
	PerfTimeValue measureEncodeStartTime, measureEncodeEndTime;

	// index of current template
	UInt32 indMpUpdMsg = 0, indMboUpdMsg = 0;

	RotateListUtil< ProvItemInfo* > updateRotateList(updateList, provThreadState.getCurrentUpdatesItem());

	UpdateMsg updateMsgLatency;
	EmaVector< UpdateMsg* >& preEncodedMpUpdateMessages = perfMessageData->getMpUpdatesPreEncoded();
	EmaVector< UpdateMsg* >& preEncodedMboUpdateMessages = perfMessageData->getMboUpdatesPreEncoded();

	UpdateMsg* pUpdateMsg;
	ProvItemInfo* itemInfo = NULL;

	for (UInt32 i = 0; i < updatesLeft && !stopThread; ++i)
	{
		itemInfo = updateRotateList.getNext();
		if (itemInfo == NULL)
			break;

		/* When appropriate, provide a latency timestamp for the updates. */
		bool addLatency = (niProvPerfConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == i);
		pUpdateMsg = NULL;

		// when we add latency to the update message then use non pre-encoded field list
		if (addLatency)
			latencyStartTime = (niProvPerfConfig.nanoTime ? perftool::common::GetTime::getTimeNano(): perftool::common::GetTime::getTimeMicro());
		else
			latencyStartTime = 0;

		if (niProvPerfConfig.measureEncode)
			measureEncodeStartTime = perftool::common::GetTime::getTimeNano();

		if (!niProvPerfConfig.preEncItems || addLatency)
		{
			pUpdateMsg = &updateMsgLatency;
			pUpdateMsg->clear();

			// prepares the update message now
			switch (itemInfo->getDomain())
			{
			case RSSL_DMT_MARKET_PRICE:
				prepareUpdateMessageMarketPrice(*pUpdateMsg, latencyStartTime);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				prepareUpdateMessageMarketByOrder(*pUpdateMsg, latencyStartTime);
				break;
			}
		}
		else
		{	// get the pre-encoded update message
			switch (itemInfo->getDomain())
			{
			case RSSL_DMT_MARKET_PRICE:
				pUpdateMsg = preEncodedMpUpdateMessages[indMpUpdMsg];

				if (++indMpUpdMsg >= preEncodedMpUpdateMessages.size())
					indMpUpdMsg = 0;
				break;

			case RSSL_DMT_MARKET_BY_ORDER:
				pUpdateMsg = preEncodedMboUpdateMessages[indMboUpdMsg];

				if (++indMboUpdMsg >= preEncodedMboUpdateMessages.size())
					indMboUpdMsg = 0;
				break;
			}
		}

		pUpdateMsg->domainType(itemInfo->getDomain());

		if (niProvPerfConfig.measureEncode)
		{
			measureEncodeEndTime = perftool::common::GetTime::getTimeNano();
			stats.messageEncodeTimeRecords.updateLatencyStats(measureEncodeStartTime, measureEncodeEndTime, 1000);
		}

		provider->submit(*pUpdateMsg, itemInfo->getHandle());

		stats.updateMsgCount.countStatIncr();

	}  // for-each (updateMsg)

	// save the current item in rotate list
	provThreadState.setCurrentUpdatesItem(itemInfo);

	return;
}

void NIProviderThread::sendBurstMessages()
{
	if (!updateItems.empty())
	{
		sendUpdateMessages();
	}
	sendRefreshMessages();

	provThreadState.incrementCurrentTick();
}


ProviderStats::ProviderStats() :
	inactiveTime(0)
{}

ProviderStats::~ProviderStats() {};

ProviderStats::ProviderStats(const ProviderStats& stats) :
	refreshMsgCount(stats.refreshMsgCount),
	updateMsgCount(stats.updateMsgCount),
	itemRequestCount(stats.itemRequestCount),
	closeMsgCount(stats.closeMsgCount),
	statusCount(stats.statusCount),
	intervalMsgEncodingStats(stats.intervalMsgEncodingStats),
	inactiveTime(stats.inactiveTime)
{}

ProviderStats& ProviderStats::operator=(const ProviderStats& stats)
{
	refreshMsgCount = stats.refreshMsgCount;
	updateMsgCount = stats.updateMsgCount;
	itemRequestCount = stats.itemRequestCount;
	closeMsgCount = stats.closeMsgCount;
	statusCount = stats.statusCount;
	intervalMsgEncodingStats = stats.intervalMsgEncodingStats;
	inactiveTime = stats.inactiveTime;

	return *this;
}
