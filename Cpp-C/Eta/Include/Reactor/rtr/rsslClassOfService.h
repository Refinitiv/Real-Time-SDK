/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef RSSL_CLASS_OF_SERVICE_H
#define RSSL_CLASS_OF_SERVICE_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMemoryBuffer.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactorCallbackReturnCodes.h"
#include "rtr/rsslReactorChannel.h"
#include "rtr/rsslClassOfService.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
  * @brief Represents Common class of service properties used within a qualified stream.
  */
typedef struct
{
	RsslUInt	maxMsgSize;				/*!< The maximum message size to be used on the qualified stream. This value is sent only by providers -- consumers do not need to configure it. */
	RsslUInt8	protocolType;			/*!< The protocol type to be used in the qualified stream. */
	RsslUInt8	protocolMajorVersion;	/*!< The major version of the protocol to be used in the qualified stream. */
	RsslUInt8	protocolMinorVersion;	/*!< The minor version of the protocol to be used in the qualified stream. */
} RsslClassOfServiceCommon;

/**
  * @brief Represents Authentication class of service properties used within a qualified stream.
  */
typedef struct
{
	RDMClassOfServiceAuthenticationType	type;	/*!< The type of authentication to use. See RDMClassOfServiceAuthenticationType. */
} RsslClassOfServiceAuthentication;

/**
  * @brief Represents Flow Control class of service properties used within a qualified stream.
  */
typedef struct
{
	RsslUInt	type;			/*!< The type of flow control to use. See RDMClassOfServiceFlowControlType. */
	RsslInt		recvWindowSize;	/*!< The largest amount of data that the remote end of the stream should send at any time when performing flow control. */
	RsslInt		sendWindowSize; /*!< Read-only. The largest amount of data that this end of the stream should send at any time when performing flow control. */
} RsslClassOfServiceFlowControl;

/**
  * @brief Represents Data Integrity class of service properties used within a qualified stream.
  */
typedef struct
{
	RsslUInt	type;	/*!< The type of data integrity to use. See RDMClassOfServiceDataIntegrityType. */
} RsslClassOfServiceDataIntegrity;

/**
  * @brief Represents Guarantee class of service properties used within a qualified stream.
  */
typedef struct
{
	RsslUInt	type;					/*!< The type of guarantee to use. See RDMClassOfServiceGuaranteeType. */
	RsslBool	persistLocally;			/*!< Consumers only. Indicates whether messages are persisted to a local file. */
	char		*persistenceFilePath;   /*!< Consumers only. Path for storing persistence files, if local persistence is enabled. */
} RsslClassOfServiceGuarantee;


/**
  * @brief Used to configure and indicate negotiated parameters of a qualified stream.
  */
typedef struct
{
	RsslClassOfServiceCommon			common;			/*!< Common properties */
	RsslClassOfServiceAuthentication	authentication;	/*!< Authentication properties */
	RsslClassOfServiceFlowControl		flowControl;	/*!< Flow Control properties */
	RsslClassOfServiceDataIntegrity		dataIntegrity;	/*!< Data Integrity properties */
	RsslClassOfServiceGuarantee			guarantee;		/*!< Guarantee properties */
} RsslClassOfService;

/**
 * @brief Clears an RsslClassOfService object.
 * @see RsslClassOfService
 */
RTR_C_INLINE void rsslClearClassOfService(RsslClassOfService *pClass)
{
	pClass->common.maxMsgSize = 6144;
	pClass->common.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	pClass->common.protocolMajorVersion = RSSL_RWF_MAJOR_VERSION;
	pClass->common.protocolMinorVersion = RSSL_RWF_MINOR_VERSION;
	pClass->authentication.type = RDM_COS_AU_NOT_REQUIRED;
	pClass->flowControl.type = RDM_COS_FC_NONE;
	pClass->flowControl.recvWindowSize = -1;
	pClass->flowControl.sendWindowSize = -1;
	pClass->dataIntegrity.type = RDM_COS_DI_BEST_EFFORT;
	pClass->guarantee.type = RDM_COS_GU_NONE;
	pClass->guarantee.persistLocally = RSSL_TRUE;
	pClass->guarantee.persistenceFilePath = NULL;
}

#ifdef __cplusplus
};
#endif

#endif
