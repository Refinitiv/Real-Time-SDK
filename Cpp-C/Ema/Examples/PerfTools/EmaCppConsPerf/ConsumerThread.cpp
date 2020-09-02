///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "ConsumerThread.h"

#include "../Common/XmlItemParser.h"

#define TIM_TRK_1_FID 3902
#define TIM_TRK_2_FID 3903
#define TIM_TRK_3_FID 3904

#define UPDATE_CAPACITY 10000
//#define POST_CAPACITY 100000


using namespace::rtsdk::ema::rdm;
using namespace::rtsdk::ema::access;
using namespace::std;
using namespace::perftool::common;
ConsumerStats::ConsumerStats() : imageTimeRecorded (false),
imageRetrievalStartTime( 0 ),
imageRetrievalEndTime(0),
firstUpdateTime(0),
firstGenMsgSentTime(0),
firstGenMsgRecvTime(0)
{}
ConsumerStats::~ConsumerStats() {}

ConsumerThread::ConsumerThread( ConsPerfConfig& consPerfCfg) : 
pConsPerfCfg( &consPerfCfg ), consumerThreadIndex( 0 ), itemListUniqueIndex(0), 
itemListCount(0),
cpuId(-1),
apiThreadCpuId(-1),
running(false),
stopThread(false),
statsFile( NULL ),
latencyLogFile( NULL ),
desiredServiceHandle(0),
isDesiredServiceUp(false),
itemsRequestedCount(0),
refreshCompleteCount(0),
updateLatList1(UPDATE_CAPACITY),
updateLatList2(UPDATE_CAPACITY),
 pWriteListPtr( &updateLatList1 ),
 pReadListPtr( &updateLatList2 ),
 testPassed(true),
pEmaOmmConsumer( NULL )
{
}
ConsumerThread::~ConsumerThread()
{
	if( pEmaOmmConsumer )
		delete pEmaOmmConsumer;
	if( latencyLogFile )
		fclose( latencyLogFile );
	if( statsFile )
		fclose( statsFile );
	UInt64 itReqListSize = itemRequestList.size();
	UInt64 i = 0;
	if(itReqListSize > 0)
	{
		for( i = 0; i < itReqListSize; ++i)
		{
			if( itemRequestList[i] )
			{
				delete itemRequestList[i];
				itemRequestList[i] = NULL;
			}
		}
	}
	itemRequestList.clear();
	postItemList.clear();
	genMsgItemList.clear();
	fprintf(stdout,"Destroying %s%d cleaned %llu Items\n",
		BASECONSUMER_NAME, consumerThreadIndex, i);
}

#if defined(WIN32)
unsigned __stdcall ConsumerThread::ThreadFunc( void* pArguments )
{
	((ConsumerThread *)pArguments)->run();

	return 0;
}

#else
extern "C" 
{
	 void * ConsumerThread::ThreadFunc( void* pArguments )
	{
		((ConsumerThread *)pArguments)->run();
		
		return 0;
	}
}
#endif

void  ConsumerThread::start()
{
#if defined(WIN32)
	_handle = (HANDLE)_beginthreadex( NULL, 0, ThreadFunc, this, 0, &_threadId );
	assert( _handle != 0 );

	SetThreadPriority( _handle, THREAD_PRIORITY_NORMAL );
#else
	pthread_create( &_threadId, NULL, ThreadFunc, this );
	assert( _threadId != 0 );
#endif

	running = true;
}

void  ConsumerThread::stop()
{
	if ( running )
	{
		running = false;
#if defined(WIN32)
		WaitForSingleObject( _handle, INFINITE );
		CloseHandle( _handle );
		_handle = 0;
		_threadId = 0;
#else
		pthread_join( _threadId, NULL );
		_threadId = 0;
#endif
	}

	running = false;
}

bool ConsumerThread::initialize()
{	
	srcClient.init( this );
	mPriceClient.init( this ); 
	mByOrderClient.init( this ); 

	XmlItemList *pXmlItemList;
	XmlItemParser xmlItemParser;

	if(!(pXmlItemList = xmlItemParser.create(pConsPerfCfg->itemFilename.c_str(), pConsPerfCfg->itemRequestCount)))
	{
		printf("Failed to load item list from file '%s'.\n", pConsPerfCfg->itemFilename.c_str());
		return false;
	}

	Int32 index = 0;
	Int32 xmlItemListIndex = 0;
	// Copy item information from the XML list.
	for(index = 0; index < itemListCount; ++index)
	{		
		ItemRequest *newItReq = new ItemRequest;

		// Once we have filled our list with the common items,
		// start using the range of items unique to this consumer thread. 
		if( xmlItemListIndex == pConsPerfCfg->commonItemCount	&& xmlItemListIndex < itemListUniqueIndex)
			xmlItemListIndex = itemListUniqueIndex;
		newItReq->position = index;
		newItReq->itemName = (*pXmlItemList)[xmlItemListIndex]->name;
		newItReq->itemInfo.domain = (*pXmlItemList)[xmlItemListIndex]->domain;
		if( newItReq->itemInfo.domain == MMT_MARKET_BY_ORDER )
			newItReq->itemInfo.pAppClient = &mByOrderClient;
		else
			newItReq->itemInfo.pAppClient = &mPriceClient;
		if( (*pXmlItemList)[xmlItemListIndex]->post )
			newItReq->itemInfo.itemFlags |= ITEM_IS_POST;
		if((*pXmlItemList)[xmlItemListIndex]->genMsg && pConsPerfCfg->genMsgsPerSec)
			newItReq->itemInfo.itemFlags |= ITEM_IS_GEN_MSG;
		if (!(*pXmlItemList)[xmlItemListIndex]->snapshot )
		{
			newItReq->itemInfo.itemFlags |= ITEM_IS_STREAMING_REQ;
		}
		
		itemRequestList.push_back(newItReq);
		++xmlItemListIndex;
	}

	return true;
}

void ConsumerThread::dumpConsumerItemList()
{
	char tmpFilename[60];
	sprintf(tmpFilename, "%s%dItems.txt",BASECONSUMER_NAME, consumerThreadIndex);
	FILE *fp;
	if (!(fp = fopen(tmpFilename, "w")))
	{
		return;
	}
	fprintf(fp,"\n %s ItemListCount: %d, temRequestList size: %llu", tmpFilename,
					itemListCount, itemRequestList.size());
	for(int i = 0; i < itemRequestList.size(); ++i)
	{
		fprintf(fp,"\n itemname: %s, Position: %llu, StreamId: %d, Domain: %u, Handle: %llu, itemFlags: %u, streaming: %s, post: %s",
			itemRequestList[i]->itemName.c_str(),
			itemRequestList[i]->position,
			itemRequestList[i]->itemInfo.StreamId,
			itemRequestList[i]->itemInfo.domain,
			itemRequestList[i]->itemInfo.handle,
			itemRequestList[i]->itemInfo.itemFlags,
			(itemRequestList[i]->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ ? "true" : "false"),
			(itemRequestList[i]->itemInfo.itemFlags & ITEM_IS_POST ? "true" : "false"));

	}
	fclose(fp);
}

void ConsumerThread::consumerThreadInit( ConsPerfConfig& consPerfConfig, Int32 consThreadId)
{
	consumerThreadIndex = consThreadId;
	char tmpFilename[sizeof(consPerfConfig.statsFilename) + 8];

	snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
		consPerfConfig.statsFilename.c_str(), consThreadId);

	/* Open stats file. */
	if (!(statsFile = fopen(tmpFilename, "w")))
	{	
		EmaString text("Error: Failed to open file '");
		text += tmpFilename;
		text += "'.\n";
		AppUtil::logError(text);
		exit(-1);
	}

	if (consPerfConfig.logLatencyToFile)
	{
		snprintf(tmpFilename, sizeof(tmpFilename), "%s%d.csv", 
			consPerfConfig.latencyLogFilename.c_str(), consThreadId);

		/* Open latency log file. */
		latencyLogFile = fopen(tmpFilename, "w");
		if (!latencyLogFile)
		{
			EmaString text("Failed to open latency log file: '");
			text += tmpFilename;
			text += "'.\n";
			AppUtil::logError(text);
			exit(-1);
		}

		fprintf(latencyLogFile, "Message type, Send time, Receive time, Latency (usec)\n");
	}

	fprintf(statsFile, "UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate (msg/sec), Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsgs received, GenMsg Latencies sent, GenMsg latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");

}
void ConsumerThread::run()
{
	// Calculations
	Int64 microSecPerTick = 0;
	PerfTimeValue currentTime = 0, nextTickTime = 0;
	Int32 postsPerTick = 0, postsPerTickRemainder = 0;
	Int32 genMsgsPerTick = 0, genMsgsPerTickRemainder = 0;
	Int32 currentTicks = 0;

	microSecPerTick = 1000000 / pConsPerfCfg->ticksPerSec;

	postsPerTick = pConsPerfCfg->postsPerSec / pConsPerfCfg->ticksPerSec;
	postsPerTickRemainder = pConsPerfCfg->postsPerSec % pConsPerfCfg->ticksPerSec;
	genMsgsPerTick = pConsPerfCfg->genMsgsPerSec / pConsPerfCfg->ticksPerSec;
	genMsgsPerTickRemainder = pConsPerfCfg->genMsgsPerSec % pConsPerfCfg->ticksPerSec;

	EmaString consumerName;
	switch (pConsPerfCfg->websocketProtocol)
	{
	case ConsPerfConfig::WebSocketJSONEnum:
		consumerName = CONSUMER_NAME_WSJSON;
		break;
	case ConsPerfConfig::WebSocketRWFEnum:
		consumerName = CONSUMER_NAME_WSRWF;
		break;

	case ConsPerfConfig::NoWebSocketEnum:
	default:
		consumerName = BASECONSUMER_NAME;
		break;
	}

	AppUtil::log("Running Thread %s%d\n", consumerName.c_str(), consumerThreadIndex);
	// Create OmmConsumer & login Routine
	consumerName += consumerThreadIndex;
	EmaString conThreadName("EmaThread for ");
	conThreadName += consumerName;
	if( !pConsPerfCfg->useUserDispatch )
	{
		if( apiThreadCpuId != -1 && apiThreadCpuId == cpuId )
		{
			testPassed = false;
			failureLocation = "ConsumerThread::run() - apiThreadCpuId[";
			failureLocation.append(apiThreadCpuId);
			failureLocation += "] == cpuId[";
			failureLocation.append(cpuId);
			failureLocation += "] ";
			stop();
			return;
		}
		if( !pConsPerfCfg->useUserDispatch && cpuId  != -1 )
			firstThreadSnapshot();
	}

	try {
		pEmaOmmConsumer = new  OmmConsumer( OmmConsumerConfig().consumerName( consumerName ).username( pConsPerfCfg->username ).operationModel( (pConsPerfCfg->useUserDispatch ? OmmConsumerConfig::UserDispatchEnum :OmmConsumerConfig::ApiDispatchEnum) ) );
	}
	catch ( const OmmException& excp )
	{
		AppUtil::logError( excp.toString());
		testPassed = false;
		failureLocation = "ConsumerThread::run() - new OmmConsumer() failed";
		stop();
		return;
	}

	if( !pConsPerfCfg->useUserDispatch && apiThreadCpuId != -1  && cpuId  != -1)
	{
		EmaString consumerApiThread(BASECONSUMER_NAME);
		consumerApiThread += consumerThreadIndex;
		consumerApiThread += "_Api";
		secondThreadSnapshot(consumerApiThread, apiThreadCpuId);
		printAllThreadBinding();
	}
	
	try {
		desiredServiceHandle = pEmaOmmConsumer->registerClient( ReqMsg().domainType( rtsdk::ema::rdm::MMT_DIRECTORY ).filter( SERVICE_INFO_FILTER | SERVICE_STATE_FILTER ), srcClient, ( void * ) pEmaOmmConsumer);
	}
	catch ( const OmmException& excp ) {
		AppUtil::logError( excp.toString());
		testPassed = false;
		failureLocation = "ConsumerThread::run() - registerClient() for MMT_DIRECTORY  failed";
		stop();
		return;
	}

	while( 1 )
	{
		if( pConsPerfCfg->useUserDispatch )
			pEmaOmmConsumer->dispatch( 10 );
		
		if( isDesiredServiceUp )
				break;
		if(!running || stopThread)
		{
			break;
		}

		AppUtil::sleep( 10 );
	}

	currentTime = perftool::common::GetTime::getTimeMicro();
	nextTickTime = currentTime + microSecPerTick;
	itemsRequestedCount = 0;

	if( !pConsPerfCfg->useUserDispatch )
	{ // ApiDispatch
		while( 1 )
		{
			if(!running || stopThread)
			{
				break;
			}
			currentTime = perftool::common::GetTime::getTimeMicro();
			// read until no more to read and then write leftover from previous burst
			if( currentTime >= nextTickTime)
			{
				nextTickTime += microSecPerTick;
				// only send bursts on tick boundary
	            // send item request and post bursts
	            if (sendBursts(currentTicks, postsPerTick, postsPerTickRemainder, genMsgsPerTick, genMsgsPerTickRemainder) == false)
				{
					testPassed = false;
					failureLocation = "ConsumerThread::run() - sendBursts at ApiDispatch";
					break;
				}

	            if (++currentTicks == pConsPerfCfg->ticksPerSec)
	                currentTicks = 0;
			}
			else			
				AppUtil::sleep( (nextTickTime - currentTime) / 1000 );
		}
	}
	else { // UserDispatch
		while( 1 )
		{
			if(!running || stopThread)
			{
				break;
			}

			if( sendBursts(currentTicks, postsPerTick, postsPerTickRemainder, genMsgsPerTick, genMsgsPerTickRemainder) == false)
			{
				testPassed = false;
				failureLocation = "ConsumerThread::run() - sendBursts failed at UserDispatch";
				break;
			}
			currentTime = perftool::common::GetTime::getTimeMicro();
			if( currentTime < nextTickTime )			
				pEmaOmmConsumer->dispatch( (nextTickTime - currentTime) ); // Dispatch either sleeps or works (dispatching msgs) till next tick time;
			else
			{
				pEmaOmmConsumer->dispatch( OmmConsumer::NoWaitEnum ); // Dispatch few messages		
				nextTickTime += microSecPerTick;
			}
		
			if (++currentTicks == pConsPerfCfg->ticksPerSec)
					currentTicks = 0;
		}
	}

	stop();	
}

bool ConsumerThread::sendBursts(Int32 &currentTicks, Int32 &postsPerTick, Int32 &postsPerTickRemainder, Int32 &genMsgsPerTick, Int32 &genMsgsPerTickRemainder)
{
	// Send some item requests(assuming dictionaries are ready).
	if( itemsRequestedCount < itemListCount )
	{
		Int32 requestBurstCount;
		requestBurstCount = pConsPerfCfg->_requestsPerTick;
		if (currentTicks > pConsPerfCfg->_requestsPerTickRemainder)
				++requestBurstCount;
		if( !stats.imageRetrievalStartTime )
			stats.imageRetrievalStartTime = perftool::common::GetTime::getTimeNano();
			
			if(sendItemRequestBurst(requestBurstCount) == false)
			{
				testPassed = false;
				failureLocation = "ConsumerThread::sendBursts() - sendItemRequestBurst failed";
				return false;
			}
	}

	if( stats.imageRetrievalEndTime )
	{
		if( postItemList.size() )
		{
			if( sendPostBurst( postsPerTick  + ((currentTicks < postsPerTickRemainder ) ? 1 : 0)) == false)
			{
				testPassed = false;
				failureLocation = "ConsumerThread::sendBursts() - sendPostBurst failed";
				return false;
			}
		}
		if( postItemList.size() )
		{
			if( sendGenMsgBurst( genMsgsPerTick  + ((currentTicks < genMsgsPerTickRemainder ) ? 1 : 0)) == false)
			{
				testPassed = false;
				failureLocation = "ConsumerThread::sendBursts() - sendGenMsgBurst failed";
				return false;
			}
		}
	}
	return true;
}


bool ConsumerThread::sendItemRequestBurst(UInt32 itemBurstCount)
{

	for( UInt32 i = 0; i < itemBurstCount; i++)
	{	
		if( itemsRequestedCount == itemListCount)
			break;
	
		requestMsg.clear();
		requestMsg.interestAfterRefresh( true );
		if( pConsPerfCfg->useServiceId )
			requestMsg.serviceId( desiredService.serviceId );
		else
			requestMsg.serviceName(desiredService.serviceName);

		if( pConsPerfCfg->requestSnapshots || !(itemRequestList[itemsRequestedCount]->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ))
			requestMsg.interestAfterRefresh( false );				

		if( itemRequestList[itemsRequestedCount]->itemInfo.itemFlags & ITEM_IS_GEN_MSG )
			requestMsg.privateStream( true );

		requestMsg.name(itemRequestList[itemsRequestedCount]->itemName);
		requestMsg.domainType( itemRequestList[itemsRequestedCount]->itemInfo.domain);
		itemRequestList[itemsRequestedCount]->pEmaConsumer = pEmaOmmConsumer;
		itemRequestList[itemsRequestedCount]->itemInfo.handle = pEmaOmmConsumer->registerClient(requestMsg, *(itemRequestList[itemsRequestedCount]->itemInfo.pAppClient), (void *)itemRequestList[itemsRequestedCount]);

		stats.requestCount.countStatIncr();
		++itemsRequestedCount;
	}
	return true;
}

bool ConsumerThread::sendPostBurst(UInt32 postItemBurstCount)
{
	fprintf(stdout,"%s\n","sendPostBurst(UInt32 postItemBurstCount) function Not Ready");
	return true;
}

bool ConsumerThread::sendGenMsgBurst(UInt32 genMsgItemBurstCount)
{
	fprintf(stdout,"%s\n","sendGenMsgBurst(UInt32 postItemBurstCount) function Not Ready");
	return true;
}

void ConsumerThread::updateLatencyStats( PerfTimeValue timeTracker, LatencyRecords* pLrec )
{
	TimeRecord ldata;

	ldata.startTime = timeTracker;
	ldata.endTime = perftool::common::GetTime::getTimeMicro();
	ldata.ticks = 1;

	statsMutex.lock();
	pLrec->push_back(ldata); // Submit Time record.
	statsMutex.unlock();
}
void ConsumerThread::getLatencyTimeRecords(LatencyRecords **pUpdateLatList, LatencyRecords *pPostLatList )
{
	statsMutex.lock();

	// pass the current write list pointer so the data can be read
	// and swap read and write pointers
	*pUpdateLatList = pWriteListPtr;
	pWriteListPtr = pReadListPtr;
	pReadListPtr = *pUpdateLatList;
	
	statsMutex.unlock();
}

bool MarketPriceClient::decodeMPUpdate(const rtsdk::ema::access::FieldList& fldList, UInt16 msgtype )
{
	Int64		intType;
	UInt64		uintType;
	float		floatType;
	double		doubleType;
	UInt16		enumType;
	EmaString	acsiiType;

	UInt64 timeTracker = 0;
	UInt64 postTimeTracker = 0;
	UInt64 genMsgTimeTracker = 0;

	while( fldList.forth() )
	{
		const FieldEntry& fe = fldList.getEntry();
		if( fe.getCode() != Data::BlankEnum )
		{
			switch( fe.getLoadType() )
			{
			case DataType::IntEnum :
				intType = fe.getInt();
				break;
			case DataType::UIntEnum :
				uintType = fe.getUInt();
				break;
			case DataType::FloatEnum :
				floatType = fe.getFloat();
				break;
			case DataType::DoubleEnum :
				doubleType = fe.getDouble();
				break;
			case DataType::RealEnum :
				doubleType = fe.getReal().getAsDouble();
				break;
			case DataType::DateEnum :
				{
				const OmmDate &ommDateType = fe.getDate();
				break;
				}
			case DataType::TimeEnum :
				{
				const OmmTime &ommTimeType = fe.getTime();
				break;
				}
			case DataType::DateTimeEnum :
				{
				const OmmDateTime &ommTDateimeType = fe.getDateTime();
				break;
				}
			case DataType::QosEnum :
				{
				const OmmQos &qosType = fe.getQos();
				break;
				}
			case DataType::StateEnum :
				{
				const OmmState &stateType = fe.getState();
				break;
				}
			case DataType::EnumEnum :
				enumType = fe.getEnum();
				break;
			case DataType::BufferEnum :
				{
				const 	EmaBuffer& bufferType = fe.getBuffer();
				break;
				}
			case DataType::AsciiEnum :
				{
				const 	EmaString& asciiType = fe.getAscii();
				break;
				}
			case DataType::Utf8Enum :
				{
				const 	EmaBuffer& bufUtf8Type = fe.getUtf8();
				break;
				}
			case DataType::RmtesEnum :
				{
				const 	RmtesBuffer& bufRmtesType = fe.getRmtes();
				break;			
				}
			default:
				{
				EmaString text = ("Error: Unhandled data type "); 
				text.append( (UInt32) fe.getLoadType() );
				text += " in field with ID ";
				text.append((UInt32) fe.getFieldId());
				AppUtil::logError(text);
				return false;
				}
			}

			if( msgtype == DataType::UpdateMsgEnum  )
			{
				if(fe.getFieldId() == TIM_TRK_1_FID)
					timeTracker = uintType;
				if(fe.getFieldId() == TIM_TRK_2_FID)
					postTimeTracker = uintType;
			}
			else if( msgtype == DataType::GenericMsgEnum  )
			{
				if(fe.getFieldId() == TIM_TRK_3_FID)
					genMsgTimeTracker = uintType;
			}
		}
	}

	if( timeTracker )
	{
		pConsumerThread->updateLatencyStats(timeTracker, pConsumerThread->pWriteListPtr);
		if( postTimeTracker && checkPostUserInfo() )
			pConsumerThread->updateLatencyStats(postTimeTracker, NULL /*pConsumerThread->pWriteListPostPtr*/);
	}
	else if( postTimeTracker && checkPostUserInfo() )
			pConsumerThread->updateLatencyStats(postTimeTracker, NULL /*pConsumerThread->pWriteListPostPtr*/);
	else if( genMsgTimeTracker )
		pConsumerThread->updateLatencyStats(genMsgTimeTracker, NULL /*pConsumerThread->pWriteListGenMsgPtr*/);

	return true;
}


void MarketPriceClient::onRefreshMsg( const rtsdk::ema::access::RefreshMsg& refresh, const rtsdk::ema::access::OmmConsumerEvent&  msgEvent)
{
	
	pConsumerThread->stats.refreshCount.countStatIncr();

	if( !decodeMPUpdate(refresh.getPayload().getFieldList(),  DataType::RefreshMsgEnum) )
	{
		pConsumerThread->stopThread = true;
		pConsumerThread->testPassed = false;
		pConsumerThread->failureLocation = "MarketPriceClient::onRefreshMsg() - decodeMPUpdate failed";
		return;
	}

	if(!pConsumerThread->stats.imageRetrievalEndTime)
	{
	
		OmmState::StreamState	streamState = refresh.getState().getStreamState();
		if( streamState == OmmState::ClosedEnum ||
			streamState == OmmState::ClosedRecoverEnum ||
			streamState == OmmState::ClosedRedirectedEnum )
		{
			EmaString text;
			if( refresh.hasName() )
			{
				text = "Received unexpected final state ";
				text += refresh.getState().toString().c_str();
				text += " in refresh for item: ";
				text += refresh.getName().c_str();
			}
			else
			{
				text = "Received unexpected final state ";
				text += refresh.getState().toString().c_str();
				text += " in refresh for unknown item";
			}
			AppUtil::logError(text);

			pConsumerThread->stopThread = true;
			pConsumerThread->testPassed = false;
			pConsumerThread->failureLocation = "MarketPriceClient::onRefreshMsg() - Failed due to Final streamstate.";
			return;
		}
		if(refresh.getComplete() 
			&& refresh.getState().getDataState() == OmmState::OkEnum )
		{
			ItemRequest *pItemClosure = (ItemRequest*) msgEvent.getClosure();
			// Received a complete refresh.
			pConsumerThread->itemRequestList[pItemClosure->position]->requestState = ITEM_HAS_REFRESH;

			++(pConsumerThread->refreshCompleteCount);
			if (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ )
			{ 					
				if( (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_POST) &&
					pConsumerThread->pConsPerfCfg->postsPerSec )
				{
					pConsumerThread->postItemList.push_back( pConsumerThread->itemRequestList[pItemClosure->position] );
				}
				if( (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_GEN_MSG) &&
					pConsumerThread->pConsPerfCfg->genMsgsPerSec )
				{
					pConsumerThread->genMsgItemList.push_back( pConsumerThread->itemRequestList[pItemClosure->position] );
				}
			}

			if( pConsumerThread->refreshCompleteCount == pConsumerThread->itemListCount )
				pConsumerThread->stats.imageRetrievalEndTime = perftool::common::GetTime::getTimeNano();
		}
	}
}

void MarketPriceClient::onUpdateMsg( const rtsdk::ema::access::UpdateMsg& update, const rtsdk::ema::access::OmmConsumerEvent& msgEvent)
{
	pConsumerThread->stats.imageRetrievalEndTime ? pConsumerThread->stats.steadyStateUpdateCount.countStatIncr() : pConsumerThread->stats.startupUpdateCount.countStatIncr();
	if(!(pConsumerThread->stats.firstUpdateTime))
		pConsumerThread->stats.firstUpdateTime = perftool::common::GetTime::getTimeNano();

	if( !decodeMPUpdate(update.getPayload().getFieldList(),  DataType::UpdateMsgEnum) )
	{
		pConsumerThread->stopThread = true;
		pConsumerThread->testPassed = false;
		pConsumerThread->failureLocation = "MarketPriceClient::onUpdateMsg() - decodeMPUpdate failed.";
		return;
	}
}

void MarketPriceClient::onStatusMsg( const rtsdk::ema::access::StatusMsg& stMsg, const rtsdk::ema::access::OmmConsumerEvent& )
{
	pConsumerThread->stats.statusCount.countStatIncr();
	AppUtil::log(" Received Status on Domain %u with message value: %s\n",
				(UInt32) stMsg.getDomainType(),	stMsg.toString().c_str());
}
void MarketPriceClient::onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent )
{
	AppUtil::log(" Received GenericMsg on Domain %u Processing not ready \n", (UInt32) genericMsg.getDomainType());
}
void MarketPriceClient::onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent ) 
{
	AppUtil::log(" Received AckMsg on Domain %u Processing not ready \n", (UInt32) ackMsg.getDomainType());
}


bool MarketByOrderClient::decodeMBOUpdate(const rtsdk::ema::access::Map& mboMap, UInt16 msgtype  )
{
	UInt64 timeTracker = 0;
	UInt64 postTimeTracker = 0;
	UInt64 genMsgTimeTracker = 0;
	if ( mboMap.getSummaryData().getDataType() == DataType::FieldListEnum )
	{
		if( !decodeFldList( mboMap.getSummaryData().getFieldList(), msgtype, timeTracker, postTimeTracker, genMsgTimeTracker, true ) )
		{
			EmaString text("decodeMBOUpdate failed to decode summary data");
			AppUtil::logError( text );
			return false;
		}
	}

	while( mboMap.forth() )
	{		
		const MapEntry& me = mboMap.getEntry();
		if( me.getKey().getDataType() != DataType::BufferEnum ) 
		{
			EmaString text("decodeMBOUpdate failed MapEntry Key DataType is not BufferEnum.");
			AppUtil::logError( text );
			return false;
		}
		
		if( me.getAction() != MapEntry::DeleteEnum )
		{
			if( me.getLoadType() != DataType::FieldListEnum )
			{
				EmaString text("decodeMBOUpdate failed MapEntry LoadType is not FieldListEnum.");
				AppUtil::logError( text );
				return false;
			}
			if( !decodeFldList( me.getFieldList(), msgtype, timeTracker, postTimeTracker, genMsgTimeTracker, false ) )
			{
				EmaString text("decodeMBOUpdate failed to decode MapEntry FieldList");
				AppUtil::logError( text );
				return false;
			}
		}

	}

	if( timeTracker )
	{
		pConsumerThread->updateLatencyStats(timeTracker, pConsumerThread->pWriteListPtr);
		if( postTimeTracker && checkPostUserInfo() )
			pConsumerThread->updateLatencyStats(postTimeTracker, NULL /*pConsumerThread->pWriteListPostPtr*/);
	}
	else if( postTimeTracker && checkPostUserInfo() )
			pConsumerThread->updateLatencyStats(postTimeTracker, NULL /*pConsumerThread->pWriteListPostPtr*/);
	else if( genMsgTimeTracker )
		pConsumerThread->updateLatencyStats(genMsgTimeTracker, NULL /*pConsumerThread->pWriteListGenMsgPtr*/);


	return true;
}

bool MarketByOrderClient::decodeFldList( const FieldList& fldList, UInt16 msgtype, UInt64 &timeTracker, UInt64 &postTimeTracker,	UInt64 &genMsgTimeTracker, bool summData )
{
	Int64		intType;
	UInt64		uintType;
	float		floatType;
	double		doubleType;
	UInt16		enumType;
	EmaString	acsiiType;

	while( fldList.forth() )
	{
		const FieldEntry& fe = fldList.getEntry();
		if( fe.getCode() != Data::BlankEnum )
		{
			switch( fe.getLoadType() )
			{
			case DataType::IntEnum :
				intType = fe.getInt();
				break;
			case DataType::UIntEnum :
				uintType = fe.getUInt();
				break;
			case DataType::FloatEnum :
				floatType = fe.getFloat();
				break;
			case DataType::DoubleEnum :
				doubleType = fe.getDouble();
				break;
			case DataType::RealEnum :
				doubleType = fe.getReal().getAsDouble();
				break;
			case DataType::DateEnum :
				{
				const OmmDate &ommDateType = fe.getDate();
				break;
				}
			case DataType::TimeEnum :
				{
				const OmmTime &ommTimeType = fe.getTime();
				break;
				}
			case DataType::DateTimeEnum :
				{
				const OmmDateTime &ommTDateimeType = fe.getDateTime();
				break;
				}
			case DataType::QosEnum :
				{
				const OmmQos &qosType = fe.getQos();
				break;
				}
			case DataType::StateEnum :
				{
				const OmmState &stateType = fe.getState();
				break;
				}
			case DataType::EnumEnum :
				enumType = fe.getEnum();
				break;
			case DataType::BufferEnum :
				{
				const 	EmaBuffer& bufferType = fe.getBuffer();
				break;
				}
			case DataType::AsciiEnum :
				{
				const 	EmaString& asciiType = fe.getAscii();
				break;
				}
			case DataType::Utf8Enum :
				{
				const 	EmaBuffer& bufUtf8Type = fe.getUtf8();
				break;
				}
			case DataType::RmtesEnum :
				{
				const 	RmtesBuffer& bufRmtesType = fe.getRmtes();
				break;			
				}
			default:
				{
				EmaString text = ("Error: Unhandled data type "); 
				text.append( (UInt32) fe.getLoadType() );
				text += " in field with ID ";
				text.append((UInt32) fe.getFieldId());
				AppUtil::logError(text);
				return false;
				}
			}
			if( summData )
			{
				if( msgtype == DataType::UpdateMsgEnum  )
				{
					if(fe.getFieldId() == TIM_TRK_1_FID)
						timeTracker = uintType;
					if(fe.getFieldId() == TIM_TRK_2_FID)
						postTimeTracker = uintType;
				}
				else if( msgtype == DataType::GenericMsgEnum  )
				{
					if(fe.getFieldId() == TIM_TRK_3_FID)
						genMsgTimeTracker = uintType;
				}
			}
		}
	}

	return true;
}
void MarketByOrderClient::onRefreshMsg( const rtsdk::ema::access::RefreshMsg& refresh, const rtsdk::ema::access::OmmConsumerEvent&  msgEvent)
{
	pConsumerThread->stats.refreshCount.countStatIncr();

	if( !decodeMBOUpdate(refresh.getPayload().getMap(),  DataType::RefreshMsgEnum) )
	{
		pConsumerThread->stopThread = true;
		pConsumerThread->testPassed = false;
		pConsumerThread->failureLocation = "MarketByOrderClient::onRefreshMsg() - decodeMBOUpdate failed";
		return;
	}

	if(!pConsumerThread->stats.imageRetrievalEndTime)
	{
	
		OmmState::StreamState	streamState = refresh.getState().getStreamState();
		if( streamState == OmmState::ClosedEnum ||
			streamState == OmmState::ClosedRecoverEnum ||
			streamState == OmmState::ClosedRedirectedEnum )
		{
			EmaString text;

			if( refresh.hasName() )
			{
				text = "Received unexpected final state ";
				text += refresh.getState().toString().c_str();
				text += " in refresh for item: ";
				text += refresh.getName().c_str();
			}
			else
			{
				text = "Received unexpected final state ";
				text += refresh.getState().toString().c_str();
				text += " in refresh for unknown item";
			}
			AppUtil::logError(text);

			pConsumerThread->stopThread = true;
			pConsumerThread->testPassed = false;
			pConsumerThread->failureLocation = "MarketByOrderClient::onRefreshMsg() - Failed due to Final streamstate.";
			return;
		}
		if(refresh.getComplete() 
			&& refresh.getState().getDataState() == OmmState::OkEnum )
		{
			ItemRequest *pItemClosure = (ItemRequest*) msgEvent.getClosure();
			// Received a complete refresh.
			pConsumerThread->itemRequestList[pItemClosure->position]->requestState = ITEM_HAS_REFRESH;
			++(pConsumerThread->refreshCompleteCount);
			if (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_STREAMING_REQ )
			{ 					
				if( (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_POST) &&
					pConsumerThread->pConsPerfCfg->postsPerSec )
				{
					pConsumerThread->postItemList.push_back( pConsumerThread->itemRequestList[pItemClosure->position] );
				}
				if( (pConsumerThread->itemRequestList[pItemClosure->position]->itemInfo.itemFlags & ITEM_IS_GEN_MSG) &&
					pConsumerThread->pConsPerfCfg->genMsgsPerSec )
				{
					pConsumerThread->genMsgItemList.push_back( pConsumerThread->itemRequestList[pItemClosure->position] );
				}
			}
		
			if( pConsumerThread->refreshCompleteCount == pConsumerThread->itemListCount )
				pConsumerThread->stats.imageRetrievalEndTime = perftool::common::GetTime::getTimeNano();
		}
	}
}

void MarketByOrderClient::onUpdateMsg( const rtsdk::ema::access::UpdateMsg& update, const rtsdk::ema::access::OmmConsumerEvent& msgEvent)
{
	pConsumerThread->stats.imageRetrievalEndTime ? pConsumerThread->stats.steadyStateUpdateCount.countStatIncr() : pConsumerThread->stats.startupUpdateCount.countStatIncr();
	if(!(pConsumerThread->stats.firstUpdateTime))
		pConsumerThread->stats.firstUpdateTime = perftool::common::GetTime::getTimeNano();

	if( !decodeMBOUpdate(update.getPayload().getMap(),  DataType::UpdateMsgEnum) )
	{
		pConsumerThread->stopThread = true;
		pConsumerThread->testPassed = false;
		pConsumerThread->failureLocation = "MarketByOrderClient::onUpdateMsg() - decodeMBOUpdate failed.";
		return;
	}
}

void MarketByOrderClient::onStatusMsg( const rtsdk::ema::access::StatusMsg& stMsg, const rtsdk::ema::access::OmmConsumerEvent& )
{
	pConsumerThread->stats.statusCount.countStatIncr();
	AppUtil::log(" Received Status on Domain %u with message value: %s\n",
				(UInt32) stMsg.getDomainType(),	stMsg.toString().c_str());
}
void MarketByOrderClient::onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent )
{
	AppUtil::log(" Received GenericMsg on Domain %u Processing not ready \n", (UInt32) genericMsg.getDomainType());
}
void MarketByOrderClient::onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent ) 
{
	AppUtil::log(" Received AckMsg on Domain %u Processing not ready \n", (UInt32) ackMsg.getDomainType());
}


void DirectoryClient::decode( const rtsdk::ema::access::Map& srcMap)
{	
	bool	gotDesiredSvc = false;	 
	while( srcMap.forth() )
	{		
		const MapEntry& me = srcMap.getEntry();
		if( me.getKey().getDataType() == DataType::UIntEnum)
			pConsThread->desiredService.serviceId = (UInt32) me.getKey().getUInt();
		if(me.getLoadType() == DataType::FilterListEnum)
		{
			const FilterList& fl = me.getFilterList();
			while (fl.forth())
			{
				const FilterEntry& fe = fl.getEntry();
				switch (fe.getFilterId())
				{
				case SERVICE_INFO_FILTER:
				{
					if(fe.getLoadType() == DataType::ElementListEnum)
						decodeInfo(fe.getElementList(), gotDesiredSvc);
					break;
				}
				case SERVICE_STATE_FILTER:
				{
					if(fe.getLoadType() == DataType::ElementListEnum 
						&& gotDesiredSvc)
						decodeState(fe.getElementList()); // Decode state only if svc is desired.
					break;
				}
				default:
					break;
				}		
			}
		}
	}
}


void DirectoryClient::decodeInfo( const rtsdk::ema::access::ElementList& elist, bool &gotDesiredSvc)
{
	while (elist.forth())
	{
		const ElementEntry& ee = elist.getEntry();
		if(ee.getName() == ENAME_NAME)
		{
			if(ee.getAscii() == pConsThread->pConsPerfCfg->serviceName)	
			{
				pConsThread->desiredService.serviceName = ee.getAscii();
				gotDesiredSvc = true;
			}
		}
	}
}
void DirectoryClient::decodeState( const rtsdk::ema::access::ElementList& elist)
{
	while (elist.forth())
	{
		const ElementEntry& ee = elist.getEntry();
		if( ee.getName() == ENAME_SVC_STATE )
			pConsThread->desiredService.serviceStateUp = ee.getUInt() == 1 ? true : false;
		else if( ee.getName() == ENAME_ACCEPTING_REQS )
		{
			pConsThread->desiredService.acceptingReq = ee.getUInt() == 1 ? true : false;
		}
	}
}
void DirectoryClient::onRefreshMsg( const rtsdk::ema::access::RefreshMsg& refreshMsg, const rtsdk::ema::access::OmmConsumerEvent& ommConsEvent)
{
	if(refreshMsg.getState().getStreamState() == OmmState::OpenEnum && 
		refreshMsg.getState().getDataState() == OmmState::OkEnum )
	{
		if( refreshMsg.getPayload().getDataType() == DataType::MapEnum)
		{
			decode(refreshMsg.getPayload().getMap());
			pConsThread->isDesiredServiceUp = pConsThread->desiredService.isDesiredServiceUp();
		}
	}
}

void DirectoryClient::onUpdateMsg( const rtsdk::ema::access::UpdateMsg& updMsg, const rtsdk::ema::access::OmmConsumerEvent& ommConsEvent)
{
	if( updMsg.getPayload().getDataType() == DataType::MapEnum)
	{
		decode(updMsg.getPayload().getMap());
		pConsThread->isDesiredServiceUp = pConsThread->desiredService.isDesiredServiceUp();
	}
}

void DirectoryClient::onStatusMsg( const rtsdk::ema::access::StatusMsg& stMsg, const rtsdk::ema::access::OmmConsumerEvent& ommConsEvent)
{
	AppUtil::log("%s%d Received Directory Status %s\n", BASECONSUMER_NAME, pConsThread->consumerThreadIndex, stMsg.toString().c_str());
}
