/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_FIELD_LIST_H
#define __RSSL_FIELD_LIST_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslIterators.h"


/**
 * @addtogroup FStruct
 *	@{
 */

/** 
 * @brief Flag values for use with the RsslFieldList (FLF = FieldList Flags)
 * @see RsslFieldList
 */
typedef enum {
	RSSL_FLF_NONE					= 0x00,			/*!< (0x00) No RsslFieldList flags are present */
	RSSL_FLF_HAS_FIELD_LIST_INFO	= 0x01,			/*!< (0x01) The RsslFieldList has an RsslFieldList::dictionaryId and RsslFieldList::fieldListNum.  If not present, RsslFieldList::dictionaryId is assumed to be <b>1</b>. */
	RSSL_FLF_HAS_SET_DATA			= 0x02,			/*!< (0x02) The RsslFieldList payload contains set defined content.  To decode, a set definition database is required - otherwise set data will be skipped while decoding. */
	RSSL_FLF_HAS_SET_ID				= 0x04,			/*!< (0x04) The RsslFieldList contains has an RsslFieldList::setId.  This indicates which set definition is required to decode any set defined content.  If not present but set data is included, this is assumed to be <b>0</b>. */
	RSSL_FLF_HAS_STANDARD_DATA		= 0x08 			/*!< (0x08) The RsslFieldList contains standard encoded content (e.g. not set defined).  */
} RsslFieldListFlags;

/** 
 * @brief General OMM strings associated with the different field list flags.
 * @see RsslFieldListFlags, rsslFieldListFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_FLF_HAS_FIELD_LIST_INFO = { 16, (char*)"HasFieldListInfo" };
static const RsslBuffer RSSL_OMMSTR_FLF_HAS_SET_DATA = { 10, (char*)"HasSetData" };
static const RsslBuffer RSSL_OMMSTR_FLF_HAS_SET_ID = { 8, (char*)"HasSetID" };
static const RsslBuffer RSSL_OMMSTR_FLF_HAS_STANDARD_DATA = { 15, (char*)"HasStandardData" };

/**
 * @brief Provide general OMM string representation of RsslFieldListFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslFieldListFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslFieldListFlags
 */
RSSL_API RsslRet rsslFieldListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);

/**
 * @brief The RsslFieldList container type allows the user to represent a collection of field identifier - value pairs, where the value can be a container type or a primitive type.  The field identifier (RsslFieldEntry::fieldId) can be cross referenced with a field dictionary to determine the specific type of the content.  
 * The dictionary contains specific name and type information (e.g. RDMFieldDictionary).
 * @see RSSL_INIT_FIELD_LIST, rsslClearFieldList, RsslFieldListFlags, rsslEncodeFieldListInit, rsslEncodeFieldListComplete, rsslDecodeFieldList
 */
typedef struct {
	RsslUInt8			flags;			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslFieldListFlags */

	RsslInt16			dictionaryId;	/*!< @brief When present, contains the identifier associated with the dictionary necessary to decode RsslFieldEntry content.  Presence is indicated by \ref RsslFieldListFlags ::RSSL_FLF_HAS_FIELD_LIST_INFO.  When not present, value should be assumed to be <b>1</b>.  */
	RsslInt16			fieldListNum;	/*!< @brief When present, contains the identifier for a field list template, also known as a record template.  Presence is indicated by \ref RsslFieldListFlags ::RSSL_FLF_HAS_FIELD_LIST_INFO.  This information is typically used by caching components to determine the space needed to cache all possible content. */
 
	RsslUInt16			setId;			/*!< @brief When present, contains the set data identifier, referring to the set definition needed to decode any set defined content.  Presence is indicated by \ref RsslFieldListFlags ::RSSL_FLF_HAS_SET_ID.  When not present, value should be assumed to be <b>0</b>. */
	RsslBuffer			encSetData;		/*!< @brief Raw encoded set defined data content.  When encoding, this can be used to provide pre-encoded set data.  When decoding, this will contain all encoded set data.  Set data presence is indicated by \ref RsslFieldListFlags ::RSSL_FLF_HAS_SET_DATA   */

	RsslBuffer			encEntries;		/*!< @brief Raw encoded payload for the RsslFieldList.  Standard encoded content presence is indicated by \ref RsslFieldListFlags ::RSSL_FLF_HAS_STANDARD_DATA */
} RsslFieldList;

/**
 * @brief RsslFieldList static initializer
 * @see RsslFieldList, rsslClearFieldList
 */
#define RSSL_INIT_FIELD_LIST { 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslFieldList
 * @see RsslFieldList, RSSL_INIT_FIELD_LIST
 */
RTR_C_INLINE void rsslClearFieldList(RsslFieldList *pFieldList)
{
	pFieldList->flags = 0;
	pFieldList->dictionaryId = 0;
	pFieldList->fieldListNum = 0;
	pFieldList->setId = 0;
	pFieldList->encSetData.length = 0;
	pFieldList->encSetData.data = 0;
	pFieldList->encEntries.length = 0;
	pFieldList->encEntries.data = 0;
}


/**
 * @}
 */

/**
 * @addtogroup FEStruct
 * @{
 */




/**
 * @brief One entry contained in an RsslFieldList.  When encoding, RsslFieldEntry::dataType should be specified.  
 *  When decoding set data, RsslFieldEntry::dataType will be populated with information from set definition.  When decoding and RsslFieldEntry::dataType is ::RSSL_DT_UNKNOWN, user should cross reference RsslFieldEntry::fieldId with the appropriate field dictionary to determine type and name information.
 * 
 * @note Typically, the field dictionary information is specified in RsslFieldList::dictionaryId or is the default value of <b>1</b>.  
 * Sending or receiving an RsslFieldEntry::fieldId of <b>0</b> followed by an \ref RsslInt containing a different dictionary id allows for a temporary change to the dictionary id.  
 * This change is valid until either the end of this RsslFieldList content or until another RsslFieldEntry::fieldId of <b>0</b> is received.  
 * 
 * @see RSSL_INIT_FIELD_ENTRY, rsslClearFieldEntry, rsslDecodeFieldEntry, rsslEncodeFieldEntry, rsslEncodeFieldEntryInit, rsslEncodeFieldEntryComplete
 */
typedef struct {
	RsslFieldId			fieldId;	/*!< @brief The field identifier that refers to specific name and type information defined by an external field dictionary, such as the RDMFieldDictionary */
	RsslUInt8			dataType;	/*!< @brief The content type housed in this entry, which can be a primitive or a container type.  When encoding, this is required to be specified.  When decoding, value of ::RSSL_DT_UNKNOWN indicates that application should determine type by cross referencing RsslFieldEntry::fieldId with appropriate field dictionary.  When decoding set defined data, the type from the set definition will be specified here. */
	RsslBuffer			encData;	/*!< @brief Raw encoded content for this RsslFieldEntry. */
} RsslFieldEntry;

/**
 * @brief RsslFieldEntry static initializer
 * @see RsslFieldEntry, rsslClearFieldEntry
 */
#define RSSL_INIT_FIELD_ENTRY { 0, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslFieldEntry
 * @see RsslFieldEntry, RSSL_INIT_FIELD_ENTRY
 */
RTR_C_INLINE void rsslClearFieldEntry(RsslFieldEntry *pField)
{
	pField->fieldId = 0;
	pField->encData.data = 0;
	pField->encData.length = 0;
}

/**
 *	@}
 */


/** 
 * @addtogroup FieldListEncoding RsslFieldList and RsslFieldEntry Encoding
 * @{
 */

/** 
 * @brief 	Begin encoding process for RsslFieldList container type.
 *
 * Begins encoding of an RsslFieldList<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFieldListInit()<BR>
 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
 *  3. Call rsslEncodeFieldListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFieldList partially populated RsslFieldList structure to encode
 * @param pSetDb Set Definition database to use while encoding any set defined content.  This database must include a definition for RsslFieldList::setId. 
 * @param setEncodingMaxSize max encoding size of the set defined data, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length set defined data.
 * @see RsslFieldList, RsslEncodeIterator, rsslEncodeFieldListComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeFieldListInit(
							RsslEncodeIterator				*pIter,
							RsslFieldList					*pFieldList,
							const RsslLocalFieldSetDefDb	*pSetDb,
							RsslUInt16						setEncodingMaxSize );


/** 
 * @brief 	Completes encoding of an RsslFieldList
 *
 * Completes RsslFieldList encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFieldListInit()<BR>
 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
 *  3. Call rsslEncodeFieldListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the vector encoding, if false - remove the vector payload from the buffer.
 * @see rsslEncodeFieldListInit, RsslFieldList, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFieldListComplete(
							RsslEncodeIterator  *pIter,
							RsslBool            success );

/** 
 * @brief 	Encodes an RsslFieldEntry where payload is an unencoded primitive type (passed in via void*), blank (where void* is NULL and RsslFieldEntry::encData is empty) , or payload is pre-encoded and populated on RsslFieldEntry::encData.  
 *
 * Encodes an RsslFieldEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFieldEntryInit()<BR>
 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
 *  3. Call rsslEncodeFieldEntryComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFieldEntry populated RsslFieldEntry to encode.  If any pre-encoded payload is present, it must be populated on RsslFieldEntry::encData and should match the type specified in RsslFieldEntry::dataType.  
 * @param pData Pointer to the primitive representation of the entry's payload, should be of type RsslFieldEntry::dataType.  If encoding blank data or content is pre-encoded, this should be passed in as NULL.  
 * @see RsslEncodeIterator, RsslFieldEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFieldEntry(
							RsslEncodeIterator	*pIter,
							RsslFieldEntry		*pField,
							const void			*pData );

/** 
 * @brief 	Begins encoding of an RsslFieldEntry, where any payload is encoded after this call using the appropriate data type encode functions.
 *
 * Begins encoding of an RsslFieldEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFieldListInit()<BR>
 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
 *  3. Call rsslEncodeFieldListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pFieldEntry populated RsslFieldEntry to encode
 * @param encodingMaxSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslFieldEntry, rsslEncodeFieldEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFieldEntryInit(
							RsslEncodeIterator	*pIter,
							RsslFieldEntry		*pField,
							RsslUInt16			encodingMaxSize );
				
/** 
 * @brief 	Completes encoding of an RsslFieldEntry.  
 *
 * Completes encoding of an RsslFieldEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeFieldListInit()<BR>
 *  2. To encode entries, call rsslEncodeFieldEntry() or rsslEncodeFieldEntryInit()..rsslEncodeFieldEntryComplete() for each RsslFieldEntry<BR>
 *  3. Call rsslEncodeFieldListComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslFieldEntry encoding, if false - remove the RsslFieldEntry from the encoded RsslFieldList.
 * @see RsslEncodeIterator, RsslFieldEntry, rsslEncodeFieldEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeFieldEntryComplete(
							RsslEncodeIterator	*pIter,
							RsslBool			success );


/** 
 * @}
 */




/** 
 * @addtogroup FieldListDecoding RsslFieldList and RsslFieldEntry Decoding
 * @{
 */

/**
 * @brief Decodes an RsslFieldList container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeFieldList()<BR>
 *  2. Call rsslDecodeFieldEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pFieldList RsslFieldList structure to populate with decoded contents.  RsslFieldList::encSetData and RsslFieldList::encEntries will point to encoded RsslFieldEntry content.
 * @param pLocalSetDb Set definition database to use while decoding any set defined content.
 * @see rsslDecodeFieldEntry, RsslFieldList, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeFieldList(
							RsslDecodeIterator 		*pIter,
							RsslFieldList			*pFieldList,
							RsslLocalFieldSetDefDb	*pLocalSetDb);


/**
 * @brief Decodes and returns a single RsslFieldEntry from within an RsslFieldList::encSetData and/or RsslFieldList::encEntries
 *
 * @note If RsslFieldList::encSetData is present, but no set definition database was passed in to rsslDecodeFieldList(), set data content will be skipped while decoding entries.
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeFieldList()<BR>
 *  2. Call rsslDecodeFieldEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pFieldEntry RsslFieldEntry to decode content into. RsslFieldEntry::encData will contain encoded content of entry.  To continue decoding into specific data type, RsslFieldEntry::fieldId can be cross referenced with appropriate field dictionary.
 * @see rsslDecodeFieldList, RsslFieldEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeFieldEntry(
							RsslDecodeIterator	*pIter,
							RsslFieldEntry		*pField );
				 

/**
 * @}
 */


/** 
 * @addtogroup FieldListSetInfo
 * @{
 */

/** @brief Encode FieldList set definitions database
 * @param pIter encode iterator
 * @param pFieldListSetDb database to encode
 */
RSSL_API RsslRet rsslEncodeLocalFieldSetDefDb(
							RsslEncodeIterator		*pIter,
							RsslLocalFieldSetDefDb	*pFieldListSetDb );

/**
 * @brief Initialize decoding iterator for a field list set definitions
 * database.
 * @see rsslDecodeMap
 * @see rsslDecodeSeries
 * @see rsslDecodeVector
 * @see rsslDecodeFieldEntry
 *
 * Typical use:<BR>
 *  1. Call rsslDecodeMap(), rsslDecodeSeries() or rsslDecodeVector()
 *  2. Call rsslDecodeLocalFieldSetDefDb()
 * If decoding local set definitions from a single thread, RSSL has internal memory that will be <BR>
 * decoded into.  To use this, entries.data and entries.length should be 0.  <BR>
 * If decoding local set definitions from across multiple threads, the user should supply memory <BR>
 * by populating entries.data.  entries.length should be set to the number of bytes available in data. <BR><BR>
 * Alternatively, a separate iterator may be set to the container's encSetDefs and passed into rsslDecodeLocalFieldSetDefDb() (the appropriate version information should also be set).<BR>
 *
 * @param pIter RsslDecodeIterator Pointer to the decoding iterator.
 * @param pLocalSetDb Pointer to the Field List Set Definitions Database structure to populate.
 */
RSSL_API RsslRet rsslDecodeLocalFieldSetDefDb(
							RsslDecodeIterator		*pIter,
							RsslLocalFieldSetDefDb	*pLocalSetDb);

/** 
 *	@brief Encode a Global set definition dictionary payload.
 *
 *  This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary, 
 *  subsequent calls can be made to this function, each producing the next segment of fields.<BR>
 *
 *	Typical use:<BR>
 *	 1.  Initialize the RsslFieldSetDefDb using rsslClearFieldSetDb, then allocate the definitions pointer array and version information with rsslAllocateFieldSetDefDb.<BR>
 *	 2.  Add the set definitions to the RsslFieldSetDefDb using rsslAddFieldSetDefToDb.<BR>
 *	 3.  Start encoding the message that contains this dictionary payload.<BR>
 *	 4.  Call rsslEncodeFieldSetDefDictionary.<BR>
 *	 5.  If rsslEncodeFieldSetDefDictionary returns RSSL_RET_DICT_PART_ENCODED, go back to step 3, and encode the next part of the payload.  Otherwise, if RSSL_RET_SUCCESS is returned, continue.<BR>
 *	 6.  When finished with the global set definition dictionary, call rsslDeleteFieldSetDefDb to free all allocated memory.<BR>
 * @param eIter Iterator to be used for encoding. Prior to each call, the iterator must be cleared and initialized to the buffer to be used for encoding.
 * @param dictionary The dictionary to encode global field set definition information from.
 * @param currentSetDef Tracks which set definitions have been encoded in case of multi-part encoding. Must be initialized to 0 on the first call and is updated with each successfully encoded part.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @return RSSL_RET_DICT_PART_ENCODED when encoding parts, RSSL_RET_SUCCESS for final part or single complete payload.
 * @see RsslFieldSetDefDb, RDMDictionaryVerbosityValues, rsslDecodeFieldSetDefDictionary
 *	
 */							
RSSL_API RsslRet rsslEncodeFieldSetDefDictionary(
	RsslEncodeIterator *eIter, 
	RsslFieldSetDefDb *dictionary, 
	int *currentSetDef, 
	RDMDictionaryVerbosityValues verbosity, 
	RsslBuffer *errorText);


/** 
 *	@brief Decode a Global field set definition dictionary payload.
 *
 * This function may be called multiple times on the same dictionary, to load information from dictionaries that have been encoded in multiple parts.<BR>
 *
 *	Typical use:<BR>
 *	 1.  Initialize the RsslFieldSetDefDb using rsslClearFieldSetDb, then allocate the definitions pointer array and version information with rsslAllocateFieldSetDefDb.<BR>
 *	 2.  When a response message containing global field set definition data is received, call rsslDecodeFieldSetDefDictionary.<BR>
 *	 3.  Continue with decoding messages.<BR>
 *	 5.  When finished with the global set definition dictionary, call rsslDeleteFieldSetDefDb to free all allocated memory.<BR>
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded field information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeFieldDictionary
 * @see RsslFieldSetDefDb, RDMDictionaryVerbosityValues, rsslDecodeFieldSetDefDictionary
 *	
 */		
RSSL_API RsslRet rsslDecodeFieldSetDefDictionary(
	RsslDecodeIterator *dIter,	
	RsslFieldSetDefDb *dictionary, 
	RDMDictionaryVerbosityValues verbosity, 
	RsslBuffer *errorText);



/**
 * @}
 */



 
 
/**
 *	@addtogroup FStruct 
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_FLF_HAS_FIELD_LIST_INFO flag on the given RsslFieldList.
 *
 * @param pFieldList Pointer to the RsslFieldList to check.
 * @see RsslFieldList, RsslFieldListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslFieldListCheckHasInfo(RsslFieldList *pFieldList)
{
	return ((pFieldList->flags & RSSL_FLF_HAS_FIELD_LIST_INFO) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_FLF_HAS_STANDARD_DATA flag on the given RsslFieldList.
 *
 * @param pFieldList Pointer to the RsslFieldList to check.
 * @see RsslFieldList, RsslFieldListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslFieldListCheckHasStandardData(RsslFieldList *pFieldList)
{
	return ((pFieldList->flags & RSSL_FLF_HAS_STANDARD_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_FLF_HAS_SET_ID flag on the given RsslFieldList.
 *
 * @param pFieldList Pointer to the RsslFieldList to check.
 * @see RsslFieldList, RsslFieldListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslFieldListCheckHasSetId(RsslFieldList *pFieldList)
{
	return ((pFieldList->flags & RSSL_FLF_HAS_SET_ID) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_FLF_HAS_SET_DATA flag on the given RsslFieldList.
 *
 * @param pFieldList Pointer to the RsslFieldList to check.
 * @see RsslFieldList, RsslFieldListFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslFieldListCheckHasSetData(RsslFieldList *pFieldList)
{
	return ((pFieldList->flags & RSSL_FLF_HAS_SET_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_FLF_HAS_FIELD_LIST_INFO flag on the given RsslFieldList.
 * 
 * @param pFieldList Pointer to the RsslFieldList to apply flag value to.
 * @see RsslFieldList, RsslFieldListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslFieldListApplyHasInfo(RsslFieldList *pFieldList)
{
	pFieldList->flags |= RSSL_FLF_HAS_FIELD_LIST_INFO;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_STANDARD_DATA flag on the given RsslFieldList.
 * 
 * @param pFieldList Pointer to the RsslFieldList to apply flag value to.
 * @see RsslFieldList, RsslFieldListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslFieldListApplyHasStandardData(RsslFieldList *pFieldList)
{
	pFieldList->flags |= RSSL_FLF_HAS_STANDARD_DATA;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_SET_ID flag on the given RsslFieldList.
 * 
 * @param pFieldList Pointer to the RsslFieldList to apply flag value to.
 * @see RsslFieldList, RsslFieldListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslFieldListApplyHasSetId(RsslFieldList *pFieldList)
{
	pFieldList->flags |= RSSL_FLF_HAS_SET_ID;
}

/**
 * @brief Applies the ::RSSL_FLF_HAS_SET_DATA flag on the given RsslFieldList.
 * 
 * @param pFieldList Pointer to the RsslFieldList to apply flag value to.
 * @see RsslFieldList, RsslFieldListFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslFieldListApplyHasSetData(RsslFieldList *pFieldList)
{
	pFieldList->flags |= RSSL_FLF_HAS_SET_DATA;
}

/**
 *	@}
 */

 

#ifdef __cplusplus
}
#endif

#endif
