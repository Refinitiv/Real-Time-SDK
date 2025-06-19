/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef WL_DIRECTORY_H
#define WL_DIRECTORY_H

#include "rtr/wlBase.h"
#include "rtr/rsslMsgKey.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslQos.h"
#include "rtr/wlService.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Flags indicating attributes about the directory request. */
typedef enum
{
	WL_DRQF_NONE 			= 0x00,
	WL_DRQF_IS_STREAMING	= 0x01	/* Request is streaming. */
} WlDirectoryRequestFlags;


/* State of the directory request. */
typedef enum
{
	WL_DR_REQUEST_PENDING_REFRESH	= 1,	/* Request should receive a refresh. */
	WL_DR_REQUEST_OK				= 2		/* Request has received a refresh. */
} WlDirectoryRequestState;

/* Represents a directory request. */
typedef struct
{
	WlRequestBase			base;			/* Base request. */
	RsslUInt8				flags;			/* WlDirectoryRequestFlags. */
	RsslUInt32				filter;			/* filter from request so correct refresh can be returned to user */
	RsslQueueLink			qlRequestedService;
	WlRequestedService		*pRequestedService;	/* Service associated with this request, if any
												 * (otherwise the request is for all services). */
	WlDirectoryRequestState	state;			/* WlDirectoryRequestState */
} WlDirectoryRequest;

/* Represents a directory stream. */
typedef struct
{
	WlStreamBase	base;
} WlDirectoryStream;

/* Creates a directory request. */
WlDirectoryRequest *wlDirectoryRequestCreate(RsslRDMDirectoryRequest *pDirectoryReqMsg,
	   void *pUserSpec, RsslErrorInfo *pErrorInfo);

/* Destroys a directory request. */
void wlDirectoryRequestDestroy(WlDirectoryRequest *pDirectoryRequest);

typedef struct
{
	WlDirectoryStream		*pStream;	/* The directory stream. All directory requests
										 * are served out of this stream. */
	RsslQueue				requests;	/* List of directory requests for all services. */
	RsslQueue				openDirectoryRequests;	/* Requests which have been served by the cache
													 * and can receive updates from the directory stream. */
} WlDirectory;

/* Creates & opens a directory stream. */
WlDirectoryStream *wlDirectoryStreamCreate(WlBase *pBase, WlDirectory *pDirectory, 
		RsslErrorInfo *pErrorInfo);

/* Closes a directory stream. */
void wlDirectoryStreamClose(WlBase *pBase, WlDirectory *pDirectory, RsslBool sendCloseMsg);

/* Destroys a directory stream. */
void wlDirectoryStreamDestroy(WlDirectoryStream *pDirectoryStream);

/* Sends a directory message to a request. */
RsslRet wlSendDirectoryMsgToRequest(WlBase *pBase, WlDirectoryRequest *pDirectoryRequest,
	RsslRDMDirectoryMsg *pDirectoryMsg, RsslMsg *pRsslMsg, RsslErrorInfo *pErrorInfo);

/* Prepares a directory message from the given list of services, and sends it to a request. */
RsslRet wlSendServiceListToRequest(WlBase *pBase, WlDirectory *pDirectory, 
		WlDirectoryRequest *pDirectoryRequest, RsslRDMService *serviceList, RsslUInt32 serviceCount,
		RsslErrorInfo *pErrorInfo);

/* Processes a directory message from the provider. */
RsslRet wlDirectoryProcessProviderMsgEvent(WlBase *pBase, WlDirectory *pDirectory,
		RsslDecodeIterator *pIter, RsslWatchlistMsgEvent *pMsgEvent, RsslErrorInfo *pErrorInfo);

/* Processes a list of updated services. */
RsslRet wlFanoutDirectoryMsg(WlBase *pBase, WlDirectory *pDirectory, RsslQueue *pUpdatedServiceList,
		RsslErrorInfo *pErrorInfo);

/* Closes a directory request. */
void wlDirectoryRequestClose(WlBase *pBase, WlDirectory *pDirectory,
		WlDirectoryRequest *pDirectoryRequest);


#ifdef __cplusplus
}
#endif


#endif

