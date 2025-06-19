/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __SERIES_ENC_DEC_H
#define __SERIES_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslSeries Container Type */

/* this function will encode a basic series with several primitives embedded in it */
RsslRet exampleEncodeSeries(RsslEncodeIterator *encIter);

/* this function will decode a basic series with several primitives embedded in it */
RsslRet exampleDecodeSeries(RsslDecodeIterator *decIter);

#ifdef __cplusplus
}
#endif

#endif

