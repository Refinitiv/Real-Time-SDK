#ifndef _testUtil_h_
#define _testUtil_h_

#include "rtr/rsslDataTypeEnums.h"
#include <stdio.h>
#include <stdlib.h>



#ifdef __cplusplus
extern "C" {
#endif

FILE * openBinFile(const char * fileName, const char * mode);
void closeBinFile(FILE * file);
int readMsg(FILE * file, RsslUInt8 *majorVer, RsslUInt8 *minorVer, RsslBuffer * buffer);
int writeMsg(FILE * out, RsslBuffer * buffer, RsslUInt8 majorVer, RsslUInt8 minorVer, RsslUInt16 len);

FILE * openXmlFile(const char * fileName, const char * opentag);
void closeXmlFile(FILE * file, const char * closetag);


#ifdef __cplusplus
}
#endif

#endif
