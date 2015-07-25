/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _SET_DEF_FILE_DECODER_H
#define _SET_DEF_FILE_DECODER_H

#define LIBXML_STATIC

#include "libxml/tree.h"
#include "libxml/parser.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslSetData.h"

#include <string.h>

extern RsslFieldSetDefDb globalFieldSetDefDb;

#ifdef __cplusplus
extern "C" {
#endif

RsslBool decodeSetDefFile(char* setDefFilename);

#ifdef __cplusplus
};
#endif

#endif

