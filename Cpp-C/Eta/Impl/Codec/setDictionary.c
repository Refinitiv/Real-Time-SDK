/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdarg.h>

#include "rtr/rsslSetData.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslPrimitiveEncoders.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/encoderTools.h"

#define LOCAL_EL_DEFS_TLS_SIZE 3825

static RsslElementSetDefEntry rsslFieldSetDefEntries[3] = 
{
	{ { 10,"NUMENTRIES"}, RSSL_DT_INT},
	{ { 4, "FIDS"}, RSSL_DT_ARRAY },
	{ { 5, "TYPES"}, RSSL_DT_ARRAY }
};

static const RsslElementSetDef fieldSetDef0_Verbose = 
{
	0,
	3,
	rsslFieldSetDefEntries
};

static RsslElementSetDefEntry rsslElementSetDefEntries[3] = 
{
	{ { 10,"NUMENTRIES"}, RSSL_DT_INT},
	{ { 5, "NAMES"}, RSSL_DT_ARRAY },
	{ { 5, "TYPES"}, RSSL_DT_ARRAY }
};

static const RsslElementSetDef elementSetDef0_Verbose = 
{
	0,
	3,
	rsslElementSetDefEntries
};

void _setDbError(RsslBuffer *errorText, char *format, ...)
{
	if (errorText && errorText->data)
	{
    	va_list fmtArgs; 
    	va_start(fmtArgs, format);
		vsnprintf(errorText->data, errorText->length, format, fmtArgs);
		va_end(fmtArgs);
	}
}

RsslRet _decodeFieldSetDefTag(RsslDecodeIterator *dIter, RsslElementEntry *element, RsslFieldSetDefDb* pFieldSetDefDb, RsslBuffer* errorText)
{
	RsslRet ret;
	RsslUInt tempUInt;
	
	if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_TYPE))
	{
		if((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
		{
			return (_setDbError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		}
		if (tempUInt != RDM_DICTIONARY_FIELD_SET_DEFINITION)
			return (_setDbError(errorText, "Type '" RTR_LLU "' indicates this is not a set data definitions dictionary.", tempUInt), RSSL_RET_FAILURE);
	}
	else if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_VERSION))
	{
		/* just memcpy the string here */
		if(pFieldSetDefDb->info_version.length != 0)
		{	
			return (_setDbError(errorText, "Version Buffer already present"), RSSL_RET_FAILURE);
		}
		
		if((pFieldSetDefDb->info_version.data = (char*)malloc(element->encData.length)) == 0)
		{
			return (_setDbError(errorText, "malloc failed to for version copy"), RSSL_RET_FAILURE);
		}

		memcpy(pFieldSetDefDb->info_version.data, element->encData.data, element->encData.length);
		pFieldSetDefDb->info_version.length =  element->encData.length;
	}
	else if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICTIONARY_ID))
	{
		if((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
		{
			return (_setDbError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		}

		pFieldSetDefDb->info_DictionaryId = (RsslInt32)tempUInt;
	}
	
	return RSSL_RET_SUCCESS;
}

RsslRet _decodeElementSetDefTag(RsslDecodeIterator *dIter, RsslElementEntry *element, RsslElementSetDefDb* pElementSetDefDb, RsslBuffer* errorText)
{
	RsslRet ret;
	RsslUInt tempUInt;
	
	if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_TYPE))
	{
		if((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
		{
			return (_setDbError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		}
		if (tempUInt != RDM_DICTIONARY_ELEMENT_SET_DEFINITION)
			return (_setDbError(errorText, "Type '" RTR_LLU "' indicates this is not a set data definitions dictionary.", tempUInt), RSSL_RET_FAILURE);
	}
	else if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_VERSION))
	{
		/* just memcpy the string here */
		if(pElementSetDefDb->info_version.length != 0)
		{	
			return (_setDbError(errorText, "Version Buffer already present"), RSSL_RET_FAILURE);
		}
		
		if((pElementSetDefDb->info_version.data = (char*)malloc(element->encData.length)) == 0)
		{
			return (_setDbError(errorText, "malloc failed to for version copy"), RSSL_RET_FAILURE);
		}

		memcpy(pElementSetDefDb->info_version.data, element->encData.data, element->encData.length);
		pElementSetDefDb->info_version.length =  element->encData.length;
	}
	else if(rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICTIONARY_ID))
	{
		if((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
		{
			return (_setDbError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		}

		pElementSetDefDb->info_DictionaryId = (RsslInt32)tempUInt;
	}
	
	return RSSL_RET_SUCCESS;
}
	
RSSL_API RsslRet rsslAllocateFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslBuffer* version)
{
	if(pFieldSetDefDb->isInitialized == RSSL_TRUE)
		return RSSL_RET_INVALID_ARGUMENT;
		
	pFieldSetDefDb->definitions = (RsslFieldSetDef**)malloc(sizeof(RsslFieldSetDef*)*(RSSL_MAX_GLOBAL_SET_ID+1));
	if(pFieldSetDefDb->definitions == NULL)
		return RSSL_RET_FAILURE;
	memset(pFieldSetDefDb->definitions, 0x00, sizeof(RsslFieldSetDef*)*(RSSL_MAX_GLOBAL_SET_ID+1));

	if(version->data && version->length != 0)
	{
		pFieldSetDefDb->info_version.data = malloc(sizeof(char)*version->length);
		if(pFieldSetDefDb->info_version.data == NULL)
		{
			free(pFieldSetDefDb->definitions);
			return RSSL_RET_FAILURE;
		}
	}
	else
	{
		pFieldSetDefDb->info_version.data = 0;
		pFieldSetDefDb->info_version.length = 0;
	}

	if (pFieldSetDefDb->info_version.data)
	{
		memcpy(pFieldSetDefDb->info_version.data, version->data, version->length);
		pFieldSetDefDb->info_version.length = version->length;
	}
	
	pFieldSetDefDb->maxSetId = 0;
	pFieldSetDefDb->isInitialized = RSSL_TRUE;
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslAddFieldSetDefToDb(RsslFieldSetDefDb* pFieldSetDefDb, RsslFieldSetDef* pFieldSetDef)
{
	int i;
	RsslFieldSetDef* tempDef;

	RSSL_ASSERT(pFieldSetDefDb && pFieldSetDef, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pFieldSetDef->setId < RSSL_MAX_GLOBAL_SET_ID, Invalid setId. Maximum is 65535);

	if(pFieldSetDefDb->isInitialized != RSSL_TRUE)
		return RSSL_RET_INVALID_ARGUMENT;

	if((tempDef = (RsslFieldSetDef*)malloc(sizeof(RsslFieldSetDef))) == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	tempDef->count = pFieldSetDef->count;
	tempDef->setId = pFieldSetDef->setId;
	
	if((tempDef->pEntries = (RsslFieldSetDefEntry*)malloc(sizeof(RsslFieldSetDefEntry)*tempDef->count)) == NULL)
	{
		free(tempDef);
		return RSSL_RET_FAILURE;
	}

	for(i = 0; i < tempDef->count; i++)
	{
		tempDef->pEntries[i].dataType = pFieldSetDef->pEntries[i].dataType;
		tempDef->pEntries[i].fieldId = pFieldSetDef->pEntries[i].fieldId;
	}

	pFieldSetDefDb->definitions[tempDef->setId] = tempDef;

	if(pFieldSetDefDb->maxSetId < tempDef->setId)
		pFieldSetDefDb->maxSetId = tempDef->setId;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslAllocateElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb, RsslBuffer* version)
{
	if(pElementSetDefDb->isInitialized == RSSL_TRUE)
		return RSSL_RET_INVALID_ARGUMENT;
		
	pElementSetDefDb->definitions = (RsslElementSetDef**)malloc(sizeof(RsslElementSetDef*)*(RSSL_MAX_GLOBAL_SET_ID+1));
	if(pElementSetDefDb->definitions == NULL)
		return RSSL_RET_FAILURE;
	memset(pElementSetDefDb->definitions, 0x00, sizeof(RsslElementSetDef*)*(RSSL_MAX_GLOBAL_SET_ID+1));
	
	if(version->data && version->length != 0)
	{
		pElementSetDefDb->info_version.data = (char*)malloc(sizeof(char)*version->length);
		if(pElementSetDefDb->info_version.data == NULL)
		{
			free(pElementSetDefDb->definitions);
			return RSSL_RET_SUCCESS;
		}

		memcpy(pElementSetDefDb->info_version.data, version->data, version->length);
		pElementSetDefDb->info_version.length = version->length;
	}
	else
	{
		pElementSetDefDb->info_version.data = 0;
		pElementSetDefDb->info_version.length = 0;
	}

	pElementSetDefDb->maxSetId = 0;
	pElementSetDefDb->isInitialized = RSSL_TRUE;
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslAddElementSetDefToDb(RsslElementSetDefDb* pElementSetDefDb, RsslElementSetDef* pElementSetDef)
{
	int i, j;
	RsslElementSetDef* tempDef;
	tempDef = 0;

	RSSL_ASSERT(pElementSetDefDb && pElementSetDef, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(pElementSetDef->setId < RSSL_MAX_GLOBAL_SET_ID, Invalid setId. Maximum is 65535);

	if(pElementSetDefDb->isInitialized != RSSL_TRUE)
		return RSSL_RET_INVALID_ARGUMENT;

	if((tempDef = (RsslElementSetDef*)malloc(sizeof(RsslElementSetDef))) == NULL)
	{
		return RSSL_RET_FAILURE;
	}

	tempDef->count = pElementSetDef->count;
	tempDef->setId = pElementSetDef->setId;
	
	if((tempDef->pEntries = (RsslElementSetDefEntry*)malloc(sizeof(RsslElementSetDefEntry)*tempDef->count)) == NULL)
	{
		free(tempDef);
		return RSSL_RET_FAILURE;
	}

	for(i = 0; i < tempDef->count; i++)
	{
		tempDef->pEntries[i].dataType = pElementSetDef->pEntries[i].dataType;

		if((tempDef->pEntries[i].name.data = (char*)malloc(sizeof(char)*pElementSetDef->pEntries[i].name.length)) == NULL)
		{
			for(j = 0; j < i; j++)
			{
				free(tempDef->pEntries[j].name.data);
			}
			free(tempDef);
			return RSSL_RET_FAILURE;
		}

		memcpy(tempDef->pEntries[i].name.data, pElementSetDef->pEntries[i].name.data, pElementSetDef->pEntries[i].name.length);
		tempDef->pEntries[i].name.length = pElementSetDef->pEntries[i].name.length;
	}

	pElementSetDefDb->definitions[tempDef->setId] = tempDef;

	if(pElementSetDefDb->maxSetId < tempDef->setId)
		pElementSetDefDb->maxSetId = tempDef->setId;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDeleteFieldSetDefDb(RsslFieldSetDefDb* pFieldSetDefDb)
{
	int setIter;
	
	RSSL_ASSERT(pFieldSetDefDb, Invalid parameters or parameters passed in as NULL);

	for(setIter = 0; setIter <= pFieldSetDefDb->maxSetId; setIter++)
	{
		if(pFieldSetDefDb->definitions[setIter] != NULL)
		{
			free(pFieldSetDefDb->definitions[setIter]->pEntries);
			free(pFieldSetDefDb->definitions[setIter]);
			pFieldSetDefDb->definitions[setIter] = 0;
		}
	}
	
	free(pFieldSetDefDb->definitions);

	if(pFieldSetDefDb->info_version.data != NULL)
	{
		free(pFieldSetDefDb->info_version.data);
	}

	pFieldSetDefDb->info_version.data = 0;
	pFieldSetDefDb->info_version.length = 0;
	pFieldSetDefDb->info_DictionaryId = 0;
	
	pFieldSetDefDb->maxSetId = 0;
	pFieldSetDefDb->isInitialized = RSSL_FALSE;
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDeleteElementSetDefDb(RsslElementSetDefDb* pElementSetDefDb)
{
	int setIter;
	int entryIter;
	
	if (!pElementSetDefDb)
		return RSSL_RET_SUCCESS;

	for(setIter = 0; setIter <= pElementSetDefDb->maxSetId; setIter++)
	{
		if(pElementSetDefDb->definitions[setIter] != NULL)
		{
			for(entryIter = 0; entryIter < pElementSetDefDb->definitions[setIter]->count; entryIter++)
			{
				free(pElementSetDefDb->definitions[setIter]->pEntries[entryIter].name.data);
			}
			free(pElementSetDefDb->definitions[setIter]->pEntries);
			free(pElementSetDefDb->definitions[setIter]);
			
			pElementSetDefDb->definitions[setIter] = 0;
		}
	}

	if(pElementSetDefDb->info_version.data != NULL)
	{
		free(pElementSetDefDb->info_version.data);
	}
	pElementSetDefDb->info_version.data = 0;
	pElementSetDefDb->info_version.length = 0;
	pElementSetDefDb->info_DictionaryId = 0;
		
	free(pElementSetDefDb->definitions);
	
	pElementSetDefDb->maxSetId = 0;
	pElementSetDefDb->isInitialized = RSSL_FALSE;
	
	return RSSL_RET_SUCCESS;
}

RsslRet _rsslDecodeFidArray(RsslDecodeIterator *dIter, RsslFieldSetDef *setDef, RsslBuffer *errorText)
{
	RsslArray 	array;
	RsslBuffer 	arrEntry;
	RsslInt		fid;
	
	RsslRet 	ret;
	int rowIter;
	
	if((ret = rsslDecodeArray(dIter, &array)) < 0)
	{
		_setDbError(errorText, "rsslDecodeArray failed %d", ret);
		return RSSL_RET_FAILURE;
	}
	
	if(array.primitiveType != RSSL_DT_INT)
	{
		_setDbError(errorText, "incorrect array primitive type %d", array.primitiveType);
		return RSSL_RET_FAILURE;
	}
	
	rowIter = 0;
	
	while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0 || (ret = rsslDecodeInt(dIter, &fid)))
		{
			_setDbError(errorText, "Error while decoding array entry for setDef number %d - %d", setDef->setId, ret);
			return RSSL_RET_FAILURE;
		}
		
		if(rowIter >=  setDef->count)
		{
			_setDbError(errorText, "Too many FIDs in setDef number %d.  Expected %d", setDef->setId, setDef->count);
			return RSSL_RET_FAILURE;
		}
		
		setDef->pEntries[rowIter].fieldId = (RsslFieldId)fid;
		
		rowIter++;
	}
	
	return RSSL_RET_SUCCESS;
}

RsslRet _rsslDecodeNamesArray(RsslDecodeIterator *dIter, RsslElementSetDef *setDef, RsslBuffer *errorText)
{
	RsslArray 	array;
	RsslBuffer 	arrEntry;
	RsslBuffer	name;
	
	RsslRet 	ret;
	int rowIter;
	
	if((ret = rsslDecodeArray(dIter, &array)) < 0)
	{
		_setDbError(errorText, "rsslDecodeArray failed %d", ret);
		return RSSL_RET_FAILURE;
	}
	
	if(array.primitiveType != RSSL_DT_BUFFER)
	{
		_setDbError(errorText, "incorrect array primitive type %d", array.primitiveType);
		return RSSL_RET_FAILURE;
	}
	
	rowIter = 0;
	
	while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0 || (ret = rsslDecodeBuffer(dIter, &name)))
		{
			_setDbError(errorText, "Error while decoding array entry for setDef number %d - %d", setDef->setId, ret);
			return RSSL_RET_FAILURE;
		}
		
		if(rowIter >=  setDef->count)
		{
			_setDbError(errorText, "Too many FIDs in setDef number %d.  Expected %d", setDef->setId, setDef->count);
			return RSSL_RET_FAILURE;
		}
		
		setDef->pEntries[rowIter].name.data = malloc(name.length*sizeof(char));
		if(setDef->pEntries[rowIter].name.data == 0)
		{
			_setDbError(errorText, "malloc failed for name");
			return RSSL_RET_FAILURE;
		}

		memcpy(setDef->pEntries[rowIter].name.data, name.data, name.length);

		setDef->pEntries[rowIter].name.length = name.length;
		
		rowIter++;
	}
	
	return RSSL_RET_SUCCESS;
}

RsslRet _rsslDecodeFieldTypesArray(RsslDecodeIterator* dIter, RsslFieldSetDef *setDef, RsslBuffer *errorText)
{
	RsslArray 	array;
	RsslBuffer 	arrEntry;
	RsslInt		dataType;
	
	RsslRet 	ret;
	int rowIter;
	
	if((ret = rsslDecodeArray(dIter, &array)) < 0)
	{
		_setDbError(errorText, "rsslDecodeArray failed %d", ret);
		return RSSL_RET_FAILURE;
	}
	
	if(array.primitiveType != RSSL_DT_INT)
	{
		_setDbError(errorText, "incorrect array primitive type %d", array.primitiveType);
		return RSSL_RET_FAILURE;
	}
	
	rowIter = 0;
	
	while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0 || (ret = rsslDecodeInt(dIter, &dataType)))
		{
			_setDbError(errorText, "Error while decoding array entry for setDef number %d - %d", setDef->setId, ret);
			return RSSL_RET_FAILURE;
		}
		
		if(rowIter >= setDef->count)
		{
			_setDbError(errorText, "Too many FIDs in setDef number %d.  Expected %d", setDef->setId, setDef->count);
			return RSSL_RET_FAILURE;
		}
		
		setDef->pEntries[rowIter].dataType = (RsslInt8)dataType;
		
		rowIter++;
	}
	
	return RSSL_RET_SUCCESS;
}

RsslRet _rsslDecodeElementTypesArray(RsslDecodeIterator* dIter, RsslElementSetDef *setDef, RsslBuffer *errorText)
{
	RsslArray 	array;
	RsslBuffer 	arrEntry;
	RsslInt		dataType;
	
	RsslRet 	ret;
	int rowIter;
	
	if((ret = rsslDecodeArray(dIter, &array)) < 0)
	{
		_setDbError(errorText, "rsslDecodeArray failed %d", ret);
		return RSSL_RET_FAILURE;
	}
	
	if(array.primitiveType != RSSL_DT_INT)
	{
		_setDbError(errorText, "incorrect array primitive type %d", array.primitiveType);
		return RSSL_RET_FAILURE;
	}
	
	rowIter = 0;
	
	while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0 || (ret = rsslDecodeInt(dIter, &dataType)))
		{
			_setDbError(errorText, "Error while decoding array entry for setDef number %d - %d", setDef->setId, ret);
			return RSSL_RET_FAILURE;
		}
		
		if(rowIter >= setDef->count)
		{
			_setDbError(errorText, "Too many FIDs in setDef number %d.  Expected %d", setDef->setId, setDef->count);
			return RSSL_RET_FAILURE;
		}
		
		setDef->pEntries[rowIter].dataType = (RsslInt8)dataType;
		
		rowIter++;
	}
	
	return RSSL_RET_SUCCESS;
}
		
typedef enum
{
	ENC_NONE			= 0,
	ENC_VECTOR			= 1,
	ENC_VECTOR_ENTRY	= 2,
	ENC_ELEM_LIST		= 3,
	ENC_ELEM_ENTRY		= 4,
	ENC_ARRAY			= 5
} setDefEncodingState;


RSSL_API RsslRet rsslEncodeFieldSetDefDictionary(
	RsslEncodeIterator 				*eIter,
	RsslFieldSetDefDb				*dictionary,
	int								*currentSetDef,
	RDMDictionaryVerbosityValues 	verbosity,
	RsslBuffer						*errorText)
{
	RsslRet ret;
	RsslVector 			vector = RSSL_INIT_VECTOR;
	RsslVectorEntry		vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslElementList 	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry 	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb	setDb;
	RsslInt				dictType;
	RsslArray			array;
	RsslInt				tmpInt;
	
	RsslFieldSetDef		*setDef;

	RsslBool finishedSet = RSSL_FALSE;
	
	setDefEncodingState encState = ENC_NONE;

	int setIter;
	
	int curSet = *currentSetDef;
	
	if(!dictionary->isInitialized)
		return (_setDbError(errorText, "Dictionary not initialized"), RSSL_RET_FAILURE);
	
	vector.containerType = RSSL_DT_ELEMENT_LIST;
	
	if (verbosity > RDM_DICTIONARY_INFO)
	{
		rsslVectorApplyHasSetDefs(&vector);
		rsslClearLocalElementSetDefDb(&setDb);
		setDb.definitions[0] = fieldSetDef0_Verbose;
	}
	
	if(curSet == 0)
	{
		rsslVectorApplyHasSummaryData(&vector);
	}
	
	if((ret = rsslEncodeVectorInit(eIter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
	{
		_setDbError(errorText, "rsslEncodeVectorInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	encState = ENC_VECTOR;
	
	/* Do not put in set defs if info only */
	if(verbosity > RDM_DICTIONARY_INFO)
	{
		if ((ret = rsslEncodeLocalElementSetDefDb(eIter, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslEncodeLocalElementSetDefDb failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		if ((ret = rsslEncodeVectorSetDefsComplete( eIter, 1 )) < 0)
		{
			_setDbError(errorText, "rsslEncodeVectorSetDefsComplete failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}
	
	/* if first message, encode summary data */
	if(curSet == 0)
	{
		/* Maybe split this out to another function */
		rsslClearElementList(&elementList);
		rsslElementListApplyHasStandardData(&elementList);

		if ((ret = rsslEncodeElementListInit(eIter, &elementList,0, 0)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementListInit failed %d", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_DICT_TYPE;
		dictType = RDM_DICTIONARY_FIELD_SET_DEFINITION;
		if (rsslEncodeElementEntry( eIter, &element, &dictType) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
		if(dictionary->info_version.length != 0)
		{
			rsslClearElementEntry(&element);
			element.dataType = RSSL_DT_ASCII_STRING;
			element.name = RSSL_ENAME_DICT_VERSION;
			if (rsslEncodeElementEntry( eIter, &element, &dictionary->info_version) < 0)
			{
				_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type",ret);
				return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			}
		}

		rsslClearElementEntry(&element);
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_DICTIONARY_ID;
		if (rsslEncodeElementEntry( eIter, &element, &(dictionary->info_DictionaryId)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		if ((ret = rsslEncodeElementListComplete(eIter,RSSL_TRUE)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementListComplete failed %d",ret);
			return RSSL_RET_FAILURE;
		}
		
		if ((ret = rsslEncodeVectorSummaryDataComplete(eIter, RSSL_TRUE)) < 0)
		{
			_setDbError(errorText, "rsslEncodeVectorSummaryDataComplete failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}
	
	if(verbosity > RDM_DICTIONARY_INFO)
	{
		while(curSet <= dictionary->maxSetId)
		{
			if(dictionary->definitions[curSet] != 0)
			{
				setDef = dictionary->definitions[curSet];
				rsslClearVectorEntry(&vectorEntry);
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorEntry.index = curSet;
				
				if ((ret = rsslEncodeVectorEntryInit(eIter, &vectorEntry, 0)) < 0)
				{
					_setDbError(errorText, "RsslEncodeVectorEntryInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_VECTOR_ENTRY;
				
				rsslClearElementList(&elementList);
				rsslElementListApplyHasSetData(&elementList);
				/* Clearing the list sets the setId to 0, which is what we want */
				
				if ((ret = rsslEncodeElementListInit(eIter, &elementList, &setDb, 0)) < 0)
				{
					_setDbError(errorText, "rsslEncodeElementListInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_LIST;
				
				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_INT;
				element.name = RSSL_ENAME_SETDEF_NUMENTRIES;
				
				if ((ret = rsslEncodeElementEntry(eIter, &element, &(setDef->count))) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntry NumEntries %d failed %d",setDef->count,ret);
					goto FieldEncRollback;
				}

				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_ARRAY;
				element.name = RSSL_ENAME_SETDEF_FIDS;
				
				if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_ENTRY;

				rsslClearArray(&array);
				array.primitiveType = RSSL_DT_INT;
				array.itemLength = 0;
				
				if ((ret = rsslEncodeArrayInit(eIter, &array)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ARRAY;
				
				for(setIter = 0; setIter < setDef->count; setIter++)
				{
					tmpInt = (RsslInt)setDef->pEntries[setIter].fieldId;
					if ((ret = rsslEncodeArrayEntry(eIter, 0, &tmpInt)) != RSSL_RET_SUCCESS)
					{
						_setDbError(errorText, "rsslEncodeArrayEntry failed %d",ret);
						goto FieldEncRollback;
					}
				}
				
				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayComplete failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_LIST;
				
				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_ARRAY;
				element.name = RSSL_ENAME_SETDEF_TYPES;
				
				if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				rsslClearArray(&array);
				array.primitiveType = RSSL_DT_INT;
				array.itemLength = 0;
				
				if ((ret = rsslEncodeArrayInit(eIter, &array)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ARRAY;
				
				for(setIter = 0; setIter < setDef->count; setIter++)
				{
					tmpInt = (RsslUInt)setDef->pEntries[setIter].dataType;
					if ((ret = rsslEncodeArrayEntry(eIter, 0, &tmpInt)) != RSSL_RET_SUCCESS)
					{
						_setDbError(errorText, "rsslEncodeArrayEntry failed %d",ret);
						goto FieldEncRollback;
					}
				}
				
				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayComplete failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) != RSSL_RET_SET_COMPLETE)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_ELEM_LIST;

				if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_VECTOR_ENTRY;
				
				if ((ret = rsslEncodeVectorEntryComplete(eIter, RSSL_TRUE)) < 0)
				{
					_setDbError(errorText, "RsslEncodeVectorEntryInit failed %d",ret);
					goto FieldEncRollback;
				}

				encState = ENC_VECTOR;

				finishedSet = RSSL_TRUE;
			}
			
			curSet++;
		}
	}
	else
	{
		curSet = dictionary->maxSetId;
	}
	
	if ((ret = rsslEncodeVectorComplete(eIter, 1)) < 0)
	{
		_setDbError(errorText, "rsslEncodeSeriesComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}

	*currentSetDef = curSet;
	return RSSL_RET_SUCCESS;
FieldEncRollback:
	switch(encState)
	{
		case ENC_ARRAY:
			if((ret = rsslEncodeArrayComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "array rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_ELEM_ENTRY:
			if((ret = rsslEncodeElementEntryComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "element entry rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_ELEM_LIST:
			if((ret = rsslEncodeElementListComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "element list rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_VECTOR_ENTRY:
			if((ret = rsslEncodeVectorEntryComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "vector entry rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_VECTOR:
			if(finishedSet == RSSL_TRUE)
			{
				if((ret = rsslEncodeVectorComplete(eIter, RSSL_TRUE)) < 0)
				{
					_setDbError(errorText, "vector complete failed %d",ret);
					return RSSL_RET_FAILURE;
				}
				*currentSetDef = curSet;
				return RSSL_RET_DICT_PART_ENCODED;
			}
			else
				return RSSL_RET_FAILURE;
        default:
            return RSSL_RET_FAILURE;
	}
}

RSSL_API RsslRet rsslEncodeElementSetDefDictionary(
	RsslEncodeIterator 				*eIter,
	RsslElementSetDefDb				*dictionary,
	int								*currentSetDef,
	RDMDictionaryVerbosityValues 	verbosity,
	RsslBuffer						*errorText)
{
	RsslRet ret;
	RsslVector 			vector = RSSL_INIT_VECTOR;
	RsslVectorEntry		vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslElementList 	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry 	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb	setDb;
	RsslInt				dictType;
	RsslArray			array;
	RsslInt tmpInt;
	
	RsslElementSetDef		*setDef;

	RsslBool finishedSet = RSSL_FALSE;
	
	setDefEncodingState encState = ENC_NONE;

	int setIter;
	
	int curSet = *currentSetDef;
	
	if(!dictionary->isInitialized)
		return (_setDbError(errorText, "Dictionary not initialized"), RSSL_RET_FAILURE);
	
	vector.containerType = RSSL_DT_ELEMENT_LIST;
	
	if (verbosity > RDM_DICTIONARY_INFO)
	{
		rsslVectorApplyHasSetDefs(&vector);
		rsslClearLocalElementSetDefDb(&setDb);
		setDb.definitions[0] = elementSetDef0_Verbose;
	}
	
	if(curSet == 0)
	{
		rsslVectorApplyHasSummaryData(&vector);
	}
	
	if((ret = rsslEncodeVectorInit(eIter, &vector, 0, 0)) < RSSL_RET_SUCCESS)
	{
		_setDbError(errorText, "rsslEncodeVectorInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	encState = ENC_VECTOR;
	
	/* Do not put in set defs if info only */
	if(verbosity > RDM_DICTIONARY_INFO)
	{
		if ((ret = rsslEncodeLocalElementSetDefDb(eIter, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslEncodeLocalElementSetDefDb failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		if ((ret = rsslEncodeVectorSetDefsComplete( eIter, 1 )) < 0)
		{
			_setDbError(errorText, "rsslEncodeVectorSetDefsComplete failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}
	
	/* if first message, encode summary data */
	if(curSet == 0)
	{
		/* Maybe split this out to another function */
		rsslClearElementList(&elementList);
		rsslElementListApplyHasStandardData(&elementList);

		if ((ret = rsslEncodeElementListInit(eIter, &elementList,0, 0)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementListInit failed %d", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_DICT_TYPE;
		dictType = RDM_DICTIONARY_ELEMENT_SET_DEFINITION;
		if (rsslEncodeElementEntry( eIter, &element, &dictType) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
		if(dictionary->info_version.length != 0)
		{
			rsslClearElementEntry(&element);
			element.dataType = RSSL_DT_ASCII_STRING;
			element.name = RSSL_ENAME_DICT_VERSION;
			if (rsslEncodeElementEntry( eIter, &element, &dictionary->info_version) < 0)
			{
				_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type",ret);
				return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			}
		}

		rsslClearElementEntry(&element);
		element.dataType = RSSL_DT_UINT;
		element.name = RSSL_ENAME_DICTIONARY_ID;
		if (rsslEncodeElementEntry( eIter, &element, &(dictionary->info_DictionaryId)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementEntry failed %d - Type", ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		if ((ret = rsslEncodeElementListComplete(eIter,RSSL_TRUE)) < 0)
		{
			_setDbError(errorText, "rsslEncodeElementListComplete failed %d",ret);
			return RSSL_RET_FAILURE;
		}
		
		if ((ret = rsslEncodeVectorSummaryDataComplete(eIter, RSSL_TRUE)) < 0)
		{
			_setDbError(errorText, "rsslEncodeVectorSummaryDataComplete failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}
	
	if(verbosity > RDM_DICTIONARY_INFO)
	{
		while(curSet <= dictionary->maxSetId)
		{
			if(dictionary->definitions[curSet] != 0)
			{
				setDef = dictionary->definitions[curSet];
				rsslClearVectorEntry(&vectorEntry);
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorEntry.index = curSet;
				
				if ((ret = rsslEncodeVectorEntryInit(eIter, &vectorEntry, 0)) < 0)
				{
					_setDbError(errorText, "RsslEncodeVectorEntryInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_VECTOR_ENTRY;
				
				rsslClearElementList(&elementList);
				rsslElementListApplyHasSetData(&elementList);
				/* Clearing the list sets the setId to 0, which is what we want */
				
				if ((ret = rsslEncodeElementListInit(eIter, &elementList, &setDb, 0)) < 0)
				{
					_setDbError(errorText, "rsslEncodeElementListInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_LIST;
				
				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_INT;
				element.name = RSSL_ENAME_SETDEF_NUMENTRIES;
				
				tmpInt = (RsslUInt)setDef->count;
				if ((ret = rsslEncodeElementEntry(eIter, &element, &tmpInt)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntry NumEntries %d failed %d",setDef->count,ret);
					goto ElementEncRollback;
				}

				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_ARRAY;
				element.name = RSSL_ENAME_SETDEF_NAMES;
				
				if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_ENTRY;

				rsslClearArray(&array);
				array.primitiveType = RSSL_DT_BUFFER;
				array.itemLength = 0;
				
				if ((ret = rsslEncodeArrayInit(eIter, &array)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ARRAY;
				
				for(setIter = 0; setIter < setDef->count; setIter++)
				{
					if ((ret = rsslEncodeArrayEntry(eIter, 0, &setDef->pEntries[setIter].name)) != RSSL_RET_SUCCESS)
					{
						_setDbError(errorText, "rsslEncodeArrayEntry failed %d",ret);
						goto ElementEncRollback;
					}
				}
				
				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayComplete failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_LIST;
				
				rsslClearElementEntry(&element);
				element.dataType = RSSL_DT_ARRAY;
				element.name = RSSL_ENAME_SETDEF_TYPES;
				
				if ((ret = rsslEncodeElementEntryInit(eIter, &element, 0)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				rsslClearArray(&array);
				array.primitiveType = RSSL_DT_INT;
				array.itemLength = 0;
				
				if ((ret = rsslEncodeArrayInit(eIter, &array)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ARRAY;
				
				for(setIter = 0; setIter < setDef->count; setIter++)
				{
					tmpInt = (RsslUInt)setDef->pEntries[setIter].dataType;
					if ((ret = rsslEncodeArrayEntry(eIter, 0, &tmpInt)) != RSSL_RET_SUCCESS)
					{
						_setDbError(errorText, "rsslEncodeArrayEntry failed %d",ret);
						goto ElementEncRollback;
					}
				}
				
				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeArrayComplete failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_ENTRY;
				
				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) != RSSL_RET_SET_COMPLETE)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_ELEM_LIST;

				if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) != RSSL_RET_SUCCESS)
				{
					_setDbError(errorText, "rsslEncodeElementEntryComplete failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_VECTOR_ENTRY;
				
				if ((ret = rsslEncodeVectorEntryComplete(eIter, RSSL_TRUE)) < 0)
				{
					_setDbError(errorText, "RsslEncodeVectorEntryInit failed %d",ret);
					goto ElementEncRollback;
				}

				encState = ENC_VECTOR;

				finishedSet = RSSL_TRUE;
			}
			
			curSet++;
		}
	}
	else
	{
		curSet = dictionary->maxSetId;
	}
	
	if ((ret = rsslEncodeVectorComplete(eIter, 1)) < 0)
	{
		_setDbError(errorText, "rsslEncodeSeriesComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}

	*currentSetDef = curSet;
	return RSSL_RET_SUCCESS;
ElementEncRollback:
	switch(encState)
	{
		case ENC_ARRAY:
			if((ret = rsslEncodeArrayComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "array rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_ELEM_ENTRY:
			if((ret = rsslEncodeElementEntryComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "element entry rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_ELEM_LIST:
			if((ret = rsslEncodeElementListComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "element list rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_VECTOR_ENTRY:
			if((ret = rsslEncodeVectorEntryComplete(eIter, RSSL_FALSE)) < 0)
			{
				_setDbError(errorText, "vector entry rollback failed %d",ret);
				return RSSL_RET_FAILURE;
			}
		case ENC_VECTOR:
			if(finishedSet == RSSL_TRUE)
			{
				if((ret = rsslEncodeVectorComplete(eIter, RSSL_TRUE)) < 0)
				{
					_setDbError(errorText, "vector complete failed %d",ret);
					return RSSL_RET_FAILURE;
				}
				*currentSetDef = curSet;
				return RSSL_RET_DICT_PART_ENCODED;
			}
			else
				return RSSL_RET_FAILURE;
	     case ENC_NONE:
		   break;
	}
	return RSSL_RET_FAILURE;
}

RSSL_API RsslRet rsslDecodeFieldSetDefDictionary(
	RsslDecodeIterator 				*dIter,
	RsslFieldSetDefDb				*dictionary,
	RDMDictionaryVerbosityValues 	verbosity,
	RsslBuffer						*errorText)
{
	RsslRet ret;
	RsslVector 			vector = RSSL_INIT_VECTOR;
	RsslVectorEntry		vEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslElementList 	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry 	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb	setDb;
	char setDefMemory[LOCAL_EL_DEFS_TLS_SIZE];
	RsslDecodeIterator tempIter;
	RsslBuffer 			tempFids;
	RsslBuffer			tempTypes;
	
	RsslBool			decodeCount;
	RsslBool			decodeFids;
	RsslBool			decodeTypes;

	RsslBuffer version = RSSL_INIT_BUFFER;

	RsslInt				tempInt;
	
	RsslFieldSetDef		*newSetDef = 0;
	
	if(dictionary == 0)
	{
		_setDbError(errorText, "NULL dictionary pointer.");
		goto FieldClearAndExit;
	}
	
	if (!dictionary->isInitialized && rsslAllocateFieldSetDefDb(dictionary, &version) != RSSL_RET_SUCCESS)
		goto FieldClearAndExit;
		
	if ((ret = rsslDecodeVector(dIter, &vector)) < 0)
	{
		_setDbError(errorText, "Invalid container type of %d; expecting %d (RSSL_DT_ELEMENT_LIST)", vector.containerType, RSSL_DT_ELEMENT_LIST);
		goto FieldClearAndExit;
	}
	if(vector.containerType != RSSL_DT_ELEMENT_LIST)
	{
		_setDbError(errorText, "wrong container type - %d",vector.containerType);
		goto FieldClearAndExit;
	}
	
	if(rsslVectorCheckHasSummaryData(&vector))
	{
		if(rsslDecodeElementList(dIter, &elementList, NULL) < 0)
		{
			_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
			goto FieldClearAndExit;
		}
		
		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if ( ret < 0)
			{
				_setDbError(errorText, "rsslDecodeElementEntry failed - %d", ret);
				goto FieldClearAndExit;
			}

			if (_decodeFieldSetDefTag(dIter, &element, dictionary, errorText) != RSSL_RET_SUCCESS)
			{
				goto FieldClearAndExit;
			}
		}
	}
		
	if(rsslVectorCheckHasSetDefs(&vector))
	{
		rsslClearLocalElementSetDefDb(&setDb);
		setDb.entries.data = setDefMemory;
		setDb.entries.length = sizeof(setDefMemory);
		
		if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslDecodeLocalElementSetDefDb failed - %d", ret);
			goto FieldClearAndExit;
		}
	}
	
	while((ret = rsslDecodeVectorEntry(dIter, &vEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0)
		{
			_setDbError(errorText, "rsslDecodeVectorEntry failed - %d", ret);
			goto FieldClearAndExit;
		}
		
		if(dictionary->definitions[vEntry.index] != NULL)
		{
			_setDbError(errorText, "duplicate entry received - %d", vEntry.index);
			goto FieldClearAndExit;
		}

		if((ret = rsslDecodeElementList(dIter, &elementList, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
			goto FieldClearAndExit;
		}

		if((newSetDef = (RsslFieldSetDef*)malloc(sizeof(RsslFieldSetDef))) == NULL)
		{
			_setDbError(errorText, "malloc failed");
			goto FieldClearAndExit;
		}
		
		rsslClearFieldSetDef(newSetDef);
		
		decodeCount = RSSL_FALSE;
		decodeFids = RSSL_FALSE;
		decodeTypes = RSSL_FALSE;
		rsslClearBuffer(&tempFids);
		rsslClearBuffer(&tempTypes);

		newSetDef->setId = vEntry.index;

		while((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if(ret < 0)
			{
				_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
				goto FieldClearAndExit;
			}
		
			if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_NUMENTRIES))
			{
				if(element.dataType != RSSL_DT_INT || rsslDecodeInt(dIter, &tempInt))
				{
					_setDbError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_SETDEF_NUMENTRIES.data);
					goto FieldClearAndExit;
				}
				
				newSetDef->count = (RsslInt8)tempInt;
				
				if((newSetDef->pEntries = (RsslFieldSetDefEntry*)malloc(sizeof(RsslFieldSetDefEntry)*newSetDef->count)) == NULL)
				{
					_setDbError(errorText, "malloc failed");
					goto FieldClearAndExit;
				}
				
				decodeCount = RSSL_TRUE;
			}
			else if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_FIDS))
			{
				if(element.dataType != RSSL_DT_ARRAY)
				{
					_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
					goto FieldClearAndExit;
				}
				
				if(decodeCount == RSSL_TRUE)
				{
					if(_rsslDecodeFidArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
						goto FieldClearAndExit;
						
					decodeFids = RSSL_TRUE;
				}
				else
				{	
					/* List has not been allocated, so this will need to be decoded later */
					tempFids = element.encData;
				}
			}
			else
			if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_TYPES))
			{
				if(element.dataType != RSSL_DT_ARRAY)
				{
					_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
					goto FieldClearAndExit;
				}
				
				if(decodeCount == RSSL_TRUE)
				{
					if((ret = _rsslDecodeFieldTypesArray(dIter, newSetDef, errorText)) != RSSL_RET_SUCCESS)
						goto FieldClearAndExit;
					
					decodeTypes = RSSL_TRUE;
				}
				else
				{	
					/* List has not been allocated, so this will need to be decoded later */
					tempTypes = element.encData;
				}
			}
		}
		
		if(decodeFids != RSSL_TRUE)
		{
			rsslClearDecodeIterator(&tempIter);
			rsslSetDecodeIteratorBuffer(&tempIter, &tempFids);
			rsslSetDecodeIteratorRWFVersion(&tempIter, dIter->_majorVersion, dIter->_minorVersion);
			
			if(_rsslDecodeFidArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
				goto FieldClearAndExit;
		}
		
		if(decodeTypes != RSSL_TRUE)
		{
			rsslClearDecodeIterator(&tempIter);
			rsslSetDecodeIteratorBuffer(&tempIter, &tempTypes);
			rsslSetDecodeIteratorRWFVersion(&tempIter, dIter->_majorVersion, dIter->_minorVersion);
			
			if(_rsslDecodeFieldTypesArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
				goto FieldClearAndExit;
		}
		
		dictionary->definitions[newSetDef->setId] = newSetDef;

		if(dictionary->maxSetId < newSetDef->setId)
			dictionary->maxSetId = newSetDef->setId;
	}

	return RSSL_RET_SUCCESS;

FieldClearAndExit:
	if(newSetDef != 0)
	{
		if(newSetDef->pEntries != 0)
			free(newSetDef->pEntries);
	
		free(newSetDef);
	}

	rsslDeleteFieldSetDefDb(dictionary);

	return RSSL_RET_FAILURE;
}

RSSL_API RsslRet rsslDecodeElementSetDefDictionary(
	RsslDecodeIterator 				*dIter,
	RsslElementSetDefDb				*dictionary,
	RDMDictionaryVerbosityValues 	verbosity,
	RsslBuffer						*errorText)
{
	RsslRet ret;
	RsslVector 			vector = RSSL_INIT_VECTOR;
	RsslVectorEntry		vEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslElementList 	elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry 	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb	setDb;
	char setDefMemory[LOCAL_EL_DEFS_TLS_SIZE];
	RsslDecodeIterator tempIter;
	RsslBuffer 			tempFids;
	RsslBuffer			tempTypes;
	
	RsslBool			decodeCount;
	RsslBool			decodeFids;
	RsslBool			decodeTypes;

	RsslInt				tempInt;
	RsslBuffer version = RSSL_INIT_BUFFER;
	
	RsslElementSetDef		*newSetDef = 0;
	
	if(dictionary == 0)
	{
		_setDbError(errorText, "NULL dictionary pointer.");
		goto ElementClearAndExit;
	}
	
	if (!dictionary->isInitialized && rsslAllocateElementSetDefDb(dictionary, &version) != RSSL_RET_SUCCESS)
		goto ElementClearAndExit;
		
	if ((ret = rsslDecodeVector(dIter, &vector)) < 0)
	{
		_setDbError(errorText, "Invalid container type of %d; expecting %d (RSSL_DT_ELEMENT_LIST)", vector.containerType, RSSL_DT_ELEMENT_LIST);
		goto ElementClearAndExit;
	}
	
	if(vector.containerType != RSSL_DT_ELEMENT_LIST)
	{
		_setDbError(errorText, "wrong container type - %d",vector.containerType);
		goto ElementClearAndExit;
	}
	
	if(rsslVectorCheckHasSummaryData(&vector))
	{
		if(rsslDecodeElementList(dIter, &elementList, NULL) < 0)
		{
			_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
			goto ElementClearAndExit;
		}
		
		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if ( ret < 0)
			{
				_setDbError(errorText, "rsslDecodeElementEntry failed - %d", ret);
				goto ElementClearAndExit;
			}

			if (_decodeElementSetDefTag(dIter, &element, dictionary, errorText) != RSSL_RET_SUCCESS)
			{
				goto ElementClearAndExit;
			}
		}
	}
		
	if(rsslVectorCheckHasSetDefs(&vector))
	{
		rsslClearLocalElementSetDefDb(&setDb);
		setDb.entries.data = setDefMemory;
		setDb.entries.length = sizeof(setDefMemory);
		
		if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslDecodeLocalElementSetDefDb failed - %d", ret);
			goto ElementClearAndExit;
		}
	}
	
	while((ret = rsslDecodeVectorEntry(dIter, &vEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if(ret < 0)
		{
			_setDbError(errorText, "rsslDecodeVectorEntry failed - %d", ret);
			goto ElementClearAndExit;
		}
		
		if(dictionary->definitions[vEntry.index] != NULL)
		{
			_setDbError(errorText, "duplicate entry received - %d", vEntry.index);
			goto ElementClearAndExit;
		}

		if((ret = rsslDecodeElementList(dIter, &elementList, &setDb)) < 0)
		{
			_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
			goto ElementClearAndExit;
		}
		
		if((newSetDef = (RsslElementSetDef*)malloc(sizeof(RsslElementSetDef))) == NULL)
		{
			_setDbError(errorText, "malloc failed");
			goto ElementClearAndExit;
		}
		
		rsslClearElementSetDef(newSetDef);
		
		decodeCount = RSSL_FALSE;
		decodeFids = RSSL_FALSE;
		decodeTypes = RSSL_FALSE;
		rsslClearBuffer(&tempFids);
		rsslClearBuffer(&tempTypes);

		newSetDef->setId = vEntry.index;

		while((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if(ret < 0)
			{
				_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
				goto ElementClearAndExit;
			}
		
			if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_NUMENTRIES))
			{
				if(element.dataType != RSSL_DT_INT || rsslDecodeInt(dIter, &tempInt))
				{
					_setDbError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_SETDEF_NUMENTRIES.data);
					goto ElementClearAndExit;
				}
				
				newSetDef->count = (RsslInt8)tempInt;
				
				if((newSetDef->pEntries = (RsslElementSetDefEntry*)malloc(sizeof(RsslElementSetDefEntry)*newSetDef->count)) == NULL)
				{
					_setDbError(errorText, "malloc failed");
					goto ElementClearAndExit;
				}
				
				decodeCount = RSSL_TRUE;
			}
			else if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_NAMES))
			{
				if(element.dataType != RSSL_DT_ARRAY)
				{
					_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
					goto ElementClearAndExit;
				}
				
				if(decodeCount == RSSL_TRUE)
				{
					if(_rsslDecodeNamesArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
						goto ElementClearAndExit;
						
					decodeFids = RSSL_TRUE;
				}
				else
				{	
					/* List has not been allocated, so this will need to be decoded later */
					tempFids = element.encData;
				}
			}
			else if(rsslBufferIsEqual(&element.name, &RSSL_ENAME_SETDEF_TYPES))
			{
				if(element.dataType != RSSL_DT_ARRAY)
				{
					_setDbError(errorText, "rsslDecodeElementList failed - %d", ret);
					goto ElementClearAndExit;
				}
				
				if(decodeCount == RSSL_TRUE)
				{
					if((ret = _rsslDecodeElementTypesArray(dIter, newSetDef, errorText)) != RSSL_RET_SUCCESS)
						goto ElementClearAndExit;
					
					decodeTypes = RSSL_TRUE;
				}
				else
				{	
					/* List has not been allocated, so this will need to be decoded later */
					tempTypes = element.encData;
				}
			}
		}
		
		if(decodeFids != RSSL_TRUE)
		{
			rsslClearDecodeIterator(&tempIter);
			rsslSetDecodeIteratorBuffer(&tempIter, &tempFids);
			rsslSetDecodeIteratorRWFVersion(&tempIter, dIter->_majorVersion, dIter->_minorVersion);
			
			if(_rsslDecodeNamesArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
				goto ElementClearAndExit;
		}
		
		if(decodeTypes != RSSL_TRUE)
		{
			rsslClearDecodeIterator(&tempIter);
			rsslSetDecodeIteratorBuffer(&tempIter, &tempTypes);
			rsslSetDecodeIteratorRWFVersion(&tempIter, dIter->_majorVersion, dIter->_minorVersion);
			
			if(_rsslDecodeElementTypesArray(dIter, newSetDef, errorText) != RSSL_RET_SUCCESS)
				goto ElementClearAndExit;
		}
		
		dictionary->definitions[newSetDef->setId] = newSetDef;

		if(dictionary->maxSetId < newSetDef->setId)
			dictionary->maxSetId = newSetDef->setId;
	}

	return RSSL_RET_SUCCESS;

ElementClearAndExit:
	if(newSetDef != 0)
	{
		if(newSetDef->pEntries != 0)
			free(newSetDef->pEntries);
	
		free(newSetDef);
	}
	rsslDeleteElementSetDefDb(dictionary);
	return RSSL_RET_FAILURE;
}
	

#ifdef __cplusplus
}
#endif
