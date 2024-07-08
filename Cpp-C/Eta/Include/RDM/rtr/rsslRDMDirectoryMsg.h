/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RSSL_RDM_DIRECTORY_MSG_H
#define _RSSL_RDM_DIRECTORY_MSG_H

#include "rtr/rsslRDMMsgBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMDirectory
 *	@{
 */

typedef enum
{
	RDM_DR_CSSF_NONE = 0x00,						    /*!< (0x00) No flags set. */
	RDM_DR_CSSF_HAS_WARM_SOURCE_MIRRORING_MODE = 0x01,	/*!< (0x01) Indicates presence of the source mirroring mode member. */
	RDM_DR_CSSF_HAS_WARM_STANDBY_MODE = 0x02,			/*!< (0x02) Indicates presence of warm standby mode member. */
}
RsslRDMDirectoryConsumerStatusFlags;

/**
 * @brief Information about how a Consumer is using a particular service with regard to Source Mirroring.
 * @see RsslRDMConsumerStatusServiceFlags, RsslRDMDirectoryConsumerStatus, rsslClearRDMConsumerStatusService
 */
typedef struct {
	RsslUInt serviceId;							/*!< ID of the service this status concerns. */
	RsslMapEntryActions action;					/*!< Action associated with this status. */
	RsslUInt32			flags;					/*!< */
	RsslUInt sourceMirroringMode;				/*!< The Source Mirroring Mode for this service.  Populated by RDMDirectorySourceMirroringMode. */
	RsslUInt warmStandbyMode;					/*!< The desired Warm Standby Mode for this service. */
} RsslRDMConsumerStatusService;

/**
 * @brief Clears an RsslRDMConsumerStatusService.
 * @see RsslRDMConsumerStatusService
 */
RTR_C_INLINE void rsslClearRDMConsumerStatusService(RsslRDMConsumerStatusService *pStatus)
{
	pStatus->action = RSSL_MPEA_ADD_ENTRY;
	pStatus->serviceId = 0;
	pStatus->flags = RDM_DR_CSSF_HAS_WARM_SOURCE_MIRRORING_MODE;
	pStatus->sourceMirroringMode = RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY;
	pStatus->warmStandbyMode = RDM_DIRECTORY_SERVICE_TYPE_ACTIVE;
}

/**
 * @brief The RDM Service Info flags
 * @see RsslRDMServiceInfo
 */
typedef enum {
	RDM_SVC_IFF_NONE                       =  0x000,	/*!< (0x000) No flags set. */
	RDM_SVC_IFF_HAS_VENDOR                 =  0x001,	/*!< (0x001) Indicates presence of the vendor member. */
	RDM_SVC_IFF_HAS_IS_SOURCE              =  0x002,	/*!< (0x002) Indicates presence of the isSource member. */
	RDM_SVC_IFF_HAS_DICTS_PROVIDED         =  0x004,	/*!< (0x004) Indicates presence of the dictionariesProvidedList member. */
	RDM_SVC_IFF_HAS_DICTS_USED             =  0x008,	/*!< (0x008) Indicates presence of the dictionariesUsedList member. */
	RDM_SVC_IFF_HAS_QOS                    =  0x010,	/*!< (0x010) Indicates presence of the qosList member. */
	RDM_SVC_IFF_HAS_SUPPORT_QOS_RANGE      =  0x020,	/*!< (0x020) Indicates presence of the supportsQosRange member. */
	RDM_SVC_IFF_HAS_ITEM_LIST              =  0x040,	/*!< (0x040) Indicates presence of the itemList member. */
	RDM_SVC_IFF_HAS_SUPPORT_OOB_SNAPSHOTS  =  0x080,	/*!< (0x080) Indicates presence of the supportsOutOfBandSnapshots member. */
	RDM_SVC_IFF_HAS_ACCEPTING_CONS_STATUS  =  0x100	 	/*!< (0x100) Indicates presence of the acceptingConsumerStatus member. */
} RsslRDMServiceInfoFlags;

/**
 * @brief The RDM Service Info. Contains information provided by the Source Directory Info filter.
 * @see RsslRDMServiceInfoFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32				flags;							/*!< The RDM Service Info flags. */
	RsslFilterEntryActions	action;							/*!< Action associated with this Service Info. */
	RsslBuffer				serviceName;					/*!< Name identifying the service. */
	RsslBuffer				vendor;							/*!< Name identifying the vendor of the service's data. */
	RsslUInt				isSource;						/*!< Indicates whether the service is provided directly by a publisher or consolidated from multiple sources. */
	RsslUInt32				capabilitiesCount;				/*!< Number of capabilities present in capabilitiesList. */
	RsslUInt				*capabilitiesList;				/*!< Array of capabilities the service supports. Populated by RsslDomainTypes. */
	RsslUInt32				dictionariesProvidedCount;		/*!< Number of dictionary names present in dictionariesProvidedList. */
	RsslBuffer				*dictionariesProvidedList;		/*!< Array of names of dictionaries that this service provides. */
	RsslUInt32				dictionariesUsedCount;			/*!< Number of dictionary names present in dictionariesUsedList. */
	RsslBuffer				*dictionariesUsedList;			/*!< Array of names of dictionaries that a Consumer will require to decode the service's data. */
	RsslUInt32				qosCount;						/*!< The number of qualities of service in qosList. */
	RsslQos					*qosList;						/*!< Array of qualities of service that this service provides. */
	RsslBuffer				itemList;						/*!< Item name a Consumer can request to get a symbol list of all item names available from this service. */
	RsslUInt				supportsQosRange;				/*!< Indicates whether items can be requested using a QoS range(using both the qos and worstQos members of an RsslRequestMsg). */
	RsslUInt				supportsOutOfBandSnapshots;		/*!< Indicates whether Snapshot(requests without the RSSL_RQMF_STREAMING flag) can be made when the OpenLimit is reached. */
	RsslUInt				acceptingConsumerStatus;		/*!< Indicates whether the service accepts messages related to Source Mirroring(see RsslRDMDirectoryConsumerStatus). */
} RsslRDMServiceInfo;


/**
 * @brief The RDM Sequenced MCast Info flags
 * @see RsslRDMSeqMCastInfo
 */
typedef enum {
	RDM_SVC_SMF_NONE			=  0x000,	/*!< (0x000) No flags set. */
	RDM_SVC_SMF_HAS_SNAPSHOT_SERV		=  0x001,	/*!< (0x001) Indicates presence of Snapshot Server Info */
	RDM_SVC_SMF_HAS_GAP_REC_SERV		=  0x002,	/*!< (0x002) Indicates presence of Gap Recovery Server Info */
	RDM_SVC_SMF_HAS_REF_DATA_SERV		=  0x004,	/*!< (0x004) Indicates presence of Reference Data Server Info */
	RDM_SVC_SMF_HAS_SMC_SERV		=  0x008,	/*!< (0x008) Indicates presence of Streaming Multicast Channels Server Info */
	RDM_SVC_SMF_HAS_GMC_SERV		=  0x010	/*!< (0x010) Indicates presence of Gap Multicast Channel Server Info */
} RsslRDMSeqMCastInfoFlags;

/**
 * @brief The Address Port Info. Contains Host Address and Port information.
 * @see RsslRDMServiceSeqMCastInfo, RsslRDMService
 */

typedef struct {
	RsslBuffer				address;
	RsslUInt				port;
} RsslRDMAddressPortInfo;

/**
 * @brief Clears an RsslRDMAddressPortInfo.
 * @see RsslRDMAddressPortInfo
 */
RTR_C_INLINE void rsslClearRDMAddressPortInfo(RsslRDMAddressPortInfo *pAddressPortInfo)
{
	rsslClearBuffer(&pAddressPortInfo->address);
	pAddressPortInfo->port = 0;
}

/**
 *  * @brief The MC Address Port Info. Contains Host Address Port and domain information.
 *   * @see RsslRDMServiceSeqMCastInfo, RsslRDMService
 *    */

typedef struct {
    RsslBuffer              address;
    RsslUInt                port;
    RsslUInt                domain;
} RsslRDMMCAddressPortInfo;

/**
 *  * @brief Clears an RsslRDMMCAddressPortInfo.
 *   * @see RsslRDMMCAddressPortInfo
 *    */
RTR_C_INLINE void rsslClearRDMMCAddressPortInfo(RsslRDMMCAddressPortInfo *pAddressPortInfo)
{
    rsslClearBuffer(&pAddressPortInfo->address);
    pAddressPortInfo->port = 0;
    pAddressPortInfo->domain = 0;
}

/**
 * @brief The RDM Sequenced Multicast Info. Contains information provided by the Source Directory Sequenced Multicast filter.
 * @see RsslRDMSeqMCastInfoFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32			flags;				/*!< The RDM Sequenced Multicast Info flags. */
	RsslFilterEntryActions		action;				/*!< Action associated with this Service Info. */
	RsslRDMAddressPortInfo		snapshotServer;			/*!< Snapshot Server Connection Infomation. */
	RsslRDMAddressPortInfo		gapRecoveryServer;		/*!< Gap Recovery Server Connection Infomation. */
	RsslRDMAddressPortInfo		refDataServer;			/*!< Reference Data Server Connection Infomation. */
	RsslUInt32			StreamingMCastChanServerCount;	/*!< Streaming Multicast Channel Server count. */
	RsslRDMMCAddressPortInfo	*StreamingMCastChanServerList;	/*!< Streaming Multicast Channel Server list. */
	RsslUInt32			GapMCastChanServerCount;	/*!< Gap Multicast Channel Server count. */
	RsslRDMMCAddressPortInfo	*GapMCastChanServerList;	/*!< Gap Multicast Channel Server list. */
} RsslRDMServiceSeqMCastInfo;


/**
 * @brief Clears an RsslRDMServiceSeqMCastInfo.
 * @see RsslRDMServiceSeqMCastInfo
 */
RTR_C_INLINE void rsslClearRDMServiceSeqMCastInfo(RsslRDMServiceSeqMCastInfo *pSeqMCastInfo)
{
	pSeqMCastInfo->flags = RDM_SVC_SMF_NONE;
	pSeqMCastInfo->action = RSSL_FTEA_SET_ENTRY;

	rsslClearRDMAddressPortInfo(&pSeqMCastInfo->snapshotServer);
	rsslClearRDMAddressPortInfo(&pSeqMCastInfo->gapRecoveryServer);
	rsslClearRDMAddressPortInfo(&pSeqMCastInfo->refDataServer);

	pSeqMCastInfo->StreamingMCastChanServerCount = 0;
	pSeqMCastInfo->StreamingMCastChanServerList = NULL;
	pSeqMCastInfo->GapMCastChanServerCount = 0;
	pSeqMCastInfo->GapMCastChanServerList = NULL;
}

/**
 * @brief Clears an RsslRDMServiceInfo.
 * @see RsslRDMServiceInfo
 */
RTR_C_INLINE void rsslClearRDMServiceInfo(RsslRDMServiceInfo *pServiceInfo)
{
	pServiceInfo->flags = RDM_SVC_IFF_NONE;
	pServiceInfo->action = RSSL_FTEA_SET_ENTRY;
	rsslClearBuffer(&pServiceInfo->serviceName);
	rsslClearBuffer(&pServiceInfo->vendor);
	pServiceInfo->isSource = 0;
	pServiceInfo->capabilitiesCount = 0;
	pServiceInfo->capabilitiesList = NULL;
	pServiceInfo->dictionariesProvidedCount = 0;
	pServiceInfo->dictionariesProvidedList = NULL;
	pServiceInfo->dictionariesUsedCount = 0;
	pServiceInfo->dictionariesUsedList = NULL;
	pServiceInfo->qosCount = 0;
	pServiceInfo->qosList = NULL;
	rsslClearBuffer(&pServiceInfo->itemList);
	pServiceInfo->supportsQosRange = 0;
	pServiceInfo->supportsOutOfBandSnapshots = 1;
	pServiceInfo->acceptingConsumerStatus = 1;
}

/**
 * @brief The RDM Service State flags
 * @see RsslRDMServiceState
 */
typedef enum {
	RDM_SVC_STF_NONE				= 0x00,	/*!< (0x00) No flags set. */
	RDM_SVC_STF_HAS_ACCEPTING_REQS	= 0x01,	/*!< (0x01) Indicates presence of the acceptingRequests member. */
	RDM_SVC_STF_HAS_STATUS			= 0x02	/*!< (0x02) Indicates presence of the status member. */
} RsslRDMServiceStateFlags;

/**
 * @brief The RDM Service State. Contains information provided by the Source Directory State filter.
 * @see RsslRDMServiceStateFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32					flags;				/*!< The Service State flags. */
	RsslFilterEntryActions		action;				/*!< The Action associated with this State. */
	RsslUInt					serviceState;		/*!< The current state of the Service. */
	RsslUInt					acceptingRequests;	/*!< Indicates whether the service is accepting item requests. */
	RsslState					status;				/*!< A status to be applied to all items being provided by this service. */
} RsslRDMServiceState;

/**
 * @brief Clears an RsslRDMServiceState.
 * @see RsslRDMServiceState
 */
RTR_C_INLINE void rsslClearRDMServiceState(RsslRDMServiceState *pServiceState)
{
	pServiceState->flags = RDM_SVC_STF_NONE;
	pServiceState->action = RSSL_FTEA_SET_ENTRY;
	pServiceState->serviceState = 1;
	pServiceState->acceptingRequests = 1;
	rsslClearState(&pServiceState->status);
	pServiceState->status.streamState = RSSL_STREAM_OPEN;
	pServiceState->status.dataState = RSSL_DATA_OK;
	pServiceState->status.code = RSSL_SC_NONE;
}

/**
 * @brief The RDM Service Group State flags
 * @see RsslRDMServiceGroupState
 */
typedef enum {
	RDM_SVC_GRF_NONE				= 0x00,	/*!< (0x00) No flags set. */
	RDM_SVC_GRF_HAS_MERGED_TO_GROUP	= 0x01,	/*!< (0x01) Indicates presence of the mergedToGroup member. */
	RDM_SVC_GRF_HAS_STATUS			= 0x02	/*!< (0x02) Indicates presence of the status member. */
} RsslRDMServiceGroupFlags;

/**
 * @brief The RDM Service Group State. Contains information provided by the Source Directory Group filter.
 * @see RsslRDMServiceGroup, RsslRDMService
 */
typedef struct {
	RsslUInt32					flags;			/*!< The RDM Service Group State flags. */
	RsslFilterEntryActions		action;			/*!< Action associated with this Group State. */
	RsslBuffer					group;			/*!< The Item Group name associated with this status. */
	RsslBuffer					mergedToGroup;	/*!< The name of the new Item Group that the items in the given Group(named by the group member) now belong to. */
	RsslState					status;			/*!< Status to apply to all items in this Group. */
} RsslRDMServiceGroupState;

/**
 * @brief Clears an RsslRDMServiceGroupState.
 * @see RsslRDMServiceGroupState
 */
RTR_C_INLINE void rsslClearRDMServiceGroupState(RsslRDMServiceGroupState *pGroupState)
{
	pGroupState->flags = RDM_SVC_GRF_NONE;
	pGroupState->action = RSSL_FTEA_SET_ENTRY;
	rsslClearBuffer(&pGroupState->group);
	rsslClearBuffer(&pGroupState->mergedToGroup);
	rsslClearState(&pGroupState->status);
	pGroupState->status.streamState = RSSL_STREAM_OPEN;
	pGroupState->status.dataState = RSSL_DATA_OK;
	pGroupState->status.code = RSSL_SC_NONE;
}

/**
 * @brief The RDM Service Load flags
 * @see RsslRDMServiceLoad
 */
typedef enum
{
	RDM_SVC_LDF_NONE             =  0x00,		/*!< (0x00) No flags set. */
	RDM_SVC_LDF_HAS_OPEN_LIMIT   =  0x01,		/*!< (0x01) Indicates presence of an open limit on the service. */
	RDM_SVC_LDF_HAS_OPEN_WINDOW  =  0x02,		/*!< (0x02) Indicates presence of an open window on the service. */
	RDM_SVC_LDF_HAS_LOAD_FACTOR  =  0x04		/*!< (0x04) Indicates presence of a load factor. */
} RsslRDMServiceLoadFlags;

/**
 * @brief The RDM Service Load. Contains information provided by the Source Directory Load filter.
 * @see RsslRDMServiceLoadFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32				flags;		/*!< The Service Load Flags. */
	RsslFilterEntryActions	action;		/*!< Action associated with this Load filter. */
	RsslUInt				openLimit;	/*!< The maximum number of items the Consumer is allowed to open from this service. */
	RsslUInt				openWindow;	/*!< The maximum number of items the Consumer may have outstanding(i.e. waiting for an RsslRefreshMsg) from this service. */
	RsslUInt				loadFactor;	/*!< A number indicating the current workload of the source providing the data. */
} RsslRDMServiceLoad;

/**
 * @brief Clears an RsslRDMServiceLoad.
 * @see RsslRDMServiceLoad
 */
RTR_C_INLINE void rsslClearRDMServiceLoad(RsslRDMServiceLoad *pServiceLoad)
{
	pServiceLoad->flags = RDM_SVC_LDF_NONE;
	pServiceLoad->action = RSSL_FTEA_SET_ENTRY;
	pServiceLoad->openLimit = 0xffffffffffffffffULL;
	pServiceLoad->openWindow = 0xffffffffffffffffULL;
	pServiceLoad->loadFactor = 65535;
}

/**
 * @brief The RDM Service Data flags
 * @see RsslRDMServiceData
 */
typedef enum
{
	RDM_SVC_DTF_NONE		= 0x00,	/*!< (0x00) No flags set. */
	RDM_SVC_DTF_HAS_DATA	= 0x01	/*!< (0x01) Indicates presence of the type, dataType and data members. */
} RsslRDMServiceDataFlags;

/**
 * @brief The RDM Service Data. Contains information provided by the Source Directory Data filter.
 * @see RsslRDMServiceDataFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32				flags;		/*!< The Service Data flags. */
	RsslFilterEntryActions	action;		/*!< Action associated with this Data filter. */
	RsslUInt				type;		/*!< Indicates the content present in the data member. Populated by RDMDirectoryDataTypes. */
	RsslDataTypes			dataType;	/*!< The OMM type of the data. Populated by RsslDataTypes. */
	RsslBuffer				data;		/*!< The encoded data, to be applied to all items being provided by this service. */
} RsslRDMServiceData;

/**
 * @brief Clears an RsslRDMServiceData.
 * @see RsslRDMServiceData
 */
RTR_C_INLINE void rsslClearRDMServiceData(RsslRDMServiceData *pServiceData)
{
	pServiceData->flags = RDM_SVC_DTF_NONE;
	pServiceData->action = RSSL_FTEA_SET_ENTRY;
	pServiceData->type = RDM_DIRECTORY_DATA_TYPE_NONE;
	pServiceData->dataType = RSSL_DT_NO_DATA;
	rsslClearBuffer(&pServiceData->data);
}

/**
 * @brief The RDM Service Link flags
 * @see RsslRDMServiceLink
 */
typedef enum
{
	RDM_SVC_LKF_NONE		= 0x00,		/*!< (0x00) No flags set. */
	RDM_SVC_LKF_HAS_TYPE	= 0x01,		/*!< (0x01) Indicates presence of the source type. */
	RDM_SVC_LKF_HAS_CODE	= 0x02,		/*!< (0x02) Indicates presence of the link code. */
	RDM_SVC_LKF_HAS_TEXT	= 0x04		/*!< (0x04) Indcates presence of link text. */
} RsslRDMServiceLinkFlags;

/**
 * @brief The RDM Service Link. Contains information about an upstream source associated with the service.
 * @see RsslRDMServiceLinkFlags, RsslRDMService
 */
typedef struct {
	RsslUInt32				flags;		/*!< The Service Link Flags. */
	RsslMapEntryActions		action;		/*!< Action associated with this source. */
	RsslBuffer				name;		/*!< Name identifying this upstream source. */
	RsslUInt				type;		/*!< Type of this source. Populated by RDMDirectoryLinkTypes. */
	RsslUInt				linkState;	/*!< Indicates whether the source is up or down. */
	RsslUInt				linkCode;	/*!< Code indicating additonal information about the status of the source. Populated by RDMDirectoryLinkCodes. */
	RsslBuffer				text;		/*!< Text further describing the state provided by the linkState and linkCode members. */
} RsslRDMServiceLink;

/**
 * @brief Clears an RsslRDMServiceLink.
 * @see RsslRDMServiceLink
 */
RTR_C_INLINE void rsslClearRDMServiceLink(RsslRDMServiceLink *pServiceLink)
{
	pServiceLink->flags = RDM_SVC_LKF_NONE;
	pServiceLink->action = RSSL_MPEA_ADD_ENTRY;
	rsslClearBuffer(&pServiceLink->name);
	pServiceLink->type = RDM_DIRECTORY_LINK_TYPE_INTERACTIVE;
	pServiceLink->linkState = 0;
	pServiceLink->linkCode = RDM_DIRECTORY_LINK_CODE_NONE;
	rsslClearBuffer(&pServiceLink->text);
}

/**
 * @brief The RDM Service Link Info. Contains information provided by the Source Directory Link filter.
 * @see RsslRDMService
 */
typedef struct
{
	RsslFilterEntryActions	action;		/*!< Action associated with this Link filter. */
	RsslUInt32				linkCount;	/*!< Number of source entries present in the linkList member. */
	RsslRDMServiceLink		*linkList;	/*!< Array of entries with information about upstream sources. */
} RsslRDMServiceLinkInfo;

/**
 * @brief Clears an RsslRDMServiceLinkInfo.
 * @see RsslRDMServiceLinkInfo
 */
RTR_C_INLINE void rsslClearRDMServiceLinkInfo(RsslRDMServiceLinkInfo *pInfo)
{
	pInfo->action = RSSL_FTEA_SET_ENTRY;
	pInfo->linkCount = 0;
	pInfo->linkList = NULL;
}

/**
 * @brief The RDM Service flags
 * @see RsslRDMService
 */
typedef enum {
	RDM_SVCF_NONE		= 0x00,	/*!< (0x00) No flags set. */
	RDM_SVCF_HAS_INFO	= 0x01,	/*!< (0x01) Indicates presence of the info member. */
	RDM_SVCF_HAS_STATE	= 0x02,	/*!< (0x02) Indicates presence of the state member. */
	RDM_SVCF_HAS_LOAD	= 0x04,	/*!< (0x04) Indicates presence of the load member. */
	RDM_SVCF_HAS_DATA	= 0x08,	/*!< (0x08) Indicates presence of the data member. */
	RDM_SVCF_HAS_LINK	= 0x10,	/*!< (0x10) Indicates presence of the linkInfo member. */
	RDM_SVCF_HAS_SEQ_MCAST 	= 0x20  /*!< (0x20) Indiacted presence of the sequenced multicast member. */
} RsslRDMServiceFlags;

/**
 * @brief The RDM Service. Contains information about a particular service.
 * @see RsslRDMServiceFlags, RsslRDMServiceInfo, RsslRDMServiceState, RsslRDMServiceGroupState, RsslRDMServiceLoad, RsslRDMServiceData, RsslRDMServiceLinkInfo, RsslRDMServiceSeqMCast
 */
typedef struct {
	RsslUInt32			flags;			/*!< The Service Flags. */
	RsslMapEntryActions		action;			/*!< Action associated with this Service entry. */
	RsslUInt			serviceId;		/*!< Number identifying this service. */
	RsslRDMServiceInfo		info;			/*!< Info filter for this service. */
	RsslRDMServiceState		state;			/*!< State filter for this service. */
	RsslUInt32			groupStateCount;	/*!< Number of Group filters present in groupStateList. */
	RsslRDMServiceGroupState	*groupStateList;	/*!< Group filters for this service. */
	RsslRDMServiceLoad		load;			/*!< Load filter for this service. */
	RsslRDMServiceData		data;			/*!< Data filter for this service. */
	RsslRDMServiceLinkInfo		linkInfo;		/*!< Link filter for this service. */
	RsslRDMServiceSeqMCastInfo	seqMCastInfo;		/*!< Sequenced Multicast filter for this service. */
} RsslRDMService;

/**
 * @brief Clears an RsslRDMService.
 * @see RsslRDMService
 */
RTR_C_INLINE void rsslClearRDMService(RsslRDMService *pService)
{
	pService->flags = RDM_SVCF_NONE;
	pService->action = RSSL_MPEA_ADD_ENTRY;
	pService->serviceId = 0;
	rsslClearRDMServiceInfo(&pService->info);
	rsslClearRDMServiceState(&pService->state);
	pService->groupStateCount = 0;
	pService->groupStateList = NULL;
	rsslClearRDMServiceLoad(&pService->load);
	rsslClearRDMServiceData(&pService->data);
	rsslClearRDMServiceLinkInfo(&pService->linkInfo);
	rsslClearRDMServiceSeqMCastInfo(&pService->seqMCastInfo);
}

/** 
 * @brief The types of RDM Directory Messages.  When the rdmMsgBase's domainType is RSSL_DMT_SOURCE, 
 * the rdmMsgType member may be set to one of these to indicate the specific RsslRDMDirectoryMsg class.
 * @see RsslRDMDirectoryMsg, RsslRDMMsgBase, RsslRDMDirectoryRequest, RsslRDMDirectoryClose, RsslRDMDirectoryConsumerStatus, RsslRDMDirectoryRefresh, RsslRDMDirectoryStatus
 */
typedef enum {
	RDM_DR_MT_UNKNOWN			= 0,	/*!< (0) Unknown */
	RDM_DR_MT_REQUEST			= 1,	/*!< (1) Directory Request */
	RDM_DR_MT_CLOSE				= 2,	/*!< (2) Directory Close */
	RDM_DR_MT_CONSUMER_STATUS	= 3,	/*!< (3) Directory Consumer Status */
	RDM_DR_MT_REFRESH			= 4,	/*!< (4) Directory Refresh */
	RDM_DR_MT_UPDATE			= 5,	/*!< (5) Directory Update */
	RDM_DR_MT_STATUS			= 6		/*!< (6) Directory Status */
} RsslRDMDirectoryMsgType;



/**
 * @brief The RDM Directory Request Flags
 * @see RsslRDMDirectoryRequest
 */
typedef enum
{
	RDM_DR_RQF_NONE				= 0x00, /*!< (0x00) No flags set. */
	RDM_DR_RQF_HAS_SERVICE_ID	= 0x01,	/*!< (0x01) Indicates presence of the serviceId member. */
	RDM_DR_RQF_STREAMING		= 0x02	/*!< (0x02) Indicates whether the request is a streaming request, i.e. whether updates about source directory information are desired. */
} RsslRDMDirectoryRequestFlags;


/**
 * @brief The RDM Directory Request.  Used by an OMM Consumer to request Source Directory information from an OMM Provider.
 * @see RsslRDMMsgBase, RsslRDMDirectoryMsg, RDMDirectoryServiceFilterFlags, rsslClearRDMDirectoryRequest
 */
typedef struct {
	RsslRDMMsgBase					rdmMsgBase; /*!< The Base RDM Message. */
	RsslUInt32						flags;		/*!< The RDM Directory Request flags. */
	RsslUInt16						serviceId;	/*!< A Service ID, if the Consumer is only requesting information about a particular service(If not present, the request is for information on all services). */
	RsslUInt32						filter;		/*!< A filter indicating which filters of information the Consumer is interested in. Populated by RDMDirectoryServiceFilterFlags. */
} RsslRDMDirectoryRequest;

/**
 * @brief Clears an RsslRDMDirectoryRequest.
 * @see RsslRDMDirectoryRequest, rsslInitDefaultRDMDirectoryRequest
 */
RTR_C_INLINE void rsslClearRDMDirectoryRequest(RsslRDMDirectoryRequest *pRequest)
{
	rsslClearRDMMsgBase(&pRequest->rdmMsgBase);
	pRequest->rdmMsgBase.rdmMsgType = RDM_DR_MT_REQUEST;
	pRequest->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
	pRequest->flags = RDM_DR_RQF_NONE;
	pRequest->serviceId = 0;
	pRequest->filter = 0;
}

/**
 * @brief The RDM Directory Refresh Flags
 * @see RsslRDMDirectoryRefresh
 */
typedef enum {
	RDM_DR_RFF_NONE				= 0x00,	/*!< (0x00) No flags set. */
	RDM_DR_RFF_HAS_SERVICE_ID	= 0x01,	/*!< (0x01) Indicates presence of the serviceId member. */
	RDM_DR_RFF_SOLICITED		= 0x02,	/*!< (0x02) Indicates whether this Refresh is provided in response to a request. */
	RDM_DR_RFF_HAS_SEQ_NUM		= 0x04,	/*!< (0x04) Indicates presence of the sequenceNumber. */
	RDM_DR_RFF_CLEAR_CACHE		= 0x08	/*!< (0x08) Indicates whether the Consumer should clear any existing cached service information. */
} RsslRDMDirectoryRefreshFlags;

/**
 * @brief The RDM Directory Refresh. Used by an OMM Provider to provide information about available services. 
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMService, rsslClearRDMDirectoryRefresh
 */
typedef struct {
	RsslRDMMsgBase					rdmMsgBase;		/*!< The Base RDM Message. */
	RsslUInt32						flags;			/*!< The RDM Directory Refresh flags. Populated by RsslRDMDirectoryRefreshFlags. */
	RsslState						state;			/*!< State of this stream. */
	RsslUInt32						filter;			/*!< Filter indicating which filters may appear on this stream. Where possible, this should match the consumer's request. Populated by RDMDirectoryServiceFilterFlags. */
	RsslUInt32						serviceCount;	/*!< Number of service entries present in serviceList. */
	RsslRDMService					*serviceList;	/*!< Array of service entries. */
	RsslUInt16						serviceId;		/*!< The ID of the service whose information is provided by this stream(if not present, all services should be provided). Should match the Consumer's request if possible. */
	RsslUInt32						sequenceNumber; /*!< Sequence number of this message. */
} RsslRDMDirectoryRefresh;

/**
 * @brief Clears an RsslRDMDirectoryRefresh.
 * @see RsslRDMDirectoryRefresh
 */
RTR_C_INLINE void rsslClearRDMDirectoryRefresh(RsslRDMDirectoryRefresh *pRefresh)
{
	rsslClearRDMMsgBase(&pRefresh->rdmMsgBase);
	pRefresh->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
	pRefresh->rdmMsgBase.rdmMsgType = RDM_DR_MT_REFRESH;
	pRefresh->flags = RDM_DR_RFF_NONE;
	rsslClearState(&pRefresh->state);
	pRefresh->state.streamState = RSSL_STREAM_OPEN;
	pRefresh->state.dataState = RSSL_DATA_OK;
	pRefresh->state.code = RSSL_SC_NONE;
	pRefresh->filter = 0;
	pRefresh->serviceCount = 0;
	pRefresh->serviceList = 0;
	pRefresh->serviceId = 0;
	pRefresh->sequenceNumber = 0;
}

/**
 * @brief The RDM Directory Update Flags
 * @see RsslRDMDirectoryUpdate
 */
typedef enum
{
	RDM_DR_UPF_NONE				= 0x00,	/*!< (0x00) No flags set. */
	RDM_DR_UPF_HAS_SERVICE_ID	= 0x01,	/*!< (0x01) Indicates presence of the serviceId member. */
	RDM_DR_UPF_HAS_FILTER		= 0x02,	/*!< (0x02) Indicates presence of the filter member. */
	RDM_DR_UPF_HAS_SEQ_NUM		= 0x04	/*!< (0x04) Indicates presence of the sequenceNumber member. */
} RsslRDMDirectoryUpdateFlags;

/**
 * @brief The RDM Directory Update. Used by an OMM Provider to provide updates about available services. 
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMService, rsslClearRDMDirectoryUpdate
 */
typedef struct {
	RsslRDMMsgBase				rdmMsgBase;		/*!< The Base RDM Message. */
	RsslUInt32					flags;			/*!< The RDM Directory Update flags. Populated by RsslRDMDirectoryUpdateFlags. */
	RsslUInt32					filter;			/*!< Filter indicating which filters may appear on this stream. Where possible, this should match the consumer's request. Populated by RDMDirectoryServiceFilterFlags. */
	RsslUInt32					serviceCount;	/*!< Number of service entries present in serviceList. */
	RsslRDMService				*serviceList;	/*!< Array of service entries. */
	RsslUInt16					serviceId;		/*!< The ID of the service whose information is provided by this stream(if not present, all services should be provided). Should match the Consumer's request if possible. */
	RsslUInt32					sequenceNumber;	/*!< Sequence number of this message. */
} RsslRDMDirectoryUpdate;

/**
 * @brief Clears an RsslRDMDirectoryUpdate.
 * @see RsslRDMDirectoryUpdate
 */
RTR_C_INLINE void rsslClearRDMDirectoryUpdate(RsslRDMDirectoryUpdate *pUpdate)
{
	rsslClearRDMMsgBase(&pUpdate->rdmMsgBase);
	pUpdate->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
	pUpdate->rdmMsgBase.rdmMsgType = RDM_DR_MT_UPDATE;
	pUpdate->flags = RDM_DR_UPF_NONE;
	pUpdate->filter = 0;
	pUpdate->serviceCount = 0;
	pUpdate->serviceList = NULL;
	pUpdate->serviceId = 0;
	pUpdate->sequenceNumber = 0;
}

/**
 * @brief Initializes an RsslRDMDirectoryRequest, clearing it and filling in a typical filter setting of
 * RDM_DIRECTORY_SERVICE_INFO_FILTER, RDM_DIRECTORY_SERVICE_STATE_FILTER, and RDM_DIRECTORY_SERVICE_GROUP_FILTER.
 * @see RsslRDMDirectoryRequest, rsslInitDefaultRDMDirectoryRequest
 */
RTR_C_INLINE RsslRet rsslInitDefaultRDMDirectoryRequest(RsslRDMDirectoryRequest *pRequest, RsslInt32 streamId)
{
	rsslClearRDMDirectoryRequest(pRequest);
	pRequest->flags = RDM_DR_RQF_STREAMING;
	pRequest->rdmMsgBase.streamId = streamId;
	pRequest->filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER;
	return RSSL_RET_SUCCESS;
}

/**
 * @brief The RDM Directory Close.  Used by an OMM Consumer to close an open Source Directory stream.
 * @see RsslRDMMsgBase, RsslRDMDirectoryMsg, rsslClearRDMDirectoryClose
 */
typedef struct {
	RsslRDMMsgBase rdmMsgBase;	/*!< The Base RDM Message. */
} RsslRDMDirectoryClose;

/**
 * @brief Clears an RsslRDMDirectoryClose.
 * @see RsslRDMDirectoryClose
 */
RTR_C_INLINE void rsslClearRDMDirectoryClose(RsslRDMDirectoryClose *pClose)
{
	memset(pClose, 0, sizeof(RsslRDMDirectoryClose));
	pClose->rdmMsgBase.rdmMsgType = RDM_DR_MT_CLOSE;
	pClose->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
}

/**
 * @brief The RDM Directory Status Flags
 * @see RsslRDMDirectoryStatus
 */
typedef enum
{
	RDM_DR_STF_NONE				= 0x00,	/*!< (0x00) No flags set. */
	RDM_DR_STF_HAS_FILTER		= 0x01,	/*!< (0x01) Indicates presence of the filter member. */
	RDM_DR_STF_HAS_SERVICE_ID	= 0x02,	/*!< (0x02) Indicates presence of the serviceId member. */
	RDM_DR_STF_HAS_STATE		= 0x04,	/*!< (0x04) Indicates presence of the state member. */
	RDM_DR_STF_CLEAR_CACHE		= 0x08	/*!< (0x08) Indicates whether the Consumer should clear any existing cached service information. */
} RsslRDMDirectoryStatusFlags;

/**
 * @brief The RDM Directory Status. Used by an OMM Provider to indicate changes to the Directory stream.
 * @see RsslRDMMsgBase, RsslRDMDirectoryMsg, rsslClearRDMDirectoryStatus
 */
typedef struct {
	RsslRDMMsgBase				rdmMsgBase;	/*!< The Base RDM Message. */
	RsslUInt32					flags;		/*!< The Directory Status flags. */
	RsslUInt32					filter;		/*!< Filter indicating which filters may appear on this stream. Where possible, this should match the consumer's request. Populated by RDMDirectoryServiceFilterFlags. */
	RsslUInt16					serviceId;	/*!< The ID of the service whose information is provided by this stream(if not present, all services should be provided). Should match the Consumer's request if possible. */
	RsslState					state;		/*!< The current state of this stream. */
} RsslRDMDirectoryStatus;

/**
 * @brief The RDM Directory Status. Used by an OMM Provider to indicate changes to the Directory stream.
 * @see RsslRDMMsgBase, RsslRDMMsg, rsslClearRDMDirectoryStatus
 */
RTR_C_INLINE void rsslClearRDMDirectoryStatus(RsslRDMDirectoryStatus *pStatus)
{
	rsslClearRDMMsgBase(&pStatus->rdmMsgBase);
	pStatus->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
	pStatus->rdmMsgBase.rdmMsgType = RDM_DR_MT_STATUS;
	pStatus->flags = RDM_DR_STF_NONE;
	pStatus->filter = 0;
	pStatus->serviceId = 0;
	rsslClearState(&pStatus->state);
	pStatus->state.streamState = RSSL_STREAM_OPEN;
	pStatus->state.dataState = RSSL_DATA_OK;
	pStatus->state.code = RSSL_SC_NONE;
}

/**
 * @brief The RDM Directory Consumer Status.  Used by an OMM Consumer to send Source Mirroring information.
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMConsumerStatusService, rsslClearRDMDirectoryConsumerStatus
 */
typedef struct
{
	RsslRDMMsgBase					rdmMsgBase;					/*!< The Base RDM Message. */
	RsslUInt32						consumerServiceStatusCount;	/*!< The number of ServiceStatus elements in the consumerServiceStatusList. */
	RsslRDMConsumerStatusService	*consumerServiceStatusList;	/*!< The array of Consumer Service Status elements. */
} RsslRDMDirectoryConsumerStatus;

/**
 * @brief Clears an RsslRDMDirectoryConsumerStatus.
 * @see RsslRDMDirectoryConsumerStatus
 */
RTR_C_INLINE void rsslClearRDMDirectoryConsumerStatus(RsslRDMDirectoryConsumerStatus *pConsumerStatus)
{
	rsslClearRDMMsgBase(&pConsumerStatus->rdmMsgBase);
	pConsumerStatus->rdmMsgBase.rdmMsgType = RDM_DR_MT_CONSUMER_STATUS;
	pConsumerStatus->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
	pConsumerStatus->consumerServiceStatusCount = 0;
	pConsumerStatus->consumerServiceStatusList = NULL;
}

/**
 * @brief The RDM Directory Msg.  The Directory Message encoder and decoder functions expect this type.
 * It is a group of the classes of message that the Directory domain supports.  Any 
 * specific message class may be cast to an RsslRDMDirectoryMsg, and an RsslRDMDirectoryMsg may be cast 
 * to any specific message class.  The RsslRDMMsgBase contains members common to each class
 * that may be used to identify the class of message.
 * @see RsslRDMDirectoryMsgType, RsslRDMMsgBase, RsslRDMDirectoryRequest, RsslRDMDirectoryClose, RsslRDMDirectoryConsumerStatus, RsslRDMDirectoryRefresh, RsslRDMDirectoryUpdate, RsslRDMDirectoryStatus, rsslClearRDMDirectoryMsg
 */
typedef union
{
	RsslRDMMsgBase					rdmMsgBase;			/*!< The Base RDM Message. */
	RsslRDMDirectoryRequest			request;			/*!< The Directory Request. */
	RsslRDMDirectoryClose			close;				/*!< The Directory close. */
	RsslRDMDirectoryConsumerStatus	consumerStatus;		/*!< The Directory Consumer Status. */
	RsslRDMDirectoryRefresh			refresh;			/*!< The Directory Refresh. */
	RsslRDMDirectoryUpdate			update;				/*!< The Directory Update. */
	RsslRDMDirectoryStatus			status;				/*!< The Directory Status */
} RsslRDMDirectoryMsg;

/**
 * @brief Clears an RsslRDMDirectoryMsg.
 * @see RsslRDMDirectoryMsg
 */
RTR_C_INLINE void rsslClearRDMDirectoryMsg(RsslRDMDirectoryMsg *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMDirectoryMsg));
	pMsg->rdmMsgBase.domainType = RSSL_DMT_SOURCE;
}

/**
 * @brief Encodes an RsslRDMDirectoryMsg.
 * @param pEncodeIter The Encode Iterator
 * @param pDirectoryMsg The RDM Directory Message to Encode
 * @param pBytesWritten Returns the total number of bytes used to encode the message.
 * @see RsslRDMDirectoryMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMDirectoryMsg(RsslEncodeIterator *pEncodeIter, RsslRDMDirectoryMsg *pDirectoryMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError);

/**
 * @brief Decodes an RsslRDMDirectoryMsg.
 * @param pEncodeIter The Decode Iterator
 * @param pDirectoryMsg The RDM Directory Message to be populated
 * @param pMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the decoded message.
 * @param pError An Error Structure, which will be populated if the decoding fails.
 * @return RSSL_RET_SUCCESS, if the message was succesfully decoded and correctly followed the RDM.
 * @return RSSL_RET_FAILURE, if the message was not successfully decoded or did  follow the RDM.  Information about the error will be stored in pError.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryMsg
 */
RSSL_VA_API RsslRet rsslDecodeRDMDirectoryMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMDirectoryMsg *pDirectoryMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError);

/**
 *	@addtogroup VARDMDirectoryHelper
 *	@{
 */

/**
 * @brief Fully copies an RsslRDMDirectoryMsg.
 * @param pNewMsg The resulting copy of the RDM Directory Message
 * @param pOldMsg The RDM Directory Message to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryMsg
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryMsg(RsslRDMDirectoryMsg *pNewMsg, RsslRDMDirectoryMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDirectoryRequest.
 * @param pNewRequest The resulting copy of the RDM Directory Request
 * @param pOldRequest The RDM Directory Request to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryRequest
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryRequest(RsslRDMDirectoryRequest *pNewRequest, RsslRDMDirectoryRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDirectoryRefresh.
 * @param pNewRefresh The resulting copy of the RDM Directory Refresh
 * @param pOldRefresh The RDM Directory Refresh to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryRefresh
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryRefresh(RsslRDMDirectoryRefresh *pNewRefresh, RsslRDMDirectoryRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDirectoryUpdate.
 * @param pNewUpdate The resulting copy of the RDM Directory Update
 * @param pOldUpdate The RDM Directory Update to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryUpdate
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryUpdate(RsslRDMDirectoryUpdate *pNewUpdate, RsslRDMDirectoryUpdate *pOldUpdate, RsslBuffer *pNewMemoryBuffer);


/**
 * @brief Fully copies an RsslRDMDirectoryClose.
 * @param pNewClose The resulting copy of the RDM Directory Close
 * @param pOldClose The RDM Directory Close to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryClose
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryClose(RsslRDMDirectoryClose *pNewClose, RsslRDMDirectoryClose *pOldClose, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDirectoryConsumerStatus.
 * @param pNewConsStatus The resulting copy of the RDM Login ConsumerStatus
 * @param pOldConsStatus The RDM Directory ConsumerStatus to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryConsumerStatus
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryConsumerStatus(RsslRDMDirectoryConsumerStatus *pNewConsStatus, RsslRDMDirectoryConsumerStatus *pOldConsStatus, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDirectoryStatus.
 * @param pNewStatus The resulting copy of the RDM Directory Status
 * @param pOldStatus The RDM Directory Status to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDirectoryStatus
 */
RSSL_VA_API RsslRet rsslCopyRDMDirectoryStatus(RsslRDMDirectoryStatus *pNewStatus, RsslRDMDirectoryStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer);

/**
 *	@}
 */

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
