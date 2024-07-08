/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslFieldList.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rtr/rsslSetData.h"

/* rsslDecodeLocalFieldSetDefDb */
#include "rtr/rsslMap.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslSeries.h"

/* used for decoding local field list set definitions if user does not pass in memory */
/* 15 defs * 255 entries per def = 3825 -- * 3 bytes per field list def (2 byte fid, 1 byte type) = ~12000.   */
#define LOCAL_FL_DEFS_TLS_SIZE 3825
static RsslFieldSetDefEntry tempLocalFLSetDefStorage[RSSL_ITER_MAX_LEVELS][LOCAL_FL_DEFS_TLS_SIZE];

RSSL_API RsslRet rsslDecodeFieldList(
				RsslDecodeIterator		*oIter,
				RsslFieldList			*oFieldList,
				RsslLocalFieldSetDefDb	*iLocalSetDb )
{
	char 		*position;
	char		*_endBufPtr;
	RsslDecodingLevel *_levelInfo;
		
	RSSL_ASSERT(oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);	
	
	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_FIELD_LIST, oFieldList); 

	_endBufPtr = _levelInfo->_endBufPtr;
	position = oIter->_curBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}

	/* Get Flags */
	position += rwfGet8(oFieldList->flags, position);
	 

	/* Get the Field List Information */
	if (rsslFieldListCheckHasInfo(oFieldList))
	{
		RsslUInt8	infoLen;
		char*		startpos;

		/* Has 1 byte length */
		position += rwfGet8(infoLen, position);
		startpos = position;

		position += rwfGetResBitI15(&oFieldList->dictionaryId, position);
		position += rwfGet16(oFieldList->fieldListNum, position);

		if ((startpos + infoLen) > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		/* Used info Length to skip */
		position = startpos + infoLen;
	}

	/* Get the Field List Set Data */
	if (rsslFieldListCheckHasSetData(oFieldList))
	{
		/* Get the set identifier */
		if (rsslFieldListCheckHasSetId(oFieldList))
			position += rwfGetResBitU15(&oFieldList->setId, position);
		else
			oFieldList->setId = 0;

		/* Set the set definition from the local or global set list */
		if (oFieldList->setId <= RSSL_FIELD_SET_MAX_LOCAL_ID)
			_levelInfo->_fieldListSetDef = (iLocalSetDb && (iLocalSetDb->definitions[oFieldList->setId].setId != RSSL_FIELD_SET_BLANK_ID) ? 
				&iLocalSetDb->definitions[oFieldList->setId] : 0);
		else
		if(oFieldList->setId > RSSL_FIELD_SET_MAX_LOCAL_ID && oIter->_pGlobalFieldListSetDb != NULL && oIter->_pGlobalFieldListSetDb->definitions[oFieldList->setId] != NULL)
		{
			_levelInfo->_fieldListSetDef = oIter->_pGlobalFieldListSetDb->definitions[oFieldList->setId];
		}
		else
			_levelInfo->_fieldListSetDef = 0;

		/* Check for field list data */
		if (rsslFieldListCheckHasStandardData(oFieldList))
		{
			/* If HasSetData and HasFieldList, then set data is length specified. */
			position = _rsslDecodeBuffer15(&oFieldList->encSetData,position);

			/* Get the Field List Data */
			position += rwfGet16(_levelInfo->_itemCount, position);
			oFieldList->encEntries.data = position;
			oFieldList->encEntries.length = (rtrUInt32)(_endBufPtr - position);

			/* check for buffer overflow - post check ok since no copy */
			if (position > _endBufPtr)
				return RSSL_RET_INCOMPLETE_DATA;
		}
		else
		{
			/* Get the field list set data. Not length specified since
			 * no field list data exists.
			 */
			oFieldList->encEntries.data = 0;
			oFieldList->encEntries.length = 0;

			oFieldList->encSetData.data = position;
			oFieldList->encSetData.length = (rtrUInt32)(_endBufPtr - position);

			if (position > _endBufPtr)
				return RSSL_RET_INCOMPLETE_DATA;
		}

		/* Setup to decode the set if able. Otherwise skip to entries. */
		if (_levelInfo->_fieldListSetDef)
		{
			_levelInfo->_setCount = _levelInfo->_fieldListSetDef->count;
			_levelInfo->_itemCount += _levelInfo->_fieldListSetDef->count; 

			_levelInfo->_nextEntryPtr = /* oIter->_curBufPtr = */
				(_levelInfo->_setCount > 0) ? oFieldList->encSetData.data : position;

			return RSSL_RET_SUCCESS;
		}
		else
		{
			_levelInfo->_setCount = 0;
			_levelInfo->_nextEntryPtr = /* oIter->_curBufPtr = */ position + oFieldList->encSetData.length;
			return RSSL_RET_SET_SKIPPED;
		}
	}
	else if (rsslFieldListCheckHasStandardData(oFieldList))
	{
		/* Get the field list data only */
		oFieldList->encSetData.data = 0;
		oFieldList->encSetData.length = 0;

		position += rwfGet16(_levelInfo->_itemCount, position);
		oFieldList->encEntries.data = position;
		oFieldList->encEntries.length = (rtrUInt32)(_endBufPtr - position);

		oIter->_curBufPtr = _levelInfo->_nextEntryPtr = position;
		_levelInfo->_setCount = 0;
	}
	else
	{
		oFieldList->encSetData.data = 0;
		oFieldList->encSetData.length = 0;
		oFieldList->encEntries.data = 0;
		oFieldList->encEntries.length = 0;
		_levelInfo->_itemCount = 0;
		oIter->_curBufPtr = 0;
		_levelInfo->_setCount = 0;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeFieldEntry(
			    RsslDecodeIterator	*iIter,
				RsslFieldEntry		*oField )
{
	char 			*position;
	RsslFieldList	*fieldList;
	RsslDecodingLevel *_levelInfo;

	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);

	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];
	
	RSSL_ASSERT(oField && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(_levelInfo->_listType, Invalid decoding attempted);

	fieldList = (RsslFieldList*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	/* Make sure we skip to the next entry if we didn't decode the previous entry payload */
	position = iIter->_curBufPtr = _levelInfo->_nextEntryPtr;

	if (_levelInfo->_nextSetPosition < _levelInfo->_setCount)
	{
		RsslRet 			ret;
		RsslFieldSetDefEntry	*encoding=0;

		RSSL_ASSERT( _levelInfo->_fieldListSetDef, Invalid parameters or parameters passed in as NULL );
		RSSL_ASSERT( _levelInfo->_fieldListSetDef->count == _levelInfo->_setCount, Invalid data );

		encoding = & (_levelInfo->_fieldListSetDef->pEntries[_levelInfo->_nextSetPosition]);

		oField->fieldId = encoding->fieldId;
		oField->dataType = encoding->dataType;

		/* Get the set data and reset position */
		if ((ret = _rsslDecodeSet(iIter,encoding->dataType,&oField->encData)) != RSSL_RET_SUCCESS)
			return(ret);

		iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = _levelInfo->_nextEntryPtr;

    	/* handle legacy conversion */
    	oField->dataType = _rsslPrimitiveType(oField->dataType);

		_levelInfo->_nextItemPosition++;
		_levelInfo->_nextSetPosition++;

		if (_levelInfo->_nextSetPosition == _levelInfo->_setCount && fieldList->encEntries.data)
			_levelInfo->_nextEntryPtr = fieldList->encEntries.data;

		return RSSL_RET_SUCCESS;
	}

	/* Get normal field list data */
	if (fieldList->encEntries.data + fieldList->encEntries.length - position < 3)
		return RSSL_RET_INCOMPLETE_DATA;

	position += rwfGet16(oField->fieldId, position);
	oField->dataType = RSSL_DT_UNKNOWN;

	/* parse Field */
	position += rwfGetBuffer16(&oField->encData,position);
	if (position > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;

	/* shift iterator */
	iIter->_curBufPtr = oField->encData.data;
	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = position;
	_levelInfo->_nextItemPosition++;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeLocalFieldSetDefDb(
				RsslDecodeIterator				*pIter,
				RsslLocalFieldSetDefDb			*oLocalSetDb )
{
	char 		*position;
	char		*_endBufPtr;
	RsslUInt8	_setCount;
	int			i,j;
	
	RsslUInt32 curEntry = 0;

	RSSL_ASSERT(pIter && oLocalSetDb, Invalid parameters or parameters passed in as NULL);
	
	if (pIter->_decodingLevel >= 0)
	{
		/* Get the encSetDefs pointer out of the current container. */
		RsslBuffer encSetDefs;

		RSSL_ASSERT(pIter->_levelInfo[pIter->_decodingLevel]._listType, Invalid decoding attempted);

		switch(pIter->_levelInfo[pIter->_decodingLevel]._containerType)
		{
		case RSSL_DT_MAP: encSetDefs = ((RsslMap*)pIter->_levelInfo[pIter->_decodingLevel]._listType)->encSetDefs; break;
		case RSSL_DT_SERIES: encSetDefs = ((RsslSeries*)pIter->_levelInfo[pIter->_decodingLevel]._listType)->encSetDefs; break;
		case RSSL_DT_VECTOR: encSetDefs = ((RsslVector*)pIter->_levelInfo[pIter->_decodingLevel]._listType)->encSetDefs; break;
		default:
			return RSSL_RET_INVALID_ARGUMENT;
		}

		if (!encSetDefs.data || !encSetDefs.length)
			return RSSL_RET_INVALID_DATA;

		position = encSetDefs.data;
		_endBufPtr = encSetDefs.data + encSetDefs.length;

		if (!oLocalSetDb->entries.data)
		{
			/* set this to our thread local storage */
			oLocalSetDb->entries.data = (char*)tempLocalFLSetDefStorage[pIter->_decodingLevel];
			oLocalSetDb->entries.length = sizeof(tempLocalFLSetDefStorage[0]); /* size of one level */
		}
	}
	else
	{
		/* Separate iterator. */
		_endBufPtr = pIter->_levelInfo[0]._endBufPtr;
		position = pIter->_curBufPtr;

		if (!oLocalSetDb->entries.data)
		{
			/* set this to our thread local storage - take decode level 0 */
			oLocalSetDb->entries.data = (char*)tempLocalFLSetDefStorage[0];
			oLocalSetDb->entries.length = sizeof(tempLocalFLSetDefStorage[0]); /* size of one level */
		}
	}

	if ((_endBufPtr - position) < 2)
		return RSSL_RET_INCOMPLETE_DATA;

	position += 1;
	position += rwfGet8(_setCount, position);

	/*
	printf("FLSDB: flags %d count %d\n",flags,_setCount);
	*/

	if (_setCount == 0)
		return RSSL_RET_SET_DEF_DB_EMPTY;

	if (_setCount > RSSL_FIELD_SET_MAX_LOCAL_ID)
		return RSSL_RET_TOO_MANY_LOCAL_SET_DEFS;

	for (i=0;i<=RSSL_FIELD_SET_MAX_LOCAL_ID;i++)
	{
		oLocalSetDb->definitions[i].setId = RSSL_FIELD_SET_BLANK_ID	;
		oLocalSetDb->definitions[i].count = 0;
		oLocalSetDb->definitions[i].pEntries = NULL;
	}

	for (i = 0; i < _setCount; i++)
	{
		RsslFieldSetDef	*pCurSetDef;
		RsslFieldSetDefEntry *pCurEntry;
		RsslUInt16			setId;
		RsslUInt8			encCount;

		if ((position + 2) > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		/* Get the setId and the number of field encodings */
		position += rwfGetResBitU15(&setId, position);
		position += rwfGet8(encCount, position);

		/*
		printf("  FLSDEF: setId %d count %d\n",setId,encCount);
		*/

		/* Basic sanity checks */
		if (setId > RSSL_FIELD_SET_MAX_LOCAL_ID)
			return RSSL_RET_ILLEGAL_LOCAL_SET_DEF;
		if (oLocalSetDb->definitions[setId].setId != RSSL_FIELD_SET_BLANK_ID)
			return RSSL_RET_DUPLICATE_LOCAL_SET_DEFS;

		/* get memory for new set definition from working memory. */
		pCurSetDef = &oLocalSetDb->definitions[setId];
  
		/* make we have space in our entry pool */
		if (((curEntry + (RsslUInt32)encCount)*(sizeof(RsslFieldSetDefEntry))) > oLocalSetDb->entries.length)
			return RSSL_RET_BUFFER_TOO_SMALL;

		/* Setup set definition and put in database */
		pCurSetDef->setId = setId;
		pCurSetDef->count = encCount;
		if (encCount > 0)
		{
			pCurSetDef->pEntries = &((RsslFieldSetDefEntry*)(oLocalSetDb->entries.data))[curEntry];  /* Point to the entries from the pool that will be used for this def. */

			/* Fill in the field list encodings */
			for (j=0;j<encCount;j++)
			{
				pCurEntry = &((RsslFieldSetDefEntry*)(oLocalSetDb->entries.data))[curEntry];  /* Point to the entries from the pool that will be used for this def. */
				position += rwfGet16(pCurEntry->fieldId, position);
				position += rwfGet8(pCurEntry->dataType, position);
				++curEntry;
			}
		}
	}
	return (position <= _endBufPtr) ? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA;
}

RSSL_API RsslRet rsslFieldListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[4 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_FLF_HAS_FIELD_LIST_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_FLF_HAS_FIELD_LIST_INFO.data;
	if (flags & RSSL_FLF_HAS_SET_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_FLF_HAS_SET_DATA.data;
	if (flags & RSSL_FLF_HAS_SET_ID) flagStrings[flagCount++] = RSSL_OMMSTR_FLF_HAS_SET_ID.data;
	if (flags & RSSL_FLF_HAS_STANDARD_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_FLF_HAS_STANDARD_DATA.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}


