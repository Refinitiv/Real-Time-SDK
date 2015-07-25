

#ifndef __RSSL_ARRAY_H
#define __RSSL_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslDataTypeEnums.h"
#include "rtr/rsslIterators.h"


/** 
 * @addtogroup ArrayStruct RsslArray Structure and Initializers
 * @{
 */

/**
 * @brief The RsslArray is a uniform primitive type that can contain multiple simple primitive entries.  Entries can be either fixed length or variable length.
 * @see RSSL_INIT_ARRAY, rsslClearArray, rsslDecodeArray
 */
typedef struct
{
	RsslPrimitiveType	primitiveType;		/*!< @brief Primitive type (0 - 64) for all items in the RsslArray */
	RsslUInt16			itemLength;			/*!< @brief If items are fixed length populate length here - otherwise make 0 for length specified item encoding */
	RsslBuffer			encData;			/*!< @brief Raw data contents of the RsslArray */
} RsslArray;

/**
 * @brief RsslArray static initializer
 * @see RsslArray, rsslClearArray
 */
#define RSSL_INIT_ARRAY { 0, 0, RSSL_INIT_BUFFER }

/**
 * @brief Clears an RsslArray 
 * @see RsslArray, RSSL_INIT_ARRAY
 */
RTR_C_INLINE void rsslClearArray(RsslArray *pArray)
{
	pArray->primitiveType = 0;
	pArray->itemLength = 0;
	pArray->encData.length = 0;
	pArray->encData.data = 0;
	
}

/**
 * @}
 */

/**
 * @addtogroup ArrayEncoding RsslArray Encoding
 * @{
 */

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

RSSL_API RsslRet rsslEncodeArrayInit( 
                    RsslEncodeIterator		*pIter,
					RsslArray				*pArray
					 );


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
RSSL_API RsslRet rsslEncodeArrayComplete( 
						RsslEncodeIterator		*pIter,
						RsslBool				success );



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
RSSL_API RsslRet rsslEncodeArrayEntry( RsslEncodeIterator	 *pIter, 
											 const RsslBuffer	 *pEncData,
										     const void			 *pData );


/**
 * @}
 */

/**
 * @addtogroup ArrayDecoding RsslArray Decoding
 * @{
 */

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
RSSL_API RsslRet rsslDecodeArray(RsslDecodeIterator	*pIter,
									   RsslArray			*pArray
									   );


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
RSSL_API RsslRet rsslDecodeArrayEntry(RsslDecodeIterator *pIter, 
											RsslBuffer		   *pBuffer
											 );
										   
/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
