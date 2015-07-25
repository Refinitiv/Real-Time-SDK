

#ifndef __FIELD_LIST_ENC_DEC_H
#define __FIELD_LIST_ENC_DEC_H

/* Include RSSL Data Package header files */
#include "rtr/rsslDataPackage.h" 


#ifdef __cplusplus
extern "C" {
#endif

/* These functions are used by Encoding and Decoding for RsslFieldList Container Type */

/* this function will encode a basic field list with several primitives embedded in it */
RsslRet exampleEncodeFieldList(RsslEncodeIterator *encIter);

/* this function will decode a basic field list with several primitives
   embedded in it */
RsslRet exampleDecodeFieldList(RsslDecodeIterator *decIter);

/* this function returns a preencoded buffer containing an encoded RsslUInt type */
/* assuming pEncUInt is an RsslBuffer with length and data properly populated */
RsslRet getPreEncodedRsslUIntBuffer(RsslBuffer *pEncUInt, RsslUInt uInt);

/* this function returns a preencoded buffer containing an encoded RsslFieldList type */
/* assuming pEncFieldList RsslBuffer contains the pre-encoded payload with data and length populated */
RsslRet getPreEncodedRsslFieldListBuffer(RsslBuffer *pEncUInt);

/* These are user defined FIDs to be used in the example so that we can show types */
/* that are not in the standard dictionary. User defined FIDs are always negative  */
#define FID_INT			-1
#define FID_FLOAT		-2
#define FID_DOUBLE		-3
#define FID_DATETIME	-4
#define FID_QOS			-5
#define FID_STATE		-6
#define FID_BUFFER		-7
#define FID_ENUM		-8


#ifdef __cplusplus
}
#endif

#endif

