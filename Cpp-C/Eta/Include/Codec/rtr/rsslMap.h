
#ifndef __RSSL_MAP_H
#define __RSSL_MAP_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"

/**
 * @addtogroup MStruct 
 *	@{
 */


/**
 * @brief Flag values for use with the RsslMap. (MPF = Map Flags)
 * @see RsslMap
 */
typedef enum
{
	RSSL_MPF_NONE						= 0x00,			/*!< (0x00) No RsslMap flags are present. */
	RSSL_MPF_HAS_SET_DEFS				= 0x01,			/*!< (0x01) The RsslMap contains local set definition information.  If pre-encoded or decoding, this will be located in RsslMap::encSetDefs.  It can also be encoded using the set definition encoding functionality and, upon successful set def encoding, calling rsslEncodeMapSetDefsComplete(). */
	RSSL_MPF_HAS_SUMMARY_DATA			= 0x02,			/*!< (0x02) The RsslMap contains summary data information.  If pre-encoded or decoding, this will be located in RsslMap::encSummaryData.  It can also be encoded with the appropriate container encode functions and, upon successful summary data container encoding, calling rsslEncodeMapSummaryDataComplete().  */  
	RSSL_MPF_HAS_PER_ENTRY_PERM_DATA	= 0x04,			/*!< (0x04) The RsslMap contains entries that have permission expressions, contained in relevant entries RsslMapEntry::permData */
	RSSL_MPF_HAS_TOTAL_COUNT_HINT		= 0x08,			/*!< (0x08) The RsslMap contains a total count hint, contained in RsslMap::totalCountHint.  */
	RSSL_MPF_HAS_KEY_FIELD_ID			= 0x10			/*!< (0x10) The RsslMap contains entries where the key (RsslMapEntry::encKey) also represents a field in the payload of the entry.  The key's FID is specified in RsslMap::keyFieldId. */
} RsslMapFlags;

/**
 * @brief The RsslMap is a uniform container type of associated key–container pair entries.  Each entry, known as
 * an RsslMapEntry, contains an entry key and a value.  The key is a base primitive type as indicated by RsslMap::keyPrimitiveType and the value is a container type as indicated 
 * by RsslMap::containerType.  
 * @see RSSL_INIT_MAP, rsslClearMap, RsslMapFlags, rsslDecodeMap, rsslEncodeMapInit, rsslEncodeMapComplete, rsslEncodeMapSetDefsComplete, rsslEncodeMapSummaryDataComplete
 */
typedef struct
{
	RsslUInt8           flags;  			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslMapFlags */

	RsslPrimitiveType	keyPrimitiveType;  	/*!< @brief The primitive type associated with each RsslMapEntry key. */
	RsslFieldId			keyFieldId;	  		/*!< @brief When present, this indicates that the RsslMapEntry key also represents a field in the payload of the entry.  Presence is indicated by RsslMapFlags::RSSL_MPF_HAS_KEY_FIELD_ID  The key's FID is indicated here.   */
	RsslContainerType	containerType;  	/*!< @brief The container type associated with the payload of each RsslMapEntry */

	RsslBuffer			encSetDefs;  		/*!< @brief Contains encoded set definition information when present. Presence is indicated by \ref RsslMapFlags ::RSSL_MPF_HAS_SET_DEFS.   When decoding, this contains encoded set definitions.  When encoding, pre-encoded set definition information can be specified here; if not pre-encoded the user can encode set definitions and invoke rsslEncodeMapSetDefsComplete() when ready to continue encoding RsslMap contents. */
	RsslBuffer			encSummaryData;  	/*!< @brief Contains encoded summary data information when present.  Presence is indicated by \ref RsslMapFlags ::RSSL_MPF_HAS_SUMMARY_DATA.  When decoding, this contains encoded set definitions.  When encoding, pre-encoded summary data information can be specified here; if not pre-encoded the user can encode using the appropriate container type encode functionality and invoke rsslEncodeMapSummaryDataComplete() when ready to continue encoding RsslMap contents. */

	RsslUInt32			totalCountHint; 	/*!< @brief Contains an approximate total RsslMapEntry count for the RsslMap.  This is typically used when splitting entries across a multi-part response (available with RsslRefreshMsg, RsslGenericMsg, RsslPostMsg).  Presence is indicated by \ref RsslMapFlags::RSSL_MPF_HAS_TOTAL_COUNT_HINT.  */
	
	RsslBuffer          encEntries;		 	/*!< @brief Raw encoded payload for the RsslMap */
} RsslMap;

/**
 * @brief RsslMap static initializer
 * @see RsslMap, rsslClearMap
 */
#define RSSL_INIT_MAP { 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslMap
 * @see RsslMap, RSSL_INIT_MAP
 */
RTR_C_INLINE void rsslClearMap(RsslMap *pMap)
{
	pMap->flags = 0;
	pMap->keyPrimitiveType = 0;
	pMap->containerType = 0;
	pMap->encSetDefs.length = 0;
	pMap->encSetDefs.data = 0;
	pMap->encSummaryData.length = 0;
	pMap->encSummaryData.data = 0;
	pMap->totalCountHint = 0;
	pMap->encEntries.length = 0;
	pMap->encEntries.data = 0;
}

/**
 * @}
 */


/**
 * @addtogroup MEStruct 
 *	@{
 */

/**
 * @brief Flag values for use with each RsslMapEntry contained in an RsslMap. (MPEF = MapEntry Flag)
 * @see RsslMapEntry, RsslMap
 */
typedef enum
{
	RSSL_MPEF_NONE	 		= 	0x00,		/*!< (0x00) No RsslMapEntry flags are present. */
	RSSL_MPEF_HAS_PERM_DATA	= 	0x01		/*!< (0x01) This RsslMapEntry contains permission data, located in RsslMapEntry::permData. */
} RsslMapEntryFlags;



/**
 * @brief Action that indicates how to apply contents of RsslMapEntry. (MPEA = MapEntry Action)
 * @see RsslMapEntry, RsslMap
 */
typedef enum
{
	RSSL_MPEA_UPDATE_ENTRY    = 1,     /*!< (1) Apply the contents of this entry as an update at the specified key.  If an RsslMapEntry with an ::RSSL_MPEA_UPDATE_ENTRY action is received prior to an ::RSSL_MPEA_ADD_ENTRY action for the same key, the entry with the ::RSSL_MPEA_UPDATE_ENTRY action can be ignored.  */
	RSSL_MPEA_ADD_ENTRY       = 2,     /*!< (2) Add the contents of this entry at the specified key.  If an RsslMapEntry with an ::RSSL_MPEA_ADD_ENTRY action is received after already adding the key from a previous ::RSSL_MPEA_ADD_ENTRY action, previously added content should be replaced or modified based on the RsslMap::containerType update logic.   */
	RSSL_MPEA_DELETE_ENTRY    = 3	   /*!< (3) Delete the contents of the entry at the specified key.  @note ::RSSL_MPEA_DELETE_ENTRY actions cannot carry any entry payload.  If payload is necessary for a ::RSSL_MPEA_DELETE_ENTRY action, an entry with an ::RSSL_MPEA_UPDATE_ENTRY action can be sent and immediatley followed by the same key on an entry with a ::RSSL_MPEA_DELETE_ENTRY action.   */
} RsslMapEntryActions;




/**
 * @brief One entry contained in an RsslMap.  The contents of the entry is specified by RsslMap::containerType and the key's type is specified by RsslMap::keyPrimitiveType.  Any contents should be applied following the rules of the specified RsslMapEntry::action ( \ref RsslMapEntryActions)
 * @see RSSL_INIT_MAP_ENTRY, rsslClearMapEntry, RsslMapEntryFlags, RsslMapEntryActions, rsslDecodeMapEntry, rsslEncodeMapEntryInit, rsslEncodeMapEntryComplete
 */
typedef struct
{
	RsslUInt8		flags;		/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslMapEntryFlags */
	RsslUInt8		action;		/*!< @brief Action to use when applying RsslMapEntry information.  The available actions are defined in RsslMapEntryActions enumeration. */
	RsslBuffer		permData;	/*!< @brief Contains permission data associated with this RsslMapEntry and its contents.  Presence is indicated by \ref RsslMapEntryFlags::RSSL_MPEF_HAS_PERM_DATA.   */
	RsslBuffer		encKey;		/*!< @brief Encoded key content.  Can be populated with pre-encoded key content when encoding and contains encoded key content when decoding. */
	RsslBuffer		encData;	/*!< @brief Raw encoded content for this RsslMapEntry */
} RsslMapEntry;

/**
 * @brief RsslMapEntry static initializer
 * @see RsslMapEntry, rsslClearMapEntry
 */
#define RSSL_INIT_MAP_ENTRY { 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslMapEntry 
 * @see RsslMapEntry, RSSL_INIT_MAP_ENTRY
 */
RTR_C_INLINE void rsslClearMapEntry( RsslMapEntry *pMapEntry )
{
	memset(pMapEntry, 0, sizeof(RsslMapEntry));
}

/**
 * @}
 */


/** 
 * @addtogroup MapEncoding RsslMap and RsslMapEntry Encoding
 * @{
 */

/** 
 * @brief 	Begin encoding process for RsslMap container type.
 *
 * Begins encoding of an RsslMap<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit()<BR>
 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pMap	partially populated RsslMap structure to encode
 * @param summaryMaxSize max expected encoding size of the summary data, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length summary data.
 * @param setMaxSize max encoding size of the set information, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length set definition data.
 * @see RsslMap, RsslEncodeIterator, rsslEncodeMapComplete, rsslEncodeMapSetDefsComplete, rsslEncodeMapSummaryDataComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeMapInit(
								RsslEncodeIterator	*pIter,
								RsslMap				*pMap,
								RsslUInt16			summaryMaxSize,
								RsslUInt16			setMaxSize );

/**
 * @brief Complete set data encoding for an RsslMap. If both rsslEncodeMapSetDefsComplete()
 * and rsslEncodeMapSummaryDataComplete() are called, rsslEncodeMapSetDefsComplete()
 * must be called first.  If set definitions are pre-encoded, they should be set on RsslMap::encSetDefs and this function is not required.
 * and this function is not required.  
 * 
 * Completes set data encoding within an RsslMap. <BR> 
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit() with ::RSSL_MPF_HAS_SET_DATA flag set on RsslMap::flags<BR>
 *  2. Encode set definition content using appropriate set definition encode functions<BR>
 *  3. Call rsslEncodeMapSetDefsComplete() <BR>
 *  4. If present, encode any non pre-encoded summary data and call rsslEncodeMapSummaryDataComplete()<BR>
 *  5. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  6. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator
 * @param success If true - successfully complete the set definition encoding within the RsslMap, if false - remove the set definitions from the encoded RsslMap.  
 * @see rsslEncodeMapInit, RsslEncodeIterator, RsslMap
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapSetDefsComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );

/**
 * @brief Complete summary data encoding for an RsslMap. If both rsslEncodeMapSetDefsComplete()
 * and rsslEncodeMapSummaryDataComplete() are called, rsslEncodeMapSetDefsComplete()
 * must be called first. If summary data is pre-encoded, it should be set on RsslMap::encSummaryData and this  function is not required.  
 * 
 * Completes summary data encoding within an RsslMap<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit() with ::RSSL_MPF_HAS_SUMMARY_DATA flag set on RsslMap::flags<BR>
 *  2. If present, encode any non pre-encoded set definition data and call rsslEncodeMapSetDefsComplete()<BR>
 *  3. Encode summary data content using appropriate container encoders.  Summary data usually matches type specified in RsslMap::containerType.<BR>
 *  4. Call rsslEncodeMapSummaryDataComplete() <BR>
 *  5. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  6. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator. 
 * @param success If true - successfully complete the summary data encoding within the RsslMap, if false - remove the summary data from the encoded RsslMap.  
 * @see rsslEncodeMapInit, RsslEncodeIterator, RsslMap
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapSummaryDataComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );

/** 
 * @brief 	Completes encoding of an RsslMap
 *
 * Completes RsslMap encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit()<BR>
 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the map encoding, if false - remove the map payload from the buffer.
 * @see rsslEncodeMapInit, RsslMap, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );

/** 
 * @brief 	Encodes an RsslMapEntry where there is no payload or any payload is pre-encoded and populated on RsslMapEntry::encData.  
 *
 * Encodes an RsslMapEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit()<BR>
 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pMapEntry populated RsslMapEntry to encode.  If any payload is present, it must be populated on RsslMapEntry::encData.  
 * @param pKeyData Pointer to the primitive representation of the entry's key, should be of type RsslMap::keyPrimitiveType.  If passed in as NULL the RsslMapEntry::encKey should be populated with a pre-encoded primitive key of type RsslMap::keyPrimitiveType.
 * @see RsslEncodeIterator, RsslMapEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapEntry(
								RsslEncodeIterator  *pIter,
								RsslMapEntry		*pMapEntry,
								const void			*pKeyData );

/** 
 * @brief 	Begins encoding of an RsslMapEntry, where any payload is encoded after this call using the appropriate container type encode functions, as specified by RsslMap::containerType.  
 *
 * Begins encoding of an RsslMapEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit()<BR>
 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pMapEntry populated RsslMapEntry to encode
 * @param pKeyData Pointer to the primitive representation of the entry's key, should be of type RsslMap::keyPrimitiveType.  If passed in as NULL the RsslMapEntry::encKey should be populated with a pre-encoded primitive key of type RsslMap::keyPrimitiveType.
 * @param maxEncodingSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslMapEntry, rsslEncodeMapEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapEntryInit(
								RsslEncodeIterator	*pIter,
								RsslMapEntry		*pMapEntry,
								const void			*pKeyData, 
								RsslUInt16			maxEncodingSize );

/** 
 * @brief 	Completes encoding of an RsslMapEntry.  
 *
 * Completes encoding of an RsslMapEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeMapInit()<BR>
 *  2. If RsslMap contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeMapSetDefsComplete()<BR>
 *  3. If RsslMap contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeMapSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeMapEntry() or rsslEncodeMapEntryInit()..rsslEncodeMapEntryComplete() for each RsslMapEntry<BR>
 *  5. Call rsslEncodeMapComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslMapEntry encoding, if false - remove the RsslMapEntry from the encoded RsslMap.
 * @see RsslEncodeIterator, RsslMapEntry, rsslEncodeMapEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMapEntryComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );


/**
 * @}
 */
 
/** 
 * @addtogroup MapDecoding RsslMap and RsslMapEntry Decoding
 * @{
 */


/**
 * @brief Decodes an RsslMap container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeMap()<BR>
 *  2. Call rsslDecodeMapEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pMap RsslMap structure to populate with decoded contents.  RsslMap::encEntries will point to encoded RsslMapEntry content.
 * @see rsslDecodeMapEntry, RsslMap, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeMap(
							RsslDecodeIterator	*pIter,
							RsslMap				*pMap );
				

/**
 * @brief Decodes and returns a single RsslMapEntry from within an RsslMap::encEntries
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeMap()<BR>
 *  2. Call rsslDecodeMapEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pMapEntry RsslMapEntry to decode content into.  RsslMapEntry::encKey will contain encoded entry key, RsslMapEntry::encData will contain encoded content of entry.  
 * @param pKeyData Can be used to optionally decode RsslMapEntry::encKey into its primitive type representation, as indicated by RsslMap::keyPrimitiveType.  If the user provides this pointer, this function will automatically decode the RsslMapEntry::encKey (the pointer MUST point to the appropriate type and size).  If passed in as NULL key will not be decoded, however appropriate primitive type decoder (as indicated by RsslMap::keyPrimitiveType) can be called on RsslMapEntry::encKey.
 * @see rsslDecodeMap, RsslMapEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeMapEntry(
							RsslDecodeIterator	*pIter,
							RsslMapEntry		*pMapEntry,
							void				*pKeyData );

/**
 * @}
 */

/**
 * @addtogroup MStruct
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_MPF_HAS_SET_DEFS flag on the given RsslMap.
 *
 * @param pMap Pointer to the RsslMap to check.
 * @see RsslMap, RsslMapFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapCheckHasSetDefs(RsslMap *pMap)
{
	return ((pMap->flags & RSSL_MPF_HAS_SET_DEFS) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MPF_HAS_SUMMARY_DATA flag on the given RsslMap.
 *
 * @param pMap Pointer to the RsslMap to check.
 * @see RsslMap, RsslMapFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapCheckHasSummaryData(RsslMap *pMap)
{
	return ((pMap->flags & RSSL_MPF_HAS_SUMMARY_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MPF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslMap.
 *
 * @param pMap Pointer to the RsslMap to check.
 * @see RsslMap, RsslMapFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapCheckHasPerEntryPermData(RsslMap *pMap)
{
	return ((pMap->flags & RSSL_MPF_HAS_PER_ENTRY_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MPF_HAS_TOTAL_COUNT_HINT flag on the given RsslMap.
 *
 * @param pMap Pointer to the RsslMap to check.
 * @see RsslMap, RsslMapFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapCheckHasTotalCountHint(RsslMap *pMap)
{
	return ((pMap->flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_MPF_HAS_KEY_FIELD_ID flag on the given RsslMap.
 *
 * @param pMap Pointer to the RsslMap to check.
 * @see RsslMap, RsslMapFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapCheckHasKeyFieldId(RsslMap *pMap)
{
	return ((pMap->flags & RSSL_MPF_HAS_KEY_FIELD_ID) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_MPF_HAS_SET_DEFS flag on the given RsslMap.
 * 
 * @param pMap Pointer to the RsslMap to apply flag value to.
 * @see RsslMap, RsslMapFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapApplyHasSetDefs(RsslMap *pMap)
{
	pMap->flags |= RSSL_MPF_HAS_SET_DEFS;
}

/**
 * @brief Applies the ::RSSL_MPF_HAS_SUMMARY_DATA flag on the given RsslMap.
 * 
 * @param pMap Pointer to the RsslMap to apply flag value to.
 * @see RsslMap, RsslMapFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapApplyHasSummaryData(RsslMap *pMap)
{
	pMap->flags |= RSSL_MPF_HAS_SUMMARY_DATA;
}

/**
 * @brief Applies the ::RSSL_MPF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslMap.
 * 
 * @param pMap Pointer to the RsslMap to apply flag value to.
 * @see RsslMap, RsslMapFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapApplyHasPerEntryPermData(RsslMap *pMap)
{
	pMap->flags |= RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_MPF_HAS_TOTAL_COUNT_HINT flag on the given RsslMap.
 * 
 * @param pMap Pointer to the RsslMap to apply flag value to.
 * @see RsslMap, RsslMapFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapApplyHasTotalCountHint(RsslMap *pMap)
{
	pMap->flags |= RSSL_MPF_HAS_TOTAL_COUNT_HINT;
}


/**
 * @brief Applies the ::RSSL_MPF_HAS_KEY_FIELD_ID flag on the given RsslMap.
 * 
 * @param pMap Pointer to the RsslMap to apply flag value to.
 * @see RsslMap, RsslMapFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapApplyHasKeyFieldId(RsslMap *pMap)
{
	pMap->flags |= RSSL_MPF_HAS_KEY_FIELD_ID;
}

/**
 * @}
 */

/**
 * @addtogroup MEStruct 
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_MPEF_HAS_PERM_DATA flag on the given RsslMapEntry.
 *
 * @param pMapEntry Pointer to the RsslMapEntry to check.
 * @see RsslMapEntry, RsslMapEntryFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslMapEntryCheckHasPermData(RsslMapEntry *pMapEntry)
{
		return((pMapEntry->flags & RSSL_MPEF_HAS_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Applies the ::RSSL_MPEF_HAS_PERM_DATA flag on the given RsslMapEntry.
 * 
 * @param pMapEntry Pointer to the RsslMapEntry to apply flag value to.
 * @see RsslMapEntry, RsslMapEntryFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslMapEntryApplyHasPermData(RsslMapEntry *pMapEntry)
{
	pMapEntry->flags |= RSSL_MPEF_HAS_PERM_DATA;
}

/**
 *	@}
 */
 

#ifdef __cplusplus
}
#endif

#endif
