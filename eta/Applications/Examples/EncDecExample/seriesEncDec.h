
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

