

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

