/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_ELEMENT_LIST_H
#define __RSSL_ELEMENT_LIST_H



#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"



/** 
 * @addtogroup EStruct
 * @{
 */


/**
 * @brief Flag values for use with the RsslElementList. (ELF = ElementList Flag)
 * @see RsslElementList
 */
typedef enum {
	RSSL_ELF_NONE					= 0x00,		/*!< (0x00) No RsslElementList flags are present */
	RSSL_ELF_HAS_ELEMENT_LIST_INFO	= 0x01,		/*!< (0x01) The RsslElementList has an RsslElementList::elementListNum.  */
	RSSL_ELF_HAS_SET_DATA			= 0x02,		/*!< (0x02) The RsslElementList payload contains set defined content.  To decode, a set definition database is required - otherwise set data will be skipped while decoding. */
	RSSL_ELF_HAS_SET_ID				= 0x04,		/*!< (0x04) The RsslElementList contains has an RsslElementList::setId.  This indicates which set definition is required to decode any set defined content.  If not present but set data is included, this is assumed to be <b>0</b>. */
	RSSL_ELF_HAS_STANDARD_DATA		= 0x08		/*!< (0x08) The RsslElementList contains standard encoded content (e.g. not set defined).  */
} RsslElementListFlags;

/** 
 * @brief General OMM strings associated with the different element list flags.
 * @see RsslElementListFlags, rsslElementListFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_ELF_HAS_ELEMENT_LIST_INFO = { 18, (char*)"HasElementListInfo" };
static const RsslBuffer RSSL_OMMSTR_ELF_HAS_SET_DATA = { 10, (char*)"HasSetData" };
static const RsslBuffer RSSL_OMMSTR_ELF_HAS_SET_ID = { 8, (char*)"HasSetID" };
static const RsslBuffer RSSL_OMMSTR_ELF_HAS_STANDARD_DATA = { 15, (char*)"HasStandardData" };

/**
 * @brief Provide general OMM string representation of RsslElementListFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslElementListFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslElementListFlags
 */
RSSL_API RsslRet rsslElementListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);

/**
 * @brief The RsslElementList container type allows the user to represent a collection of name - type - value triples, where the value can be a container type  or a primitive type.  The name and type information is similar to the content that would be cross referenced with a field dictionary when using an RsslFieldList. No dictionary is required to decode RsslElementEntry contents.
 * @see RSSL_INIT_ELEMENT_LIST, rsslClearElementList, RsslElementListFlags, rsslEncodeElementListInit, rsslEncodeElementListComplete, rsslDecodeElementList
 */
typedef struct {
	RsslUInt8			flags;	 				/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslElementListFlags */

	RsslInt16			elementListNum;			/*!< @brief When present, contains the identifier for an element list template, also known as a record template.  Presence is indicated by \ref RsslElementListFlags ::RSSL_ELF_HAS_ELEMENT_LIST_INFO.  This information is typically used by caching components to determine the space needed to cache all possible content. */

	RsslUInt16			setId;					/*!< @brief When present, contains the set data identifier, referring to the set definition needed to decode any set defined content.  Presence is indicated by \ref RsslElementListFlags ::RSSL_ELF_HAS_SET_ID.  When not present, value should be assumed to be <b>0</b>. */
	RsslBuffer			encSetData;				/*!< @brief Raw encoded set defined data content.  When encoding, this can be used to provide pre-encoded set data.  When decoding, this will contain all encoded set data.  Set data presence is indicated by \ref RsslElementListFlags ::RSSL_ELF_HAS_SET_DATA   */
	
	RsslBuffer			encEntries;				/*!< @brief Raw encoded payload for the RsslElementList.  Standard encoded content presence is indicated by \ref RsslElementListFlags ::RSSL_ELF_HAS_STANDARD_DATA */
} RsslElementList;

/**
 * @brief RsslElementList static initializer
 * @see RsslElementList, rsslClearElementList
 */
#define RSSL_INIT_ELEMENT_LIST { 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslElementList
 * @see RsslElement, RSSL_INIT_ELEMENT_LIST
 */
RTR_C_INLINE void rsslClearElementList(RsslElementList *pElementList)
{
	pElementList->flags = 0;
	pElementList->elementListNum = 0;
	pElementList->setId = 0;
	pElementList->encSetData.length = 0;
	pElementList->encSetData.data = 0;
	pElementList->encEntries.length = 0;
	pElementList->encEntries.data = 0;
}

/**
 * @}
 */

/**
 * @addtogroup EEStruct
 * @{
 */


/**
 * @brief One entry contained in an RsslElementList.  The RsslElementEntry::dataType indicates the content type.
 * @see RSSL_INIT_ELEMENT_ENTRY, rsslClearElementEntry, rsslDecodeElementEntry, rsslEncodeElementEntry, rsslEncodeElementEntryInit, rsslEncodeElementEntryComplete
 */
typedef struct {
	RsslBuffer			name;			/*!< @brief An \ref RsslBuffer that specifies the name of this RsslElementEntry. */
	RsslDataType		dataType;		/*!< @brief The content type housed in this entry. This can house a primitive or a container type.  */
	RsslBuffer			encData;		/*!< @brief Raw encoded content for this RsslElementEntry.  */
} RsslElementEntry;

/**
 * @brief RsslElementEntry static initializer
 * @see RsslElementEntry, rsslClearElementEntry
 */
#define RSSL_INIT_ELEMENT_ENTRY { RSSL_INIT_BUFFER, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslElementEntry 
 * @see RsslElementEntry, RSSL_INIT_ELEMENT_ENTRY
 */
RTR_C_INLINE void rsslClearElementEntry(RsslElementEntry *pElement)
{
	pElement->name.data = 0;
	pElement->name.length = 0;
	pElement->dataType = 0;
	pElement->encData.data = 0;
	pElement->encData.length = 0;
}

/** 
 *	@}
 */
 

/** 
 * @addtogroup ElementListEncoding RsslElementList and RsslElementEntry Encoding
 * @{
 */

/** 
 * @brief 	Begin encoding process for RsslElementList container type.
 *
 * Begins encoding of an RsslElementList<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeElementListInit()<BR>
 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
 *  3. Call rsslEncodeElementListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pElementList partially populated RsslElementList structure to encode
 * @param pSetDb Set Definition database to use while encoding any set defined content.  This database must include a definition for RsslElementList::setId. 
 * @param setEncodingMaxSize max encoding size of the set defined data, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length set defined data.
 * @see RsslElementList, RsslEncodeIterator, rsslEncodeElementListComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeElementListInit( 
							RsslEncodeIterator				*pIter,
							RsslElementList					*pElementList,
							const RsslLocalElementSetDefDb	*pSetDb, 
							RsslUInt16						setEncodingMaxSize );

/** 
 * @brief 	Completes encoding of an RsslElementList
 *
 * Completes RsslElementList encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeElementListInit()<BR>
 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
 *  3. Call rsslEncodeElementListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the vector encoding, if false - remove the vector payload from the buffer.
 * @see rsslEncodeElementListInit, RsslElementList, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeElementListComplete( 
							RsslEncodeIterator  *pIter,
							RsslBool			success );



/** 
 * @brief 	Encodes an RsslElementEntry where payload is an unencoded primitive type (passed in via void*), blank (where void* is NULL and RsslElementEntry::encData is empty) , or payload is pre-encoded and populated on RsslElementEntry::encData.  
 *
 * Encodes an RsslElementEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeElementEntryInit()<BR>
 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
 *  3. Call rsslEncodeElementEntryComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pElementEntry populated RsslElementEntry to encode.  If any pre-encoded payload is present, it must be populated on RsslElementEntry::encData and should match the type specified in RsslElementEntry::dataType.  
 * @param pData Pointer to the primitive representation of the entry's payload, should be of type RsslElementEntry::dataType.  If encoding blank data or content is pre-encoded, this should be passed in as NULL.  
 * @see RsslEncodeIterator, RsslElementEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeElementEntry( 
							RsslEncodeIterator	*pIter,
							RsslElementEntry	*pElement,
							const void			*pData );

/** 
 * @brief 	Begins encoding of an RsslElementEntry, where any payload is encoded after this call using the appropriate data type encode functions.
 *
 * Begins encoding of an RsslElementEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeElementListInit()<BR>
 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
 *  3. Call rsslEncodeElementListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pElementEntry populated RsslElementEntry to encode
 * @param encodingMaxSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslElementEntry, rsslEncodeElementEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeElementEntryInit( 
							RsslEncodeIterator	*pIter,
							RsslElementEntry	*pElement,
							RsslUInt16			encodingMaxSize );


/** 
 * @brief 	Completes encoding of an RsslElementEntry.  
 *
 * Completes encoding of an RsslElementEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeElementListInit()<BR>
 *  2. To encode entries, call rsslEncodeElementEntry() or rsslEncodeElementEntryInit()..rsslEncodeElementEntryComplete() for each RsslElementEntry<BR>
 *  3. Call rsslEncodeElementListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslElementEntry encoding, if false - remove the RsslElementEntry from the encoded RsslElementList.
 * @see RsslEncodeIterator, RsslElementEntry, rsslEncodeElementEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeElementEntryComplete( 
							RsslEncodeIterator	*pIter,
							RsslBool			success );

/** 
 *	@}
 */


/** 
 * @addtogroup ElementListDecoding RsslElementList and RsslElementEntry Decoding
 * @{
 */



/**
 * @brief Decodes an RsslElementList container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeElementList()<BR>
 *  2. Call rsslDecodeElementEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pElementList RsslElementList structure to populate with decoded contents.  RsslElementList::encSetData and RsslElementList::encEntries will point to encoded RsslElementEntry content.
 * @param pLocalSetDb Set definition database to use while decoding any set defined content.
 * @see rsslDecodeElementEntry, RsslElementList, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeElementList(
							RsslDecodeIterator 				*pIter,
							RsslElementList					*pElementList,
							const RsslLocalElementSetDefDb	*pLocalSetDb );


/**
 * @brief Decodes and returns a single RsslElementEntry from within an RsslElementList::encSetData and/or RsslElementList::encEntries
 *
 * @note If RsslElementList::encSetData is present, but no set definition database was passed in to rsslDecodeElementList(), set data content will be skipped while decoding entries.
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeElementList()<BR>
 *  2. Call rsslDecodeElementEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pElementEntry RsslElementEntry to decode content into. RsslElementEntry::encData will contain encoded content of entry.  To continue decoding into specific data type, RsslElementEntry::dataType decoder can be used. 
 * @see rsslDecodeElementList, RsslElementEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeElementEntry(
							RsslDecodeIterator		*pIter,
							RsslElementEntry 		*pElement );
				 

/**
 *	@}
 */


/** 
 * @addtogroup ElementListSetInfo
 * @{
 */

/** @brief Encode ElementlistList set definitions database
 * @param pIter encode iterator
 * @param pElementListSetDb database to encode
 */
RSSL_API RsslRet rsslEncodeLocalElementSetDefDb(
				RsslEncodeIterator			*pIter,
				RsslLocalElementSetDefDb	*pElementListSetDb);




/**
 * @brief Decode set definitions contained on RsslMap, RsslVector, or RsslSeries into setDefDB
 * database.
 * @see rsslDecodeMap
 * @see rsslDecodeSeries
 * @see rsslDecodeVector
 * @see rsslDecodeElement
 *
 * Typical use:<BR>
 *  1. Call rsslDecodeMap(), rsslDecodeSeries() or rsslDecodeVector()
 *  2. Call rsslDecodeLocalElementSetDefDb()
 *
 * If decoding local set definitions from a single thread, RSSL has internal memory that will be <BR>
 * decoded into.  To use this, entries.data and entries.length should be 0.  <BR>
 * If decoding local set definitions from across multiple threads, the user should supply memory <BR>
 * by populating entries.data.  entries.length should be set to the number of bytes available in data. <BR>
 * Alternatively, a separate iterator may be set to the container's encSetDefs and passed into rsslDecodeLocalElementSetDefDb() (the appropriate version information should also be set).
 * @param pIter RsslDecodeIterator Pointer to the decoding iterator.
 * @param pLocalSetDb Pointer to the Element List Set Definitions Database structure to populate.
 */
RSSL_API RsslRet rsslDecodeLocalElementSetDefDb(
				RsslDecodeIterator			*pIter,
				RsslLocalElementSetDefDb	*pLocalSetDb );


				
/** 
 *	@brief Encode a Global element set definition dictionary payload.
 *
 *  This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary, 
 *  subsequent calls can be made to this function, each producing the next segment of fields.<BR>
 *
 *	Typical use:<BR>
 *	 1.  Initialize the RsslElementSetDefDb using rsslClearElementSetDb, then allocate the definitions pointer array and version information with rsslAllocateElementSetDefDb.<BR>
 *	 2.  Add the set definitions to the RsslElementSetDefDb using rsslAddElementSetDefToDb.<BR>
 *	 3.  Start encoding the message that contains this dictionary payload.<BR>
 *	 4.  Call rsslEncodeElementSetDefDictionary.<BR>
 *	 5.  If rsslEncodeElementSetDefDictionary returns RSSL_RET_DICT_PART_ENCODED, go back to step 3, and encode the next part of the payload.  Otherwise, if RSSL_RET_SUCCESS is returned, continue.<BR>
 *	 6.  When finished with the global set definition dictionary, call rsslElementFieldSetDefDb to free all allocated memory.<BR>
 * @param eIter Iterator to be used for encoding. Prior to each call, the iterator must be cleared and initialized to the buffer to be used for encoding.
 * @param dictionary The dictionary to encode global element set definition information from.
 * @param currentSetDef Tracks which set definitions have been encoded in case of multi-part encoding. Must be initialized to 0 on the first call and is updated with each successfully encoded part.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @return RSSL_RET_DICT_PART_ENCODED when encoding parts, RSSL_RET_SUCCESS for final part or single complete payload.
 * @see RsslElementSetDefDb, RDMDictionaryVerbosityValues, rsslDecodeElementSetDefDictionary
 *	
 */			
RSSL_API RsslRet rsslEncodeElementSetDefDictionary(
	RsslEncodeIterator *eIter, 
	RsslElementSetDefDb *dictionary, 
	int *currentSetDef, 
	RDMDictionaryVerbosityValues verbosity, 
	RsslBuffer *errorText);

/** 
 *	@brief Decode a Global element set definition dictionary payload.
 *
 * This function may be called multiple times on the same dictionary, to load information from dictionaries that have been encoded in multiple parts.<BR>
 *
 *	Typical use:<BR>
 *	 1.  Initialize the RsslFieldSetDefDb using rsslClearFieldSetDb, then allocate the definitions pointer array and version information with rsslAllocateElementSetDefDb.<BR>
 *	 2.  When a response message containing global field set definition data is received, call rsslDecodeElementSetDefDictionary.<BR>
 *	 3.  Continue with decoding messages.<BR>
 *	 5.  When finished with the global set definition dictionary, call rsslDeleteElementSetDefDb to free all allocated memory.<BR>
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded field information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslElementSetDefDb, RDMDictionaryVerbosityValues, rsslEncodeElementSetDefDictionary
 *	
 */	
RSSL_API RsslRet rsslDecodeElementSetDefDictionary(
	RsslDecodeIterator *dIter,	
	RsslElementSetDefDb *dictionary, 
	RDMDictionaryVerbosityValues verbosity, 
	RsslBuffer *errorText);

/**
 * @}
 */

 
/**
 *	@addtogroup EStruct
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_ELF_HAS_ELEMENT_LIST_INFO flag on the given RsslElementList.
 *
 * @param pElementList Pointer to the RsslElementList to check.
 * @see RsslElementList, RsslElementListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslElementListCheckHasInfo(RsslElementList *pElementList)
{
	return ((pElementList->flags & RSSL_ELF_HAS_ELEMENT_LIST_INFO) ? 
						RSSL_TRUE : RSSL_FALSE );
}
#define rsslElementListHasInfo rsslElementListCheckHasInfo


/**
 * @brief Checks the presence of the ::RSSL_ELF_HAS_STANDARD_DATA flag on the given RsslElementList.
 *
 * @param pElementList Pointer to the RsslElementList to check.
 * @see RsslElementList, RsslElementListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslElementListCheckHasStandardData(RsslElementList *pElementList)
{
	return ((pElementList->flags & RSSL_ELF_HAS_STANDARD_DATA) ?
						RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Checks the presence of the ::RSSL_ELF_HAS_SET_ID flag on the given RsslElementList.
 *
 * @param pElementList Pointer to the RsslElementList to check.
 * @see RsslElementList, RsslElementListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslElementListCheckHasSetId(RsslElementList *pElementList)
{
	return ((pElementList->flags & RSSL_ELF_HAS_SET_ID) ?
						RSSL_TRUE : RSSL_FALSE );
}		


/**
 * @brief Checks the presence of the ::RSSL_ELF_HAS_SET_DATA flag on the given RsslElementList.
 *
 * @param pElementList Pointer to the RsslElementList to check.
 * @see RsslElementList, RsslElementListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslElementListCheckHasSetData(RsslElementList *pElementList)
{
	return ((pElementList->flags & RSSL_ELF_HAS_SET_DATA) ?
					RSSL_TRUE : RSSL_FALSE );
}



/**
 * @brief Applies the ::RSSL_FLF_HAS_ELEMENT_LIST_INFO flag on the given RsslElementList.
 * 
 * @param pElementList Pointer to the RsslElementList to apply flag value to.
 * @see RsslElementList, RsslElementListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslElementListApplyHasInfo(RsslElementList *pElementList)
{
	pElementList->flags |= RSSL_ELF_HAS_ELEMENT_LIST_INFO;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_STANDARD_DATA flag on the given RsslElementList.
 * 
 * @param pElementList Pointer to the RsslElementList to apply flag value to.
 * @see RsslElementList, RsslElementListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslElementListApplyHasStandardData(RsslElementList *pElementList)
{
	pElementList->flags |= RSSL_ELF_HAS_STANDARD_DATA;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_SET_ID flag on the given RsslElementList.
 * 
 * @param pElementList Pointer to the RsslElementList to apply flag value to.
 * @see RsslElementList, RsslElementListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslElementListApplyHasSetId(RsslElementList *pElementList)
{
	pElementList->flags |= RSSL_ELF_HAS_SET_ID;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_SET_DATA flag on the given RsslElementList.
 * 
 * @param pElementList Pointer to the RsslElementList to apply flag value to.
 * @see RsslElementList, RsslElementListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslElementListApplyHasSetData(RsslElementList *pElementList)
{
	pElementList->flags |= RSSL_ELF_HAS_SET_DATA;
}

 
/** 
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
