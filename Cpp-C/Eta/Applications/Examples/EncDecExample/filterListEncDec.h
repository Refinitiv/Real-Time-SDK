/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __FILTER_LIST_ENC_DEC_H
#define __FILTER_LIST_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslFilterList Container Type */

/* This function encodes a filter list which contains 2 element list and a field list */
RsslRet exampleEncodeFilterList(RsslEncodeIterator *encIter);


/* decodes the filter list that is encoded by exampleEncodeVector */
RsslRet exampleDecodeFilterList(RsslDecodeIterator *decIter);



#ifdef __cplusplus
}
#endif

#endif

