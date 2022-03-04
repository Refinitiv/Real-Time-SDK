/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



#ifndef __ELEMENT_LIST_ENC_DEC_H
#define __ELEMENT_LIST_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslElementList Container Type */

/* this function will encode a basic field list with several primitives embedded in it */
RsslRet exampleEncodeElementList(RsslEncodeIterator *encIter);

/* this function will decode a basic field list with several primitives embedded in it */
RsslRet exampleDecodeElementList(RsslDecodeIterator *decIter);

#ifdef __cplusplus
}
#endif

#endif

