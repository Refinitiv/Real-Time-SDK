/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2022 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_ITERATORS_H
#define __RTR_RSSL_ITERATORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslSetData.h"


/**
 * @addtogroup RsslIteratorVersion
 * @{
 */

/**
 * @brief Protocol type definition for RWF, can be used in conjunction transport layer to indicate protocol being used on the connection. 
 * @see rsslConnectOpts, rsslBindOpts, rsslConnect, rsslBind
 */
#define     RSSL_RWF_PROTOCOL_TYPE  0 
#define     RSSL_JSON_PROTOCOL_TYPE  2 

/**
 * @brief Version Major number for the version of RWF supported by this Message and Data package
 * @see rsslSetEncodeIteratorRWFVersion, rsslSetDecodeIteratorRWFVersion
 */
#define     RSSL_RWF_MAJOR_VERSION  14 

/**
 * @brief Version Minor number for the version of RWF supported by this Message and Data package
 * @see rsslSetEncodeIteratorRWFVersion, rsslSetDecodeIteratorRWFVersion
 */
#define     RSSL_RWF_MINOR_VERSION  1 


/** 
 * @brief Maximum supported RWF versions that this library can encode or decode 
 * @see rsslSetEncodeIteratorRWFVersion, rsslSetDecodeIteratorRWFVersion
 */
#define		RSSL_RWF_MAX_SUPPORTED_MAJOR_VERSION 14
/** 
 * @brief Minimum supported RWF versions that this library can encode or decode 
 * @see rsslSetEncodeIteratorRWFVersion, rsslSetDecodeIteratorRWFVersion
 */
#define		RSSL_RWF_MIN_SUPPORTED_MAJOR_VERSION 14


/**
 * @}
 */



/**
 * @addtogroup RsslIteratorGroup
 * @{
 */

/**
 * @brief Encoding or Decoding depth supported with a single RSSL Iterator.  If a domain model requires deeper than RSSL_ITER_MAX_LEVELS levels, multiple iterators or pre-encoded data can be used to nest as deep as required.
 * @see RsslEncodeIterator, RsslDecodeIterator
 */
#define RSSL_ITER_MAX_LEVELS 16

/**
 * @}
 */



/* INTERNAL START: Content below this block is used internally by the iterators - the user should not need to modify or manipulate */

/* Create a forward declaration for pointer to iterator */
typedef struct RsslEncIterator *rsslEncIterPtr;
typedef struct RsslDecIterator *rsslDecIterPtr;




/*
 * Buffer size encoding marker, used internally.
 */
typedef struct
{
	char			*_sizePtr;			/* Position of size to encode */
	RsslUInt8		_sizeBytes;			/* Number of bytes reserved for the size */
} RsslEncodeSizeMark;


/*
 * Clears an RsslEncodeSizeMark 
 */
RTR_C_ALWAYS_INLINE void rsslClearEncodeSizeMark(RsslEncodeSizeMark *pMark)
{
	pMark->_sizePtr = 0;
	pMark->_sizeBytes = 0;
}


/*
 * Encoding buffer size marker static initializer
 */
#define RSSL_INIT_ENCODE_SIZE_MARK {0, 0}

/*
 * Information for one level of nesting in the encode iterator, used internally
 */
typedef struct RsslEncLevel
{
	char					*_countWritePtr;	/* The position of the count pointer in the buffer */
	char					*_initElemStartPos;	/* The start position of an initialize element */
	char					*_containerStartPos;/* The start position of a container */
	RsslUInt16				_currentCount;		/* Current number of elements */
	RsslUInt8				_encodingState;		/* The current encoding state */
	RsslUInt8				_containerType;		/* The containerType of the list */
	RsslUInt32				_flags;				/* Internal flag values used for various purposes */
	void					*_listType;			/* Pointer to actual list type */
	const RsslFieldSetDef	*_fieldListSetDef;  /* RsslFieldListSetDef, used to encode this level */
	const RsslElementSetDef	*_elemListSetDef;	/* RsslElementListSetDef, used to decode this level */
	RsslEncodeSizeMark		_internalMark;		/* Buffer size mark used internally */
	RsslEncodeSizeMark		_internalMark2;		/* Second Internal mark  */
} RsslEncodingLevel;


/*
 * Encoding level static initializer
 */
#define RSSL_INIT_ENCODING_LEVEL { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, RSSL_INIT_ENCODE_SIZE_MARK, RSSL_INIT_ENCODE_SIZE_MARK }


/*
 * Information for one level of nesting in the decode iterator, used internally
 */
typedef struct RsslDecIterLevel
{
	char					*_endBufPtr;		/* The end of the buffer being decoded */
	void					*_listType;			/* Pointer to actual list type being decoded */
	char					*_nextEntryPtr;		/* Expected start of next entry */
	const RsslFieldSetDef	*_fieldListSetDef;  /* RsslFieldListSetDef, used to decode this level */
	const RsslElementSetDef	*_elemListSetDef;	/* RsslElementListSetDef, used to decode this level */
	RsslUInt16				_itemCount;			/* number of items in the list */
	RsslUInt16				_nextItemPosition;	/* index of next item.  Iterator is off when _nextItemPosition >= itemCount */
	RsslUInt16				_setCount;			/* number of items in the set */
	RsslUInt16				_nextSetPosition;	/* index of next item in a set */
	RsslUInt8				_containerType;		/* Type of container to decode for this level */
} RsslDecodingLevel;

/*
 * Decoding level static initializer
 */
#define RSSL_INIT_DECODING_LEVEL {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}


/* INTERNAL END: Content above this block is used internally by the iterators - the user should not need to modify or manipulate */



/** 
 * @addtogroup RsslEncodeIteratorType 
 * @{
 */

/**
 * @brief Iterator used for RWF encoding
 * @see rsslClearEncodeIterator, rsslSetEncodeIteratorBuffer, rsslSetEncodeIteratorRWFVersion
 */
typedef struct RsslEncIterator
{
	RsslBuffer				*_pBuffer;							/*!< Buffer to encode into, should be set with rsslSetEncodeIteratorBuffer().  RsslBuffer::length should indicate the number of bytes available.*/
	char					*_curBufPtr;						/*!< The current encoding position in the associated buffer */
	char					*_endBufPtr;						/*!< The end of the buffer for encoding, used to ensure that encoding functions do not go past end of buffer */
	RsslUInt8				_majorVersion;						/*!< RWF Major Version this iterator will encode, should be set with rsslSetEncodeIteratorRWFVersion() */
	RsslUInt8				_minorVersion;						/*!< RWF Minor Version this iterator will encode, should be set with rsslSetEncodeIteratorRWFVersion() */
	RsslInt8				_encodingLevel;						/*!< Current nesting level */
	RsslEncodingLevel		_levelInfo[RSSL_ITER_MAX_LEVELS];   /*!< Information needed for encoding across all levels */
	RsslElementSetDefDb		*_pGlobalElemListSetDb;				/*!< Pointer to a global set definition for element lists.*/
	RsslFieldSetDefDb		*_pGlobalFieldListSetDb;			/*!< Pointer to a global set definition for field lists.*/
} RsslEncodeIterator;

/* @brief Sets a global Element List Set Definition Database on the iterator.
 * @see RsslElementSetDefDb
 */
RTR_C_ALWAYS_INLINE void rsslSetEncodeIteratorGlobalElementListSetDB(RsslEncodeIterator *pIter, RsslElementSetDefDb *pDb)
{
	pIter->_pGlobalElemListSetDb = pDb;
}

/* @brief Sets a global Field List Set Definition Database on the iterator.
 * @see RsslFieldSetDefDb
 */
RTR_C_ALWAYS_INLINE void rsslSetEncodeIteratorGlobalFieldListSetDB(RsslEncodeIterator *pIter, RsslFieldSetDefDb *pDb)
{
	pIter->_pGlobalFieldListSetDb = pDb;
}




/**
 * @brief Clears an RsslEncodeIterator, defaults to use current version of RWF. 
 * 
 * After clearing an iterator, the buffer needs to be set using rsslSetEncodeIteratorBuffer().  If using a different RWF version, this should be set using rsslSetEncodeIteratorRWFVersion().
 * @note This should be used to achieve the best performance while clearing the iterator.
 * @param pIter \ref RsslEncodeIterator to clear
 * @see RsslEncodeIterator, rsslSetEncodeIteratorBuffer, rsslSetEncodeIteratorRWFVersion, RSSL_INIT_ENCODE_ITERATOR
 */
RTR_C_ALWAYS_INLINE void rsslClearEncodeIterator( RsslEncodeIterator *pIter)
{
	pIter->_encodingLevel = -1;
	pIter->_pBuffer = 0;
	pIter->_curBufPtr = 0;
	pIter->_endBufPtr = 0;
	pIter->_majorVersion = RSSL_RWF_MAJOR_VERSION;  //This should be initialized to the MAJOR version of RWF being encoded
	pIter->_minorVersion = RSSL_RWF_MINOR_VERSION;  // This should be initialized to the MINOR version of RWF being encoded
	pIter->_pGlobalFieldListSetDb = NULL;
	pIter->_pGlobalElemListSetDb = NULL;
}



/**
 * @brief \ref RsslEncodeIterator static initialization, defaults to use current version of RWF
 *
 * After clearing an iterator, the buffer needs to be set using rsslSetEncodeIteratorBuffer().  If using a different RWF version, this should be set using rsslSetEncodeIteratorRWFVersion().
 * @warning Because this clears every member of every level of the iterator, this should not be used
 * to achieve best performance.  Use the rsslClearEncodeIterator() as it clears only necessary members.
 * @see RsslEncodeIterator, rsslSetEncodeIteratorBuffer, rsslSetEncodeIteratorRWFVersion, rsslClearEncodeIterator
 */
#define RSSL_INIT_ENCODE_ITERATOR  { 0, 0, 0, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, -1, { \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL, \
	RSSL_INIT_ENCODING_LEVEL \
}, 0, 0 \
}

/**
 * @brief Set the \ref RsslBuffer to encode into on the \ref RsslEncodeIterator.  RsslBuffer::data should point to memory to encode into, RsslBuffer::length should be set to number of bytes pointed to.  
 * @param pIter \ref RsslEncodeIterator to set buffer on 
 * @param pBuffer \ref RsslBuffer to associate with iterator and encode into
 * @see RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_SUCCESS if \ref RsslBuffer is successfully associated with iterator, ::RSSL_RET_FAILURE if not, typically due to invalid RsslBuffer::length or RsslBuffer::data.
 */
RTR_C_ALWAYS_INLINE RsslRet rsslSetEncodeIteratorBuffer(
	RsslEncodeIterator		*pIter,
	RsslBuffer				*pBuffer)
{
	if (pBuffer->length && pBuffer->data)
	{
		pIter->_pBuffer = pBuffer;
		pIter->_curBufPtr = pBuffer->data;
		pIter->_endBufPtr = pBuffer->data + pBuffer->length;

		return RSSL_RET_SUCCESS;
	}
	else
	{
		pIter->_pBuffer = 0;
	}
	return RSSL_RET_FAILURE;
}


/**
 * @brief Set the RWF version to encode with this iterator.  See \ref RsslIteratorVersion for more information about RWF versioning. 
 *
 * @note When using rsslClearEncodeIterator(), the default RWF versions are set on the iterator.
 * @param pIter \ref RsslEncodeIterator to set version information on
 * @param rwfMajorVersion The major version of RWF to be encoded by this iterator
 * @param rwfMinorVersion The minor version of RWF to be encoded by this iterator
 * @see RsslEncodeIterator, rsslClearEncodeIterator, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION
 * @return RsslRet ::RSSL_RET_SUCCESS when version is properly associated with \ref RsslEncodeIterator, ::RSSL_RET_FAILURE otherwise, typically if version is outside of this libraries supported values.
 */
RTR_C_ALWAYS_INLINE RsslRet rsslSetEncodeIteratorRWFVersion(RsslEncodeIterator *pIter,
															RsslUInt8 rwfMajorVersion,
															RsslUInt8 rwfMinorVersion
															)
{	
	RsslUInt8 majorMax = RSSL_RWF_MAX_SUPPORTED_MAJOR_VERSION;
	RsslUInt8 majorMin = RSSL_RWF_MIN_SUPPORTED_MAJOR_VERSION;
	
	/* This function does not validate minor version as this should not
	   break wire format compatibility */
	if ((rwfMajorVersion <= majorMax) && (rwfMajorVersion >= majorMin))
	{
		pIter->_majorVersion = rwfMajorVersion;
		pIter->_minorVersion = rwfMinorVersion;
		return RSSL_RET_SUCCESS;
	}
	return RSSL_RET_FAILURE;

}



/**
 * @brief Returns the length of content encoded by this iterator since the last time it was cleared.
 *
 * Typical use:<BR>
 * Can be used to determine the length of the buffer used so far as well as the length of encoded messages, buffers, element lists, etc.
 *
 * @param pIter Pointer to the encode iterator to return encoded length from
 * @see RsslEncodeIterator
 * @return RsslUInt32 current encoded length
*/
RTR_C_ALWAYS_INLINE RsslUInt32 rsslGetEncodedBufferLength(
					const RsslEncodeIterator	*pIter )
{
	/* It is expected user passes in a valid iterator */
	return (RsslUInt32)((char*)pIter->_curBufPtr - (char*)pIter->_pBuffer->data);
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
RSSL_API RsslRet rsslRealignEncodeIteratorBuffer(
		RsslEncodeIterator	*pIter,
		RsslBuffer			*pNewEncodeBuffer 
		);


/**
 * @}
 */


/** 
 * @addtogroup RsslDecodeIteratorType 
 * @{
 */



/**
 * @brief Iterator used for RWF decoding
 * @see rsslClearDecodeIterator, rsslSetDecodeIteratorBuffer, rsslSetDecodeIteratorRWFVersion
 */
typedef struct RsslDecIterator
{
	RsslUInt8				_majorVersion;							/*!< RWF Major Version this iterator will encode, should be set with rsslSetDecodeIteratorRWFVersion() */
	RsslUInt8				_minorVersion;							/*!< RWF Minor Version this iterator will encode, should be set with rsslSetDecodeIteratorRWFVersion() */
	RsslInt8				_decodingLevel;							/*!< Current level in _levelInfo */
	char					*_curBufPtr;							/*!< The current decoding position in the associated buffer */
	RsslBuffer				*_pBuffer;								/*!< Buffer to decode from, should be set with rsslSetDecodeIteratorBuffer().  RsslBuffer::length should indicate the number of bytes available to decode.*/
	RsslDecodingLevel		_levelInfo[RSSL_ITER_MAX_LEVELS];		/*!< Information needed for decoding across all levels of iterator*/
	RsslElementSetDefDb		*_pGlobalElemListSetDb;				/*!< Pointer to a global set definition for element lists.*/
	RsslFieldSetDefDb		*_pGlobalFieldListSetDb;			/*!< Pointer to a global set definition for field lists.*/
} RsslDecodeIterator;



/**
 * @brief Clears an RsslDecodeIterator, defaults to use current version of RWF. 
 * 
 * After clearing an iterator, the buffer needs to be set using rsslSetDecodeIteratorBuffer().  If using a different RWF version, this should be set using rsslSetDecodeIteratorRWFVersion().
 *
 * @note This should be used to achieve the best performance while clearing the iterator.
 * @param pIter \ref RsslDecodeIterator to clear
 * @see RsslDecodeIterator, rsslSetDecodeIteratorBuffer, rsslSetDecodeIteratorRWFVersion, RSSL_INIT_DECODE_ITERATOR
 */
RTR_C_ALWAYS_INLINE void rsslClearDecodeIterator(RsslDecodeIterator *pIter)
{
	pIter->_majorVersion = RSSL_RWF_MAJOR_VERSION;  // This should be initialized to the RWF Major version being decoded
	pIter->_minorVersion = RSSL_RWF_MINOR_VERSION;   // This should be initialized to the RWF Minor version being decoded
	pIter->_decodingLevel = -1;
	pIter->_curBufPtr = NULL;
	pIter->_pBuffer = NULL;
	pIter->_pGlobalElemListSetDb = NULL;
	pIter->_pGlobalFieldListSetDb = NULL;
}

/* @brief Sets a global Element List Set Definition Database on the iterator.
 * @see RsslElementSetDefDb
 */
RTR_C_ALWAYS_INLINE void rsslSetDecodeIteratorGlobalElementListSetDB(RsslDecodeIterator *pIter, RsslElementSetDefDb* pDb)
{
	pIter->_pGlobalElemListSetDb = pDb;
}

/* @brief Sets a global Field List Set Definition Database on the iterator.
 * @see RsslFieldSetDefDb
 */
RTR_C_ALWAYS_INLINE void rsslSetDecodeIteratorGlobalFieldListSetDB(RsslDecodeIterator *pIter, RsslFieldSetDefDb* pDb)
{
	pIter->_pGlobalFieldListSetDb = pDb;
}




/**
 * @brief \ref RsslDecodeIterator static initialization, defaults to use current version of RWF
 *
 * After clearing an iterator, the buffer needs to be set using rsslSetDecodeIteratorBuffer().  If using a different RWF version, this should be set using rsslSetDecodeIteratorRWFVersion().
 *
 * @warning Because this clears every member of every level of the iterator, this should not be used 
 * to achieve best performance.  Use the rsslClearDecodeIterator() as it clears only necessary members.
 * @see RsslDecodeIterator, rsslSetDecodeIteratorBuffer, rsslSetDecodeIteratorRWFVersion, rsslClearDecodeIterator
 */
#define RSSL_INIT_DECODE_ITERATOR { RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, -1, 0, 0, { \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL, \
	RSSL_INIT_DECODING_LEVEL \
}, 0, 0 \
}


/**
 * @brief Set the \ref RsslBuffer to decode from on the \ref RsslDecodeIterator.  RsslBuffer::data should point to memory to decode, RsslBuffer::length should be set to number of bytes pointed to.  
 * @param pIter \ref RsslDecodeIterator to set buffer on 
 * @param pBuffer \ref RsslBuffer to associate with iterator and decode from
 * @see RsslDecodeIterator
 * @return RsslRet ::RSSL_RET_SUCCESS if \ref RsslBuffer is successfully associated with iterator, ::RSSL_RET_FAILURE if not, typically due to invalid RsslBuffer::length or RsslBuffer::data.
 */
RTR_C_ALWAYS_INLINE RsslRet rsslSetDecodeIteratorBuffer(
	RsslDecodeIterator		*pIter,
	RsslBuffer				*pBuffer)
{
	if (pBuffer->data || !pBuffer->length)
	{
		pIter->_pBuffer = pBuffer;
		pIter->_curBufPtr = pBuffer->data;
		pIter->_levelInfo[0]._endBufPtr = pBuffer->data + pBuffer->length; 
		return RSSL_RET_SUCCESS;
	}
	else
	{
		pIter->_pBuffer = 0;
	}
	return RSSL_RET_FAILURE;
}


/**
 * @brief Set the RWF version to decode with this iterator.  See \ref RsslIteratorVersion for more information about RWF versioning. 
 *
 * @note When using rsslClearDecodeIterator(), the default RWF versions are set on the iterator.
 * @param pIter \ref RsslDecodeIterator to set version information on
 * @param rwfMajorVersion The major version of RWF to be decoded by this iterator
 * @param rwfMinorVersion The minor version of RWF to be decoded by this iterator
 * @see RsslDecodeIterator, rsslClearDecodeIterator, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION
 * @return RsslRet ::RSSL_RET_SUCCESS when version is properly associated with \ref RsslDecodeIterator, ::RSSL_RET_FAILURE otherwise, typically if version is outside of this libraries supported values.
 */
RTR_C_ALWAYS_INLINE RsslRet rsslSetDecodeIteratorRWFVersion(RsslDecodeIterator *pIter,
															RsslUInt8 rwfMajorVersion,
															RsslUInt8 rwfMinorVersion
														    )
{	
	RsslUInt8 majorMax = RSSL_RWF_MAX_SUPPORTED_MAJOR_VERSION;
	RsslUInt8 majorMin = RSSL_RWF_MIN_SUPPORTED_MAJOR_VERSION;
	
	/* This function does not validate minor version as this should not
	   break wire format compatibility */
	if ((rwfMajorVersion <= majorMax) && (rwfMajorVersion >= majorMin))
	{
		pIter->_majorVersion = rwfMajorVersion;
		pIter->_minorVersion = rwfMinorVersion;
		return RSSL_RET_SUCCESS;
	}
	return RSSL_RET_FAILURE;
}



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
 RSSL_API RsslRet rsslFinishDecodeEntries(RsslDecodeIterator *pIter);

/**
 * @}
 */



#ifdef __cplusplus
}
#endif 

#endif

