/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/


#ifndef _RTR_RSSL_DIRECTORY_ENCDEC_H
#define _RTR_RSSL_DIRECTORY_ENCDEC_H

#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CAPABILITIES 10
#define MAX_QOS 5
#define MAX_DICTIONARIES 5
#define MAX_GROUP_INFO_LEN 256
#define MAX_DATA_INFO_LEN 1024
#define MAX_LINKS 5
#define MAX_SOURCE_DIRECTORY_SERVICES 15
#define MAX_SRCDIR_INFO_STRLEN 256
#define MAX_REAL_TIME_FEED_SESSIONS 30
#define MAX_GAP_FILL_SESSIONS 	30

/* source directory request information */
typedef struct {
	RsslInt32		StreamId;
	char			ServiceName[MAX_SRCDIR_INFO_STRLEN];
	RsslMsgKey		MsgKey;
	RsslChannel*	Chnl;
	RsslBool		IsInUse;
} RsslSourceDirectoryRequestInfo;

/* reasons a source directory request is rejected */
typedef enum {
	MAX_SRCDIR_REQUESTS_REACHED	= 0,
	INCORRECT_FILTER_FLAGS		= 1
} RsslSrcDirRejectReason;

/* source directory response information */
/* service general information */
typedef struct {
	char		ServiceName[MAX_SRCDIR_INFO_STRLEN];
	char		Vendor[MAX_SRCDIR_INFO_STRLEN];
	RsslUInt64	IsSource;
	RsslUInt64	Capabilities[MAX_CAPABILITIES];
	char		DictionariesProvided[MAX_DICTIONARIES][MAX_SRCDIR_INFO_STRLEN];
	char		DictionariesUsed[MAX_DICTIONARIES][MAX_SRCDIR_INFO_STRLEN];
	RsslQos		QoS[MAX_QOS];
	RsslUInt64	SupportsQosRange;
	char		ItemList[MAX_SRCDIR_INFO_STRLEN];
	RsslUInt64	SupportsOutOfBandSnapshots;
	RsslUInt64	AcceptingConsumerStatus;
} RsslServiceGeneralInfo;
/* service state information */
typedef struct {
	RsslUInt64	ServiceState;
	RsslUInt64	AcceptingRequests;
	RsslState	Status;
} RsslServiceStateInfo;
/* service group information */
typedef struct {
	RsslUInt8	Group[MAX_GROUP_INFO_LEN];
	RsslUInt8	MergedToGroup[MAX_GROUP_INFO_LEN];
	RsslState	Status;
} RsslServiceGroupInfo;
/* service load information */
typedef struct {
	RsslUInt64	OpenLimit;
	RsslUInt64	OpenWindow;
	RsslUInt64	LoadFactor;
} RsslServiceLoadInfo;
/* service data information */
typedef struct {
	RsslUInt64	Type;
	RsslUInt8	Data[MAX_DATA_INFO_LEN];
} RsslServiceDataInfo;
/* service link information */
typedef struct {
	char		LinkName[MAX_SRCDIR_INFO_STRLEN];
	RsslUInt64	Type;
	RsslUInt64	LinkState;
	RsslUInt64	LinkCode;
	char		Text[MAX_SRCDIR_INFO_STRLEN];
} RsslServiceLinkInfo;

typedef struct {
	char		Address[128];
	char		Port[128];
} RsslAddressPortInfo;

typedef struct {
	char		Address[128];
	char		Port[128];
	RsslUInt64	Domain;
} RsslMulticastAddressPortInfo;

typedef struct {
	RsslAddressPortInfo	SnapshotServer;
	RsslAddressPortInfo	GapRequestServer;
	RsslAddressPortInfo	RefDataServer;
	RsslMulticastAddressPortInfo	MBORealTimeDataServer[MAX_REAL_TIME_FEED_SESSIONS];
	RsslMulticastAddressPortInfo	MPRealTimeDataServer[MAX_REAL_TIME_FEED_SESSIONS];
	RsslMulticastAddressPortInfo	MBOGapFillServer[MAX_GAP_FILL_SESSIONS];
	RsslMulticastAddressPortInfo	MPGapFillServer[MAX_GAP_FILL_SESSIONS];
} RsslEDFInfo;
	
/* entire source directory response information */
typedef struct {
	RsslInt32 StreamId;
	RsslUInt64 ServiceId;
	RsslServiceGeneralInfo ServiceGeneralInfo;
	RsslServiceStateInfo ServiceStateInfo;
	RsslServiceGroupInfo ServiceGroupInfo;
	RsslServiceLoadInfo ServiceLoadInfo;
	RsslServiceDataInfo ServiceDataInfo;
	RsslServiceLinkInfo ServiceLinkInfo[MAX_LINKS];
	RsslEDFInfo	EDFInfo;
} RsslSourceDirectoryResponseInfo;

RsslRet decodeSourceDirectoryResponse(RsslSourceDirectoryResponseInfo* srcDirRespInfo,
										  RsslDecodeIterator* dIter,
										  RsslUInt32 maxServices,
										  RsslUInt32 maxCapabilities,
										  RsslUInt32 maxQOS,
										  RsslUInt32 maxDictionaries,
										  RsslUInt32 maxLinks);
RsslRet decodeServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo,
										  RsslDecodeIterator* dIter,
										  RsslUInt32 maxCapabilities,
										  RsslUInt32 maxQOS,
										  RsslUInt32 maxDictionaries);
RsslRet decodeServiceStateInfo(RsslServiceStateInfo* serviceStateInfo,
										  RsslDecodeIterator* dIter);
RsslRet decodeServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo,
										  RsslDecodeIterator* dIter);
RsslRet decodeServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo,
										  RsslDecodeIterator* dIter);
RsslRet decodeServiceDataInfo(RsslServiceDataInfo* serviceDataInfo,
										  RsslDecodeIterator* dIter);
RsslRet decodeServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo,
										  RsslDecodeIterator* dIter,
										  RsslUInt32 maxLinks);

RTR_C_INLINE void clearEDFInfo(RsslEDFInfo* edfInfo)
{
	int i;
	edfInfo->SnapshotServer.Address[0] = '\0';
	edfInfo->SnapshotServer.Port[0] = '\0';
	edfInfo->GapRequestServer.Address[0] = '\0';
	edfInfo->GapRequestServer.Port[0] = '\0';
	edfInfo->RefDataServer.Address[0] = '\0';
	edfInfo->RefDataServer.Port[0] = '\0';

	for (i = 0; i < MAX_REAL_TIME_FEED_SESSIONS; i++)
	{
		edfInfo->MBORealTimeDataServer[i].Address[0] = '\0';
		edfInfo->MBORealTimeDataServer[i].Port[0] = '\0';
		edfInfo->MPRealTimeDataServer[i].Address[0] = '\0';
		edfInfo->MPRealTimeDataServer[i].Port[0] = '\0';
	}
	for (i = 0; i < MAX_GAP_FILL_SESSIONS; i++)
	{
		edfInfo->MBOGapFillServer[i].Address[0] = '\0';
		edfInfo->MBOGapFillServer[i].Port[0] = '\0';
		edfInfo->MPGapFillServer[i].Address[0] = '\0';
		edfInfo->MPGapFillServer[i].Port[0] = '\0';
	}
}
/*
 * Clears the source directory request information.
 * srcDirReqInfo - The source directory request information to be cleared
 */
RTR_C_INLINE void clearSourceDirectoryReqInfo(RsslSourceDirectoryRequestInfo* srcDirReqInfo)
{
	srcDirReqInfo->StreamId = 0;
	srcDirReqInfo->ServiceName[0] = '\0';
	rsslClearMsgKey(&srcDirReqInfo->MsgKey);
	srcDirReqInfo->MsgKey.name.data = srcDirReqInfo->ServiceName;
	srcDirReqInfo->MsgKey.name.length = MAX_SRCDIR_INFO_STRLEN;
	srcDirReqInfo->Chnl = 0;
	srcDirReqInfo->IsInUse = RSSL_FALSE;
}
/*
 * Clears the service's general information.
 * serviceGeneralInfo - The service's general information to be cleared
 */
RTR_C_INLINE void clearServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo)
{
	int i;

	serviceGeneralInfo->ServiceName[0] = '\0';
	serviceGeneralInfo->Vendor[0] = '\0';
	serviceGeneralInfo->IsSource = 0;
	for (i = 0; i < MAX_CAPABILITIES; i++)
	{
		serviceGeneralInfo->Capabilities[i] = 0;
	}
	for (i = 0; i < MAX_DICTIONARIES; i++)
	{
		serviceGeneralInfo->DictionariesProvided[i][0] = '\0';
		serviceGeneralInfo->DictionariesUsed[i][0] = '\0';
	}
	for (i = 0; i < MAX_QOS; i++)
	{
		serviceGeneralInfo->QoS[i].dynamic = 0;
		serviceGeneralInfo->QoS[i].rate = RSSL_QOS_RATE_UNSPECIFIED;
		serviceGeneralInfo->QoS[i].rateInfo = 0;
		serviceGeneralInfo->QoS[i].timeInfo = 0;
		serviceGeneralInfo->QoS[i].timeliness = RSSL_QOS_TIME_UNSPECIFIED;
	}
	serviceGeneralInfo->SupportsQosRange = 0;
	serviceGeneralInfo->ItemList[0] = '\0';
	serviceGeneralInfo->SupportsOutOfBandSnapshots = 0;
	serviceGeneralInfo->AcceptingConsumerStatus = 0;
}


/*
 * Initializes the service's general information.
 * serviceGeneralInfo - The service's general information to be initialized
 */
RTR_C_INLINE void initServiceGeneralInfo(RsslServiceGeneralInfo* serviceGeneralInfo)
{
	int i;

	serviceGeneralInfo->ServiceName[0] = '\0';
	serviceGeneralInfo->Vendor[0] = '\0';
	serviceGeneralInfo->IsSource = 1;
	for (i = 0; i < MAX_CAPABILITIES; i++)
	{
		serviceGeneralInfo->Capabilities[i] = 0;
	}
	for (i = 0; i < MAX_DICTIONARIES; i++)
	{
		serviceGeneralInfo->DictionariesProvided[i][0] = '\0';
		serviceGeneralInfo->DictionariesUsed[i][0] = '\0';
	}
	for (i = 0; i < MAX_QOS; i++)
	{
		serviceGeneralInfo->QoS[i].dynamic = 0;
		serviceGeneralInfo->QoS[i].rate = RSSL_QOS_RATE_UNSPECIFIED;
		serviceGeneralInfo->QoS[i].rateInfo = 0;
		serviceGeneralInfo->QoS[i].timeInfo = 0;
		serviceGeneralInfo->QoS[i].timeliness = RSSL_QOS_TIME_UNSPECIFIED;
	}
	serviceGeneralInfo->SupportsQosRange = 0;
	serviceGeneralInfo->ItemList[0] = '\0';
	serviceGeneralInfo->SupportsOutOfBandSnapshots = 0;
	serviceGeneralInfo->AcceptingConsumerStatus = 0;
}

/*
 * Clears the service's state information.
 * serviceStateInfo - The service's state information to be cleared
 */
RTR_C_INLINE void clearServiceStateInfo(RsslServiceStateInfo* serviceStateInfo)
{
	serviceStateInfo->ServiceState = 0;
	serviceStateInfo->AcceptingRequests = 0;
	rsslClearState(&serviceStateInfo->Status);
}

/*
 * Initializes the service's state information.
 * serviceStateInfo - The service's state information to be initialized
 */
RTR_C_INLINE void initServiceStateInfo(RsslServiceStateInfo* serviceStateInfo)
{
	serviceStateInfo->ServiceState = 1;
	serviceStateInfo->AcceptingRequests = 1;
	rsslClearState(&serviceStateInfo->Status);
}

/*
 * Clears the service's group information.
 * serviceGroupInfo - The service's group information to be cleared
 */
RTR_C_INLINE void clearServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo)
{
	memset(serviceGroupInfo->Group, 0, sizeof(serviceGroupInfo->Group));
	memset(serviceGroupInfo->MergedToGroup, 0, sizeof(serviceGroupInfo->MergedToGroup));
	rsslClearState(&serviceGroupInfo->Status);
}

/*
 * Initializes the service's group information.
 * serviceGroupInfo - The service's group information to be initialized
 */
RTR_C_INLINE void initServiceGroupInfo(RsslServiceGroupInfo* serviceGroupInfo)
{
	clearServiceGroupInfo(serviceGroupInfo);
}

/*
 * Clears the service's load information.
 * serviceLoadInfo - The service's load information to be cleared
 */
RTR_C_INLINE void clearServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo)
{
	serviceLoadInfo->OpenLimit = 0;
	serviceLoadInfo->OpenWindow = 0;
	serviceLoadInfo->LoadFactor = 0;
}

/*
 * Initializes the service's load information.
 * serviceLoadInfo - The service's load information to be initialized
 */
RTR_C_INLINE void initServiceLoadInfo(RsslServiceLoadInfo* serviceLoadInfo)
{
	serviceLoadInfo->OpenLimit = 0;
	serviceLoadInfo->OpenWindow = 1;
	serviceLoadInfo->LoadFactor = 1;
}

/*
 * Clears the service's data information.
 * serviceDataInfo - The service's data information to be cleared
 */
RTR_C_INLINE void clearServiceDataInfo(RsslServiceDataInfo* serviceDataInfo)
{
	serviceDataInfo->Type = 0;
	memset(serviceDataInfo->Data, 0, sizeof(serviceDataInfo->Data));
}

/*
 * Initializes the service's data information.
 * serviceDataInfo - The service's data information to be initialized
 */
RTR_C_INLINE void initServiceDataInfo(RsslServiceDataInfo* serviceDataInfo)
{
	clearServiceDataInfo(serviceDataInfo);
}

/*
 * Clears the service's link information.
 * serviceLinkInfo - The service's link information to be cleared
 */
RTR_C_INLINE void clearServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo)
{
	int i;

	for (i = 0; i < MAX_LINKS; i++)
	{
		serviceLinkInfo[i].LinkName[0] = '\0';
		serviceLinkInfo[i].Type = 0;
		serviceLinkInfo[i].LinkState = 0;
		serviceLinkInfo[i].LinkCode = 0;
		serviceLinkInfo[i].Text[0] = '\0';
	}
}

/*
 * Initializes the service's link information.
 * serviceLinkInfo - The service's link information to be initialized
 */
RTR_C_INLINE void initServiceLinkInfo(RsslServiceLinkInfo* serviceLinkInfo)
{
	int i;

	for (i = 0; i < MAX_LINKS; i++)
	{
		serviceLinkInfo[i].LinkName[0] = '\0';
		serviceLinkInfo[i].Type = 1;
		serviceLinkInfo[i].LinkState = 1;
		serviceLinkInfo[i].LinkCode = 1;
		serviceLinkInfo[i].Text[0] = '\0';
	}
}

/*
 * Clears the source directory response information.
 * srcDirRespInfo - The source directory response information to be cleared
 */
RTR_C_INLINE void clearSourceDirRespInfo(RsslSourceDirectoryResponseInfo* srcDirRespInfo)
{
	int i;

	for (i = 0; i < MAX_SOURCE_DIRECTORY_SERVICES; i++)
	{
		srcDirRespInfo[i].StreamId = 0;
		srcDirRespInfo[i].ServiceId = 0;
		clearEDFInfo(&srcDirRespInfo[i].EDFInfo);
		clearServiceGeneralInfo(&srcDirRespInfo[i].ServiceGeneralInfo);
		clearServiceStateInfo(&srcDirRespInfo[i].ServiceStateInfo);
		clearServiceGroupInfo(&srcDirRespInfo[i].ServiceGroupInfo);
		clearServiceLoadInfo(&srcDirRespInfo[i].ServiceLoadInfo);
		clearServiceDataInfo(&srcDirRespInfo[i].ServiceDataInfo);
		clearServiceLinkInfo(&srcDirRespInfo[i].ServiceLinkInfo[0]);
	}
}

/*
 * Initializes the source directory response information.
 * srcDirRespInfo - The source directory response information to be initialized
 */
RTR_C_INLINE void initSourceDirRespInfo(RsslSourceDirectoryResponseInfo* srcDirRespInfo)
{
	srcDirRespInfo->StreamId = 0;
	srcDirRespInfo->ServiceId = 0;
	initServiceGeneralInfo(&srcDirRespInfo->ServiceGeneralInfo);
	initServiceStateInfo(&srcDirRespInfo->ServiceStateInfo);
	initServiceGroupInfo(&srcDirRespInfo->ServiceGroupInfo);
	initServiceLoadInfo(&srcDirRespInfo->ServiceLoadInfo);
	initServiceDataInfo(&srcDirRespInfo->ServiceDataInfo);
	initServiceLinkInfo(&srcDirRespInfo->ServiceLinkInfo[0]);
}


#ifdef __cplusplus
};
#endif

#endif
