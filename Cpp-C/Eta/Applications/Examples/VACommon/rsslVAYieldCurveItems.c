/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#include "rsslVAYieldCurveItems.h"
#include "rsslVASendMessage.h"
#include <stdlib.h>

/* 
 * This handles the generation of example yield curve data, and shows
 * how to encode it.  Provides functions for managing the list,
 * and a function for encoding a yield curve message and payload.
 */

#define MAX_YIELD_CURVE_ITEM_LIST_SIZE 100

/* item information list */
static RsslYieldCurveItem yieldCurveItemList[MAX_YIELD_CURVE_ITEM_LIST_SIZE];

/*
 * Create a local set definitions.
 * Set definitions are used to reduce the size of messages with a
 * large amount of repetitive data.  For example, the sets defined here
 * is for a payload that contains many Vectors with multiple entries.
 * The set definitions will provide the fieldIds and data types
 * so that they don't need to be present on each Vector Entry.
 */

/*
 * Create an array of set definition entries. This defines
 * what entries will be encoded when using this set.
 */
static RsslFieldSetDefEntry swapRatesRefreshSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{SWAP_SDATE_FID, RSSL_DT_DATE},			/* Swap Settle Date */ 
	{SWAP_RDATE_FID, RSSL_DT_DATE},			/* Swap Roll Date */ 
	{SWAP_MDATE_FID, RSSL_DT_DATE},			/* Swap Maturity Date */ 
	{SWAP_RATE_VAL_FID, RSSL_DT_REAL},		/* Swap Rate Value */ 
	{SWAP_SRC_FID, RSSL_DT_ASCII_STRING}	/* Swap Source */ 
};

static RsslFieldSetDefEntry swapRatesUpdateSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{SWAP_RATE_VAL_FID, RSSL_DT_REAL}		/* Swap Rate Value */ 
};

static RsslFieldSetDefEntry cashRatesRefreshSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{CASH_SDATE_FID, RSSL_DT_DATE},			/* Cash Settle Date */ 
	{CASH_MDATE_FID, RSSL_DT_DATE},			/* Cash Maturity Date */ 
	{CASH_RATE_FID, RSSL_DT_REAL},			/* Cash Rate */ 
	{CASH_SRC_FID, RSSL_DT_ASCII_STRING}	/* Cash Source */ 
};

static RsslFieldSetDefEntry cashRatesUpdateSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{CASH_RATE_FID, RSSL_DT_REAL}			/* Cash Rate */ 
};

static RsslFieldSetDefEntry futureRatesRefreshSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{FUTR_SDATE_FID, RSSL_DT_DATE},			/* Future Settle Date */  
	{FUTR_MDATE_FID, RSSL_DT_DATE},			/* Future Maturity Date */ 
	{FUTR_PRICE_FID, RSSL_DT_REAL},			/* Future Price */
	{FUTR_SRC_FID, RSSL_DT_ASCII_STRING}	/* Future Source */ 
};											

static RsslFieldSetDefEntry futureRatesUpdateSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{FUTR_PRICE_FID, RSSL_DT_REAL}			/* Future Price */
};

static RsslFieldSetDefEntry yieldCurveRefreshSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{YCT_FWRATE_FID, RSSL_DT_ARRAY},		/* YC Tenor FW Rate */  
	{YCT_FWDATE_FID, RSSL_DT_ARRAY},		/* YC Tenor FW Date */ 
	{YCT_DATE_FID, RSSL_DT_DATE},			/* YC Tenor Date */ 
	{YCT_ZRATE_FID, RSSL_DT_REAL},			/* YC Tenor Zero Rate */ 
	{YCT_DISFAC_FID, RSSL_DT_REAL}			/* YC Tenor Disc Factor */ 
};

static RsslFieldSetDefEntry yieldCurveUpdateSetEntries[] =
{
	/* { fieldId, dataType } */
	/* ---------------------------------------- */
	{YCT_FWRATE_FID, RSSL_DT_ARRAY},		/* YC Tenor FW Rate */  
	{YCT_ZRATE_FID, RSSL_DT_REAL},			/* YC Tenor Zero Rate */  
	{YCT_DISFAC_FID, RSSL_DT_REAL}			/* YC Tenor Disc Factor */ 
};

/* 
 * Create a set definition database to store the definition.
 * This particular database is encoded with the RsslVector
 * A database may store up to 16 sets for use. 
 */
static RsslLocalFieldSetDefDb yieldCurveSetDefDb =
{
	{
		/* {setId, entry count, entries array} */
    	{ SWAP_RATES_REFRESH_SET_DEFS_DB,	5, swapRatesRefreshSetEntries },
    	{ SWAP_RATES_UPDATE_SET_DEFS_DB,	1, swapRatesUpdateSetEntries },
		{ CASH_RATES_REFRESH_SET_DEFS_DB,	4, cashRatesRefreshSetEntries },
    	{ CASH_RATES_UPDATE_SET_DEFS_DB,	1, cashRatesUpdateSetEntries },
    	{ FUTR_RATES_REFRESH_SET_DEFS_DB,	4, futureRatesRefreshSetEntries },
    	{ FUTR_RATES_UPDATE_SET_DEFS_DB,	1, futureRatesUpdateSetEntries },
    	{ YIELD_CURVE_REFRESH_SET_DEFS_DB,	5, yieldCurveRefreshSetEntries },
    	{ YIELD_CURVE_UPDATE_SET_DEFS_DB,	3, yieldCurveUpdateSetEntries },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 }
	},
	{0, 0}
};

/* 
 * Create a set definition database to store the definition.
 * This particular database is encoded with the RsslVector and is passed into 
 * rsslEncodeFieldListInit().
 * A database may store up to 16 sets for use. 
 */
static RsslLocalFieldSetDefDb tmpYieldCurveSetDefDb =
{
	{
		/* {setId, entry count, entries array} */
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
		{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
    	{ RSSL_FIELD_SET_BLANK_ID, 0, 0 }
	},
	{0, 0}
};

/*
 * Helper function to get data type of the field entry from dictionary and encoded in the 
 * field list.
 */
static RsslRet encodeFieldEntry(RsslEncodeIterator* encodeIter, RsslDataDictionary* dictionary, int FID, void* data)
{
	RsslRet ret = RSSL_RET_FAILURE;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	/* get information from dictionary for a FID */
	dictionaryEntry = dictionary->entriesArray[FID];
	/* check if FID exists in the dictionary */
	if (dictionaryEntry)
	{	
		/* populate field entry */
		fEntry.fieldId = FID;
		fEntry.dataType = dictionaryEntry->rwfType;
		/* encode field entry */
		if ((ret = rsslEncodeFieldEntry(encodeIter, &fEntry, data)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntry() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		printf("FID %d not found in RDMFieldDictionary\n", FID);
		return RSSL_RET_FAILURE;
	}

	return ret;
}

/*
 * Helper function to encode vector along with sed data defintions.
 */
static RsslRet encodeVectorInit(RsslEncodeIterator* encodeIter, short setId, RsslEncodeIterator* summaryDataIter)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslVector vector = RSSL_INIT_VECTOR;
	int i = 0;	
	vector.containerType = RSSL_DT_FIELD_LIST;
	vector.flags = RSSL_VTF_HAS_SET_DEFS;

	if (summaryDataIter)
	{
		vector.flags = RSSL_VTF_HAS_SET_DEFS | RSSL_VTF_HAS_SUMMARY_DATA;
		vector.encSummaryData.data = (char*) summaryDataIter->_pBuffer->data;
		vector.encSummaryData.length = rsslGetEncodedBufferLength(summaryDataIter);
	}

	/* start encoding vector */
	if ((ret = rsslEncodeVectorInit(encodeIter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeVectorInit() failed with return code: %d\n", ret);
		return ret;
	}

	/* copy the data set definitions to a temporary db before encoding.
	 * we do not want to encode whole database for all vectors. 
	 * We want to encode only one data set defintion used by the vector */
	rsslClearLocalFieldSetDefDb(&tmpYieldCurveSetDefDb);
	tmpYieldCurveSetDefDb.definitions[setId] = yieldCurveSetDefDb.definitions[setId];
	for (i = 0; i < yieldCurveSetDefDb.definitions[setId].count; i++)
	{
		/* copy entries from the data set defintions */
		tmpYieldCurveSetDefDb.definitions[setId].pEntries[i] = 
			yieldCurveSetDefDb.definitions[setId].pEntries[i];
	}

	/* Encode the field set definition database into the vector.
	* The definitions are used by the field lists that the vector contains. */
	if ((ret = rsslEncodeLocalFieldSetDefDb(encodeIter, &tmpYieldCurveSetDefDb)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeLocalFieldSetDefDb() failed with return code: %d\n", ret);
		return ret;
	}

	/* complete of data set definitions encoding */
	if ((ret = rsslEncodeVectorSetDefsComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeVectorSetDefsComplete() failed with return code: %d\n", ret);
		return ret;
	}
	return ret;
}

/*
 * Helper function to complete encoding of few types of containers and entries
 */
static RsslRet completeEncoding(RsslEncodeIterator* encodeIter)
{
	RsslRet ret = RSSL_RET_SUCCESS;

	if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if ((ret = rsslEncodeVectorComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeVectorComplete() failed with return code: %d\n", ret);
		return ret;
	}

	if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
		return ret;
	}
	return ret;

}

/*
 * Helper function to encode the CASH RATES VECTOR
 */
static RsslRet encodeYieldCurveCashRates(RsslEncodeIterator* encodeIter, RsslDataDictionary* dictionary,RsslYieldCurveItem* ycItem, RsslBool isRefresh)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
	RsslYieldCurveCashRates* cr = &ycItem->cashRates;

	/* CASH_RATES */
	dictionaryEntry = dictionary->entriesArray[CASH_RATES_FID];
	if (dictionaryEntry)
	{	
		/* populate Field Entry */
		fieldEntry.fieldId = CASH_RATES_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;

		/* start encoding complex field entry */
		if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryInit() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		printf("FID %d not found in RDMFieldDictionary\n", CASH_RATES_FID);
		return RSSL_RET_FAILURE;
	}

	if (isRefresh)
	{
		RsslBuffer tmpBuf;

		/* start encoding vector */
		if ((ret = encodeVectorInit(encodeIter, CASH_RATES_REFRESH_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
		{
			printf("encodeVectorInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* populate Vector Entry */
		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_SET_ENTRY;
		vectorEntry.index = 0;

		/* start encoding Vector Entries */
		if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* populate Field List */
		fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
		fieldList.setId = CASH_RATES_REFRESH_SET_DEFS_DB;
		/* start encoding Field List inside a Vector */
		if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* CASH_SDATE */
		encodeFieldEntry(encodeIter, dictionary, CASH_SDATE_FID, (void*)&cr->CASH_SDATE);

		/* CASH_MDATE */
		encodeFieldEntry(encodeIter, dictionary, CASH_MDATE_FID, (void*)&cr->CASH_MDATE);

		/* CASH_RATE */ 
		encodeFieldEntry(encodeIter, dictionary, CASH_RATE_FID, (void*)&cr->CASH_RATE);

		/* CASH_SRC */ 
		tmpBuf.data = &cr->CASH_SRC[0];
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(encodeIter, dictionary, CASH_SRC_FID, (void*)&tmpBuf);
	}
	else /* Update */
	{
		/* start encoding vector */
		if ((ret = encodeVectorInit(encodeIter, CASH_RATES_UPDATE_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
		{
			printf("encodeVectorInit() failed with return code: %d\n", ret);
			return ret;
		}

		vectorEntry.flags = RSSL_VTEF_NONE;
		vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
		vectorEntry.index = 0;

		if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* populate Field List */		
		fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
		fieldList.setId = CASH_RATES_UPDATE_SET_DEFS_DB;
		/* start encoding Field List inside a Vector */
		if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* CASH_RATE */ 
		encodeFieldEntry(encodeIter, dictionary, CASH_RATE_FID, (void*)&cr->CASH_RATE);
	}

	if ((ret = completeEncoding(encodeIter)) < RSSL_RET_SUCCESS)
	{
		printf("completeEncoding() failed with return code: %d\n", ret);
		return ret;
	}
	return ret;
}

/*
 * Helper function to encode the FUTURE PRICES VECTOR
 */
static RsslRet encodeYieldCurveFuturePrices(RsslEncodeIterator* encodeIter, RsslDataDictionary* dictionary,RsslYieldCurveItem* ycItem, RsslBool isRefresh)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
	RsslYieldCurveFuturePrices *fp = &ycItem->futurePrices;

	/* FUTR_PRCS */
	dictionaryEntry = dictionary->entriesArray[FUTR_PRCS_FID];
	if (dictionaryEntry)
	{	
		/* populate Field Entry */
		fieldEntry.fieldId = FUTR_PRCS_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;

		/* start encoding complex field entry */
		if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryInit() failed with return code: %d\n", ret);
			return ret;
		}

		if (isRefresh)
		{
			RsslBuffer tmpBuf;
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, FUTR_RATES_REFRESH_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			vectorEntry.flags = RSSL_VTEF_NONE;
			vectorEntry.action = RSSL_VTEA_SET_ENTRY;
			vectorEntry.index = 0;

			if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
				return ret;
			}

			fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
			fieldList.setId = FUTR_RATES_REFRESH_SET_DEFS_DB;
			/* start encoding Field List inside a Vector */
			if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
				return ret;
			}

			/* FUTR_SDATE */
			encodeFieldEntry(encodeIter, dictionary, FUTR_SDATE_FID, (void*)&fp->FUTR_SDATE);

			/* FUTR_MDATE */
			encodeFieldEntry(encodeIter, dictionary, FUTR_MDATE_FID, (void*)&fp->FUTR_MDATE);

			/* FUTR_PRICE */ 
			encodeFieldEntry(encodeIter, dictionary, FUTR_PRICE_FID, (void*)&fp->FUTR_PRICE);

			/* FUTR_SRC */ 
			tmpBuf.data = &fp->FUTR_SRC[0];
			tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
			encodeFieldEntry(encodeIter, dictionary, FUTR_SRC_FID, (void*)&tmpBuf);

		}
		else /* Update */
		{
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, FUTR_RATES_UPDATE_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			vectorEntry.flags = RSSL_VTEF_NONE;
			vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
			vectorEntry.index = 0;

			if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
				return ret;
			}

			fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
			fieldList.setId = FUTR_RATES_UPDATE_SET_DEFS_DB;
			/* start encoding Field List inside a Vector */
			if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
				return ret;
			}

			/* FUTR_PRICE */ 
			encodeFieldEntry(encodeIter, dictionary, FUTR_PRICE_FID, (void*)&fp->FUTR_PRICE);
		}

		if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeVectorComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		printf("FID %d not found in RDMFieldDictionary\n", CASH_RATES_FID);
		return RSSL_RET_FAILURE;
	}
		
	return ret;
}

/*
 * Helper function to encode the SWAP RATES VECTOR
 */
static RsslRet encodeYieldCurveSwapRates(RsslEncodeIterator* encodeIter, RsslDataDictionary* dictionary,RsslYieldCurveItem* ycItem, RsslBool isRefresh)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
	RsslYieldCurveSwapRates *sr;

	/* SWAP_RATES */
	dictionaryEntry = dictionary->entriesArray[SWAP_RATES_FID];
	if (dictionaryEntry)
	{	
		int i = 0;

		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = SWAP_RATES_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;

		/* start encoding complex field entry */
		if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryInit() failed with return code: %d\n", ret);
			return ret;
		}

		if (isRefresh)
		{
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, SWAP_RATES_REFRESH_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			for (i = 0; i < SWAP_RATES_NUM; i++)
			{
				RsslBuffer tmpBuf;
				sr = &ycItem->swapRates[i];

				vectorEntry.flags = RSSL_VTEF_NONE;
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorEntry.index = i;

				if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
					return ret;
				}

				fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
				fieldList.setId = SWAP_RATES_REFRESH_SET_DEFS_DB;
				/* start encoding Field List inside a Vector */
				if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
					return ret;
				}

				/* SWAP_SDATE */
				encodeFieldEntry(encodeIter, dictionary, SWAP_SDATE_FID, (void*)&sr->SWAP_SDATE);

				/* SWAP_RDATE */
				encodeFieldEntry(encodeIter, dictionary, SWAP_RDATE_FID, (void*)&sr->SWAP_RDATE);

				/* SWAP_MDATE */
				encodeFieldEntry(encodeIter, dictionary, SWAP_MDATE_FID, (void*)&sr->SWAP_MDATE);

				/* SWAP_RATE_VAL */ 
				encodeFieldEntry(encodeIter, dictionary, SWAP_RATE_VAL_FID, (void*)&sr->SWAP_RATE_VAL);

				/* SWAP_SRC */ 
				tmpBuf.data = &sr->SWAP_SRC[0];
				tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
				encodeFieldEntry(encodeIter, dictionary, SWAP_SRC_FID, (void*)&tmpBuf);

				if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
					return ret;
				}

				if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
					return ret;
				}

				/* clear both vector entry and field list */
				rsslClearVectorEntry(&vectorEntry);
				rsslClearFieldList(&fieldList);
			} /* for loop end */
		}
		else
		{
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, SWAP_RATES_UPDATE_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			for (i = 0; i < SWAP_RATES_NUM; i++)
			{
				sr = &ycItem->swapRates[i];

				vectorEntry.flags = RSSL_VTEF_NONE;
				vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
				vectorEntry.index = i;

				if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
					return ret;
				}

				fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
				fieldList.setId = SWAP_RATES_UPDATE_SET_DEFS_DB;
				/* start encoding Field List inside a Vector */
				if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
					return ret;
				}

				/* SWAP_RATE_VAL */ 
				encodeFieldEntry(encodeIter, dictionary, SWAP_RATE_VAL_FID, (void*)&sr->SWAP_RATE_VAL);

				if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
					return ret;
				}

				if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
					return ret;
				}

				/* clear both vector entry and field list */
				rsslClearVectorEntry(&vectorEntry);
				rsslClearFieldList(&fieldList);

			} /* for loop end */

		}

		if ((ret = rsslEncodeVectorComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		printf("FID %d not found in RDMFieldDictionary\n", CASH_RATES_FID);
		return RSSL_RET_FAILURE;
	}
		
	return ret;
}

/*
 * Helper function to encode the YIELD CURVE VECTOR
 */
static RsslRet encodeYieldCurveYieldCurve(RsslEncodeIterator* encodeIter, RsslDataDictionary* dictionary,RsslYieldCurveItem* ycItem, RsslEncodeIterator* summaryDataIter, RsslBool isRefresh)
{
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslFieldList fieldList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fieldEntry = RSSL_INIT_FIELD_ENTRY;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	RsslDateTime ycDate = RSSL_INIT_DATETIME;
	RsslYieldCurve *yc;

	rsslDateTimeGmtTime(&dateTime);
	rsslDateTimeGmtTime(&ycDate);

	/* YLD_CURVE */
	dictionaryEntry = dictionary->entriesArray[YLD_CURVE_FID];
	if (dictionaryEntry)
	{	
		int i = 0;

		fieldEntry.fieldId = YLD_CURVE_FID;
		fieldEntry.dataType = dictionaryEntry->rwfType;

		/* start encoding complex field entry */
		if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryInit() failed with return code: %d\n", ret);
			return ret;
		}

		if (isRefresh)
		{
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, YIELD_CURVE_REFRESH_SET_DEFS_DB, summaryDataIter)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			for (i = 0; i < YC_NUM; i++)
			{
				yc = &ycItem->yc[i];

				vectorEntry.flags = RSSL_VTEF_NONE;
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorEntry.index = i;

				if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
					return ret;
				}

				fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
				fieldList.setId = YIELD_CURVE_REFRESH_SET_DEFS_DB;
				/* start encoding Field List inside a Vector */
				if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
					return ret;
				}

				/* YCT_FWRATE */
				dictionaryEntry = dictionary->entriesArray[YCT_FWRATE_FID];
				if (dictionaryEntry)
				{	
					rsslClearFieldEntry(&fieldEntry);
					fieldEntry.fieldId = YCT_FWRATE_FID;
					fieldEntry.dataType = dictionaryEntry->rwfType;

					/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
					if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) == RSSL_RET_SUCCESS)
					{
						rsslClearArray(&rsslArray);
						/* now encode nested container using its own specific encode functions */
						/* encode RsslReal values into the array */
						rsslArray.primitiveType = RSSL_DT_REAL;
						/* values are variable length */
						rsslArray.itemLength = 0;
						/* begin encoding of array - using same encodeIter as field list */
						if ((ret = rsslEncodeArrayInit(encodeIter, &rsslArray)) == RSSL_RET_SUCCESS)
						{
							int j = 0;
							for (j = 0; j < MAX_ENTRIES; j++)
							{
								if ((ret = rsslEncodeArrayEntry(encodeIter, NULL, &yc->YCT_FWRATE[j])) < RSSL_RET_SUCCESS)
								{
									printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
									return ret;
								}
							}
						}
						else
						{
							printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
							return ret;
						}

						/* Complete nested container encoding */
						if ((ret = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
						{
							printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
							return ret;
						}
					}

					if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
						return ret;
					}
				}

				/* YCT_FWDATE */
				dictionaryEntry = dictionary->entriesArray[YCT_FWDATE_FID];
				if (dictionaryEntry)
				{	
					rsslClearFieldEntry(&fieldEntry);
					fieldEntry.fieldId = YCT_FWDATE_FID;
					fieldEntry.dataType = dictionaryEntry->rwfType;

					/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
					if((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) == RSSL_RET_SUCCESS)
					{
						rsslClearArray(&rsslArray);
						/* now encode nested container using its own specific encode functions */
						/* encode RsslReal values into the array */
						rsslArray.primitiveType = RSSL_DT_DATE;
						/* values are variable length */
						rsslArray.itemLength = 0;
						/* begin encoding of array - using same encIterator as field list */
						if ((ret = rsslEncodeArrayInit(encodeIter, &rsslArray)) == RSSL_RET_SUCCESS)
						{
							int j = 0;
							for (j = 0; j < MAX_ENTRIES; j++)
							{
								if ((ret = rsslEncodeArrayEntry(encodeIter, NULL, (void*)&yc->YCT_FWDATE[j])) < RSSL_RET_SUCCESS)
								{
									printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
									return ret;
								}
							}
						}
						else
						{
							printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
							return ret;
						}

						/* Complete nested container encoding */
						if ((ret = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
						{
							printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
							return ret;
						}
					}
				}

				if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
					return ret;
				}

				/* YCT_DATE */
				encodeFieldEntry(encodeIter, dictionary, YCT_DATE_FID, (void*)&yc->YCT_DATE);

				/* YCT_ZRATE */ 
				encodeFieldEntry(encodeIter, dictionary, YCT_ZRATE_FID, (void*)&yc->YCT_ZRATE);

				/* YCT_DISFAC */ 
				encodeFieldEntry(encodeIter, dictionary, YCT_DISFAC_FID, (void*)&yc->YCT_DISFAC);

				if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
					return ret;
				}

				if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
					return ret;
				}

				/* clear both vector entry and field list */
				rsslClearVectorEntry(&vectorEntry);
				rsslClearFieldList(&fieldList);
			} /* for loop end */

		}
		else
		{
			/* start encoding vector */
			if ((ret = encodeVectorInit(encodeIter, YIELD_CURVE_UPDATE_SET_DEFS_DB, 0)) < RSSL_RET_SUCCESS)
			{
				printf("encodeVectorInit() failed with return code: %d\n", ret);
				return ret;
			}

			for (i = 0; i < YC_NUM; i++)
			{
				yc = &ycItem->yc[i];

				vectorEntry.flags = RSSL_VTEF_NONE;
				vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;
				vectorEntry.index = i;

				if ((ret = rsslEncodeVectorEntryInit(encodeIter, &vectorEntry, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryInit() failed with return code: %d\n", ret);
					return ret;
				}

				fieldList.flags = RSSL_FLF_HAS_SET_ID | RSSL_FLF_HAS_SET_DATA;
				fieldList.setId = YIELD_CURVE_UPDATE_SET_DEFS_DB;
				/* start encoding Field List inside a Vector */
				if ((ret = rsslEncodeFieldListInit(encodeIter, &fieldList, &tmpYieldCurveSetDefDb, 0)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
					return ret;
				}

				/* YCT_FWRATE */
				dictionaryEntry = dictionary->entriesArray[YCT_FWRATE_FID];
				if (dictionaryEntry)
				{	
					rsslClearFieldEntry(&fieldEntry);
					fieldEntry.fieldId = YCT_FWRATE_FID;
					fieldEntry.dataType = dictionaryEntry->rwfType;

					/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
					if ((ret = rsslEncodeFieldEntryInit(encodeIter, &fieldEntry, 0)) == RSSL_RET_SUCCESS)
					{
						rsslClearArray(&rsslArray);
						/* now encode nested container using its own specific encode functions */
						/* encode RsslReal values into the array */
						rsslArray.primitiveType = RSSL_DT_REAL;
						/* values are variable length */
						rsslArray.itemLength = 0;
						/* begin encoding of array - using same encodeIter as field list */
						if ((ret = rsslEncodeArrayInit(encodeIter, &rsslArray)) == RSSL_RET_SUCCESS)
						{
							int j = 0;
							for (j = 0; j < MAX_ENTRIES; j++)
							{
								if ((ret = rsslEncodeArrayEntry(encodeIter, NULL, &yc->YCT_FWRATE[j])) < RSSL_RET_SUCCESS)
								{
									printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
									return ret;
								}
							}
						}
						else
						{
							printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
							return ret;
						}

						/* Complete nested container encoding */
						if ((ret = rsslEncodeArrayComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
						{
							printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
							return ret;
						}
					}

					if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
					{
						printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
						return ret;
					}
				}

				/* YCT_ZRATE */ 
				encodeFieldEntry(encodeIter, dictionary, YCT_ZRATE_FID, (void*)&yc->YCT_ZRATE);

				/* YCT_DISFAC */ 
				encodeFieldEntry(encodeIter, dictionary, YCT_DISFAC_FID, (void*)&yc->YCT_DISFAC);

				if ((ret = rsslEncodeFieldListComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
					return ret;
				}

				if ((ret = rsslEncodeVectorEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeVectorEntryComplete() failed with return code: %d\n", ret);
					return ret;
				}

				/* clear both vector entry and field list */
				rsslClearVectorEntry(&vectorEntry);
				rsslClearFieldList(&fieldList);

			} /* for loop end */
		}

		if ((ret = rsslEncodeVectorComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeVectorComplete() failed with return code: %d\n", ret);
			return ret;
		}

		if ((ret = rsslEncodeFieldEntryComplete(encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else
	{
		printf("FID %d not found in RDMFieldDictionary\n", CASH_RATES_FID);
		return RSSL_RET_FAILURE;
	}
		
	return ret;
}

/*
 * Initializes yield curve handler internal structures.
 */
void initYieldCurveItems()
{
	int i;

	/* clear item information list */
	for (i = 0; i < MAX_YIELD_CURVE_ITEM_LIST_SIZE; i++)
	{
		clearYieldCurveItem(&yieldCurveItemList[i]);
	}
}

/*
 * Initializes yield curve item fields.
 * ycItem - The item to be initialized
 */
void initYieldCurveItemFields(RsslYieldCurveItem* ycItem)
{
	int i;
	RsslYieldCurveForwardPeriod* fwdPeriod;
	RsslYieldCurveTenors* tenors;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	RsslDate tmpDate = RSSL_INIT_DATE;

	ycItem->isInUse = RSSL_TRUE;

	ycItem->CRV_ID = 0;
	ycItem->SPOT_LAG = 2;

	for (i = 0; i < FWD_PERIOD_NUM; i++)
	{
		fwdPeriod = &ycItem->fwdPeriod[i];
		snprintf(fwdPeriod->FWD_PERIOD, MAX_CRV_NAME_STRLEN, "Y%d", i+1);
	}

	tenors = &ycItem->tenors[0];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "Y1");
	tenors = &ycItem->tenors[1];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "15 MAY 2013 6M");
	tenors = &ycItem->tenors[2];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "15 MAY 2014");
	tenors = &ycItem->tenors[3];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "2Y");
	tenors = &ycItem->tenors[4];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "05 MAY 2015");
	tenors = &ycItem->tenors[5];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "3Y");
	tenors = &ycItem->tenors[6];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "4Y");
	tenors = &ycItem->tenors[7];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "5Y");
	tenors = &ycItem->tenors[8];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "6Y");
	tenors = &ycItem->tenors[9];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "7Y");
	tenors = &ycItem->tenors[10];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "8Y");
	tenors = &ycItem->tenors[11];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "9Y");
	tenors = &ycItem->tenors[12];
	snprintf(tenors->TENORS, MAX_CRV_NAME_STRLEN, "10Y");


	snprintf(ycItem->CRV_ALGTHM, MAX_CRV_NAME_STRLEN, "Refinitiv UPA VA Yield Curve Provider");
	snprintf(ycItem->CRV_TYPE, MAX_CRV_NAME_STRLEN, "Swap");
	snprintf(ycItem->CRV_STYPE, MAX_CRV_NAME_STRLEN, "Standard");
	snprintf(ycItem->CITIES, MAX_CRV_NAME_STRLEN, "US");
	snprintf(ycItem->INTER_MTHD, MAX_CRV_NAME_STRLEN, "");
	snprintf(ycItem->EXTRP_MTHD, MAX_CRV_NAME_STRLEN, "");
	snprintf(ycItem->CC_METHOD, MAX_CRV_NAME_STRLEN, "Bootstrap");
	snprintf(ycItem->ROLL_CONV, MAX_CRV_NAME_STRLEN, "Modified Following");
	snprintf(ycItem->ZC_BASIS, MAX_CRV_NAME_STRLEN, "30/360(BOND)");
	snprintf(ycItem->DSCT_FACT, MAX_CRV_NAME_STRLEN, "Compound");
	snprintf(ycItem->DSCT_BASIS, MAX_CRV_NAME_STRLEN, "30/360(BOND)");
	snprintf(ycItem->CURVE_STS, MAX_CRV_NAME_STRLEN, "Created");
	snprintf(ycItem->USER_ID, MAX_CRV_NAME_STRLEN, "ADMIN");
	snprintf(ycItem->MOD_USERID, MAX_CRV_NAME_STRLEN, "ADMIN");
	snprintf(ycItem->COMMENT, MAX_CRV_NAME_STRLEN, "");
	snprintf(ycItem->FWD_BASIS, MAX_CRV_NAME_STRLEN, "ACT/360");
	snprintf(ycItem->CASH_BASIS, MAX_CRV_NAME_STRLEN, "ACT/360.25");
	snprintf(ycItem->CASH_INTER_MTHD, MAX_CRV_NAME_STRLEN, "Linear");
	snprintf(ycItem->FUTR_BASIS, MAX_CRV_NAME_STRLEN, "ACT/360.25");
	snprintf(ycItem->FUTR_INTER_MTHD, MAX_CRV_NAME_STRLEN, "Linear");
	snprintf(ycItem->SWAP_INTER_MTHD_LT, MAX_CRV_NAME_STRLEN, "Linear");
	snprintf(ycItem->SWAP_FREQ_ST, MAX_CRV_NAME_STRLEN, "N/A");
	snprintf(ycItem->SWAP_BASIS_LT, MAX_CRV_NAME_STRLEN, "ACT/360");
	snprintf(ycItem->SWAP_FREQ_LT, MAX_CRV_NAME_STRLEN, "Monthly");

	rsslDateTimeGmtTime(&dateTime);
	ycItem->CRV_DATE = dateTime;
	ycItem->CRT_TIME = dateTime;
	ycItem->MOD_TIME = dateTime;
	ycItem->ACTIVE_DATE = dateTime.date;
	ycItem->SETTL_DATE = dateTime.date;
	ycItem->VAL_DATE = dateTime.date;
	ycItem->TRADE_DATE = dateTime.date;
	ycItem->TIMACT = dateTime.time;

	ycItem->CASH_RATE.hint = RSSL_RH_EXPONENT_2;
	ycItem->CASH_RATE.value = rand() % 1000;
	ycItem->CASH_RATE.isBlank = RSSL_FALSE;

	ycItem->SPRD_RATE.hint = RSSL_RH_EXPONENT_2;
	ycItem->SPRD_RATE.value = rand() % 1000;
	ycItem->SPRD_RATE.isBlank = RSSL_FALSE;

	tmpDate = dateTime.date;
	/* Initializes fields for the orders associated with the item. */
	for (i = 0; i < YC_NUM; i++)
	{
		int j;
		RsslYieldCurve *yc = &ycItem->yc[i];

		yc->YCT_DATE = tmpDate;

		yc->YCT_ZRATE.hint = RSSL_RH_EXPONENT_2;
		yc->YCT_ZRATE.value = rand() % 1000;
		yc->YCT_ZRATE.isBlank = RSSL_FALSE;

		yc->YCT_DISFAC.hint = RSSL_RH_EXPONENT_2;
		yc->YCT_DISFAC.value = rand() % 1000;
		yc->YCT_DISFAC.isBlank = RSSL_FALSE;

		yc->YCT_YTM.hint = RSSL_RH_EXPONENT_2;
		yc->YCT_YTM.value = rand() % 1000;
		yc->YCT_YTM.isBlank = RSSL_FALSE;	

		for (j = 0; j < MAX_ENTRIES; j++)
		{
			yc->YCT_FWDATE[j] = tmpDate;

			yc->YCT_FWRATE[j].hint = RSSL_RH_EXPONENT_2;
			yc->YCT_FWRATE[j].value = rand() % 1000;
			yc->YCT_FWRATE[j].isBlank = RSSL_FALSE;

			tmpDate.year++;
		}
	}

	tmpDate = dateTime.date;
	for (i = 0; i < SWAP_RATES_NUM; i++)
	{
		RsslYieldCurveSwapRates *sr = &ycItem->swapRates[i];

		sr->SWAP_SDATE = tmpDate;
		sr->SWAP_MDATE = tmpDate;
		sr->SWAP_RDATE = tmpDate;

		sr->SWAP_RATE_VAL.hint = RSSL_RH_EXPONENT_2;
		sr->SWAP_RATE_VAL.value = rand() % 1000;
		sr->SWAP_RATE_VAL.isBlank = RSSL_FALSE;
		
		snprintf(sr->SWAP_SRC, MAX_CRV_NAME_STRLEN, "USDAM3L%dY=", i);

		tmpDate.year++;
	}

	tmpDate = dateTime.date;
	{
		RsslYieldCurveCashRates *cr = &ycItem->cashRates;
		RsslDate tmpDate = dateTime.date;

		cr->CASH_SDATE = dateTime.date;
		cr->CASH_MDATE = tmpDate;

		cr->CASH_RATE.hint = RSSL_RH_EXPONENT_2;
		cr->CASH_RATE.value = rand() % 1000;
		cr->CASH_RATE.isBlank = RSSL_FALSE;
		
		snprintf(cr->CASH_SRC, MAX_CRV_NAME_STRLEN, "USDAM3L1Y=");
	}

	tmpDate = dateTime.date;
	{
		RsslYieldCurveFuturePrices *fp = &ycItem->futurePrices;
		RsslDate tmpDate = dateTime.date;

		fp->FUTR_SDATE = dateTime.date;
		fp->FUTR_MDATE = tmpDate;

		fp->FUTR_PRICE.hint = RSSL_RH_EXPONENT_2;
		fp->FUTR_PRICE.value = rand() % 1000;
		fp->FUTR_PRICE.isBlank = RSSL_FALSE;
		
		snprintf(fp->FUTR_SRC, MAX_CRV_NAME_STRLEN, "USDAM3L0Y=");
	}
}

/*
 * Updates any item that's currently in use.
 */
void updateYieldCurveItemFields(RsslYieldCurveItem* itemInfo)
{
	int i;
	RsslYieldCurveFuturePrices *fp = &itemInfo->futurePrices;
	RsslYieldCurveCashRates *cr = &itemInfo->cashRates;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	
	rsslDateTimeGmtTime(&dateTime);

	itemInfo->TIMACT = dateTime.time;
	itemInfo->CRV_ID++;
	fp->FUTR_PRICE.value++;
	cr->CASH_RATE.value++;

	for (i = 0; i < YC_NUM; i++)
	{
		int j;
		RsslYieldCurve *yc = &itemInfo->yc[i];
		yc->YCT_ZRATE.value++;
		yc->YCT_DISFAC.value++;
		yc->YCT_YTM.value++;

		for (j = 0; j < MAX_ENTRIES; j++)
		{
			yc->YCT_FWRATE[j].value++;
		}
	}

	for (i = 0; i < SWAP_RATES_NUM; i++)
	{
		RsslYieldCurveSwapRates *sr = &itemInfo->swapRates[i];
		sr->SWAP_RATE_VAL.value++;
	}
}

/*
 * Encodes the yield curve response.  Returns success if
 * encoding succeeds or failure if encoding fails.
 * chnl - The channel to send a yield curve response to
 * itemInfo - The item information
 * isSolicited - The response is solicited if set
 * msgBuf - The message buffer to encode the yield curve response into
 * streamId - The stream id of the yield curve response
 * isStreaming - Flag for streaming or snapshot
 * serviceId - The service id of the yield curve response
 * dictionary - The dictionary used for encoding
 */
RsslRet encodeYieldCurveResponse(RsslReactorChannel* pReactorChannel, RsslItemInfo* itemInfo, RsslBuffer* msgBuf, RsslBool isSolicited, RsslInt32 streamId, RsslBool isStreaming, RsslBool isPrivateStream, RsslUInt16 serviceId, RsslDataDictionary* dictionary)
{
	RsslRet ret = 0;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslUpdateMsg updateMsg = RSSL_INIT_UPDATE_MSG;
	RsslMsgBase* msgBase;
	RsslMsg* msg;
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	char stateText[MAX_ITEM_INFO_STRLEN];
	RsslDictionaryEntry* dictionaryEntry = NULL;
	RsslEncodeIterator encodeIter;
	RsslYieldCurveItem *ycItem;
	int i;
	RsslDateTime dateTime = RSSL_INIT_DATETIME;
	RsslBuffer tmpBuf;
	RsslBuffer summaryData;
	RsslEncodeIterator summaryDataEncodeIter;


	/* clear encode iterator */
	rsslClearEncodeIterator(&encodeIter);
	rsslClearEncodeIterator(&summaryDataEncodeIter);

	ycItem = (RsslYieldCurveItem*)itemInfo->itemData;

	/* set-up message */
	/* set message depending on whether refresh or update */
	if (!itemInfo->IsRefreshComplete) /* this is a refresh message */
	{
		msgBase = &refreshMsg.msgBase;
		msgBase->msgClass = RSSL_MC_REFRESH;

		if (isStreaming)
		{
			refreshMsg.state.streamState = RSSL_STREAM_OPEN;
		}
		else
		{
			refreshMsg.state.streamState = RSSL_STREAM_NON_STREAMING;
		}
		
		refreshMsg.state.dataState = RSSL_DATA_OK;
		refreshMsg.state.code = RSSL_SC_NONE;

		if (isSolicited)
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}
		else
		{
			refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE;
		}

		if (isPrivateStream)
		{
			refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;
		}

		sprintf(stateText, "Item Refresh Completed");
		refreshMsg.state.text.data = stateText;
		refreshMsg.state.text.length = (RsslUInt32)strlen(stateText);
		msgBase->msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE;
		
		/* ServiceId */
		msgBase->msgKey.serviceId = serviceId;
		
		/* Itemname */
		msgBase->msgKey.name.data = itemInfo->Itemname;
		msgBase->msgKey.name.length = (RsslUInt32)strlen(itemInfo->Itemname);
		msgBase->msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
		
		/* Qos */
		refreshMsg.qos.dynamic = RSSL_FALSE;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		msg = (RsslMsg *)&refreshMsg;
	}
	else /* this is an update message */
	{
		msgBase = &updateMsg.msgBase;
		msgBase->msgClass = RSSL_MC_UPDATE;
		msg = (RsslMsg *)&updateMsg;
	}

	msgBase->domainType = RSSL_DMT_YIELD_CURVE;
	msgBase->containerType = RSSL_DT_FIELD_LIST;

	/* StreamId */
	msgBase->streamId = streamId;

	/* encode message */
	rsslSetEncodeIteratorBuffer(&encodeIter, msgBuf);
	rsslSetEncodeIteratorRWFVersion(&encodeIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion);
	if ((ret = rsslEncodeMsgInit(&encodeIter, msg, 0)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgInit() failed with return code: %d\n", ret);
		return ret;
	}

	if (!itemInfo->IsRefreshComplete) /* this is a refresh message */
	{
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* encode items for private stream */
		if (isPrivateStream)
		{
			/* TIMACT */
			encodeFieldEntry(&encodeIter, dictionary, TIMACT_FID, (void*)&ycItem->TIMACT);
		}

		/* CRV ID */
		encodeFieldEntry(&encodeIter, dictionary, CRV_ID_FID, (void*)&ycItem->CRV_ID);

		/* SPOT_LAG */
		encodeFieldEntry(&encodeIter, dictionary, SPOT_LAG_FID, (void*)&ycItem->SPOT_LAG);

		/* CRV_DATE */
		encodeFieldEntry(&encodeIter, dictionary, CRV_DATE_FID, (void*)&ycItem->CRV_DATE);

		/* SETTL_DATE */
		encodeFieldEntry(&encodeIter, dictionary, SETTL_DATE_FID, (void*)&ycItem->SETTL_DATE);

		/* VAL_DATE */
		encodeFieldEntry(&encodeIter, dictionary, VAL_DATE_FID, (void*)&ycItem->VAL_DATE);

		/* CRT_TIME */
		encodeFieldEntry(&encodeIter, dictionary, CRT_TIME_FID, (void*)&ycItem->CRT_TIME);

		/* MOD_TIME */
		encodeFieldEntry(&encodeIter, dictionary, MOD_TIME_FID, (void*)&ycItem->MOD_TIME);

		/* TRADE_DATE */
		encodeFieldEntry(&encodeIter, dictionary, TRADE_DATE_FID, (void*)&ycItem->TRADE_DATE);

		/* CASH_RATE */
		encodeFieldEntry(&encodeIter, dictionary, CASH_RATE_FID, (void*)&ycItem->CASH_RATE);

		/* SPRD_RATE */ 
		encodeFieldEntry(&encodeIter, dictionary, SPRD_RATE_FID, (void*)&ycItem->SPRD_RATE);

		/* ACTIVE DATE */
		encodeFieldEntry(&encodeIter, dictionary, ACTIVE_DATE_FID, (void*)&ycItem->ACTIVE_DATE);

		/* CRV_NAME */
		tmpBuf.data = itemInfo->Itemname;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CRV_NAME_FID, (void*)&tmpBuf);

		/* CRV_TYPE */
		tmpBuf.data = ycItem->CRV_TYPE;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CRV_TYPE_FID, (void*)&tmpBuf);

		/* CRV_STYPE */
		tmpBuf.data = ycItem->CRV_STYPE;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CRV_STYPE_FID, (void*)&tmpBuf);

		/* CITIES */
		tmpBuf.data = ycItem->CITIES;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CITIES_FID, (void*)&tmpBuf);

		/* CRV_ALGTHM */
		tmpBuf.data = ycItem->CRV_ALGTHM;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CRV_ALGTHM_FID, (void*)&tmpBuf);

		/* INTER_MTHD */
		tmpBuf.data = ycItem->INTER_MTHD;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, INTER_MTHD_FID, (void*)&tmpBuf);

		/* EXTRP_MTHD */
		tmpBuf.data = ycItem->EXTRP_MTHD;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, EXTRP_MTHD_FID, (void*)&tmpBuf);

		/* CC_METHOD */
		tmpBuf.data = ycItem->CC_METHOD;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CC_METHOD_FID, (void*)&tmpBuf);

		/* ROLL_CONV */
		tmpBuf.data = ycItem->ROLL_CONV;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, ROLL_CONV_FID, (void*)&tmpBuf);

		/* ZC_BASIS */
		tmpBuf.data = ycItem->ZC_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, ZC_BASIS_FID, (void*)&tmpBuf);

		/* DSCT_FACT */
		tmpBuf.data = ycItem->DSCT_FACT;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, DSCT_FACT_FID, (void*)&tmpBuf);

		/* DSCT_BASIS */
		tmpBuf.data = ycItem->DSCT_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, DSCT_BASIS_FID, (void*)&tmpBuf);

		/* CURVE_STS */
		tmpBuf.data = ycItem->CURVE_STS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CURVE_STS_FID, (void*)&tmpBuf);

		/* USER_ID */
		tmpBuf.data = ycItem->USER_ID;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, USER_ID_FID, (void*)&tmpBuf);

		/* MOD_USERID */
		tmpBuf.data = ycItem->MOD_USERID;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, MOD_USERID_FID, (void*)&tmpBuf);

		/* COMMENT */
		tmpBuf.data = ycItem->COMMENT;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, COMMENT_FID, (void*)&tmpBuf);

		/* FWD_BASIS */
		tmpBuf.data = ycItem->FWD_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, FWD_BASIS_FID, (void*)&tmpBuf);

		/* CASH_BASIS */
		tmpBuf.data = ycItem->CASH_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CASH_BASIS_FID, (void*)&tmpBuf);

		/* CASH_INTER_MTHD */
		tmpBuf.data = ycItem->CASH_INTER_MTHD;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, CASH_INTER_MTHD_FID, (void*)&tmpBuf);

		/* FUTR_BASIS */
		tmpBuf.data = ycItem->FUTR_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, FUTR_BASIS_FID, (void*)&tmpBuf);

		/* FUTR_INTER_MTHD */
		tmpBuf.data = ycItem->FUTR_INTER_MTHD;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, FUTR_INTER_MTHD_FID, (void*)&tmpBuf);

		/* SWAP_INTER_MTHD_LT */
		tmpBuf.data = ycItem->SWAP_INTER_MTHD_LT;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, SWAP_INTER_MTHD_LT_FID, (void*)&tmpBuf);

		/* ZC_BASIS */
		tmpBuf.data = ycItem->ZC_BASIS;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, ZC_BASIS_FID, (void*)&tmpBuf);

		/* SWAP_FREQ_ST */
		tmpBuf.data = ycItem->SWAP_FREQ_ST;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, SWAP_FREQ_ST_FID, (void*)&tmpBuf);

		/* SWAP_BASIS_LT */
		tmpBuf.data = ycItem->SWAP_BASIS_LT;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, SWAP_BASIS_LT_FID, (void*)&tmpBuf);

		/* SWAP_FREQ_LT */
		tmpBuf.data = ycItem->SWAP_FREQ_LT;
		tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
		encodeFieldEntry(&encodeIter, dictionary, SWAP_FREQ_LT_FID, (void*)&tmpBuf);

		/* FWD_PERIOD */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[FWD_PERIOD_FID];
		if (dictionaryEntry)
		{	
			fEntry.fieldId = FWD_PERIOD_FID;
			fEntry.dataType = dictionaryEntry->rwfType;

			/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
			if ((ret = rsslEncodeFieldEntryInit(&encodeIter, &fEntry, 0)) == RSSL_RET_SUCCESS)
			{
				/* now encode nested container using its own specific encode functions */
				/* encode RsslReal values into the array */
				rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
				/* values are variable length */
				rsslArray.itemLength = 0;
				/* begin encoding of array - using same encIterator as field list */
				if ((ret = rsslEncodeArrayInit(&encodeIter, &rsslArray)) == RSSL_RET_SUCCESS)
				{
					RsslYieldCurveForwardPeriod* fwdPeriod;
					for (i = 0; i < FWD_PERIOD_NUM; i++)
					{
						fwdPeriod = &ycItem->fwdPeriod[i];
						tmpBuf.data = fwdPeriod->FWD_PERIOD;
						tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
						if ((ret = rsslEncodeArrayEntry(&encodeIter, NULL, &tmpBuf)) < RSSL_RET_SUCCESS)
						{
							printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
							return ret;
						}
					}
				}
				else
				{
					printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
					return ret;
				}
				/* Complete nested container encoding */
				if ((ret = rsslEncodeArrayComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
					return ret;
				}
			}
		}

		if ((ret = rsslEncodeFieldEntryComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
			return ret;
		}

		encodeYieldCurveSwapRates(&encodeIter, dictionary, ycItem, RSSL_TRUE);

		encodeYieldCurveFuturePrices(&encodeIter, dictionary, ycItem, RSSL_TRUE);

		encodeYieldCurveCashRates(&encodeIter, dictionary, ycItem, RSSL_TRUE);

		/* encode summary data for yield curve */
		summaryData.data = (char*) malloc(200);
		summaryData.length = 200;
		rsslSetEncodeIteratorBuffer(&summaryDataEncodeIter, &summaryData);

		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(&summaryDataEncodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

		/* TENORS_FID */
		rsslClearFieldEntry(&fEntry);
		dictionaryEntry = dictionary->entriesArray[TENORS_FID];
		if (dictionaryEntry)
		{	
			fEntry.fieldId = TENORS_FID;
			fEntry.dataType = dictionaryEntry->rwfType;

			/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
			if ((ret = rsslEncodeFieldEntryInit(&summaryDataEncodeIter, &fEntry, 0)) == RSSL_RET_SUCCESS)
			{
				/* now encode nested container using its own specific encode functions */
				/* encode RsslReal values into the array */
				rsslArray.primitiveType = RSSL_DT_ASCII_STRING;
				/* values are variable length */
				rsslArray.itemLength = 0;
				/* begin encoding of array - using same encIterator as field list */
				if ((ret = rsslEncodeArrayInit(&summaryDataEncodeIter, &rsslArray)) == RSSL_RET_SUCCESS)
				{
					RsslYieldCurveTenors* tenors;
					for (i = 0; i < TENORS_NUM; i++)
					{
						tenors = &ycItem->tenors[i];
						tmpBuf.data = tenors->TENORS;
						tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
						if ((ret = rsslEncodeArrayEntry(&summaryDataEncodeIter, NULL, &tmpBuf)) < RSSL_RET_SUCCESS)
						{
							printf("rsslEncodeArrayEntry() failed with return code: %d\n", ret);
							return ret;
						}
					}
				}
				else
				{
					printf("rsslEncodeArrayInit() failed with return code: %d\n", ret);
					return ret;
				}
				/* Complete nested container encoding */
				if ((ret = rsslEncodeArrayComplete(&summaryDataEncodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
				{
					printf("rsslEncodeArrayComplete() failed with return code: %d\n", ret);
					return ret;
				}
			}
			else
			{
				printf("rsslEncodeFieldEntryInit() failed with return code: %d\n", ret);
				return ret;
			}

			/* Complete nested container encoding */
			if ((ret = rsslEncodeFieldEntryComplete(&summaryDataEncodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldEntryComplete() failed with return code: %d\n", ret);
				return ret;
			}

			/* Complete nested container encoding */
			if ((ret = rsslEncodeFieldListComplete(&summaryDataEncodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
			{
				printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
				return ret;
			}
		}

		encodeYieldCurveYieldCurve(&encodeIter, dictionary, ycItem, &summaryDataEncodeIter, RSSL_TRUE);

		/* free the memory */
		free(summaryData.data);

		if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}
	else /* Update */
	{
		rsslClearFieldList(&fList);
		fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		if ((ret = rsslEncodeFieldListInit(&encodeIter, &fList, 0, 0)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListInit() failed with return code: %d\n", ret);
			return ret;
		}

                /* encode items for private stream */
                if (isPrivateStream)
                {
			/* TIMACT */
			encodeFieldEntry(&encodeIter, dictionary, TIMACT_FID, (void*)&ycItem->TIMACT);
		}

		/* CRV ID */
		encodeFieldEntry(&encodeIter, dictionary, CRV_ID_FID, (void*)&ycItem->CRV_ID);

		encodeYieldCurveSwapRates(&encodeIter, dictionary, ycItem, RSSL_FALSE);

		encodeYieldCurveFuturePrices(&encodeIter, dictionary, ycItem, RSSL_FALSE);

		encodeYieldCurveCashRates(&encodeIter, dictionary, ycItem, RSSL_FALSE);

		encodeYieldCurveYieldCurve(&encodeIter, dictionary, ycItem, 0, RSSL_FALSE);

		if ((ret = rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
		{
			printf("rsslEncodeFieldListComplete() failed with return code: %d\n", ret);
			return ret;
		}
	}

	/* complete encode message */
	if ((ret = rsslEncodeMsgComplete(&encodeIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("rsslEncodeMsgComplete() failed with return code: %d\n", ret);
		return ret;
	}

	msgBuf->length = rsslGetEncodedBufferLength(&encodeIter);

	return RSSL_RET_SUCCESS;
}

/*
 * Gets storage for a yield curve item from the list.
 * itemName - The item name of the item 
 */
RsslYieldCurveItem* getYieldCurveItem(char* itemName)
{
	int i;
	RsslYieldCurveItem* itemInfo = NULL;

	/* first check for existing item */
	for (i = 0; i < MAX_YIELD_CURVE_ITEM_LIST_SIZE; i++)
	{
		if (!yieldCurveItemList[i].isInUse)
		{
			initYieldCurveItemFields(&yieldCurveItemList[i]);
			return &yieldCurveItemList[i];
		}
	}

	return NULL;
}

/*
 * Frees an item information structure.
 * itemInfo - The item information structure to free
 */
void freeYieldCurveItem(RsslYieldCurveItem* ycItem)
{
	clearYieldCurveItem(ycItem);
}
