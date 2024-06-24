/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2024 LSEG. All rights reserved.
*/

#ifndef WL_ITEM_H
#define WL_ITEM_H

#include "rtr/wlBase.h"
#include "rtr/wlView.h" 
#include "rtr/rsslRequestMsg.h"
#include "rtr/wlService.h"
#include "rtr/wlMsgReorderQueue.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WlService;
struct WlItemGroup;
struct WlFTGroup;

typedef struct WlItemRequest WlItemRequest;
typedef struct WlItemStream WlItemStream;
typedef struct WlItemGroup WlItemGroup;
typedef struct WlFTGroup WlFTGroup;
typedef struct WlItems WlItems;
struct WlSymbolListRequest;

typedef enum
{
	WL_ISRS_NONE						= 0,	/* No refresh needed. */
	WL_ISRS_PENDING_OPEN_WINDOW			= 1,	/* Need to request a refresh,
												 * but currently in excess of service OpenWindow. */
	WL_ISRS_REQUEST_REFRESH				= 2,	/* Need to request a refresh. */
	WL_ISRS_PENDING_REFRESH				= 3,	/* Currently waiting for a refresh .*/
	WL_ISRS_PENDING_REFRESH_COMPLETE	= 4		/* Recevied partial refresh, need the rest. */
} WlItemStreamRefreshState;

/* Indicates current status of the stream. */
typedef enum
{
	WL_IOSF_NONE						= 0x000,
	WL_IOSF_PENDING_SNAPSHOT			= 0x001,	/* Stream is currently waiting for a snapshot. */
	WL_IOSF_PAUSED						= 0x002,	/* Stream is currently paused. */
	WL_IOSF_PENDING_PRIORITY_CHANGE		= 0x004,	/* Stream needs to change priority. */
	WL_IOSF_VIEWED						= 0x008,	/* Stream's view is currently active. */
	WL_IOSF_PENDING_VIEW_CHANGE			= 0x010,	/* Stream needs to update its view. */
	WL_IOSF_PENDING_VIEW_REFRESH		= 0x020,	/* Stream is expecting a refresh that changes the view. */
	WL_IOSF_PRIVATE						= 0x040,	/* Stream is private. */
	WL_IOSF_ESTABLISHED					= 0x080,	/* Stream is established and can receive generic/post messages. */
	WL_IOSF_HAS_BC_SEQ_NUM				= 0x100,	/* WlItemStream::seqNum contains the last received broadcast sequence number (supersedes WL_IOSF_HAS_UC_SEQ_NUM). */
	WL_IOSF_HAS_UC_SEQ_NUM				= 0x200,	/* WlItemStream::seqNum contains the first unicast sequence number. */
	WL_IOSF_HAS_BC_SEQ_GAP				= 0x400,	/* Stream has detected a gap. */
	WL_IOSF_HAS_PART_GAP				= 0x800,	/* Stream has detected a gap in refresh parts. */
	WL_IOSF_BC_BEHIND_UC				= 0x1000,	/* Broadcast stream is behind unicast stream. */
	WL_IOSF_HAS_BC_SYNCH_SEQ_NUM		= 0x2000,	/* WlItemStream::bcSynchSeqNum contains the sequence number of a broadcast message that was used to syncrhonize. */
	WL_IOSF_CLOSED						= 0x4000,	/* If closing this stream, do we need to send a close upstream? */
	WL_IOSF_QUALIFIED					= 0x8000	/* Stream is qualified. */
} WlItemStreamFlags;

/* Maintains information about a stream open on the network. */
struct WlItemStream
{
	WlStreamBase		base;
	RsslQueueLink		qlOpenWindow;
	RsslQueueLink		qlItemGroup;
	RsslQueueLink		qlServiceStreams;
	RsslQueueLink		qlFTGroup;
	RsslQueueLink		qlGap;
	RsslHashLink		hlStreamsByAttrib;
	RsslUInt32			flags;						/* WlItemStreamFlags. */
	RsslUInt8			refreshState;				/* WlItemStreamRefreshState. */
	WlStreamAttributes	streamAttributes;			/* Stream attributes. */
	RsslUInt8			priorityClass;				/* Stream's current priority class. */
	RsslUInt16			priorityCount;				/* Stream's current priority count. */
	RsslQueue			requestsRecovering;			/* Requests waiting to send a request message. */
	RsslQueue			requestsPendingRefresh;		/* Requests waiting for a refresh. */
	RsslQueue			requestsOpen;				/* Requests that have received their complete refresh. */
	RsslUInt32			requestsStreamingCount;		/* Number of requests that are streaming. */
	RsslUInt32			requestsPausedCount;		/* Number of streaming requests that requested pause. */
	RsslUInt32			requestsWithViewCount;		/* Number of requests that have a view. */
	WlItemGroup			*pItemGroup;				/* Item group associated with this stream. */
	WlFTGroup			*pFTGroup;					/* FTGroup associated with this stream. */
	WlAggregateView		*pAggregateView;			/* The stream's view, aggregated from requests. */
	WlService			*pWlService;				/* Service associated with this stream. */
	char				*msgKeyMemoryBuffer;		/* Memory for the stream's message key. */
	RsslUInt32			seqNum;						/* Sequence number (meaning is determined by WL_IOSF_HAS_UC_SEQ_NUM and WL_IOSF_HAS_BC_SEQ_NUM) */
	RsslUInt32			bcSynchSeqNum;				/* Sequence number of the last broadcast message used for synchronization. */
	RsslUInt16			nextPartNum;				/* Next expected refresh partNum. */
	WlMsgReorderQueue	bufferedMsgQueue;           /* Multicast message synch queue. */
	WlItemRequest		*pRequestWithExtraInfo;		/* If present, use the extendedHeader and
													 * encDataBody, if any, from this request. */
	RsslBool			itemIsClosedForAllStandby; /* This is used by the active server whether the item is closed for all standby servers. */
};

/* Initializes an item stream. */
RsslRet wlItemStreamInit(WlItemStream *pItemStream, WlStreamAttributes *pStreamAttributes,
		RsslInt32 streamId, RsslErrorInfo *pErrorInfo);

/* Resets a stream to its initial state (generally used when transitioning a stream
 * from non-streaming to streaming). */
void wlItemStreamResetState(WlItemStream *pItemStream);


/* Creates an item stream. */
WlItemStream *wlCreateItemStream(WlBase *pBase, WlStreamAttributes *pStreamAttributes, RsslErrorInfo *pErrorInfo);

/* Destroys an item stream. */
void wlItemStreamDestroy(WlBase *pBase, WlItemStream *pItemStream);

/* Aggregates priority from stream's requests. */
RsslBool wlItemStreamMergePriority(WlItemStream *pItemStream, RsslUInt8 *pPriorityClass,
		RsslUInt16 *pPriorityCount);

typedef enum
{
	WL_IRQF_NONE 			= 0x00,	/* None. */
	WL_IRQF_HAS_STATIC_QOS	= 0x01,	/* Request has established a static QoS. */
	WL_IRQF_PRIVATE			= 0x02,	/* Request is for a private stream. */
	WL_IRQF_PROV_DRIVEN		= 0x04,	/* Request is provider-driven, and should be unique among
									 * provider-driven requests. */
	WL_IRQF_REFRESHED		= 0x08,	/* Request has received a full refresh. */
	WL_IRQF_BATCH			= 0x10,	/* Request is a batch request and needs acknowledgement. */
	WL_IRQF_HAS_PROV_KEY	= 0x20,	/* Request is provider driven but has received a message with 
									 * a key. */
	WL_IRQF_QUALIFIED		= 0x40	/* Request is for a qualified stream. */
} WlItemRequestFlags;

struct WlItemRequest
{
	WlRequestBase			base;
	RsslHashLink			hlProviderRequestsByAttrib;
	RsslQueueLink			qlPendingRefresh;
	RsslQueueLink			qlRequestedService;
	RsslUInt16				flags;					/* WlItemRequestFlags */
	RsslUInt16				requestMsgFlags;		/* RsslRequestFlags */
	RsslMsgKey				msgKey;					/* Key associated with this request. */
	WlRequestedService		*pRequestedService;		/* Service criteria of this request. */
	RsslUInt8				containerType;			/* Container type of the request. */
	RsslUInt8				priorityClass;			/* Priority class of this request. */
	RsslUInt16				priorityCount;			/* Priority count of this request. */
	RsslQos					qos;					/* Qos of this request, if any. */
	RsslQos					worstQos;				/* Worst Qos of this request, if any. */
	RsslQos					staticQos;				/* Static QoS, if established. */
	char					*msgKeyMemoryBuffer;	/* MsgKey memory. */
	WlView					*pView;					/* View set by this request. */
	RsslBuffer				encDataBody;			/* Encoded dataBody. */
	RsslBuffer				extendedHeader;			/* Extended header, if any. */
};

/* Closes an item request. */
void wlItemRequestClose(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest);

/* Destroys an item request. */
void wlItemRequestDestroy(WlBase *pBase, WlItemRequest *pItemRequest);

/* Attempts to find a suitable QoS in the given list. */
const RsslQos *wlItemRequestMatchQos(WlItemRequest *pItemRequest, RsslQos *qosList, 
		RsslUInt32 qosCount);

/* Sets the request to use a only a single QoS, if appropriate. Use when a stream has been
 * established(i.e. the item has requested on a given QoS and received a refresh). */
void wlItemRequestEstablishQos(WlItemRequest *pItemRequest, RsslQos *pQos);

/* Creates a copy of the given MsgKey. */
RsslRet wlItemCopyKey(RsslMsgKey *pNewMsgKey, RsslMsgKey *pOldMsgKey, char **pMemoryBuffer,
		RsslErrorInfo *pErrorInfo);

RsslUInt32 wlProviderRequestHashSum(void *pKey);

/* Adds the view of a request to a stream. */
RsslRet wlItemStreamAddRequestView(WlItemStream *pItemStream, WlItemRequest *pItemRequest, 
		RsslErrorInfo *pErrorInfo);

/* Removes the view of a request from a stream. */
void wlItemStreamRemoveRequestView(WlItemStream *pItemStream, WlItemRequest *pItemRequest);

/* Indicates if there are requests associated with a stream. */
RsslBool wlItemStreamHasRequests(WlItemStream *pItemStream);

/* Hash sum for provider-driven requests. */
RsslUInt32 wlProviderRequestHashSum(void *pKey);

/* Hash comparison for provider-driven requests. */
RsslBool wlProviderRequestHashCompare(void *pKey1, void *pKey2);

/* ID of a group of streams. */
struct WlItemGroup
{
	RsslHashLink	hlItemGroupTable;
	RsslQueueLink	qlItemGroups;
	WlService		*pWlService;		/* Service associated with this group. */
	RsslBuffer		groupId;			/* The Group's ID. */
	RsslQueue		openStreamList;		/* List of streams that have this Group ID. */
};


/* Adds an item stream to a group. */
RsslRet wlItemGroupAddStream(WlItems *pItems, RsslBuffer *pGroupId, WlItemStream *pItemStream,
		RsslErrorInfo *pErrorInfo);

/* Merges/Renames one item group to another. */
void wlItemGroupMerge(WlItemGroup *pItemGroup, WlService *pWlService, RsslBuffer *pNewGroupId);

/* Removes a stream from a group. */
static void wlItemGroupRemoveStream(WlItems *pItems, WlItemGroup *pItemGroup, WlItemStream *pItemStream);

/* Removes an item group from the table of groups and destroys it (used during general cleanup). */
void wlItemGroupRemove(WlItemGroup *pItemGroup);

/* Represents a fault-tolerant multicast group. */
struct WlFTGroup
{
	RsslQueueLink	qlGroups;			/* Group list list. */
	RsslUInt8		ftGroupId;			/* FTGroup ID. */
	RsslQueue		openStreamList;		/* List of streams provided by this group. */
	RsslInt64		expireTime;			/* Next time before which we should receive a ping for
										 * this group. */
};

/* Adds an item stream to an FTGroup. Creates the FTGroup if it does not exist. */
RsslRet wlFTGroupAddStream(WlBase *pBase, WlItems *pItems, RsslUInt8 ftGroupId, 
		WlItemStream *pItemStream, RsslErrorInfo *pErrorInfo);

/* Removes an item stream from an FTGroup. Destroys the FTGroup if it is emptied. */
void wlFTGroupRemoveStream(WlItems *pItems, WlItemStream *pItemStream);

/* Removes an FTGroup from the table of groups and destroys it (used during general cleanup). */
void wlFTGroupRemove(WlItems *pItems, WlFTGroup *pGroup);


/* Handles item requests. */
#define WL_FTGROUP_TABLE_SIZE 256
struct WlItems
{
	RsslHashTable	providerRequestsByAttrib;	/* Provider-driven streams. */
	WlFTGroup*		ftGroupTable[WL_FTGROUP_TABLE_SIZE];
												/* FTGroup table. */
	RsslQueue		ftGroupTimerQueue;			/* FTGroup list. Should be ordered
												 * by each group's expireTime. */
	RsslQueue		gapStreamQueue;				/* Streams that have detected a gap. */
	RsslInt64		gapExpireTime;				/* Time at which streams begin any recovery from
												 * gaps. */
	WlItemStream	*pCurrentFanoutStream;		/* Used to detect a close of the current stream while
												 * fanning out. */
	WlItemGroup		*pCurrentFanoutGroup;
	WlFTGroup		*pCurrentFanoutFTGroup;
};

/* Initializes the WlItems structure. */
RsslRet wlItemsInit(WlItems *pItems, RsslErrorInfo *pErrorInfo);

/* Cleans up the WlItems structure. */
void wlItemsCleanup(WlItems *pItems);

/* View actions. Indicates what to do with the given view, if any. */
typedef enum
{
	WL_IVA_NONE				= 0,	/* No view. If a view is present, remove it. */
	WL_IVA_SET				= 1,	/* Set the given view. */
	WL_IVA_MAINTAIN_VIEW	= 2		/* Maintain the current view. */
} RsslReactorViewActionOptions;

typedef struct
{
	RsslRequestMsg	*pRequestMsg;
	RsslBuffer		*pServiceName;
	RsslUInt32		viewAction;
	RsslUInt		viewType;
	void*			viewElemList;
	RsslUInt32		viewElemCount;
	void			*pUserSpec;
	RsslUInt		slDataStreamFlags;
	RsslUInt32		majorVersion;
	RsslUInt32		minorVersion;
} WlItemRequestCreateOpts;

RTR_C_INLINE void wlClearItemRequestCreateOptions(WlItemRequestCreateOpts *pOpts)
{
	memset(pOpts, 0, sizeof(WlItemRequestCreateOpts));
}

/* Creates an item request. */
RsslRet wlItemRequestCreate(WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts, 
		RsslErrorInfo *pErrorInfo);

/* Initialization function for an item request.  wlItemRequestCreate calls this (more
 * specialized classes of items may call this). */
RsslRet wlItemRequestInit(WlItemRequest *pItemRequest, WlBase *pBase, WlItems *pItems, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo);

/* Cleans up an item request. */
RsslRet wlItemRequestCleanup(WlItemRequest *pItemRequest);

/* Reissues an item request. */
RsslRet wlItemRequestReissue(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest,
		WlItemRequestCreateOpts *pOpts, RsslErrorInfo *pErrorInfo);

/* Attempts to find a stream for a request. Joins a stream if one exists, otherwise
 * creates one if service indicates the request can be made. */
RsslRet wlItemRequestFindStream(WlBase *pBase, WlItems *pItems, WlItemRequest *pItemRequest, 
		RsslErrorInfo *pErrorInfo, RsslBool generateStatus);

/* Adds a request to an item stream. */
RsslRet wlItemStreamAddRequest(WlBase *pBase, WlItems *pItems, WlItemStream *pItemStream,
		WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo);

/* Removes a request from an item stream. */
void wlItemStreamRemoveRequest(WlItemStream *pItemStream, WlItemRequest *pItemRequest);

/* Closes an item stream. */
RsslRet wlItemStreamClose(WlBase *pBase, WlItems *pItems, WlItemStream *pItemStream,
		RsslErrorInfo *pErrorInfo);

/* Sends a message to an item request. */
RsslRet wlItemRequestSendMsgEvent(WlBase *pBase,
		RsslWatchlistMsgEvent *pEvent, WlItemRequest *pItemRequest, RsslErrorInfo *pErrorInfo);

/* Creates a message event and sends it to an item request. */
RsslRet wlItemRequestSendMsg(WlBase *pBase,
		WlItemRequest *pItemRequest, RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo);

/* Sets an item stream to request a message. */
void wlItemStreamSetMsgPending(WlBase *pBase, WlItemStream *pItemStream, RsslBool requestRefresh);

/* Removes an item stream from the message pending queue. */
void wlItemStreamUnsetMsgPending(WlBase *pBase, WlItemStream *pItemStream);

/* Checks if requests can be made due to room in a service's OpenWindow. */
void wlItemStreamCheckOpenWindow(WlBase *pBase, WlService *pWlService);

/* Processes a refresh complete for an item stream. */
void wlItemStreamProcessRefreshComplete(WlBase *pBase, WlItemStream *pItemStream);

/* Decodes a view from an RsslRequestMsg payload. */
RsslRet wlExtractViewFromMsg(WlBase *pBase, WlItemRequestCreateOpts *pOpts,
		RsslErrorInfo *pErrorInfo);

/* Decodes view data from an RsslRequestMsg payload. */
static RsslRet wlDecodeViewData(WlBase *pBase, WlItemRequestCreateOpts *pOpts,
	RsslDecodeIterator* dIter, RsslErrorInfo *pErrorInfo);

#ifdef __cplusplus
}
#endif


#endif

