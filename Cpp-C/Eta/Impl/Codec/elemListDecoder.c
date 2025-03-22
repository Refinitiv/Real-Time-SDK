/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslElementList.h"
#include "rtr/decoderTools.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rtr/rsslSetData.h"

/* rsslDecodeLocalElementSetDefDb */
#include "rtr/rsslMap.h"
#include "rtr/rsslVector.h"
#include "rtr/rsslSeries.h"

/* used for decoding local element list set definitions if user does not pass in memory */
/* 15 defs * 255 entries per def = 3825 -- * 13 bytes per element list def (4 byte length, 8 byte pointer, 1 byte type) = ~50000 */
#define LOCAL_EL_DEFS_TLS_SIZE 3825
static RsslElementSetDefEntry tempLocalELSetDefStorage[RSSL_ITER_MAX_LEVELS][LOCAL_EL_DEFS_TLS_SIZE];

RSSL_API RsslRet rsslDecodeElementList( 
							  RsslDecodeIterator		*oIter,
							  RsslElementList			*oElementList,
							  const RsslLocalElementSetDefDb	*iLocalSetDb )
{
	char *position;
	char *_endBufPtr;
	RsslDecodingLevel *_levelInfo;
		
	RSSL_ASSERT(oIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(oIter->_curBufPtr, Invalid iterator use - check buffer);

	_levelInfo = &oIter->_levelInfo[++oIter->_decodingLevel]; if (oIter->_decodingLevel >= RSSL_ITER_MAX_LEVELS) return RSSL_RET_ITERATOR_OVERRUN;
	_rsslSetupDecodeIterator(_levelInfo, RSSL_DT_ELEMENT_LIST, oElementList); 

	position = oIter->_curBufPtr;
	_endBufPtr = _levelInfo->_endBufPtr;

	if (_endBufPtr == position)
	{
		_endOfList(oIter);
		return RSSL_RET_NO_DATA;
	}

	/* get flags */
	position += rwfGet8(oElementList->flags, position);
	 
	/* get element list information */
	if (rsslElementListHasInfo(oElementList))
	{
		RsslUInt8 infoLen;
		char * startpos;

		/* has 1 byte length */
		position += rwfGet8(infoLen, position);
		startpos = position;

		position += rwfGet16(oElementList->elementListNum, position);

		if ((startpos + infoLen) > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		/* used info length to skip  */
		position = startpos + infoLen;
	}

	/* get set data */
	if (rsslElementListCheckHasSetData(oElementList))
	{
		/* get set id */
		if (rsslElementListCheckHasSetId(oElementList))
			position += rwfGetResBitU15(&oElementList->setId, position);
		else
			oElementList->setId = 0;

		/* set definitoin from the local or global set list */
		if (oElementList->setId <= RSSL_ELEMENT_SET_MAX_LOCAL_ID)
			_levelInfo->_elemListSetDef = iLocalSetDb && (iLocalSetDb->definitions[oElementList->setId].setId != RSSL_ELEMENT_SET_BLANK_ID) ? 
				&iLocalSetDb->definitions[oElementList->setId] : 0;
		else 
		if (oElementList->setId > RSSL_ELEMENT_SET_MAX_LOCAL_ID && oIter->_pGlobalElemListSetDb && (oElementList->setId <= oIter->_pGlobalElemListSetDb->maxSetId))
			_levelInfo->_elemListSetDef = oIter->_pGlobalElemListSetDb->definitions[oElementList->setId];
		else
			_levelInfo->_elemListSetDef = 0;

		/* check for element list data */
		if (rsslElementListCheckHasStandardData(oElementList))
		{
			/* if hassetdata and HasFieldList then set data is length specified */
			position = _rsslDecodeBuffer15(&oElementList->encSetData, position);
			if (position + sizeof(_levelInfo->_itemCount) > _endBufPtr)
				return RSSL_RET_INCOMPLETE_DATA;

			/* get element list data */
			position += rwfGet16(_levelInfo->_itemCount, position);
			oElementList->encEntries.data = position;
			oElementList->encEntries.length = (rtrUInt32)(_endBufPtr - position);

			/* check for buffer overflow - post check ok since no copy */
			if (position > _endBufPtr)
				return RSSL_RET_INCOMPLETE_DATA;
		}
		else
		{
			/* get the element list set data - not length specified since no field list data exists */
			oElementList->encEntries.data = 0;
			oElementList->encEntries.length = 0;

			oElementList->encSetData.data = position;
			oElementList->encSetData.length = (rtrUInt32)(_endBufPtr - position);

			if (position > _endBufPtr)
				return RSSL_RET_INCOMPLETE_DATA;
		}

		/* Setup to decode the set if able. Otherwise skip to entries. */
		if (_levelInfo->_elemListSetDef)
		{
			_levelInfo->_setCount = _levelInfo->_elemListSetDef->count;
			_levelInfo->_itemCount += _levelInfo->_elemListSetDef->count; 

			_levelInfo->_nextEntryPtr = /* oIter->_curBufPtr = */
				(_levelInfo->_setCount > 0) ? oElementList->encSetData.data : position;

			return RSSL_RET_SUCCESS;
		}
		else
		{
			_levelInfo->_setCount = 0;
			_levelInfo->_nextEntryPtr = /* oIter->_curBufPtr = */ position + oElementList->encSetData.length;
			return RSSL_RET_SET_SKIPPED;
		}
	}
	else if (rsslElementListCheckHasStandardData(oElementList))
	{
		/* get element list data only */
		oElementList->encSetData.data = 0;
		oElementList->encSetData.length = 0;

		position += rwfGet16(_levelInfo->_itemCount, position);
		oElementList->encEntries.data = position;
		oElementList->encEntries.length = (rtrUInt32)(_endBufPtr - position);
		
		oIter->_curBufPtr = position;
		_levelInfo->_setCount = 0;
	}
	else
	{
		oElementList->encSetData.data = 0;
		oElementList->encSetData.length = 0;
		oElementList->encEntries.data = 0;
		oElementList->encEntries.length = 0;
		_levelInfo->_itemCount = 0;
		oIter->_curBufPtr = 0;
		_levelInfo->_setCount = 0;
	}

	_levelInfo->_nextEntryPtr = oIter->_curBufPtr;
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeElementEntry( RsslDecodeIterator *iIter, RsslElementEntry *oElement )
{
	char		*position;
	RsslElementList	*elementList;
	RsslDecodingLevel *_levelInfo;
	RSSL_ASSERT(iIter->_decodingLevel > -1 && iIter->_decodingLevel < 17, Invalid or incorrect iterator used);
	_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

	RSSL_ASSERT(oElement && iIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(iIter->_levelInfo[iIter->_decodingLevel]._listType, Invalid decoding attempted);

	elementList = (RsslElementList*)_levelInfo->_listType;

	if (_levelInfo->_nextItemPosition >= _levelInfo->_itemCount)
	{
		_endOfList(iIter);
		return RSSL_RET_END_OF_CONTAINER;
	}

	/* Make sure we skip the iterator to the next entry if we didn't decode the previous entry payload */
	position = iIter->_curBufPtr = _levelInfo->_nextEntryPtr;

	if (_levelInfo->_nextSetPosition < _levelInfo->_setCount)
	{
		RsslRet		ret;
		const RsslElementSetDefEntry *encoding = 0;

		RSSL_ASSERT(_levelInfo->_elemListSetDef, Invalid parameters or parameters passed in as NULL);
		RSSL_ASSERT(_levelInfo->_elemListSetDef->count == _levelInfo->_setCount, Invalid data);

		encoding = &(_levelInfo->_elemListSetDef->pEntries[_levelInfo->_nextSetPosition]);

		oElement->name = encoding->name;
		oElement->dataType = encoding->dataType;

		/* get the set data and reset position */
		if ((ret = _rsslDecodeSet(iIter, encoding->dataType, &oElement->encData)) != RSSL_RET_SUCCESS)
			return(ret);

		iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr = _levelInfo->_nextEntryPtr;

    	/* handle legacy conversion */
    	oElement->dataType = _rsslPrimitiveType(oElement->dataType);

		_levelInfo->_nextItemPosition++;
		_levelInfo->_nextSetPosition++;

		if (_levelInfo->_nextSetPosition == _levelInfo->_setCount && elementList->encEntries.data)
			_levelInfo->_nextEntryPtr = elementList->encEntries.data;

		return RSSL_RET_SUCCESS;
	}

	/* get normal element list data */
	if (elementList->encEntries.data + elementList->encEntries.length - position < 3)
		return RSSL_RET_INCOMPLETE_DATA;

	position = _rsslDecodeBuffer15(&oElement->name, position);

	if (position > _levelInfo->_endBufPtr)
		return RSSL_RET_INCOMPLETE_DATA;
	position += rwfGet8(oElement->dataType, position);

	if (oElement->dataType != RSSL_DT_NO_DATA)
	{
		position += rwfGetBuffer16(&oElement->encData, position);

		if (position > _levelInfo->_endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;
    
    	/* handle legacy conversion */
    	oElement->dataType = _rsslPrimitiveType(oElement->dataType);
    
    	/* shift iterator */
    	iIter->_curBufPtr = oElement->encData.data;
    	_levelInfo->_nextEntryPtr = iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
    	_levelInfo->_nextItemPosition++;
    	return RSSL_RET_SUCCESS;
	}
	else
	{
		oElement->encData.data = 0;
		oElement->encData.length = 0;
    	_levelInfo->_nextItemPosition++;
    	iIter->_curBufPtr  = _levelInfo->_nextEntryPtr 
			= iIter->_levelInfo[iIter->_decodingLevel+1]._endBufPtr= position;
		return RSSL_RET_SUCCESS;
	}
}

/* element list set decoding */

RSSL_API RsslRet rsslDecodeLocalElementSetDefDb(
										RsslDecodeIterator		*pIter,
										RsslLocalElementSetDefDb		*oLocalSetDb )													
{
	char *position;
	char *_endBufPtr;
	RsslUInt8	_setCount;
	int		i, j;

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
			oLocalSetDb->entries.data = (char*)tempLocalELSetDefStorage[pIter->_decodingLevel];
			oLocalSetDb->entries.length = sizeof(tempLocalELSetDefStorage[0]); /* size of one level */
		}
	}
	else
	{
		/* Separate iterator. */
		_endBufPtr = pIter->_levelInfo[0]._endBufPtr;
		position = pIter->_curBufPtr;

		if (!oLocalSetDb->entries.data)
		{
			/* set this to our thread local storage */
			oLocalSetDb->entries.data = (char*)tempLocalELSetDefStorage[0];
			oLocalSetDb->entries.length = sizeof(tempLocalELSetDefStorage[0]); /* size of one level */
		}
	}

	if ((_endBufPtr - position) < 2)
		return RSSL_RET_INCOMPLETE_DATA;

	position += 1;
	position += rwfGet8(_setCount, position);

	if (_setCount == 0)
		return RSSL_RET_SET_DEF_DB_EMPTY;

	if (_setCount > RSSL_ELEMENT_SET_MAX_LOCAL_ID)
		return RSSL_RET_TOO_MANY_LOCAL_SET_DEFS;

	for (i=0; i<=RSSL_ELEMENT_SET_MAX_LOCAL_ID;i++)
	{
		oLocalSetDb->definitions[i].setId = RSSL_ELEMENT_SET_BLANK_ID;
		oLocalSetDb->definitions[i].count = 0;
		oLocalSetDb->definitions[i].pEntries = NULL;
	}

	for (i = 0; i < _setCount; i++)
	{
		RsslElementSetDef		*pCurSetDef;
		RsslElementSetDefEntry	*pCurEntry;
		RsslUInt16				setId;
		RsslUInt8				encCount;

		if ((position + 2) > _endBufPtr)
			return RSSL_RET_INCOMPLETE_DATA;

		/* get the setId and the number of element encodings */
		position += rwfGetResBitU15(&setId, position);
		position += rwfGet8(encCount, position);

		/*
		printf("  ELSDEF: setId %d count %d\n", setId, encCount);
		*/

		/* sanity checks */
		if (setId > RSSL_ELEMENT_SET_MAX_LOCAL_ID)
			return RSSL_RET_ILLEGAL_LOCAL_SET_DEF;
		if (oLocalSetDb->definitions[setId].setId != RSSL_ELEMENT_SET_BLANK_ID)
			return RSSL_RET_DUPLICATE_LOCAL_SET_DEFS;

		/* get memory for new set definition from working memory. */
		pCurSetDef = &oLocalSetDb->definitions[setId];
  
		/* make we have space in our entry pool */
		if (((curEntry + (RsslUInt32)encCount)*(sizeof(RsslElementSetDefEntry))) > oLocalSetDb->entries.length)
			return RSSL_RET_BUFFER_TOO_SMALL;

		/* setup set def and put in database */
		pCurSetDef->setId = setId;
		pCurSetDef->count = encCount;
		if (encCount > 0)
		{
			pCurSetDef->pEntries = &((RsslElementSetDefEntry*)(oLocalSetDb->entries.data))[curEntry];

			/* populate the entries */
			for (j= 0; j< encCount; j++)
			{
				pCurEntry = &((RsslElementSetDefEntry*)(oLocalSetDb->entries.data))[curEntry];
				position = _rsslDecodeBuffer15(&pCurEntry->name, position);
				position += rwfGet8(pCurEntry->dataType, position);
				++curEntry;
			}
		}
	}
	return (position <= _endBufPtr) ? RSSL_RET_SUCCESS : RSSL_RET_INCOMPLETE_DATA;
}
		
RSSL_API RsslRet rsslElementListFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt8 flags)
{
	const char *flagStrings[4 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_ELF_HAS_ELEMENT_LIST_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_ELF_HAS_ELEMENT_LIST_INFO.data;
	if (flags & RSSL_ELF_HAS_SET_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_ELF_HAS_SET_DATA.data;
	if (flags & RSSL_ELF_HAS_SET_ID) flagStrings[flagCount++] = RSSL_OMMSTR_ELF_HAS_SET_ID.data;
	if (flags & RSSL_ELF_HAS_STANDARD_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_ELF_HAS_STANDARD_DATA.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}
