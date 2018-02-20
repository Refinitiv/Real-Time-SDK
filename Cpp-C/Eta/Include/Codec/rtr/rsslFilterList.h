/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_FILTER_LIST_H
#define __RSSL_FILTER_LIST_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"


/**
 * @addtogroup FLStruct
 * @{
 */


/**
 * @brief Flag values for use with the RsslFilterList. (FTF = FilterList Flags)
 * @see RsslFilterList
 */
typedef enum
{
	RSSL_FTF_NONE						= 0x00,			/*!< (0x00) No RsslFilterList flags are present. */
	RSSL_FTF_HAS_PER_ENTRY_PERM_DATA	= 0x01,			/*!< (0x01) The RsslFilterList contains entries that have permission expressions, contained in relevant entries RsslFilterEntry::permData */
	RSSL_FTF_HAS_TOTAL_COUNT_HINT		= 0x02 			/*!< (0x02) The RsslFilterList contains a total count hint, contained in RsslFilterList::totalCountHint.  */
} RsslFilterListFlags;

/** 
 * @brief Strings associated with the different filter list flags.
 * @see RsslFilterListFlags, rsslFilterListFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_FTF_HAS_PER_ENTRY_PERM_DATA = { 19, (char*)"HasPerEntryPermData" };
static const RsslBuffer RSSL_OMMSTR_FTF_HAS_TOTAL_COUNT_HINT = { 17, (char*)"HasTotalCountHint" };

/**
 * @brief Provide general OMM string representation of RsslFilterListFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslFilterListFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslFilterListFlags
 */
RSSL_API RsslRet rsslFilterListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);


/**
 * @brief The RsslFilterList is a non-uniform container type of filterId (8-bit identifier) – value pair entries.  Each entry, known as
 * an RsslFilterEntry, contains an id corresponding to one of 32 possible bit-value identifiers.  These identifiers
 * are typically defined by a domain model specification and can be used to indicate interest or presence of specific
 * entries through the inclusion of the of the id in the RsslMsgKey::filter. 
 * @see RSSL_INIT_FILTER_LIST, rsslClearFilterList, RsslFilterListFlags
 */
typedef struct
{
	RsslUInt8           flags;			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslFilterListFlags */

	RsslContainerType	containerType;	/*!< @brief The most common container type associated with the payload of each RsslFilterEntry (e.g. if there are eight entries and six are ::RSSL_DT_MAP and the remaining two are ::RSSL_DT_FIELD_LIST, this would be set to ::RSSL_DT_MAP)*/

	RsslUInt8			totalCountHint; /*!< @brief Contains an approximate total RsslFilterEntry count for the RsslFilterList */

	RsslBuffer          encEntries;		/*!< @brief Raw encoded payload for the RsslFilterList */
} RsslFilterList;


/**
 * @brief RsslFilterList static initializer
 * @see RsslFilterList, rsslClearFilterList
 */
#define RSSL_INIT_FILTER_LIST { 0, 0, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an rsslFilterList
 * @see RsslFilterList, RSSL_INIT_FILTER_LIST
 */
RTR_C_INLINE void rsslClearFilterList(RsslFilterList *pFilterList)
{
	pFilterList->flags = 0;
	pFilterList->containerType = 0;
	pFilterList->totalCountHint = 0;
	pFilterList->encEntries.length = 0;
	pFilterList->encEntries.data = 0;
}

/**
 * @}
 */

/**
 * @addtogroup FLEStruct
 * @{
 */

/**
 * @brief Flag values for use with each RsslFilterEntry contained in an RsslFilterList. (FTEF = FilterEntry Flags) 
 * @see RsslFilterEntry, RsslFilterList
 */
typedef enum
{
	RSSL_FTEF_NONE					= 0x00,	/*!< (0x00) No RsslFilterEntry flags are present */
	RSSL_FTEF_HAS_PERM_DATA			= 0x01,	/*!< (0x01) This RsslFilterEntry contains permission data, located in RsslFilterEntry::permData. */
	RSSL_FTEF_HAS_CONTAINER_TYPE	= 0x02	/*!< (0x02) This RsslFilterEntry has a container type specified and present in RsslFilterEntry::containerType - this type may differ from the RsslFilterList::containerType  */
} RsslFilterEntryFlags;

/** 
 * @brief General OMM strings associated with the different filter entry flags.
 * @see RsslFilterEntryFlags, rsslFilterEntryFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_FTEF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_FTEF_HAS_CONTAINER_TYPE = { 16, (char*)"HasContainerType" };

/**
 * @brief Provide general OMM string representation of RsslFilterEntryFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslFilterEntryFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslFilterEntryFlags
 */
RSSL_API RsslRet rsslFilterEntryFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);

/**
 * @brief Action that indicates how to apply contents of an RsslFilterEntry (FTEA = FilterEntry Action) 
 * @see RsslFilterEntry, RsslFilterList
 */
typedef enum
{
	RSSL_FTEA_UPDATE_ENTRY	= 1,	/*!< (1) Apply the contents of this entry as an update at the specified id.  If an RsslFilterEntry with an ::RSSL_FTEA_UPDATE_ENTRY action is received prior to an ::RSSL_FTEA_SET_ACTION action for the same RsslFilterEntry::id, the entry with the ::RSSL_FTEA_UPDATE_ENTRY action can be ignored.  */
	RSSL_FTEA_SET_ENTRY		= 2,	/*!< (2) Set/Replace the contents of the specified RsslFilterEntry::id with the contents of this entry */
	RSSL_FTEA_CLEAR_ENTRY	= 3		/*!< (3) Clear the contents of the entry at the specified RsslFilterEntry::id, leaving the id present but empty. @note ::RSSL_FTEA_CLEAR_ENTRY actions cannot carry any entry payload.  If payload is necessary for a ::RSSL_FTEA_CLEAR_ENTRY action, an entry with an ::RSSL_FTEA_UPDATE_ENTRY action can be sent and immediatley followed by the same RsslFilterEntry::id on an entry with a ::RSSL_FTEA_CLEAR_ENTRY action.   */
} RsslFilterEntryActions;

/** 
 * @brief General OMM strings associated with the different filter entry actions
 * @see RsslFilterEntryActions, rsslFilterEntryActionToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_FTEA_UPDATE_ENTRY = { 6, (char*)"Update" };
static const RsslBuffer RSSL_OMMSTR_FTEA_SET_ENTRY = { 3, (char*)"Set" };
static const RsslBuffer RSSL_OMMSTR_FTEA_CLEAR_ENTRY = { 5, (char*)"Clear" };

/**
 * @brief Provide a general OMM string representation for a filter entry action enumeration
 * @see RsslFilterEntryActions
 */
RSSL_API const char* rsslFilterEntryActionToOmmString(RsslUInt8 action);

/**
 * @brief One entry contained in an RsslFilterList. If RsslFilterEntry::flags contains ::RSSL_FTEF_HAS_CONTAINER_TYPE, the contents of the entry is specified by RsslFilterEntry::containerType; otherwise the contents of the entry is specified by RsslFilterList::containerType.  Any contents should be applied following the rules of the specified RsslFilterEntry::action ( \ref RsslFilterEntryActions)
 * @see RSSL_INIT_FILTER_ENTRY, rsslClearFilterEntry, RsslFilterEntryFlags, RsslFilterEntryActions, rsslDecodeFilterEntry, rsslEncodeFilterEntryInit, rsslEncodeFilterEntryComplete
 */
typedef struct {
	RsslUInt8			flags;			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslFilterEntryFlags */
	RsslUInt8			action;			/*!< @brief Action to use when applying RsslFilterEntry information.  The available actions are defined in RsslFilterEntryActions enumeration. */
	RsslUInt8			id;				/*!< @brief Identifier corresponding to one of 32 possible bit-value identifiers.  These identifiers are typically defined by a domain model specification and can be used to indicate interest or presence of specific entries through the inclusion of the of the id in the RsslMsgKey::filter. */
	RsslContainerType	containerType;	/*!< @brief The container type associated with the payload of this RsslFilterEntry */
	RsslBuffer			permData;		/*!< @brief Contains permission data associated with this RsslFilterEntry and its contents.  Presence is indicated by \ref RsslFilterEntryFlags::RSSL_FTEF_HAS_PERM_DATA.   */
    RsslBuffer			encData;		/*!< @brief Raw encoded content for this RsslFilterEntry. */
} RsslFilterEntry;

/**
 * @brief RsslFilterEntry static initializer
 * @see RsslFilterEntry, rsslClearFilterEntry
 */
#define RSSL_INIT_FILTER_ENTRY { 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }


/**
 * @brief Clears an RsslFilterEntry 
 * @see RsslFilterEntry, RSSL_INIT_FILTER_ENTRY
 */
RTR_C_INLINE void rsslClearFilterEntry(RsslFilterEntry *pFilterEntry)
{
	memset(pFilterEntry, 0, sizeof(RsslFilterEntry));
}

/**
 * @}
 */


/** 
 * @addtogroup FilterListEncoding RsslFilterList and RsslFilterEntry Encoding
 * @{
 */


/** 
 * @brief 	Begin encoding process for RsslFilterList container type.
 *
 * Begins encoding of an RsslFilterList<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFilterListInit()<BR>
 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFilterList	partially populated RsslFilterList structure to encode
 * @see RsslFilterList, RsslEncodeIterator, rsslEncodeFilterListComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeFilterListInit( 
								RsslEncodeIterator	*pIter,
								RsslFilterList		*pFilterList  );
							 



/** 
 * @brief 	Completes encoding of an RsslFilterList
 *
 * Completes RsslFilterList encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFilterListInit()<BR>
 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the filter list encoding, if false - remove the filter list payload from the buffer.
 * @see rsslEncodeFilterListInit, RsslFilterList, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFilterListComplete( 
								RsslEncodeIterator	*pIter,
								RsslBool			success );



/** 
 * @brief 	Encodes an RsslFilterEntry where there is no payload or any payload is pre-encoded and populated on RsslFilterEntry::encData.  
 *
 * Encodes an RsslFilterEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFilterListInit()<BR>
 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFilterEntry populated RsslFilterEntry to encode.  If any payload is present, it must be populated on RsslFilterEntry::encData.  
 * @see RsslEncodeIterator, RsslFilterEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFilterEntry( 
								RsslEncodeIterator	*pIter,
 								RsslFilterEntry		*pFilterEntry );
                    


/** 
 * @brief 	Begins encoding of an RsslFilterEntry, where any payload is encoded after this call using the appropriate container type encode functions, as specified by RsslFilterList::containerType or RsslFilterEntry::containerType.  
 *
 * Begins encoding of an RsslFilterEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFilterListInit()<BR>
 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFilterEntry populated RsslFilterEntry to encode
 * @param maxEncodingSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslFilterEntry, rsslEncodeFilterEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFilterEntryInit(
								RsslEncodeIterator	*pIter,
 								RsslFilterEntry		*pFilterEntry,
								RsslUInt16			maxEncodingSize );
                    



/** 
 * @brief 	Completes encoding of an RsslFilterEntry.  
 *
 * Completes encoding of an RsslFilterEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFilterListInit()<BR>
 *  2. To encode entries, call rsslEncodeFilterEntry() or rsslEncodeFilterEntryInit()..rsslEncodeFilterEntryComplete() for each RsslFilterEntry<BR>
 *  3. Call rsslEncodeFilterListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslVectorEntry encoding, if false - remove the RsslVectorEntry from the encoded RsslVector.
 * @see RsslEncodeIterator, RsslFilterEntry, rsslEncodeFilterEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFilterEntryComplete( 
								RsslEncodeIterator	*pIter,
 								RsslBool			success );



/**
 * @}
 */
 
/** 
 * @addtogroup FilterListDecoding RsslFilterList and RsslFilterEntry Decoding
 * @{
 */

/**
 * @brief Decodes an RsslFilterList container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeFilterList()<BR>
 *  2. Call rsslDecodeFilterEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pFilterList RsslFilterList structure to populate with decoded contents.  RsslFilterList::encEntries will point to encoded RsslFilterEntry content.
 * @see rsslDecodeFilterEntry, RsslFilterList, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeFilterList(
								RsslDecodeIterator	*pIter, 
								RsslFilterList		*pFilterList );
							  

/**
 * @brief Decodes and returns a single RsslFilterEntry from within an RsslFilterList::encEntries
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeFilterList()<BR>
 *  2. Call rsslDecodeFilterEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pFilterEntry RsslFilterEntry to decode content into.  RsslFilterEntry::encData will contain encoded content of entry.  
 * @see rsslDecodeFilterList, RsslFilterEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeFilterEntry(
								RsslDecodeIterator  *pIter, 
								RsslFilterEntry		*pFilterEntry );
						  	  


/**
 * @}
 */
 

/**
 * @addtogroup FLStruct
 * @{
 */



/**
 * @brief Checks the presence of the ::RSSL_FTF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslFilterList.
 *
 * @param pFilterList Pointer to the RsslFilterList to check.
 * @see RsslFilterList, RsslFilterListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslFilterListCheckHasPerEntryPermData(RsslFilterList *pFilterList)
{
	return ((pFilterList->flags & RSSL_FTF_HAS_PER_ENTRY_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Checks the presence of the ::RSSL_FTF_HAS_TOTAL_COUNT_HINT flag on the given RsslFilterList.
 *
 * @param pFilterList Pointer to the RsslFilterList to check.
 * @see RsslFilterList, RsslFilterListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslFilterListCheckHasTotalCountHint(RsslFilterList *pFilterList)
{
	return ((pFilterList->flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT) ?
					RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Applies the ::RSSL_FTF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslFilterList.
 * 
 * @param pFilterList Pointer to the RsslFilterList to apply flag value to.
 * @see RsslFilterList, RsslFilterListFlags
 * @return No return value
 */
RTR_C_INLINE void rsslFilterListApplyHasPerEntryPermData(RsslFilterList *pFilterList)
{
	pFilterList->flags |= RSSL_FTF_HAS_PER_ENTRY_PERM_DATA;
}


/**
 * @brief Applies the ::RSSL_FTF_HAS_TOTAL_COUNT_HINT flag on the given RsslFilterList.
 * 
 * @param pFilterList Pointer to the RsslFilterList to apply flag value to.
 * @see RsslFilterList, RsslFilterListFlags
 * @return No return value
 */
RTR_C_INLINE void rsslFilterListApplyHasTotalCountHint(RsslFilterList *pFilterList)
{
	pFilterList->flags |= RSSL_FTF_HAS_TOTAL_COUNT_HINT;
}


/**
 *	@}
 */


/**
 * @addtogroup FLEStruct
 * @{
 */


/**
 * @brief Checks the presence of the ::RSSL_FTEF_HAS_PERM_DATA flag on the given RsslFilterEntry.
 *
 * @param pFilterEntry Pointer to the RsslFilterEntry to check.
 * @see RsslFilterEntry, RsslFilterEntryFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslFilterEntryCheckHasPermData(RsslFilterEntry *pFilterEntry)
{
	return ((pFilterEntry->flags & RSSL_FTEF_HAS_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE);
}

/**
 * @brief Checks the presence of the ::RSSL_FTEF_HAS_CONTAINER_TYPE flag on the given RsslFilterEntry.
 *
 * @param pFilterEntry Pointer to the RsslFilterEntry to check.
 * @see RsslFilterEntry, RsslFilterEntryFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslFilterEntryCheckHasContainerType(RsslFilterEntry *pFilterEntry)
{
	return ((pFilterEntry->flags & RSSL_FTEF_HAS_CONTAINER_TYPE) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Applies the ::RSSL_FTEF_HAS_PERM_DATA flag on the given RsslFilterEntry.
 * 
 * @param pFilterEntry Pointer to the RsslFilterEntry to apply flag value to.
 * @see RsslFilterEntry, RsslFilterEntryFlags
 * @return No return value
 */
RTR_C_INLINE void rsslFilterEntryApplyHasPermData(RsslFilterEntry *pFilterEntry)
{
	pFilterEntry->flags |= RSSL_FTEF_HAS_PERM_DATA;
}

#define rsslFilterEntrySetHasPermData rsslFilterEntryApplyHasPermData

/**
 * @brief Applies the ::RSSL_FTEF_HAS_CONTAINER_TYPE flag on the given RsslFilterEntry.
 * 
 * @param pFilterEntry Pointer to the RsslFilterEntry to apply flag value to.
 * @see RsslFilterEntry, RsslFilterEntryFlags
 * @return No return value
 */
RTR_C_INLINE void rsslFilterEntryApplyHasContainerType(RsslFilterEntry *pFilterEntry)
{
	pFilterEntry->flags |= RSSL_FTEF_HAS_CONTAINER_TYPE;
}

#define rsslFilterEntrySetHasContainerType rsslFilterEntryApplyHasContainerType


/**
 *	@}
 */


#ifdef __cplusplus
}
#endif

#endif
