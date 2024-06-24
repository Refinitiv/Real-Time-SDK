/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_MESSAGE_ENCODERS_H
#define __RSSL_MESSAGE_ENCODERS_H

#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"


#ifdef __cplusplus
extern "C" {
#endif


/** 
 * @addtogroup Messages
 * @{
 */


/**
 * @brief Extract header buffer from decoded message structure
 *
 * @param header             Target buffer for header
 * @param pMsg             fully populated RsslMsg
 */
RsslRet rsslExtractHeader(RsslBuffer * header, const RsslMsg * pMsg);



/**
 * @brief Extract buffer for data header and data body from decoded message structure
 *
 * @param data             Target buffer for data
 * @param pMsg             fully populated RsslMsg
 */
RsslRet rsslExtractDataSection(RsslBuffer * data, const RsslMsg * pMsg);


/**
 * @brief Decode encoded base key
 *
 * @param key              pointer to RsslMsgKey structure
 * @param data             pointer to encoded key
 */
RsslRet RTR_FASTCALL rsslDecodeBaseKey(RsslMsgKey * key, const char * data);



//extern RsslRet rsslDecodeMarketfeedMsg( RsslMsg *oMsg, const RsslBuffer *iBuffer );

/**
 * @brief Decode header information from buffer into RsslMsg
 * @returns number of bytes read
 */
RSSL_API RsslRet rsslDecodeMsgHeader(const RsslDecodeIterator * dIter, RsslMsg * msg);

/**
 * @brief Decode data section (extendedHeader and encDataBody) from buffer into RsslMsg.
 *        RsslMsg's header must already be populated.
 * @returns number of bytes read
 */
RSSL_API RsslRet rsslDecodeDataSection(RsslMsg * msg, const RsslBuffer * buffer);

#ifdef __cplusplus
}
#endif


/* End of defgroup MessageEncoders */
/** 
 * @}
 */


/* End of addtogroup Messages */
/** 
 * @}
 */

#endif /*  __RSSL_MESSAGE_ENCODERS_H */

