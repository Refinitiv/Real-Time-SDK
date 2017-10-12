
#ifndef __RSSL_DATA_DICTIONARY_H
#define __RSSL_DATA_DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslRDM.h"

/**
 * @addtogroup RSSLWFDict
 * @{
 */

/** 
 * @defgroup DataDict Data Dictionary
 * @{
 */


/** 
 * @brief Enumerated Marketfeed types
 * @see RsslStateValue, RsslState
 */
typedef enum {
	RSSL_MFEED_NONE					= -1,
	RSSL_MFEED_TIME_SECONDS			= 0, 
	RSSL_MFEED_INTEGER				= 1, 
	RSSL_MFEED_NUMERIC				= 2,
	RSSL_MFEED_DATE					= 3,
	RSSL_MFEED_PRICE				= 4,
	RSSL_MFEED_ALPHANUMERIC			= 5, 
	RSSL_MFEED_ENUMERATED			= 6,
	RSSL_MFEED_TIME					= 7,
	RSSL_MFEED_BINARY				= 8,
	RSSL_MFEED_LONG_ALPHANUMERIC	= 9,
	RSSL_MFEED_OPAQUE				= 10

} RsslMfFieldTypes;

/** 
 * @brief A single defined enumerated value.
 * @see RsslStateValue, RsslState
 */
typedef struct {
	RsslEnum	value;		/*!< The actual value representing the type. */
	RsslBuffer	display;	/*!< A brief string representation describing what the type means(For example, this may be an abbreviation of a currency to be displayed to a user) */
	RsslBuffer	meaning;	/*!< A more elaborate description of what the value means. This information is typically optional and not displayed. */
} RsslEnumType;

/** 
 * @brief A table of enumerated types.  A field that uses this table will contain a value corresponding to an enumerated type in this table.
 * @see RsslStateValue, RsslState
 */
typedef struct {
	RsslEnum		maxValue;				/*!< The highest value type present. This also indicates the length of the enumTypes list */
	RsslEnumType	**enumTypes;			/*!< The list of enumerated types. */
	RsslUInt32		fidReferenceCount;		/*!< The number of fields in the fidReferences list. */
	RsslFieldId		*fidReferences;			/*!< A list of fieldId's representing fields that reference this table */
} RsslEnumTypeTable;

/**
 * @brief A data dictionary entry, containing field information and an enumeration table reference if present.
 * @see RsslMfFieldTypes, RsslDataType
 */
typedef struct
{
	RsslBuffer	acronym;								/*!< Acronym */
	RsslBuffer	ddeAcronym;								/*!< DDE Acronym */
	RsslInt16	fid;									/*!< The fieldId the entry corresponds to */
	RsslInt16	rippleToField;							/*!< The field to ripple data to */
	RsslInt8	fieldType;								/*!< Marketfeed Field Type */
	RsslUInt16	length;									/*!< Marketfeed length */
	RsslUInt8	enumLength;								/*!< Marketfeed enum length */
	RsslUInt8	rwfType;								/*!< RWF type */
	RsslUInt16	rwfLength;								/*!< RWF Length */
	RsslEnumTypeTable *pEnumTypeTable;					/*!< A reference to the corresponding enumerated types table, if this field uses one. */
} RsslDictionaryEntry;

#define RSSL_MIN_FID -32768
#define RSSL_MAX_FID 32767

/**
 * @brief A data dictionary
 * Houses all known fields loaded from a field dictionary and their corresponding enum types loaded from an enum type dictionary.
 * The dictionary also saves general information about the dictionary itself -- this is found in the "!tag" comments in the file or in the summary data of dictionaries encoded via the official domain model.
 * @see RsslMfFieldTypes
 */
typedef struct
{
	RsslInt32			minFid;					/*!< The lowest fieldId present in the dictionary */
	RsslInt32			maxFid;					/*!< The highest fieldId present in the dictionary */
	RsslInt32			numberOfEntries;		/*!< The total number of entries in the dictionary */
	RsslDictionaryEntry	**entriesArray;			/*!< The list of entries, indexed by FieldId.  The full range of possible FieldId's may be looked up -- nonexistent fields are NULL. */
	RsslBool			isInitialized;			

	RsslEnumTypeTable**	enumTables;				/*!< The tables present in this dictionary.  The entries in entriesArray hold pointers to their respective tables in this list. */
	RsslUInt16			enumTableCount;			/*!< Total number of tables present. */

	/* Tags */
	RsslInt32			info_DictionaryId;		/*!< Tag: DictionaryId.  All dictionaries loaded using this object will have this tag matched if found. */

	/* Field Dictionary Tags */
	RsslBuffer          infoField_Version;		/*!< Tag: Version */

	/* Enum Type Dictionary Tags */
	RsslBuffer			infoEnum_RT_Version;	/*!< Tag: RT_Version */
	RsslBuffer			infoEnum_DT_Version;	/*!< Tag: DT_Version */

	/* Field Dictionary Additional tags(currently these are not defined by the domain model and are not sent by the encode/decode functions) */
	RsslBuffer          infoField_Filename;		/*!< Tag: Filename */
	RsslBuffer          infoField_Desc;			/*!< Tag: Desc */
	RsslBuffer          infoField_Build;			/*!< Tag: Build */
	RsslBuffer          infoField_Date;			/*!< Tag: Date */

	/* Enum Type Dictionary Additional Tags */
	RsslBuffer          infoEnum_Filename;		/*!< Tag: Filename */
	RsslBuffer          infoEnum_Desc;			/*!< Tag: Desc */
	RsslBuffer          infoEnum_Date;			/*!< Tag: Date */
	void				*_internal;				/*!< Internal use only. */
} RsslDataDictionary;


/**
 * @brief RsslDataDictionary initializer
 * @see RsslDataDictionary, rsslClearDataDictionary
 */
#define RSSL_INIT_DATA_DICTIONARY { RSSL_MAX_FID + 1, RSSL_MIN_FID - 1, 0, 0, RSSL_FALSE, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER}

/**
 * @brief Clears an RsslDataDictionary. This should be done prior to the first call of a dictionary loading function, if the initializer is not used.
 * @see RSSL_INIT_DATA_DICTIONARY, RsslDataDictionary 
 */
RTR_C_INLINE void rsslClearDataDictionary(RsslDataDictionary *pDataDict)
{
	pDataDict->isInitialized = RSSL_FALSE;
}

/**
 * @brief Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
 * @see RsslDictionaryEntry
 */
RTR_C_INLINE RsslDictionaryEntry *getDictionaryEntry(RsslDataDictionary *pDictionary, RsslFieldId fieldId)
{
	return pDictionary->entriesArray[fieldId];
}

/**
 * @brief Returns the corresponding enumerated type in the dictionary entry's table, if the type exists.
 * @see RsslEnumType, RsslDictionaryEntry
 */
RTR_C_INLINE RsslEnumType *getFieldEntryEnumType(RsslDictionaryEntry *pEntry, RsslEnum value)
{
	return (pEntry->pEnumTypeTable && value <= pEntry->pEnumTypeTable->maxValue) ? pEntry->pEnumTypeTable->enumTypes[value] : 0;
}

/**
 * @brief Adds information from a field dictionary file to the data dictionary object.
 * Subsequent calls to this function may be made to the same RsslDataDictionary to load additional dictionaries(provided the fields do not conflict).
 * @see RsslDataDictionary
 */
RSSL_API RsslRet rsslLoadFieldDictionary(
				const char				*filename,
				RsslDataDictionary	*dictionary,
				RsslBuffer			*errorText );

/**
 * @brief Fully dumps information contained in a data dictionary object to a file.
 * @see  RsslDataDictionary
 */
RSSL_API RsslRet rsslPrintDataDictionary(
				FILE *fileptr,
				RsslDataDictionary	*dictionary );


/**
 * @brief Uninitializes a data dictionary and frees all memory associated with it.
 * @see RsslDataDictionary
 */
RSSL_API RsslRet rsslDeleteDataDictionary(
				RsslDataDictionary	*dictionary );




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
RSSL_API RsslRet rsslEncodeFieldDictionary(
	RsslEncodeIterator				*eIter,
	RsslDataDictionary				*dictionary,
	int								*currentFid,
	RDMDictionaryVerbosityValues	 verbosity,
	RsslBuffer						*errorText);


/**
 * @brief Decode the field dictionary information contained in a data payload according to the domain model.
 * This function may be called multiple times on the same dictionary, to load information from dictionaries that have been encoded in multiple parts.
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded field information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeFieldDictionary
 */
RSSL_API RsslRet rsslDecodeFieldDictionary(
	RsslDecodeIterator				*dIter,
	RsslDataDictionary				*dictionary,
	RDMDictionaryVerbosityValues	 verbosity,
	RsslBuffer						*errorText);



/**
 * @brief Adds information from an enumerated types dictionary file to the data dictionary object.
 * Subsequent calls to this function may be made to the same RsslDataDictionary to load additional dictionaries(provided that there are no duplicate table references for any field)
 * @see RsslDataDictionary
 */
RSSL_API RsslRet rsslLoadEnumTypeDictionary(	const char				*filename,
							RsslDataDictionary	*dictionary,
							RsslBuffer			*errorText );


/**
 * @brief Encode the enumerated types dictionary according the domain model, using the information from the tables and referencing fields present in this dictionary.
 * Note: This function will use the type RSSL_DT_ASCII_STRING for the DISPLAY array.
 * @param eIter Iterator to be used for encoding.
 * @param dictionary The dictionary to encode enumerated type information from.
 * @param verbosity The desired verbosity to encode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the encoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionaryAsMultiPart, rsslDecodeEnumTypeDictionary
 */

RSSL_API RsslRet rsslEncodeEnumTypeDictionary(
	RsslEncodeIterator				*eIter,
	RsslDataDictionary				*dictionary,
	RDMDictionaryVerbosityValues	 verbosity,
	RsslBuffer						*errorText);

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

RSSL_API RsslRet rsslEncodeEnumTypeDictionaryAsMultiPart(
	RsslEncodeIterator				*eIter,
	RsslDataDictionary				*dictionary,
	int								*currentFid,
	RDMDictionaryVerbosityValues	 verbosity,
	RsslBuffer						*errorText);

/**
 * @brief Decode the enumerated types information contained in an encoded enum types dictionary according to the domain model.
 * @param dIter An iterator to use. Must be set to the encoded buffer.
 * @param dictionary The dictionary to store the decoded enumerated types information.
 * @param verbosity The desired verbosity to decode. See RDMDictionaryVerbosityValues.
 * @param errorText Buffer to hold error text if the decoding fails.
 * @see RsslDataDictionary, RDMDictionaryVerbosityValues, rsslEncodeEnumTypeDictionary, rsslEncodeEnumTypeDictionaryAsMultiPart
 */
RSSL_API RsslRet rsslDecodeEnumTypeDictionary(
	RsslDecodeIterator				*dIter,
	RsslDataDictionary				*dictionary,
	RDMDictionaryVerbosityValues	verbosity,
	RsslBuffer						*errorText);

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
RSSL_API RsslRet rsslExtractDictionaryType(
	RsslDecodeIterator		*dIter,
	RDMDictionaryTypes		*dictionaryType,
	RsslBuffer				*errorText);

/**
 * @brief Lookup a field based on its name.
 * @param pDictionary Dictionary to use for the lookup.
 * @param pFieldName RsslBuffer containing the name of the field to lookup.
 * @return The matching RsslDictionaryEntry, or NULL if no such field exists.
 */
RSSL_API RsslDictionaryEntry *rsslDictionaryGetEntryByFieldName(RsslDataDictionary *pDictionary, const RsslBuffer *pFieldName);
 
/**
 * @brief Lookup an Enumeration's value by its display string.
 * @param pEntry Dictionary Entry to use for the lookup.
 * @param pEnumDisplay Display string to lookup.
 * @param pEnumValue Storage for the matching enumerated value.
 * @param errorText Buffer to hold errorText if the lookup fails.
 * @return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE if a matching value does not exist, RSSL_RET_DICT_DUPLICATE_ENUM_VALUE if the display string corresponds to multiple values.
 */
RSSL_API RsslRet rsslDictionaryEntryGetEnumValueByDisplayString(const RsslDictionaryEntry *pEntry, const RsslBuffer *pEnumDisplay, RsslEnum *pEnumValue, RsslBuffer *errorText);

/*
 * @brief For internal use only. Matches fields of two dictionaries, then reuses the allocated RsslDictionaryEntry objects of the old dictionary.
 * The two dictionaries will share their RsslDictionaryEntry objects and the respective RsslEnumTypeTable objects.
 * After rsslLinkDataDictionary is called, calling rsslDeleteDataDictionary on the old dictionary will not delete the shared RsslDictionaryEntry objects.
 * Functions that add definitions, such as rsslLoadFieldDictionary, rsslLoadEnumTypeDictionary, rsslDecodeFieldDictionary, and rsslDecodeEnumTypeDictionary, may be called on the new dictionary, but should no longer be called on the old dictionary.
 * @param pNewDictionary New dictionary to link.
 * @param pOldDictionary Existing dictionary to link.
 * @param errorText Buffer to hold error text if the link fails.
 * @return RSSL_RET_SUCCESS if the link was succesful, other error codes on failure.
 */
RSSL_API RsslRet rsslLinkDataDictionary(RsslDataDictionary *pNewDictionary, RsslDataDictionary *pOldDictionary, RsslBuffer *errorText);

/** 
 * @}
 */

/** 
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* __RSSL_STATE_H */

