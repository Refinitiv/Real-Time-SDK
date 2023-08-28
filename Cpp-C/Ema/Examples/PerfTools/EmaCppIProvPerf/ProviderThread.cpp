///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|         Copyright (C) 2021-2022 Refinitiv.     All rights reserved.       --
///*|-----------------------------------------------------------------------------

#include "ProviderThread.h"
#include "MessageDataUtil.h"
#include <map>

#include <iostream>
#include <fstream>

using namespace std;
using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

using namespace perftool::common;


ProviderThreadState::ProviderThreadState(const IProvPerfConfig& config) :
	provConfig(config), currentTick(0)
{
	/* Determine update rates on per-tick basis */
	updatesPerTick = provConfig.updatesPerSec / provConfig.ticksPerSec;
	updatesPerTickRemainder = provConfig.updatesPerSec % provConfig.ticksPerSec;

	/* Determine generic rates on per-tick basis */
	genericsPerTick = provConfig.genMsgsPerSec / provConfig.ticksPerSec;
	genericsPerTickRemainder = provConfig.genMsgsPerSec % provConfig.ticksPerSec;

	/* Reset current pointers to lists of update/generic items */
	reset();
}


ProviderThread::ProviderThread(IProvPerfConfig& config, PerfMessageData* perfData, Int32 provIndex)
	:	stopThread(false), running(false),
		providerThreadIndex(provIndex),
		provPerfConfig(config), perfMessageData(perfData),
		providerClient(NULL),
		provider(NULL),
#if defined(WIN32)
		_handle(NULL), _threadId(0),
#else
		_threadId(0),
#endif
		provThreadState(config),
		statsFile(NULL),
		latencyLogFile(NULL),
		latencyUpdateRandomArray(NULL),
		latencyGenericRandomArray(NULL)
{
}

ProviderThread::~ProviderThread()
{
	stop();
	clean();
}

void ProviderThread::clean()
{
	if (provider != NULL)
		delete provider;
	provider = NULL;

	if (providerClient != NULL)
		delete providerClient;
	providerClient = NULL;

	if (statsFile != NULL)
		fclose(statsFile);
	statsFile = NULL;

	if (latencyLogFile != NULL)
		fclose(latencyLogFile);
	latencyLogFile = NULL;

	if (latencyUpdateRandomArray != NULL)
		delete latencyUpdateRandomArray;
	latencyUpdateRandomArray = NULL;

	if (latencyGenericRandomArray != NULL)
		delete latencyGenericRandomArray;
	latencyGenericRandomArray = NULL;

	ProvItemInfo* itemInfo = updateItems.front();
	while (itemInfo != NULL)
	{
		ProvItemInfo* itemNext = itemInfo->next();
		delete itemInfo;
		itemInfo = itemNext;
	}
	updateItems.clear();
}

void ProviderThread::providerThreadInit()
{
	EmaString tmpFilename;

	tmpFilename = provPerfConfig.statsFilename;
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
	if (provPerfConfig.logLatencyToFile)
	{
		tmpFilename = provPerfConfig.latencyLogFilename;
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


//		case PROVIDER_INTERACTIVE:
	fprintf(statsFile, "UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsgs received, GenMSg Latencies sent, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");

	/* Determine latency ticks number when sending UpdateMsg */
	if (provPerfConfig.updatesPerSec != 0 && provPerfConfig.latencyUpdatesPerSec > 0)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		randomArrayOpts.totalMsgsPerSec = provPerfConfig.updatesPerSec;
		randomArrayOpts.latencyMsgsPerSec = provPerfConfig.latencyUpdatesPerSec;
		randomArrayOpts.ticksPerSec = provPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		latencyUpdateRandomArray = new LatencyRandomArray(randomArrayOpts);
	}

	/* Determine latency ticks number when sending GenericMsg */
	if (provPerfConfig.genMsgsPerSec != 0 && provPerfConfig.latencyGenMsgsPerSec > 0)
	{
		LatencyRandomArrayOptions randomArrayOpts;
		randomArrayOpts.totalMsgsPerSec = provPerfConfig.genMsgsPerSec;
		randomArrayOpts.latencyMsgsPerSec = provPerfConfig.latencyGenMsgsPerSec;
		randomArrayOpts.ticksPerSec = provPerfConfig.ticksPerSec;
		randomArrayOpts.arrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

		latencyGenericRandomArray = new LatencyRandomArray(randomArrayOpts);
	}
	return;
}

#if defined(WIN32)
unsigned __stdcall ProviderThread::ThreadFunc(void* pArguments)
{
	((ProviderThread*)pArguments)->run();

	return 0;
}

#else
extern "C"
{
	void* ProviderThread::ThreadFunc(void* pArguments)
	{
		((ProviderThread*)pArguments)->run();

		return 0;
	}
}
#endif

void ProviderThread::start()
{
//	cout << "ProviderThread.MainThread. start." << endl;
#if defined(WIN32)
	_handle = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &_threadId);
	assert(_handle != 0);

	SetThreadPriority(_handle, THREAD_PRIORITY_NORMAL);
#else
	pthread_create(&_threadId, NULL, ThreadFunc, this);
	assert(_threadId != 0);
#endif

	running = true;
//	cout << "ProviderThread.MainThread. start.  running = true" << endl;
	return;
}

void ProviderThread::stop()
{
	cout << "ProviderThread. stop. #" << providerThreadIndex << endl;

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

void ProviderThread::run()
{
	const PerfTimeValue microSecPerTick = 1000000 / provPerfConfig.ticksPerSec;
	PerfTimeValue currentTime = 0, nextTickTime = 0;  // in microseconds

	cout << "ProviderThread.MainThread. run. #" << providerThreadIndex << endl;
	try {
		providerClient = new ProviderPerfClient(this, provPerfConfig);
		
		//cout << "ProviderThread.MainThread. run.1" << endl;
		OmmIProviderConfig providerConfig;

		providerConfig.operationModel((provPerfConfig.useUserDispatch ? OmmIProviderConfig::UserDispatchEnum : OmmIProviderConfig::ApiDispatchEnum));

		EmaString sProviderName;
		if (!provPerfConfig.providerName.empty())
		{
			sProviderName = provPerfConfig.providerName;
			if (providerThreadIndex > 1)
				sProviderName += providerThreadIndex;
		}
		else
		{
			sProviderName = providerThreadNameBase;
			sProviderName += providerThreadIndex;
		}

		providerConfig.providerName(sProviderName);

		if ( !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
			providerConfig.apiThreadBind(apiThreadCpuId);
		if ( !workerThreadCpuId.empty() && !workerThreadCpuId.caseInsensitiveCompare("-1") )
			providerConfig.workerThreadBind(workerThreadCpuId);

		provider = new OmmProvider(providerConfig, *providerClient);

		//cout << "ProviderThread.MainThread. run.2" << endl;
		while ( !running && !stopThread )
		{
			cout << "ProviderThread. run. Waiting initilization... #" << providerThreadIndex << endl;
			AppUtil::sleep(200);
		}

		if ( !cpuId.empty() && !cpuId.caseInsensitiveCompare("-1") )
		{
			EmaString provThreadName("EmaThread for ");
			provThreadName.append(sProviderName);

			// bind cpuId for the provider thread
			if (!bindThisThread(provThreadName, cpuId))
			{
				cout << "ProviderThread::run() #" << providerThreadIndex << " bindThisThread failed!"
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
		if ( !provPerfConfig.useUserDispatch
			&& !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
		{
			EmaString provApiThreadName(sProviderName);
			provApiThreadName.append("_Api");
			// Add information about API internal thread
			addThisThread(provApiThreadName, apiThreadCpuId, 0);
		}

		if ( !cpuId.empty() && !cpuId.caseInsensitiveCompare("-1")
			|| !workerThreadCpuId.empty() && !workerThreadCpuId.caseInsensitiveCompare("-1")
			|| !provPerfConfig.useUserDispatch && !apiThreadCpuId.empty() && !apiThreadCpuId.caseInsensitiveCompare("-1") )
		{
			printAllThreadBinding();
		}

		currentTime = perftool::common::GetTime::getTimeMicro();
		nextTickTime = currentTime + microSecPerTick;

		while (running && !stopThread)
		{
			//cout << "ProviderThread. run. MainThread..." << endl;
			if (currentTime >= nextTickTime)
			{
				nextTickTime += microSecPerTick;

				// send burst messages
				sendBurstMessages();

				if (provPerfConfig.useUserDispatch)
				{
					provider->dispatch(OmmConsumer::NoWaitEnum); // Dispatch few messages
				}
			}
			else  // if (currentTime < nextTickTime)
			{
				if (provPerfConfig.useUserDispatch)
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
		cout << "ProviderThread. run. #" << providerThreadIndex << " Exception: " << excp.toString() << endl;
	}
	catch (...)
	{
		cout << "ProviderThread. run. #" << providerThreadIndex << " GeneralException" << endl;
	}
	cout << "ProviderThread. run. #" << providerThreadIndex << " FINISH!!!" << endl;
	running = false;

	// Remove the omm provider instance to stop the server
	if (provider != NULL)
		delete provider;
	provider = NULL;

	return;
}


void ProviderThread::prepareRefreshMessageMarketPrice(RefreshMsg& refreshMsg)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	FieldList& fieldListPreEncoded = perfMessageData->getRefreshFieldList();
	FieldList fieldList;

	if (!provPerfConfig.preEncItems)
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

void ProviderThread::prepareRefreshMessageMarketByOrder(RefreshMsg& refreshMsg)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Map& mapOrdersPreEncoded = perfMessageData->getRefreshMboMapOrders();
	Map mapOrders;

	if (!provPerfConfig.preEncItems)
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

void ProviderThread::sendRefreshMessages()
{
	EmaList< ProvItemInfo* >& refreshList = refreshItems.prepareReadList();
	if (refreshList.empty())
		return;

	RefreshMsg refreshMsg;

	for (ProvItemInfo* itemInfo = refreshList.front(); itemInfo != NULL && !stopThread; itemInfo = itemInfo->next())
	{
		refreshMsg.clear();

		refreshMsg.name(itemInfo->getName());
		refreshMsg.domainType(itemInfo->getDomain());
		if (itemInfo->getFlags() & ITEM_IS_SOLICITED)
			refreshMsg.solicited(true);
		if (itemInfo->getFlags() & ITEM_IS_PRIVATE)
			refreshMsg.privateStream(true);

		refreshMsg.state(((itemInfo->getFlags() & ITEM_IS_STREAMING_REQ) ? OmmState::OpenEnum : OmmState::NonStreamingEnum),
			OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed");
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

void ProviderThread::prepareUpdateMessageMarketPrice(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	FieldList fieldList;

	msgDataUtil->fillMarketPriceFieldListUpdateMsg(fieldList, latencyStartTime);

	updateMsg.payload(fieldList);
}

void ProviderThread::prepareUpdateMessageMarketByOrder(UpdateMsg& updateMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Map mapOrders;

	msgDataUtil->fillMarketByOrderMapUpdateMsg(mapOrders, latencyStartTime);

	updateMsg.payload(mapOrders);
}

void ProviderThread::prepareGenericMessageMarketPrice(GenericMsg& genericMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	FieldList fieldList;

	msgDataUtil->fillMarketPriceFieldListGenericMsg(fieldList, latencyStartTime);

	genericMsg.payload(fieldList);
}

void ProviderThread::prepareGenericMessageMarketByOrder(GenericMsg& genericMsg, PerfTimeValue latencyStartTime)
{
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Map mapOrders;

	msgDataUtil->fillMarketByOrderMapGenericMsg(mapOrders, latencyStartTime);

	genericMsg.payload(mapOrders);
}


void ProviderThread::sendUpdateMessages()
{
	EmaList< ProvItemInfo* >& updateList = updateItems;
	if (updateList.empty() || provPerfConfig.updatesPerSec == 0)
		return;

	/* Determine updates to send out. Spread the remainder out over the first ticks */
	UInt32 updatesLeft = provThreadState.getUpdatesPerTick();
	if (provThreadState.getUpdatesPerTickRemainder() > provThreadState.getCurrentTick())
		++updatesLeft;

	// templates for messages (from MsgData.xml)
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Int32 latencyUpdateNumber = (provPerfConfig.latencyUpdatesPerSec > 0) ? latencyUpdateRandomArray->getNext() : -1;
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

		UInt64 clientHandle = itemInfo->getClientHandle();
		if (!providerClient->isActiveStream(clientHandle))
			continue;

		/* When appropriate, provide a latency timestamp for the updates. */
		bool addLatency = (provPerfConfig.latencyUpdatesPerSec == ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == i);
		pUpdateMsg = NULL;

		// when we add latency to the update message then use non pre-encoded field list
		if (addLatency)
			latencyStartTime = (provPerfConfig.nanoTime ? perftool::common::GetTime::getTimeNano(): perftool::common::GetTime::getTimeMicro());
		else
			latencyStartTime = 0;

		if (provPerfConfig.measureEncode)
			measureEncodeStartTime = perftool::common::GetTime::getTimeNano();

		if (!provPerfConfig.preEncItems || addLatency)
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

		if (provPerfConfig.measureEncode)
		{
			measureEncodeEndTime = perftool::common::GetTime::getTimeNano();
			stats.messageEncodeTimeRecords.updateLatencyStats(measureEncodeStartTime, measureEncodeEndTime, 1000);
		}

		if (providerClient->isActiveStream(clientHandle))
			provider->submit(*pUpdateMsg, itemInfo->getHandle());

		stats.updateMsgCount.countStatIncr();

	}  // for-each (updateMsg)

	// save the current item in rotate list
	provThreadState.setCurrentUpdatesItem(itemInfo);

	return;
}

void ProviderThread::sendGenericMessages()
{
	EmaList< ProvItemInfo* >& genericList = updateItems;  // the list of items is common for sending UpdateMsg and GenericMsg 
	if (genericList.empty() || provPerfConfig.genMsgsPerSec == 0)
		return;

	/* Determine generics to send out. Spread the remainder out over the first ticks */
	UInt32 genMsgsLeft = provThreadState.getGenericsPerTick();
	if (provThreadState.getGenericsPerTickRemainder() > provThreadState.getCurrentTick())
		++genMsgsLeft;

	// templates for messages (from MsgData.xml)
	MessageDataUtil* msgDataUtil = MessageDataUtil::getInstance();

	Int32 latencyGenericNumber = (provPerfConfig.latencyGenMsgsPerSec > 0) ? latencyGenericRandomArray->getNext() : -1;
	PerfTimeValue latencyStartTime;
	PerfTimeValue measureEncodeStartTime, measureEncodeEndTime;

	// index of current template
	UInt32 indMpGenericMsg = 0, indMboGenericMsg = 0;

	RotateListUtil< ProvItemInfo* > genericRotateList(genericList, provThreadState.getCurrentGenericsItem());

	GenericMsg genericMsgLatency;
	EmaVector< GenericMsg* >& preEncodedMpGenericMessages = perfMessageData->getMpGenericsPreEncoded();
	EmaVector< GenericMsg* >& preEncodedMboGenericMessages = perfMessageData->getMboGenericsPreEncoded();

	GenericMsg* pGenericMsg;
	ProvItemInfo* itemInfo = NULL;

	for (UInt32 i = 0; i < genMsgsLeft && !stopThread; ++i)
	{
		itemInfo = genericRotateList.getNext();
		if (itemInfo == NULL)
			break;

		UInt64 clientHandle = itemInfo->getClientHandle();
		if (!providerClient->isActiveStream(clientHandle))
			continue;

		/* When appropriate, provide a latency timestamp for the genMsgs. */
		bool addLatency = (provPerfConfig.latencyGenMsgsPerSec == ALWAYS_SEND_LATENCY_GENMSG || latencyGenericNumber == i);
		pGenericMsg = NULL;

		// when we add latency to the generic message then use non pre-encoded field list
		if (addLatency)
		{
			stats.latencyGenMsgSentCount.countStatIncr();
			latencyStartTime = (provPerfConfig.nanoTime ? perftool::common::GetTime::getTimeNano() : perftool::common::GetTime::getTimeMicro());
		}
		else
		{
			latencyStartTime = 0;
		}

		if (provPerfConfig.measureEncode)
			measureEncodeStartTime = perftool::common::GetTime::getTimeNano();

		if (!provPerfConfig.preEncItems || addLatency)
		{
			pGenericMsg = &genericMsgLatency;
			pGenericMsg->clear();

			// prepares the generic message now
			switch (itemInfo->getDomain())
			{
			case RSSL_DMT_MARKET_PRICE:
				prepareGenericMessageMarketPrice(*pGenericMsg, latencyStartTime);
				break;
			case RSSL_DMT_MARKET_BY_ORDER:
				prepareGenericMessageMarketByOrder(*pGenericMsg, latencyStartTime);
				break;
			}
		}
		else
		{	// get the pre-encoded generic message
			switch (itemInfo->getDomain())
			{
			case RSSL_DMT_MARKET_PRICE:
				pGenericMsg = preEncodedMpGenericMessages[indMpGenericMsg];

				if (++indMpGenericMsg >= preEncodedMpGenericMessages.size())
					indMpGenericMsg = 0;
				break;

			case RSSL_DMT_MARKET_BY_ORDER:
				pGenericMsg = preEncodedMboGenericMessages[indMboGenericMsg];

				if (++indMboGenericMsg >= preEncodedMboGenericMessages.size())
					indMboGenericMsg = 0;
				break;
			}
		}

		pGenericMsg->domainType(itemInfo->getDomain());

		if (provPerfConfig.measureEncode)
		{
			measureEncodeEndTime = perftool::common::GetTime::getTimeNano();
			stats.messageEncodeTimeRecords.updateLatencyStats(measureEncodeStartTime, measureEncodeEndTime, 1000);
		}

		if (providerClient->isActiveStream(clientHandle))
			provider->submit(*pGenericMsg, itemInfo->getHandle());

		stats.genMsgSentCount.countStatIncr();
		if ( !stats.firstGenMsgSentTime )
			stats.firstGenMsgSentTime = perftool::common::GetTime::getTimeNano();
	}  // for-each (genericMsg)

	// save the current item in rotate list
	provThreadState.setCurrentGenericsItem(itemInfo);

	return;
}

void ProviderThread::sendBurstMessages()
{
	// the list of items is common for sending UpdateMsg and GenericMsg
	try {
		if (!updateItems.empty())
		{
			sendUpdateMessages();
			sendGenericMessages();

			processClosedHandles();
		}
		sendRefreshMessages();
	}
	catch(OmmInvalidUsageException& exp)
	{
		// when an exception generates after a client closes connection so invokes processClosedHandles and tries continue work
		if (exp.getErrorCode() == OmmInvalidUsageException::InvalidArgumentEnum  && !listClosedClientHandles.empty())
		{
			processClosedHandles();
		}
		else {
			// forwards the exception to the external level
			throw;
		}
	}

	provThreadState.incrementCurrentTick();
}

void ProviderThread::processClosedHandles()
{
	if (!listClosedClientHandles.empty() && !stopThread)
	{
		listClosedClientHandlesMutex.lock();
		if (!updateItems.empty())
		{
			UInt32 sizeUpdateItems = updateItems.size();

			for (UInt32 k = 0; k < listClosedClientHandles.size(); ++k)
			{
				UInt64 clientHandle = listClosedClientHandles[k];
				//cout << "processClosedHandles. Remove clientHandle: " << clientHandle << endl;

				ProvItemInfo* itemInfo = updateItems.front();
				while (itemInfo != NULL)
				{
					ProvItemInfo* itemNext = itemInfo->next();

					if (clientHandle == itemInfo->getClientHandle())
					{	
						updateItems.remove(itemInfo);
						delete itemInfo;
					}
					itemInfo = itemNext;
				}
			}

			// reset all the access indexes in the provThreadState
			if (updateItems.size() < sizeUpdateItems)
			{
				provThreadState.reset();
			}
			//cout << "processClosedHandles. updateItems.size = " << updateItems.size() << " (" << sizeUpdateItems << ")" << endl << endl;
		}

		listClosedClientHandles.clear();
		listClosedClientHandlesMutex.unlock();
	}
}

void ProviderThread::addClosedClientHandle(UInt64 clientHandle)
{
	listClosedClientHandlesMutex.lock();
	listClosedClientHandles.push_back(clientHandle);
	listClosedClientHandlesMutex.unlock();
}


ProviderStats::ProviderStats() :
	inactiveTime(0),
	firstGenMsgSentTime(0),
	firstGenMsgRecvTime(0)
{}

ProviderStats::~ProviderStats() {};

ProviderStats::ProviderStats(const ProviderStats& stats) :
	refreshMsgCount(stats.refreshMsgCount),
	updateMsgCount(stats.updateMsgCount),
	itemRequestCount(stats.itemRequestCount),
	closeMsgCount(stats.closeMsgCount),
	postMsgCount(stats.postMsgCount),
	outOfBuffersCount(stats.outOfBuffersCount),
	statusCount(stats.statusCount),
	intervalMsgEncodingStats(stats.intervalMsgEncodingStats),
	inactiveTime(stats.inactiveTime),
	firstGenMsgSentTime(stats.firstGenMsgSentTime),
	firstGenMsgRecvTime(stats.firstGenMsgRecvTime),
	genMsgLatencyStats(stats.genMsgLatencyStats),
	intervalGenMsgLatencyStats(stats.intervalGenMsgLatencyStats),
	genMsgSentCount(stats.genMsgSentCount),
	genMsgRecvCount(stats.genMsgRecvCount),
	latencyGenMsgSentCount(stats.latencyGenMsgSentCount),
	tunnelStreamBufUsageStats(stats.tunnelStreamBufUsageStats)
{}

ProviderStats& ProviderStats::operator=(const ProviderStats& stats)
{
	refreshMsgCount = stats.refreshMsgCount;
	updateMsgCount = stats.updateMsgCount;
	itemRequestCount = stats.itemRequestCount;
	closeMsgCount = stats.closeMsgCount;
	postMsgCount = stats.postMsgCount;
	outOfBuffersCount = stats.outOfBuffersCount;
	statusCount = stats.statusCount;
	intervalMsgEncodingStats = stats.intervalMsgEncodingStats;
	inactiveTime = stats.inactiveTime;
	firstGenMsgSentTime = stats.firstGenMsgSentTime;
	firstGenMsgRecvTime = stats.firstGenMsgRecvTime;
	genMsgLatencyStats = stats.genMsgLatencyStats;
	intervalGenMsgLatencyStats = stats.intervalGenMsgLatencyStats;
	genMsgSentCount = stats.genMsgSentCount;
	genMsgRecvCount = stats.genMsgRecvCount;
	latencyGenMsgSentCount = stats.latencyGenMsgSentCount;
	tunnelStreamBufUsageStats = stats.tunnelStreamBufUsageStats;

	return *this;
}
