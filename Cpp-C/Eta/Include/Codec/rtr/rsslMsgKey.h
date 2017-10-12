

#ifndef __RSSL_KEYS_H
#define __RSSL_KEYS_H

#include "rtr/rsslTypes.h"

/************************************
 * Key enums and flags
 ************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup MsgKey
 * @{
 */


/**
 * @brief The RsslMsgKey contains a variety of attributes used to identify the
 * contents flowing within a particular stream. This information, in conjunction with
 * domainType and quality of service information, can be used to uniquely identify a
 * data stream.
 * 
 * @see RsslMsg
 */
typedef struct {
	RsslUInt16		flags;				 /*!< @brief Flag values used to indicate optional member presence. The available options are defined by values present in \ref RsslMsgKeyFlags. */
	RsslUInt16		serviceId;			 /*!< @brief The serviceId is a two-byte unsigned integer used to identify a specific service.   */
	RsslUInt8		nameType;			 /*!< @brief Name Type. This indicates the type of the name member, examples are ‘User Name’ or ‘RIC’ (Reuters Instrument Code). */
	RsslBuffer		name;				 /*!< @brief Name associated with the contents of the item stream. */
	RsslUInt32		filter;			  	 /*!< @brief Filter specification, used to request or indicate which filter entries are present with a filter list payload.  */
	RsslInt32		identifier;			 /*!< @brief User specified numeric identifier. Usage of this is defined on a per-domain model basis. */
	RsslUInt8		attribContainerType; /*!< @brief Container Type of the msgKey attributes.  Must be a container type from the RsslDataTypes enumeration */
	RsslBuffer		encAttrib;			 /*!< @brief Encoded MsgKey attribute information, used for additional item stream identification attributes.  Contents are typically specified in the domain model.  Type is specified by attribContainerType  */
} RsslMsgKey;

/**
 * @brief Static initializer for RsslMsgKey
 *
 * @note The static initializer is intended for use by the RSSL message structure static initializers.  Most users will not need to use either this or the clear function to clear an RsslMsgKey, unless they are storing the RsslMsgKey structures outside of the message structures.
 * @see RsslMsgKey, rsslClearMsgKey
 */
#define RSSL_INIT_MSG_KEY { 0, 0, 0, RSSL_INIT_BUFFER, 0, 0, 0, RSSL_INIT_BUFFER}


/**
 * @brief Clears an Rssl message key
 * @see RsslMsgKey, RSSL_INIT_MSG_KEY
 */
RTR_C_ALWAYS_INLINE void rsslClearMsgKey(RsslMsgKey *pKey)
{
	memset(pKey, 0, sizeof(RsslMsgKey));
}
						
/**
 * @brief RSSL Message Key Flags (MKF = MsgKey Flag)
 *
 * @see RsslMsgKey
 */
typedef enum
{
	RSSL_MKF_NONE			= 0x0000,	/*!< (0x0000) No RsslMsgKeyFlags flags are present  */
	RSSL_MKF_HAS_SERVICE_ID	= 0x0001,	/*!< (0x0001) This RsslMsgKey has a service id, contained in \ref RsslMsgKey::serviceId.  */
	RSSL_MKF_HAS_NAME		= 0x0002,	/*!< (0x0002) This RsslMsgKey has a name buffer, contained in \ref RsslMsgKey::name.  */
	RSSL_MKF_HAS_NAME_TYPE	= 0x0004,	/*!< (0x0004) This RsslMsgKey has a nameType enumeration, contained in \ref RsslMsgKey::nameType.  */
	RSSL_MKF_HAS_FILTER		= 0x0008,	/*!< (0x0008) This RsslMsgKey has a filter, contained in \ref RsslMsgKey::filter.  */
	RSSL_MKF_HAS_IDENTIFIER	= 0x0010,	/*!< (0x0010) This RsslMsgKey has a numeric identifier, contained in \ref RsslMsgKey::identifier*/
	RSSL_MKF_HAS_ATTRIB		= 0x0020	/*!< (0x0020) This RsslMsgKey has additional attribute information, contained in \ref RsslMsgKey::encAttrib. The container type of the attribute information is contained in \ref RsslMsgKey::attribContainerType. */
} RsslMsgKeyFlags;

/** 
 * @brief General OMM strings associated with the different message key flags.
 * @see RsslMsgKeyFlags, rsslMsgKeyFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_SERVICE_ID = { 12, (char*)"HasServiceID" };
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_NAME = { 7, (char*)"HasName" };
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_NAME_TYPE = { 11, (char*)"HasNameType" };
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_FILTER = { 9, (char*)"HasFilter" };
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_IDENTIFIER = { 13, (char*)"HasIdentifier" };
static const RsslBuffer RSSL_OMMSTR_MKF_HAS_ATTRIB = { 9, (char*)"HasAttrib" };

/**
 * @brief Provide general OMM string representation of RsslMsgKeyFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslMsgKeyFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslMsgKeyFlags
 */
RSSL_API RsslRet rsslMsgKeyFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */

/**
 *	@defgroup MsgKeyHelpers Message Key Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_SERVICE_ID flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasServiceId(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_SERVICE_ID) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_NAME flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasName(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_NAME) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_NAME_TYPE flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasNameType(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_NAME_TYPE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_FILTER flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasFilter(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_FILTER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_IDENTIFIER flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasIdentifier(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_IDENTIFIER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MKF_HAS_ATTRIB flag on the given RsslMsgKey.
 *
 * @param pMsgKey Pointer to the message key.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMsgKeyCheckHasAttrib(RsslMsgKey *pMsgKey)
{
	return ((pMsgKey->flags & RSSL_MKF_HAS_ATTRIB) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_SERVICE_ID flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasServiceId(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_SERVICE_ID;
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_NAME flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasName(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_NAME;
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_NAME_TYPE flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasNameType(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_NAME_TYPE;
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_FILTER flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasFilter(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_FILTER;
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_IDENTIFIER flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasIdentifier(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_IDENTIFIER;
}

/**
 * @brief Applies the ::RSSL_MKF_HAS_ATTRIB flag on the given RsslRefreshMsg.
 * 
 * @param pMsgKey Pointer to the message key.
 */
RTR_C_ALWAYS_INLINE void rsslMsgKeyApplyHasAttrib(RsslMsgKey *pMsgKey)
{
	pMsgKey->flags |= RSSL_MKF_HAS_ATTRIB;
}

/** 
 * @brief Compares two MsgKey structures to determine if they are the same
 * 
 * @param pKey1 First key to compare 
 * @param pKey2 Second key to compare
 * @return RsslRet returns RSSL_RET_SUCCESS if keys match, RSSL_RET_FAILURE otherwise.
 */
RSSL_API RsslRet 	rsslCompareMsgKeys(const RsslMsgKey* pKey1, const RsslMsgKey* pKey2);
 
/** 
 * @brief Performs a deep copy of a MsgKey.  Expects all memory to be owned and managed by user.  
 * 
 * @param pDestKey Destination to copy into.  Should have sufficient memory to contain copy of sourceKey.
 * @param pSourceKey Source to copy from.  
 * @return RsslRet returns RSSL_RET_SUCCESS if copy succeeds, RSSL_RET_FAILURE otherwise.
 */
RSSL_API RsslRet	rsslCopyMsgKey(RsslMsgKey* pDestKey, const RsslMsgKey* pSourceKey);

/** 
 * @brief generates a hash ID from a message key.  
 * 
 * @returns hash ID
 */
RSSL_API RsslUInt32	rsslMsgKeyHash(const RsslMsgKey* pMsgKey);


/** 
 * @brief Adds a FilterId to the key filter
 * 
 * @param filterId The FilterId you want added to the filter. (e.g. RSSL_SERVICE_STATE_ID)
 * @param pFilter The filter to add the FilterId to
 */
RSSL_API RsslRet	rsslAddFilterIdToFilter( const RsslUInt8 filterId, 
												 RsslUInt32 *pFilter);

/** 
 * @brief Checks if FilterId is present in key filter
 * 
 * @param filterId The FilterId you want to check for
 * @param filter The filter to check for the FilterId
 */
RSSL_API RsslBool	rsslCheckFilterForFilterId(const RsslUInt8 filterId, 
												   const RsslUInt32 filter);

/**
 *	@}
 */
 



#ifdef __cplusplus
}
#endif


#endif


