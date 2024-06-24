/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_SET_DATA_H
#define __RSSL_SET_DATA_H


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslRDM.h"


/**
 * @addtogroup FieldListSetInfo
 *	@{
 */

/** 
 * @brief Field List Set Definition Field Entry.
 * @see RsslFieldSetDef, rsslClearFieldSetDefEntry, RSSL_INIT_FIELD_SET_DEF_ENTRY
 */
typedef struct {
	RsslFieldId			fieldId;	/*!<  The field identifier */
	RsslUInt8			dataType;   /*!<  The field data type */
} RsslFieldSetDefEntry;


/**
 * @brief RsslFieldSetDefEntry initializer
 * @see RsslFieldSetDefEntry, rsslClearFieldSetDefEntry
 */
#define RSSL_INIT_FIELD_SET_DEF_ENTRY { 0, 0 }

/**
 * @brief Clears an RsslFieldSetDefEntry
 * @see RsslFieldSetDef, RSSL_INIT_FIELD_SET_DEF_ENTRY
 */
RTR_C_INLINE void rsslClearFieldSetDefEntry(RsslFieldSetDefEntry *pFieldEnc)
{
	pFieldEnc->fieldId = 0;
	pFieldEnc->dataType = 0;
}


/**
 * @brief Field List Set Definition.
 * @see RsslLocalFieldSetDefDb, RsslFieldSetDefDb, rsslClearFieldSetDef, RSSL_INIT_FIELD_SET_DEF
 */
typedef struct {
	RsslUInt16				setId;		 /*!< The set id */
	RsslUInt8				count;		 /*!< the number of entries in pEntries. */
	RsslFieldSetDefEntry	*pEntries;  /*!< Pointer to the array of entries this set def will use. */
} RsslFieldSetDef;

/**
 * @brief RsslFieldSetDef static initializer
 * @see RsslFieldSetDef, rsslClearFieldSetDef
 */
#define RSSL_INIT_FIELD_SET_DEF { 0, 0, 0 }

/**
 * @brief Clears an RsslFieldSetDef
 * @see RsslFieldSetDef, RSSL_INIT_FIELD_SET_DEF
 */
RTR_C_INLINE void rsslClearFieldSetDef(RsslFieldSetDef *pFieldLSD)
{
	pFieldLSD->setId = 0;
	pFieldLSD->count = 0;
	pFieldLSD->pEntries = 0;
}

/**
 * @brief Return the maximum local message scope set identifier.
 * @see RsslLocalFieldSetDefDb, RsslFieldSetDef
 * @return value.
 */
#define RSSL_FIELD_SET_MAX_LOCAL_ID 15
#define RSSL_FIELD_SET_BLANK_ID 255
#define RSSL_MAX_GLOBAL_SET_ID 65535

/**
 * @brief Local Message Field List Set Definitions Database.  Must be cleared before use.
 * 
 * Typical Use: <BR>
 * If decoding local set definitions from a single thread, RSSL has internal memory that will be <BR>
 * decoded into.  To use this, entries.data and entries.length should be 0.  <BR>
 * If decoding local set definitions from across multiple threads, the user should supply memory <BR>
 * by populating entries.data.  entries.length should be set to the number of bytes available in data. <BR>
 * @see RsslFieldSetDef, rsslClearLocalFieldSetDefDb
 */
typedef struct {
	RsslFieldSetDef			definitions[RSSL_FIELD_SET_MAX_LOCAL_ID+1];		/*!< The list of definitions, indexed by ID. */
	RsslBuffer				entries;										/*!< A buffer that can point to workspace for encoding or decoding.  When encoding a database, the user may optionally populate the SetDef objects with these entries instead of creating separate objects. When decoding, rsslDecodeElementSetDefDb() will decode into any memory populated here or point to internal storage if no memory is provided. */
} RsslLocalFieldSetDefDb;


/**
 * @brief Clears an RsslLocalFieldSetDefDb
 * @see RsslLocalFieldSetDefDb
 */
RTR_C_INLINE void rsslClearLocalFieldSetDefDb(RsslLocalFieldSetDefDb *pDb)
{
	int i;
	for (i = 0; i <= RSSL_FIELD_SET_MAX_LOCAL_ID; ++i) 
		pDb->definitions[i].setId = RSSL_FIELD_SET_BLANK_ID;
	pDb->entries.data = 0;
	pDb->entries.length = 0;
}

/**
 * @brief Global Field List Set Definitions Database.
 * @see RsslFieldSetDef
 ***/
typedef struct {
	RsslUInt16			maxSetId; 			/*!< Maximum set definition index */
	RsslFieldSetDef		**definitions;		/*!< Pointer array of RsslFieldSetDef definitions.  This is allocated with rsslAllocateFieldSetDefDb. */
	RsslBool			isInitialized;
	RsslBuffer			info_version;		/*!< Tag: Dictionary version */
	RsslInt32			info_DictionaryId;	/*!< Tag: DictionaryId.  All dictionaries loaded using this object will have this tag matched if found. */
} RsslFieldSetDefDb;

/**
 ** @brief RsslFieldSetDefDb initializer
 ** @see RsslFieldSetDefDb, rsslClearFieldSetDb
 **/
#define RSSL_INIT_FIELD_LIST_SET_DB { 0, 0, 0, RSSL_INIT_BUFFER, 0 }

/**
 ** @brief Clears an RsslFieldSetDefDb
 ** @see RsslFieldSetDefDb, RSSL_INIT_FIELD_LIST_SET_DB
 **/
RTR_C_INLINE void rsslClearFieldSetDb(RsslFieldSetDefDb *pFieldSDb)
{
	pFieldSDb->isInitialized = RSSL_FALSE;
}
/**
 *	@brief Allocates the definitions pointer array of the provided RsslFieldSetDefDb, and deep copies the version string into the structure.
 */
RSSL_API RsslRet rsslAllocateFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslBuffer* version);


/**
 *	@brief Deep copies the given RsslFieldSetDef into the RsslFieldSetDefDb's definitions array.
 */
RSSL_API RsslRet rsslAddFieldSetDefToDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslFieldSetDef* pFieldSetDef);

/**
 *	@brief Deletes and frees all memory allocated by rsslAllocateFieldSetDefDb and rsslAddFieldSetDefToDb.
 *
 *	@note If the memory used by the RsslFieldSetDefDb has been created and managed outside of rsslAllocateFieldSetDefDb and rsslAddFieldSetDefToDb, this function should not be used.
 *
 */
RSSL_API RsslRet rsslDeleteFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb);

/**
 * @}
 */

/** 
 * @addtogroup ElementListSetInfo
 * @{
 */

/**
 * @brief Element List Set Element Encoding
 * @see RsslElementSetDef, rsslClearElementSetDefEntry, RSSL_INIT_ELEMENT_SET_DEF_ENTRY 
 */
typedef struct {
	RsslBuffer			name;	/*!<  The element identifier */
	RsslUInt8			dataType;  /*!< The element data type */
} RsslElementSetDefEntry;


/**
 * @brief RsslElementSetDefEntry initializer
 * @see RsslElementSetDefEntry, rsslClearElementSetDefEntry
 */
#define RSSL_INIT_ELEMENT_SET_DEF_ENTRY { 0, 0 }

/**
 * @brief Clears an RsslElementSetDefEntry
 * @see RSSL_INIT_ELEMENT_SET_DEF_ENTRY, RsslElementSetDefEntry 
 */
RTR_C_INLINE void rsslClearElementSetDefEntry(RsslElementSetDefEntry *pElementEnc)
{
	pElementEnc->dataType = 0;
	pElementEnc->name.data = 0;
	pElementEnc->name.length = 0;
}


/**
 * @brief Element List Set Definition 
 * @see RsslLocalElementSetDefDb, RsslElementSetDefDb, rsslClearElementSetDef, RSSL_INIT_ELEMENT_SET_DEF
 */
typedef struct {
	RsslUInt16			setId;  /*!< The set id */
	RsslUInt8			count;  /*!< The number of entries in pEntries. */
	RsslElementSetDefEntry	*pEntries; /* Pointer to the array of entries this definition will use. */
} RsslElementSetDef;


/**
 * @brief RsslElementSetDef initializer
 * @see RsslElementSetDef, rsslClearElementSetDef
 */
#define RSSL_INIT_ELEMENT_SET_DEF { 0, 0, 0 }

/**
 * @brief Clears an RsslElementSetDef
 * @see RsslElementSetDef, RSSL_INIT_ELEMENT_SET_DEF
 */
RTR_C_INLINE void rsslClearElementSetDef(RsslElementSetDef *pElementLSD)
{
	pElementLSD->count = 0;
	pElementLSD->pEntries = 0;
	pElementLSD->setId = 0;
}

/** 
 * @brief Return the maximum local message scope set identifier
 * @see RsslLocalElementSetDefDb, RsslElementSetDef
 * @return value
 */
#define RSSL_ELEMENT_SET_MAX_LOCAL_ID 15
#define RSSL_ELEMENT_SET_BLANK_ID 255

/**
 * @brief Local Message Element List Set Definitions Database, must be cleared before use
 *
 * Typical Use: <BR>
 * If decoding local set definitions from a single thread, RSSL has internal memory that will be <BR>
 * decoded into.  To use this, entries.data and entries.length should be 0.  <BR>
 * If decoding local set definitions from across multiple threads, the user should supply memory <BR>
 * by populating entries.data.  entries.length should be set to the number of bytes available in data. <BR>
 * @see RsslElementSetDef, rsslClearLocalElementSetDefDb
 */
typedef struct {
	RsslElementSetDef		definitions[RSSL_ELEMENT_SET_MAX_LOCAL_ID+1];	/*!< The list of definitions, indexed by ID. */
	RsslBuffer				entries; /*!< A buffer that can point to workspace for encoding or decoding.  When encoding a database, the user may optionally populate the SetDef objects with these entries instead of creating separate objects. When decoding, rsslDecodeElementSetDefDb() will decode into any memory populated here or point to internal storage if no memory is provided. */
} RsslLocalElementSetDefDb;

/**
 * @brief Clears an RsslLocalElementSetDefDb
 * @see RsslLocalElementSetDefDb
 */
RTR_C_INLINE void rsslClearLocalElementSetDefDb(RsslLocalElementSetDefDb *pDb)
{
	int i;

	for (i = 0; i <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; ++i) 
		pDb->definitions[i].setId = RSSL_ELEMENT_SET_BLANK_ID;
	pDb->entries.data = 0;
	pDb->entries.length = 0;
}


/* Global List Set Database */
/** 
 ** @brief Global Element List Set Definitions Database
 ** @see RsslElemetnListSetDef
 **/
typedef struct {
	RsslUInt16				maxSetId;				/*!< Maximum set definition index */
	RsslElementSetDef		**definitions;			/*!< Pointer array of RsslElementSetDef definitions.  This is allocated with rsslAllocateElementSetDefDb. */
	RsslBool				isInitialized;		
	RsslBuffer				info_version;			/*!< Tag: Dictionary version */
	RsslInt32				info_DictionaryId;		/*!< Tag: DictionaryId.  All dictionaries loaded using this object will have this tag matched if found. */
} RsslElementSetDefDb;


/**
 ** @brief RsslElementSetDefDb initializer
 ** @see RsslElementSetDefDb, rsslClearElementSetDefDb
 **/
#define RSSL_INIT_ELEMENT_LIST_SET_DB { 0, 0, 0, RSSL_INIT_BUFFER, 0 }

/**
 ** @brief Clears an RsslElementSetDef
 ** @see RsslElementSetDefDb, RSSL_INIT_ELEMENT_SET_DEF_DB
 **/
RTR_C_INLINE void rsslClearElementSetDefDb(RsslElementSetDefDb *pElementLSDb)
{
	pElementLSDb->isInitialized = RSSL_FALSE;
}


/**
 *	@brief Allocates the definitions pointer array of the provided RsslElementSetDefDb, and deep copies the version string into the structure.
 */
RSSL_API RsslRet rsslAllocateElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb, RsslBuffer* version);

/**
 *	@brief Deep copies the given RsslFieldSetDef into the RsslElementSetDefDb's definitions array.
 */
RSSL_API RsslRet rsslDeleteElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb);


/**
 *	@brief Deletes and frees all memory allocated by rsslAllocateElementSetDefDb and rsslAddElementSetDefToDb.
 *
 *	@note If the memory used by the RsslFieldSetDefDb has been created and managed outside of rsslAllocateElementSetDefDb and rsslAddElementSetDefToDb, this function should not be used.
 *
 */
RSSL_API RsslRet rsslAddElementSetDefToDb(RsslElementSetDefDb* pElementSetDefDb, RsslElementSetDef* pElementSetDef);

/**
 * @}
 */



/********************************************************************************************/


 

#ifdef __cplusplus
}
#endif

#endif
