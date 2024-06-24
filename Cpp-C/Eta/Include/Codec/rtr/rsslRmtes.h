/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

 #ifndef __RSSL_RMTES_H__
#define __RSSL_RMTES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslRetCodes.h"

/**
 *	@addtogroup RsslRmtesCacheStruct
 *	@{
 */	

/**
 *	@brief RsslRmtesCacheBuffer structure
 *
 *	This buffer contains a data member that points 
 *	to the first byte of char* data, an RsslUInt32 
 *	that describes the total allocated length of the 
 *	data buffer, and an RsslUInt32 that describes the 
 *	total used length of the buffer.<BR>
 *
 *	This buffer is used by the RMTES Decoding interface to
 *	store data that may have marketfeed semantics applied to it.
 *
 *	@see rsslClearRmtesCacheBuffer, rsslRMTESApplyToCache
 */
typedef struct {
	RsslUInt32 		length;				/*!< @brief Length of data used in the buffer. */
	RsslUInt32 		allocatedLength;	/*!< @brief Total allocated length of the buffer. */
	char* 			data;				/*!< @brief Pointer to the first byte of char* data contained in the buffer */
} RsslRmtesCacheBuffer;

/**
 *	@brief Clears an RsslRmtesCacheBuffer structure
 *
 *	@param pBuffer Pointer to RsslRmtesCacheBuffer to clear
 */
RTR_C_INLINE void rsslClearRmtesCacheBuffer(RsslRmtesCacheBuffer *pBuffer)
{
	pBuffer->length = 0;
	pBuffer->allocatedLength = 0;
	pBuffer->data = 0;
}

/**
 *	@}
 */
 
/**
 *	@addtogroup RsslU16BufferStruct
 *	@{
 */	

/**
 *	@brief RsslU16Buffer structure
 *
 *	This buffer contains a data member that points 
 *	to the first byte of RsslUInt16 data, an RsslUInt32 
 *	that describes the length of the buffer.<BR>
 *
 *	This buffer is used by the RMTES Decoding interface to
 *	store UCS2 Unicode strings.
 *
 *	@see rsslClearRmtesCacheBuffer, rsslRMTESToUCS2
 */
typedef struct {
	RsslUInt32 length;		/*!< @brief Length of data used in the buffer. */
	RsslUInt16* data;		/*!< @brief Pointer to the first byte of RsslUInt16* data contained in the buffer */
} RsslU16Buffer;


/**
 *	@brief Clears an RsslU16Buffer structure
 *
 *	@param pBuffer Pointer to RsslU16Buffer to clear
 */
RTR_C_INLINE void rsslClearU16Buffer(RsslU16Buffer *pBuffer)
{
	pBuffer->length = 0;
	pBuffer->data = 0;
}

/**
 *	@}
 */

/**
 *	@addtogroup RsslRmtesDec 
 *	@{
 */

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
 *	@return RsslRet code indicating success or failure:<BR>  
 *			::RSSL_RET_SUCCESS if the operation completes<BR>
 *			::RSSL_RET_INVALID_ARGUMENT if the input buffer has not been allocated<BR> 
 *			::RSSL_RET_BUFFER_TOO_SMALL if the cache buffer is too small<BR>
 *			::RSSL_RET_FAILURE if the operation is unable to complete due to invalid data<BR>
 */
RSSL_API RsslRet rsslRMTESApplyToCache(RsslBuffer *inBuffer, RsslRmtesCacheBuffer *cacheBuf);

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
 *	@return RsslRet code indicating success or failure:<BR>  
 *			::RSSL_RET_SUCCESS if the operation completes<BR>
 *			::RSSL_RET_INVALID_DATA if the input buffer has not been allocated<BR> 
 *			::RSSL_RET_BUFFER_TOO_SMALL if the output buffer is too small<BR>
 *			::RSSL_RET_FAILURE if the operation is unable to complete due to invalid data<BR>
 */
RSSL_API RsslRet rsslRMTESToUTF8(RsslRmtesCacheBuffer *pRmtesBuffer, RsslBuffer *charBuffer);

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
 *	@return RsslRet code indicating success or failure:<BR>  
 *			::RSSL_RET_SUCCESS if the operation completes<BR>
 *			::RSSL_RET_INVALID_DATA if the input buffer has not been allocated<BR> 
 *			::RSSL_RET_BUFFER_TOO_SMALL if the output buffer is too small<BR>
 *			::RSSL_RET_FAILURE if the operation is unable to complete due to invalid data<BR>
 */
RSSL_API RsslRet rsslRMTESToUCS2(RsslRmtesCacheBuffer *pRmtesBuffer, RsslU16Buffer *shortBuffer);

/**
 *	@brief Checks the presence of partial update commands in the buffer.
 *	
 *	@param pBuffer Pointer to the RsslBuffer to check
 *	@return RSSL_TRUE if a partial update command is present; RSSL_FALSE if not.
 */
RSSL_API RsslBool rsslHasPartialRMTESUpdate(RsslBuffer *pBuffer);

/**
 *	@}
 */


#ifdef __cplusplus
}
#endif

#endif
