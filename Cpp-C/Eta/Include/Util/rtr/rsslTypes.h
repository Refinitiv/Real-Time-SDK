/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_PRIMITIVE_TYPES_H
#define __RSSL_PRIMITIVE_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "rtr/os.h"
 
/* Used when exporting or importing as Windows DLL or linking as static library */
#if defined(RSSL_EXPORTS)
	#define 	RSSL_API				RTR_API_EXPORT
#elif defined(RSSL_IMPORTS)
	#define 	RSSL_API				RTR_API_IMPORT
#else
	#define 	RSSL_API
#endif


/**
 *	@addtogroup DataPrimitive
 *	@{
 */
	
/* These definitions leverage some common platform type defines 
 * contained in os.h which is a header file used for cross-platform
 * development
 */

/**
 * @brief System signed 8 bit integer value
 */
typedef rtrInt8 RsslInt8;

/**
 * @brief System unsigned 8 bit integer value 
 */
typedef rtrUInt8 RsslUInt8;

/**
 * @brief System signed 16 bit integer value
 */
typedef rtrInt16 RsslInt16;

/**
 * @brief System unsigned 16 bit integer value
 */
typedef rtrUInt16 RsslUInt16;

/** 
 * @brief System signed 32 bit integer value 
 */
typedef rtrInt32 RsslInt32;

/**
 * @brief System unsigned 32 bit integer value
 */
typedef rtrUInt32 RsslUInt32;

/** 
 * @brief System signed 64 bit integer value 
 */
typedef rtrInt64 RsslInt64;

/** 
 * @brief System unsigned 64 bit integer value
 */
typedef rtrUInt64 RsslUInt64;

/**
 * @}
 */

/** 
 * @addtogroup RsslIntType
 * @{
 */

/** 
 * @brief Rssl Signed Integer type 
 */
typedef RsslInt64 RsslInt;

/**
 * @}
 */

/** 
 * @addtogroup RsslUIntType
 * @{
 */

/** 
 * @brief Rssl Unsigned Integer type 
 */
typedef RsslUInt64 RsslUInt;

/**
 * @}
 */

/** 
 * @addtogroup RsslFloatType
 * @{
 */

/**
 * @brief Rssl 4-byte floating point value
 */
typedef rtrFloat RsslFloat;

/**
 * @}
 */


/** 
 * @addtogroup RsslDoubleType
 * @{
 */

/**
 * @brief Rssl 8-byte floating point value
 */
typedef rtrDouble RsslDouble;

/**
 * @}
 */

/**
 *	@addtogroup DataPrimitive
 *	@{
 */

/**
 * @brief Simple Boolean value
 */
typedef rtrInt8 RsslBool;

/**
 * @brief The Boolean value for True.
 * @see RsslBool
 */
#define RSSL_TRUE 1

/**
 * @brief The Boolean value for False.
 * @see RsslBool
 */
#define RSSL_FALSE 0


/**
 * @brief Field Identifier type 
 * @see RsslFieldList, RsslFieldListField
*/
typedef rtrInt16 RsslFieldId;

/**
 * @}
 */

/** 
 * @addtogroup RsslEnumType
 * @{
 */

/**
 * @brief Enumerated Value type
 */
typedef rtrUInt16 RsslEnum;

/**
 * @}
 */

/**
 *	@addtogroup RSSLRetCodes
 *	@{
 */

/**
 * @brief Rssl Return Code type
 * @see RsslReturnCodes
 */
typedef RsslInt32 RsslRet;

/**
 * @}
 */

/**
 *	@addtogroup RsslBufferStruct
 *	@{
 */

/** 
 * @brief RsslBuffer structure
 *
 * This buffer contains a data member that points
 * to the first byte of char* data and a RsslUInt32
 * length that describes the length of the
 * data buffer.
 *
 * @see RSSL_INIT_BUFFER, rsslClearBuffer, rsslEncodeBuffer, rsslDecodeBuffer
 */
typedef RwfBuffer RsslBuffer;

/**
 * @brief RsslBuffer static initializer
 * @see RsslBuffer, rsslClearBuffer
 */
#define RSSL_INIT_BUFFER {0, 0}


/**
 * @brief Clears an RsslBuffer structure
 * 
 * @param pBuffer Pointer to RsslBuffer to clear
 * @see RsslBuffer, RSSL_INIT_BUFFER
 */
#define rsslClearBuffer(pBuffer) rwfClearBuffer(pBuffer)

/**
 * @}
 */

/**
 * @}
 */


/**
 *	@addtogroup RSSLWFCommon
 *	@{
 */
 
/**
 *	@defgroup LibVersion RSSL Library Version Information Structure
 *	@{
 */

/** 
 * @brief Library Version Information structure to be populated with library version info
 * @see rsslClearLibraryVersionInfo, RSSL_INIT_LIBRARY_VERSION_INFO, rsslQueryDataLibraryVersion, rsslQueryMessagesLibraryVersion, rsslQueryTransportLibraryVersion
 */
typedef struct
{
	char*		productVersion;			/*!< Product Release and Load information */
	char*		internalVersion;		/*!< Internal Node information, useful for raising questions or reporting issues */
	char*		productDate;			/*!< Date library was produced for product release */
} RsslLibraryVersionInfo;

/**
 * @brief Static RsslLibraryVersionInfo initializer
 * @see RsslLibraryVersionInfo, rsslClearLibraryVersionInfo
 **/
#define RSSL_INIT_LIBRARY_VERSION_INFO { 0, 0, 0 }

/**
 * @brief Clears RsslLibraryVersionInfo 
 * @see RsslLibraryVersionInfo, RSSL_INIT_LIBRARY_VERSION_INFO
 */

RTR_C_INLINE void rsslClearLibraryVersionInfo(RsslLibraryVersionInfo *pLibInfo)
{
	pLibInfo->internalVersion = 0;
	pLibInfo->productDate = 0;
	pLibInfo->productVersion = 0;	
}

/**
 * @}
 */
 
/**
 * @}
 */


#ifdef __cplusplus
}
#endif 

#endif

