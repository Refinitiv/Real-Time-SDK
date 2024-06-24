/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */


#ifndef __RSSL_ITERATOR_UTILS_INT_H
#define __RSSL_ITERATOR_UTILS_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"

#ifdef Linux
#undef RSSL_THREAD_LOCAL
#define RSSL_THREAD_LOCAL __thread
#endif

#if defined(_WIN32)
#undef RSSL_THREAD_LOCAL
#define RSSL_THREAD_LOCAL        __declspec(thread)
#endif


/* special flags for encode iterator use */
typedef enum {
	RSSL_EIF_NONE				= 0x0000,
	RSSL_EIF_LENU15				= 0x0001,	/*< Length needs to be encoded as u15 */
	RSSL_EIF_HAS_PER_ENTRY_PERM	= 0x0002	/* map, vector, or filter list needs per-entry perm flag set */
} RsslEncodeIteratorFlags;

/** 
 * @brief Internal iterator states 
 */
typedef enum {
	RSSL_EIS_NONE				= 0,
	RSSL_EIS_SET_DEFINITIONS	= 1,
	RSSL_EIS_SUMMARY_DATA		= 2,
	RSSL_EIS_SET_DATA			= 3,
	RSSL_EIS_SET_ENTRY_INIT		= 4,
	RSSL_EIS_PRIMITIVE			= 5,
	RSSL_EIS_PRIMITIVE_U15		= 6,
	RSSL_EIS_ENTRIES			= 7,
	RSSL_EIS_ENTRY_INIT			= 8,
	RSSL_EIS_ENTRY_WAIT_COMPLETE = 9,
	RSSL_EIS_SET_ENTRY_WAIT_COMPLETE = 10,
	RSSL_EIS_EXTENDED_HEADER	= 11,
	RSSL_EIS_OPAQUE				= 12,
	RSSL_EIS_OPAQUE_AND_EXTENDED_HEADER = 13,
	RSSL_EIS_WAIT_COMPLETE		= 14,
	RSSL_EIS_COMPLETE			= 15,
	RSSL_EIS_NON_RWF_DATA		= 16,
	RSSL_EIS_REQATTRIB							= 17,
	RSSL_EIS_OPAQUE_REQATTRIB					= 18,
	RSSL_EIS_EXTENDED_HEADER_REQATTRIB			= 19,
	RSSL_EIS_OPAQUE_EXTENDED_HEADER_REQATTRIB	= 20
} RsslEncodeIteratorStates;

/* For some reason, macros seem to perform better than inline functions here. */
#define _rsslSetupDecodeIterator(_levelInfo, cType, container) \
{ \
	(_levelInfo)->_itemCount = 0; \
	(_levelInfo)->_nextItemPosition = 0; \
	(_levelInfo)->_nextSetPosition = 0; \
	(_levelInfo)->_containerType = (cType); \
	(_levelInfo)->_listType = (container); \
}

#define _rsslInitEncodeIterator(lInfo, dFormat, eState, lType, cStartPos) \
{\
	(lInfo)->_countWritePtr = 0;\
	(lInfo)->_initElemStartPos = 0;\
	(lInfo)->_currentCount = 0;\
	(lInfo)->_encodingState = (eState);\
	(lInfo)->_containerType = (dFormat);\
	(lInfo)->_internalMark._sizePtr = 0;\
	(lInfo)->_internalMark._sizeBytes = 0;\
	(lInfo)->_internalMark2._sizePtr = 0;\
	(lInfo)->_internalMark2._sizeBytes = 0;\
	(lInfo)->_fieldListSetDef = 0; \
	(lInfo)->_elemListSetDef = 0; \
	(lInfo)->_listType = (lType);\
	(lInfo)->_containerStartPos = (cStartPos);\
}

/* Called when decoding a container and the end of the container's entries is reached. */
RTR_C_ALWAYS_INLINE void _endOfList(RsslDecodeIterator *iIter)
{
	/* Go back to previous level and check its type */
    RsslDecodingLevel *_levelInfo;

	while(--iIter->_decodingLevel >= 0)
	{
		_levelInfo = &iIter->_levelInfo[iIter->_decodingLevel];

		switch(_levelInfo->_containerType)
		{
			case RSSL_DT_MSG: /* This was contained by a message */
				break; /* Keep unwinding (in case the message was contained by another message) */

			case RSSL_DT_NO_DATA: /* Finished decoding a 'temporary' container. Just undo the changes. */
				iIter->_curBufPtr = _levelInfo->_nextEntryPtr;
				--iIter->_decodingLevel;
				return; /* STOP */

			default: /* Inside an RWF container */
				return; /* STOP */
		}
	}
}

/**
 * @brief This function is used to query the iterater state.
 * @param pIter Encoder Iterator
 * @see RsslEncodeIterator, RSSL_INIT_ENCODE_ITERATOR, RsslEncoderIteratorStates
 */
RTR_C_ALWAYS_INLINE RsslUInt8 rsslGetEncodeIteratorState(RsslEncodeIterator	*pIter)
{
	return pIter->_levelInfo[pIter->_encodingLevel]._encodingState;
}

/**
 * @brief This function is for overriding iterator so you can encode specific types into a stand alone buffer
 * Should not be used unless it is specifically known what is being done
 * @see RsslEncodeIterator, RSSL_INIT_ENCODE_ITERATOR
 */
RTR_C_ALWAYS_INLINE void rsslSetEncodeIterator(
			RsslEncodeIterator	*pIter,
			RsslBuffer			*pBuffer,
			RsslUInt8			encodingState )
{
	RsslEncodingLevel *_levelInfo;

	pIter->_encodingLevel = 0;
	_levelInfo = &pIter->_levelInfo[pIter->_encodingLevel];

	pIter->_pBuffer = pBuffer;
	pIter->_curBufPtr = pBuffer->data;
	pIter->_endBufPtr = pBuffer->data + pBuffer->length;
	_levelInfo->_countWritePtr = 0;
	_levelInfo->_initElemStartPos = 0;
	_levelInfo->_currentCount = 0;
	_levelInfo->_encodingState = encodingState;
	_levelInfo->_containerType = 0;
	_levelInfo->_internalMark._sizePtr = 0;
	_levelInfo->_internalMark._sizeBytes = 0;
	_levelInfo->_listType = 0;
}

/**
 * @brief Moves the iterator forward as specified by increment. 
 *
 * Moves the iterator forward as specified by increment.
 * Usefull for "injecting" your own data. 
 * Typical use: <BR>
 * 1. Encode message up to the part for your data format
 * 3. Populate your "own" part of the buffer, using, for example, memcpy. <BR>
 * 4. Call rsslAdavanceEncoderIterator() to move the iterator <BR>
 * @param pIter	Pointer to the encoder iterator.
 * @param increment How far to move the iterator forward.
 * @see RsslEncodeIterator
*/
RTR_C_ALWAYS_INLINE RsslRet rsslAdvanceEncodeIterator(
                  	RsslEncodeIterator	*pIter,
					RsslUInt16	   		increment )
{
	/* It is expected that the user passes in a valid iterator
	   which has a populated buffer */

	if ((pIter->_curBufPtr + increment) > pIter->_endBufPtr)
		return RSSL_RET_BUFFER_TOO_SMALL;

	pIter->_curBufPtr += increment;

	return RSSL_RET_SUCCESS;
}

/**
 * @brief Directly writes data into the iterator's buffer. Intended for encoding some legacy data. Use only with extreme caution.
 * @param pIter Iterator to encode into
 * @param pBuffer Data to write
 */
RTR_C_ALWAYS_INLINE RsslRet rsslWritePreEncodedData(RsslEncodeIterator *pIter, const RsslBuffer *pBuffer)
{
	if (pIter->_curBufPtr + pBuffer->length > pIter->_endBufPtr)
		return RSSL_RET_BUFFER_TOO_SMALL;

	memcpy(pIter->_curBufPtr, pBuffer->data, pBuffer->length); pIter->_curBufPtr += pBuffer->length;
	return RSSL_RET_SUCCESS;
}

/**
 ** @brief Get item count from decode iterator 
 ** @param pIter decode iterator
 ** @see RsslDecodeIterator
 **/
RTR_C_ALWAYS_INLINE RsslUInt16 rsslDecodeItemCount(RsslDecodeIterator	*pIter)
{
	return pIter->_levelInfo[pIter->_decodingLevel]._itemCount;
}




#ifdef __cplusplus
}
#endif

#endif

