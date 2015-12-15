

#ifndef __VECTOR_ENC_DEC_H
#define __VECTOR_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslVector Container Type */

/* This function encodes a vector which contains an element list */
RsslRet exampleEncodeVector(RsslEncodeIterator *encIter);


/* decodes the vector that is encoded by exampleEncodeVector() */
RsslRet exampleDecodeVector(RsslDecodeIterator *decIter);



#ifdef __cplusplus
}
#endif

#endif

