#include "rtr/rsslArray.h"
#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslDateTime.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslFieldList.h"
#include "rtr/rsslFilterList.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslMap.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/rsslPrimitiveEncoders.h"
#include "rtr/rsslQos.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslReal.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslRmtes.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslSetData.h"
#include "rtr/rsslState.h"
#include "rtr/rsslTypes.h"
#include "rtr/rsslVector.h"

/**
 * @brief Finish decoding a container if application does not want to finish decoding remaining container entries.
 * 
 * Once a user begins decoding a container, typically they must decode all entries in that container before continuing.
 * This function may be used to skip straight to the end of the current container being decoded. <BR> 
 * Typical use:<BR>
 *	1. Call the appropriate container decoder(e.g. rsslDecodeFieldList()).
 *	2. Call the entry decoder until the desired entry is found(e.g. an RsslFieldEntry with a particular fieldID).
 *	3. Call rsslFinishDecodeEntries() to complete decoding the container and continue encoding from level above (e.g. next RsslMapEntry if decoding RsslMap of field lists).
 *
 * @param pIter RsslDecodeIterator used to decode container
 * @return ::RSSL_RET_END_OF_CONTAINER when successful.
 * @see RsslDecodeIterator
 */
RsslRet rsslFinishDecodeEntries(RsslDecodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Begin encoding process for RsslArray primitive type.
 *
 * Begins encoding of an RsslArray.<BR>
 * Typical use:<BR>
 *	1. Call rsslEncodeArrayInit()<BR>
 *	2. Call rsslEncodeArrayEntry() for each item in the array<BR>
 *	3. Call rsslEncodeArrayComplete()<BR>
 * 
 * @param pIter	Pointer to the encoder iterator.
 * @param pArray partially populated RsslArray structure to encode
 * @see RsslArray, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RsslRet rsslEncodeArrayInit(RsslEncodeIterator *pIter, RsslArray *pArray)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Completes array encoding
 *
 * Completes encoding of an RsslArray.<BR>
 * Typical use:<BR>
 *	1. Call rsslEncodeArrayInit()<BR>
 *	2. Call rsslEncodeArrayEntry() for each item in the array<BR>
 *	3. Call rsslEncodeArrayComplete()<BR>
 * 
 * @param pIter	Pointer to the encoder iterator.
 * @param success If true - successfully complete the aggregate, if false - remove the aggregate from the buffer.
 * @see RsslArray, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RsslRet rsslEncodeArrayComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Perform array item encoding (item can only be simple primitive type such as \ref RsslInt, RsslReal, or RsslDate and not another RsslArray or container type)
 * 
 * Encodes entries in an RsslArray.<BR>
 * Typical use:<BR>
 *	1. Call rsslEncodeArrayInit()<BR>
 *	2. Call rsslEncodeArrayEntry() for each item in the array<BR>
 *	3. Call rsslEncodeArrayComplete()<BR>
 * 
 * @note Only one of pEncBuffer or pData should be supplied.
 * 
 * @note If specifying RsslArray::itemLength and an RsslArray::primitiveType of ::RSSL_DT_ASCII_STRING, ::RSSL_DT_BUFFER, ::RSSL_DT_RMTES_STRING, or ::RSSL_DT_UTF8_STRING, the length of the buffer should
 * be less than or equal to RsslArray::itemLength.  If content is longer, it will be truncated.  
 * @param pIter encode iterator
 * @param pEncData A pointer to pre-encoded data, if the user wishes to copy pre-encoded data.
 * @param pData A pointer to the primitive, if the user wishes to encode it.
 * @see RsslArray, RsslBuffer, RsslEncodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RsslRet rsslEncodeArrayEntry(RsslEncodeIterator *pIter, const RsslBuffer *pEncData, const void *pData)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decodes an RsslArray primitive type
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeArray()<BR>
 *  2. Call rsslDecodeArrayEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 * 
 * @param pIter Decode iterator to use for decode process
 * @param pArray RsslArray structure to populate with decoded contents.  RsslArray::encData will point to encoded array content.
 * @see RsslArray, RsslDecodeIterator, rsslDecodeArrayEntry
 * @return Returns an RsslRet to provide success or failure information
 */
RsslRet rsslDecodeArray(RsslDecodeIterator	*pIter, RsslArray *pArray)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decodes and returns a single RsslArray entry from within RsslArray::encEntries
 * 
  * Typical use:<BR>
 *  1. Call rsslDecodeArray()<BR>
 *  2. Call rsslDecodeArrayEntry until error or ::RSSL_RET_END_OF_CONTAINER is returned.<BR>
 *
 * @param pIter Decode iterator to use for decode process
 * @param pBuffer RsslBuffer to decode content into.  User can continue to decode into primitive by calling appropriate primitive type decoder, as indicated by RsslArray::primitiveType
 * @see RsslArray, rsslDecodeArray, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RsslRet rsslDecodeArrayEntry(RsslDecodeIterator *pIter, RsslBuffer *pBuffer)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Adds information from a field dictionary file to the data dictionary object.
 * Subsequent calls to this function may be made to the same RsslDataDictionary to load additional dictionaries(provided the fields do not conflict).
 * @see RsslDataDictionary
 */
RsslRet rsslLoadFieldDictionary(const char *filename, RsslDataDictionary *dictionary, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Fully dumps information contained in a data dictionary object to a file.
 * @see  RsslDataDictionary
 */
RsslRet rsslPrintDataDictionary(FILE *fileptr, RsslDataDictionary *dictionary)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Uninitializes a data dictionary and frees all memory associated with it.
 * @see RsslDataDictionary
 */
RsslRet rsslDeleteDataDictionary(RsslDataDictionary *dictionary)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Encode the field definitions dictionary information into a data payload according the domain model, using the field information from the entries present in this dictionary.
 * This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary, 
 *  subsequent calls can be made to this function, each producing the next segment of fields.
 * @param eIter Iterator to be used for encoding. Prior to each call, the iterator must be cleared and initialized to the buffer to be used for encoding.
 * @param dictionary The dictionary to encode field information from.
 * @param currentFid Tracks which fields have been encoded in case of multi-part encoding. Must be initialized to dictionary->minFid on the first call and is updated with each successfully encoded part.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @return RSSL_RET_DICT_PART_ENCODED when encoding parts, RSSL_RET_SUCCESS for final part or single complete payload.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslDecodeFieldDictionary
 */
RsslRet rsslEncodeFieldDictionary(RsslEncodeIterator *eIter, RsslDataDictionary *dictionary, int *currentFid, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode the field dictionary information contained in a data payload according to the domain model.
 * This function may be called multiple times on the same dictionary, to load information from dictionaries that have been encoded in multiple parts.
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded field information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeFieldDictionary
 */
RsslRet rsslDecodeFieldDictionary(RsslDecodeIterator *dIter, RsslDataDictionary *dictionary, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Adds information from an enumerated types dictionary file to the data dictionary object.
 * Subsequent calls to this function may be made to the same RsslDataDictionary to load additional dictionaries(provided that there are no duplicate table references for any field)
 * @see RsslDataDictionary
 */
RsslRet rsslLoadEnumTypeDictionary(const char *filename, RsslDataDictionary *dictionary, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Encode the enumerated types dictionary according the domain model, using the information from the tables and referencing fields present in this dictionary.
 * Note: This function will use the type RSSL_DT_ASCII_STRING for the DISPLAY array.
 * @param eIter Iterator to be used for encoding.
 * @param dictionary The dictionary to encode enumerated type information from.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionaryAsMultiPart, rsslDecodeEnumTypeDictionary
 */

RsslRet rsslEncodeEnumTypeDictionary(RsslEncodeIterator *eIter, RsslDataDictionary *dictionary, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Encode the enumerated types dictionary according the domain model, using the information from the tables and referencing fields present in this dictionary.
 * This function supports building the encoded data in multiple parts -- if there is not enough available buffer space to encode the entire dictionary, 
 *  subsequent calls can be made to this function, each producing the next segment of fields.
 * Note: This function will use the type RSSL_DT_ASCII_STRING for the DISPLAY array.
 * @param eIter Iterator to be used for encoding.
 * @param dictionary The dictionary to encode enumerated type information from.
 * @param currentFid Tracks which fields have been encoded in case of multi-part encoding. Must be initialized to 0 on the first call and is updated with each successfully encoded part.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionary, rsslDecodeEnumTypeDictionary
 */

RsslRet rsslEncodeEnumTypeDictionaryAsMultiPart(RsslEncodeIterator *eIter, RsslDataDictionary *dictionary, int *currentFid, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode the enumerated types information contained in an encoded enum types dictionary according to the domain model.
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded enumerated types information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionary, rsslEncodeEnumTypeDictionaryAsMultiPart
 */
RsslRet rsslDecodeEnumTypeDictionary(RsslDecodeIterator *dIter, RsslDataDictionary *dictionary, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Extract dictionary type from the encoded payload of an RSSL message where the domain type is RSSL_DMT_DICTIONARY
 * Typical use:<BR>
 * 1. Call rsslDecodeMsg().<BR>
 * 2. If domainType is RSSL_DMT_DICTIONARY, call this function.
 * 3. Call appropriate dictionary decode function based on returned dictionary type (e.g., if returned type is RDM_DICTIONARY_FIELD_DEFINITIONS, call rsslDecodeFieldDictionary()).<BR>
 * @param dIter An iterator to use. Must be set to the encoded payload of the dictionary message.
 * @param dictionaryType The dictionary type, from RDMDictionaryTypes in rsslRDM.h.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @returns RsslRet, if success dictionary type is populated.  If failure, dictionary type not available.
 * @see RsslDataDictionary, RDMDictionaryTypes
 */
RsslRet rsslExtractDictionaryType(RsslDecodeIterator *dIter, RDMDictionaryTypes *dictionaryType, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

void rsslQueryDataLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	return;
}

/**
 * @brief Provide string representation for a data type enumeration
 * @see RsslDataType, RsslDataTypes
 */
const char* rsslDataTypeToString(RsslDataType type)
{
	return NULL;
}

/**
 * @brief Returns maximum encoded size for primitiveTypes
 */
/**
 * @brief Checks if the passed in type is a valid primitive type (0 - 128)
 *
 * @param dataType RsslDataTypes enumerated value to return encoded size for
 * @see RsslDataTypes, RsslDataType
 * @return \ref RsslUInt32 containing the maximum encoded length when this can be determined.  255 (0xFF) is returned for types that contain buffer content and the length varies based on the \ref RsslBuffer::length.  0 is returned for invalid types.  
 */
RsslUInt32 rsslPrimitiveTypeSize(const RsslDataType dataType)
{
	return RSSL_RET_FAILURE;
}

/* @brief Translates certain RSSL type enumerations to their respective basic primitive type.
 * Useful mainly for set types, e.g. UINT_1 returns UINT.
 * See RsslDataType
 */
RsslDataType rsslPrimitiveBaseType(RsslDataType dataType)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Begin encoding of a Non-RWF type (e.g. AnsiPage, Opaque, XML, etc)
 * This allows a user to obtain a starting position to encode from as well as 
 * length of data left in the buffer to encode into.  User can then leverage
 * non-RWF encoding functions/APIs or memcpy routines to nest non-RWF data 
 * into the buffer.  
 * Typical use:<BR>
 *	1. Use your rssl Init function (e.g. rsslEncodeMsgInit, rsslEncodeMapEntryInit) for the container you intend to nest non-RWF data into<BR>
 *  2. Call rsslEncodeNonRWFDataTypeInit with the iterator being used for encoding and a RsslBuffer structure.<BR>
 *  3. rsslEncodeNonRWFDataTypeInit will populate the pBuffer->data with the address where encoding should begin and the pBuffer->length with the amount of space left to encode into<BR>
 *	4. When NonRWF encoding is complete, set pBuffer->length to actual number of bytes encoded and pass pIter and pBuffer to rsslEncodeNonRWFDataTypeComplete<BR>
 *  5. Finish RWF container encoding (e.g. message, map entry, etc).  <BR>
 *  Note: User should ensure not to encode or copy more than pBuffer->length indicates is available <BR>
 * @param pIter RsslEncodeIterator used for encoding container
 * @param pBuffer RsslBuffer to populate with pointer to encode to and available length to encode into
 */
RsslRet rsslEncodeNonRWFDataTypeInit(RsslEncodeIterator *pIter, RsslBuffer *pBuffer)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Complete encoding of a Non-RWF type (e.g. AnsiPage, Opaque, XML, etc)
 * This allows a user to complete encoding of a Non-RWF type.  User must set
 * buffer.length to the amount of data encoded or copied into the buffer.data.  
 * Typical use:<BR>
 *	1. Use your rssl Init function (e.g. rsslEncodeMsgInit, rsslEncodeMapEntryInit) for the container you intend to nest non-RWF data into<BR>
 *  2. Call rsslEncodeNonRWFDataTypeInit with the iterator being used for encoding and a RsslBuffer structure.<BR>
 *  3. rsslEncodeNonRWFDataTypeInit will populate the pBuffer->data with the address where encoding should begin and the pBuffer->length with the amount of space left to encode into<BR>
 *	4. When NonRWF encoding is complete, set pBuffer->length to actual number of bytes encoded and pass pIter and pBuffer to rsslEncodeNonRWFDataTypeComplete<BR>
 *  5. Finish RWF container encoding (e.g. message, map entry, etc).  <BR>
 *  Note: User should ensure not to encode or copy more than pBuffer->length indicates is available <BR>
 * @param pIter RsslEncodeIterator used for encoding container
 * @param pBuffer RsslBuffer obtained from rsslEncodeNonRWFDataTypeInit.  pBuffer->length should be set to number of bytes put into pBuffer->data.  pBuffer->data must remain unchanged.  
 * @param success if RSSL_TRUE, encoding was successful and RSSL will complete transfer of data.  If RSSL_FALSE, RSSL will roll back to last successful encoding prior to Non-RWF Init call.
 */
RsslRet rsslEncodeNonRWFDataTypeComplete(RsslEncodeIterator *pIter, RsslBuffer *pBuffer, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

 /**
 * @brief Verifies that day, month, and year information is valid (e.g. values that correspond to actual month or day information)
 * @param iDate RsslDate to check for validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslDate
 */
RsslBool rsslDateIsValid(const RsslDate * iDate)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Converts string date in strftime()'s "%d %b %Y" format (e.g. 01 JUN 2003) or "%m/%d/%y" to RsslDate
 * @param oDate RsslDate structure to populate from string
 * @param iDateString RsslBuffer containing an appropriately formatted string to convert from.  RsslBuffer::data should point to date string, RsslBuffer::length should indicate the number of bytes pointed to.
 * @return RsslRet RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE otherwise  
 * @see RsslDate, RsslBuffer
 */
RsslRet rsslDateStringToDate(RsslDate * oDate, const RsslBuffer * iDateString)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Verifies that hour, minute, second, and millisecond information is valid (e.g. values that correspond to actual time information).  Allows seconds to be set up to 60 to account for periodic leap seconds.
 * @param iTime RsslTime to check validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslTime
 */
RsslBool rsslTimeIsValid(const RsslTime * iTime)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Converts string time in strftime()'s "%H:%M" or "%H:%M:%S" format (e.g. 13:01 or 15:23:54) to RsslTime
 * @param oTime RsslTime structure to have populated from string
 * @param iTimeString RsslBuffer containing an appropriately formatted string to convert from.  RsslBuffer::data should point to time string, RsslBuffer::length should indicate the number of bytes pointed to.
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslTime, RsslBuffer
 */
RsslRet rsslTimeStringToTime(RsslTime * oTime, const RsslBuffer * iTimeString)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Verifies that day, month, year, hour, minute, second, and millisecond information is valid (e.g. values that correspond to actual date and time information)
 * @param iDateTime RsslDateTime to check validity
 * @return RsslBool RSSL_TRUE if validly populated, RSSL_FALSE otherwise
 * @see RsslDateTime
 */
RsslBool rsslDateTimeIsValid(const RsslDateTime * iDateTime)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Converts DateTime string to RsslDateTime, expects date before time, where date is formatted in in strftime()'s "%d %b %Y" format (e.g. 01 JUN 2003) or "%m/%d/%y" and time is formatted in strftime()'s "%H:%M" or "%H:%M:%S" format (e.g. 13:01 or 15:23:54).
 * @param oDateTime RsslDateTime structure to populate from string
 * @param iDateTimeString RsslBuffer containing an appropriately formatted string to convert from
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslDateTime, RsslBuffer
 */
RsslRet rsslDateTimeStringToDateTime(RsslDateTime *oDateTime, const RsslBuffer *iDateTimeString)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Converts a populated RsslDateTime structure to a string
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param dataType Either RSSL_DT_DATE, RSSL_DT_TIME or RSSL_DT_DATETIME depending on which portion(s) to convert to string
 * @param iDateTime RsslDateTime structure to convert to string
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 */
RsslRet rsslDateTimeToString(RsslBuffer * oBuffer, RsslUInt8 dataType, RsslDateTime * iDateTime)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Set the given RsslDateTime to now/current time and date in the local time zone.
 * @param oDateTime RsslDateTime structure to populate with local time
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise  
 * @see RsslDateTime
 */
RsslRet rsslDateTimeLocalTime(RsslDateTime * oDateTime)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Set the given RsslDateTime to now/current time and date in GMT.
 * @param oDateTime RsslDateTime structure to populate with GMT time
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise  
 * @see RsslDateTime
 */
RsslRet rsslDateTimeGmtTime(RsslDateTime * oDateTime)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeElementListInit(RsslEncodeIterator *pIter, RsslElementList *pElementList, const RsslLocalElementSetDefDb	*pSetDb, RsslUInt16 setEncodingMaxSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeElementListComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeElementEntry(RsslEncodeIterator	*pIter, RsslElementEntry *pElement, const void *pData)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeElementEntryInit(RsslEncodeIterator *pIter, RsslElementEntry	*pElement, RsslUInt16 encodingMaxSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeElementEntryComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeElementList(RsslDecodeIterator *pIter, RsslElementList *pElementList, const RsslLocalElementSetDefDb	*pLocalSetDb)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeElementEntry(RsslDecodeIterator *pIter, RsslElementEntry *pElement)
{
	return RSSL_RET_FAILURE;
}

/** @brief Encode ElementlistList set definitions database
 * @param pIter encode iterator
 * @param pElementListSetDb database to encode
 */
RsslRet rsslEncodeLocalElementSetDefDb(RsslEncodeIterator *pIter, RsslLocalElementSetDefDb *pElementListSetDb)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeLocalElementSetDefDb(RsslDecodeIterator *pIter, RsslLocalElementSetDefDb	*pLocalSetDb)
{
	return RSSL_RET_FAILURE;
}
			
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
RsslRet rsslEncodeElementSetDefDictionary(RsslEncodeIterator *eIter, RsslElementSetDefDb *dictionary, int *currentSetDef, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeElementSetDefDictionary(RsslDecodeIterator *dIter, RsslElementSetDefDb *dictionary, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFieldListInit(RsslEncodeIterator *pIter, RsslFieldList *pFieldList, const RsslLocalFieldSetDefDb	*pSetDb, RsslUInt16 setEncodingMaxSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFieldListComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFieldEntry(RsslEncodeIterator *pIter, RsslFieldEntry *pField, const void	*pData)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFieldEntryInit(RsslEncodeIterator *pIter, RsslFieldEntry *pField, RsslUInt16 encodingMaxSize)
{
	return RSSL_RET_FAILURE;
}
				
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
RsslRet rsslEncodeFieldEntryComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeFieldList(RsslDecodeIterator *pIter, RsslFieldList *pFieldList, RsslLocalFieldSetDefDb *pLocalSetDb)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeFieldEntry(RsslDecodeIterator *pIter, RsslFieldEntry *pField)
{
	return RSSL_RET_FAILURE;
}

/** @brief Encode FieldList set definitions database
 * @param pIter encode iterator
 * @param pFieldListSetDb database to encode
 */
RsslRet rsslEncodeLocalFieldSetDefDb(RsslEncodeIterator *pIter, RsslLocalFieldSetDefDb	*pFieldListSetDb)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeLocalFieldSetDefDb(RsslDecodeIterator *pIter, RsslLocalFieldSetDefDb	*pLocalSetDb)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFieldSetDefDictionary(RsslEncodeIterator *eIter, RsslFieldSetDefDb *dictionary, int *currentSetDef, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeFieldSetDefDictionary(RsslDecodeIterator *dIter, RsslFieldSetDefDb *dictionary, RDMDictionaryVerbosityValues verbosity, RsslBuffer *errorText)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFilterListInit(RsslEncodeIterator *pIter, RsslFilterList *pFilterList)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFilterListComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFilterEntry(RsslEncodeIterator *pIter, RsslFilterEntry *pFilterEntry)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFilterEntryInit(RsslEncodeIterator *pIter, RsslFilterEntry *pFilterEntry, RsslUInt16 maxEncodingSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeFilterEntryComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeFilterList(RsslDecodeIterator *pIter, RsslFilterList *pFilterList)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeFilterEntry(RsslDecodeIterator *pIter, RsslFilterEntry *pFilterEntry)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to associate \ref RsslEncodeIterator with a new, larger buffer when encoding requires more space (e.g. ::RSSL_RET_BUFFER_TOO_SMALL).
 * 
 * Typical use:<BR>
 * 1. Call rsslRealignEncodeIteratorBuffer() with the current \ref RsslEncodeIterator and the new larger buffer to complete encoding into.<BR>
 * 2. Finish encoding using the new buffer using the same iterator you were using before.
 *
 * @note The user must pass in a newly allocated buffer, and the function does not deallocate the previous buffer.  Content from previous buffer will be copied into the new buffer so encoding can continue from where it left off.
 * @param pIter	Pointer to the \ref RsslEncodeIterator to associate new buffer with.
 * @param pNewEncodeBuffer Pointer to the larger \ref RsslBuffer to continue encoding into. 
 * @see RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_SUCCESS if new \ref RsslBuffer is successfully associated with iterator, ::RSSL_RET_FAILURE otherwise, typically due to new buffer not being sufficiently populated with RsslBuffer::data and RsslBuffer::length
 */
RsslRet rsslRealignEncodeIteratorBuffer(RsslEncodeIterator	*pIter, RsslBuffer *pNewEncodeBuffer)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapInit(RsslEncodeIterator *pIter, RsslMap *pMap, RsslUInt16 summaryMaxSize, RsslUInt16 setMaxSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapSetDefsComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapSummaryDataComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapEntry(RsslEncodeIterator *pIter, RsslMapEntry *pMapEntry, const void *pKeyData)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapEntryInit(RsslEncodeIterator *pIter, RsslMapEntry	*pMapEntry,	const void *pKeyData, RsslUInt16 maxEncodingSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMapEntryComplete(RsslEncodeIterator *pIter, RsslBool	success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeMap(RsslDecodeIterator *pIter, RsslMap *pMap)
{
	return RSSL_RET_FAILURE;
}		

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
RsslRet rsslDecodeMapEntry(RsslDecodeIterator *pIter, RsslMapEntry *pMapEntry, void *pKeyData)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Encoded Primitive Data To String
 * @param pIter RsslDecodeIterator with buffer to decode and convert from. Iterator should also have appropriate version information set
 * @param primitiveType type of input buffer
 * @param oBuffer Output Buffer to put string into
 */
RsslRet rsslEncodedPrimitiveToString(RsslDecodeIterator *pIter, RsslPrimitiveType primitiveType, RsslBuffer *oBuffer)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Primitive To String
 * @param pType the primitive object to convert to a string.
 * @param primitiveType type of the object
 * @param oBuffer Output Buffer to put string into
 */
RsslRet rsslPrimitiveToString(void *pType, RsslPrimitiveType primitiveType, RsslBuffer *oBuffer)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to decode any primitive type (e.g. \ref RsslInt, RsslReal, RsslDate) from buffer referred to by \ref RsslDecodeIterator.  This function cannot be used for RsslArray decoding.
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information. * @param primitiveType primitive type contained in iterator
 * @param primitiveType RsslPrimitiveType enumeration corresponding to the primitive type to decode (e.g. ::RSSL_DT_REAL)
 * @param oValue void pointer to primitive type representation to decode into (e.g. \ref RsslInt, RsslReal)
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and primitive type is blank value, , ::RSSL_RET_FAILURE if invalid parameter or invalid primitive type is passed
 */
RsslRet rsslDecodePrimitiveType( RsslDecodeIterator *pIter, RsslPrimitiveType primitiveType, void *oValue)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslInt
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslInt primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RsslRet rsslDecodeInt(RsslDecodeIterator *pIter, RsslInt *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslUInt
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslUInt primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RsslRet rsslDecodeUInt(RsslDecodeIterator *pIter, RsslUInt *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslFloat
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslFloat primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RsslRet rsslDecodeFloat(RsslDecodeIterator *pIter, RsslFloat *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslDouble
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value The \ref RsslDouble primitive type to put decoded data into
 * @return ::RSSL_RET_SUCCESS if decoding success, ::RSSL_RET_INCOMPLETE_DATA if decoding failure, ::RSSL_RET_BLANK_DATA if decoding success and \ref RsslInt is blank value
 */
RsslRet rsslDecodeDouble(RsslDecodeIterator *pIter, RsslDouble *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslEnum 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslEnum to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RsslRet rsslDecodeEnum(RsslDecodeIterator *pIter, RsslEnum *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslBuffer 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslBuffer to decode content into.  \ref RsslBuffer::data will point to contents in iterator's buffer and \ref RsslBuffer::length will indicate length of content.
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RsslRet rsslDecodeBuffer(RsslDecodeIterator *pIter, RsslBuffer *value)
{
	return RSSL_RET_FAILURE;
}		

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslReal
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslReal to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RsslRet rsslDecodeReal(RsslDecodeIterator *pIter, RsslReal *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslQos
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslQos to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RsslRet rsslDecodeQos(RsslDecodeIterator *pIter, RsslQos *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an RsslState
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value RsslState to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank/zero length.
 */
RsslRet rsslDecodeState(RsslDecodeIterator *pIter, RsslState *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslDate 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslDate to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RsslRet rsslDecodeDate(RsslDecodeIterator *pIter, RsslDate *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode \ref RsslDecodeIterator buffer's contents into an \ref RsslTime 
 * @param pIter The \ref RsslDecodeIterator that has been populated with a buffer to decode from, along with appropriate version information.
 * @param value \ref RsslTime to decode content into.  
 * @return ::RSSL_RET_SUCCESS if success, ::RSSL_RET_INCOMPLETE_DATA if failure, ::RSSL_RET_BLANK_DATA if data is blank value
 */
RsslRet rsslDecodeTime(RsslDecodeIterator *pIter, RsslTime *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Decode DateTime 
 * @param pIter RsslDecodeIterator with buffer to decode from and appropriate version information set
 * @param value dataType to put decoded data into
 * @return RSSL_RET_SUCCESS if success, RSSL_RET_INCOMPLETE_DATA if failure, RSSL_RET_BLANK_DATA if data is blank value
 */
RsslRet rsslDecodeDateTime(RsslDecodeIterator *pIter, RsslDateTime *value)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Used to encode any primitive type (e.g. \ref RsslInt, RsslReal, RsslDate) into buffer referred to by \ref RsslEncodeIterator.  This function cannot be used for RsslArray encoding.
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param primitiveType RsslPrimitiveType enumeration corresponding to the primitive type to encode (e.g. ::RSSL_DT_REAL)
 * @param pData void pointer to primitive type representation to encode from (e.g. \ref RsslInt, RsslReal)
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if contents exceed length of encode buffer referred to by \ref RsslEncodeIterator, ::RSSL_RET_FAILURE if invalid parameter or invalid primitive type is passed
 */
RsslRet rsslEncodePrimitiveType(RsslEncodeIterator *pIter, RsslPrimitiveType primitiveType, const void	*pData)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an  \ref RsslInt into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pInt The \ref RsslInt value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslInt contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeInt(RsslEncodeIterator *pIter, const RsslInt *pInt)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an  \ref RsslUInt into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pUInt The \ref RsslUInt value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslUInt contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeUInt(RsslEncodeIterator *pIter, const RsslUInt *pUInt)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an \ref RsslFloat into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pFloat The \ref RsslFloat value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslFloat contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeFloat(RsslEncodeIterator *pIter, const RsslFloat *pFloat)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an \ref RsslDouble into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pDouble The \ref RsslDouble value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslDouble contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeDouble(RsslEncodeIterator *pIter, const RsslDouble *pDouble)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an \ref RsslEnum into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pEnum The \ref RsslEnum to encode.
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslEnum contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeEnum(RsslEncodeIterator *pIter, const RsslEnum *pEnum)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an \ref RsslBuffer into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pBuffer The \ref RsslBuffer to encode, where \ref RsslBuffer::data points to the content and \ref RsslBuffer::length indicates the length of content.
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslBuffer contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeBuffer(RsslEncodeIterator *pIter, const RsslBuffer *pBuffer)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an RsslReal into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pReal The RsslReal value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslReal contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeReal(RsslEncodeIterator *pIter, const RsslReal *pReal)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an RsslQos into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pQos The RsslQos value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslQos contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeQos(RsslEncodeIterator *pIter, const RsslQos *pQos)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an RsslState into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pState The RsslState value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if \ref RsslState contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeState(RsslEncodeIterator *pIter, const RsslState *pState)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an RsslDate into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pDate The RsslDate value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if RsslDate contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeDate(RsslEncodeIterator *pIter, const RsslDate *pDate)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode an RsslTime into buffer referred to by \ref RsslEncodeIterator 
 * @param pIter The \ref RsslEncodeIterator that has been populated with a buffer to encode into, along with appropriate version information.
 * @param pTime The RsslTime value to encode
 * @return ::RSSL_RET_SUCCESS for successful encoding, ::RSSL_RET_BUFFER_TOO_SMALL if RsslTime contents exceed length of encode buffer referred to by \ref RsslEncodeIterator
 */
RsslRet rsslEncodeTime(RsslEncodeIterator *pIter, const RsslTime *pTime)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Used to encode a datetime into a buffer 
 * @param pIter RsslEncodeIterator with buffer to encode into. Iterator should also have appropriate version information set
 * @param pDateTime datetime value to encode
 */
RsslRet rsslEncodeDateTime(RsslEncodeIterator *pIter, const RsslDateTime *pDateTime)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Checks if the new QoS is better than the old QoS, where real-time is more desirable than delayed, tick-by-tick is more desirable than conflated.
 * @param newQos Pointer to the RsslQos representing the new quality of service.
 * @param oldQos Pointer to the RsslQos representing the old quality of service.
 * @return RsslBool RSSL_TRUE if newQos is better, RSSL_FALSE otherwise
 * @see RsslQos
 */
RsslBool rsslQosIsBetter(const RsslQos *newQos, const RsslQos *oldQos)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Checks if the QoS is in the range defined using the best and worst QoS.
 * @param bestQos Pointer to the RsslQos representing the best QoS within the desired range
 * @param worstQos Pointer to the RsslQos representing the worst QoS within the desired range
 * @param Qos Pointer to the RsslQos to check against the specified QoS range
 * @return RsslBool RSSL_TRUE if Qos is within the range, RSSL_FALSE otherwise
 * @see RsslQos
 */
RsslBool rsslQosIsInRange(const RsslQos *bestQos, const RsslQos *worstQos, const RsslQos *Qos)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert double to a RsslReal
 * @param oReal RsslReal to populate with hint and value from double
 * @param iValue double to convert to RsslReal
 * @param iHint \ref RsslRealHints enumeration hint value to use for converting double
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion occurs; ::RSSL_RET_FAILURE if unable to convert, typically due to invalie hint value
 */
RsslRet rsslDoubleToReal(RsslReal * oReal, RsslDouble * iValue, RsslUInt8 iHint)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert float to a RsslReal 
 * @param oReal RsslReal to populate with hint and value from float
 * @param iValue float to convert to RsslReal
 * @param iHint \ref RsslRealHints enumeration hint value to use for converting float
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion; ::RSSL_RET_FAILURE if unable to convert, typically due to invalid hint value
 */
RsslRet rsslFloatToReal(RsslReal * oReal, RsslFloat * iValue, RsslUInt8 iHint)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert RsslReal to a double
 * @param oValue double to convert into
 * @param iReal RsslReal to convert to double
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion; ::RSSL_RET_FAILURE if unable to convert, typically because passed in RsslReal is blank or due to invalid hint value
 */
RsslRet rsslRealToDouble(RsslDouble * oValue, RsslReal * iReal)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert numeric string to double
 * @param oValue double to convert into
 * @param iNumericString \ref RsslBuffer to convert to double, where \ref RsslBuffer::data points to memory populated with a numeric string and  \ref RsslBuffer::length indicates number of bytes pointed to
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid 
 */
RsslRet rsslNumericStringToDouble(RsslDouble * oValue, RsslBuffer * iNumericString)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Converts a numeric string to an RsslReal, similar to atof, but with support for fractions and decimals.  No null termination required.
 * @param oReal RsslReal to convert string into
 * @param iNumericString Populated \ref RsslBuffer to convert into RsslReal, where \ref RsslBuffer::data points to memory populated with a numeric string and \ref RsslBuffer::length indicates number of bytes pointed to
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid
 */
RsslRet rsslNumericStringToReal(RsslReal * oReal, RsslBuffer * iNumericString)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert RsslReal to a numeric string
 * @param buffer \ref RsslBuffer to use for string representation. \ref RsslBuffer::data should point to some amount of memory and \ref RsslBuffer::length specifies size of memory.
 * @param iReal RsslReal to convert into a numeric string
 * @return Returns ::RSSL_RET_SUCCESS if successful conversion;  ::RSSL_RET_INVALID_DATA if numeric string is invalid; ::RSSL_RET_BUFFER_TOO_SMALL if passed in buffer does not have sufficient length for conversion
 */
RsslRet rsslRealToString(RsslBuffer * buffer, RsslReal * iReal)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Convert an RsslReturnCodes return code enumerated value to a string representation (e.g. 16 to "RSSL_RET_NO_DATA")
 *
 * @param code RsslReturnCodes value to convert to string representation
 * @see RsslRet, RsslReturnCodes
 * @return char* containing string representation of RsslReturnCodes enumerated value
 */
const char* rsslRetCodeToString(RsslRet code)
{
	return NULL;
}

/**
 * @brief Provide additional information about the meaning of an RsslReturnCodes return code enumerated value.
 *
 * @param code RsslReturnCodes value to obtain extended error information for
 * @see RsslRet, RsslReturnCodes
 * @return char* containing extended error description
 */
const char* rsslRetCodeInfo(RsslRet code)
{
	return NULL;
}

/**
 *	@brief Applies the buffer to cache.  Also parses any marketfeed commands, including partial update and repeat commands.
 *
 *	Typical use:<BR>
 *	1.  Allocate memory for the cache buffer.<BR>
 *	2.  After decoding the payload buffer, call rsslRMTESApplyToCache to copy the data to the RsslRmtesCacheBuffer.<BR>
 *	3.  Call appropriate unicode decoding function to display the RMTES data.<BR>
 *
 *	@param inBuffer Input rsslBuffer that contains RMTES data
 *	@param RsslRmtesCacheBuffer Cache buffer, this is populated after the function is complete
 *	@return Returns an RsslRet code to provide success or failure information.
 */
RsslRet rsslRMTESApplyToCache(RsslBuffer *inBuffer, RsslRmtesCacheBuffer *cacheBuf)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Converts the given cache to UTF8 Unicode
 *
 *	Typical use:<BR>
 *	1.  Allocate memory for the cache buffer.<BR>
 *	2.  After decoding the payload buffer, call rsslRMTESApplyToCache to copy the data to the RsslRmtesCacheBuffer.<BR>
 *	3.	Allocate memory for the unicode string.<BR>
 *	3.  Call rsslRMTESToUTF8 to convert the RMTES data for display or parsing.<BR>
 *
 *	@param pRmtesBuffer Input RMTES cache buffer that contains RMTES data
 *	@param charBuffer character buffer, populated after the function is complete
 *	@return Returns an RsslRet code to provide success or failure information.
 */
RsslRet rsslRMTESToUTF8(RsslRmtesCacheBuffer *pRmtesBuffer, RsslBuffer *charBuffer)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Converts the given cache to UCS2 Unicode
 *
 *	Typical use:<BR>
 *	1.  Allocate memory for the cache buffer.<BR>
 *	2.  After decoding the payload buffer, call rsslRMTESApplyToCache to copy the data to the RsslRmtesCacheBuffer.<BR>
 *	3.	Allocate memory for the unicode string.<BR>
 *	3.  Call rsslRMTESToUCS2 to convert the RMTES data for display or parsing.<BR>
 *
 *	@param pRmtesBuffer Input RMTES cache buffer that contains RMTES data
 *	@param shortBuffer unsigned 16-bit integer buffer, populated after the function is complete
 *	@return Returns an RsslRet code to provide success or failure information.
 */
RsslRet rsslRMTESToUCS2(RsslRmtesCacheBuffer *pRmtesBuffer, RsslU16Buffer *shortBuffer)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Checks the presence of partial update commands in the buffer.
 *	
 *	@param pBuffer Pointer to the RsslBuffer to check
 *	@return RSSL_TRUE if a partial update command is present; RSSL_FALSE if not.
 */
RsslBool rsslHasPartialRMTESUpdate(RsslBuffer *pBuffer)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesInit(RsslEncodeIterator *pIter, RsslSeries *pSeries, RsslUInt16 summaryMaxSize, RsslUInt16 setMaxSize)
{
	return RSSL_RET_FAILURE;
}       

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
RsslRet rsslEncodeSeriesSetDefsComplete(RsslEncodeIterator	*pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesSummaryDataComplete(RsslEncodeIterator	*pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesEntry(RsslEncodeIterator *pIter, RsslSeriesEntry *pSeriesEntry)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesEntryInit(RsslEncodeIterator *pIter, RsslSeriesEntry *pSeriesEntry, RsslUInt16 maxEncodingSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeSeriesEntryComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeSeries(RsslDecodeIterator *pIter, RsslSeries *pSeries)
{
	return RSSL_RET_FAILURE;
}		  

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
RsslRet rsslDecodeSeriesEntry(RsslDecodeIterator *pIter, RsslSeriesEntry *pSeriesEntry)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Allocates the definitions pointer array of the provided RsslFieldSetDefDb, and deep copies the version string into the structure.
 */
RsslRet rsslAllocateFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslBuffer* version)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Deep copies the given RsslFieldSetDef into the RsslFieldSetDefDb's definitions array.
 */
RsslRet rsslAddFieldSetDefToDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslFieldSetDef* pFieldSetDef)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Deletes and frees all memory allocated by rsslAllocateFieldSetDefDb and rsslAddFieldSetDefToDb.
 *
 *	@note If the memory used by the RsslFieldSetDefDb has been created and managed outside of rsslAllocateFieldSetDefDb and rsslAddFieldSetDefToDb, this function should not be used.
 *
 */
RsslRet rsslDeleteFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Allocates the definitions pointer array of the provided RsslElementSetDefDb, and deep copies the version string into the structure.
 */
RsslRet rsslAllocateElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb, RsslBuffer* version)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Deep copies the given RsslFieldSetDef into the RsslElementSetDefDb's definitions array.
 */
RsslRet rsslDeleteElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb)
{
	return RSSL_RET_FAILURE;
}

/**
 *	@brief Deletes and frees all memory allocated by rsslAllocateElementSetDefDb and rsslAddElementSetDefToDb.
 *
 *	@note If the memory used by the RsslFieldSetDefDb has been created and managed outside of rsslAllocateElementSetDefDb and rsslAddElementSetDefToDb, this function should not be used.
 *
 */
RsslRet rsslAddElementSetDefToDb(RsslElementSetDefDb* pElementSetDefDb, RsslElementSetDef* pElementSetDef)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Provide string representation for an RsslState, including all RsslState members
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param pState Fully populated RsslState structure
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_FAILURE otherwise
 * @see RsslState
 */
RsslRet rsslStateToString(RsslBuffer *oBuffer, RsslState *pState)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Provide string representation for an RsslState::code
 * @param value \ref RsslStateCodes enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
const char* rsslStateCodeToString(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string description for an RsslState::code
 * @param value \ref RsslStateCodes enumeration to provide description for
 * @return const char* description of corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
const char* rsslStateCodeDescription(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string representation for an RsslState::streamState
 * @param value \ref RsslStreamStates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslStreamStates
 * @see RsslStreamStates, RsslState
 */
const char* rsslStreamStateToString(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string representation for an RsslState::dataState
 * @param value \ref RsslDataStates enumeration to convert to string
 * @return const char* representation of corresponding \ref RsslDataStates
 * @see RsslDataStates, RsslState
 */
const char* rsslDataStateToString(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string representation of meaning associated with RsslState::code
 * @param code \ref RsslStateCodes enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslStateCodes
 * @see RsslStateCodes, RsslState
 */
const char* rsslStateCodeInfo(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string representation of meaning associated with RsslState::streamState
 * @param code \ref RsslStreamStates enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslStreamStates
 * @see RsslStreamStates, RsslState
 */
const char* rsslStreamStateInfo(RsslUInt8 code)
{
	return NULL;
}

/**
 * @brief Provide string representation of meaning associated with RsslState::dataState
 * @param code \ref RsslDataStates enumeration to provide meaning as string
 * @return const char* representation of meaning for corresponding \ref RsslDataStates
 * @see RsslDataStates, RsslState
 */
const char* rsslDataStateInfo(RsslUInt8 code)
{
	return NULL;
}

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
RsslRet rsslEncodeVectorInit(RsslEncodeIterator *pIter, RsslVector *pVector, RsslUInt16 summaryMaxSize, RsslUInt16 setMaxSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorSetDefsComplete(RsslEncodeIterator	*pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorSummaryDataComplete(RsslEncodeIterator	*pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorEntry(RsslEncodeIterator *pIter, RsslVectorEntry *pVectorEntry)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorEntryInit(RsslEncodeIterator *pIter, RsslVectorEntry *pVectorEntry, RsslUInt16 maxEncodingSize)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeVectorEntryComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeVector(RsslDecodeIterator *pIter, RsslVector *pVector)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeVectorEntry(RsslDecodeIterator *pIter, RsslVectorEntry *pVectorEntry)
{
	return RSSL_RET_FAILURE;
}
