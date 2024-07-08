/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_VECTOR_H
#define __RSSL_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"


/**
 * @addtogroup VStruct
 * @{
 */




/**
 * @brief Flag values for use with the RsslVector. (VTF = Vector Flags)
 * @see RsslVector
 */
typedef enum {
	RSSL_VTF_NONE						= 0x00,		/*!< (0x00) No RsslVector flags are present */
	RSSL_VTF_HAS_SET_DEFS				= 0x01,		/*!< (0x01) The RsslVector contains local set definition information.  If pre-encoded or decoding, this will be located in RsslVector::encSetDefs.  It can also be encoded using the set definition encoding functionality and, upon successful set def encoding, calling rsslEncodeVectorSetDefsComplete().   */
	RSSL_VTF_HAS_SUMMARY_DATA			= 0x02,		/*!< (0x02) The RsslVector contains summary data information.  If pre-encoded or decoding, this will be located in RsslVector::encSummaryData.  It can also be encoded with the appropriate container encode functions and, upon successful summary data container encoding, calling rsslEncodeVectorSummaryDataComplete().  */  
	RSSL_VTF_HAS_PER_ENTRY_PERM_DATA	= 0x04,		/*!< (0x04) The RsslVector contains entries that have permission expressions, contained in relevant entries RsslVectorEntry::permData */
	RSSL_VTF_HAS_TOTAL_COUNT_HINT		= 0x08,		/*!< (0x08) The RsslVector contains a total count hint, contained in RsslVector::totalCountHint.  */
	RSSL_VTF_SUPPORTS_SORTING			= 0x10		/*!< (0x10) The RsslVector supports sorting.  The receiving application is responsible for sorting content and following appropriate behavior based on individual RsslVectorEntry::action information. */
} RsslVectorFlags;

/** 
 * @brief General OMM strings associated with the different vector flags.
 * @see RsslVectorFlags, rsslVectorFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_VTF_HAS_SET_DEFS = { 10, (char*)"HasSetDefs" };
static const RsslBuffer RSSL_OMMSTR_VTF_HAS_SUMMARY_DATA = { 14, (char*)"HasSummaryData" };
static const RsslBuffer RSSL_OMMSTR_VTF_HAS_PER_ENTRY_PERM_DATA = { 19, (char*)"HasPerEntryPermData" };
static const RsslBuffer RSSL_OMMSTR_VTF_HAS_TOTAL_COUNT_HINT = { 17, (char*)"HasTotalCountHint" };
static const RsslBuffer RSSL_OMMSTR_VTF_SUPPORTS_SORTING = { 15, (char*)"SupportsSorting" };

/**
 * @brief Provide general OMM string representation of RsslVectorFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslVectorFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslVectorFlags
 */
RSSL_API RsslRet rsslVectorFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);

/**
 * @brief The RsslVector is a uniform container type of associated index–container pair entries.  Each entry, known as
 * an RsslVectorEntry, contains an \ref RsslUInt index and a container payload.  The payload is a container type as indicated 
 * by RsslVector::containerType.  
 * @see RSSL_INIT_VECTOR, rsslClearVector, RsslVectorFlags, rsslDecodeVector, rsslEncodeVectorInit, rsslEncodeVectorComplete, rsslEncodeVectorSetDefsComplete, rsslEncodeVectorSummaryDataComplete
 */
typedef struct
{
	RsslUInt8           flags;			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslVectorFlags */

	RsslContainerType	containerType;  /*!< @brief The container type associated with the payload of each RsslVectorEntry */

	RsslBuffer			encSetDefs;		/*!< @brief Contains encoded set definition information when present. Presence is indicated by \ref RsslVectorFlags ::RSSL_VTF_HAS_SET_DEFS.   When decoding, this contains encoded set definitions.  When encoding, pre-encoded set definition information can be specified here; if not pre-encoded the user can encode set definitions and invoke rsslEncodeVectorSetDefsComplete() when ready to continue encoding RsslVector contents. */
	RsslBuffer			encSummaryData; /*!< @brief Contains encoded summary data information when present.  Presence is indicated by \ref RsslVectorFlags ::RSSL_VTF_HAS_SUMMARY_DATA.  When decoding, this contains encoded set definitions.  When encoding, pre-encoded summary data information can be specified here; if not pre-encoded the user can encode using the appropriate container type encode functionality and invoke rsslEncodeVectorSummaryDataComplete() when ready to continue encoding RsslVector contents. */

	RsslUInt32			totalCountHint; /*!< @brief Contains an approximate total RsslVectorEntry count for the RsslVector */
	
	RsslBuffer          encEntries;		/*!< @brief Raw encoded payload for the RsslVector */
} RsslVector;


/**
 * @brief RsslVector static initializer
 * @see RsslVector, rsslClearVector
 */
#define RSSL_INIT_VECTOR { 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslVector
 * @see RsslVector, RSSL_INIT_VECTOR
 */
RTR_C_INLINE void rsslClearVector(RsslVector *pVector)
{
	pVector->flags = 0;
	pVector->containerType = 0;
	pVector->encSetDefs.length = 0;
	pVector->encSetDefs.data = 0;
	pVector->encSummaryData.length = 0;
	pVector->encSummaryData.data = 0;
	pVector->totalCountHint = 0;
	pVector->encEntries.length = 0;
	pVector->encEntries.data = 0;
}

/**
 * @}
 */


/**
 * @addtogroup VEStruct 
 *	@{
 */

/**
 * @brief Flag values for use with each RsslVectorEntry contained in an RsslVector. (VTEF = VectorEntry Flag)
 * @see RsslVectorEntry, RsslVector
 */
typedef enum
{
	RSSL_VTEF_NONE			= 0x00,		/*!< (0x00) No RsslVectorEntry flags are present*/
	RSSL_VTEF_HAS_PERM_DATA	= 0x01		/*!< (0x01) This RsslVectorEntry contains permission data, located in RsslVectorEntry::permData. */
} RsslVectorEntryFlags;

/** 
 * @brief General OMM strings associated with the different vector entry flags.
 * @see RsslVectorEntryFlags, rsslVectorEntryFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_VTEF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };

/**
 * @brief Action that indicates how to apply contents of RsslVectorEntry. (VTEA = VectorEntry Action)
 * @see RsslVectorEntry, RsslVector
 */
typedef enum
{
	RSSL_VTEA_UPDATE_ENTRY	= 1,		/*!< (1) Apply the contents of this entry as an update at the specified index.  If an RsslVectorEntry with an ::RSSL_VTEA_UPDATE_ENTRY action is received prior to an ::RSSL_VTEA_SET_ACTION or ::RSSL_VTEA_INSERT_ACTION action for the same RsslVectorEntry::index, the entry with the ::RSSL_VTEA_UPDATE_ENTRY action can be ignored.  */
	RSSL_VTEA_SET_ENTRY		= 2,		/*!< (2) Set/Replace the contents of the specified RsslVectorEntry::index with the contents of this entry */
	RSSL_VTEA_CLEAR_ENTRY	= 3,		/*!< (3) Clear the contents of the entry at the specified RsslVectorEntry::index, leaving the index present but empty. */
	RSSL_VTEA_INSERT_ENTRY	= 4,		/*!< (4) Applies only when RsslVector is sortable (e.g. ::RSSL_VTF_SUPPORTS_SORTING present in RsslVector::flags).  Insert the contents of this entry at RsslVectorEntry::index specified, any higher value entries index values are incremented.  */
	RSSL_VTEA_DELETE_ENTRY	= 5			/*!< (5) Applies only when RsslVector is sortable (e.g. ::RSSL_VTF_SUPPORTS_SORTING present in RsslVector::flags).  Delete the contents of the entry at the specified RsslVectorEntry::index, any higher order index values are decremented.  @note ::RSSL_VTEA_DELETE_ENTRY and ::RSSL_VTEA_CLEAR_ENTRY actions cannot carry any entry payload.  If payload is necessary for a ::RSSL_VTEA_DELETE_ENTRY or ::RSSL_VTEA_CLEAR_ENTRY action, an entry with an ::RSSL_VTEA_UPDATE_ENTRY action can be sent and immediatley followed by the same RsslVectorEntry::index on an entry with a ::RSSL_VTEA_DELETE_ENTRY or ::RSSL_VTEA_CLEAR_ENTRY action.   */
} RsslVectorEntryActions;

/** 
 * @brief General OMM strings associated with the different vector entry actions.
 * @see RsslVectorEntryActions, rsslVectorEntryActionToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_VTEA_UPDATE_ENTRY = { 6, (char*)"Update" };
static const RsslBuffer RSSL_OMMSTR_VTEA_SET_ENTRY = { 3, (char*)"Set" };
static const RsslBuffer RSSL_OMMSTR_VTEA_CLEAR_ENTRY = { 5, (char*)"Clear" };
static const RsslBuffer RSSL_OMMSTR_VTEA_INSERT_ENTRY = { 6, (char*)"Insert" };
static const RsslBuffer RSSL_OMMSTR_VTEA_DELETE_ENTRY = { 6, (char*)"Delete" };

/**
 * @brief Provide general OMM string representation of RsslVectorEntryFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslVectorEntryFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslVectorEntryFlags
 */
RSSL_API RsslRet rsslVectorEntryFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags);

/**
 * @brief Provide a general OMM string representation for a vector entry action enumeration
 * @see RsslVectorEntryActions
 */
RSSL_API const char* rsslVectorEntryActionToOmmString(RsslUInt8 action);

/**
 * @brief One entry contained in an RsslVector.  The contents of the entry is specified by RsslVector::containerType.  Any contents should be applied following the rules of the specified RsslVectorEntry::action ( \ref RsslVectorEntryActions)
 * @see RSSL_INIT_VECTOR_ENTRY, rsslClearVectorEntry, RsslVectorEntryFlags, RsslVectorEntryActions, rsslDecodeVectorEntry, rsslEncodeVectorEntryInit, rsslEncodeVectorEntryComplete
 */
typedef struct {
	RsslUInt8				flags;		/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslVectorEntryFlags */
	RsslUInt8				action;		/*!< @brief Action to use when applying RsslVectorEntry information.  The available actions are defined in RsslVectorEntryActions enumeration. */
	RsslUInt32				index;		/*!< @brief 0-base entry index for the RsslVectorEntry.  This can be up to a 30-bit value (RSSL_VTE_MAX_INDEX). */
	RsslBuffer				permData;	/*!< @brief Contains permission data associated with this RsslVectorEntry and its contents.  Presence is indicated by \ref RsslVectorEntryFlags::RSSL_VTEF_HAS_PERM_DATA.   */
	RsslBuffer				encData;	/*!< @brief Raw encoded content for this RsslVectorEntry */
} RsslVectorEntry;

/**
 * @brief RsslVectorEntry static initializer
 * @see RsslVectorEntry, rsslClearVectorEntry
 */
#define RSSL_INIT_VECTOR_ENTRY { 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslVectorEntry 
 * @see RsslVectorEntry, RSSL_INIT_VECTOR_ENTRY
 */
RTR_C_INLINE void rsslClearVectorEntry( RsslVectorEntry *pVectorEntry )
{
	memset(pVectorEntry, 0, sizeof(RsslVectorEntry));
}

/**
 * @brief Maximum allowed value for a vector index (RsslVectorEntry::index).
 */
#define RSSL_VTE_MAX_INDEX 0x3fffffffU

/**
 * @}
 */


/**
 * @addtogroup VectorEncoding RsslVector and RsslVectorEntry Encoding
 * @{
 */

/** 
 * @brief 	Begin encoding process for RsslVector container type.
 *
 * Begins encoding of an RsslVector<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit()<BR>
 *  2. If RsslVector contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeVectorSetDefsComplete()<BR>
 *  3. If RsslVector contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeVectorSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  5. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pVector	partially populated RsslVector structure to encode
 * @param summaryMaxSize max expected encoding size of the summary data, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length summary data.
 * @param setMaxSize max encoding size of the set information, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length set definition data.
 * @see RsslVector, RsslEncodeIterator, rsslEncodeVectorComplete, rsslEncodeVectorSetDefsComplete, rsslEncodeVectorSummaryDataComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeVectorInit(
								RsslEncodeIterator	*pIter, 
								RsslVector			*pVector,
								RsslUInt16			summaryMaxSize,
								RsslUInt16			setMaxSize );


/**
 * @brief Complete set data encoding for an RsslVector. If both rsslEncodeVectorSetDefsComplete()
 * and rsslEncodeVectorSummaryDataComplete() are called, rsslEncodeVectorSetDefsComplete()
 * must be called first.  If set definitions are pre-encoded, they should be set on RsslVector::encSetDefs and this function is not required.
 * and this function is not required.  
 * 
 * Completes set data encoding within an RsslVector. <BR> 
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit() with ::RSSL_VTF_HAS_SET_DATA flag set on RsslVector::flags<BR>
 *  2. Encode set definition content using appropriate set definition encode functions<BR>
 *  3. Call rsslEncodeVectorSetDefsComplete() <BR>
 *  4. If present, encode any non pre-encoded summary data and call rsslEncodeVectorSummaryDataComplete()<BR>
 *  5. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  6. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator
 * @param success If true - successfully complete the set definition encoding within the RsslVector, if false - remove the set definitions from the encoded RsslVector.  
 * @see rsslEncodeVectorInit, RsslEncodeIterator, RsslVector
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorSetDefsComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );




/**
 * @brief Complete summary data encoding for an RsslVector. If both rsslEncodeVectorSetDefsComplete()
 * and rsslEncodeVectorSummaryDataComplete() are called, rsslEncodeVectorSetDefsComplete()
 * must be called first. If summary data is pre-encoded, it should be set on RsslVector::encSummaryData and this  function is not required.  
 * 
 * Completes summary data encoding within an RsslVector<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit() with ::RSSL_VTF_HAS_SUMMARY_DATA flag set on RsslVector::flags<BR>
 *  2. If present, encode any non pre-encoded set definition data and call rsslEncodeVectorSetDefsComplete()<BR>
 *  3. Encode summary data content using appropriate container encoders.  Summary data usually matches type specified in RsslVector::containerType.<BR>
 *  4. Call rsslEncodeVectorSummaryDataComplete() <BR>
 *  5. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  6. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator. 
 * @param success If true - successfully complete the summary data encoding within the RsslVector, if false - remove the summary data from the encoded RsslVector.  
 * @see rsslEncodeVectorInit, RsslEncodeIterator, RsslVector
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorSummaryDataComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );



/** 
 * @brief 	Completes encoding of an RsslVector
 *
 * Completes RsslVector encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit()<BR>
 *  2. If RsslVector contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeVectorSetDefsComplete()<BR>
 *  3. If RsslVector contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeVectorSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  5. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the vector encoding, if false - remove the vector payload from the buffer.
 * @see rsslEncodeVectorInit, RsslVector, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorComplete( 
								RsslEncodeIterator	*pIter,
								RsslBool			success );


/** 
 * @brief 	Encodes an RsslVectorEntry where there is no payload or any payload is pre-encoded and populated on RsslVectorEntry::encData.  
 *
 * Encodes an RsslVectorEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit()<BR>
 *  2. If RsslVector contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeVectorSetDefsComplete()<BR>
 *  3. If RsslVector contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeVectorSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  5. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pVectorEntry populated RsslVectorEntry to encode.  If any payload is present, it must be populated on RsslVectorEntry::encData.  
 * @see RsslEncodeIterator, RsslVectorEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorEntry( 
								RsslEncodeIterator	*pIter,
								RsslVectorEntry		*pVectorEntry );


/** 
 * @brief 	Begins encoding of an RsslVectorEntry, where any payload is encoded after this call using the appropriate container type encode functions, as specified by RsslVector::containerType.  
 *
 * Begins encoding of an RsslVectorEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit()<BR>
 *  2. If RsslVector contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeVectorSetDefsComplete()<BR>
 *  3. If RsslVector contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeVectorSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  5. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pVectorEntry populated RsslVectorEntry to encode
 * @param maxEncodingSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslVectorEntry, rsslEncodeVectorEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorEntryInit( 
									RsslEncodeIterator	*pIter,
									RsslVectorEntry		*pVectorEntry,
									RsslUInt16			maxEncodingSize );


/** 
 * @brief 	Completes encoding of an RsslVectorEntry.  
 *
 * Completes encoding of an RsslVectorEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeVectorInit()<BR>
 *  2. If RsslVector contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeVectorSetDefsComplete()<BR>
 *  3. If RsslVector contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeVectorSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeVectorEntry() or rsslEncodeVectorEntryInit()..rsslEncodeVectorEntryComplete() for each RsslVectorEntry<BR>
 *  5. Call rsslEncodeVectorComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslVectorEntry encoding, if false - remove the RsslVectorEntry from the encoded RsslVector.
 * @see RsslEncodeIterator, RsslVectorEntry, rsslEncodeVectorEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeVectorEntryComplete( 
									RsslEncodeIterator	*pIter,
									RsslBool			success );

/**
 * @}
 */



/**
 * @addtogroup VectorDecoding RsslVector and RsslVectorEntry Decoding
 * @{
 */


/**
 * @brief Decodes an RsslVector container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeVector()<BR>
 *  2. Call rsslDecodeVectorEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pVector RsslVector structure to populate with decoded contents.  RsslVector::encEntries will point to encoded RsslVectorEntry content.
 * @see rsslDecodeVectorEntry, RsslVector, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeVector( 
								RsslDecodeIterator	*pIter,
								RsslVector			*pVector  );
						 



/**
 * @brief Decodes and returns a single RsslVectorEntry from within an RsslVector::encEntries
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeVector()<BR>
 *  2. Call rsslDecodeVectorEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pVectorEntry RsslVectorEntry to decode content into.  RsslVectorEntry::encData will contain encoded content of entry.  
 * @see rsslDecodeVector, RsslVectorEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeVectorEntry( 
								RsslDecodeIterator	*pIter,
								RsslVectorEntry		*pVectorEntry  );
							  



/**
 * @}
 */


/**
 * @addtogroup VStruct
 * @{
 */
 
/**
 * @brief Checks the presence of the ::RSSL_VTF_HAS_SET_DEFS flag on the given RsslVector.
 *
 * @param pVector Pointer to the RsslVector to check.
 * @see RsslVector, RsslVectorFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorCheckHasSetDefs(RsslVector *pVector)
{
	return ((pVector->flags & RSSL_VTF_HAS_SET_DEFS) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_VTF_HAS_SUMMARY_DATA flag on the given RsslVector.
 *
 * @param pVector Pointer to the RsslVector to check.
 * @see RsslVector, RsslVectorFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorCheckHasSummaryData(RsslVector *pVector)
{
	return ((pVector->flags & RSSL_VTF_HAS_SUMMARY_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_VTF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslVector.
 *
 * @param pVector Pointer to the RsslVector to check.
 * @see RsslVector, RsslVectorFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorCheckHasPerEntryPermData(RsslVector *pVector)
{
	return ((pVector->flags & RSSL_VTF_HAS_PER_ENTRY_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_VTF_HAS_TOTAL_COUNT_HINT flag on the given RsslVector.
 *
 * @param pVector Pointer to the RsslVector to check.
 * @see RsslVector, RsslVectorFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorCheckHasTotalCountHint(RsslVector *pVector)
{
	return ((pVector->flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT) ?
				RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_VTF_SUPPORTS_SORTING flag on the given RsslVector.
 *
 * @param pVector Pointer to the RsslVector to check.
 * @see RsslVector, RsslVectorFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorCheckSupportsSorting(RsslVector *pVector)
{
	return ((pVector->flags & RSSL_VTF_SUPPORTS_SORTING) ?
					RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_VTF_HAS_SET_DEFS flag on the given RsslVector.
 * 
 * @param pVector Pointer to the RsslVector to apply flag value to.
 * @see RsslVector, RsslVectorFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorApplyHasSetDefs(RsslVector *pVector)
{
	pVector->flags |= RSSL_VTF_HAS_SET_DEFS;
}

/**
 * @brief Applies the ::RSSL_VTF_HAS_SUMMARY_DATA flag on the given RsslVector.
 * 
 * @param pVector Pointer to the RsslVector to apply flag value to.
 * @see RsslVector, RsslVectorFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorApplyHasSummaryData(RsslVector *pVector)
{
	pVector->flags  |= RSSL_VTF_HAS_SUMMARY_DATA;
}

/**
 * @brief Applies the ::RSSL_VTF_HAS_PER_ENTRY_PERM_DATA flag on the given RsslVector.
 * 
 * @param pVector Pointer to the RsslVector to apply flag value to.
 * @see RsslVector, RsslVectorFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorApplyHasPerEntryPermData(RsslVector *pVector)
{
	pVector->flags |=  RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_VTF_HAS_TOTAL_COUNT_HINT flag on the given RsslVector.
 * 
 * @param pVector Pointer to the RsslVector to apply flag value to.
 * @see RsslVector, RsslVectorFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorApplyHasTotalCountHint(RsslVector *pVector)
{
	pVector->flags |= RSSL_VTF_HAS_TOTAL_COUNT_HINT;
}

/**
 * @brief Applies the ::RSSL_VTF_SUPPORTS_SORTING flag on the given RsslVector.
 * 
 * @param pVector Pointer to the RsslVector to apply flag value to.
 * @see RsslVector, RsslVectorFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorApplySupportsSorting(RsslVector *pVector)
{
	pVector->flags |= RSSL_VTF_SUPPORTS_SORTING;
}

/**
 *	@}
 */
 
/**
 * @addtogroup VEStruct
 * @{
 */

/**
 * @brief Checks the presence of the ::RSSL_VTEF_HAS_PERM_DATA flag on the given RsslVectorEntry.
 *
 * @param pVectorEntry Pointer to the RsslVectorEntry to check.
 * @see RsslVectorEntry, RsslVectorEntryFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslVectorEntryCheckHasPermData(RsslVectorEntry *pVectorEntry)
{
	return ((pVectorEntry->flags & RSSL_VTEF_HAS_PERM_DATA) ?
				RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_VTEF_HAS_PERM_DATA flag on the given RsslVectorEntry.
 * 
 * @param pVectorEntry Pointer to the RsslVectorEntry to apply flag value to.
 * @see RsslVectorEntry, RsslVectorEntryFlags
 * @return No return value
 */
RTR_C_INLINE void rsslVectorEntryApplyHasPermData(RsslVectorEntry *pVectorEntry)
{
	pVectorEntry->flags |=  RSSL_VTEF_HAS_PERM_DATA;
}

/**
 * @}
 */
 



#ifdef __cplusplus
}
#endif

#endif
