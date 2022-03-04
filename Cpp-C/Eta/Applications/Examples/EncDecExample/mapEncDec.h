/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */



#ifndef __MAP_ENC_DEC_H
#define __MAP_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslMap Container Type */

/* This function encodes a map which contains nested field lists */
RsslRet exampleEncodeMap(RsslEncodeIterator *encIter);


/* decodes the map that is encoded by exampleEncodeMap */
RsslRet exampleDecodeMap(RsslDecodeIterator *decIter);



#ifdef __cplusplus
}
#endif

#endif

