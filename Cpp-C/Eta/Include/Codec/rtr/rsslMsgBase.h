/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_MSGBASE_H_
#define __RSSL_MSGBASE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslState.h"
#include "rtr/rsslMsgKey.h"
#include "rtr/rsslIterators.h"

/**
 * @addtogroup MsgBaseStruct
 * @{
 */


/** 
 * @brief The RsslMsgBase contains common members that are used across all Rssl Messages.
 * @see RsslMsg, RSSL_INIT_MSG_BASE, rsslClearMsgBase
 */
typedef struct {
	RsslUInt8			msgClass;		/*!< @brief Class of this message (Update, Refresh, Status, etc).  Populated from RsslMsgClasses enumeration */
	RsslUInt8			domainType;		/*!< @brief Domain Type of this message, corresponds to a domain model definition Values less than 128 are Refinitiv defined domain models, values between 128 – 255 are user defined domain models. */
	RsslContainerType	containerType;	/*!< @brief Container type that is held in the encDataBody. */
	RsslInt32			streamId;		/*!< @brief Unique signed-integer identifier associated with all messages flowing within a stream. Positive values indicate a consumer instantiated stream, negative values indicate a provider instantiated stream often associated with non-interactive providers. */
	RsslMsgKey			msgKey;			/*!< @brief Key providing unique identifier information for an item stream. The msgKey, in conjunction with quality of service and domainType, is used to uniquely identify a stream. See \ref RsslMsgKey for more details.  */
	RsslBuffer			encDataBody;    /*!< @brief Contains the encoded payload contents of the message.  This buffer is populated by the user when encoding a pre-encoded payload.  This is also populated during decode. */
	RsslBuffer			encMsgBuffer;	/*!< @brief Contains the entire encoded message, including both the encoded header and payload. This buffer is typically only populated during decode. */
} RsslMsgBase;


/**
 * @brief Static initializer for RsslMsgBase
 *
 * @note The static initializer is intended for use by the RSSL message structure static initializers.  Most users will not need to use either this or the clear function to clear an RsslMsgBase structure.
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 *
 * @see RsslMsgBase, rsslClearMsgBase
 */ 

#define	RSSL_INIT_MSG_BASE { 0, 0, 0, 0, RSSL_INIT_MSG_KEY, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }


/**
 * @brief Clears RsslMsgBase
 * @see RsslMsgBase, RSSL_INIT_MSG_BASE
 */
RTR_C_ALWAYS_INLINE void rsslClearMsgBase(RsslMsgBase *pMsgBase)  
{
	memset(pMsgBase, 0, sizeof(RsslMsgBase));
}

/**
 * @}
 */
 
/**
 *	@defgroup MsgUtilsHelpers
 *	@{
 */

/** 
 * @brief Converts the provided message class enumeration to a string.
 * @param msgClass message class.
 * @return Null terminated character string containing the name of the message class.
 * @see RsslMsgClasses
 */

RSSL_API const char* rsslMsgClassToString(RsslUInt8 msgClass);

/** 
 * @brief Converts the provided message class enumeration to a general OMM string.
 * @param msgClass message class.
 * @return Null terminated character string containing the name of the message class.
 * @see RsslMsgClasses
 */

RSSL_API const char* rsslMsgClassToOmmString(RsslUInt8 msgClass);

/**
 * @}
 */
 
/**
 *	@defgroup RSSLWFDomainHelpers Reuters Domain Model Helper Functions
 * 	@{
 */

/** 
 * @brief Converts the provided domain type enumeration to a string.
 *
 * @param domainType Domain type enumeration to translate to string.
 * @return Null terminated character string containing the name of the domain type.
 * @see RsslDomainTypes
 */
RSSL_API const char* rsslDomainTypeToString(RsslUInt8 domainType);

/** 
 * @brief Converts the provided domain type enumeration to a general OMM string.
 *
 * @param domainType Domain type enumeration to translate to string.
 * @return Null terminated character string containing the name of the domain type.
 * @see RsslDomainTypes
 */
RSSL_API const char* rsslDomainTypeToOmmString(RsslUInt8 domainType);



/** 
 * @brief Returns domainType enummeration value from a domain type string.
 * @param domainTypeString domain type string representation.
 * @return Eight bit unsigned integer containing the domain type enumeration that corresponds to the provided string.
 * @see RsslDomainTypes
 */
RSSL_API RsslUInt8 rsslDomainTypeFromString(char *domainTypeString);

/**
 * @}
 */

/**
 * @addtogroup PostUserInfoStruct 
 * @{
 */

/** 
 * @brief The RsslPostUserInfo is used to identify the user that has posted a message.
 * @see RsslMsg, RSSL_INIT_POST_USER_INFO, rsslClearPostUserInfo, RsslPostMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg
 */
typedef struct {
	RsslUInt32		postUserAddr;	/*!< @brief Four byte, network byte order IP Address of user that posted this data. */
	RsslUInt32		postUserId;		/*!< @brief Identifier of the specific user that posted this data  */
} RsslPostUserInfo;

/**
 * @brief Static Initializer for RsslPostUserInfo
 * @see RsslPostMsg, RsslPostUserInfo
 */
#define RSSL_INIT_POST_USER_INFO { 0, 0 }

/**
 * @brief Clears RSSL Post User Info
 * @see RsslPostMsg, RsslPostUserInfo
 */
RTR_C_ALWAYS_INLINE void rsslClearPostUserInfo(RsslPostUserInfo *pPostUserInfo)
{
	pPostUserInfo->postUserAddr = 0;
	pPostUserInfo->postUserId = 0;
}

/**
 * @}
 */

/**
 * @addtogroup PostUserInfoHelpers 
 * @{
 */

/** 
 * @brief Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer equivalent.
 * 
 * @param[in] pAddrString 	The IP address string.
 * @param[out] pAddrUInt 	The output integer value, in host byte order.
 * @return RSSL_RET_SUCCESS, or RSSL_RET_FAILURE if the string could not be parsed.
 */
RSSL_API RsslRet	rsslIPAddrStringToUInt(const char *pAddrString, RsslUInt32 *pAddrUInt);

/**
* @brief Converts dotted-decimal IP address string (e.g. "127.0.0.1") to integer equivalent.
*
* @param[in] pAddrString  The RsslBuffer containing an IP address string and its length.
* @param[out] pAddrUInt   The output integer value, in host byte order.
* @return RSSL_RET_SUCCESS, or RSSL_RET_FAILURE if the string could not be parsed.
*/
RSSL_API RsslRet rsslIPAddrBufferToUInt(RsslUInt32 *pAddrUInt, const RsslBuffer *pAddrString);

/** 
 * @brief Converts IPv4 address in integer format to string equivalent.
 * 
 * @param[in] addrUInt 		The input integer value, in host byte order.
 * @param[out] pAddrString 	The array to fill with the IP Address string. This array must be at least 16 characters in size.
 */
RSSL_API void rsslIPAddrUIntToString(RsslUInt32 addrUInt, char *pAddrString);


/**
 * @}
 */



#ifdef __cplusplus
}
#endif


#endif

