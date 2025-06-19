/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef RSSL_TEST_FRAMEWORK_H
#define RSSL_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rtr/rsslTypes.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslMemoryBuffer.h"

extern RsslBuffer g_userName;
extern RsslBuffer g_password;
extern RsslBuffer g_clientId;
extern RsslBuffer g_clientSecret;

extern RsslBuffer g_proxyHost;
extern RsslBuffer g_proxyPort;

#ifdef __cplusplus
extern "C" {
#endif

extern const char *funcName;

#ifdef WIN32
#define __func__ __FUNCTION__ 
#endif

#define rssl_test_start() (funcName = __func__)

#define rssl_test_finish() (funcName = NULL)


/* When defined, tests will intentionally dereference a null pointer when they fail.  This makes it easier for certain windows IDEs to
 * get us a stack trace since they can't catch abort. */
//#define TEST_FAILURE_CAUSES_CRASH

extern RsslInt64 totalTestCount;

/* Static initializer of buffer from given string */
#define rssl_init_buffer_from_string(__string) { (RsslUInt32)strlen(__string), __string }

RsslUInt32 _createFlagCombinations(RsslUInt32** destFlags, const RsslUInt32* baseFlags, RsslUInt32 baseFlagsSize, RsslBool skipZero);

/* Clears encode iterator and sets it and pBuf to a pre-allocated block */
void setupEncodeIterator(RsslEncodeIterator *pIter, RsslBuffer *pBuf);

/* Sets decode iterator to pBuf, and sets pMemBuf to a pre-allocated block */
void setupDecodeIterator(RsslDecodeIterator *pIter, RsslBuffer *pBuf, RsslBuffer *pMemBuf);

/* Sets pMemBuf to a pre-allocated block */
void setupMemoryBuffer(RsslBuffer *pMemBuf);

/* Creates a file containing text from the given string.  Used mainly for creating dictionaries to test with. */
void createFileFromString(const char *pFilename, const char *pFileData, RsslInt32 length);

/* Removes file with the given filename */
void deleteFile(const char *pFilename);

#ifdef __cplusplus
};
#endif

#endif
