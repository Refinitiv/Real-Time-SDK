/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __MSG_ENC_DEC_H
#define __MSG_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for a simple RsslRefreshMsg message  */

/* This function encodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_MAP.  For the payload of the message, 
just have it call the map encoding function.  */
RsslRet exampleEncodeRefreshMsgWithMap(RsslEncodeIterator *encIter);

/* This function decodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_MAP.  For the payload of the message, 
just have it call the map decoding function.  */
RsslRet exampleDecodeRefreshMsgWithMap(RsslDecodeIterator *decIter);

/* This function encodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_SERIES.  For the payload of the message, 
just have it call the series encoding function.  */
RsslRet exampleEncodeRefreshMsgWithSeries(RsslEncodeIterator *encIter);

/* This function decodes a a simple RsslRefreshMsg message
that contains an RSSL_DT_SERIES.  For the payload of the message, 
just have it call the series decoding function.  */
RsslRet exampleDecodeRefreshMsgWithSeries(RsslDecodeIterator *decIter);

#ifdef __cplusplus
}
#endif

#endif
