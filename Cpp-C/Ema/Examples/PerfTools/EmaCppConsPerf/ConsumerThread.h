///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

#ifndef _CONSUMER_THREADS_H
#define _CONSUMER_THREADS_H

#if defined(WIN32)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#include <assert.h>

#if defined(WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <process.h>
#include <windows.h>
#else

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

#include "ConsPerfConfig.h"
#include "../Common/Statistics.h"
#include "../Common/GetTime.h"
#include "../Common/AppVector.h"
#include "../Common/Mutex.h"
#include "../Common/AppUtil.h"
#include "../Common/ThreadBinding.h"

#define		BASECONSUMER_NAME "Consumer_"
#define		CONSUMER_NAME_WSJSON "Consumer_WSJSON_"
#define		CONSUMER_NAME_WSRWF "Consumer_WSRWF_"

class EmaCppConsPerf;
class ConsumerThread;
class ItemRequest;
typedef perftool::common::AppVector<ItemRequest*> ItemRequestList;
typedef perftool::common::AppVector<ItemRequest*> PostGenMsgItemList;
typedef perftool::common::AppVector< TimeRecord > LatencyRecords;
class ConsumerStats
{
public:
	ConsumerStats();
	~ConsumerStats();
	TimeValue	imageRetrievalStartTime;		// Time at which first item request was made. 
	TimeValue	imageRetrievalEndTime;			// Time at which last item refresh was received. 
	TimeValue	firstUpdateTime;				// Time at which first item update was received. 
	TimeValue	firstGenMsgSentTime;			// Time at which first generic message was sent 
	TimeValue	firstGenMsgRecvTime;			// Time at which first generic message was received 


	CountStat		refreshCount;				// Number of item refreshes received. 
	CountStat		startupUpdateCount;			// Number of item updates received during startup. 
	CountStat		steadyStateUpdateCount;		// Number of item updates received during steady state. 
	CountStat		requestCount;				// Number of requests sent. 
	CountStat		statusCount;				// Number of item status messages received. 
	CountStat		postSentCount;				// Number of posts sent. 
	CountStat		postOutOfBuffersCount;		// Number of posts not sent due to lack of buffers. 
	CountStat		genMsgSentCount;			// Number of generic msgs sent. 
	CountStat		genMsgRecvCount;			// Number of generic msgs received. 
	CountStat		latencyGenMsgSentCount;		// Number of latency generic msgs sent. 
	CountStat		genMsgOutOfBuffersCount;	// Number of generic msgs not sent due to lack of buffers. 
	ValueStatistics	intervalLatencyStats;		// Latency statistics (recorded by stats thread). 
	ValueStatistics	intervalPostLatencyStats;	// Post latency statistics (recorded by stats thread). 
	ValueStatistics	intervalGenMsgLatencyStats;	// Gen Msg latency statistics (recorded by stats thread). 

	ValueStatistics startupLatencyStats;		// Statup latency statistics. 
	ValueStatistics steadyStateLatencyStats;	// Steady-state latency statistics. 
	ValueStatistics overallLatencyStats;		// Overall latency statistics. 
	ValueStatistics postLatencyStats;			// Posting latency statistics. 
	ValueStatistics genMsgLatencyStats;			// Gen Msg latency statistics. 
	bool		imageTimeRecorded;				// Stats thread sets this once it has recorded/printed
												// this consumer's image retrieval time. 
};
class ServiceInfo {

public:
	ServiceInfo() : serviceId(0), serviceStateUp( false ), acceptingReq( true)  {};
	~ServiceInfo() {};
	EmaString	serviceName;
	UInt32 serviceId;
	bool	serviceStateUp;
	bool	acceptingReq;	
	
	bool isDesiredServiceUp()
	{
		if( serviceStateUp && acceptingReq )
			return true;
		return false;
	};
	void dumpServiceInfo()
	{
		fprintf(stdout,"ServiceName: %s, ServiceId: %u, serviceStateUp: %s, acceptingReq: %s, isDesiredServiceUp: %s",
			serviceName.c_str(), serviceId, 
			( serviceStateUp ? "true" : "false"), ( acceptingReq ? "true" : "false"), ( isDesiredServiceUp() ? "true" : "false"));
	}
};
// DirectoryHandler
class DirectoryClient : public thomsonreuters::ema::access::OmmConsumerClient
{
public :
	DirectoryClient() : pConsThread( NULL ) {};
	void decode( const thomsonreuters::ema::access::Map& ); 
	void decodeInfo( const thomsonreuters::ema::access::ElementList& elist, bool &gotDesiredSvc);
	void decodeState( const thomsonreuters::ema::access::ElementList& elist);
	void init( ConsumerThread *pConsThr );
protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	ConsumerThread *pConsThread;
};

// application defined client class for receiving and processing of item messages
class MarketPriceClient : public thomsonreuters::ema::access::OmmConsumerClient
{
public :
	MarketPriceClient() : pConsumerThread( NULL ) {};
	void init( ConsumerThread *pConsThr );

	bool decodeMPUpdate( const thomsonreuters::ema::access::FieldList&, UInt16 msgtype  );
	bool checkPostUserInfo() { return true; };

protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );
	
	void onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent );

	void onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent );

	ConsumerThread *pConsumerThread;
};

// application defined client class for receiving and processing of item messages
class MarketByOrderClient : public thomsonreuters::ema::access::OmmConsumerClient
{
public :
	MarketByOrderClient() : pConsumerThread( NULL ) {};
	void init( ConsumerThread *pConsThr );

	bool decodeMBOUpdate(const thomsonreuters::ema::access::Map&, UInt16 msgtype );
	bool decodeFldList( const FieldList& fldList, UInt16 msgtype, UInt64 &timeTracker, UInt64 &postTimeTracker,	UInt64 &genMsgTimeTracker, bool summData = false );
	bool checkPostUserInfo() { return true; };

protected :

	void onRefreshMsg( const thomsonreuters::ema::access::RefreshMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const thomsonreuters::ema::access::UpdateMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const thomsonreuters::ema::access::StatusMsg&, const thomsonreuters::ema::access::OmmConsumerEvent& );
	
	void onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent );

	void onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent );

	ConsumerThread *pConsumerThread;
};

typedef enum
{
	ITEM_NOT_REQUESTED,			// Item request has not been set. 
	ITEM_WAITING_FOR_REFRESH,	// Item is waiting for its solicited refresh. 
	ITEM_HAS_REFRESH			// Item has received its solicited refresh. 
} ItemRequestState;

typedef enum {
	ITEM_IS_STREAMING_REQ		= 0x04,		/* Provider should send updates */
	ITEM_IS_SOLICITED			= 0x10,		/* Item was requested(not published) */
	ITEM_IS_POST				= 0x20,		/* Consumer should send posts */
	ITEM_IS_GEN_MSG				= 0x40,		/* Consumer should send generic messages */
	ITEM_IS_PRIVATE				= 0x80		/* Item should be requested on private stream */
} ItemFlags;

class ItemInfo
{
public:
	ItemInfo() : handle(0), StreamId(0), domain( thomsonreuters::ema::rdm::MMT_MARKET_PRICE ), 
				itemFlags ( ITEM_IS_SOLICITED ),itemData( NULL), pAppClient(NULL) {}; 
	UInt64		handle;
	Int32		StreamId; 
	UInt16		domain;
	OmmConsumerClient	*pAppClient;
	UInt8		itemFlags;	// See ItemFlags struct 
	void*		itemData; // Is it Closure???. Holds information about the item's data. This data will be different depending on the domain of the item. 
};

class ItemRequest
{
public:
	ItemRequest(): requestState( ITEM_NOT_REQUESTED ), pEmaConsumer( NULL ) {};
	UInt64		position; // Link for item list.
	ItemRequestState requestState;	// Item state. 
	EmaString	itemName;
	ItemInfo	itemInfo;
	OmmConsumer	*pEmaConsumer;
};


class ConsumerThread 
{
	friend class EmaCppConsPerf;
	friend class MarketPriceClient;
	friend class MarketByOrderClient;
	friend class DirectoryClient;
public:
	ConsumerThread( ConsPerfConfig& );
	void consumerThreadInit( ConsPerfConfig&, Int32 consIndex);
	virtual ~ConsumerThread();
	bool initialize();

	void start();

	void stop();

	void run();

	bool sendBursts(Int32 &currentTicks, Int32 &postsPerTick, Int32 &postsPerTickRemainder, Int32 &genMsgsPerTick, Int32 &genMsgsPerTickRemainder );

	bool sendItemRequestBurst(UInt32 itemBurstCount);

	bool sendPostBurst(UInt32 postItemBurstCount);

	bool sendGenMsgBurst(UInt32 genMsgItemBurstCount);

	void updateLatencyStats( TimeValue timeTracker, LatencyRecords* pLrec );

	void getLatencyTimeRecords(LatencyRecords **pUpdateList, LatencyRecords *pPostList = NULL );

	void clearReadLatTimeRecords(LatencyRecords *pReadList ) { pReadList->clear(); };

protected:
	
	const ConsPerfConfig*   pConsPerfCfg;
	Int32				consumerThreadIndex;
	OmmConsumer			*pEmaOmmConsumer;

	DirectoryClient		srcClient;
	ServiceInfo			desiredService;
	bool				isDesiredServiceUp;
	UInt64				desiredServiceHandle;

	MarketPriceClient   mPriceClient;
	MarketByOrderClient mByOrderClient;

	Int32				itemListUniqueIndex;		// Index into the item list at which item 
												// requests unique to this consumer start.
	Int32				itemListCount;				// Number of item requests to make.
	ConsumerStats		stats;				// Other stats, collected periodically by the main thread. */
	FILE				*statsFile;			// File for logging stats for this connection. */
	FILE				*latencyLogFile;	// File for logging latency for this connection. */
	Int32				cpuId;
	Int32				apiThreadCpuId;
	ItemRequestList		itemRequestList;
	PostGenMsgItemList	postItemList;
	PostGenMsgItemList	genMsgItemList;
	bool				stopThread;
	bool				running;

	ReqMsg				requestMsg;
	Int32				itemsRequestedCount;
	Int32				refreshCompleteCount;

	perftool::common::Mutex	statsMutex;
	// collection of update latency numbers
	LatencyRecords			updateLatList1;
	LatencyRecords			updateLatList2;
	LatencyRecords*			pWriteListPtr;
	LatencyRecords*			pReadListPtr;

	bool					testPassed;
	EmaString				failureLocation;

#if defined(WIN32)
	static unsigned __stdcall ThreadFunc( void* pArguments );

	HANDLE					_handle;
	unsigned int			_threadId;
#else
	static void *ThreadFunc( void* pArguments );

	pthread_t				_threadId;
#endif
private:
	void dumpConsumerItemList();
};
 
inline void MarketPriceClient::init( ConsumerThread *pConsThr )
{
	pConsumerThread = pConsThr;
}

inline void MarketByOrderClient::init( ConsumerThread *pConsThr )
{
	pConsumerThread = pConsThr;
}

inline void DirectoryClient::init( ConsumerThread *pConsThr )
{
	pConsThread = pConsThr;
}
#endif // _CONSUMER_THREADS_H
