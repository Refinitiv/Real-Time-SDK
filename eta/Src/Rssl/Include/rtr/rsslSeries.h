
#ifndef __RSSL_SERIES_H
#define __RSSL_SERIES_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"

/**
 * @addtogroup SStruct
 *	@{
 */


/**
 * @brief Flag values for use with the RsslSeries.  (SRF = Series Flags)
 * @see RsslSeries
 */
typedef enum
{
	RSSL_SRF_NONE					= 0x00,			/*!< (0x00) No RsslSeries flags are present. */
	RSSL_SRF_HAS_SET_DEFS			= 0x01,			/*!< (0x01) The RsslSeries contains local set definition information.  If pre-encoded or decoding, this will be located in RsslSeries::encSetDefs.  It can also be encoded using the set definition encoding functionality and, upon successful set def encoding, calling rsslEncodeSeriesSetDefsComplete().    */
	RSSL_SRF_HAS_SUMMARY_DATA		= 0x02,			/*!< (0x02) The RsslSeries contains summary data information.  If pre-encoded or decoding, this will be located in RsslSeries::encSummaryData.  It can also be encoded with the appropriate container encode functions and, upon successful summary data container encoding, calling rsslEncodeSeriesSummaryDataComplete().  */  
	RSSL_SRF_HAS_TOTAL_COUNT_HINT	= 0x04 			/*!< (0x04) The RsslSeries contains a total count hint, contained in RsslSeries::totalCountHint.  */
} RsslSeriesFlags;


/**
 * @brief The RsslSeries is a uniform container type entries.  Each entry, known as
 * an RsslSeriesEntry, is implicitly indexed and houses a container type  payload as indicated by RsslSeries::containerType. 
 * This is well suited for sending infrequently or non-updating table like content, where each RsslSeriesEntry represents an additional row in a table.  
 * @see RSSL_INIT_SERIES, rsslClearSeries, RsslSeriesFlags, rsslDecodeSeries, rsslEncodeSeriesInit, rsslEncodeSeriesComplete, rsslEncodeSeriesSetDefsComplete, rsslEncodeSeriesSummaryDataComplete
 */
typedef struct
{
	RsslUInt8			flags;  			/*!< @brief Flag values used to indicate optional member presence or behavior.  The available options are defined by values present in RsslSeriesFlags */
	
	RsslContainerType	containerType;		/*!< @brief The container type associated with the payload of each RsslSeriesEntry */

	RsslBuffer			encSetDefs; 		/*!< @brief Contains encoded set definition information when present. Presence is indicated by \ref RsslSeriesFlags ::RSSL_SRF_HAS_SET_DEFS.   When decoding, this contains encoded set definitions.  When encoding, pre-encoded set definition information can be specified here; if not pre-encoded the user can encode set definitions and invoke rsslEncodeSeriesSetDefsComplete() when ready to continue encoding RsslSeries contents. */
	RsslBuffer			encSummaryData; 	/*!< @brief Contains encoded summary data information when present.  Presence is indicated by \ref RsslSeriesFlags ::RSSL_SRF_HAS_SUMMARY_DATA.  When decoding, this contains encoded set definitions.  When encoding, pre-encoded summary data information can be specified here; if not pre-encoded the user can encode using the appropriate container type encode functionality and invoke rsslEncodeSeriesSummaryDataComplete() when ready to continue encoding RsslSeries contents. */

	RsslUInt32			totalCountHint;		/*!< @brief Contains an approximate total RsslSeriesEntry count for the RsslSeries */
	
	RsslBuffer			encEntries;			/*!< @brief Raw encoded payload for the RsslSeries */
} RsslSeries;


/** 
 * @brief RsslSeries static initializer
 * @see RsslSeries, rsslClearSeries
 */
#define RSSL_INIT_SERIES { 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslSeries
 * @see RsslSeries, RSSL_INIT_SERIES
 */
RTR_C_INLINE void rsslClearSeries(RsslSeries *pSeries)
{
	pSeries->flags = 0;
	pSeries->containerType = 0;
	pSeries->encSetDefs.length = 0;
	pSeries->encSetDefs.data = 0;
	pSeries->encSummaryData.length = 0;
	pSeries->encSummaryData.data = 0;
	pSeries->totalCountHint = 0;
	pSeries->encEntries.length = 0;
	pSeries->encEntries.data = 0;
}

/**
 * @}
 */


/**
 * @addtogroup SEStruct 
 *	@{
 */

/**
 * @brief One entry contained in an RsslSeries.  The contents of the entry is specified by RsslSeries::containerType.  RsslSeriesEntry has an implicit action of Add, so any additional entries should be appended to any existing content.
 * @see RSSL_INIT_SERIES_ENTRY, rsslClearSeriesEntry, rsslDecodeSeriesEntry, rsslEncodeSeriesEntryInit, rsslEncodeSeriesEntryComplete
 */
typedef struct {
	RsslBuffer		encData;			/*!< @brief Raw encoded content for this RsslSeriesEntry */
} RsslSeriesEntry;

/**
 * @brief RsslSeriesEntry static initializer
 * @see RsslSeriesEntry, rsslClearSeriesEntry
 */
#define RSSL_INIT_SERIES_ENTRY { RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslSeriesEntry 
 * @see RsslSeriesEntry, RSSL_INIT_SERIES_ENTRY
 */
RTR_C_INLINE void rsslClearSeriesEntry(RsslSeriesEntry *pSeriesEntry)     
{
	pSeriesEntry->encData.data = 0;
	pSeriesEntry->encData.length = 0;
}

/**
 * @}
 */


/**
 * @addtogroup SeriesEncoding RsslSeries and RsslSeriesEntry Encoding
 * @{
 */


/** 
 * @brief 	Begin encoding process for RsslSeries container type.
 *
 * Begins encoding of an RsslSeries<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit()<BR>
 *  2. If RsslSeries contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. If RsslSeries contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeSeriesSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  5. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pSeries	partially populated RsslSeries structure to encode
 * @param summaryMaxSize max expected encoding size of the summary data, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length summary data.
 * @param setMaxSize max encoding size of the set information, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length set definition data.
 * @see RsslSeries, RsslEncodeIterator, rsslEncodeSeriesComplete, rsslEncodeSeriesSetDefsComplete, rsslEncodeSeriesSummaryDataComplete
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet     rsslEncodeSeriesInit( 
									RsslEncodeIterator	*pIter,
									RsslSeries			*pSeries,
									RsslUInt16			summaryMaxSize,
									RsslUInt16			setMaxSize );
			        


/**
 * @brief Complete set data encoding for an RsslSeries. If both rsslEncodeSeriesSetDefsComplete()
 * and rsslEncodeSeriesSummaryDataComplete() are called, rsslEncodeSeriesSetDefsComplete()
 * must be called first.  If set definitions are pre-encoded, they should be set on RsslSeries::encSetDefs and this function is not required.
 * and this function is not required.  
 * 
 * Completes set data encoding within an RsslSeries. <BR> 
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit() with ::RSSL_SRF_HAS_SET_DATA flag set on RsslSeries::flags<BR>
 *  2. Encode set definition content using appropriate set definition encode functions<BR>
 *  3. Call rsslEncodeSeriesSetDefsComplete() <BR>
 *  4. If present, encode any non pre-encoded summary data and call rsslEncodeSeriesSummaryDataComplete()<BR>
 *  5. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  6. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator
 * @param success If true - successfully complete the set definition encoding within the RsslSeries, if false - remove the set definitions from the encoded RsslSeries.  
 * @see rsslEncodeSeriesInit, RsslEncodeIterator, RsslSeries
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesSetDefsComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );



/**
 * @brief Complete summary data encoding for an RsslSeries. If both rsslEncodeSeriesSetDefsComplete()
 * and rsslEncodeSeriesSummaryDataComplete() are called, rsslEncodeSeriesSetDefsComplete()
 * must be called first. If summary data is pre-encoded, it should be set on RsslSeries::encSummaryData and this  function is not required.  
 * 
 * Completes summary data encoding within an RsslSeries<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit() with ::RSSL_SRF_HAS_SUMMARY_DATA flag set on RsslSeries::flags<BR>
 *  2. If present, encode any non pre-encoded set definition data and call rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. Encode summary data content using appropriate container encoders.  Summary data usually matches type specified in RsslSeries::containerType.<BR>
 *  4. Call rsslEncodeSeriesSummaryDataComplete() <BR>
 *  5. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  6. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter Pointer to the encode iterator. 
 * @param success If true - successfully complete the summary data encoding within the RsslSeries, if false - remove the summary data from the encoded RsslSeries.  
 * @see rsslEncodeSeriesInit, RsslEncodeIterator, RsslSeries
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesSummaryDataComplete(
								RsslEncodeIterator	*pIter,
								RsslBool			success );



/** 
 * @brief 	Completes encoding of an RsslSeries
 *
 * Completes RsslSeries encoding <BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit()<BR>
 *  2. If RsslSeries contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. If RsslSeries contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeSeriesSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  5. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the Series encoding, if false - remove the Series payload from the buffer.
 * @see rsslEncodeSeriesInit, RsslSeries, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesComplete( 
								RsslEncodeIterator	*pIter,
								RsslBool			success );




/** 
 * @brief 	Encodes an RsslSeriesEntry where there is no payload or any payload is pre-encoded and populated on RsslSeriesEntry::encData.  
 *
 * Encodes an RsslSeriesEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit()<BR>
 *  2. If RsslSeries contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. If RsslSeries contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeSeriesSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  5. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pSeriesEntry populated RsslSeriesEntry to encode.  If any payload is present, it must be populated on RsslSeriesEntry::encData.  
 * @see RsslEncodeIterator, RsslSeriesEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesEntry(
								RsslEncodeIterator	*pIter,
								RsslSeriesEntry		*pSeriesEntry );


/** 
 * @brief 	Begins encoding of an RsslSeriesEntry, where any payload is encoded after this call using the appropriate container type encode functions, as specified by RsslSeries::containerType.  
 *
 * Begins encoding of an RsslSeriesEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit()<BR>
 *  2. If RsslSeries contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. If RsslSeries contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeSeriesSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  5. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pSeriesEntry populated RsslSeriesEntry to encode
 * @param maxEncodingSize max expected encoding size of the entry's payload, if encoding.  This allows for optimized storage of length when encoding - value of 0 can be used if not encoding or to allow for maximum supported length for the entry's payload.
 * @see RsslEncodeIterator, RsslSeriesEntry, rsslEncodeSeriesEntryComplete
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesEntryInit( 
								RsslEncodeIterator	*pIter,
								RsslSeriesEntry		*pSeriesEntry,
								RsslUInt16			maxEncodingSize );




/** 
 * @brief 	Completes encoding of an RsslSeriesEntry.  
 *
 * Completes encoding of an RsslSeriesEntry<BR>
 * Typical use:<BR>
 *  1. Call rsslEncodeSeriesInit()<BR>
 *  2. If RsslSeries contains set definitions that are not pre-encoded, call appropriate set definition encode functions, followed by rsslEncodeSeriesSetDefsComplete()<BR>
 *  3. If RsslSeries contains summary data that is not pre-encoded, call appropriate summary data container encoders, followed by rsslEncodeSeriesSummaryDataComplete()<BR>
 *  4. To encode entries, call rsslEncodeSeriesEntry() or rsslEncodeSeriesEntryInit()..rsslEncodeSeriesEntryComplete() for each RsslSeriesEntry<BR>
 *  5. Call rsslEncodeSeriesComplete() when all entries are completed<BR>
 *
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the RsslSeriesEntry encoding, if false - remove the RsslSeriesEntry from the encoded RsslSeries.
 * @see RsslEncodeIterator, RsslSeriesEntry, rsslEncodeSeriesEntryInit
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeSeriesEntryComplete( 
								RsslEncodeIterator	*pIter,
								RsslBool			success );



/**
 * @}
 */
 

/**
 * @addtogroup SeriesDecoding RsslSeries and RsslSeriesEntry Decoding
 * @{
 */


/**
 * @brief Decodes an RsslSeries container
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeSeries()<BR>
 *  2. Call rsslDecodeSeriesEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pSeries RsslSeries structure to populate with decoded contents.  RsslSeries::encEntries will point to encoded RsslSeriesEntry content.
 * @see rsslDecodeSeriesEntry, RsslSeries, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeSeries(
								RsslDecodeIterator	*pIter, 
								RsslSeries			*pSeries );
						  

/**
 * @brief Decodes and returns a single RsslSeriesEntry from within an RsslSeries::encEntries
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeSeries()<BR>
 *  2. Call rsslDecodeSeriesEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter	Decode iterator  to use for decode process
 * @param pSeriesEntry RsslSeriesEntry to decode content into.  RsslSeriesEntry::encData will contain encoded content of entry.  
 * @see rsslDecodeSeries, RsslSeriesEntry, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeSeriesEntry(
								RsslDecodeIterator *pIter, 
								RsslSeriesEntry    *pSeriesEntry );
								 



/**
 * @}
 */


/**
 * @addtogroup SStruct
 * @{
 */

/**
 * @brief Checks the presence of the ::RSSL_SRF_HAS_SET_DEFS flag on the given RsslSeries.
 *
 * @param pSeries Pointer to the RsslSeries to check.
 * @see RsslSeries, RsslSeriesFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslSeriesCheckHasSetDefs(RsslSeries *pSeries)
{
	return ((pSeries->flags & RSSL_SRF_HAS_SET_DEFS) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Checks the presence of the ::RSSL_SRF_HAS_SUMMARY_DATA flag on the given RsslSeries.
 *
 * @param pSeries Pointer to the RsslSeries to check.
 * @see RsslSeries, RsslSeriesFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslSeriesCheckHasSummaryData(RsslSeries *pSeries)
{
	return ((pSeries->flags & RSSL_SRF_HAS_SUMMARY_DATA) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Checks the presence of the ::RSSL_SRF_HAS_TOTAL_COUNT_HINT flag on the given RsslSeries.
 *
 * @param pSeries Pointer to the RsslSeries to check.
 * @see RsslSeries, RsslSeriesFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_INLINE RsslBool rsslSeriesCheckHasTotalCountHint(RsslSeries *pSeries)
{
	return ((pSeries->flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT) ?
				RSSL_TRUE : RSSL_FALSE);
}


/**
 * @brief Applies the ::RSSL_SRF_HAS_SET_DEFS flag on the given RsslSeries.
 * 
 * @param pSeries Pointer to the RsslSeries to apply flag value to.
 * @see RsslSeries, RsslSeriesFlags
 * @return No return value
 */
RTR_C_INLINE void rsslSeriesApplyHasSetDefs(RsslSeries *pSeries)
{
	pSeries->flags |= RSSL_SRF_HAS_SET_DEFS;
}

/**
 * @brief Applies the ::RSSL_SRF_HAS_SUMMARY_DATA flag on the given RsslSeries.
 * 
 * @param pSeries Pointer to the RsslSeries to apply flag value to.
 * @see RsslSeries, RsslSeriesFlags
 * @return No return value
 */
RTR_C_INLINE void rsslSeriesApplyHasSummaryData(RsslSeries *pSeries)
{
	pSeries->flags |= RSSL_SRF_HAS_SUMMARY_DATA;
}

/**
 * @brief Applies the ::RSSL_SRF_HAS_TOTAL_COUNT_HINT flag on the given RsslSeries.
 * 
 * @param pSeries Pointer to the RsslSeries to apply flag value to.
 * @see RsslSeries, RsslSeriesFlags
 * @return No return value
 */
RTR_C_INLINE void rsslSeriesApplyHasTotalCountHint(RsslSeries *pSeries)
{
	pSeries->flags |= RSSL_SRF_HAS_TOTAL_COUNT_HINT;
}

/**
 *	@}
 */


#ifdef __cplusplus
}
#endif 



#endif
