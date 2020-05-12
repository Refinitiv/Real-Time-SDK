///*
// *|---------------------------------------------------------------
// *|	Copyright (C) 2019-2020 Refinitiv. All rights reserved.   --
// *|            Duplication or distribution prohibited           --
// *|---------------------------------------------------------------
// */

/* rsslDataTest
 * Provides functionality and performance testing of the RSSL Data structures, both primitives and containers.
 * This test was made for to provide functional testing for the expected public release of RSSL.
 * To do functionality testing, simply run the test.
 * Command line options can be found in the main() function at the bottom.
 * Total time for each test is always provided.
 * TEST is still used as the enumeration base name, but the values have been changed to match RSSL.
 */

#ifdef WIN32
#define _VARIADIC_MAX 10 /* GoogleTest needs this to build properly w/ Visual Studio*/
#endif
#include "gtest/gtest.h"

#include <limits>
#include <malloc.h>
#include <math.h>
#include <stdio.h>

#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rwfNet.h"

#include "rtr/rsslDataPackage.h"

#include "rtr/encoderTools.h"
#include "rtr/decoderTools.h"
#include "rtr/textFileReader.h"
#include "rtr/rsslCharSet.h"
#include "rtr/rsslcnvtab.h"
#include "rtr/rsslRmtes.h"

#include <math.h>

#ifdef WIN32
#define snprintf _snprintf
#include <windows.h>

#ifndef nextafter
#define nextafter _nextafter
#endif

#ifndef nextafterf
#define nextafterf _nextafterf
#endif
#endif

#include "dictionaries.h"

/* Report Mode */

/* Controls whether to test array with both parameters as NULL for blank or with empty buffer passed in for blank */
#define BLANK_ARRAY_BOTH_NULL 

RsslBool g_ToString;

const static char * dictionaryFileName = "RDMFieldDictionary";
const static char * customDictionaryFileName = "RDMFD_CustomFids.txt";
const static char * dictionaryDumpFileName = "RDMFieldDictionary.dump";
const static char * dictionaryDecodeDumpFileName = "RDMFieldDictionary.decode.dump";
const static char * multipartDictionaryDecodeDumpFileName = "RDMFieldDictionary.decode.multipart.dump";
static unsigned int c_RDMFieldDictionary_Fids = 13841; /* Current version (4.20.03_TREP-RT_13.71). Update as new versions come out. */
static unsigned int c_Custom_Fids = 4;

RsslRmtesWorkingSet baseSet;

RsslRmtesWorkingSet standardSet;

RsslRmtesWorkingSet C1G0;
RsslRmtesWorkingSet C2G0;
RsslRmtesWorkingSet C1G1;
RsslRmtesWorkingSet C2G1;
RsslRmtesWorkingSet C1G2;
RsslRmtesWorkingSet C2G2;
RsslRmtesWorkingSet C1G3;
RsslRmtesWorkingSet C2G3;
RsslRmtesWorkingSet JLG3;
RsslRmtesWorkingSet JKG2;
RsslRmtesWorkingSet R2G1;
RsslRmtesWorkingSet R1G1;
RsslRmtesWorkingSet JKG1;
RsslRmtesWorkingSet JLG1;
RsslRmtesWorkingSet R1G0;
RsslRmtesWorkingSet JKG0;
RsslRmtesWorkingSet JLG0;
RsslRmtesWorkingSet K1G0;
RsslRmtesWorkingSet K1G1;
RsslRmtesWorkingSet K1G2;
RsslRmtesWorkingSet K1G3;

void workingSetSetup()
{
	initWorkingSet(&standardSet);

	initWorkingSet(&C1G0);
	C1G0.G0 = const_cast<RsslRmtesCharSet*>(&_rsslChinese1);
	
	initWorkingSet(&C2G0);
	C2G0.G0 = const_cast<RsslRmtesCharSet*>(&_rsslChinese2);
	
	initWorkingSet(&C1G1);
	C1G1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslChinese1);
	
	initWorkingSet(&C2G1);
	C2G1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslChinese2);

	initWorkingSet(&C1G2);
	C1G2.G2 = const_cast<RsslRmtesCharSet*>(&_rsslChinese1);
	
	initWorkingSet(&C2G2);
	C2G2.G2 = const_cast<RsslRmtesCharSet*>(&_rsslChinese2);
	
	initWorkingSet(&C1G3);
	C1G3.G3 = const_cast<RsslRmtesCharSet*>(&_rsslChinese1);
	
	initWorkingSet(&C2G3);
	C2G3.G3 = const_cast<RsslRmtesCharSet*>(&_rsslChinese2);
	
	initWorkingSet(&JLG3);
	JLG3.G3 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseLatin);
	
	initWorkingSet(&JKG2);
	JKG2.G2 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKatakana);

	initWorkingSet(&R2G1);
	R2G1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslReuterBasic2);

	initWorkingSet(&R1G1);
	R1G1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslReuterBasic1);

	initWorkingSet(&JKG1);
	JKG1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKatakana);

	initWorkingSet(&JLG1);
	JLG1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseLatin);

	initWorkingSet(&R1G0);
	R1G0.G0 = const_cast<RsslRmtesCharSet*>(&_rsslReuterBasic1);

	initWorkingSet(&JLG0);
	JLG0.G0 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseLatin);

	initWorkingSet(&K1G0);
	K1G0.G0 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKanji);

	initWorkingSet(&K1G1);
	K1G1.G1 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKanji);

	initWorkingSet(&K1G2);
	K1G2.G2 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKanji);

	initWorkingSet(&K1G3);
	K1G3.G3 = const_cast<RsslRmtesCharSet*>(&_rsslJapaneseKanji);
	
}
	
typedef enum
{
	NoLR = 0,
	G0ToGL = 1,
	G1ToGL = 2,
	G2ToGL = 3,
	G3ToGL = 4,
	G1ToGR = 5,
	G2ToGR = 6,
	G3ToGR = 7
} LRCode;



typedef struct
{
	char* testString;
	int strLen;
	ESCReturnCode expectedRet;
	RsslRmtesWorkingSet* endSet;
	LRCode endLR;
	int returnValue;
} controlTest;

unsigned char nullControlChar[] = 		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char C1G0ControlChar[] = 		{0x1B, 0x24, 0x28, 0x47, 0x00, 0x00, 0x00, 0x00};
unsigned char C2G0ControlChar[] = 		{0x1B, 0x24, 0x28, 0x48, 0x00, 0x00, 0x00, 0x00};
unsigned char C1G1ControlChar[] = 		{0x1B, 0x24, 0x29, 0x47, 0x00, 0x00, 0x00, 0x00};
unsigned char C2G1ControlChar[] = 		{0x1B, 0x24, 0x29, 0x48, 0x00, 0x00, 0x00, 0x00};
unsigned char C1G247ControlChar[] = 	{0x1B, 0x24, 0x2A, 0x47, 0x00, 0x00, 0x00, 0x00};
unsigned char C1G235ControlChar[] = 	{0x1B, 0x24, 0x2A, 0x35, 0x00, 0x00, 0x00, 0x00};
unsigned char C2G2ControlChar[] = 		{0x1B, 0x24, 0x2A, 0x48, 0x00, 0x00, 0x00, 0x00};
unsigned char C1G3ControlChar[] = 		{0x1B, 0x24, 0x2B, 0x47, 0x00, 0x00, 0x00, 0x00};
unsigned char C2G348ControlChar[] = 	{0x1B, 0x24, 0x2B, 0x48, 0x00, 0x00, 0x00, 0x00};
unsigned char C2G336ControlChar[] = 	{0x1B, 0x24, 0x2B, 0x36, 0x00, 0x00, 0x00, 0x00};
unsigned char K1G3ControlChar[] = 		{0x1B, 0x24, 0x2B, 0x34, 0x00, 0x00, 0x00, 0x00};
unsigned char JLG3ControlChar[] = 		{0x1B, 0x2B, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char JKG2ControlChar[] = 		{0x1B, 0x2A, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char R2G1ControlChar[] = 		{0x1B, 0x29, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char R1G1ControlChar[] = 		{0x1B, 0x29, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char JKG1ControlChar[] = 		{0x1B, 0x29, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char JLG1ControlChar[] = 		{0x1B, 0x29, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char UTFControlChar[] =		{0x1B, 0x25, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char K1G0LongControlChar[] = 	{0x1B, 0x26, 0x40, 0x1B, 0x24, 0x42, 0x00, 0x00};
unsigned char K1G1LongControlChar[] = 	{0x1B, 0x26, 0x40, 0x1B, 0x24, 0x29, 0x42, 0x00};
unsigned char K1G2LongControlChar[] = 	{0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2A, 0x42, 0x00};
unsigned char K1G3LongControlChar[] = 	{0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2B, 0x42, 0x00};
unsigned char G1GRControlChar[] =		{0x1B, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G2GLControlChar[] =		{0x1B, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G2GRControlChar[] =		{0x1B, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G3GLControlChar[] =		{0x1B, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G3GRControlChar[] =		{0x1B, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G0GLControlChar[] =		{0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char G1GLControlChar[] =		{0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char EndCharControlChar[] =	{0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* Fail cases */
unsigned char FailureControlChar[] =	{0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define CONTROL_TEST_COUNT 	32

controlTest controlTestArray[CONTROL_TEST_COUNT] = 
{
	{(char*)nullControlChar, 1, ESC_SUCCESS, &standardSet, NoLR, 1},
	{(char*)C1G0ControlChar, 4, ESC_SUCCESS, &C1G0, NoLR, 4},
	{(char*)C2G0ControlChar, 4, ESC_SUCCESS, &C2G0, NoLR, 4},
	{(char*)C1G1ControlChar, 4, ESC_SUCCESS, &C1G1, NoLR, 4},
	{(char*)C2G1ControlChar, 4, ESC_SUCCESS, &C2G1, NoLR, 4},
	{(char*)C1G247ControlChar, 4, ESC_SUCCESS, &C1G2, NoLR, 4},
	{(char*)C1G235ControlChar, 4, ESC_SUCCESS, &C1G2, NoLR, 4},
	{(char*)C2G2ControlChar, 4, ESC_SUCCESS, &C2G2, NoLR, 4},
	{(char*)C1G3ControlChar, 4, ESC_SUCCESS, &C1G3, NoLR, 4},
	{(char*)C2G348ControlChar, 4, ESC_SUCCESS, &C2G3, NoLR, 4},
	{(char*)C2G336ControlChar, 4, ESC_SUCCESS, &C2G3, NoLR, 4},
	{(char*)K1G3ControlChar, 4, ESC_SUCCESS, &K1G3, NoLR, 4},
	{(char*)JLG3ControlChar, 3, ESC_SUCCESS, &JLG3, NoLR, 3},
	{(char*)JKG2ControlChar, 3, ESC_SUCCESS, &JKG2, NoLR, 3},
	{(char*)R2G1ControlChar, 3, ESC_SUCCESS, &R2G1, NoLR, 3},
	{(char*)R1G1ControlChar, 3, ESC_SUCCESS, &R1G1, NoLR, 3},
	{(char*)JKG1ControlChar, 3, ESC_SUCCESS, &JKG1, NoLR, 3},
	{(char*)JLG1ControlChar, 3, ESC_SUCCESS, &JLG1, NoLR, 3},
	{(char*)UTFControlChar, 3, UTF_ENC, &standardSet, NoLR, 3},
	{(char*)K1G0LongControlChar, 6, ESC_SUCCESS, &K1G0, NoLR, 6},
	{(char*)K1G1LongControlChar, 7, ESC_SUCCESS, &K1G1, NoLR, 7},
	{(char*)K1G2LongControlChar, 7, ESC_SUCCESS, &K1G2, NoLR, 7},
	{(char*)K1G3LongControlChar, 7, ESC_SUCCESS, &K1G3, NoLR, 7},
	{(char*)G1GRControlChar, 2, ESC_SUCCESS, &standardSet, G1ToGR, 2},
	{(char*)G2GLControlChar, 2, ESC_SUCCESS, &standardSet, G2ToGL, 2},
	{(char*)G2GRControlChar, 2, ESC_SUCCESS, &standardSet, G2ToGR, 2},
	{(char*)G3GLControlChar, 2, ESC_SUCCESS, &standardSet, G3ToGL, 2},
	{(char*)G3GRControlChar, 2, ESC_SUCCESS, &standardSet, G3ToGR, 2},
	{(char*)G0GLControlChar, 1, ESC_SUCCESS, &standardSet, G0ToGL, 1},
	{(char*)G1GLControlChar, 1, ESC_SUCCESS, &standardSet, G1ToGL, 1},	  // 30
	{(char*)EndCharControlChar, 1, END_CHAR, &standardSet, NoLR, 0},
	{(char*)FailureControlChar, 1, ESC_SUCCESS, &standardSet, NoLR, 0}
};

/* Buffers and iterators */

RsslEncodeIterator encIter;
RsslDecodeIterator decIter;

char buf1[1];
RsslBuffer tbuf1 = { 1, buf1 };	

char buf2[2];
RsslBuffer tbuf2 = { 2, buf2 };

char buf15[15];
RsslBuffer tbuf15 = { 15, buf15 }; 

#define TEST_BIG_BUF_SIZE 16000000
char bufBig[TEST_BIG_BUF_SIZE];
RsslBuffer tbufBig = { TEST_BIG_BUF_SIZE, bufBig };

char charBuf1[100];
char charBuf2[100];

unsigned char inBuf1[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 0x1B, 0x5B, '3', 0x60, 'j', 'k', 'l'};
unsigned char inBuf2[] = {'a', 'b', 'c', 'd', 0x1B, 0x5B, '3', 0x62, 'e', 'f', 'g'};
unsigned char inBuf3[] = {0x1B, 0x5B, '1', '0', 0x60, 'm', 'n', 'o'};
unsigned char inBuf4[] = {'a', 'b', 'c', 0x1B, 0x5B, '2', 0x60, 'd', 'e', 0x1B, 0x5B, '3', 0x62};
/* tab character buffer */
unsigned char inBuf5[] = {'\t', '\t', 'a', 'b', 'c'};


char outBuf[100];

short shortBuf[100];
char utfBuf[100];


/* _allocateFlagCombinations()
 *   Pass in an array of masks(masksBase), this will allocate 'masks' with a block containing all combinations of those flags 
 *     (including all & none).
 */
RsslUInt32 _allocateFlagCombinations(RsslUInt32** dstMasks, const RsslUInt32* srcMasks, RsslUInt32 srcMasksSize, RsslBool skipZero)
{
	RsslUInt32 skip = skipZero? 1 : 0;
	RsslUInt32 dstMasksSize = (2 >> srcMasksSize) - skip; 
	RsslUInt32 srcMasksIter, dstMasksIter;

	*dstMasks = (RsslUInt32*)malloc(dstMasksSize * sizeof(RsslUInt32));

	for (dstMasksIter = skip; dstMasksIter < dstMasksSize + skip; ++dstMasksIter)
	{
		(*dstMasks)[dstMasksIter-skip] = 0;
		for (srcMasksIter = 0; srcMasksIter < srcMasksSize; ++srcMasksIter)
		{
			if ((dstMasksIter >> srcMasksIter) & 0x1) 
				(*dstMasks)[dstMasksIter-skip] |= srcMasks[srcMasksIter];			
		}
	}

	return dstMasksSize;

}

/* FIDs and primitives for field list */
RsslUInt8 fieldListFids[] =
{
	1,
	2,
	3,
	4,
	5,
	6
};

RsslUInt8 fieldListDataTypes[] = 
{
	RSSL_DT_INT,
	RSSL_DT_DOUBLE,
	RSSL_DT_REAL,
	RSSL_DT_DATE,
	RSSL_DT_TIME,
	RSSL_DT_DATETIME
};


RsslInt64 fieldListInt = 0;
RsslDouble fieldListFloat = 0xFF;
RsslReal fieldListReal = {0, 1, 0xFFFFF};
RsslDate fieldListDate = {8, 3, 1892};
RsslTime fieldListTime = { 23, 59, 59, 999, 999, 999 };
RsslDateTime fieldListDateTime = { {8, 3, 1892}, {23, 59, 59, 999, 999, 999} };

#define TEST_FIELD_LIST_MAX_ENTRIES 6 /* Using all primitives makes the top container too big to test nested structure to maximum depth */

void _encodeFieldList(RsslEncodeIterator* pEncIter)
{
	/* construct field list 
	 * This field list will be used as every field list in the structure */
	
	RsslFieldList fieldList;
	RsslFieldEntry entry;
	RsslUInt8 iiEntry;

	rsslClearFieldList(&fieldList);

	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	/* init */
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(pEncIter, &fieldList, 0, 0));

	/* add entries */
	for(iiEntry = 0; iiEntry < TEST_FIELD_LIST_MAX_ENTRIES ; ++iiEntry)
	{
		rsslClearFieldEntry(&entry);
		entry.fieldId = fieldListFids[iiEntry];
		entry.dataType = fieldListDataTypes[iiEntry];
		switch(fieldListDataTypes[iiEntry])
		{
		case RSSL_DT_INT:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListInt)); break;
			case RSSL_DT_DOUBLE:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListFloat)); break;
			case RSSL_DT_REAL:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListReal)); break;
			case RSSL_DT_DATE:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListDate)); break;
			case RSSL_DT_TIME:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListTime)); break;
			case RSSL_DT_DATETIME:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListDateTime)); break;
			default:
				FAIL() << "Error in _encodeFieldList()";
		}

	}


	/* finish encoding */
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(pEncIter, RSSL_TRUE));
}

void _decodeFieldList(RsslDecodeIterator *pDecIter)
{

	RsslInt64 decInt;
	RsslDouble decFloat;
	RsslReal decReal;
	RsslDate decDate;
	RsslTime decTime;
	RsslDateTime decDateTime;

	RsslFieldList container = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry entry;

	RsslUInt8 iiEntry;


	// Setup container
	rsslClearFieldList(&container);
	rsslClearFieldEntry(&entry);

	// Begin container decoding
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(pDecIter, &container, 0));

	ASSERT_TRUE( container.flags == RSSL_FLF_HAS_STANDARD_DATA );

	// Decode entries
	for(iiEntry = 0; iiEntry < TEST_FIELD_LIST_MAX_ENTRIES ; ++iiEntry)
	{
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(pDecIter, &entry));
						
			ASSERT_TRUE(entry.fieldId == fieldListFids[iiEntry] && entry.dataType == RSSL_DT_UNKNOWN);

			switch(fieldListDataTypes[iiEntry])
			{
				case RSSL_DT_INT:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(pDecIter, &decInt)
							&& decInt == fieldListInt);
					break;
				case RSSL_DT_DOUBLE:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(pDecIter, &decFloat)
						&& decFloat == fieldListFloat); /* not rounded inside encoding/decoding, so this should match exactly */
					break;
				case RSSL_DT_REAL:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(pDecIter, &decReal)
							&& decReal.isBlank == 0
							&& decReal.hint == fieldListReal.hint
							&& decReal.value == fieldListReal.value);
					break;
				case RSSL_DT_DATE:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(pDecIter, &decDate)
							&& decDate.day == fieldListDate.day
							&& decDate.month == fieldListDate.month
							&& decDate.year == fieldListDate.year);
					break;
				case RSSL_DT_TIME:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(pDecIter, &decTime)
						&& decTime.hour == fieldListTime.hour
						&& decTime.minute == fieldListTime.minute
						&& decTime.second == fieldListTime.second
						&& decTime.millisecond == fieldListTime.millisecond);
					break;
				case RSSL_DT_DATETIME:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(pDecIter, &decDateTime)
							&& decDateTime.date.day == fieldListDateTime.date.day
							&& decDateTime.date.month == fieldListDateTime.date.month
							&& decDateTime.date.year == fieldListDateTime.date.year
							&& decDateTime.time.hour == fieldListDateTime.time.hour
							&& decDateTime.time.minute == fieldListDateTime.time.minute
							&& decDateTime.time.second == fieldListDateTime.time.second
							&& decDateTime.time.millisecond == fieldListDateTime.time.millisecond);
					break;
				default:
					FAIL() << "Error in _decodeFieldList()";
			}
	}
	ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(pDecIter, &entry));

}

void partialUpdateTest()
{
	RsslBuffer inBuffer;
	RsslRmtesCacheBuffer cacheBuffer;
	int ret;
	
	printf("Partial update caching tests:\n\n");
	
	inBuffer.data = charBuf1;
	
	sprintf(inBuffer.data, "abcdefghijkl");
	
	inBuffer.length = 12;
	
	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 100;
	
	sprintf(outBuf, "abcdefghijkl");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_FALSE); //Partial update check, no update
	
	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //Standard string apply to cache
	ASSERT_TRUE(cacheBuffer.length == 12); //Cache length standard string
	ASSERT_TRUE(memcmp(inBuffer.data, cacheBuffer.data, cacheBuffer.length) == 0); //String compare
		
	memcpy(inBuffer.data, inBuf1, 16*sizeof(char));
	inBuffer.length = 16;
	
	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "abcjklghi");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //partial update check, with update
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update apply to cache
	ASSERT_TRUE(cacheBuffer.length == 9); //Cache length partial update
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare
	
	memcpy(inBuffer.data, inBuf2, 11*sizeof(char));
	inBuffer.length = 11;
	
	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "abcddddefg");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //partial update check, with repeat
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update repeat apply to cache
	ASSERT_TRUE(cacheBuffer.length == 10); //Cache length repeat
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare
	
	memcpy(inBuffer.data, inBuf3, 8*sizeof(char));
	inBuffer.length = 8;
	
	cacheBuffer.data = charBuf2;
	sprintf(cacheBuffer.data, "abcdefghijkl");
	cacheBuffer.length = 12;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "abcdefghijmno");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //partial update check, with existing cache
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update repeat apply to cache
	ASSERT_TRUE(cacheBuffer.length == 13); //Cache length on existing cache
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare

	sprintf(inBuffer.data, "abcdefgh");
	inBuffer.length = 8;
	
	cacheBuffer.data = charBuf2;
	sprintf(cacheBuffer.data, "abcdefghijkl");
	cacheBuffer.length = 12;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "abcdefgh");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_FALSE); //partial update check, no update present
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update repeat apply to cache with cache present
	ASSERT_TRUE(cacheBuffer.length == 8); //Cache length on existing cache
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare
	
	
	memcpy(inBuffer.data, inBuf4, 13*sizeof(char));
	inBuffer.length = 13;
	
	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "abdeeee");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //partial update check, with update
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update repeat apply to cache
	ASSERT_TRUE(cacheBuffer.length == 7); //Cache length on both
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare	
}

void controlParseTest()
{
	RsslRmtesWorkingSet testSet;
	controlTest *curTest;
	int testCount;
	ESCReturnCode retCode;
	int ret;
	char* start;
	char* end;
	
	//printf("Control parse tests:\n\n");
	
	workingSetSetup();
	
	for(testCount = 0; testCount < CONTROL_TEST_COUNT; testCount++)
	{
		//printf("Testcount = %i\n", testCount);
		initWorkingSet(&testSet);
		curTest = &controlTestArray[testCount];
		start = curTest->testString;
		end = curTest->testString + curTest->strLen;
	
		
		ret = controlParse((unsigned char*)start, (unsigned char*)end, &testSet, &retCode);
		
		ASSERT_TRUE(retCode == curTest->expectedRet); //controlParse retcode
		//printf("retCode = %i\n", retCode);
		ASSERT_TRUE(ret == curTest->returnValue); //controlParse ret
		//printf("ret = %i\n", ret);
		
		ASSERT_TRUE(testSet.G0 == curTest->endSet->G0); //G0
		//printf("testSet.G0 = %X, curTest->endSet->G0 = %X\n", testSet.G0, curTest->endSet->G0);
		ASSERT_TRUE(testSet.G1 == curTest->endSet->G1); //G1
		//printf("testSet.G1 = %X, curTest->endSet->G1 = %X\n", testSet.G1, curTest->endSet->G1);
		ASSERT_TRUE(testSet.G2 == curTest->endSet->G2); //G2
		ASSERT_TRUE(testSet.G3 == curTest->endSet->G3); //G3
		
		switch(curTest->endLR)
		{
			case NoLR:
				break;
			case G0ToGL:
				ASSERT_TRUE(testSet.GL == &testSet.G0); //GL->G0
				break;
			case G1ToGL:
				ASSERT_TRUE(testSet.GL == &testSet.G1); //GL->G1
				break;
			case G2ToGL:
				ASSERT_TRUE(testSet.GL == &testSet.G2); //GL->G2
				break;
			case G3ToGL:
				ASSERT_TRUE(testSet.GL == &testSet.G3); //GL->G3
				break;
			case G1ToGR:
				ASSERT_TRUE(testSet.GR == &testSet.G1); //GR->G1
				break;
			case G2ToGR:
				ASSERT_TRUE(testSet.GR == &testSet.G2); //GR->G2
				break;
			case G3ToGR:
				ASSERT_TRUE(testSet.GR == &testSet.G3); //GR->G3
				break;
		}
	}

}

void bufferEdgeCaseTest()
{
	RsslBuffer inBuffer;
	RsslRmtesCacheBuffer cacheBuffer;
	int ret;

	//printf("buffer edge case test:\n\n");

	inBuffer.data = charBuf1;

	sprintf(inBuffer.data, "abcdefghijkl");



	inBuffer.length = 12;

	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 10;

	sprintf(outBuf, "abcdefghijkl");

	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_FALSE); //1Partial update check, no update

	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_BUFFER_TOO_SMALL); //1 buffer too small
	cacheBuffer.allocatedLength = 50;
	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //1 success
	ASSERT_TRUE(cacheBuffer.length == 12); //1 Cache length standard string
	ASSERT_TRUE(memcmp(inBuffer.data, cacheBuffer.data, cacheBuffer.length) == 0); //1 String compare


	inBuffer.length = 12;

	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 10;

	sprintf(outBuf, "abcdefghijkl");

	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_FALSE); //2 Partial update check, no update

	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_BUFFER_TOO_SMALL); //2 buffer too small
	cacheBuffer.allocatedLength = 12;
	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //2 success
	ASSERT_TRUE(cacheBuffer.length == 12); //1 Cache length standard string
	ASSERT_TRUE(memcmp(inBuffer.data, cacheBuffer.data, cacheBuffer.length) == 0); //2 String compare


	memcpy(inBuffer.data, inBuf3, 8*sizeof(char));
	inBuffer.length = 8;
	sprintf(outBuf, "abcdefghijmno");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //2 partial update check, with existing cache
	
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_BUFFER_TOO_SMALL); //2 buffer too small
	cacheBuffer.allocatedLength = 50;
	ASSERT_TRUE(ret = rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //2 success
	ASSERT_TRUE(cacheBuffer.length == 13); //2 Cache length on existing cache
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare


	memcpy(inBuffer.data, inBuf4, 13*sizeof(char));
	inBuffer.length = 13;
	
	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 6;
	sprintf(outBuf, "abdeeee");
	
	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_TRUE); //3 partial update check, with update

	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_BUFFER_TOO_SMALL); //3 buffer too small
	cacheBuffer.allocatedLength = 50;
	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //3 partial update repeat apply to cache

	ASSERT_TRUE(cacheBuffer.length == 7); //3 Cache length on both
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //3 String compare	
}

void deleteCharTest()
{
	RsslBuffer outBuffer;
	RsslU16Buffer shortBuffer;
	RsslRmtesCacheBuffer cacheBuffer;
	int ret;

	//printf("Delete character test\n");

	cacheBuffer.data = charBuf1;
	cacheBuffer.length = 1;
	cacheBuffer.allocatedLength = 100;

	*(cacheBuffer.data) = 0x7F;

	outBuffer.data = charBuf2;
	outBuffer.length = 100;

	ASSERT_TRUE((ret = rsslRMTESToUTF8(&cacheBuffer, &outBuffer)) == RSSL_RET_SUCCESS); //UTF8 delete character parsed

	ASSERT_TRUE(outBuffer.length == 3); //buffer length is correct

	shortBuffer.data = (RsslUInt16*)shortBuf;
	shortBuffer.length = 100;

	ASSERT_TRUE((ret = rsslRMTESToUCS2(&cacheBuffer, &shortBuffer)) == RSSL_RET_SUCCESS); //UTF8 delete character parsed

	ASSERT_TRUE(shortBuffer.length == 1); //length correct
	ASSERT_TRUE(*(shortBuffer.data) == 0xFFFD); //Delete character set
}

unsigned char overflowIn[] = { 'A', 'B', 'C' };

void overflowTest()
{
	RsslBuffer inBuffer;
	RsslRmtesCacheBuffer cacheBuffer;
	RsslBuffer outBuffer;

	//printf("Overflow protection tests\n\n");

	inBuffer.data = (char*)overflowIn;
	inBuffer.length = 3;

	cacheBuffer.allocatedLength = 100;
	cacheBuffer.length = 0;
	cacheBuffer.data = charBuf1;

	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //Apply to cache
	outBuffer.data = charBuf2;
	outBuffer.length = 2;
	memset(charBuf2, 0, 100);

	ASSERT_TRUE(rsslRMTESToUTF8(&cacheBuffer, &outBuffer) == RSSL_RET_BUFFER_TOO_SMALL); //Buffer too small
	ASSERT_TRUE(strcmp("ABC", (const char*)charBuf2) != 0); //Buffer overflow check

	outBuffer.length = 3;
	ASSERT_TRUE(rsslRMTESToUTF8(&cacheBuffer, &outBuffer) == RSSL_RET_SUCCESS); //Buffer set
	ASSERT_TRUE(strcmp("ABC", (const char*)charBuf2) == 0); //Buffer proplery set
}

void controlCharacterParse()
{
	RsslBuffer inBuffer;
	RsslRmtesCacheBuffer cacheBuffer;
	RsslBuffer utfBuffer;

	inBuffer.data = charBuf1;

	//printf("Control character tests\n");

	memcpy(inBuffer.data, inBuf5, 5 * sizeof(char));
	inBuffer.length = 5;

	cacheBuffer.data = charBuf2;
	cacheBuffer.length = 0;
	cacheBuffer.allocatedLength = 100;
	sprintf(outBuf, "\t\tabc");

	utfBuffer.data = utfBuf;
	utfBuffer.length = 100;


	ASSERT_TRUE(rsslHasPartialRMTESUpdate(&inBuffer) == RSSL_FALSE); //control chacaters starting string

	ASSERT_TRUE(rsslRMTESApplyToCache(&inBuffer, &cacheBuffer) == RSSL_RET_SUCCESS); //partial update repeat apply to cache
	ASSERT_TRUE(cacheBuffer.length == 5); //Cache length on both
	ASSERT_TRUE(memcmp(outBuf, cacheBuffer.data, cacheBuffer.length) == 0); //String compare

	ASSERT_TRUE(rsslRMTESToUTF8(&cacheBuffer, &utfBuffer) == RSSL_RET_SUCCESS); //Apply to UTF8

}

TEST(fieldSetDefDictionaryTest,fieldSetDefDictionaryTest)
{
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	char error[255];
	RsslBuffer errorText;
	int currentSid;
	RsslRet ret;

	RsslFieldSetDefDb inputDb = RSSL_INIT_FIELD_LIST_SET_DB;
	RsslFieldSetDefDb outputDb = RSSL_INIT_FIELD_LIST_SET_DB;

	RsslFieldSetDef set16, set255;

	RsslFieldSetDef *globalDefs[256];

	RsslBuffer version = {5, const_cast<char*>("1.1.1")};

	RsslFieldSetDefEntry globalFields[8] = 
	{	
		{1, RSSL_DT_INT},
		{2, RSSL_DT_DOUBLE},
		{3, RSSL_DT_REAL},
		{4, RSSL_DT_DATE},
		{5, RSSL_DT_TIME},
		{6, RSSL_DT_DATETIME},
		{7, RSSL_DT_ARRAY},
		{8, RSSL_DT_UINT}
	};

	RsslFieldSetDefEntry everyType[29] = 
	{	
		{1, RSSL_DT_INT},
		{2, RSSL_DT_DOUBLE},
		{3, RSSL_DT_REAL},
		{4, RSSL_DT_DATE},
		{5, RSSL_DT_TIME},
		{6, RSSL_DT_DATETIME},
		{7, RSSL_DT_ARRAY},
		{8, RSSL_DT_UINT},
		{9, RSSL_DT_INT_1},
		{10, RSSL_DT_INT_2},
		{11, RSSL_DT_INT_4},
		{12, RSSL_DT_INT_8},
		{13, RSSL_DT_UINT_1},
		{14, RSSL_DT_UINT_2},
		{15, RSSL_DT_UINT_4},
		{16, RSSL_DT_UINT_8},
		{17, RSSL_DT_FLOAT_4},
		{18, RSSL_DT_DOUBLE_8},
		{19, RSSL_DT_REAL_4RB},
		{20, RSSL_DT_REAL_8RB},
		{21, RSSL_DT_DATE_4},
		{22, RSSL_DT_TIME_3},
		{23, RSSL_DT_TIME_5},
		{24, RSSL_DT_DATETIME_7},
		{25, RSSL_DT_DATETIME_9},
		{26, RSSL_DT_TIME_7},
		{27, RSSL_DT_TIME_8},
		{28, RSSL_DT_DATETIME_11},
		{29, RSSL_DT_DATETIME_12}
	};

	int testIter;

	set16.count = 8;
	set16.pEntries = globalFields;
	set16.setId = 16;

	set255.count = 25;
	set255.pEntries = everyType;
	set255.setId = 255;

	inputDb.definitions = globalDefs;
	inputDb.isInitialized = RSSL_TRUE;
	inputDb.maxSetId = 255;
	inputDb.info_version = version;

	memset(inputDb.definitions, 0x00, sizeof(globalDefs));

	inputDb.definitions[16] = &set16;
	inputDb.definitions[255] = &set255;
	inputDb.info_DictionaryId = 1;

	errorText.data = error;
	errorText.length = 255;

	outputDb.isInitialized = RSSL_FALSE;


	// Test encode/decode
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorBuffer(&eIter, &tbufBig);
	currentSid = 0;
	ASSERT_TRUE(rsslEncodeFieldSetDefDictionary(&eIter, &inputDb, &currentSid, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);
	tbufBig.length = rsslGetEncodedBufferLength(&eIter);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorBuffer(&dIter, &tbufBig);
	ret = rsslDecodeFieldSetDefDictionary(&dIter, &outputDb, RDM_DICTIONARY_VERBOSE, &errorText); 
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
	//if (ret != RSSL_RET_SUCCESS)
		//printf("\t%s", errorText.data);


	ASSERT_TRUE(outputDb.maxSetId == 255);
	ASSERT_TRUE(outputDb.definitions[16] != NULL);
	ASSERT_TRUE(outputDb.definitions[255] != NULL);

	ASSERT_TRUE(outputDb.definitions[16]->count == 8);
	ASSERT_TRUE(outputDb.definitions[255]->count == 25);
	ASSERT_TRUE(outputDb.info_DictionaryId == inputDb.info_DictionaryId);

	for(testIter = 0; testIter < 8; testIter++)
	{
		ASSERT_TRUE(outputDb.definitions[16]->pEntries[testIter].dataType == globalFields[testIter].dataType);
		ASSERT_TRUE(outputDb.definitions[16]->pEntries[testIter].fieldId == globalFields[testIter].fieldId);
	}

	for(testIter = 0; testIter < 25; testIter++)
	{
		ASSERT_TRUE(outputDb.definitions[255]->pEntries[testIter].dataType == everyType[testIter].dataType);
		ASSERT_TRUE(outputDb.definitions[255]->pEntries[testIter].fieldId == everyType[testIter].fieldId);
	}
		

	tbufBig.length = TEST_BIG_BUF_SIZE;

	rsslDeleteFieldSetDefDb(&outputDb);
}

TEST(elementSetDefDictionaryTest,elementSetDefDictionaryTest)
{
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	char error[255];
	RsslBuffer errorText;
	int currentSid;
	RsslRet ret;

	RsslElementSetDefDb inputDb, outputDb;

	RsslElementSetDef set16, set255;

	RsslElementSetDef *globalDefs[256];

	RsslBuffer version = {5, const_cast<char*>("1.1.1")};

	RsslElementSetDefEntry globalFields[8] = 
	{	
	  {{3, const_cast<char*>("INT")}, RSSL_DT_INT},
	  {{6, const_cast<char*>("DOUBLE")}, RSSL_DT_DOUBLE},
	  {{4, const_cast<char*>("REAL")}, RSSL_DT_REAL},
	  {{4, const_cast<char*>("DATE")}, RSSL_DT_DATE},
	  {{4, const_cast<char*>("TIME")}, RSSL_DT_TIME},
	  {{8, const_cast<char*>("DATETIME")}, RSSL_DT_DATETIME},
	  {{5, const_cast<char*>("ARRAY")}, RSSL_DT_ARRAY},
	  {{4, const_cast<char*>("UINT")}, RSSL_DT_UINT}
	};

	RsslElementSetDefEntry everyType[29] = 
	{	
	  {{3, const_cast<char*>("INT")}, RSSL_DT_INT},
	  {{6, const_cast<char*>("DOUBLE")}, RSSL_DT_DOUBLE},
	  {{4, const_cast<char*>("REAL")}, RSSL_DT_REAL},
	  {{4, const_cast<char*>("DATE")}, RSSL_DT_DATE},
	  {{4, const_cast<char*>("TIME")}, RSSL_DT_TIME},
	  {{8, const_cast<char*>("DATETIME")}, RSSL_DT_DATETIME},
	  {{5, const_cast<char*>("ARRAY")}, RSSL_DT_ARRAY},
	  {{4, const_cast<char*>("UINT")}, RSSL_DT_UINT},
	  {{4, const_cast<char*>("INT1")}, RSSL_DT_INT_1},
	  {{4, const_cast<char*>("INT2")}, RSSL_DT_INT_2},
	  {{4, const_cast<char*>("INT4")}, RSSL_DT_INT_4},
	  {{4, const_cast<char*>("INT8")}, RSSL_DT_INT_8},
	  {{5, const_cast<char*>("UINT1")}, RSSL_DT_UINT_1},
	  {{5, const_cast<char*>("UINT2")}, RSSL_DT_UINT_2},
	  {{5, const_cast<char*>("UINT4")}, RSSL_DT_UINT_4},
	  {{5, const_cast<char*>("UINT8")}, RSSL_DT_UINT_8},
	  {{6, const_cast<char*>("FLOAT4")}, RSSL_DT_FLOAT_4},
	  {{7, const_cast<char*>("DOUBLE8")}, RSSL_DT_DOUBLE_8},
	  {{7, const_cast<char*>("REALRB4")}, RSSL_DT_REAL_4RB},
	  {{7, const_cast<char*>("REALRB8")}, RSSL_DT_REAL_8RB},
	  {{5, const_cast<char*>("DATE4")}, RSSL_DT_DATE_4},
	  {{5, const_cast<char*>("TIME3")}, RSSL_DT_TIME_3},
	  {{5, const_cast<char*>("TIME5")}, RSSL_DT_TIME_5},
	  {{9, const_cast<char*>("DATETIME7")}, RSSL_DT_DATETIME_7},
	  {{9, const_cast<char*>("DATETIME9")}, RSSL_DT_DATETIME_9},
	  {{5, const_cast<char*>("TIME7")}, RSSL_DT_TIME_7},
	  {{5, const_cast<char*>("TIME8")}, RSSL_DT_TIME_8},
	  {{10, const_cast<char*>("DATETIME11")}, RSSL_DT_DATETIME_11},
	  {{10, const_cast<char*>("DATETIME12")}, RSSL_DT_DATETIME_12}
	};

	int testIter;

	set16.count = 8;
	set16.pEntries = globalFields;
	set16.setId = 16;

	set255.count = 25;
	set255.pEntries = everyType;
	set255.setId = 255;

	inputDb.definitions = globalDefs;
	inputDb.isInitialized = RSSL_TRUE;
	inputDb.maxSetId = 255;
	inputDb.info_version = version;

	memset(inputDb.definitions, 0x00, sizeof(globalDefs));

	inputDb.definitions[16] = &set16;
	inputDb.definitions[255] = &set255;
	inputDb.info_DictionaryId = 1;

	errorText.data = error;
	errorText.length = 255;

	outputDb.isInitialized = RSSL_FALSE;

	// Test encode/decode
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorBuffer(&eIter, &tbufBig);
	currentSid = 0;
	ASSERT_TRUE(rsslEncodeElementSetDefDictionary(&eIter, &inputDb, &currentSid, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);
	tbufBig.length = rsslGetEncodedBufferLength(&eIter);

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorBuffer(&dIter, &tbufBig);
	ret = rsslDecodeElementSetDefDictionary(&dIter, &outputDb, RDM_DICTIONARY_VERBOSE, &errorText); 
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
	//if (ret != RSSL_RET_SUCCESS)
		//printf("\t%s", errorText.data);


	ASSERT_TRUE(outputDb.maxSetId == 255);
	ASSERT_TRUE(outputDb.definitions[16] != NULL);
	ASSERT_TRUE(outputDb.definitions[255] != NULL);

	ASSERT_TRUE(outputDb.definitions[16]->count == 8);
	ASSERT_TRUE(outputDb.definitions[255]->count == 25);
	ASSERT_TRUE(outputDb.info_DictionaryId == inputDb.info_DictionaryId);

	for(testIter = 0; testIter < 8; testIter++)
	{
		ASSERT_TRUE(outputDb.definitions[16]->pEntries[testIter].dataType == globalFields[testIter].dataType);
		ASSERT_TRUE(outputDb.definitions[16]->pEntries[testIter].name.length == globalFields[testIter].name.length);
		ASSERT_TRUE(memcmp(outputDb.definitions[16]->pEntries[testIter].name.data, globalFields[testIter].name.data, globalFields[testIter].name.length) == 0);
	}

	for(testIter = 0; testIter < 25; testIter++)
	{
		ASSERT_TRUE(outputDb.definitions[255]->pEntries[testIter].dataType == everyType[testIter].dataType);
		ASSERT_TRUE(outputDb.definitions[255]->pEntries[testIter].name.length == everyType[testIter].name.length);
		ASSERT_TRUE(memcmp(outputDb.definitions[255]->pEntries[testIter].name.data, everyType[testIter].name.data, everyType[testIter].name.length) == 0);
	}
		

	tbufBig.length = TEST_BIG_BUF_SIZE;

	rsslDeleteElementSetDefDb(&outputDb);
}

TEST(fieldDictionaryTest,fieldDictionaryTest)
{
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	RsslDataDictionary dictionary, decodeDictionary; 
	RsslBuffer errorText;
	RsslBuffer smallBuffer;
	RsslBuffer fieldName;
	char enumDisplayChar[32];
	RsslBuffer enumDisplayString;
	RsslRet ret, ret2;
	RsslUInt32 loopCount = 0;
	RsslEnum enumValue;
	const RsslDictionaryEntry *pDictionaryEntry, *pDecodeDictEntry;
	FILE* dictionaryDump;
	int currentFid = 0;
	unsigned int rdmFieldDictionaryFidCount;


	errorText.data = (char*)malloc(255*sizeof(char));
	errorText.length = 255;

	smallBuffer.data = (char*)malloc(1024*sizeof(char));
	smallBuffer.length = 1024;

	// Test loadFieldDictionary
	rsslClearDataDictionary(&dictionary);

	/*** enumType dictionary ***/
	ASSERT_TRUE(rsslLoadEnumTypeDictionary("enumtype.def", &dictionary, &errorText) == RSSL_RET_SUCCESS);
	dictionaryDump = fopen("enumType.dump", "w");
	ASSERT_TRUE(rsslPrintDataDictionary(dictionaryDump, &dictionary) == RSSL_RET_SUCCESS);

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorBuffer(&eIter, &tbufBig);
	ASSERT_TRUE(rsslEncodeEnumTypeDictionary(&eIter, &dictionary, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);

	rsslClearDataDictionary(&decodeDictionary);
	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorBuffer(&dIter, &tbufBig);
	ASSERT_TRUE(rsslDecodeEnumTypeDictionary(&dIter, &decodeDictionary, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);

	dictionaryDump = fopen("enumType.decode.dump", "w");
	ASSERT_TRUE(rsslPrintDataDictionary(dictionaryDump, &decodeDictionary) == RSSL_RET_SUCCESS);

	ASSERT_TRUE(rsslDeleteDataDictionary(&decodeDictionary)== RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDeleteDataDictionary(&dictionary)== RSSL_RET_SUCCESS);

	/*** Field Dictionary ***/
	ret = rsslLoadFieldDictionary( dictionaryFileName, &dictionary, &errorText );
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s", errorText.data);
    	free(errorText.data);
    	free(smallBuffer.data);
		return; /* This is usually because the file isn't found. May as well stop here. */
	}

	//printf("    Info: Loaded dictionary contains %d fields.", dictionary.numberOfEntries);

	rdmFieldDictionaryFidCount = dictionary.numberOfEntries;

	/* Add custom fids to the dictionary */
	ret = rsslLoadFieldDictionary( customDictionaryFileName, &dictionary, &errorText );
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s", errorText.data);
    	free(errorText.data);
    	free(smallBuffer.data);
		return; /* This is usually because the file isn't found. May as well stop here. */
	}
	
	/* Custom dict adds 4 Fids */
	ASSERT_TRUE(dictionary.numberOfEntries == rdmFieldDictionaryFidCount + c_Custom_Fids);

	// Test rsslPrintFieldDictionary
	// Save a dump of the dict info.
	dictionaryDump = fopen(dictionaryDumpFileName, "w");
	if (dictionaryDump)
	{
		ASSERT_TRUE((ret = rsslPrintDataDictionary( dictionaryDump, &dictionary)) == RSSL_RET_SUCCESS);
	}
	else 
		FAIL() << "Print Field Dictionary(couldn't get file handle)";

	// Test encode/decode
	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorBuffer(&eIter, &tbufBig);
	currentFid = dictionary.minFid;
	ASSERT_TRUE(rsslEncodeFieldDictionary(&eIter, &dictionary, &currentFid, RDM_DICTIONARY_VERBOSE, &errorText) == RSSL_RET_SUCCESS);
	tbufBig.length = rsslGetEncodedBufferLength(&eIter);

	rsslClearDecodeIterator(&dIter);
	rsslClearDataDictionary(&decodeDictionary);
	rsslSetDecodeIteratorBuffer(&dIter, &tbufBig);
	ret = rsslDecodeFieldDictionary(&dIter, &decodeDictionary, RDM_DICTIONARY_VERBOSE, &errorText); 
	ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
	//if (ret != RSSL_RET_SUCCESS)
		//printf("\t%s", errorText.data);

	ASSERT_TRUE(decodeDictionary.numberOfEntries == rdmFieldDictionaryFidCount + c_Custom_Fids);

	/* Did all fields make it across? */
	ASSERT_TRUE(decodeDictionary.info_DictionaryId == dictionary.info_DictionaryId);
	ASSERT_TRUE(rsslBufferIsEqual(&decodeDictionary.infoField_Version, &dictionary.infoField_Version));

	ASSERT_TRUE(rsslBufferIsEqual(&decodeDictionary.infoEnum_Filename, &dictionary.infoEnum_Filename));
	ASSERT_TRUE(rsslBufferIsEqual(&decodeDictionary.infoEnum_Desc, &dictionary.infoEnum_Desc));
	ASSERT_TRUE(rsslBufferIsEqual(&decodeDictionary.infoEnum_Date, &dictionary.infoEnum_Date));

	currentFid = RSSL_MIN_FID;
	do
	{
		pDictionaryEntry = dictionary.entriesArray[currentFid];
		pDecodeDictEntry = decodeDictionary.entriesArray[currentFid];
		if (!pDictionaryEntry)
		{
			ASSERT_TRUE(!pDecodeDictEntry);
		}
		else
		{
			
			ASSERT_TRUE(rsslBufferIsEqual(&pDecodeDictEntry->acronym, &pDictionaryEntry->acronym));
			ASSERT_TRUE(rsslBufferIsEqual(&pDecodeDictEntry->ddeAcronym, &pDictionaryEntry->ddeAcronym));
			ASSERT_TRUE(pDecodeDictEntry->enumLength == pDictionaryEntry->enumLength);
			ASSERT_TRUE(pDecodeDictEntry->fid == pDictionaryEntry->fid);
			ASSERT_TRUE(pDecodeDictEntry->fieldType == pDictionaryEntry->fieldType);
			ASSERT_TRUE(pDecodeDictEntry->length == pDictionaryEntry->length);
			ASSERT_TRUE(pDecodeDictEntry->rippleToField == pDictionaryEntry->rippleToField);
			ASSERT_TRUE(pDecodeDictEntry->rwfLength == pDictionaryEntry->rwfLength);
		}
		++currentFid;
	} while (currentFid != RSSL_MAX_FID);

	dictionaryDump = fopen(dictionaryDecodeDumpFileName, "w");
	if (dictionaryDump)
	{
		ASSERT_TRUE((ret = rsslPrintDataDictionary( dictionaryDump, &decodeDictionary)) == RSSL_RET_SUCCESS);
	}
	else 
		FAIL() << "Print Field Dictionary(couldn't get file handle)";

	// Test sending dictionary in multiple parts
	rsslDeleteDataDictionary(&decodeDictionary);
	currentFid = dictionary.minFid;
	do
	{
		++loopCount;
    	rsslClearEncodeIterator(&eIter);
    	rsslSetEncodeIteratorBuffer(&eIter, &smallBuffer);
		ret = rsslEncodeFieldDictionary(&eIter, &dictionary, &currentFid, RDM_DICTIONARY_VERBOSE, &errorText);

		ASSERT_TRUE(ret == RSSL_RET_DICT_PART_ENCODED || ret == RSSL_RET_SUCCESS );
		if (ret != RSSL_RET_DICT_PART_ENCODED && ret != RSSL_RET_SUCCESS)
		{
    		//printf("\t%s", errorText.data);
        	free(errorText.data);
        	free(smallBuffer.data);
			return;
		}

		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorBuffer(&dIter, &smallBuffer);
		ASSERT_TRUE((ret2 = rsslDecodeFieldDictionary(&dIter, &decodeDictionary, RDM_DICTIONARY_VERBOSE, &errorText)) == RSSL_RET_SUCCESS);
		if (ret2 != RSSL_RET_SUCCESS)
		{
    		//printf("\t%s", errorText.data);
        	free(errorText.data);
        	free(smallBuffer.data);
			return;
		}
	}
	while (ret != RSSL_RET_SUCCESS);

	//printf("\tInfo: Fragmented Dictionary test sent dictionary in %u parts.", loopCount);
	ASSERT_TRUE(decodeDictionary.numberOfEntries == rdmFieldDictionaryFidCount + c_Custom_Fids);

	/* Did all fields make it across? */
	ASSERT_TRUE(decodeDictionary.info_DictionaryId == dictionary.info_DictionaryId);
	ASSERT_TRUE(decodeDictionary.infoField_Version.length == dictionary.infoField_Version.length
		&& 0 == strcmp(decodeDictionary.infoField_Version.data, dictionary.infoField_Version.data));
	currentFid = RSSL_MIN_FID;
	do
	{
		pDictionaryEntry = dictionary.entriesArray[currentFid];
		pDecodeDictEntry = decodeDictionary.entriesArray[currentFid];
		if (!pDictionaryEntry)
		{
			ASSERT_TRUE(!pDecodeDictEntry);
		}
		else
		{
			
			ASSERT_TRUE(rsslBufferIsEqual(&pDecodeDictEntry->acronym, &pDictionaryEntry->acronym));
			ASSERT_TRUE(rsslBufferIsEqual(&pDecodeDictEntry->ddeAcronym, &pDictionaryEntry->ddeAcronym));
			ASSERT_TRUE(pDecodeDictEntry->enumLength == pDictionaryEntry->enumLength);
			ASSERT_TRUE(pDecodeDictEntry->fid == pDictionaryEntry->fid);
			ASSERT_TRUE(pDecodeDictEntry->fieldType == pDictionaryEntry->fieldType);
			ASSERT_TRUE(pDecodeDictEntry->length == pDictionaryEntry->length);
			ASSERT_TRUE(pDecodeDictEntry->rippleToField == pDictionaryEntry->rippleToField);
			ASSERT_TRUE(pDecodeDictEntry->rwfLength == pDictionaryEntry->rwfLength);
		}
		++currentFid;
	} while (currentFid != RSSL_MAX_FID);
	


	dictionaryDump = fopen(multipartDictionaryDecodeDumpFileName, "w");
	if (dictionaryDump)
	{
		ASSERT_TRUE((ret = rsslPrintDataDictionary( dictionaryDump, &decodeDictionary)) == RSSL_RET_SUCCESS);
	}
	else 
		FAIL() << "Print Field Dictionary(couldn't get file handle)";

	// Test looking up fields by name.
	fieldName.data = const_cast<char*>("BID");
	fieldName.length = 3;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pDictionaryEntry->acronym));
	ASSERT_TRUE(pDictionaryEntry->fid == 22);

	fieldName.data = const_cast<char*>("ASK-"); /* Make sure we use the passed-in fieldName.length. Don't rely on a null-terminator being present. */
	fieldName.length = 3;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pDictionaryEntry->acronym));
	ASSERT_TRUE(pDictionaryEntry->fid == 25);

	fieldName.data = const_cast<char*>("QUOTIM_MS");
	fieldName.length = 9;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pDictionaryEntry->acronym));
	ASSERT_TRUE(pDictionaryEntry->fid == 3855);

	fieldName.data = const_cast<char*>("404_FIELD_NOT_FOUND");
	fieldName.length = 19;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) == NULL);

	ASSERT_TRUE(rsslLoadEnumTypeDictionary("enumtype.def", &dictionary, &errorText) == RSSL_RET_SUCCESS);

	// Test looking up enumerated values by display string for RDN_EXCHID.

	fieldName.data = const_cast<char*>("RDN_EXCHID");
	fieldName.length = 10;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pDictionaryEntry->acronym));

	// Find NYS (2).
	enumDisplayString.data = const_cast<char*>("NYS");
	enumDisplayString.length = 3;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 2);

	// Find TOR (10).
	enumDisplayString.data = const_cast<char*>("TOR");
	enumDisplayString.length = 3;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 10);

	// Find VNF (should fail).
	enumDisplayString.data = const_cast<char*>("VNF");
	enumDisplayString.length = 3;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_FAILURE);

	// Test looking up enumerated values by display string for PRCTICK_1.

	fieldName.data = const_cast<char*>("PRCTCK_1");
	fieldName.length = 8;
	ASSERT_TRUE((pDictionaryEntry = rsslDictionaryGetEntryByFieldName(&dictionary, &fieldName)) != NULL);

	// Find uptick.
	enumDisplayChar[0] = (char)0xde;
	enumDisplayString.data = enumDisplayChar;
	enumDisplayString.length = 1;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 1);

	// Find downtick.
	enumDisplayChar[0] = (char)0xfe;
	enumDisplayString.data = enumDisplayChar;
	enumDisplayString.length = 1;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 2);

	// Find 0xff (should fail).
	enumDisplayChar[0] = (char)0xff;
	enumDisplayString.data = enumDisplayChar;
	enumDisplayString.length = 1;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_FAILURE);

	// Test duplicate values (" " is used for both the "no tick" and "unchanged" values)
	enumDisplayString.data = const_cast<char*>(" ");
	enumDisplayString.length = 1;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pDictionaryEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_DICT_DUPLICATE_ENUM_VALUE);

	// Test rsslDeleteFieldDictionary
	ASSERT_TRUE((ret = rsslDeleteDataDictionary(&dictionary)) == RSSL_RET_SUCCESS);
	ASSERT_TRUE((ret = rsslDeleteDataDictionary(&decodeDictionary)) == RSSL_RET_SUCCESS);
	
	free(errorText.data);
	free(smallBuffer.data);

	tbufBig.length = TEST_BIG_BUF_SIZE;
}

TEST(dataDictionaryLinkTest,dataDictionaryLinkTest)
{
	RsslDataDictionary oldDictionary, newDictionary;
	char errorTextChar[255];
	RsslBuffer errorText = { 255, errorTextChar };
	RsslRet ret;
	int currentFid;
	RsslBuffer fieldName;
	RsslBuffer enumDisplayString;
	RsslEnum enumValue;

	RsslDictionaryEntry *pOldEntry, *pNewEntry;

	//printf("Data Dictionary Link Tests:\n");
	
	//startTimer();

	rsslClearDataDictionary(&oldDictionary);
	rsslClearDataDictionary(&newDictionary);
	
	/* Load old dictionary. */
	ASSERT_TRUE((ret = rsslLoadFieldDictionary( "RDMFieldDictionary", &oldDictionary, &errorText )) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	ASSERT_TRUE((ret = rsslLoadEnumTypeDictionary( "enumtype.def", &oldDictionary, &errorText )) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	/* Load new dictionary. */
	ASSERT_TRUE((ret = rsslLoadFieldDictionary( "RDMFieldDictionary", &newDictionary, &errorText )) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	ASSERT_TRUE((ret = rsslLoadEnumTypeDictionary( "enumtype.def", &newDictionary, &errorText )) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	/* Double-check that entries match. */
	for (currentFid = RSSL_MIN_FID; currentFid <= RSSL_MAX_FID; ++currentFid)
	{
		pOldEntry = oldDictionary.entriesArray[currentFid];
		pNewEntry = newDictionary.entriesArray[currentFid];
		if (!pOldEntry)
		{
			ASSERT_TRUE(!pNewEntry);
		}
		else
		{
			ASSERT_TRUE(pOldEntry != pNewEntry); /* Pointers shouldn't match, of course. */
			
			ASSERT_TRUE(rsslBufferIsEqual(&pNewEntry->acronym, &pOldEntry->acronym));
			ASSERT_TRUE(rsslBufferIsEqual(&pNewEntry->ddeAcronym, &pOldEntry->ddeAcronym));
			ASSERT_TRUE(pNewEntry->enumLength == pOldEntry->enumLength);
			ASSERT_TRUE(pNewEntry->fid == pOldEntry->fid);
			ASSERT_TRUE(pNewEntry->fieldType == pOldEntry->fieldType);
			ASSERT_TRUE(pNewEntry->length == pOldEntry->length);
			ASSERT_TRUE(pNewEntry->rippleToField == pOldEntry->rippleToField);
			ASSERT_TRUE(pNewEntry->rwfLength == pOldEntry->rwfLength);
		}
	}

	/* Add custom fields to the new dictionary. */
	ASSERT_TRUE((ret = rsslLoadFieldDictionary( "RDMFD_CustomFids.txt", &newDictionary, &errorText )) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	/* Double-check that existing entries still match. */
	for (currentFid = RSSL_MIN_FID; currentFid <= RSSL_MAX_FID; ++currentFid)
	{
		pOldEntry = oldDictionary.entriesArray[currentFid];
		pNewEntry = newDictionary.entriesArray[currentFid];

		if (pOldEntry)
		{
			ASSERT_TRUE(pOldEntry != pNewEntry); /* Pointers shouldn't match, of course. */
			ASSERT_TRUE(rsslBufferIsEqual(&pNewEntry->acronym, &pOldEntry->acronym));
			ASSERT_TRUE(rsslBufferIsEqual(&pNewEntry->ddeAcronym, &pOldEntry->ddeAcronym));
			ASSERT_TRUE(pNewEntry->enumLength == pOldEntry->enumLength);
			ASSERT_TRUE(pNewEntry->fid == pOldEntry->fid);
			ASSERT_TRUE(pNewEntry->fieldType == pOldEntry->fieldType);
			ASSERT_TRUE(pNewEntry->length == pOldEntry->length);
			ASSERT_TRUE(pNewEntry->rippleToField == pOldEntry->rippleToField);
			ASSERT_TRUE(pNewEntry->rwfLength == pOldEntry->rwfLength);
		}
	}

	/* Link the dictionaries together. */
	ASSERT_TRUE((ret = rsslLinkDataDictionary(&newDictionary, &oldDictionary, &errorText)) == RSSL_RET_SUCCESS);
	if (ret != RSSL_RET_SUCCESS)
	{
		//printf("\t%s\n", errorText.data);
		ASSERT_TRUE(false); 
	}

	/* Now the actual pointers in the entry arrays should match. */
	for (currentFid = RSSL_MIN_FID; currentFid <= RSSL_MAX_FID; ++currentFid)
	{
		pOldEntry = oldDictionary.entriesArray[currentFid];
		pNewEntry = newDictionary.entriesArray[currentFid];

		if (pOldEntry)
			ASSERT_TRUE(pOldEntry == pNewEntry); 
	}

	/* New dictionary should have one or more of entries that old dictionary doesn't. */
	ASSERT_TRUE(newDictionary.entriesArray[-9004] != NULL);
	ASSERT_TRUE(oldDictionary.entriesArray[-9004] == NULL);

	/* Make sure Field-by-name lookup still works for both. */
	fieldName.data = const_cast<char*>("RDN_EXCHID");
	fieldName.length = 10;
	ASSERT_TRUE((pNewEntry = rsslDictionaryGetEntryByFieldName(&newDictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pNewEntry->acronym));

	ASSERT_TRUE((pOldEntry = rsslDictionaryGetEntryByFieldName(&oldDictionary, &fieldName)) != NULL);
	ASSERT_TRUE(pNewEntry == pOldEntry); /* Should get same entry pointer. */

	/* Make sure enum lookup still works. */
	enumDisplayString.data = const_cast<char*>("NYS");
	enumDisplayString.length = 3;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pNewEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 2);

	/* Entry pointer should match old dictionary. */
	ASSERT_TRUE(rsslDictionaryGetEntryByFieldName(&oldDictionary, &fieldName) == pNewEntry);

	/* Delete the old dictionary. */
	ASSERT_TRUE(rsslDeleteDataDictionary(&oldDictionary) == RSSL_RET_SUCCESS);

	/* Does the new dictionary still work? */
	fieldName.data = const_cast<char*>("RDN_EXCHID");
	fieldName.length = 10;
	ASSERT_TRUE((pNewEntry = rsslDictionaryGetEntryByFieldName(&newDictionary, &fieldName)) != NULL);
	ASSERT_TRUE(rsslBufferIsEqual(&fieldName, &pNewEntry->acronym));
	ASSERT_TRUE(pNewEntry->fid == 4);

	enumDisplayString.data = const_cast<char*>("NYS");
	enumDisplayString.length = 3;
	ASSERT_TRUE(rsslDictionaryEntryGetEnumValueByDisplayString(pNewEntry, &enumDisplayString, &enumValue, &errorText)
			== RSSL_RET_SUCCESS);
	ASSERT_TRUE(enumValue == 2);

	ASSERT_TRUE(rsslDeleteDataDictionary(&newDictionary) == RSSL_RET_SUCCESS);

	//endTimerAndPrint();
	//printf("\n");
}

/* More extensive testing of dictionary loading, encoding */
TEST(dataDictionaryTests,dataDictionaryTests)
{
	int i, j, k;
	RsslRet ret;
	RsslDataDictionary dict;
	RsslBuffer errorText = {255, (char*)alloca(255)};

	char *string1, *string2, *string3;

	/*** Loading tests ***/

	/* Error cases -- this should be run through a memory leak checker when convenient. */
	for (i = 0; i < dictErrorCases_length; ++i)
	{
		_createTmpFile(dictErrorCases[i].dictData.data, dictErrorCases[i].dictData.length);
		rsslClearDataDictionary(&dict);
		ret = rsslLoadEnumTypeDictionary(rsslTmpFile, &dict, &errorText);
		ASSERT_TRUE(ret == RSSL_RET_FAILURE);
		//if (ret == RSSL_RET_FAILURE)
			//printf(	"	Error case: %s"
      //  			"	    result: \"%.*s\"", dictErrorCases[i].errorText, errorText.length, errorText.data);
	}

	rsslClearDataDictionary(&dict);
	for (i = 0; i < rsslTestEnumTables_length; ++i)
	{
		_createTmpFile(rsslTestEnumTables[i].dictData.data, rsslTestEnumTables[i].dictData.length);
		ret = rsslLoadEnumTypeDictionary(rsslTmpFile, &dict, &errorText);
		ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
		ASSERT_TRUE(dict.enumTableCount == rsslTestEnumTables[i].length);

		for(j = 0; j < dict.enumTableCount; ++j)
		{
			int enumCount = 0;
			ASSERT_TRUE(dict.enumTables[j] && dict.enumTables[j]->fidReferenceCount == rsslTestEnumTables[i].table[j].expectedFidCount);

			for(k = 0; k <= dict.enumTables[j]->maxValue; ++k)
				if (dict.enumTables[j]->enumTypes[k]) ++enumCount;

			ASSERT_TRUE(enumCount == rsslTestEnumTables[i].table[j].expectedEnumCount);
		}

		ASSERT_TRUE(rsslDeleteDataDictionary(&dict) == RSSL_RET_SUCCESS);
	}

	_deleteTestFile();

	/** Test field/enum dictionary entries of varying (and large) string lengths. */
	string1 = (char*)malloc(8192);
	string2 = (char*)malloc(8192); 
	string3 = (char*)malloc(8192);
	memset(string1, 'A', 8192);
	memset(string2, 'B', 8192);
	memset(string3, 'C', 8192);

	for (i = 1; i <= 8192; i *= 2)
	{
		for (j = 1; j <= 8192; j *= 2)
		{
			for (k = 1; k <= 8192; k *= 2)
			{
				RsslDataDictionary dataDictionary;
				RsslBuffer errorText = { 255, (char*)alloca(255) };
				RsslBuffer tmpBuffer;
				RsslDictionaryEntry *tmpEntry;
				RsslEnumTypeTable *tmpTable;
				FILE *dictionaryFile;

				RsslBuffer fd_fileTag = { 7, const_cast<char*>("RWF.DAT") };
				RsslBuffer fd_descTag = { 19, const_cast<char*>("RDF-D RWF field set") };
				RsslBuffer fd_versionTag = { 7, const_cast<char*>("4.20.15") };
				RsslBuffer fd_buildTag = { 3, const_cast<char*>("001") };
				RsslBuffer fd_dateTag = { 11, const_cast<char*>("14-Jan-2015") };

				RsslBuffer et_fileTag = { 12, const_cast<char*>("ENUMTYPE.001") };
				RsslBuffer et_descTag = { 34, const_cast<char*>("IDN Marketstream enumerated tables") };
				RsslBuffer et_rtVersionTag = { 7, const_cast<char*>("4.20.15") };
				RsslBuffer et_dtVersionTag =  { 5, const_cast<char*>("15.11") };
				RsslBuffer et_dateTag = { 11, const_cast<char*>("14-Jan-2015") };



				ASSERT_TRUE((dictionaryFile = fopen("tmpFile.txt", "w")) != NULL);

				/* Field dictionary tags */
				fprintf(dictionaryFile,
						"!tag Filename  %s\n"
						"!tag Desc      %s\n"
						"!tag Type      1\n"
						"!tag Version   %s\n"
						"!tag Build     %s\n"   
						"!tag Date      %s\n",
							fd_fileTag.data,
							fd_descTag.data,
							fd_versionTag.data,
							fd_buildTag.data,
							fd_dateTag.data);



				/* First entry */
				fprintf(dictionaryFile,
						/* acronym, dde acronyum, ripples-to, mf type, length, rwf type, rwf len */
						"%.*s \"%.*s\" 1 NULL INTEGER 5 UINT64 2\n",
						i, string1, i, string1);

				/* Second entry */
				fprintf(dictionaryFile,
						/* acronym, dde acronyum, ripples-to, mf type, length, rwf type, rwf len */
						"%.*s \"%.*s\" 2 NULL INTEGER 5 INT64 2\n",
						j, string2, j, string2);

				/* Third entry */
				fprintf(dictionaryFile,
						/* acronym, dde acronyum, ripples-to, mf type, length, rwf type, rwf len */
						"%.*s \"%.*s\" 3 NULL INTEGER 5 UINT64 2\n",
						k, string3, k, string3);

				fclose(dictionaryFile);

				/* Load dictioanries */
				rsslClearDataDictionary(&dataDictionary);
				ASSERT_TRUE(rsslLoadFieldDictionary("tmpFile.txt", &dataDictionary, &errorText) == RSSL_RET_SUCCESS);

				/*** Now load enum table ***/
				ASSERT_TRUE((dictionaryFile = fopen("tmpFile.txt", "w")) != NULL);

				/* Enum dictionary tags */
				fprintf(dictionaryFile,
					"!tag Filename    %s\n"
					"!tag Desc        %s\n"
					"!tag RT_Version  %s\n"
					"!tag Build_RDMD  4.20.15\n"
					"!tag DT_Version  %s\n"
					"!tag Date        %s\n",
					et_fileTag.data,
					et_descTag.data,
					et_rtVersionTag.data,
					et_dtVersionTag.data,
					et_dateTag.data);

				/* Add enum tables to first and second fields */
				fprintf(dictionaryFile, 
						/* Fid reference */
						"%.*s 1\n"
						/* Value, Display, Meaning */
						"0 \"%.*s\" %.*s\n"
						"1 \"%.*s\" %.*s\n"
						"2 \"%.*s\" %.*s\n",
						i, string1,
						i, string1, i, string1,
						j, string2, j, string2,
						k, string3, k, string3);

				fprintf(dictionaryFile, 
						/* Fid reference */
						"%.*s 2\n"
						/* Value, Display, Meaning */
						"0 \"%.*s\" %.*s\n"
						"1 \"%.*s\" %.*s\n"
						"2 \"%.*s\" %.*s\n",
						j, string2, 
						k, string3, k, string3,
						j, string2, j, string2,
						i, string1, i, string1);

				fclose(dictionaryFile);

				ASSERT_TRUE(rsslLoadEnumTypeDictionary("tmpFile.txt", &dataDictionary, &errorText) == RSSL_RET_SUCCESS);

				/* Check field dictionary tags */
				ASSERT_TRUE(rsslBufferIsEqual(&fd_fileTag, &dataDictionary.infoField_Filename));
				ASSERT_TRUE(rsslBufferIsEqual(&fd_descTag, &dataDictionary.infoField_Desc));
				ASSERT_TRUE(rsslBufferIsEqual(&fd_versionTag, &dataDictionary.infoField_Version));
				ASSERT_TRUE(rsslBufferIsEqual(&fd_buildTag, &dataDictionary.infoField_Build));
				ASSERT_TRUE(rsslBufferIsEqual(&fd_dateTag, &dataDictionary.infoField_Date));

				/* Check enum dictionary tags */
				ASSERT_TRUE(rsslBufferIsEqual(&et_fileTag, &dataDictionary.infoEnum_Filename));
				ASSERT_TRUE(rsslBufferIsEqual(&et_descTag, &dataDictionary.infoEnum_Desc));
				ASSERT_TRUE(rsslBufferIsEqual(&et_rtVersionTag, &dataDictionary.infoEnum_RT_Version));
				ASSERT_TRUE(rsslBufferIsEqual(&et_dtVersionTag, &dataDictionary.infoEnum_DT_Version));
				ASSERT_TRUE(rsslBufferIsEqual(&et_dateTag, &dataDictionary.infoEnum_Date));

				/* Test first field entry */
				tmpEntry = dataDictionary.entriesArray[1];
				tmpBuffer.data = string1;
				tmpBuffer.length = i;
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->acronym));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->ddeAcronym));
				ASSERT_TRUE(tmpEntry->fid == 1);
				ASSERT_TRUE(tmpEntry->rippleToField == 0);
				ASSERT_TRUE(tmpEntry->fieldType == RSSL_MFEED_INTEGER);
				ASSERT_TRUE(tmpEntry->length == 5);
				ASSERT_TRUE(tmpEntry->enumLength == 0);
				ASSERT_TRUE(tmpEntry->rwfType == RSSL_DT_UINT);
				ASSERT_TRUE(tmpEntry->rwfLength == 2);
				ASSERT_TRUE(tmpEntry->pEnumTypeTable != NULL);

				/* Test first field entry's enum table */
				tmpTable = tmpEntry->pEnumTypeTable;
				ASSERT_TRUE(tmpTable->maxValue = 2);

				ASSERT_TRUE(tmpTable->enumTypes[0] != NULL);
				tmpBuffer.data = string1;
				tmpBuffer.length = i;
				ASSERT_TRUE(tmpTable->enumTypes[0]->value == 0);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[0]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[0]->meaning));
				ASSERT_TRUE(tmpTable->enumTypes[1] != NULL);
				tmpBuffer.data = string2;
				tmpBuffer.length = j;
				ASSERT_TRUE(tmpTable->enumTypes[1]->value == 1);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[1]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[1]->meaning));
				ASSERT_TRUE(tmpTable->enumTypes[1] != NULL);
				tmpBuffer.data = string3;
				tmpBuffer.length = k;
				ASSERT_TRUE(tmpTable->enumTypes[2]->value == 2);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[2]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[2]->meaning));

				/* Test second entry */
				tmpEntry = dataDictionary.entriesArray[2];
				tmpBuffer.data = string2;
				tmpBuffer.length = j;
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->acronym));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->ddeAcronym));
				ASSERT_TRUE(tmpEntry->fid == 2);
				ASSERT_TRUE(tmpEntry->rippleToField == 0);
				ASSERT_TRUE(tmpEntry->fieldType == RSSL_MFEED_INTEGER);
				ASSERT_TRUE(tmpEntry->length == 5);
				ASSERT_TRUE(tmpEntry->enumLength == 0);
				ASSERT_TRUE(tmpEntry->rwfType == RSSL_DT_INT);
				ASSERT_TRUE(tmpEntry->rwfLength == 2);
				ASSERT_TRUE(tmpEntry->pEnumTypeTable != NULL);

				/* Test first field entry's enum table */
				tmpTable = tmpEntry->pEnumTypeTable;
				ASSERT_TRUE(tmpTable->maxValue = 2);

				ASSERT_TRUE(tmpTable->enumTypes[0] != NULL);
				tmpBuffer.data = string3;
				tmpBuffer.length = k;
				ASSERT_TRUE(tmpTable->enumTypes[0]->value == 0);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[0]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[0]->meaning));
				ASSERT_TRUE(tmpTable->enumTypes[1] != NULL);
				tmpBuffer.data = string2;
				tmpBuffer.length = j;
				ASSERT_TRUE(tmpTable->enumTypes[1]->value == 1);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[1]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[1]->meaning));
				ASSERT_TRUE(tmpTable->enumTypes[1] != NULL);
				tmpBuffer.data = string1;
				tmpBuffer.length = i;
				ASSERT_TRUE(tmpTable->enumTypes[2]->value == 2);
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[2]->display));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpTable->enumTypes[2]->meaning));

				/* Test third entry */
				tmpEntry = dataDictionary.entriesArray[3];
				tmpBuffer.data = string3;
				tmpBuffer.length = k;
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->acronym));
				ASSERT_TRUE(rsslBufferIsEqual(&tmpBuffer, &tmpEntry->ddeAcronym));
				ASSERT_TRUE(tmpEntry->fid == 3);
				ASSERT_TRUE(tmpEntry->rippleToField == 0);
				ASSERT_TRUE(tmpEntry->fieldType == RSSL_MFEED_INTEGER);
				ASSERT_TRUE(tmpEntry->length == 5);
				ASSERT_TRUE(tmpEntry->enumLength == 0);
				ASSERT_TRUE(tmpEntry->rwfType == RSSL_DT_UINT);
				ASSERT_TRUE(tmpEntry->rwfLength == 2);

				/* Test third field entry's enum table (null) */
				ASSERT_TRUE(tmpEntry->pEnumTypeTable == NULL);

				ASSERT_TRUE(rsslDeleteDataDictionary(&dataDictionary) == RSSL_RET_SUCCESS);
			}
		}
	}

	free(string3);
	free(string2);
	free(string1);
	remove("tmpFile.txt");
}

TEST(dataDictionaryTest, MismatchAcronymDefinitionTest)
{
	RsslDataDictionary dataDictionary;
	RsslBuffer errorText = { 255, (char*)alloca(255) };
	FILE *dictionaryFile;

	ASSERT_TRUE((dictionaryFile = fopen("tmpFile.txt", "w")) != NULL);

	/* An enumerated type */
	fprintf(dictionaryFile,
		/*ACRONYM    FID*/
		"SPS_STYPE  12833\n"
		/* VALUE      DISPLAY   MEANING */
		"      0   \"        \"   undefined\n"
		"      1   \"CHE     \"   CHE\n"
		"      2   \"CVA     \"   CVA\n"
		"      3   \"ITDG    \"   ITDG\n");

	fclose(dictionaryFile);
	rsslClearDataDictionary(&dataDictionary);

	/* Load enumerated type definition */
	ASSERT_TRUE(rsslLoadEnumTypeDictionary("tmpFile.txt", &dataDictionary, &errorText) == RSSL_RET_SUCCESS);

	ASSERT_TRUE((dictionaryFile = fopen("tmpFile.txt", "w")) != NULL);

	/* A field type */
	fprintf(dictionaryFile,
		/* ACRONYM    DDE ACRONYM          FID  RIPPLES TO  FIELD TYPE     LENGTH  RWF TYPE   RWF LEN */
		"PRE_1ET132 \"PRED 1 ET 132\"      12833  NULL        ENUMERATED    3 ( 8 )  ENUM             1 \n");

	fclose(dictionaryFile);

	/* Load field definition */
	ASSERT_TRUE(rsslLoadFieldDictionary("tmpFile.txt", &dataDictionary, &errorText) == RSSL_RET_FAILURE);
	EXPECT_STREQ("Acronym mismatch \"SPS_STYPE\" and \"PRE_1ET132\" between Field Dictionary and Enum Type Dictionary", errorText.data);

	remove("tmpFile.txt");
}

TEST(lengthSpecifiedConversionTest,lengthSpecifiedConversionTest)
{

	RsslInt64 i, putval, getVal;
	char buf[10];
	RsslInt64 size, sizeg;
	RsslInt64 failure = 0;
	RsslInt64 cursize = 0;
	int count = 0;
	int batchcount = 0;
	RsslUInt8 bitmask = 0;

	do
	{
		getVal = 0;
		putval = 0;
		for (i = 0; i < 8; ++i)
		{
			if ( bitmask & (0x01 << i) )
				((char*)(&putval))[i] = (char)0xFF;
			else
				((char*)(&putval))[i] = 0x00;
		}

		size = RWF_PUT_LENSPEC_I64(buf, &putval);
		sizeg = RWF_GET_LENSPEC_I64(&getVal, buf);
		//printf(LLX "", putval); // If needed to verify test values.

		ASSERT_TRUE(putval == getVal);
	} while (bitmask++ < 0xFF);

}

TEST(nonLengthSpecifiedConversionTest,nonLengthSpecifiedConversionTest)
{

	RsslInt64 i, putval, getVal;
	RsslUInt64 putUVal, getUVal;
	char buf[10];
	RsslInt64 size, sizeg;
	RsslUInt8 bitmask = 0;

	do
	{
		getVal = 0;
		putval = 0;
		for (i = 0; i < 8; ++i)
		{
			if ( bitmask & (0x01 << i) )
				((char*)(&putval))[i] = (char)0xFF;
			else
				((char*)(&putval))[i] = 0x00;
		}

		size = TRWF_PUT_LENSPEC_I64_NO_LENGTH(buf, &putval);
		sizeg = RWF_GET_LENSPEC_I64_SIZE(&getVal, buf, size);
		//printf("%llx", putval); // If needed to verify test values.

		ASSERT_TRUE(putval == getVal);
	} while (bitmask++ < 0xFF);

	bitmask = 0;
	do
	{
		getUVal = 0;
		putUVal = 0;
		for (i = 0; i < 8; ++i)
		{
			if ( bitmask & (0x01 << i) )
				((char*)(&putUVal))[i] = (char)0xFF;
			else
				((char*)(&putUVal))[i] = 0x00;
		}

		size = RWF_PUT_LENSPEC_U64_NO_LENGTH(buf, &putUVal);
		sizeg = RWF_GET_LENSPEC_U64_SIZE(&getUVal, buf, size);
		//printf("%llx", putUVal); // If needed to verify test values.

		ASSERT_TRUE(putUVal == getUVal);
	} while (bitmask++ < 0xFF);

	//printf("");
}


// Simple test to check that casting to unsigned prevents the >> operator from doing sign extension.
TEST(unsignedRightShiftTest,unsignedRightShiftTest)
{	
	RsslInt8 testChar = (RsslInt8)0xFF;
	RsslInt16 testShort = (RsslInt16)0xFFFF;
	RsslInt32 testInt =   0xFFFFFFFF;
	RsslInt64 testLong =  0xFFFFFFFFFFFFFFFFLL;

	RsslInt8* pTestChar = (RsslInt8*)malloc(sizeof(char));
	RsslUInt8* pTestUChar = (RsslUInt8*)malloc(sizeof(char));

	*pTestChar = testChar;
	*pTestUChar = (unsigned char)testChar;

	ASSERT_TRUE( testChar == testChar >> 1);
	ASSERT_TRUE( (RsslUInt8)testChar != (RsslUInt8)testChar >> 1);

	ASSERT_TRUE( testShort == testShort >> 1);
	ASSERT_TRUE( (RsslUInt16)testShort != (RsslUInt16)testShort >> 1);

	ASSERT_TRUE( testInt == testInt >> 1);
	ASSERT_TRUE( (RsslUInt32)testInt != (RsslUInt32)testInt >> 1);

	ASSERT_TRUE( testLong == testLong >> 1);
	ASSERT_TRUE( (RsslUInt64)testLong != (RsslUInt64)testLong >> 1);

	ASSERT_TRUE( *pTestChar == *pTestChar >> 1);
	ASSERT_TRUE( *pTestUChar != *pTestUChar >> 1);

	free(pTestChar);
	free(pTestUChar);

	//printf("");
}

void _int64EncDecTest(RsslInt64 putVal)
{
	RsslInt64 getVal = putVal + 1;

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeInt(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
				
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(&decIter, &getVal));

	ASSERT_TRUE(putVal == getVal);
}

// Test primitive encoders/decoders.
TEST(int64EncodeDecodeTest, int64EncodeDecodeTest)
{
	RsslInt64 putVal;
	
	/* Positive numbers with leading 0's */
	for (putVal = RTR_LL(0x5555555555555555); putVal != 0; putVal >>= 1)
		_int64EncDecTest(putVal);

	/* Negative numbers with leading 1's */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0xFFFFFFFFFFFFFFFF; putVal >>= 1)
		_int64EncDecTest(putVal);
	
	/* Numbers with trailing zeros */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0; putVal <<= 1)
		_int64EncDecTest(putVal);

	/* Highest hex number */
	_int64EncDecTest(RTR_LL(0xFFFFFFFFFFFFFFFF));

	/* 0 */
	_int64EncDecTest(RTR_LL(0));
	
}

void _dateEncDecTest(RsslDate* putVal)
{
	RsslDate getVal = { static_cast<RsslUInt8>(putVal->day + 1), static_cast<RsslUInt8>(putVal->month + 1), static_cast<RsslUInt8>(putVal->year + 1) };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(&decIter, &getVal));

	ASSERT_TRUE(putVal->day == getVal.day
		&& putVal->month == getVal.month
		&& putVal->year == getVal.year);
}

void _dateEncDecBlankTest()
{
	RsslDate putVal = RSSL_BLANK_DATE;
	RsslDate getVal = { static_cast<RsslUInt8>(putVal.day + 1), static_cast<RsslUInt8>(putVal.month + 1), static_cast<RsslUInt8>(putVal.year + 1) };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_BLANK_DATA == rsslDecodeDate(&decIter, &getVal));

	ASSERT_TRUE(putVal.day == getVal.day
		&& putVal.month == getVal.month
		&& putVal.year == getVal.year);
}


TEST(dateEncodeDecodeTest,dateEncodeDecodeTest)
{
	RsslDate putVal;
	
	/* test blank date first */
	_dateEncDecBlankTest();

	putVal.year = 4095 << 1; /* RsslDate years are only 16b at most */
	do 
	{
		
		for (putVal.month = 0; putVal.month <= 12; ++putVal.month)
		{
			for (putVal.day = 0; putVal.day <= 31; ++putVal.day)
			{
				_dateEncDecTest(&putVal);
			}	
		}
		putVal.year >>= 1;
	} while (putVal.year != 0);

}

void _stateEncodeDecodeTest(RsslState* putVal)
{
	RsslState getVal;
	
	getVal.code = putVal->code + 1;
	getVal.dataState = RSSL_DATA_NO_CHANGE;
	getVal.streamState = RSSL_STREAM_OPEN;
	getVal.text.data = NULL;
	getVal.text.length = 0;

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeState(&encIter, putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);

	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeState(&decIter, &getVal));
	ASSERT_TRUE(putVal->code == getVal.code);

}

TEST(stateEncodeDecodeTest,stateEncodeDecodeTest)
{
	RsslState putVal;
	int i = 0;

	putVal.code = 0;
	putVal.dataState = RSSL_DATA_NO_CHANGE;
	putVal.streamState = RSSL_STREAM_OPEN;
	putVal.text.data = NULL;
	putVal.text.length = 0;

	for(i = 0; i < 128; i++)
	{
		_stateEncodeDecodeTest(&putVal);
		putVal.code++;
	}
}


RsslRet _oldDecodeTime(RsslDecodeIterator *pIter, RsslTime *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			value->hour = value->minute = value->second = 255;
			value->millisecond = 65535;
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 2:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));

			return (((value->hour == 255) && (value->minute == 255)) ?
					(value->second = 255, value->millisecond = 65535, RSSL_RET_BLANK_DATA) :
					(value->second = 0, value->millisecond = 0, RSSL_RET_SUCCESS));
		break;
		case 3:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));

			return (((value->hour == 255) && (value->minute == 255) && (value->second == 255)) ?
					(value->millisecond = 65535, RSSL_RET_BLANK_DATA) : 
					(value->millisecond = 0, RSSL_RET_SUCCESS));
		break;
		case 4: 
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 5:
			rwfGet8(value->hour, pIter->_curBufPtr);
			rwfGet8(value->minute, (pIter->_curBufPtr + 1));
			rwfGet8(value->second, (pIter->_curBufPtr + 2));
			rwfGet16(value->millisecond, (pIter->_curBufPtr + 3));

			return (((value->hour == 255) && (value->minute == 255) && 
					 (value->second == 255) && (value->millisecond == 65535)) ?
					RSSL_RET_BLANK_DATA :
					RSSL_RET_SUCCESS);
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}

TEST(newToOldTimeTest,newToOldTimeTest)
{
	RsslTime getVal = RSSL_INIT_TIME;
	RsslTime putVal = {1,2,3,5,6,0};

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_INCOMPLETE_DATA == _oldDecodeTime(&decIter, &getVal));
}



void _timeEncDecTest(RsslTime* putVal)
{
	RsslTime getVal = { static_cast<RsslUInt8>(putVal->hour + 1), static_cast<RsslUInt8>(putVal->minute + 1), static_cast<RsslUInt8>(putVal->second + 1), static_cast<RsslUInt8>(putVal->millisecond + 1), static_cast<RsslUInt8>(putVal->microsecond + 1), static_cast<RsslUInt8>(putVal->nanosecond + 1) };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeTime(&decIter, &getVal));

	ASSERT_TRUE(putVal->hour == getVal.hour
		&& putVal->minute == getVal.minute
		&& putVal->second == getVal.second
		&& putVal->millisecond == getVal.millisecond
		&& putVal->microsecond == getVal.microsecond
		&& putVal->nanosecond == getVal.nanosecond);
}

void _timeEncDecBlankTest()
{
	RsslTime putVal = RSSL_BLANK_TIME;
	RsslTime getVal = { static_cast<RsslUInt8>(putVal.hour + 1), static_cast<RsslUInt8>(putVal.minute + 1), static_cast<RsslUInt8>(putVal.second + 1), static_cast<RsslUInt8>(putVal.millisecond + 1), static_cast<RsslUInt8>(putVal.microsecond + 1), static_cast<RsslUInt8>(putVal.nanosecond + 1) };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_BLANK_DATA == rsslDecodeTime(&decIter, &getVal));

	ASSERT_TRUE(putVal.hour == getVal.hour
		&& putVal.minute == getVal.minute
		&& putVal.second == getVal.second
		&& putVal.millisecond == getVal.millisecond
		&& putVal.microsecond == getVal.microsecond
		&& putVal.nanosecond == getVal.nanosecond);
}

TEST(timeEncDecTest,timeEncDecTest)
{
	RsslTime putVal;
	
	/* blank test */
	_timeEncDecBlankTest();

	putVal.hour = 23 + 1;
	do
	{
		--putVal.hour; putVal.minute = 59 + 1;
		do
		{
			--putVal.minute; putVal.second = 59 + 1;
			do
			{
				--putVal.second; putVal.millisecond = 999 << 1;
				do
				{
					putVal.millisecond >>= 1;putVal.microsecond = 999 << 1;
					do
					{
						putVal.microsecond >>=1;putVal.nanosecond = 999 << 1;
						do
						{
							putVal.nanosecond >>=1;
							_timeEncDecTest(&putVal);
						} while (putVal.nanosecond != 0);
					} while (putVal.microsecond != 0);
				} while ( putVal.millisecond != 0 );
			} while ( putVal.second != 0 );			
		} while ( putVal.minute != 0 );
	} while ( putVal.hour != 0 );
	
}

void  _floatEncDecTestInt(RsslDouble putVal)
{
	
	RsslDouble getVal;	

	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDouble(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
	
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(&decIter, &getVal));

	ASSERT_TRUE( *(RsslUInt64*)&putVal == *(RsslUInt64*)&getVal );
}

void  _float32EncDecTestInt(RsslFloat putVal)
{
	
	RsslFloat getVal;	

	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFloat(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
	
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeFloat(&decIter, &getVal));

	ASSERT_TRUE( *(RsslUInt32*)&putVal == *(RsslUInt32*)&getVal );
}

TEST(floatEncDecTest,floatEncDecTest)
{

	RsslDouble ii, jj;
	RsslDouble putVal = 0;
	RsslFloat putVal32 = 0;


	tbuf15.length = 15;

	_floatEncDecTestInt(0.0f);

	for (ii = -50; ii <= 50; ++ii)
	{
		for (jj = 1.0; jj < 10.0; jj += 0.0001111111)
		{
			// positive
			putVal32 = (RsslFloat)(jj*pow(10, ii));
			_float32EncDecTestInt(putVal32);

			// negative
			putVal32 = (RsslFloat)(-jj*pow(10, ii));
			_float32EncDecTestInt(putVal32);
		}
	}


	tbuf15.length = 15;
	// test effective exponent range
	for (ii = -308; ii <= 307; ++ii)
	{
		for (jj = 1.0; jj < 10.0; jj += 0.0001111111)
		{
			// positive
			putVal = jj*pow(10, ii);
			_floatEncDecTestInt(putVal);

			// negative
			putVal = -jj*pow(10, ii);
			_floatEncDecTestInt(putVal);
		}

	}

}

RsslRet _oldDecodeReal(RsslDecodeIterator *pIter, RsslReal *value)
{
	RsslUInt8 format;

	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0: 
			value->isBlank = RSSL_TRUE;
			value->hint = 0;
			value->value = 0;
			return RSSL_RET_BLANK_DATA;
		break;
		case 1: 
		value->isBlank = RSSL_TRUE;
		value->hint = 0;
		value->value = 0;
			return RSSL_RET_SUCCESS;
		break;

		default:
			if (rwfGetReal64(&value->value, &format, ((RsslUInt16)(pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr)), pIter->_curBufPtr ) > 0)
			{
				value->isBlank = (format & 0x20) ? RSSL_TRUE : RSSL_FALSE;
				value->hint = format & 0x1F;
				return RSSL_RET_SUCCESS;
			}
			return RSSL_RET_INVALID_DATA;
	}

	return RSSL_RET_INVALID_DATA;
}

TEST(newToOldReal,newToOldReal)
{
	RsslReal putVal = { 0, RSSL_RH_INFINITY, 0};
	RsslReal getVal = { 0, 0, 0 };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE(  (RSSL_RET_SUCCESS == _oldDecodeReal(&decIter, &getVal)));

	ASSERT_TRUE((getVal.isBlank == RSSL_TRUE));

}


void _realEncDecTest(RsslReal* putVal)
{
	RsslReal getVal = { 0, static_cast<RsslUInt8>(putVal->hint + 1), putVal->value + 1 };

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( (putVal->isBlank ? RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS) == rsslDecodeReal(&decIter, &getVal));

	ASSERT_TRUE(
		(putVal->isBlank == RSSL_TRUE && getVal.isBlank == RSSL_TRUE)
		|| putVal->hint == getVal.hint && putVal->value == getVal.value && putVal->isBlank == getVal.isBlank);
}

TEST(realEncDecTest,realEncDecTest)
{
	RsslReal putVal;
	putVal.isBlank = RSSL_FALSE;
	
	/* format ranges from 0 to 30 */
	for(putVal.hint = 0; putVal.hint <= 30; ++putVal.hint)
	{

		/* Works like integer test*/

		/* Positive numbers with leading 0's */
		for (putVal.value = RTR_LL(0x5555555555555555); putVal.value != 0; putVal.value >>= 1)
			_realEncDecTest(&putVal);

		/* Negative numbers with leading 1's */
		for (putVal.value = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal.value != 0xFFFFFFFFFFFFFFFF; putVal.value >>= 1)
			_realEncDecTest(&putVal);
		
		/* Numbers with trailing zeros */
		for (putVal.value = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal.value != 0; putVal.value = putVal.value << 1)
			_realEncDecTest(&putVal);

		/* Highest hex number */
		putVal.value = RTR_LL(0xFFFFFFFFFFFFFFFF);
		_realEncDecTest(&putVal);

		/* 0 */
		putVal.value = 0;
		_realEncDecTest(&putVal);

	}

	putVal.isBlank = RSSL_TRUE;
	_realEncDecTest(&putVal);

	/* Test Infinity, -Infinity, and NaN */
	putVal.isBlank = RSSL_FALSE;
	putVal.hint = RSSL_RH_INFINITY;
	_realEncDecTest(&putVal);

	putVal.isBlank = RSSL_FALSE;
	putVal.hint = RSSL_RH_NEG_INFINITY;
	_realEncDecTest(&putVal);

	putVal.isBlank = RSSL_FALSE;
	putVal.hint = RSSL_RH_NOT_A_NUMBER;
	_realEncDecTest(&putVal);

}

#define TEST_MAX_BUFFER_SIZE 0x3fff
TEST(bufferEncDecTest,bufferEncDecTest)
{
	RsslInt8 success = 1;
	
	RsslBuffer putVal, getVal;
	RsslUInt32 iiBuffer;
	RsslUInt8 _sizeBytes = 0;
	RsslUInt32 bufferLen[] = { 0x3, 0x3F, 0x40, 0x3FFF, 0x40, TEST_MAX_BUFFER_SIZE };

	char* buffer = (char*)malloc(TEST_MAX_BUFFER_SIZE*sizeof(char));
	
	for (iiBuffer = 0; iiBuffer < sizeof(bufferLen)/sizeof(RsslUInt32); ++iiBuffer)
	{
		putVal.length = bufferLen[iiBuffer];
		putVal.data = buffer;
		
		ASSERT_TRUE((_rsslEncodeBuffer15(bufBig, &putVal), RSSL_TRUE));
		_sizeBytes = (bufferLen[iiBuffer] < 0x80 ? 1 : 2);

		ASSERT_TRUE((_rsslDecodeBuffer15(&getVal, bufBig), RSSL_TRUE));

		if (putVal.length != getVal.length
			|| bufBig + _sizeBytes != getVal.data // buffer pointer should match
			)
		{
			success = 0;
			break;
		}

	}
	
	free(buffer);
	 
	ASSERT_TRUE(success);

}


TEST(bufferInterfaceEncDecTest,bufferInterfaceEncDecTest)
{
	RsslInt8 success = 1;
	
	RsslBuffer putVal, getVal, tempBuf;

	RsslUInt32 iiBuffer;
	RsslUInt8 _sizeBytes = 0;
	RsslUInt32 bufferLen[] = { 0x3, 0x3F, 0x40, 0x3FFF, 0x40, TEST_MAX_BUFFER_SIZE };

	char* buffer = (char*)malloc(TEST_MAX_BUFFER_SIZE*sizeof(char));
	
	for (iiBuffer = 0; iiBuffer < sizeof(bufferLen)/sizeof(RsslUInt32); ++iiBuffer)
	{
		putVal.length = bufferLen[iiBuffer];
		putVal.data = buffer;
		tempBuf.data = bufBig;
		tempBuf.length = 16000000;

		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, &tempBuf);		
		ASSERT_TRUE((rsslEncodeBuffer(&encIter, &putVal), RSSL_TRUE));
		tempBuf.length = rsslGetEncodedBufferLength(&encIter);

		_sizeBytes = (bufferLen[iiBuffer] < 0x80 ? 1 : 2);

		rsslClearDecodeIterator(&decIter);
		rsslSetDecodeIteratorBuffer(&decIter, &tempBuf);
		ASSERT_TRUE((rsslDecodeBuffer(&decIter, &getVal), RSSL_TRUE));

		if (putVal.length != getVal.length)			
		{
			success = 0;
			break;
		}

		if (memcmp(putVal.data, getVal.data, putVal.length) != 0)
		{
			success = 0;
			break;
		}

	}
	
	free(buffer);
	 
	ASSERT_TRUE(success);

}


RsslRet _oldDecodeDateTime(RsslDecodeIterator *pIter, RsslDateTime *value)
{
	RSSL_ASSERT(pIter, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(value, Invalid parameters or parameters passed in as NULL);

	switch ((pIter->_levelInfo[pIter->_decodingLevel + 1]._endBufPtr - pIter->_curBufPtr))
	{
		case 0:
			value->date.day = value->date.month = 0;
			value->date.year = 0;
			value->time.hour = value->time.minute = value->time.second = 255;
			value->time.millisecond = 65535;
			return RSSL_RET_BLANK_DATA;
		break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 6:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));

			/* need this to populate rest of time properly */
			/* if the time we took from wire matches blank, fill in rest of time as blank, 
			 * then ensure that date portion is also blank - if so, return blank data.  
			 * If time portion was not blank, just return success */
			return (((value->time.hour == 255) && (value->time.minute == 255)) ?
					(value->time.second = 255, value->time.millisecond = 65535,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.second = 0,value->time.millisecond = 0, RSSL_RET_SUCCESS));
		break;
		case 7:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));

			/* need this to populate rest of time properly */
			/* if the time we took from wire matches blank, fill in rest of time as blank, 
			 * then ensure that date portion is also blank - if so, return blank data.  
			 * If time portion was not blank, just return success */
			return (((value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255)) ?
					(value->time.millisecond = 65535,
					(((value->date.day == 0) && (value->date.month == 0) && (value->date.year == 0)) ? 
					RSSL_RET_BLANK_DATA : RSSL_RET_SUCCESS)) :
					(value->time.millisecond = 0, RSSL_RET_SUCCESS));
		  
			return RSSL_RET_SUCCESS;
		case 8:
			return RSSL_RET_INCOMPLETE_DATA;
		break;
		case 9:
			rwfGet8(value->date.day, pIter->_curBufPtr);
			rwfGet8(value->date.month, (pIter->_curBufPtr + 1));
			rwfGet16(value->date.year, (pIter->_curBufPtr + 2));
			rwfGet8(value->time.hour, (pIter->_curBufPtr + 4));
			rwfGet8(value->time.minute, (pIter->_curBufPtr + 5));
			rwfGet8(value->time.second, (pIter->_curBufPtr + 6));
			rwfGet16(value->time.millisecond, (pIter->_curBufPtr + 7));

			if ((value->date.day == 0) && (value->date.year == 0) && (value->date.month == 0) &&
				(value->time.hour == 255) && (value->time.minute == 255) && (value->time.second == 255) &&
				(value->time.millisecond == 65535))
				return RSSL_RET_BLANK_DATA;
			else
				return RSSL_RET_SUCCESS;
		break;
		default:
			return RSSL_RET_INCOMPLETE_DATA;
	}
	return RSSL_RET_INCOMPLETE_DATA;
}


TEST(newToOldDateTimeTest,newToOldDateTimeTest)
{
	RsslDateTime getVal = RSSL_INIT_DATETIME;
	RsslDateTime putVal = {RSSL_INIT_DATE, RSSL_INIT_TIME};
	putVal.time.nanosecond = 100;

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_INCOMPLETE_DATA == _oldDecodeDateTime(&decIter, &getVal));
}


void _dateTimeEncDecTest( RsslDateTime* putVal)
{
	RsslDateTime getVal = { 
		{static_cast<RsslUInt8>(putVal->date.day + 1), static_cast<RsslUInt8>(putVal->date.month + 1), static_cast<RsslUInt8>(putVal->date.year + 1)},
		{static_cast<RsslUInt8>(putVal->time.hour + 1), static_cast<RsslUInt8>(putVal->time.minute + 1), static_cast<RsslUInt8>(putVal->time.second + 1), static_cast<RsslUInt8>(putVal->time.millisecond + 1), static_cast<RsslUInt8>(putVal->time.microsecond + 1), static_cast<RsslUInt8>(putVal->time.nanosecond + 1)}
	};

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(&decIter, &getVal));

	ASSERT_TRUE(
		putVal->date.day == getVal.date.day
		&& putVal->date.month == getVal.date.month
		&& putVal->date.year == getVal.date.year
		&& putVal->time.hour == getVal.time.hour
		&& putVal->time.minute == getVal.time.minute
		&& putVal->time.second == getVal.time.second
		&& putVal->time.millisecond == getVal.time.millisecond
		&& putVal->time.microsecond == getVal.time.microsecond
		&& putVal->time.nanosecond == getVal.time.nanosecond);
}

void _dateTimeEncDecBlankTest()
{
	RsslDateTime putVal = RSSL_BLANK_DATETIME;
	RsslDateTime getVal = { 
		{static_cast<RsslUInt8>(putVal.date.day + 1), static_cast<RsslUInt8>(putVal.date.month + 1), static_cast<RsslUInt8>(putVal.date.year + 1)},
		{static_cast<RsslUInt8>(putVal.time.hour + 1), static_cast<RsslUInt8>(putVal.time.minute + 1), static_cast<RsslUInt8>(putVal.time.second + 1), static_cast<RsslUInt8>(putVal.time.millisecond + 1), static_cast<RsslUInt8>(putVal.time.microsecond + 1), static_cast<RsslUInt8>(putVal.time.nanosecond + 1)}
	};

	/* Encode value */
	tbuf15.length = 15;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &putVal));
	tbuf15.length = rsslGetEncodedBufferLength(&encIter);
			
	/* Decode value */
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbuf15);
	ASSERT_TRUE( RSSL_RET_BLANK_DATA == rsslDecodeDateTime(&decIter, &getVal));

	ASSERT_TRUE(
		putVal.date.day == getVal.date.day
		&& putVal.date.month == getVal.date.month
		&& putVal.date.year == getVal.date.year
		&& putVal.time.hour == getVal.time.hour
		&& putVal.time.minute == getVal.time.minute
		&& putVal.time.second == getVal.time.second
		&& putVal.time.millisecond == getVal.time.millisecond
		&& putVal.time.microsecond == getVal.time.microsecond
		&& putVal.time.nanosecond == getVal.time.nanosecond);
}

TEST(dateTimeEncDecTest,dateTimeEncDecTest)
{
	RsslDateTime putVal;
	_dateTimeEncDecBlankTest();
	// Iterates by shifting 1 bit at a time for each member.  
	// Helps us check all possible value sizes without going through every single possible value.
	// shift left all starting values 1 bit. The loop will shift 1 to the right, going from the max valid value to zero.
	putVal.date.year = 4095 << 1;
	do { putVal.date.year   >>= 1; putVal.date.month = 0xC /*12*/ << 1;
	do { putVal.date.month >>= 1; putVal.date.day = 0x1F/*31*/ << 1; 
	do { putVal.date.day >>= 1;

	putVal.time.hour = 23 + 1;
	do
	{
		--putVal.time.hour; putVal.time.minute = 59 << 1;
		do
		{
			putVal.time.minute >>= 1; putVal.time.second = 59 << 1;
			do
			{
				putVal.time.second >>= 1; putVal.time.millisecond = 999 << 1;
				do
				{
					putVal.time.millisecond >>= 1;putVal.time.microsecond = 999 << 1;
					do
					{
						putVal.time.microsecond >>=1;putVal.time.nanosecond = 999 << 1;
						do
						{
							putVal.time.nanosecond >>=1;
							_dateTimeEncDecTest(&putVal);
						} while (putVal.time.nanosecond != 0);
					} while (putVal.time.microsecond != 0);
				} while ( putVal.time.millisecond != 0 );
			} while ( putVal.time.second != 0 );			
		} while ( putVal.time.minute != 0 );
	} while ( putVal.time.hour != 0 );

	} while (putVal.date.day != 0);
	} while (putVal.date.month != 0);
	} while (putVal.date.year != 0);
}

#define TEST_MAP_MAX_ENTRY_FLAGS 0x1
#define TEST_MAP_MAX_ENTRY_ACTIONS 0x3
#define TEST_MAP_MAX_ENTRIES ((TEST_MAP_MAX_ENTRY_FLAGS + 1) * TEST_MAP_MAX_ENTRY_ACTIONS) // Flags start at 0

TEST(mapEncDecTest,mapEncDecTest)
{
	RsslMap container = RSSL_INIT_MAP;
	RsslMapEntry entry;
	char summaryData[] = "$";
	char permissionData[] = "permission";
	char testExpBuf[128];

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags, *entryFlags;
	RsslUInt32 flagsSize, iiFlags, entryFlagsSize, iiEntryFlags;
	RsslUInt32 curEntryCount;
	RsslUInt8 flagsCopy;

	RsslUInt32 flagsBase[] = 
	{ 
    	RSSL_MPF_HAS_SUMMARY_DATA,
    	RSSL_MPF_HAS_PER_ENTRY_PERM_DATA,
    	RSSL_MPF_HAS_TOTAL_COUNT_HINT,
    	RSSL_MPF_HAS_KEY_FIELD_ID
	};
	RsslUInt32 entryFlagsBase[] = { RSSL_MPEF_HAS_PERM_DATA };
	RsslUInt32 entryActions[] = { RSSL_MPEA_ADD_ENTRY, RSSL_MPEA_DELETE_ENTRY, RSSL_MPEA_UPDATE_ENTRY };
	RsslUInt32 entryActionsSize = sizeof(entryActions)/sizeof(RsslUInt32), iiEntryActions;
	RsslInt64 mapKey = 0;

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	entryFlagsSize = _allocateFlagCombinations(&entryFlags, entryFlagsBase, sizeof(entryFlagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = entryFlagsSize * entryActionsSize;



	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.


	for (iiMaxEntries = 0; iiMaxEntries < maxEntries; ++iiMaxEntries)
	{
		for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
		{
			//// Encode Tests ////

			// Set up container
			rsslClearMap(&container);
			container.containerType = RSSL_DT_FIELD_LIST;
			container.flags = flags[iiFlags];
			flagsCopy = container.flags;

			container.keyPrimitiveType = RSSL_DT_INT; // integer keys

			// Fill parts of the container based on the flag combination being tested.

			/* Pre-encoded summary data */
			if (container.flags & RSSL_MPF_HAS_SUMMARY_DATA)
			{
				container.encSummaryData.length = sizeof(summaryData);
				container.encSummaryData.data = summaryData;
			}

			if (container.flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT)
			{
				container.totalCountHint = 5;
			}

			// Begin container encoding
			tbufBig.length = TEST_BIG_BUF_SIZE;
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%u-Elem Map(flags=%u): rsslEncodeMapInit", maxEntries, flags[iiFlags]);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapInit(&encIter, &container, 0, 0));

			// Encode entries
			curEntryCount = 0;
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				if (curEntryCount == iiMaxEntries) break;
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					if (curEntryCount == iiMaxEntries) break;
					++curEntryCount;

					rsslClearMapEntry(&entry);
					entry.flags = entryFlags[iiEntryFlags];

					entry.action = entryActions[iiEntryActions];

					if (entryFlags[iiEntryFlags] & RSSL_MPEF_HAS_PERM_DATA)
					{						
						entry.permData.length = sizeof(permissionData);
						entry.permData.data = permissionData;
						flagsCopy |= RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;
					}
					else
						flagsCopy &= ~RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;

					mapKey = entryFlags[iiEntryFlags]; /* use entry flags as map key */

					// Begin entry encoding
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapEntryInit(&encIter, &entry, &mapKey, 0));

					// Add payload
					if (entryActions[iiEntryActions] != RSSL_MPEA_DELETE_ENTRY)
					{
						_encodeFieldList(&encIter);

					}

					// Finish entry encoding
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapEntryComplete(&encIter, RSSL_TRUE)); // assume success for now.
				}
			}
			/* if no entries, there is no per-entry perms */
			if (iiMaxEntries == 0)
				flagsCopy &= ~RSSL_MPF_HAS_PER_ENTRY_PERM_DATA;

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator
			//rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

			// Setup container
			rsslClearMap(&container);
			rsslClearMapEntry(&entry);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMap(&decIter, &container));

			ASSERT_TRUE(
					container.keyPrimitiveType == RSSL_DT_INT
					&& container.flags == flagsCopy
					&& (!(container.flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT) || container.totalCountHint == 5)
					&& (!(container.flags & RSSL_MPF_HAS_SUMMARY_DATA) 
						|| container.encSummaryData.length == sizeof(summaryData) && 0 == memcmp(container.encSummaryData.data, summaryData, sizeof(summaryData)))
					&& container.containerType == RSSL_DT_FIELD_LIST);

			// Decode entries
			curEntryCount = 0;
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				if (curEntryCount == iiMaxEntries) break;
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					if (curEntryCount == iiMaxEntries) break;
					++curEntryCount;
					rsslClearMapEntry(&entry);
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMapEntry(&decIter, &entry, &mapKey));

					if (entryActions[iiEntryActions] != RSSL_MPEA_DELETE_ENTRY)
					{
						_decodeFieldList(&decIter);
					}
					else
					{
						RsslFieldList fList;
						ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(&decIter, &fList, 0));
					}

					ASSERT_TRUE(entry.flags == entryFlags[iiEntryFlags]
							&& mapKey == entryFlags[iiEntryFlags] /* entry flags are used as map key */
							&& entry.action == entryActions[iiEntryActions]
							&& ((entryFlags[iiEntryFlags] & RSSL_MPEF_HAS_PERM_DATA && flags[iiFlags] & RSSL_MPF_HAS_PER_ENTRY_PERM_DATA ) ? 
								(entry.permData.length == sizeof(permissionData) && 0 == memcmp(entry.permData.data, permissionData, entry.permData.length)) : 1));
				}
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeMapEntry(&decIter, &entry, 0));

		}

		//// End Decode Tests ////
	}
	free(flags);
	free(entryFlags);
}

// Test is designed to be copied and modified; make sure the defined values aren't accidentally reused.
#undef TEST_MAP_MAX_ENTRY_FLAGS
#undef TEST_MAP_MAX_ENTRY_ACTIONS
#undef TEST_MAP_MAX_ENTRIES

TEST(vectorEncDecTest,vectorEncDecTest)
{
	RsslVector container = RSSL_INIT_VECTOR;
	RsslVectorEntry entry;
	char summaryData[] = "$";
	char permissionData[] = "permission";
	char testExpBuf[128];
	RsslUInt8 flagsCopy;

RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags, *entryFlags;
	RsslUInt32 flagsSize, iiFlags, entryFlagsSize, iiEntryFlags;

	RsslUInt32 flagsBase[] = 
	{ 
    	RSSL_VTF_HAS_SUMMARY_DATA,
    	RSSL_VTF_HAS_PER_ENTRY_PERM_DATA,
    	RSSL_VTF_HAS_TOTAL_COUNT_HINT,
    	RSSL_VTF_SUPPORTS_SORTING			
	};
	RsslUInt32 entryFlagsBase[] = { RSSL_VTEF_HAS_PERM_DATA };
	RsslUInt32 entryActions[] = { RSSL_VTEA_CLEAR_ENTRY, RSSL_VTEA_DELETE_ENTRY, RSSL_VTEA_INSERT_ENTRY, RSSL_VTEA_SET_ENTRY, RSSL_VTEA_UPDATE_ENTRY };
	RsslUInt32 entryActionsSize = sizeof(entryActions)/sizeof(RsslUInt32), iiEntryActions;
	RsslUInt32 curEntryCount;
	RsslInt64 mapKey = 0;

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	entryFlagsSize = _allocateFlagCombinations(&entryFlags, entryFlagsBase, sizeof(entryFlagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = entryFlagsSize * entryActionsSize;

	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	for (iiMaxEntries = 0; iiMaxEntries < maxEntries; ++iiMaxEntries)
	{
		for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
		{
			//// Encode Tests ////

			// Set up container
			rsslClearVector(&container);
			container.containerType = RSSL_DT_FIELD_LIST;
			container.flags = flags[iiFlags];
			flagsCopy = container.flags;

			// Fill parts of the container based on the flag combination being tested.

			/* Pre-encoded summary data */
			if (container.flags & RSSL_VTF_HAS_SUMMARY_DATA)
			{
				container.encSummaryData.length = sizeof(summaryData);
				container.encSummaryData.data = summaryData;
			}

			if (container.flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT)
			{
				container.totalCountHint = 5;
			}

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%u-Elem Vector(flags=%u): rsslEncodeVectorInit", maxEntries, flags[iiFlags]);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorInit(&encIter, &container, 0, 0));

			// Encode entries
			curEntryCount = 0;
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				if (curEntryCount == iiMaxEntries) break;
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					if (curEntryCount == iiMaxEntries) break;
					++curEntryCount;
					rsslClearVectorEntry(&entry);
					entry.flags = entryFlags[iiEntryFlags];

					entry.action = entryActions[iiEntryActions];

					if (entryFlags[iiEntryFlags] & RSSL_VTEF_HAS_PERM_DATA)
					{
						entry.permData.length = sizeof(permissionData);
						entry.permData.data = permissionData;
						flagsCopy |= RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;
					}
					else
						flagsCopy &= ~RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;

					// Begin entry encoding
					entry.index = entryFlags[iiEntryFlags]; // same as index in this case
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorEntryInit(&encIter, &entry, 0));

					// Add payload
					if (entryActions[iiEntryActions] != RSSL_VTEA_CLEAR_ENTRY && entryActions[iiEntryActions] != RSSL_VTEA_DELETE_ENTRY)
					{
						_encodeFieldList(&encIter);
					}

					// Finish entry encoding
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorEntryComplete(&encIter, RSSL_TRUE)); // assume success for now.
				}
			}
			/* if no entries, there is no per-entry perms */
			if (iiMaxEntries == 0)
				flagsCopy &= ~RSSL_VTF_HAS_PER_ENTRY_PERM_DATA;

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator
			//rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

			// Setup container
			rsslClearVector(&container);
			rsslClearVectorEntry(&entry);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeVector(&decIter, &container));


			ASSERT_TRUE(
				container.flags == flagsCopy
				&& (!(container.flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT) || container.totalCountHint == 5)
				&& (!(container.flags & RSSL_VTF_HAS_SUMMARY_DATA) 
					|| container.encSummaryData.length == sizeof(summaryData) && 0 == memcmp(container.encSummaryData.data, summaryData, sizeof(summaryData)))
				&& container.containerType == RSSL_DT_FIELD_LIST);
		
			// Decode entries
			curEntryCount = 0;
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				if (curEntryCount == iiMaxEntries) break;
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					if (curEntryCount == iiMaxEntries) break;
					++curEntryCount;
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeVectorEntry(&decIter, &entry));

					if (entryActions[iiEntryActions] != RSSL_VTEA_CLEAR_ENTRY && entryActions[iiEntryActions] != RSSL_VTEA_DELETE_ENTRY)
					{
						_decodeFieldList(&decIter);
					}
					else
					{
						RsslFieldList fList;
						ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(&decIter, &fList, 0));
					}

					ASSERT_TRUE(entry.flags == entryFlags[iiEntryFlags]
						&& entry.action == entryActions[iiEntryActions]
						&& (entry.flags & RSSL_VTEF_HAS_PERM_DATA ? 0 == memcmp(entry.permData.data, permissionData, entry.permData.length) : 1));
				}
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeVectorEntry(&decIter, &entry));

		}

		//// End Decode Tests ////
	}
	free(flags);
	free(entryFlags);
}

#define TEST_ARRAY_MAX_ENTRIES 6
TEST(arrayEncDecTest,arrayEncDecTest)
{
	RsslArray container = RSSL_INIT_ARRAY;
	RsslInt64 entry = 0;
	RsslUInt64 entU64 = 0;
	RsslUInt16 entU16 = 0;
	RsslInt64 comparissonEntry = 0;
	RsslBuffer decArrayBuf;

	char testExpBuf[128];

	RsslUInt32 maxEntries, iEntries;

	/* fixed length 8 byte int */
	// Encode & decode containers of increasing size.
	const RsslInt myInt = 0xfaabd00d;

	for (maxEntries = 0; maxEntries <= TEST_ARRAY_MAX_ENTRIES; ++maxEntries)
	{
			//// Encode Tests ////

			// Set up container
			rsslClearArray(&container);
			container.primitiveType = RSSL_DT_INT;
			container.itemLength = 8;

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%i-Elem Array: rsslEncodeArrayInit", maxEntries);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayInit(&encIter, &container));

			// Encode entries
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				entry = myInt + iEntries;
				// Begin entry encoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeArrayEntry(&encIter, 0, &entry));
			}

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator

			// Setup container
			rsslClearArray(&container);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeArray(&decIter, &container));


			ASSERT_TRUE(
				container.primitiveType == RSSL_DT_INT
				&& container.itemLength == 8);
		
			// Decode entries
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeArrayEntry(&decIter, &decArrayBuf));
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeInt(&decIter, &entry));
				ASSERT_TRUE(entry == (myInt + iEntries));
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeArrayEntry(&decIter, &decArrayBuf));


		//// End Decode Tests ////
	}

	/* fixed length 2 byte uint */
	entU64 = 0xfa;
	// Encode & decode containers of increasing size.
	for (maxEntries = 0; maxEntries <= TEST_ARRAY_MAX_ENTRIES; ++maxEntries)
	{
			//// Encode Tests ////

			// Set up container
			rsslClearArray(&container);
			container.primitiveType = RSSL_DT_UINT;
			container.itemLength = 2;

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%i-Elem Array: rsslEncodeArrayInit", maxEntries);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayInit(&encIter, &container));

			// Encode entries
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				// Begin entry encoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeArrayEntry(&encIter, 0, &entU64));
			}

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator

			// Setup container
			rsslClearArray(&container);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeArray(&decIter, &container));


			ASSERT_TRUE(
				container.primitiveType == RSSL_DT_UINT
				&& container.itemLength == 2);
		
			// Decode entries
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeArrayEntry(&decIter, &decArrayBuf));
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &entU64));
				ASSERT_TRUE(entU64 == 0xfa);
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeArrayEntry(&decIter, &decArrayBuf));


		//// End Decode Tests ////
	}


	// Encode & decode containers of increasing size.
	for (maxEntries = 0; maxEntries <= TEST_ARRAY_MAX_ENTRIES; ++maxEntries)
	{
			//// Encode Tests ////
			/* variable length int */
			/* starting value */
			entU64 = 0xf0;

			// Set up container
			rsslClearArray(&container);
			container.primitiveType = RSSL_DT_UINT;
			container.itemLength = 0; // variable length

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%i-Elem Array: rsslEncodeArrayInit", maxEntries);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayInit(&encIter, &container));

			// Encode entries
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				// Begin entry encoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeArrayEntry(&encIter, 0, &entU64));
				/* now change entry value */
				entU64 *= entU64;
			}

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeArrayComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator

			// Setup container
			rsslClearArray(&container);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeArray(&decIter, &container));


			ASSERT_TRUE(
				container.primitiveType == RSSL_DT_UINT
				&& container.itemLength == 0);
		
			// Decode entries
			comparissonEntry = 0xf0;
			for(iEntries = 0; iEntries < maxEntries; ++iEntries)
			{
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeArrayEntry(&decIter, &decArrayBuf));
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &entU64));
				ASSERT_TRUE(entU64 == comparissonEntry);
				comparissonEntry *= comparissonEntry;
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeArrayEntry(&decIter, &decArrayBuf));


		//// End Decode Tests ////
	}
}

// Test is designed to be copied and modified; make sure the defined values aren't accidentally reused.
#undef TEST_ARRAY_MAX_ENTRY_FLAGS
#undef TEST_ARRAY_MAX_ENTRY_ACTIONS
#undef TEST_ARRAY_MAX_ENTRIES

#define TEST_SERIES_MAX_ENTRIES 8

TEST(seriesEncDecTest,seriesEncDecTest)
{
	RsslSeries container = RSSL_INIT_SERIES;
	RsslSeriesEntry entry;
	char summaryData[] = "$";
	char permissionData[] = "permission";
	char testExpBuf[128];

	RsslUInt8 flagsList[] = { RSSL_SRF_NONE, RSSL_SRF_HAS_SUMMARY_DATA, RSSL_SRF_HAS_TOTAL_COUNT_HINT, RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT };
	RsslUInt8 iiFlagsList;
	RsslUInt8 flags, entryFlags;
	RsslUInt32 maxEntries;

	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.

	for (maxEntries = 0; maxEntries <= TEST_SERIES_MAX_ENTRIES; ++maxEntries)
	{

		for (iiFlagsList = 0; iiFlagsList < sizeof(flagsList)/sizeof(RsslUInt8); ++iiFlagsList)
		{
			flags = flagsList[iiFlagsList];

			//// Encode Tests ////

			// Set up container
			rsslClearSeries(&container);
			container.containerType = RSSL_DT_FIELD_LIST;
			container.flags = flags;

			// Fill parts of the container based on the flag combination being tested.
		
			/* Pre-encoded summary data */
			if (container.flags & RSSL_SRF_HAS_SUMMARY_DATA)
			{
				container.encSummaryData.length = sizeof(summaryData);
				container.encSummaryData.data = summaryData;
			}

			if (container.flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT)
			{
				container.totalCountHint = 5;
			}

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%i-Elem Series(flags=%x): rsslEncodeSeriesInit", maxEntries, flags);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesInit(&encIter, &container, 0, 0));

			// Encode entries
			for(entryFlags = 0; entryFlags < maxEntries; ++entryFlags)
			{
				rsslClearSeriesEntry(&entry);

				// Begin entry encoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesEntryInit(&encIter, &entry, 0));

				// Add payload
				_encodeFieldList(&encIter);

				// Finish entry encoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesEntryComplete(&encIter, RSSL_TRUE)); // assume success for now.
			}

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////

			//// Decode Tests ////

			// Setup decode iterator
			//rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

			// Setup container
			rsslClearSeries(&container);
			rsslClearSeriesEntry(&entry);

			rsslClearDecodeIterator(&decIter);
			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeSeries(&decIter, &container));


			ASSERT_TRUE(
				container.flags == flags
				&& (!(container.flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT) || container.totalCountHint == 5)
				&& (!(container.flags & RSSL_SRF_HAS_SUMMARY_DATA) 
					|| container.encSummaryData.length == sizeof(summaryData) && 0 == memcmp(container.encSummaryData.data, summaryData, sizeof(summaryData)))
				&& container.containerType == RSSL_DT_FIELD_LIST);

			// Decode entries
			for(entryFlags = 0; entryFlags < maxEntries; ++entryFlags)
			{
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeSeriesEntry(&decIter, &entry));
				_decodeFieldList(&decIter);
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeSeriesEntry(&decIter, &entry));

		}

		//// End Decode Tests ////
	}
}


// Test is designed to be copied and modified; make sure the defined values aren't accidentally reused.
#undef TEST_SERIES_MAX_ENTRY_FLAGS 
#undef TEST_SERIES_MAX_ENTRY_ACTIONS 
#undef TEST_SERIES_MAX_ENTRIES

TEST(filterEncDecTest,filterEncDecTest)
{

	RsslFilterList container = RSSL_INIT_FILTER_LIST;
	RsslFilterEntry entry;
	char summaryData[] = "$";
	char permissionData[] = "permission";

	char testExpBuf[128];

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags, *entryFlags;
	RsslUInt32 flagsSize, iiFlags, entryFlagsSize, iiEntryFlags;
	RsslUInt8 flagsCopy;

	RsslUInt32 flagsBase[] = { RSSL_FTF_HAS_PER_ENTRY_PERM_DATA,	RSSL_FTF_HAS_TOTAL_COUNT_HINT };
	RsslUInt32 entryFlagsBase[] = { RSSL_FTEF_HAS_CONTAINER_TYPE, RSSL_FTEF_HAS_PERM_DATA };
	RsslUInt32 entryActions[] = { RSSL_FTEA_CLEAR_ENTRY, RSSL_FTEA_SET_ENTRY, RSSL_FTEA_UPDATE_ENTRY };
	RsslUInt32 entryActionsSize = sizeof(entryActions)/sizeof(RsslUInt32), iiEntryActions;
	RsslUInt32 expectedCount;

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	entryFlagsSize = _allocateFlagCombinations(&entryFlags, entryFlagsBase, sizeof(entryFlagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = entryFlagsSize * entryActionsSize;
 
	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.

	for (iiMaxEntries = 0; iiMaxEntries < maxEntries; ++iiMaxEntries)
	{

		for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
		{
			//// Encode Tests ////
		
			// Set up container
			rsslClearFilterList(&container);
			container.containerType = RSSL_DT_FIELD_LIST;
			container.flags = flags[iiFlags];
			flagsCopy = container.flags;

			// Fill parts of the container based on the flag combination being tested.

			if (container.flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT)
			{
				container.totalCountHint = 5;
			}

			// Begin container encoding
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			sprintf(testExpBuf, "%u-Elem FilterList(flags=%u): rsslEncodeFilterListInit", maxEntries, flags[iiFlags]);
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterListInit(&encIter, &container));

			expectedCount = 0;
			// Encode entries
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					rsslClearFilterEntry(&entry);

					entry.flags = entryFlags[iiEntryFlags]; 

					entry.containerType = 
						(entryFlags[iiEntryFlags] & RSSL_FTEF_HAS_CONTAINER_TYPE)
						? RSSL_DT_FIELD_LIST : RSSL_DT_NO_DATA /* this should get overwritten */;

					entry.action = entryActions[iiEntryActions];

					if (entryFlags[iiEntryFlags] & RSSL_FTEF_HAS_PERM_DATA)
					{
						entry.permData.length = sizeof(permissionData);
						entry.permData.data = permissionData;
						flagsCopy |= RSSL_FTF_HAS_PER_ENTRY_PERM_DATA;
					}
					else
						flagsCopy &= ~RSSL_FTF_HAS_PER_ENTRY_PERM_DATA;

					entry.id = entryFlags[iiEntryFlags]; /*same as index in this case*/
			
					// Begin entry encoding
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFilterEntryInit(&encIter, &entry, 0));

					// Add  payload
					if (entryActions[iiEntryActions] != RSSL_FTEA_CLEAR_ENTRY)
					{
						_encodeFieldList(&encIter);
					}

					// Finish entry encoding
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFilterEntryComplete(&encIter, RSSL_TRUE)); // assume success for now.
					++expectedCount;
				}
			}
			/* if no entries, there is no per-entry perms */

			// Finish container encoding
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterListComplete(&encIter, RSSL_TRUE)); // assume success for now.

			//// End Encode Tests ////
		
			//// Decode Tests ////

			// Setup decode iterator
			//rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

			// Setup container
			rsslClearFilterList(&container);
			rsslClearFilterEntry(&entry);

			// Begin container decoding
			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFilterList(&decIter, &container));


			ASSERT_TRUE( 
				container.flags == flagsCopy
				&& (!(container.flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT) || container.totalCountHint == 5)
				&& container.containerType == RSSL_DT_FIELD_LIST);
		
			// Decode entries
			for(iiEntryFlags = 0; iiEntryFlags < entryFlagsSize; ++iiEntryFlags)
			{
				for(iiEntryActions = 0; iiEntryActions < entryActionsSize; ++iiEntryActions)
				{
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFilterEntry(&decIter, &entry));

				
					if (entry.containerType == RSSL_DT_FIELD_LIST)
					{
						if (entryActions[iiEntryActions] != RSSL_FTEA_CLEAR_ENTRY)
							_decodeFieldList(&decIter);
						else
						{
							RsslFieldList fList;
							ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(&decIter, &fList, 0));
						}
					}

					ASSERT_TRUE(
					
						entry.flags == entryFlags[iiEntryFlags] /* Has container flag should be gone if container is same type */
						&& entry.id == entryFlags[iiEntryFlags]
						&& (entry.containerType == !(entryFlags[iiEntryFlags] & RSSL_FTEF_HAS_CONTAINER_TYPE)
							? RSSL_DT_FIELD_LIST : RSSL_DT_FIELD_LIST)
						&& entry.action == entryActions[iiEntryActions]
						&& (entry.flags & RSSL_FTEF_HAS_PERM_DATA ? 0 == memcmp(entry.permData.data, permissionData, entry.permData.length) : 1));
				}
			}
			ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFilterEntry(&decIter, &entry));

		}

		//// End Decode Tests ////
	}
	free(flags);
	free(entryFlags);
}

/*Byte swap macro test*/
void _swapU64OldToNewEncDecTest(RsslUInt64 putVal)
{
	RsslUInt64 getVal = putVal + 1;

	/* Encode value */

	RWF_PUT_LENSPEC_U64(tbuf15.data, &putVal);
	rwfGetLenSpecU64(&getVal, tbuf15.data);
	
	ASSERT_TRUE(putVal == getVal);
}

TEST(oldToNewU64EncodeDecodeTest,oldToNewU64EncodeDecodeTest)
{
	RsslUInt64 uPutVal;

	/* leading 0's */
	for (uPutVal = RTR_ULL(0x5555555555555555); uPutVal != 0; uPutVal >>= 1)
		_swapU64OldToNewEncDecTest(uPutVal);
	
	for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal >>= 1)
		_swapU64OldToNewEncDecTest(uPutVal);

	/* trailing 0's */

	for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal <<= 1)
		_swapU64OldToNewEncDecTest(uPutVal);

	/* Highest hex number */
	_swapU64OldToNewEncDecTest(RTR_ULL(0xFFFFFFFFFFFFFFFF));

	/* 0 */
	_swapU64OldToNewEncDecTest(RTR_ULL(0));
}

void _swapU64NewToNewEncDecTest(RsslUInt64 putVal)
{
	RsslUInt64 getVal = putVal + 1;

	/* Encode value */
	rwfPutLenSpecU64(tbuf15.data, putVal);
	rwfGetLenSpecU64(&getVal, tbuf15.data);
	
	ASSERT_TRUE(putVal == getVal);
}

TEST(newToNewU64EncodeDecodeTest,newToNewU64EncodeDecodeTest)
{
	RsslUInt64 uPutVal;
	int i;
	
	for (i = 0; i < 1000; ++i)
	{

		/* leading 0's */
		for (uPutVal = RTR_ULL(0x5555555555555555); uPutVal != 0; uPutVal >>= 1)
			_swapU64NewToNewEncDecTest(uPutVal);
		
		for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal >>= 1)
			_swapU64NewToNewEncDecTest(uPutVal);

		/* trailing 0's */

		for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal <<= 1)
			_swapU64NewToNewEncDecTest(uPutVal);

		/* Highest hex number */
		_swapU64NewToNewEncDecTest(RTR_ULL(0xFFFFFFFFFFFFFFFF));

		/* 0 */
		_swapU64NewToNewEncDecTest(RTR_ULL(0));
	}
}

void _swapU64SizeNewToNewEncDecTest(RsslUInt64 putVal)
{
	RsslUInt64 getVal = putVal + 1;
	rtrInt32 size;
	/* Encode value */
	size = rwfPutLenSpecU64(tbuf15.data, putVal);
	rwfGetLenSpecU64Size(&getVal, (tbuf15.data+1), (size-1));
	
	ASSERT_TRUE(putVal == getVal);
}

TEST(newToNewU64SizeEncodeDecodeTest,newToNewU64SizeEncodeDecodeTest)
{
	RsslUInt64 uPutVal;
	int i;
	
	for (i = 0; i < 1000; ++i)
	{

		/* leading 0's */
		for (uPutVal = RTR_ULL(0x5555555555555555); uPutVal != 0; uPutVal >>= 1)
			_swapU64SizeNewToNewEncDecTest(uPutVal);
		
		for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal >>= 1)
			_swapU64SizeNewToNewEncDecTest(uPutVal);

		/* trailing 0's */

		for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal <<= 1)
			_swapU64SizeNewToNewEncDecTest(uPutVal);

		/* Highest hex number */
		_swapU64SizeNewToNewEncDecTest(RTR_ULL(0xFFFFFFFFFFFFFFFF));

		/* 0 */
		_swapU64SizeNewToNewEncDecTest(RTR_ULL(0));
	}
}

void _swapU64NewToOldEncDecTest(RsslUInt64 putVal)
{
	RsslUInt64 getVal = putVal + 1;

	/* Encode value */
	rwfPutLenSpecU64(tbuf15.data, putVal);
	RWF_GET_LENSPEC_U64(&getVal, tbuf15.data);
	
	ASSERT_TRUE(putVal == getVal);
}

TEST(newToOldU64EncodeDecodeTest,newToOldU64EncodeDecodeTest)
{
	RsslUInt64 uPutVal;

	/* leading 0's */
	for (uPutVal = RTR_ULL(0x5555555555555555); uPutVal != 0; uPutVal >>= 1)
		_swapU64NewToOldEncDecTest(uPutVal);
	
	for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal >>= 1)
		_swapU64NewToOldEncDecTest(uPutVal);

	/* trailing 0's */

	for (uPutVal = RTR_ULL(0xAAAAAAAAAAAAAAAA); uPutVal != 0; uPutVal <<= 1)
		_swapU64NewToOldEncDecTest(uPutVal);

	/* Highest hex number */
	_swapU64NewToOldEncDecTest(RTR_ULL(0xFFFFFFFFFFFFFFFF));

	/* 0 */
	_swapU64NewToOldEncDecTest(RTR_ULL(0));
}

void _swapI64NewToNewEncDecTest(RsslInt64 putVal)
{
	RsslInt64 getVal = putVal + 1;

	/* Encode value */
	rwfPutLenSpecI64(tbuf15.data, putVal);
	rwfGetLenSpecI64(&getVal, tbuf15.data);
	

	ASSERT_TRUE(putVal == getVal);
}

TEST(newToNewI64EncodeDecodeTest,newToNewI64EncodeDecodeTest)
{
	RsslInt64 putVal;
	int i;

	for (i = 0; i < 1000; ++i)
	{
	
		/* Positive numbers with leading 0's */
		for (putVal = RTR_LL(0x5555555555555555); putVal != 0; putVal >>= 1)
			_swapI64NewToNewEncDecTest(putVal);

		/* Negative numbers with leading 1's */
		for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != RTR_LL(0xFFFFFFFFFFFFFFFF); putVal >>= 1)
			_swapI64NewToNewEncDecTest(putVal);
		
		/* Numbers with trailing zeros */
		for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0; putVal <<= 1)
			_swapI64NewToNewEncDecTest(putVal);

		/* Highest hex number */
		_swapI64NewToNewEncDecTest(RTR_LL(0xFFFFFFFFFFFFFFFF));

		/* 0 */
		_swapI64NewToNewEncDecTest(RTR_LL(0));
	}
}

void _swapI64SizeNewToNewEncDecTest(RsslInt64 putVal)
{
	RsslInt64 getVal = putVal + 1;
	rtrInt32 size;

	/* Encode value */
	size = rwfPutLenSpecI64(tbuf15.data, putVal);
	rwfGetLenSpecI64Size(&getVal, (tbuf15.data+1), (size-1));
	

	ASSERT_TRUE(putVal == getVal);
}

TEST(newToNewI64SizeEncodeDecodeTest,newToNewI64SizeEncodeDecodeTest)
{
	RsslInt64 putVal;
	int i;

	for (i = 0; i < 1000; ++i)
	{
	
		/* Positive numbers with leading 0's */
		for (putVal = RTR_LL(0x5555555555555555); putVal != 0; putVal >>= 1)
			_swapI64SizeNewToNewEncDecTest(putVal);

		/* Negative numbers with leading 1's */
		for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != RTR_LL(0xFFFFFFFFFFFFFFFF); putVal >>= 1)
			_swapI64SizeNewToNewEncDecTest(putVal);
		
		/* Numbers with trailing zeros */
		for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0; putVal <<= 1)
			_swapI64SizeNewToNewEncDecTest(putVal);

		/* Highest hex number */
		_swapI64SizeNewToNewEncDecTest(RTR_LL(0xFFFFFFFFFFFFFFFF));

		/* 0 */
		_swapI64SizeNewToNewEncDecTest(RTR_LL(0));
	}
}


void _swapI64NewToOldEncDecTest(RsslInt64 putVal)
{
	RsslInt64 getVal = putVal + 1;


	/* Encode value */	
	rwfPutLenSpecI64(tbuf15.data, putVal);
	RWF_GET_LENSPEC_I64(&getVal, tbuf15.data);
	
	ASSERT_TRUE(putVal == getVal);
}

TEST(newToOldI64EncodeDecodeTest,newToOldI64EncodeDecodeTest)
{
	RsslInt64 putVal;
	
	/* Positive numbers with leading 0's */
	for (putVal = RTR_LL(0x5555555555555555); putVal != 0; putVal >>= 1)
		_swapI64NewToOldEncDecTest(putVal);

	/* Negative numbers with leading 1's */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != RTR_LL(0xFFFFFFFFFFFFFFFF); putVal >>= 1)
		_swapI64NewToOldEncDecTest(putVal);
	
	/* Numbers with trailing zeros */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0; putVal <<= 1)
		_swapI64NewToOldEncDecTest(putVal);

	/* Highest hex number */
	_swapI64NewToOldEncDecTest(RTR_LL(0xFFFFFFFFFFFFFFFF));

	/* 0 */
	_swapI64NewToOldEncDecTest(RTR_LL(0));
}

void _swapI64OldToNewEncDecTest(RsslInt64 putVal)
{
	RsslInt64 getVal =  putVal + 1;

	/* Encode value */
	RWF_PUT_LENSPEC_I64(tbuf15.data, &putVal);
	rwfGetLenSpecI64(&getVal, tbuf15.data);
	

	ASSERT_TRUE(putVal == getVal);
}

TEST(oldToNewI64EncodeDecodeTest,oldToNewI64EncodeDecodeTest)
{
	RsslInt64 putVal;
	
	/* Positive numbers with leading 0's */
	for (putVal = RTR_LL(0x5555555555555555); putVal != 0; putVal >>= 1)
		_swapI64OldToNewEncDecTest(putVal);

	/* Negative numbers with leading 1's */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != RTR_LL(0xFFFFFFFFFFFFFFFF); putVal >>= 1)
		_swapI64NewToOldEncDecTest(putVal);
	
	/* Numbers with trailing zeros */
	for (putVal = RTR_LL(0xAAAAAAAAAAAAAAAA); putVal != 0; putVal <<= 1)
		_swapI64OldToNewEncDecTest(putVal);

	/* Highest hex number */
	_swapI64OldToNewEncDecTest(RTR_LL(0xFFFFFFFFFFFFFFFF));

	/* 0 */
	_swapI64OldToNewEncDecTest(RTR_LL(0));
}




// Test is designed to be copied and modified; make sure the defined values aren't accidentally reused.
#undef TEST_FILTER_LIST_MAX_CONTAINER_FLAGS
#undef TEST_FILTER_LIST_MAX_ENTRY_FLAGS
#undef TEST_FILTER_LIST_MAX_ENTRY_ACTIONS
#undef TEST_FILTER_LIST_MAX_ENTRIES

TEST(fieldListEncDecTest,fieldListEncDecTest)
{
    /* The elementList test is a direct copy of this. Make sure any updates are propogated. */

	/* TODO float? double? QoS? */
	char testExpBuf[128];

	RsslFieldList container = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry entry;

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags;
	RsslUInt32 flagsSize, iiFlags;
	RsslUInt32 iiEntry;

	RsslUInt32 flagsBase[] = 
	{ 
		RSSL_FLF_HAS_STANDARD_DATA, 
		RSSL_FLF_HAS_FIELD_LIST_INFO
	};

	RsslInt64 payloadVal = 0, decodePayloadVal = 0, expectedPayloadVal = 0;
	RsslUInt8 useFieldEntryInitComplete = 0;

	// Encode and decode one of each primitive type.
	// These arrays for fids and dataTypes make up our "Dictionary"
	
	RsslUInt8 fids[] =
	{
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8
	};

	RsslUInt8 dataTypes[] = 
	{
		RSSL_DT_INT,
		RSSL_DT_DOUBLE,
		RSSL_DT_REAL,
		RSSL_DT_DATE,
		RSSL_DT_TIME,
		RSSL_DT_DATETIME,
		RSSL_DT_ARRAY,
		RSSL_DT_UINT
	};


	RsslInt64 paylInt = -2049, decInt;
	RsslUInt64 paylUInt = 2049, decUInt;
	RsslDouble paylFloat = 0xFF, decFloat;
	RsslReal paylReal = {0, 1, 0xFFFFF}, decReal;
	RsslDate paylDate = {8, 3, 1892}, decDate;
	RsslTime paylTime = { 23, 59, 59, 999, 999,999 }, decTime;
	RsslDateTime paylDateTime = { {8, 3, 1892}, {23, 59, 59, 999, 999, 999} }, decDateTime;
	RsslBuffer decBuf;
	const RsslUInt64 arrayUInt = 0xDEEEDEEE; /* 4 byte length limit in test */
	char preEncodeArrayBuf[256];
	RsslBuffer preEncodeArrayRBuf = { 256, preEncodeArrayBuf };

	RsslArray array;

	/* Pre-encode array */
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &preEncodeArrayRBuf);
	array.primitiveType = RSSL_DT_UINT;
	array.itemLength = 4;
	ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
	preEncodeArrayRBuf.length = rsslGetEncodedBufferLength(&encIter);

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = sizeof(dataTypes)/sizeof(RsslUInt8);
	
	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	for (iiMaxEntries = 0; iiMaxEntries <= maxEntries; ++iiMaxEntries)
	{
		for (useFieldEntryInitComplete = 0; useFieldEntryInitComplete <= 1; ++useFieldEntryInitComplete )
		{
			for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
			{
				//// Encode Tests ////

				// Set up encoder iterator
				rsslClearEncodeIterator(&encIter);
				tbufBig.length = TEST_BIG_BUF_SIZE;
				rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

				// Set up container
				rsslClearFieldList(&container);
				container.setId = 0; // No sidding for now
				container.flags = flags[iiFlags];

				// Fill parts of the container based on the flag combination being tested.

				// TODO: Currently not using these flags
				//RSSL_FLF_SET_DATA
				//RSSL_FLF_SET_ID

				if ( flags[iiFlags] & RSSL_FLF_HAS_FIELD_LIST_INFO ) /* Indicates presence of Dict ID and Field List Number */
				{
					container.dictionaryId = 257;
					container.fieldListNum = 256;
				}

				// Begin container encoding
				sprintf(testExpBuf, "%u-Elem FieldList(flags=%u): rsslEncodeFieldListInit", maxEntries, flags[iiFlags]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&encIter, &container, 0, 0));

				if ( flags[iiFlags] & RSSL_FLF_HAS_STANDARD_DATA ) /* Indicates field list has elements */
				{
					// Encode entries
					for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
					{
						rsslClearFieldEntry(&entry);
						entry.fieldId = fids[iiEntry];
						entry.dataType = dataTypes[iiEntry];

						if (!useFieldEntryInitComplete)
						{
							// Begin entry encoding
							switch(dataTypes[iiEntry])
							{
							case RSSL_DT_INT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylInt)); break;
							case RSSL_DT_UINT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylUInt)); break;
							case RSSL_DT_DOUBLE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylFloat)); break;
							case RSSL_DT_REAL:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylReal)); break;
							case RSSL_DT_DATE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylDate)); break;
							case RSSL_DT_TIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylTime)); break;
							case RSSL_DT_DATETIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylDateTime)); break;
							case RSSL_DT_ARRAY:
								entry.encData = preEncodeArrayRBuf;
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, 0)); break;
								break;
							default:
								ASSERT_TRUE(0); break;
							}
						}
						else
						{
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntryInit(&encIter, &entry, 15));
							// Begin entry encoding
							switch(dataTypes[iiEntry])
							{
							case RSSL_DT_INT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeInt(&encIter, &paylInt)); break;
							case RSSL_DT_UINT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeUInt(&encIter, &paylUInt)); break;
							case RSSL_DT_DOUBLE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDouble(&encIter, &paylFloat)); break;
							case RSSL_DT_REAL:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &paylReal)); break;
							case RSSL_DT_DATE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, &paylDate)); break;
							case RSSL_DT_TIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &paylTime)); break;
							case RSSL_DT_DATETIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &paylDateTime)); break;
							case RSSL_DT_ARRAY:
								array.primitiveType = RSSL_DT_UINT;
								array.itemLength = 4;
								ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
								break;
							default:
								ASSERT_TRUE(0); break;
							}

							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntryComplete(&encIter, RSSL_TRUE));
						}
					}
				}

				// Finish container encoding
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encIter, RSSL_TRUE)); // assume success for now.

				//// End Encode Tests ////

				//// Decode Tests ////

				// Setup container
				rsslClearFieldList(&container);
				rsslClearFieldEntry(&entry);

				// Begin container decoding
				rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
				ASSERT_TRUE(RSSL_RET_SUCCESS  == rsslDecodeFieldList(&decIter, &container, 0));

				ASSERT_TRUE( container.flags == flags[iiFlags]);

				if ( flags[iiFlags] & RSSL_FLF_HAS_FIELD_LIST_INFO ) /* Indicates presence of Dict ID and Field List Number */
				{
					ASSERT_TRUE(
						container.dictionaryId == 257
						&& container.fieldListNum == 256);
				}

				if ( rsslFieldListCheckHasStandardData(&container))
				{
					// Decode entries
					for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
					{
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));

						++expectedPayloadVal;

						ASSERT_TRUE(entry.fieldId == fids[iiEntry] && entry.dataType == RSSL_DT_UNKNOWN);

						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							decInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(&decIter, &decInt)
								&& decInt == paylInt);
							break;
						case RSSL_DT_UINT:
							decUInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &decUInt)
								&& decUInt == paylUInt);
							break;
						case RSSL_DT_DOUBLE:
							decFloat = 0.0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(&decIter, &decFloat)
								&& decFloat == paylFloat); /* not rounded inside encoding/decoding, so this should match exactly */
							break;
						case RSSL_DT_REAL:
							decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
								&& decReal.isBlank == paylReal.isBlank
								&& (decReal.isBlank == RSSL_TRUE 
								|| decReal.hint == paylReal.hint
								&& decReal.value == paylReal.value));
							break;
						case RSSL_DT_DATE:
							decDate.day = 0; decDate.month = 0; decDate.year = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(&decIter, &decDate)
								&& decDate.day == paylDate.day
								&& decDate.month == paylDate.month
								&& decDate.year == paylDate.year);
							break;
						case RSSL_DT_TIME:
							decTime.hour = 0; decTime.minute = 0; decTime.second = 0; decTime.millisecond = 0; 
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(&decIter, &decTime)
								&& decTime.hour == paylTime.hour
								&& decTime.minute == paylTime.minute
								&& decTime.second == paylTime.second
								&& decTime.millisecond == paylTime.millisecond);
							break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(&decIter, &decDateTime)
								&& decDateTime.date.day == paylDateTime.date.day
								&& decDateTime.date.month == paylDateTime.date.month
								&& decDateTime.date.year == paylDateTime.date.year
								&& decDateTime.time.hour == paylDateTime.time.hour
								&& decDateTime.time.minute == paylDateTime.time.minute
								&& decDateTime.time.second == paylDateTime.time.second
								&& decDateTime.time.millisecond == paylDateTime.time.millisecond);
							break;
						case RSSL_DT_ARRAY:
							ASSERT_TRUE(rsslDecodeArray(&decIter, &array) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(array.primitiveType == RSSL_DT_UINT && array.itemLength == 4);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_END_OF_CONTAINER);
							break;
							
						default:
							ASSERT_TRUE(0);
							break;
						}
					}
				}
				ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(&decIter, &entry));

			}
		}
	}
		//// End Decode Tests ////

	free(flags);
}



TEST(fieldListRealTest,fieldListRealTest)
{
	RsslFieldList container = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry entry;
	RsslInt64 payloadVal = 0, decodePayloadVal = 0, expectedPayloadVal = 0;
	RsslUInt8 useFieldEntryInitComplete = 0;


	
	RsslReal paylRealInf = {0, RSSL_RH_INFINITY, 0};
	RsslReal paylRealNan = {0, RSSL_RH_NOT_A_NUMBER, 0};
	RsslReal decReal;
	
	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	//// Encode Tests ////

	// Set up encoder iterator
	rsslClearEncodeIterator(&encIter);
	tbufBig.length = TEST_BIG_BUF_SIZE;
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	// Set up container
	rsslClearFieldList(&container);
	container.setId = 0; // No sidding for now
	container.flags = RSSL_FLF_HAS_STANDARD_DATA;


	// Begin container encoding
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&encIter, &container, 0, 0));

	rsslClearFieldEntry(&entry);
	entry.fieldId = 1;
	entry.dataType = RSSL_DT_REAL;
	//entry 1 is fid 1 with Real set to infinity; encode with single pass
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &paylRealInf)); 					// entry 2, fid 2 is real as nan, encode with Init/Compl
	entry.fieldId = 2;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntryInit(&encIter, &entry, 15));
	// Begin entry encoding
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &paylRealNan)); 			
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntryComplete(&encIter, RSSL_TRUE));
	// Finish container encoding
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encIter, RSSL_TRUE)); // assume success for now.

	//// End Encode Tests ////

	//// Decode Tests ////

	// Setup container
	rsslClearFieldList(&container);
	rsslClearFieldEntry(&entry);

	// Begin container decoding
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
	ASSERT_TRUE(RSSL_RET_SUCCESS  == rsslDecodeFieldList(&decIter, &container, 0));
	if ( rsslFieldListCheckHasStandardData(&container))
	{
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));
		decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
		ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
				&& decReal.isBlank == paylRealInf.isBlank
				&& (decReal.isBlank == RSSL_TRUE 
					|| decReal.hint == paylRealInf.hint
					&& decReal.value == paylRealInf.value));

		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));
		decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
		ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
				&& decReal.isBlank == paylRealNan.isBlank
				&& (decReal.isBlank == RSSL_TRUE 
					|| decReal.hint == paylRealNan.hint
					&& decReal.value == paylRealNan.value));
	}
	ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(&decIter, &entry));

		//// End Decode Tests ////
}


TEST(fieldListSetEncDecTest,fieldListSetEncDecTest)
{
    /* The elementList test is a direct copy of this. Make sure any updates are propogated. */
	RsslRet ret;
	/* TODO float? double? QoS? */
	char testExpBuf[128];

	RsslFieldList container = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry entry;

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags;
	RsslUInt32 flagsSize, iiFlags;
	RsslUInt32 iiEntry;
	RsslBuffer version = {4, const_cast<char*>("1.00")};
	RsslUInt32 flagsBase[] = 
	{ 
		RSSL_FLF_HAS_FIELD_LIST_INFO
	};

	RsslInt64 payloadVal = 0, decodePayloadVal = 0, expectedPayloadVal = 0;
	RsslUInt8 useFieldEntryInitComplete = 0;

	// Encode and decode one of each primitive type.
	// These arrays for fids and dataTypes make up our "Dictionary"
	
	RsslUInt8 fids[] =
	{
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8
	};

	RsslUInt8 dataTypes[] = 
	{
		RSSL_DT_INT,
		RSSL_DT_DOUBLE,
		RSSL_DT_REAL,
		RSSL_DT_DATE,
		RSSL_DT_TIME,
		RSSL_DT_DATETIME,
		RSSL_DT_ARRAY,
		RSSL_DT_UINT
	};


	RsslInt64 paylInt = -2049, decInt;
	RsslUInt64 paylUInt = 2049, decUInt;
	RsslDouble paylFloat = 0xFF, decFloat;
	RsslReal paylReal = {0, 1, 0xFFFFF}, decReal;
	RsslDate paylDate = {8, 3, 1892}, decDate;
	RsslTime paylTime = { 23, 59, 59, 999, 999, 999 }, decTime;
	RsslDateTime paylDateTime = { {8, 3, 1892}, {23, 59, 59, 999, 999, 999} }, decDateTime;
	RsslBuffer decBuf;
	const RsslUInt64 arrayUInt = 0xDEEEDEEE; /* 4 byte length limit in test */
	char preEncodeArrayBuf[256];
	RsslBuffer preEncodeArrayRBuf = { 256, preEncodeArrayBuf };

	RsslArray array;

	RsslFieldSetDefEntry globalFields[8] = 
	{	
		{1, RSSL_DT_INT},
		{2, RSSL_DT_DOUBLE},
		{3, RSSL_DT_REAL},
		{4, RSSL_DT_DATE},
		{5, RSSL_DT_TIME},
		{6, RSSL_DT_DATETIME},
		{7, RSSL_DT_ARRAY},
		{8, RSSL_DT_UINT}
	};

	RsslFieldSetDef globalDef = RSSL_INIT_FIELD_SET_DEF;
	RsslFieldSetDefDb globalDb = RSSL_INIT_FIELD_LIST_SET_DB;

	/* Pre-encode array */
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &preEncodeArrayRBuf);
	array.primitiveType = RSSL_DT_UINT;
	array.itemLength = 4;
	ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
	preEncodeArrayRBuf.length = rsslGetEncodedBufferLength(&encIter);


	globalDef.setId = 16;
	globalDef.pEntries = globalFields;

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = sizeof(dataTypes)/sizeof(RsslUInt8);
	
	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	for (iiMaxEntries = 1; iiMaxEntries <= maxEntries; ++iiMaxEntries)
	{
		for (useFieldEntryInitComplete = 0; useFieldEntryInitComplete <= 1; ++useFieldEntryInitComplete )
		{
			for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
			{
				//// Encode Tests ////

				// Set up encoder iterator
				rsslClearEncodeIterator(&encIter);
				tbufBig.length = TEST_BIG_BUF_SIZE;
				rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

				// Setup global set def db
				rsslAllocateFieldSetDefDb(&globalDb, &version);
				globalDef.count = iiMaxEntries;
				rsslAddFieldSetDefToDb(&globalDb, &globalDef);
				rsslSetEncodeIteratorGlobalFieldListSetDB(&encIter, &globalDb);

				// Set up container
				rsslClearFieldList(&container);
				container.flags = flags[iiFlags];

				container.flags |= RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;

				// Fill parts of the container based on the flag combination being tested.

				container.setId = 16;
				globalDef.count = iiMaxEntries;

				if ( flags[iiFlags] & RSSL_FLF_HAS_FIELD_LIST_INFO ) /* Indicates presence of Dict ID and Field List Number */
				{
					container.dictionaryId = 257;
					container.fieldListNum = 256;
				}

				// Begin container encoding
				sprintf(testExpBuf, "%i-Elem FieldList(flags=%x): rsslEncodeFieldListInit", iiMaxEntries, flags[iiFlags]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == (ret = rsslEncodeFieldListInit(&encIter, &container, 0, 0)));

				// Encode entries
				for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
				{
					rsslClearFieldEntry(&entry);
					entry.fieldId = globalFields[iiEntry].fieldId;
					entry.dataType = globalFields[iiEntry].dataType;
						

					if (!useFieldEntryInitComplete)
					{
						RsslRet expectedRet;
						expectedRet = ((iiEntry == iiMaxEntries - 1) ? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS);
						// Begin entry encoding
						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							ASSERT_TRUE( expectedRet == (ret = rsslEncodeFieldEntry(&encIter, &entry, &paylInt))); break;
						case RSSL_DT_UINT:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylUInt)); break;
						case RSSL_DT_DOUBLE:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylFloat)); break;
						case RSSL_DT_REAL:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylReal)); break;
						case RSSL_DT_DATE:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylDate)); break;
						case RSSL_DT_TIME:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylTime)); break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, &paylDateTime)); break;
						case RSSL_DT_ARRAY:
							entry.encData = preEncodeArrayRBuf;
							ASSERT_TRUE( expectedRet == rsslEncodeFieldEntry(&encIter, &entry, 0)); break;
							break;
						default:
							ASSERT_TRUE(0); break;
						}
					}
					else
					{
						RsslRet expectedRet;
						expectedRet = ((iiEntry == iiMaxEntries - 1) ? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS);
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntryInit(&encIter, &entry, 15));
						// Begin entry encoding
						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeInt(&encIter, &paylInt)); break;
						case RSSL_DT_UINT:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeUInt(&encIter, &paylUInt)); break;
						case RSSL_DT_DOUBLE:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDouble(&encIter, &paylFloat)); break;
						case RSSL_DT_REAL:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &paylReal)); break;
						case RSSL_DT_DATE:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, &paylDate)); break;
						case RSSL_DT_TIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &paylTime)); break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &paylDateTime)); break;
						case RSSL_DT_ARRAY:
							array.primitiveType = RSSL_DT_UINT;
							array.itemLength = 4;
							ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
							break;
						default:
							ASSERT_TRUE(0); break;
						}

						ASSERT_TRUE(expectedRet == rsslEncodeFieldEntryComplete(&encIter, RSSL_TRUE));
					}
				}

				// Finish container encoding
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encIter, RSSL_TRUE)); // assume success for now.

				//// End Encode Tests ////

				//// Decode Tests ////

				// Setup container
				rsslClearFieldList(&container);
				rsslClearFieldEntry(&entry);

				// Begin container decoding
				rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
				rsslSetDecodeIteratorGlobalFieldListSetDB(&decIter, &globalDb);
				ASSERT_TRUE(RSSL_RET_SUCCESS  == rsslDecodeFieldList(&decIter, &container, 0));

				EXPECT_EQ(container.flags, (flags[iiFlags] | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID));

				if ( flags[iiFlags] & RSSL_FLF_HAS_FIELD_LIST_INFO ) /* Indicates presence of Dict ID and Field List Number */
				{
					ASSERT_TRUE( container.dictionaryId == 257
						&& container.fieldListNum == 256);
				}

				if ( rsslFieldListCheckHasSetData(&container))
				{
					ASSERT_TRUE(container.setId == 16);
					// Decode entries
					for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
					{
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));

						++expectedPayloadVal;

						ASSERT_TRUE(entry.fieldId == globalFields[iiEntry].fieldId && entry.dataType == globalFields[iiEntry].dataType);

						switch(entry.dataType)
						{
						case RSSL_DT_INT:
							decInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == (ret = rsslDecodeInt(&decIter, &decInt))
								&& decInt == paylInt);
							break;
						case RSSL_DT_UINT:
							decUInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &decUInt)
								&& decUInt == paylUInt);
							break;
						case RSSL_DT_DOUBLE:
							decFloat = 0.0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(&decIter, &decFloat)
								&& decFloat == paylFloat); /* not rounded inside encoding/decoding, so this should match exactly */
							break;
						case RSSL_DT_REAL:
							decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
								&& decReal.isBlank == paylReal.isBlank
								&& (decReal.isBlank == RSSL_TRUE 
								|| decReal.hint == paylReal.hint
								&& decReal.value == paylReal.value));
							break;
						case RSSL_DT_DATE:
							decDate.day = 0; decDate.month = 0; decDate.year = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(&decIter, &decDate)
								&& decDate.day == paylDate.day
								&& decDate.month == paylDate.month
								&& decDate.year == paylDate.year);
							break;
						case RSSL_DT_TIME:
							decTime.hour = 0; decTime.minute = 0; decTime.second = 0; decTime.millisecond = 0; 
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(&decIter, &decTime)
								&& decTime.hour == paylTime.hour
								&& decTime.minute == paylTime.minute
								&& decTime.second == paylTime.second
								&& decTime.millisecond == paylTime.millisecond);
							break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(&decIter, &decDateTime)
								&& decDateTime.date.day == paylDateTime.date.day
								&& decDateTime.date.month == paylDateTime.date.month
								&& decDateTime.date.year == paylDateTime.date.year
								&& decDateTime.time.hour == paylDateTime.time.hour
								&& decDateTime.time.minute == paylDateTime.time.minute
								&& decDateTime.time.second == paylDateTime.time.second
								&& decDateTime.time.millisecond == paylDateTime.time.millisecond);
							break;
						case RSSL_DT_ARRAY:
							ASSERT_TRUE(rsslDecodeArray(&decIter, &array) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(array.primitiveType == RSSL_DT_UINT && array.itemLength == 4);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_END_OF_CONTAINER);
							break;
							
						default:
							ASSERT_TRUE(0);
							break;
						}
					}
				}
				ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == (ret =  rsslDecodeFieldEntry(&decIter, &entry)));

				// Delete global set def db
				rsslDeleteFieldSetDefDb(&globalDb);

			}
		}
	}
		//// End Decode Tests ////

	free(flags);
}

/** Test encoding a local field list set into a map, vector, and series. */
TEST(localSetDefContainerEncodeTest,localSetDefContainerEncodeTest)
{
	RsslMap map;
	RsslVector vector;
	RsslSeries series;
	RsslEncodeIterator eIter;
	RsslDecodeIterator dIter;
	const RsslInt32 bufCapacity = 512;
	RsslBuffer buf = { 512, (char*)alloca(512) };
	RsslLocalFieldSetDefDb fieldSetDb, decodeFieldSetDb;
	RsslLocalElementSetDefDb elemSetDb, decodeElemSetDb;
	RsslFieldSetDefEntry fieldSetDefEntry;
	RsslElementSetDefEntry elemSetDefEntry;
	RsslBuffer elemSetName = { 9, const_cast<char*>("RareEarth") };
	int i;

	/* Estimated encoded lengths for an empty map, vector, and series (no set definitions).
	 * These cannot be exact lengths as our overrun checks check for more than the exact
	 * space required (this is expected). */
	const RsslInt32 mapHeaderEstimateLength = 10;
	const RsslInt32 vectorHeaderEstimateLength = 10;
	const RsslInt32 seriesHeaderEstimateLength = 10;

	/* Setup set def dbs */
	rsslClearFieldSetDefEntry(&fieldSetDefEntry);
	fieldSetDefEntry.fieldId = 5;
	fieldSetDefEntry.dataType = RSSL_DT_DATETIME_7;

	rsslClearLocalFieldSetDefDb(&fieldSetDb);
	fieldSetDb.definitions[0].setId = 0;
	fieldSetDb.definitions[0].count = 1;
	fieldSetDb.definitions[0].pEntries = &fieldSetDefEntry;

	rsslClearElementSetDefEntry(&elemSetDefEntry);
	elemSetDefEntry.name = elemSetName;
	elemSetDefEntry.dataType = RSSL_DT_TIME_3;

	rsslClearLocalElementSetDefDb(&elemSetDb);
	elemSetDb.definitions[0].setId = 0;
	elemSetDb.definitions[0].count = 1;
	elemSetDb.definitions[0].pEntries = &elemSetDefEntry;


	for(i = 0; i < 2; ++i)
	{
		RsslContainerType containerType;

		if (i == 0)
			containerType = RSSL_DT_FIELD_LIST;
		else
			containerType = RSSL_DT_ELEMENT_LIST;


		/*** Test map rollback -- get the size for just the map header, then
		 * try to encode the header and set defs into a buffer of that size (this will give us 
		 * RSSL_RET_BUFFER_TOO_SMALL)  ***/

		buf.length = mapHeaderEstimateLength;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode map. */
		rsslClearMap(&map);
		map.flags = RSSL_MPF_HAS_SET_DEFS;
		map.keyPrimitiveType = RSSL_DT_INT;
		map.containerType = containerType;
		ASSERT_TRUE(rsslEncodeMapInit(&eIter, &map, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should run out of space. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_BUFFER_TOO_SMALL);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_BUFFER_TOO_SMALL);

		/* Rollback */
		ASSERT_TRUE(rsslEncodeMapSetDefsComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeMapComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslGetEncodedBufferLength(&eIter) == 0);

		/* Test that we can then encode something (in this case, just an empty map). */
		rsslClearMap(&map);
		map.flags = RSSL_MPF_NONE;
		map.keyPrimitiveType = RSSL_DT_INT;
		map.containerType = containerType;
		ASSERT_TRUE(rsslEncodeMapInit(&eIter, &map, 0, 0) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeMapComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode map */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearMap(&map);
		ASSERT_TRUE(rsslDecodeMap(&dIter, &map) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(map.keyPrimitiveType == RSSL_DT_INT);
		ASSERT_TRUE(map.flags == RSSL_MPF_NONE);


		/*** Test a successful map encoding. ***/

		buf.length = bufCapacity;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode map. */
		rsslClearMap(&map);
		map.flags = RSSL_MPF_HAS_SET_DEFS;
		map.keyPrimitiveType = RSSL_DT_INT;
		map.containerType = containerType;
		ASSERT_TRUE(rsslEncodeMapInit(&eIter, &map, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should succeed now. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_SUCCESS);

		/* Complete */
		ASSERT_TRUE(rsslEncodeMapSetDefsComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeMapComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode map */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearMap(&map);
		ASSERT_TRUE(rsslDecodeMap(&dIter, &map) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(map.keyPrimitiveType == RSSL_DT_INT);
		ASSERT_TRUE(map.flags == RSSL_MPF_HAS_SET_DEFS);

		/* Decode set defs. */
		if (containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&decodeFieldSetDb);
			ASSERT_TRUE(rsslDecodeLocalFieldSetDefDb(&dIter, &decodeFieldSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].count == 1);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].fieldId == 5);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_DATETIME_7);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&decodeElemSetDb);
			ASSERT_TRUE(rsslDecodeLocalElementSetDefDb(&dIter, &decodeElemSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].count == 1);
			ASSERT_TRUE(rsslBufferIsEqual(&decodeElemSetDb.definitions[0].pEntries[0].name, &elemSetName));
			ASSERT_TRUE(decodeElemSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_TIME_3);
		}

		/*** Test vector rollback -- get the size for just the vector header, then
		 * try to encode the header and set defs into a buffer of that size (this will give us 
		 * RSSL_RET_BUFFER_TOO_SMALL)  ***/

		buf.length = vectorHeaderEstimateLength;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode vector. */
		rsslClearVector(&vector);
		vector.flags = RSSL_VTF_HAS_SET_DEFS;
		vector.containerType = containerType;
		ASSERT_TRUE(rsslEncodeVectorInit(&eIter, &vector, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should run out of space. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_BUFFER_TOO_SMALL);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_BUFFER_TOO_SMALL);

		/* Rollback */
		ASSERT_TRUE(rsslEncodeVectorSetDefsComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeVectorComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslGetEncodedBufferLength(&eIter) == 0);

		/* Test that we can then encode something (in this case, just an empty vector). */
		rsslClearVector(&vector);
		vector.flags = RSSL_VTF_NONE;
		vector.containerType = containerType;
		ASSERT_TRUE(rsslEncodeVectorInit(&eIter, &vector, 0, 0) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeVectorComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode vector */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearVector(&vector);
		ASSERT_TRUE(rsslDecodeVector(&dIter, &vector) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(vector.flags == RSSL_VTF_NONE);


		/*** Test a successful vector encoding. ***/

		buf.length = bufCapacity;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode vector. */
		rsslClearVector(&vector);
		vector.flags = RSSL_VTF_HAS_SET_DEFS;
		vector.containerType = containerType;
		ASSERT_TRUE(rsslEncodeVectorInit(&eIter, &vector, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should succeed now. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_SUCCESS);

		/* Complete */
		ASSERT_TRUE(rsslEncodeVectorSetDefsComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeVectorComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode vector */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearVector(&vector);
		ASSERT_TRUE(rsslDecodeVector(&dIter, &vector) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(vector.flags == RSSL_VTF_HAS_SET_DEFS);

		/* Decode set defs. */
		if (containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&decodeFieldSetDb);
			ASSERT_TRUE(rsslDecodeLocalFieldSetDefDb(&dIter, &decodeFieldSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].count == 1);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].fieldId == 5);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_DATETIME_7);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&decodeElemSetDb);
			ASSERT_TRUE(rsslDecodeLocalElementSetDefDb(&dIter, &decodeElemSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].count == 1);
			ASSERT_TRUE(rsslBufferIsEqual(&decodeElemSetDb.definitions[0].pEntries[0].name, &elemSetName));
			ASSERT_TRUE(decodeElemSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_TIME_3);
		}

		/*** Test series rollback -- get the size for just the series header, then
		 * try to encode the header and set defs into a buffer of that size (this will give us 
		 * RSSL_RET_BUFFER_TOO_SMALL)  ***/

		buf.length = seriesHeaderEstimateLength;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode series. */
		rsslClearSeries(&series);
		series.flags = RSSL_SRF_HAS_SET_DEFS;
		series.containerType = containerType;
		ASSERT_TRUE(rsslEncodeSeriesInit(&eIter, &series, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should run out of space. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_BUFFER_TOO_SMALL);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_BUFFER_TOO_SMALL);

		/* Rollback */
		ASSERT_TRUE(rsslEncodeSeriesSetDefsComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeSeriesComplete(&eIter, RSSL_FALSE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslGetEncodedBufferLength(&eIter) == 0);

		/* Test that we can then encode something (in this case, just an empty series). */
		rsslClearSeries(&series);
		series.flags = RSSL_SRF_NONE;
		series.containerType = containerType;
		ASSERT_TRUE(rsslEncodeSeriesInit(&eIter, &series, 0, 0) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeSeriesComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode series */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearSeries(&series);
		ASSERT_TRUE(rsslDecodeSeries(&dIter, &series) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(series.flags == RSSL_SRF_NONE);


		/*** Test a successful series encoding. ***/

		buf.length = bufCapacity;
		rsslClearEncodeIterator(&eIter);
		rsslSetEncodeIteratorRWFVersion(&eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&eIter, &buf) == RSSL_RET_SUCCESS);

		/* Encode series. */
		rsslClearSeries(&series);
		series.flags = RSSL_SRF_HAS_SET_DEFS;
		series.containerType = containerType;
		ASSERT_TRUE(rsslEncodeSeriesInit(&eIter, &series, 0, 0) == RSSL_RET_SUCCESS);

		/* Encode set def db -- we should succeed now. */
		if (containerType == RSSL_DT_FIELD_LIST)
			ASSERT_TRUE(rsslEncodeLocalFieldSetDefDb(&eIter, &fieldSetDb) == RSSL_RET_SUCCESS);
		else
			ASSERT_TRUE(rsslEncodeLocalElementSetDefDb(&eIter, &elemSetDb) == RSSL_RET_SUCCESS);

		/* Complete */
		ASSERT_TRUE(rsslEncodeSeriesSetDefsComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslEncodeSeriesComplete(&eIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
		buf.length = rsslGetEncodedBufferLength(&eIter);

		/* Decode series */
		rsslClearDecodeIterator(&dIter);
		rsslSetDecodeIteratorRWFVersion(&dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&dIter, &buf) == RSSL_RET_SUCCESS);

		rsslClearSeries(&series);
		ASSERT_TRUE(rsslDecodeSeries(&dIter, &series) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(series.flags == RSSL_SRF_HAS_SET_DEFS);

		/* Decode set defs. */
		if (containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&decodeFieldSetDb);
			ASSERT_TRUE(rsslDecodeLocalFieldSetDefDb(&dIter, &decodeFieldSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].count == 1);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].fieldId == 5);
			ASSERT_TRUE(decodeFieldSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_DATETIME_7);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&decodeElemSetDb);
			ASSERT_TRUE(rsslDecodeLocalElementSetDefDb(&dIter, &decodeElemSetDb) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].setId == 0);
			ASSERT_TRUE(decodeElemSetDb.definitions[0].count == 1);
			ASSERT_TRUE(rsslBufferIsEqual(&decodeElemSetDb.definitions[0].pEntries[0].name, &elemSetName));
			ASSERT_TRUE(decodeElemSetDb.definitions[0].pEntries[0].dataType == RSSL_DT_TIME_3);
		}
	}
}



TEST(elementListEncDecTest,elementListEncDecTest)
{
    /* This test was copied from the fieldList test. Make sure any updates are propogated. */

	/* TODO float? double? QoS? */
	char testExpBuf[128];

	RsslElementList container = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry entry;

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags;
	RsslUInt32 flagsSize, iiFlags;
	RsslUInt32 iiEntry;

	RsslUInt32 flagsBase[] = 
	{ 
		RSSL_ELF_HAS_STANDARD_DATA,
		RSSL_ELF_HAS_ELEMENT_LIST_INFO
	};

	RsslInt64 payloadVal = 0, decodePayloadVal = 0, expectedPayloadVal = 0;
	RsslUInt8 useElementEntryInitComplete = 0;

	// Encode and decode one of each primitive type.
	// These arrays for fids and dataTypes make up our "Dictionary"

	RsslBuffer names[] =
	{
	  { sizeof("INT"), const_cast<char*>("INT") },
	  { sizeof("DOUBLE"), const_cast<char*>("DOUBLE") },
	  { sizeof("REAL"), const_cast<char*>("REAL") },
	  { sizeof("DATE"), const_cast<char*>("DATE") },
	  { sizeof("TIME"), const_cast<char*>("TIME") },
	  { sizeof("DATETIME"), const_cast<char*>("DATETIME") },
	  { sizeof("ARRAY"), const_cast<char*>("ARRAY") },
	  { sizeof("UINT"), const_cast<char*>("UINT") },
	};

	RsslUInt8 dataTypes[] = 
	{
		RSSL_DT_INT,
		RSSL_DT_DOUBLE,
		RSSL_DT_REAL,
		RSSL_DT_DATE,
		RSSL_DT_TIME,
		RSSL_DT_DATETIME,
		RSSL_DT_ARRAY,
		RSSL_DT_UINT
	};



	RsslInt64 paylInt = -2049, decInt;
	RsslUInt64 paylUInt = 2049, decUInt;
	RsslDouble paylFloat = 0xFF, decFloat;
	RsslReal paylReal = {0, 1, 0xFFFFF}, decReal;
	RsslDate paylDate = {8, 3, 1892}, decDate;
	RsslTime paylTime = { 23, 59, 59, 999, 999, 999 }, decTime;
	RsslDateTime paylDateTime = { {8, 3, 1892}, {23, 59, 59, 999, 999, 999} }, decDateTime;
	RsslBuffer decBuf;
	const RsslUInt64 arrayUInt = 0xDEEEDEEE; /* 4 byte length limit in test */
	char preEncodeArrayBuf[256];
	RsslBuffer preEncodeArrayRBuf = { 256, preEncodeArrayBuf };

	RsslArray array;

	/* Pre-encode array */
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &preEncodeArrayRBuf);
	array.primitiveType = RSSL_DT_UINT;
	array.itemLength = 4;
	ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
	preEncodeArrayRBuf.length = rsslGetEncodedBufferLength(&encIter);

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = sizeof(dataTypes)/sizeof(RsslUInt8);

	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	for (iiMaxEntries = 0; iiMaxEntries <= maxEntries; ++iiMaxEntries)
	{
		for (useElementEntryInitComplete = 0; useElementEntryInitComplete <= 1; ++useElementEntryInitComplete )
		{
			for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
			{
				//// Encode Tests ////

				// Set up encoder iterator
				rsslClearEncodeIterator(&encIter);
				tbufBig.length = TEST_BIG_BUF_SIZE;
				rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

				// Set up container
				rsslClearElementList(&container);
				container.setId = 0; // No sidding for now
				container.flags = flags[iiFlags];


				if ( flags[iiFlags] & RSSL_ELF_HAS_ELEMENT_LIST_INFO ) /* Indicates presence of Element List Number */
					container.elementListNum = 256;

				// Begin container encoding
				sprintf(testExpBuf, "%u-Elem ElementList(flags=%u): rsslEncodeElementListInit", maxEntries, flags[iiFlags]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementListInit(&encIter, &container, 0, 0));

				if ( flags[iiFlags] & RSSL_ELF_HAS_STANDARD_DATA ) /* Indicates element list has elements */
				{
					// Encode entries
					for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
					{
						rsslClearElementEntry(&entry);
						entry.name = names[iiEntry];
						entry.dataType = dataTypes[iiEntry];

						if (!useElementEntryInitComplete)
						{
							// Begin entry encoding
							switch(dataTypes[iiEntry])
							{
							case RSSL_DT_INT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylInt)); break;
							case RSSL_DT_UINT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylUInt)); break;
							case RSSL_DT_DOUBLE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylFloat)); break;
							case RSSL_DT_REAL:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylReal)); break;
							case RSSL_DT_DATE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylDate)); break;
							case RSSL_DT_TIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylTime)); break;
							case RSSL_DT_DATETIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, &paylDateTime)); break;
							case RSSL_DT_ARRAY:
								entry.encData = preEncodeArrayRBuf;
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encIter, &entry, 0)); break;
								break;
							default:
    								FAIL() << "Error in elementListEncDecTest()";
							}
						}
						else
						{
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntryInit(&encIter, &entry, 15));
							// Begin entry encoding
							switch(dataTypes[iiEntry])
							{
							case RSSL_DT_INT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeInt(&encIter, &paylInt)); break;
							case RSSL_DT_UINT:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeUInt(&encIter, &paylUInt)); break;
							case RSSL_DT_DOUBLE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDouble(&encIter, &paylFloat)); break;
							case RSSL_DT_REAL:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &paylReal)); break;
							case RSSL_DT_DATE:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, &paylDate)); break;
							case RSSL_DT_TIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &paylTime)); break;
							case RSSL_DT_DATETIME:
								ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &paylDateTime)); break;
							case RSSL_DT_ARRAY:
								array.primitiveType = RSSL_DT_UINT;
								array.itemLength = 4;
								ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
								ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
								break;
							default:
								FAIL() << "Error in elementListEncDecTest()";
							}

							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntryComplete(&encIter, RSSL_TRUE));
						}
					}
				}

				// Finish container encoding
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementListComplete(&encIter, RSSL_TRUE)); // assume success for now.

				//// End Encode Tests ////

				//// Decode Tests ////

				// Setup container
				rsslClearElementList(&container);
				rsslClearElementEntry(&entry);

				// Begin container decoding
				rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
				ASSERT_TRUE(RSSL_RET_SUCCESS  == rsslDecodeElementList(&decIter, &container, 0));

				ASSERT_TRUE(container.flags == flags[iiFlags]);

				if ( flags[iiFlags] & RSSL_ELF_HAS_ELEMENT_LIST_INFO ) /* Indicates presence of Dict ID and Element List Number */
				{
					ASSERT_TRUE(container.elementListNum == 256);
				}

				if ( rsslElementListCheckHasStandardData(&container))
				{
					// Decode entries
					for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
					{
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementEntry(&decIter, &entry));

						++expectedPayloadVal;

						ASSERT_TRUE(rsslBufferIsEqual(&entry.name, &names[iiEntry]) && entry.dataType == dataTypes[iiEntry]);

						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							decInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(&decIter, &decInt)
								&& decInt == paylInt);
							break;
						case RSSL_DT_UINT:
							decUInt = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &decUInt)
								&& decUInt == paylUInt);
							break;
						case RSSL_DT_DOUBLE:
							decFloat = 0.0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(&decIter, &decFloat)
								&& decFloat == paylFloat); /* not rounded inside encoding/decoding, so this should match exactly */
							break;
						case RSSL_DT_REAL:
							decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
								&& decReal.isBlank == paylReal.isBlank
								&& (decReal.isBlank == RSSL_TRUE 
								|| decReal.hint == paylReal.hint
								&& decReal.value == paylReal.value));
							break;
						case RSSL_DT_DATE:
							decDate.day = 0; decDate.month = 0; decDate.year = 0;
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(&decIter, &decDate)
								&& decDate.day == paylDate.day
								&& decDate.month == paylDate.month
								&& decDate.year == paylDate.year);
							break;
						case RSSL_DT_TIME:
							decTime.hour = 0; decTime.minute = 0; decTime.second = 0; decTime.millisecond = 0; 
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(&decIter, &decTime)
								&& decTime.hour == paylTime.hour
								&& decTime.minute == paylTime.minute
								&& decTime.second == paylTime.second
								&& decTime.millisecond == paylTime.millisecond);

							break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(&decIter, &decDateTime)
								&& decDateTime.date.day == paylDateTime.date.day
								&& decDateTime.date.month == paylDateTime.date.month
								&& decDateTime.date.year == paylDateTime.date.year
								&& decDateTime.time.hour == paylDateTime.time.hour
								&& decDateTime.time.minute == paylDateTime.time.minute
								&& decDateTime.time.second == paylDateTime.time.second
								&& decDateTime.time.millisecond == paylDateTime.time.millisecond);
							break;
						case RSSL_DT_ARRAY:
							ASSERT_TRUE(rsslDecodeArray(&decIter, &array) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(array.primitiveType == RSSL_DT_UINT && array.itemLength == 4);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
							ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
							ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_END_OF_CONTAINER);
							break;
							
						default:
							FAIL() << "Error in elementListEncDecTest()";
							break;
						}
					}
				}
				ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeElementEntry(&decIter, &entry));

			}
		}
	}
		//// End Decode Tests ////

	free(flags);

}


TEST(elementListSetEncDecTest,elementListSetEncDecTest)
{
    /* This test was copied from the fieldList test. Make sure any updates are propogated. */

	/* TODO float? double? QoS? */
	char testExpBuf[128];

	RsslElementList container = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry entry;

	RsslUInt32 maxEntries, iiMaxEntries;
	RsslUInt32 *flags;
	RsslUInt32 flagsSize, iiFlags;
	RsslUInt32 iiEntry;
	RsslBuffer version = {4, const_cast<char*>("1.00")};

	RsslUInt32 flagsBase[] = 
	{ 
		RSSL_ELF_HAS_ELEMENT_LIST_INFO
	};

	RsslInt64 payloadVal = 0, decodePayloadVal = 0, expectedPayloadVal = 0;
	RsslUInt8 useElementEntryInitComplete = 0;

	// Encode and decode one of each primitive type.
	// These arrays for fids and dataTypes make up our "Dictionary"

	RsslBuffer names[] =
	{
	  { 3, const_cast<char*>("INT") },
	  { 6, const_cast<char*>("DOUBLE") },
	  { 4, const_cast<char*>("REAL") },
	  { 4, const_cast<char*>("DATE") },
	  { 4, const_cast<char*>("TIME") },
	  { 8, const_cast<char*>("DATETIME") },
	  { 5, const_cast<char*>("ARRAY") },
	  { 4, const_cast<char*>("UINT") },
	};

	RsslUInt8 dataTypes[] = 
	{
		RSSL_DT_INT,
		RSSL_DT_DOUBLE,
		RSSL_DT_REAL,
		RSSL_DT_DATE,
		RSSL_DT_TIME,
		RSSL_DT_DATETIME,
		RSSL_DT_ARRAY,
		RSSL_DT_UINT
	};



	RsslInt64 paylInt = -2049, decInt;
	RsslUInt64 paylUInt = 2049, decUInt;
	RsslDouble paylFloat = 0xFF, decFloat;
	RsslReal paylReal = {0, 1, 0xFFFFF}, decReal;
	RsslDate paylDate = {8, 3, 1892}, decDate;
	RsslTime paylTime = { 23, 59, 59, 999, 999, 999 }, decTime;
	RsslDateTime paylDateTime = { {8, 3, 1892}, {23, 59, 59, 999, 999, 999} }, decDateTime;
	RsslBuffer decBuf;
	const RsslUInt64 arrayUInt = 0xDEEEDEEE; /* 4 byte length limit in test */
	char preEncodeArrayBuf[256];
	RsslBuffer preEncodeArrayRBuf = { 256, preEncodeArrayBuf };

	RsslArray array;

	RsslRet ret;

	RsslElementSetDefEntry globalElements[8] = 
	{	
	  {{3, const_cast<char*>("INT")}, RSSL_DT_INT},
	  {{6, const_cast<char*>("DOUBLE")}, RSSL_DT_DOUBLE},
	  {{4, const_cast<char*>("REAL")}, RSSL_DT_REAL},
	  {{4, const_cast<char*>("DATE")}, RSSL_DT_DATE},
	  {{4, const_cast<char*>("TIME")}, RSSL_DT_TIME},
	  {{8, const_cast<char*>("DATETIME")}, RSSL_DT_DATETIME},
	  {{5, const_cast<char*>("ARRAY")}, RSSL_DT_ARRAY},
	  {{4, const_cast<char*>("UINT")}, RSSL_DT_UINT}
	};

	RsslElementSetDef globalDef = RSSL_INIT_ELEMENT_SET_DEF;
	RsslElementSetDefDb globalDb = RSSL_INIT_ELEMENT_LIST_SET_DB;

	/* Pre-encode array */
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &preEncodeArrayRBuf);
	array.primitiveType = RSSL_DT_UINT;
	array.itemLength = 4;
	ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
	preEncodeArrayRBuf.length = rsslGetEncodedBufferLength(&encIter);

	flagsSize = _allocateFlagCombinations(&flags, flagsBase, sizeof(flagsBase)/sizeof(RsslUInt32), 0);
	maxEntries = sizeof(dataTypes)/sizeof(RsslUInt8);

	globalDef.setId = 16;
	globalDef.pEntries = globalElements;
	
	// Encode & decode containers of increasing size, until big enough to test all the different forms of elements(e.g. flag & action combinations).
	//   The first container tested will be empty. Then the next will have one element. Then two elements of different form.
	//   The final and largest container should contain an element of each form.
	for (iiMaxEntries = 1; iiMaxEntries <= maxEntries; ++iiMaxEntries)
	{
		for (useElementEntryInitComplete = 0; useElementEntryInitComplete <= 1; ++useElementEntryInitComplete )
		{
			for (iiFlags = 0; iiFlags < flagsSize; ++iiFlags)
			{
				//// Encode Tests ////

				// Set up encoder iterator
				rsslClearEncodeIterator(&encIter);
				tbufBig.length = TEST_BIG_BUF_SIZE;
				rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
		
				// Setup global set def db
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslAllocateElementSetDefDb(&globalDb, &version));
				globalDef.count = iiMaxEntries;

				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslAddElementSetDefToDb(&globalDb, &globalDef));
				ASSERT_TRUE(globalDb.maxSetId == 16);
				ASSERT_TRUE(globalDb.definitions[16] != NULL);
				rsslSetEncodeIteratorGlobalElementListSetDB(&encIter, &globalDb);

				// Set up container
				rsslClearElementList(&container);
				container.setId = 16;
				container.flags = flags[iiFlags];
				container.flags |= RSSL_ELF_HAS_SET_ID | RSSL_ELF_HAS_SET_DATA;

				globalDef.count = iiMaxEntries;

				if ( flags[iiFlags] & RSSL_ELF_HAS_ELEMENT_LIST_INFO ) /* Indicates presence of Element List Number */
					container.elementListNum = 256;

				// Begin container encoding
				sprintf(testExpBuf, "%u-Elem ElementList(flags=%u): rsslEncodeElementListInit", maxEntries, flags[iiFlags]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == (ret = rsslEncodeElementListInit(&encIter, &container, 0, 0)));

				

				for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
				{
					rsslClearElementEntry(&entry);
					entry.name = names[iiEntry];
					entry.dataType = dataTypes[iiEntry];

					if (!useElementEntryInitComplete)
					{
						// Begin entry encoding
						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylInt)); break;
						case RSSL_DT_UINT:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylUInt)); break;
						case RSSL_DT_DOUBLE:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylFloat)); break;
						case RSSL_DT_REAL:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylReal)); break;
						case RSSL_DT_DATE:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylDate)); break;
						case RSSL_DT_TIME:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylTime)); break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, &paylDateTime)); break;
						case RSSL_DT_ARRAY:
							entry.encData = preEncodeArrayRBuf;
							ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntry(&encIter, &entry, 0)); break;
							break;
						default:
							ASSERT_TRUE(0); break;
						}
					}
					else
					{
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntryInit(&encIter, &entry, 15));
						// Begin entry encoding
						switch(dataTypes[iiEntry])
						{
						case RSSL_DT_INT:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeInt(&encIter, &paylInt)); break;
						case RSSL_DT_UINT:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeUInt(&encIter, &paylUInt)); break;
						case RSSL_DT_DOUBLE:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDouble(&encIter, &paylFloat)); break;
						case RSSL_DT_REAL:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeReal(&encIter, &paylReal)); break;
						case RSSL_DT_DATE:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDate(&encIter, &paylDate)); break;
						case RSSL_DT_TIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeTime(&encIter, &paylTime)); break;
						case RSSL_DT_DATETIME:
							ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeDateTime(&encIter, &paylDateTime)); break;
						case RSSL_DT_ARRAY:
							array.primitiveType = RSSL_DT_UINT;
							array.itemLength = 4;
							ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &array) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &arrayUInt) == RSSL_RET_SUCCESS);
							ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
							break;
						default:
							ASSERT_TRUE(0); break;
						}

						ASSERT_TRUE(((iiEntry == iiMaxEntries-1)? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS) == rsslEncodeElementEntryComplete(&encIter, RSSL_TRUE));
					}
				}

				// Finish container encoding
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeElementListComplete(&encIter, RSSL_TRUE)); // assume success for now.

				//// End Encode Tests ////

				//// Decode Tests ////

				// Setup container
				rsslClearDecodeIterator(&decIter);
				rsslClearElementList(&container);
				rsslClearElementEntry(&entry);

				// Begin container decoding
				rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
				rsslSetDecodeIteratorGlobalElementListSetDB(&decIter, &globalDb);
				ASSERT_TRUE(RSSL_RET_SUCCESS  == rsslDecodeElementList(&decIter, &container, 0));

				ASSERT_EQ(container.flags, flags[iiFlags] | RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID);

				if ( flags[iiFlags] & RSSL_ELF_HAS_ELEMENT_LIST_INFO ) /* Indicates presence of Dict ID and Element List Number */
				{
					ASSERT_TRUE(container.elementListNum == 256);
				}


				// Decode entries
				for(iiEntry = 0; iiEntry < iiMaxEntries; ++iiEntry)
				{
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementEntry(&decIter, &entry));

					++expectedPayloadVal;

					ASSERT_TRUE(rsslBufferIsEqual(&entry.name, &names[iiEntry]) && entry.dataType == dataTypes[iiEntry]);

					switch(dataTypes[iiEntry])
					{
					case RSSL_DT_INT:
						decInt = 0;
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(&decIter, &decInt)
							&& decInt == paylInt);
						break;
					case RSSL_DT_UINT:
						decUInt = 0;
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeUInt(&decIter, &decUInt)
							&& decUInt == paylUInt);
						break;
					case RSSL_DT_DOUBLE:
						decFloat = 0.0;
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(&decIter, &decFloat)
							&& decFloat == paylFloat); /* not rounded inside encoding/decoding, so this should match exactly */
						break;
					case RSSL_DT_REAL:
						decReal.hint = 0; decReal.value = 0; decReal.isBlank = 0;
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
							&& decReal.isBlank == paylReal.isBlank
							&& (decReal.isBlank == RSSL_TRUE 
							|| decReal.hint == paylReal.hint
							&& decReal.value == paylReal.value));
						break;
					case RSSL_DT_DATE:
						decDate.day = 0; decDate.month = 0; decDate.year = 0;
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(&decIter, &decDate)
							&& decDate.day == paylDate.day
							&& decDate.month == paylDate.month
							&& decDate.year == paylDate.year);
						break;
					case RSSL_DT_TIME:
						decTime.hour = 0; decTime.minute = 0; decTime.second = 0; decTime.millisecond = 0; 
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(&decIter, &decTime)
							&& decTime.hour == paylTime.hour
							&& decTime.minute == paylTime.minute
							&& decTime.second == paylTime.second
							&& decTime.millisecond == paylTime.millisecond);
						break;
					case RSSL_DT_DATETIME:
						ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(&decIter, &decDateTime)
							&& decDateTime.date.day == paylDateTime.date.day
							&& decDateTime.date.month == paylDateTime.date.month
							&& decDateTime.date.year == paylDateTime.date.year
							&& decDateTime.time.hour == paylDateTime.time.hour
							&& decDateTime.time.minute == paylDateTime.time.minute
							&& decDateTime.time.second == paylDateTime.time.second
							&& decDateTime.time.millisecond == paylDateTime.time.millisecond);
						break;
					case RSSL_DT_ARRAY:
						ASSERT_TRUE(rsslDecodeArray(&decIter, &array) == RSSL_RET_SUCCESS);
						ASSERT_TRUE(array.primitiveType == RSSL_DT_UINT && array.itemLength == 4);
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
						ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
						ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_SUCCESS); decUInt = 0;
						ASSERT_TRUE(rsslDecodeUInt(&decIter, &decUInt) == RSSL_RET_SUCCESS); ASSERT_TRUE(decUInt == arrayUInt);
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &decBuf) == RSSL_RET_END_OF_CONTAINER);
						break;
							
					default:
						ASSERT_TRUE(0);
						break;
					}
				}

				ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeElementEntry(&decIter, &entry));

				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDeleteElementSetDefDb(&globalDb));

			}
		}
	}
		//// End Decode Tests ////

	free(flags);
}

/***** nestedEncDecTest() *****/

RsslUInt8 nestedTypes[] = { RSSL_DT_MAP, RSSL_DT_VECTOR, RSSL_DT_SERIES, RSSL_DT_FILTER_LIST, RSSL_DT_NO_DATA, RSSL_DT_FIELD_LIST }; // this can only be put in field or element lists since its a primitive, RSSL_DT_ARRAY };
RsslUInt8 nestedTypesSize = sizeof(nestedTypes)/sizeof(RsslUInt8);
#define TEST_NESTED_TYPES_HAS_NO_DATA 1 /* Flag for FieldList -- lets the FieldList part of the nested decoder know there will be one less entry(FieldList can't encode NO_DATA) */

/* FIDs for nesting field list */
/* They translate to the nestedTypes[] listed above */
#define TEST_PRIMITIVE_FLF_FID 12  /* Represents the base field list of primitives -- used to differentiate from the FieldList that contains other containers */
RsslUInt8 fieldListNestingFids[] =
{
	7,
	8,
	9,
	10,
	11,
	12 // Primitive Field List
};



enum {
	TEST_DECODE_ONE_ITER, /* Decode nested structure using a single iterator */
	TEST_DECODE_EACH_LEVEL_ITER /* Decode nested structure by creating an iterator to decode each entry payload */
};

void _decodeNestedStructureEmpty( RsslDecodeIterator *pDecIter, RsslUInt8 containerType, RsslUInt8 depth )
{
	RsslFieldList fList;
	RsslFilterList fiList;
	RsslVector vec;
	RsslMap map;
	RsslSeries series;

	if (depth == 0)
	{
		ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(pDecIter, &fList, 0));
	}
	else
	{
		switch(containerType)
		{
		case RSSL_DT_FIELD_LIST: ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(pDecIter, &fList, 0)); break;
		case RSSL_DT_FILTER_LIST: ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFilterList(pDecIter, &fiList)); break;
		case RSSL_DT_MAP: ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeMap(pDecIter, &map)); break;
		case RSSL_DT_VECTOR: ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeVector(pDecIter, &vec)); break;
		case RSSL_DT_SERIES: ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeSeries(pDecIter, &series)); break;
		case RSSL_DT_NO_DATA: break;
		default:
			ASSERT_TRUE(0);
			break;
		}
	}
}
void _decodeNestedStructure( RsslDecodeIterator *pDecIter, RsslUInt8 containerType, RsslUInt8 depth )
{

	if (depth == 0)
	{
		/* bottom level -- should be field list */
		_decodeFieldList(pDecIter);
	}
	else
	{
		/* non-bottom level -- recursively encode series, vector, and field list */

		/* Each container (except for filter list) is homogenous. So we cannot test the different types in the same container.
		 * Each container will therefore contain containers of its own type, each of which contains the other types.
		 * For example, a map when depth == 1 will expand to:
		 *
		 *       ->  Map  -> Vector -> FieldList
		 *  Map  ->  Map  -> Series -> FieldList
		 *       ->  Map  -> Map    -> FieldList
		 * This means that the structure is (depth*2+1) levels deep.  The max encoding depth is 16, so depth should be <= 7.
		 */

		switch(containerType)
		{
			case RSSL_DT_FIELD_LIST:
			{

				/* Unlike the other containers, fieldLists need not be homogenous.
				 * So the fieldList will directly contain the different types, rather than containing child filterLists and having
				 * them contain the children.
				 */

				RsslFieldList container = RSSL_INIT_FIELD_LIST;
				RsslFieldEntry entry;

				RsslUInt8 ii;


				// Setup container
				rsslClearFieldList(&container);
				

				// Begin container decoding
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(pDecIter, &container, 0));

				ASSERT_TRUE(container.flags == RSSL_FLF_HAS_STANDARD_DATA);

				// Decode entries
				for(ii = 0; ii < nestedTypesSize ; ++ii)
				{
					/* NO_DATA not valid for FieldList */
					if ( nestedTypes[ii] == RSSL_DT_NO_DATA )
						continue;

					rsslClearFieldEntry(&entry);
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(pDecIter, &entry));
								
					ASSERT_TRUE(entry.fieldId == (depth == 1 ? TEST_PRIMITIVE_FLF_FID : fieldListNestingFids[ii]) && entry.dataType == RSSL_DT_UNKNOWN);
					_decodeNestedStructure(pDecIter, nestedTypes[ii], depth - 1);


				}

				ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(pDecIter, &entry));


				break;
			}
			case RSSL_DT_FILTER_LIST:
			{
				/* Unlike the other containers, filterLists need not be homogenous.
				 * So the filterList will directly contain the different types, rather than containing child filterLists and having
				 * them contain the children.
				 */

				RsslFilterList filterList = RSSL_INIT_FILTER_LIST;
				RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;

				RsslUInt8 ii;

				/* Decode top filter list */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeFilterList(pDecIter, &filterList));
				ASSERT_TRUE( filterList.flags == 0);

				for( ii = 0; ii < nestedTypesSize; ++ii )
				{
					/* Decode entries */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFilterEntry(pDecIter, &filterEntry));

					/* FilterList default container type will be Map*/
					ASSERT_TRUE( filterEntry.flags ==  RSSL_FTEF_HAS_CONTAINER_TYPE
							&& filterEntry.action == RSSL_FTEA_SET_ENTRY
							&& filterEntry.id == ii
							&& (nestedTypes[ii] != RSSL_DT_MAP || depth == 1 || filterEntry.containerType == nestedTypes[ii]));

						/* decode nested container */
						if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
						{
							_decodeNestedStructure(pDecIter, nestedTypes[ii], depth - 1);
						}
				}

				ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeFilterEntry(pDecIter, &filterEntry));

				 break;
			}
			case RSSL_DT_MAP:
			{
				RsslMap containerMap = RSSL_INIT_MAP;
				RsslInt64 containerMapKey;
				RsslMapEntry containerMapEntry = RSSL_INIT_MAP_ENTRY;

				RsslMap map = RSSL_INIT_MAP;
				RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
				RsslInt64 mapKey;

				RsslUInt8 ii;

				/* Decode top map */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeMap(pDecIter, &containerMap));
				ASSERT_TRUE( containerMap.flags == 0
				&& containerMap.keyPrimitiveType == RSSL_DT_INT
				&& containerMap.containerType == RSSL_DT_MAP);



				for( ii = 0; ii < nestedTypesSize; ++ii )
				{
					/* Decode top map entry to get inner map*/
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeMapEntry(pDecIter, &containerMapEntry, &containerMapKey));
					
					ASSERT_TRUE( containerMapEntry.flags == 0
						&& containerMapEntry.action == RSSL_MPEA_ADD_ENTRY
						&& containerMapKey == 2*ii+1);

					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeMap(pDecIter, &map));

				
					ASSERT_TRUE( map.flags == 0
						&& map.keyPrimitiveType == RSSL_DT_INT
						&& map.containerType == (nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : (((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]))));

					/* Decode inner map entry to get nested container */

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMapEntry(pDecIter, &mapEntry, &mapKey));

					ASSERT_TRUE( mapEntry.flags == 0
					&& mapEntry.action == RSSL_MPEA_ADD_ENTRY
					&& mapKey == 2*ii+2);
					
					if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
					{
						_decodeNestedStructure(pDecIter, nestedTypes[ii], depth - 1);
					}

					/* Decode empty map entry */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMapEntry(pDecIter, &mapEntry, &mapKey));
					ASSERT_TRUE(mapEntry.flags == 0
						&& mapEntry.action == RSSL_MPEA_DELETE_ENTRY
						&& mapKey == 2*ii+3);
					_decodeNestedStructureEmpty(pDecIter, nestedTypes[ii], depth - 1);

					ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeMapEntry(pDecIter, &mapEntry, 0));

				}
					
				ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeMapEntry(pDecIter, &containerMapEntry, 0));
				
				break;
			}

			case RSSL_DT_VECTOR:
			{
				RsslVector containerVector = RSSL_INIT_VECTOR;
				RsslVectorEntry containerVectorEntry = RSSL_INIT_VECTOR_ENTRY;

				RsslVector vector = RSSL_INIT_VECTOR;
				RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;

				RsslUInt8 ii;

				/* Decode top vector */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeVector(pDecIter, &containerVector));
				ASSERT_TRUE( containerVector.flags == 0
				&& containerVector.containerType == RSSL_DT_VECTOR);



				for( ii = 0; ii < nestedTypesSize; ++ii )
				{
					/* Decode top vector entry to get inner vector*/
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeVectorEntry(pDecIter, &containerVectorEntry));
					
					ASSERT_TRUE( containerVectorEntry.flags == 0
						&& containerVectorEntry.action == RSSL_VTEA_SET_ENTRY
						&& containerVectorEntry.index == 2*ii+1);

					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeVector(pDecIter, &vector));
				
					ASSERT_TRUE( vector.flags == 0
						&& vector.containerType == (nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : (((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]))));

					/* Decode inner vector entry to get nested container */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeVectorEntry(pDecIter, &vectorEntry));

					ASSERT_TRUE( vectorEntry.flags == 0
						&& vectorEntry.action == RSSL_VTEA_SET_ENTRY
						&& vectorEntry.index == 2*ii+2);

					/* decode nested container */
					if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
					{
						_decodeNestedStructure(pDecIter, nestedTypes[ii], depth - 1);
					}

					ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeVectorEntry(pDecIter, &vectorEntry));
				}
					
				ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeVectorEntry(pDecIter, &containerVectorEntry));

				break;
			}

			case RSSL_DT_SERIES:
			{
				RsslSeries containerSeries = RSSL_INIT_SERIES;
				RsslSeriesEntry containerSeriesEntry = RSSL_INIT_SERIES_ENTRY;

				RsslSeries series = RSSL_INIT_SERIES;
				RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;

				RsslUInt8 ii;

				/* Init containing series */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeSeries(pDecIter, &containerSeries));
				ASSERT_TRUE( containerSeries.flags == 0
				&& containerSeries.containerType == RSSL_DT_SERIES);

				for( ii = 0; ii < nestedTypesSize; ++ii )
				{
					/* Decode top series entry to get inner series*/
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeSeriesEntry(pDecIter, &containerSeriesEntry));

					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeSeries(pDecIter, &series));

					ASSERT_TRUE( series.flags == 0
						&& series.containerType == (nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : (((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]))));

					/* Decode inner vector entry to get nested container */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeSeriesEntry(pDecIter, &seriesEntry));

					/* decode nested container(map) */
					if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
					{
						_decodeNestedStructure(pDecIter, nestedTypes[ii], depth - 1);
					}

					ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeSeriesEntry(pDecIter, &seriesEntry));
				}

				ASSERT_TRUE( RSSL_RET_END_OF_CONTAINER == rsslDecodeSeriesEntry(pDecIter, &containerSeriesEntry));

				break;
			}
			

			default:
			break;

		}
	}
}

void _encodeNestedStructure( RsslEncodeIterator *pEncIter, RsslUInt8 containerType, RsslUInt8 depth )
{

	if (depth == 0)
	{
		/* bottom level -- encode field list only */
		_encodeFieldList(pEncIter);
	}
	else
	{

		/* non-bottom level -- recursively encode series, vector, and field list */

		/* Each container (except for filter list) is homogenous. So we cannot test the different types in the same container.
		 * Each container will therefore contain containers of its own type, each of which contains the other types.
		 * For example, a map when depth == 1 will expand to:
		 *
		 *       ->  Map  -> Vector -> FieldList
		 *  Map  ->  Map  -> Series -> FieldList
		 *       ->  Map  -> Map    -> FieldList
		 * This means that the structure is (depth*2+1) levels deep.  The max encoding depth is 16, so depth should be <= 7.
		 */


		switch(containerType)
		{
		case RSSL_DT_FIELD_LIST:
			{
				/* Unlike the containers Map, Series, and Vector, fieldLists need not be homogenous.
				* So the fieldLists will directly contain the different types, rather than containing child fieldLists and having
				* them contain the children.
			  */

				RsslFieldList fieldList;
				RsslFieldEntry entry;
				RsslUInt8 ii;

				rsslClearFieldList(&fieldList);

				fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

				/* init */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(pEncIter, &fieldList, 0, 0));

				/* add entries */
				for(ii = 0; ii < nestedTypesSize ; ++ii)
				{					
					/* NO_DATA not valid for FieldList */
					if ( nestedTypes[ii] == RSSL_DT_NO_DATA )
						continue;

					rsslClearFieldEntry(&entry);
					entry.fieldId = (depth == 1) ? TEST_PRIMITIVE_FLF_FID : fieldListNestingFids[ii];

					/* Encode entry */
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntryInit(pEncIter, &entry, 0));

					/* Encode nested structure */
					_encodeNestedStructure(pEncIter, nestedTypes[ii], depth - 1);
					/* Finish encoding entry */
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntryComplete(pEncIter, RSSL_TRUE));
					
				}

			/* finish encoding */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(pEncIter, RSSL_TRUE));

			break;
		}
		case RSSL_DT_FILTER_LIST:
		{
			/* Unlike the containers Map, Series, and Vector, filterLists need not be homogenous.
			 * So the filterList will directly contain the different types, rather than containing child filterLists and having
			 * them contain the children.
			 */
			RsslFilterList filterList = RSSL_INIT_FILTER_LIST;
			RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;
			RsslUInt8 ii;


			filterList.flags = 0;
			filterList.containerType = RSSL_DT_MAP; /* Just to provide a default container type */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterListInit(pEncIter, &filterList));
			for( ii = 0; ii < nestedTypesSize; ++ii )
			{
				/* Encode entries */
				filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
				filterEntry.containerType = nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : ((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]);
				filterEntry.action = RSSL_FTEA_SET_ENTRY;
				filterEntry.id = ii;
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterEntryInit(pEncIter, &filterEntry, 0));

				/* Encode nested container */
				if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
				{
					_encodeNestedStructure(pEncIter, nestedTypes[ii], depth - 1);
				}

				/* Finish entry */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterEntryComplete(pEncIter, RSSL_TRUE));
			}

			/* Finish filter list */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFilterListComplete(pEncIter, RSSL_TRUE));

			break;
		}
		case RSSL_DT_MAP:
		{
			RsslMap containerMap = RSSL_INIT_MAP;
			RsslInt64 containerMapKey;
			RsslMapEntry containerMapEntry = RSSL_INIT_MAP_ENTRY;

			RsslMap map = RSSL_INIT_MAP;
			RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
			RsslInt64 mapKey;

			RsslUInt8 ii;

			/* Init containing map */
			containerMap.flags = 0;
			containerMap.keyPrimitiveType = RSSL_DT_INT;
			containerMap.containerType = RSSL_DT_MAP;
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapInit(pEncIter, &containerMap, 0, 0));

			for( ii = 0; ii < nestedTypesSize; ++ii )
			{
				/* Nested map */
				/* Encode top map entry for inner map*/
				rsslClearMapEntry(&containerMapEntry);
				containerMapEntry.flags = 0;
				containerMapEntry.action = RSSL_MPEA_ADD_ENTRY;
				containerMapKey = 2*ii+1;
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapEntryInit(pEncIter, &containerMapEntry, &containerMapKey, 0));

				/* Encode inner map */
				rsslClearMap(&map);
				map.flags = 0;
				map.keyPrimitiveType = RSSL_DT_INT;
				map.containerType = nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : ((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]);

				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapInit(pEncIter, &map, 0, 0));

				/* Encode inner map entry that will contain the nested container */
				rsslClearMapEntry(&mapEntry);
				mapEntry.flags = 0;
				mapEntry.action = RSSL_MPEA_ADD_ENTRY;
				mapKey = 2*ii+2;

				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapEntryInit(pEncIter, &mapEntry, &mapKey, 0));

				/* Encode nested container */
				if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
				{
					_encodeNestedStructure(pEncIter, nestedTypes[ii], depth - 1);
				}

				/* Finish entry for nested container */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapEntryComplete(pEncIter, RSSL_TRUE));

				/* Add an empty entry for this type */
				rsslClearMapEntry(&mapEntry);
				mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
				mapKey += 1;
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapEntry(pEncIter, &mapEntry, &mapKey));

				/* Finish inner map */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapComplete(pEncIter, RSSL_TRUE));

				/* Finish containing map entry */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapEntryComplete(pEncIter, RSSL_TRUE));
			}

			/* Finish containing map */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeMapComplete(pEncIter, RSSL_TRUE));

			break;
		}

		case RSSL_DT_VECTOR:
		{
			RsslVector containerVector = RSSL_INIT_VECTOR;
			RsslUInt32 containerVectorIndex;
			RsslVectorEntry containerVectorEntry;

			RsslVector vector = RSSL_INIT_VECTOR;
			RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
			RsslUInt32 vectorIndex;

			RsslUInt8 ii;

			/* Init containing vector */
			containerVector.flags = 0;
			containerVector.containerType = RSSL_DT_VECTOR;
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorInit(pEncIter, &containerVector, 0, 0));



			for( ii = 0; ii < nestedTypesSize; ++ii )
			{
				/* Encode top vector entry for inner vector*/
				containerVectorEntry.flags = 0;
				containerVectorEntry.action = RSSL_VTEA_SET_ENTRY;
				containerVectorIndex = 2*ii+1;
				containerVectorEntry.index = containerVectorIndex;
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorEntryInit(pEncIter, &containerVectorEntry, 0));

				/* Encode inner vector */
				rsslClearVector(&vector);
				vector.flags = 0;
				vector.containerType = nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : ((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorInit(pEncIter, &vector, 0, 0));

				/* Encode inner vector entry that will contain the nested container */
				rsslClearVectorEntry(&vectorEntry);
				vectorEntry.flags = 0;
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorIndex = (2*ii+1)+1;
				vectorEntry.index = vectorIndex;
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorEntryInit(pEncIter, &vectorEntry, 0));

				/* Encode nested container */
				if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
				{
					_encodeNestedStructure(pEncIter, nestedTypes[ii], depth - 1);
				}

				/* Finish entry for nested container */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorEntryComplete(pEncIter, RSSL_TRUE));

				/* Finish inner vector */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorComplete(pEncIter, RSSL_TRUE));

				/* Finish containing vector entry */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorEntryComplete(pEncIter, RSSL_TRUE));
			}


			/* Finish containing vector */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeVectorComplete(pEncIter, RSSL_TRUE));

			break;
		}

		case RSSL_DT_SERIES:
		{
			RsslSeries containerSeries = RSSL_INIT_SERIES;
			RsslSeriesEntry containerSeriesEntry;

			RsslSeries series = RSSL_INIT_SERIES;
			RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;

			RsslUInt8 ii;

			containerSeries.flags = 0;
			containerSeries.containerType = RSSL_DT_SERIES;

			/* Init containing series */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesInit(pEncIter, &containerSeries, 0, 0));


			for( ii = 0; ii < nestedTypesSize ; ++ii )
			{
				/* Encode top series entry for inner series*/
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesEntryInit(pEncIter, &containerSeriesEntry, 0));

				/* Encode inner series */
				rsslClearSeries(&series);
				series.flags = 0;
				series.containerType = nestedTypes[ii] == RSSL_DT_NO_DATA ? RSSL_DT_NO_DATA : ((depth == 1) ? RSSL_DT_FIELD_LIST : nestedTypes[ii]);
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesInit(pEncIter, &series, 0, 0));

				/* Encode inner series entry that will contain the nested container */
				rsslClearSeriesEntry(&seriesEntry);

				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesEntryInit(pEncIter, &seriesEntry, 0));

				/* Encode nested container */
				if ( nestedTypes[ii] != RSSL_DT_NO_DATA )
				{
					_encodeNestedStructure(pEncIter, nestedTypes[ii], depth - 1);
				}

				/* Finish entry for nested container */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesEntryComplete(pEncIter, RSSL_TRUE));

				/* Finish inner series */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesComplete(pEncIter, RSSL_TRUE));

				/* Finish containing series entry */
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesEntryComplete(pEncIter, RSSL_TRUE));
			}


			/* Finish containing series */
			ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeSeriesComplete(pEncIter, RSSL_TRUE));

			break;
		}
		default:
			break;

		}
	}
}



/* _encodeNestedStructure adds two levels of depth per recursive call.
* Rssl has a 16b maximum payload size,
* so this test can only run with this depth <= 5(or by reducing the number of containers or field list elements).
*/
#define TEST_NESTED_STRUCT_MAX_DEPTH 5
TEST(nestedEncDecTest,nestedEncDecTest)
{
	RsslUInt8 maxDepth;

	rsslClearEncodeIterator(&encIter);
	rsslClearDecodeIterator(&decIter);

	for (maxDepth = 0; maxDepth <= TEST_NESTED_STRUCT_MAX_DEPTH; ++maxDepth)
	{
			/* Test nested structure with field list at top */
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			_encodeNestedStructure(&encIter, RSSL_DT_FIELD_LIST, maxDepth);

			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			_decodeNestedStructure(&decIter, RSSL_DT_FIELD_LIST, maxDepth);

			/* Test nested structure with map at top */
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			_encodeNestedStructure(&encIter, RSSL_DT_MAP, maxDepth);

			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			_decodeNestedStructure(&decIter, RSSL_DT_MAP, maxDepth);

			/* Test nested structure with vector at top */
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			_encodeNestedStructure(&encIter, RSSL_DT_VECTOR, maxDepth);

			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			_decodeNestedStructure(&decIter, RSSL_DT_VECTOR, maxDepth);

			/* Test nested structure with series at top */
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			_encodeNestedStructure(&encIter, RSSL_DT_SERIES, maxDepth);

			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			_decodeNestedStructure(&decIter, RSSL_DT_SERIES, maxDepth);

			/* Test nested structure with filter list at top */
			rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);
			_encodeNestedStructure(&encIter, RSSL_DT_FILTER_LIST, maxDepth);

			rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
			_decodeNestedStructure(&decIter, RSSL_DT_FILTER_LIST, maxDepth);
	}
}

TEST(basicFieldListNestingTest,basicFieldListNestingTest)
{
	RsslBuffer encBuf;
	char buf[500];
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldList nFieldList = RSSL_INIT_FIELD_LIST;
	RsslEncodeIterator encIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDate dDate;
	RsslDate dDate2;
	RsslReal dReal2;
	RsslReal dReal;
	RsslFieldEntry fEnt = RSSL_INIT_FIELD_ENTRY;
	RsslDecodeIterator decIter = RSSL_INIT_DECODE_ITERATOR;

	encBuf.data = buf;
	encBuf.length = 500;

	rsslSetEncodeIteratorBuffer(&encIter, &encBuf);

	fList.setId = 0;
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	
	/* put a date in for good measuer */
	dDate.day = 21;
	dDate.month = 10;
	dDate.year = 1978;

	rsslEncodeFieldListInit(&encIter, &fList, 0, 0);
	fEnt.fieldId = 1;
	fEnt.dataType = RSSL_DT_DATE;

	rsslEncodeFieldEntry(&encIter, &fEnt, &dDate);

	fEnt.fieldId = 2;
	/* now encode nested field list */
	rsslEncodeFieldEntryInit(&encIter, &fEnt, 100);

	nFieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	rsslEncodeFieldListInit(&encIter, &nFieldList, 0, 0);
	dReal.isBlank = 0;
	dReal.hint = 2;
	dReal.value = 69;
	fEnt.fieldId = 3;
	fEnt.dataType = RSSL_DT_REAL;
	rsslEncodeFieldEntry(&encIter, &fEnt, &dReal);
	rsslEncodeFieldListComplete(&encIter, RSSL_TRUE);
	/* complete nested fieldlist */
	rsslEncodeFieldEntryComplete(&encIter, RSSL_TRUE);

	fEnt.fieldId = 4;
	fEnt.dataType = RSSL_DT_DATE;
	rsslEncodeFieldEntry(&encIter, &fEnt, &dDate);
	/* complete outer field list */
	rsslEncodeFieldListComplete(&encIter, RSSL_TRUE);

	encBuf.length = rsslGetEncodedBufferLength(&encIter);
	/* switch into decode mode */

	rsslSetDecodeIteratorBuffer(&decIter, &encBuf);
	rsslDecodeFieldList(&decIter, &fList, 0);
	
	while (rsslDecodeFieldEntry(&decIter, &fEnt) != RSSL_RET_END_OF_CONTAINER)
	{
		if ((fEnt.fieldId == 1) || (fEnt.fieldId == 4))
			rsslDecodeDate(&decIter, &dDate2);

	
		if (fEnt.fieldId == 2)
		{
			/* should be our nested field list */
			rsslDecodeFieldList(&decIter, &nFieldList, 0);

			rsslDecodeFieldEntry(&decIter, &fEnt);

			if (fEnt.fieldId == 3)
				rsslDecodeReal(&decIter, &dReal2);

			while (rsslDecodeFieldEntry(&decIter, &fEnt) != RSSL_RET_END_OF_CONTAINER)
			{

				if (fEnt.fieldId == 3)
					rsslDecodeReal(&decIter, &dReal2);
			}
		}
	}
}

TEST(stringCompareTests,stringCompareTests)
{
    RsslBuffer buf1 = { 0, const_cast<char*>("Test\0garbage") };
	RsslBuffer buf2 = { 0, const_cast<char*>("tEST\0moreGarbage") };
	RsslBuffer buf3 = { 0, const_cast<char*>("tESTer") };

	/* All zero length */
	ASSERT_TRUE( rsslBufferIsEqual(&buf1, &buf2) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf1, &buf3) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf2, &buf3) );
	
	/* Length vs. No length */
	buf1.length = 1;
	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf2) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf3) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf2, &buf1) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf3, &buf1) );

	/* Test vs. teST vs. teST */ 
	buf1.length = 4; buf2.length = 4; buf3.length = 4;
	ASSERT_TRUE( rsslBufferIsEqual(&buf1, &buf1) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf2, &buf2) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf3, &buf3) );

	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf2) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf3) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf2, &buf3) );

	/* Test\0 vs. teST\0 vs. teSTe */
	buf1.length = 5; buf2.length = 5; buf3.length = 5;
	ASSERT_TRUE( rsslBufferIsEqual(&buf1, &buf1) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf2, &buf2) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf3, &buf3) );

	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf2) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf2, &buf1) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf3) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf2, &buf3) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf3, &buf1) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf3, &buf2) );

	/* Test\0g vs. teST\0m vs. teSTer */ 
	buf1.length = 6; buf2.length = 6; buf3.length = 6;
	ASSERT_TRUE( rsslBufferIsEqual(&buf1, &buf1) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf2, &buf2) );
	ASSERT_TRUE( rsslBufferIsEqual(&buf3, &buf3) );

	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf2) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf2, &buf1) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf1, &buf3) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf2, &buf3) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf3, &buf1) );
	ASSERT_TRUE( !rsslBufferIsEqual(&buf3, &buf2) );
}

/***** END nestedEncDecTest() *****/

TEST(realFieldTest,realFieldTest)
{
	/* JimC: Believe this was written to test some things involving
	 * 32-bit and 64-bit reals(making sure they encode and decode properly and
	 * return errors in appropriate cases, most notably when trying to decode
	 * Real64 with the Real32 function). The Reals have been merged now,
	 * so some of that is no longer needed.
	 * Will keep the rest -- it certainly doesn't hurt. */

	/* construct field list 
	 * This field list will be used as every field list in the structure */
		
	RsslFieldList fieldList;
	RsslFieldEntry entry;
	RsslReal real;
	RsslReal decReal;
	RsslReal encReal;
	RsslReal outReal;
	RsslEncodeIterator encIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDecodeIterator decIter = RSSL_INIT_DECODE_ITERATOR;

	rsslClearFieldList(&fieldList);

	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	/* init */
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&encIter, &fieldList, 0, 0));
	rsslClearFieldEntry(&entry);
	entry.fieldId = 1;
	entry.dataType = RSSL_DT_REAL;

	/* 32-bit 64-bit real */
	real.hint = RSSL_RH_EXPONENT_2;
	real.isBlank = 0;
	real.value = 65535;

	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &real));

	/* 64-bit real */
	rsslClearFieldEntry(&entry);
	entry.fieldId = 2;
	entry.dataType = RSSL_DT_REAL;
	real.hint = RSSL_RH_EXPONENT_2;
	real.isBlank = 0;
	real.value = 68719476735;

	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &real));
	
	rsslClearFieldEntry(&entry);
	entry.fieldId = 3;
	entry.dataType = RSSL_DT_REAL;

	/* 32-bit 64-bit real */
	encReal.hint = RSSL_RH_EXPONENT_2;
	encReal.isBlank = 0;
	encReal.value = 65535;

	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encIter, &entry, &encReal));
	/* finish encoding */
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encIter, RSSL_TRUE));

	tbufBig.length = rsslGetEncodedBufferLength(&encIter);

	// Setup container
	rsslClearFieldList(&fieldList);
	rsslClearFieldEntry(&entry);
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

	// Begin container decoding
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(&decIter, &fieldList, 0));

	ASSERT_TRUE(fieldList.flags == RSSL_FLF_HAS_STANDARD_DATA);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));
						
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
		&& decReal.isBlank == 0
		&& decReal.hint == RSSL_RH_EXPONENT_2
		&& decReal.value == 65535);

	rsslClearFieldEntry(&entry);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));

	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &outReal)
		&& outReal.isBlank == 0
		&& outReal.hint == RSSL_RH_EXPONENT_2
		&& outReal.value == 68719476735);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &entry));

	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(&decIter, &decReal)
			&& decReal.isBlank == 0
			&& decReal.hint == RSSL_RH_EXPONENT_2
			&& decReal.value == 65535);

	tbufBig.length = TEST_BIG_BUF_SIZE;

}

TEST(bufferRealignTest,bufferRealignTest)
{
	char *buf;
	RsslBuffer rBuf[2];
	RsslUInt32 bufLen = 64;
	int whichBuf = 0;

	RsslFieldList fList; 
	RsslFieldEntry field;
	RsslEncodeIterator eIter;
	const RsslUInt64 uint = 0xfaabd00ddeeedeee;

	RsslInt32 fid; 
	RsslUInt32 bufferFailures;
	RsslRet ret;

	RsslDecodeIterator dIter;

	buf = (char*)malloc(bufLen);

	rBuf[0].data = buf;
	rBuf[0].length = bufLen;

	rsslClearEncodeIterator(&eIter);
	rsslSetEncodeIteratorBuffer(&eIter, &rBuf[0]);

	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&eIter, &fList, 0, 0));

	/* Encode and decode a long list of fields, recovering whenever the buffer runs out of room. */
	bufferFailures = 0;
	for(fid = -256; fid < 256; ++fid)
	{
		rsslClearFieldEntry(&field);
		field.dataType = RSSL_DT_INT;
		field.fieldId = fid;
		ret = rsslEncodeFieldEntry(&eIter, &field, &uint);

		ASSERT_TRUE(ret == RSSL_RET_SUCCESS || ret == RSSL_RET_BUFFER_TOO_SMALL);
		
		if (ret == RSSL_RET_BUFFER_TOO_SMALL)
		{
			RsslBuffer *oldBuf = whichBuf ? &rBuf[1] : &rBuf[0];
			RsslBuffer *newBuf = whichBuf ? &rBuf[0] : &rBuf[1];
			++bufferFailures;
			bufLen *= 2;
			buf = newBuf->data = (char*)malloc(bufLen);
			newBuf->length = bufLen;

			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslRealignEncodeIteratorBuffer(&eIter, newBuf));
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&eIter, &field, &uint));

			whichBuf = (whichBuf == 1) ? 0 : 1;
			free(oldBuf->data);
		}
		
	}

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&eIter, RSSL_TRUE));
	//printf("	Info: Buffer was realigned %i times.", bufferFailures);

	rsslClearDecodeIterator(&dIter); rsslSetDecodeIteratorBuffer(&dIter, &rBuf[whichBuf ? 1 : 0]);
	rsslDecodeFieldList(&dIter, &fList, 0);
	for (fid = -256; fid < 256; ++fid)
	{
		RsslUInt64 oVal = -5;
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&dIter, &field));
		ASSERT_TRUE(field.fieldId == fid);
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeUInt(&dIter, &oVal));
		ASSERT_TRUE(oVal == 0xfaabd00ddeeedeee);
	}
	ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(&dIter, &field));

	free(buf);
}

TEST(entrySkippingTest,entrySkippingTest)
{
	RsslMap map; RsslMapEntry mapEntry;
	RsslFieldList fieldList; RsslFieldEntry fieldEntry;
	int iMapEntries, iFields, iSkipField;

	/* Some values to use - total 4 mapEntries with 4 fields in each entry. */
	const int totalMapEntries = 4;
	const int totalFields = 4;
	RsslBuffer mapKeyStrings[] = 
	  {{sizeof("GROUCHO"),const_cast<char*>("GROUCHO")},
	   {sizeof("ZEPPO"),  const_cast<char*>("ZEPPO")},
	   {sizeof("HARPO"),  const_cast<char*>("HARPO")},
	   {sizeof("GUMMO"),  const_cast<char*>("GUMMO")}};
	RsslFieldId fids[] = { 22, 26, 30, 31 };
	RsslReal reals[] = { {0, RSSL_RH_EXPONENT_2, 3194}, {0, RSSL_RH_EXPONENT_2, 3196}, {0, RSSL_RH_EXPONENT_2, 4}, {0, RSSL_RH_EXPONENT_2, 5}};

	tbufBig.length = TEST_BIG_BUF_SIZE;

	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	/* Encode a map with a field list in each entry */

	/* Map */
	rsslClearMap(&map);
	map.flags = RSSL_MPF_NONE;
	map.keyPrimitiveType = RSSL_DT_ASCII_STRING;
	map.containerType = RSSL_DT_FIELD_LIST;
	ASSERT_TRUE(rsslEncodeMapInit(&encIter, &map, 0, 0) == RSSL_RET_SUCCESS);

	/* MapEntry */
	for(iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
	{
		rsslClearMapEntry(&mapEntry);
		mapEntry.flags = RSSL_MPEF_NONE;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		ASSERT_TRUE(rsslEncodeMapEntryInit(&encIter, &mapEntry, &mapKeyStrings[iMapEntries], 0) == RSSL_RET_SUCCESS);

		/* FieldList */
		rsslClearFieldList(&fieldList);
		fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
		ASSERT_TRUE(rsslEncodeFieldListInit(&encIter, &fieldList, 0, 0) == RSSL_RET_SUCCESS);
		for (iFields = 0; iFields < totalFields; ++iFields)
		{
			rsslClearFieldEntry(&fieldEntry);
			fieldEntry.fieldId = fids[iFields]; fieldEntry.dataType = RSSL_DT_REAL; 
			ASSERT_TRUE(rsslEncodeFieldEntry(&encIter, &fieldEntry, &reals[iFields]) == RSSL_RET_SUCCESS);
		}
		ASSERT_TRUE(rsslEncodeFieldListComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

		ASSERT_TRUE(rsslEncodeMapEntryComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
	}

	ASSERT_TRUE(rsslEncodeMapComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	tbufBig.length = rsslGetEncodedBufferLength(&encIter);


	/* Decode all map entries, skipping at various points */
	for (iSkipField = 0; iSkipField <= totalFields /* Includes decoding all fields before 'skipping' */; ++iSkipField)
	{
    	RsslReal real;
		RsslBuffer mapKey;
		rsslClearDecodeIterator(&decIter);
		rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

		/* Decode map */
		rsslClearMap(&map);
		ASSERT_TRUE(rsslDecodeMap(&decIter, &map) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(map.flags == RSSL_MPF_NONE);
		ASSERT_TRUE(map.keyPrimitiveType == RSSL_DT_ASCII_STRING);

		for (iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
		{
			/* Decode mapEntry */
			rsslClearMapEntry(&mapEntry);
			ASSERT_TRUE(rsslDecodeMapEntry(&decIter, &mapEntry, &mapKey) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(rsslBufferIsEqual(&mapKey, &mapKeyStrings[iMapEntries]));

    		rsslClearFieldList(&fieldList);
    		ASSERT_TRUE(rsslDecodeFieldList(&decIter, &fieldList, 0) == RSSL_RET_SUCCESS);

			for (iFields = 0; iFields < iSkipField; ++iFields)
			{
				/* Decode fieldEntry and check value. */
				rsslClearFieldEntry(&fieldEntry);
				ASSERT_TRUE(rsslDecodeFieldEntry(&decIter, &fieldEntry) == RSSL_RET_SUCCESS);
				ASSERT_TRUE(fieldEntry.fieldId == fids[iFields]);
				ASSERT_TRUE(rsslDecodeReal(&decIter, &real) == RSSL_RET_SUCCESS);

				ASSERT_TRUE(real.hint == reals[iFields].hint);
				ASSERT_TRUE(real.isBlank == reals[iFields].isBlank);
				ASSERT_TRUE(real.value == reals[iFields].value);
			}
			ASSERT_TRUE(rsslFinishDecodeEntries(&decIter) == RSSL_RET_END_OF_CONTAINER);
		}

	}

	tbufBig.length = TEST_BIG_BUF_SIZE;
}

TEST(stringConversionTest,stringConversionTest)
{
	RsslBuffer testDataBuf, testStrBuf;
	char testData[128], testString[128], testStringSmall[1], textData[] = "test state text";
	RsslInt testInt, testIntOut;
	RsslUInt testUInt, testUIntOut;
	RsslFloat testFloat, testFloatOut;
	RsslDouble testDouble, testDoubleOut;
	RsslReal testReal, testRealOut;
	RsslDate testDate, testDateOut;
	RsslTime testTime, testTimeOut;
	RsslDateTime testDateTime, testDateTimeOut;
	RsslQos testQos;
	RsslState testState;
	RsslEnum testEnum, testEnumOut;
	RsslBuffer testBuffer = {sizeof("Refinitiv"),  const_cast<char*>("Refinitiv")};

	/* Int conversion test */
	testInt = 987654321;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_INT, &testInt) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_INT, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_INT, &testStrBuf) == RSSL_RET_SUCCESS);
	testIntOut = atoi(testStrBuf.data);
	ASSERT_TRUE(testInt == testIntOut);

	/* UInt conversion test */
	testUInt = 1234567891;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_UINT, &testUInt) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_UINT, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_UINT, &testStrBuf) == RSSL_RET_SUCCESS);
	testUIntOut = atol(testStrBuf.data);
	ASSERT_TRUE(testUInt == testUIntOut);

	/* Float conversion test */
	testFloat = (RsslFloat)123.456;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_FLOAT, &testFloat) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_FLOAT, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_FLOAT, &testStrBuf) == RSSL_RET_SUCCESS);
	testFloatOut = (RsslFloat)atof(testStrBuf.data);
	ASSERT_TRUE(testFloat == testFloatOut);

	/* Double conversion test */
	testDouble = 98765.4321;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_DOUBLE, &testDouble) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DOUBLE, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DOUBLE, &testStrBuf) == RSSL_RET_SUCCESS);
	testDoubleOut = atof(testStrBuf.data);
	ASSERT_TRUE(testDouble == testDoubleOut);
	//printf("testDouble: %f   testDoubleOut:  %f", testDouble, testDoubleOut); 


	/* Double conversion test */
	testDouble = 11.11;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_DOUBLE, &testDouble) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DOUBLE, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DOUBLE, &testStrBuf) == RSSL_RET_SUCCESS);
	testDoubleOut = atof(testStrBuf.data);
	ASSERT_TRUE(testDouble == testDoubleOut);
	//printf("testDouble: %f   testDoubleOut:  %f", testDouble, testDoubleOut); 

	/* Real conversion test */
	testDouble = 12345.6789012;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, 0) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_REAL, &testReal) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_REAL, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_REAL, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslNumericStringToReal(&testRealOut, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslRealIsEqual(&testReal, &testRealOut) == RSSL_TRUE);

	/* Real string conversion edge cases */
	testStrBuf.length = sprintf(testString, "-9223372036854775808");
	testStrBuf.data = testString;
	ASSERT_TRUE(rsslNumericStringToReal(&testReal, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(testReal.hint == RSSL_RH_EXPONENT0);
	ASSERT_EQ(testReal.value,(-9223372036854775807 -1));

	testStrBuf.length = sprintf(testString, "9223372036854775807");
	testStrBuf.data = testString;
	ASSERT_TRUE(rsslNumericStringToReal(&testReal, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(testReal.hint == RSSL_RH_EXPONENT0);
	ASSERT_TRUE(testReal.value == 9223372036854775807);

	testStrBuf.length = sprintf(testString, "922337203685477.5807");
	testStrBuf.data = testString;
	ASSERT_TRUE(rsslNumericStringToReal(&testReal, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(testReal.hint == RSSL_RH_EXPONENT_4);
	ASSERT_TRUE(testReal.value == 9223372036854775807);

	testStrBuf.length = sprintf(testString, "-922337203685477.5808");
	testStrBuf.data = testString;
	ASSERT_TRUE(rsslNumericStringToReal(&testReal, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(testReal.hint == RSSL_RH_EXPONENT_4);
	ASSERT_EQ(testReal.value,(-9223372036854775807 - 1));

	/* Additional Real conversion tests */
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);

	testReal.isBlank = false;
	testReal.hint = RSSL_RH_EXPONENT7;
	testReal.value = (std::numeric_limits<RsslInt>::max)();

	ASSERT_TRUE( rsslRealToString(&testDataBuf, &testReal) == RSSL_RET_SUCCESS );
	ASSERT_TRUE( rsslNumericStringToReal(&testRealOut, &testDataBuf) == RSSL_RET_SUCCESS );
	ASSERT_TRUE( rsslRealIsEqual(&testReal, &testRealOut) == RSSL_TRUE );

	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	testReal.value = (std::numeric_limits<RsslInt>::min)();

	ASSERT_TRUE( rsslRealToString(&testDataBuf, &testReal) == RSSL_RET_SUCCESS );
	ASSERT_TRUE( rsslNumericStringToReal(&testRealOut, &testDataBuf) == RSSL_RET_SUCCESS );
	ASSERT_TRUE( rsslRealIsEqual(&testReal, &testRealOut) == RSSL_TRUE );

	/* Date conversion test */
	testDate.month = 2;
	testDate.day = 5;
	testDate.year = 1900;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_DATE, &testDate) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DATE, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DATE, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDateStringToDate(&testDateOut, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDateIsEqual(&testDate, &testDateOut) == RSSL_TRUE);

	/* Time conversion test */
	testTime.hour = 1;
	testTime.minute = 2;
	testTime.second = 3;
	testTime.millisecond = 4;
	testTime.microsecond = 5;
	testTime.nanosecond = 6;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_TIME, &testTime) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_TIME, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_TIME, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslTimeStringToTime(&testTimeOut, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslTimeIsEqual(&testTime, &testTimeOut) == RSSL_TRUE);

	/* Date and Time conversion test */
	testDateTime.date.month = 10;
	testDateTime.date.day = 10;
	testDateTime.date.year = 2010;
	testDateTime.time.hour = 1;
	testDateTime.time.minute = 2;
	testDateTime.time.second = 3;
	testDateTime.time.millisecond = 4;
	testDateTime.time.microsecond = 5;
	testDateTime.time.nanosecond = 6;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_DATETIME, &testDateTime) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DATETIME, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_DATETIME, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDateTimeStringToDateTime(&testDateTimeOut, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslDateTimeIsEqual(&testDateTime, &testDateTimeOut) == RSSL_TRUE);

	/* QOS conversion test */
	testQos.dynamic = RSSL_TRUE;
	testQos.rate = RSSL_QOS_TIME_DELAYED;
	testQos.rateInfo = 123;
	testQos.timeInfo = 456;
	testQos.timeliness = RSSL_QOS_RATE_TIME_CONFLATED;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_QOS, &testQos) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_QOS, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_QOS, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(!strcmp(testStrBuf.data, "Qos: DelayedByTimeInfo/ConflatedByRateInfo/Dynamic - timeInfo: 456 - rateInfo: 123"));
	/* no translation function for QOS string back to QOS structure */

	/* State conversion test */
	testState.streamState = RSSL_STREAM_REDIRECTED;
	testState.dataState = RSSL_DATA_SUSPECT;
	testState.code = RSSL_SC_FULL_VIEW_PROVIDED;
	testState.text.data  = textData;
	testState.text.length = sizeof(textData);
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_STATE, &testState) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_STATE, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_STATE, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(!strcmp(testStrBuf.data, "State: Redirected/Suspect/Full view provided - text: \"test state text\""));
	/* no translation function for State string back to State structure */

	/* Enum conversion test */
	testEnum = 9;
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_ENUM, &testEnum) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_ENUM, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_ENUM, &testStrBuf) == RSSL_RET_SUCCESS);
	testEnumOut = atoi(testStrBuf.data);
	ASSERT_TRUE(testEnum == testEnumOut);

	/* Buffer conversion test */
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_BUFFER, &testBuffer) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_BUFFER, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_BUFFER, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslBufferIsEqual(&testBuffer, &testStrBuf) == RSSL_TRUE);

	/* RSSL_DT_ASCII_STRING conversion test */
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_ASCII_STRING, &testBuffer) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_ASCII_STRING, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_ASCII_STRING, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslBufferIsEqual(&testBuffer, &testStrBuf) == RSSL_TRUE);

	/* RSSL_DT_UTF8_STRING conversion test */
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_UTF8_STRING, &testBuffer) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_UTF8_STRING, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_UTF8_STRING, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslBufferIsEqual(&testBuffer, &testStrBuf) == RSSL_TRUE);

	/* RSSL_DT_RMTES_STRING conversion test */
	rsslClearBuffer(&testDataBuf);
	testDataBuf.data = testData;
	testDataBuf.length = sizeof(testData);
	rsslClearEncodeIterator(&encIter);
	ASSERT_TRUE(rsslSetEncodeIteratorBuffer(&encIter, &testDataBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslEncodePrimitiveType(&encIter, RSSL_DT_RMTES_STRING, &testBuffer) == RSSL_RET_SUCCESS);
	testDataBuf.length = rsslGetEncodedBufferLength(&encIter);
	rsslClearDecodeIterator(&decIter);
	ASSERT_TRUE(rsslSetDecodeIteratorBuffer(&decIter, &testDataBuf) == RSSL_RET_SUCCESS);
	/* should fail */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testStringSmall;
	testStrBuf.length = sizeof(testStringSmall);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_RMTES_STRING, &testStrBuf) != RSSL_RET_SUCCESS);
	/* should pass */
	rsslClearBuffer(&testStrBuf);
	testStrBuf.data = testString;
	testStrBuf.length = sizeof(testString);
	ASSERT_TRUE(rsslEncodedPrimitiveToString(&decIter, RSSL_DT_RMTES_STRING, &testStrBuf) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(rsslBufferIsEqual(&testBuffer, &testStrBuf) == RSSL_TRUE);

}

/* Basic test of blank vs. nonblank data, w/ and w/out set defs */
/* Also tests rsslEncodedPrimitiveToString(), and rsslDecodePrimitiveType() */
typedef struct
{
	RsslDataType dataType;
	void *value;
} TestPrimitive;

void _testPrimitiveElementList(TestPrimitive *primitives, RsslUInt primitivesCount, RsslLocalElementSetDefDb *pSetDb, RsslUInt16 setId, RsslBool blank)
{
	/* Encode and decode the primitive types given by 'primitives' into and out of an elementList. Encode as blank if specified. Use a set definition if given. */

	RsslDecodeIterator primDecIter;
	char primitiveData[24];
	RsslUInt32 primitiveStringLen = 256;
	RsslBuffer primitiveString = { primitiveStringLen, (char*)alloca(primitiveStringLen) };
	RsslElementList eList;
	RsslElementEntry eEntry;
	RsslElementSetDef *pSetDef = 0;
	RsslBuffer standardElemName = STR_TO_RSSLBUF("element");
	int i;

	tbufBig.length = TEST_BIG_BUF_SIZE;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	rsslClearElementList(&eList);

	if (pSetDb)
	{
		/* Encode entries as set data. */
		eList.flags |= RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID;
		eList.setId = setId;
		pSetDef = &pSetDb->definitions[setId];
		RSSL_ASSERT(pSetDef->setId == setId && primitivesCount == pSetDef->count, proper set def usage);
	}
	else
	{
		/* Encode entries as standard data. */
		eList.flags |= RSSL_ELF_HAS_STANDARD_DATA;
	}

	ASSERT_TRUE(rsslEncodeElementListInit(&encIter, &eList, pSetDb, 0) == RSSL_RET_SUCCESS);


	/* Encode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		RsslRet expectedRet = (pSetDb && i == pSetDef->count-1) ? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS;

		rsslClearElementEntry(&eEntry);
		eEntry.name = pSetDb ? pSetDef->pEntries[i].name : standardElemName;
		eEntry.dataType = primitives[i].dataType;

		if (blank)
		{
			ASSERT_TRUE(rsslEncodeElementEntry(&encIter, &eEntry, 0) == expectedRet);
		}
		else
		{

			switch (primitives[i].dataType)
			{
				case RSSL_DT_ARRAY:
				{
					/* Encode an int array of 1, 2, 3, 4 */
					RsslArray rArray;
					RsslInt testIntArray[] = {1, 2, 3, 4};
					int j;

					rsslClearArray(&rArray);
					rArray.primitiveType = RSSL_DT_INT;
					rArray.itemLength = 0;
					ASSERT_TRUE(rsslEncodeElementEntryInit(&encIter, &eEntry, 0) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &rArray) == RSSL_RET_SUCCESS);
					for (j = 0; j < 4; ++j)
						ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &testIntArray[j]) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeElementEntryComplete(&encIter, RSSL_TRUE) == expectedRet);
					break;
				}
				default:
					ASSERT_TRUE(rsslEncodeElementEntry(&encIter, &eEntry, primitives[i].value) == expectedRet);
					break;
			}
		}


	}
	ASSERT_TRUE(rsslEncodeElementListComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	tbufBig.length = rsslGetEncodedBufferLength(&encIter);

	rsslClearElementList(&eList);
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

	ASSERT_TRUE(rsslDecodeElementList(&decIter, &eList, pSetDb) == RSSL_RET_SUCCESS);

	/* Decode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		RsslRet ret;
		RsslBuffer expectedName = pSetDb ? pSetDef->pEntries[i].name : standardElemName;

		rsslClearElementEntry(&eEntry);


		ASSERT_TRUE(rsslDecodeElementEntry(&decIter, &eEntry) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslBufferIsEqual(&eEntry.name, &expectedName));
		ASSERT_TRUE(eEntry.dataType == primitives[i].dataType);

		if (blank)
		{

			/* Test rsslDecodePrimitiveType for blank(array should return failure, buffers will just be empty) */
			ret = rsslDecodePrimitiveType(&decIter, eEntry.dataType, &primitiveData); 
			switch(eEntry.dataType)
			{
				case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_RMTES_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && eEntry.encData.length == 0); break;
				default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); break;
			}
		}
		else
		{

			switch (primitives[i].dataType)
			{
				case RSSL_DT_ARRAY:
				{
					/* Should be an int array of 1, 2, 3, 4 */
					RsslArray rArray;
					RsslBuffer rArrayEntry;
					RsslInt testIntArray[] = {1, 2, 3, 4};
					RsslInt decInt;
					int j;

					ASSERT_TRUE(rsslDecodeArray(&decIter, &rArray) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rArray.primitiveType == RSSL_DT_INT);
					ASSERT_TRUE(rArray.itemLength == 0);
					for (j = 0; j < 4; ++j)
					{
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &rArrayEntry) == RSSL_RET_SUCCESS);
						ASSERT_TRUE(rsslDecodeInt(&decIter, &decInt) == RSSL_RET_SUCCESS);
						ASSERT_TRUE(decInt == testIntArray[j]);
					}
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &rArrayEntry) == RSSL_RET_END_OF_CONTAINER);
					break;
				}
				default:
					/* Tests that decodePrimitive returns SUCCESS. Doesn't check for matched value. */
					ASSERT_TRUE(rsslDecodePrimitiveType(&decIter, eEntry.dataType, &primitiveData) == RSSL_RET_SUCCESS);
					break;
			}
		}

		if (g_ToString)
		{
			/* Test printing out the primitive with rsslEncodedPrimitiveToString, along with the name, type, and the return code we got. */
			rsslClearDecodeIterator(&primDecIter);
			rsslSetDecodeIteratorBuffer(&primDecIter, &eEntry.encData);
			primitiveString.length = primitiveStringLen;

			ret = rsslEncodedPrimitiveToString(&primDecIter, eEntry.dataType, &primitiveString);
			if (blank)
			{
				switch(eEntry.dataType)
				{
					case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
					case RSSL_DT_ASCII_STRING:
					case RSSL_DT_RMTES_STRING:
					case RSSL_DT_UTF8_STRING:
					case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && eEntry.encData.length == 0); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
					default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
				}
			}
			else
			{
				switch(eEntry.dataType)
				{
					case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
					default: ASSERT_TRUE(ret == RSSL_RET_SUCCESS); break;
				}
			}

			//if (ret >= 0)
			//	printf("	<elementEntry name=\"%.*s\" dataType=\"%s\" ret=\"%s\" data=\"%.*s\" />", eEntry.name.length, eEntry.name.data, 
			//			rsslDataTypeToString(eEntry.dataType),
			//			rsslRetCodeToString(ret),
			//			primitiveString.length, primitiveString.data);
		}
	}
	ASSERT_TRUE(rsslDecodeElementEntry(&decIter, &eEntry) == RSSL_RET_END_OF_CONTAINER);

	tbufBig.length = TEST_BIG_BUF_SIZE;
}


void _testPrimitiveFieldList(TestPrimitive *primitives, RsslUInt primitivesCount, RsslLocalFieldSetDefDb *pSetDb, RsslUInt16 setId, RsslBool blank)
{
	/* Encode and decode the primitive types given by 'primitives' into and out of an elementList. Encode as blank if specified. Use a set definition if given. */

	RsslDecodeIterator primDecIter;
	char primitiveData[24];
	RsslUInt32 primitiveStringLen = 256;
	RsslBuffer primitiveString = { primitiveStringLen, (char*)alloca(primitiveStringLen) };
	RsslFieldList fList;
	RsslFieldEntry fEntry;
	RsslFieldSetDef *pSetDef = 0;
	RsslBuffer standardElemName = STR_TO_RSSLBUF("element");
	int i;

	tbufBig.length = TEST_BIG_BUF_SIZE;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	rsslClearFieldList(&fList);

	if (pSetDb)
	{
		/* Encode entries as set data. */
		fList.flags |= RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
		fList.setId = setId;
		pSetDef = &pSetDb->definitions[setId];
		RSSL_ASSERT(pSetDef->setId == setId && primitivesCount == pSetDef->count, proper set def usage);
	}
	else
	{
		/* Encode entries as standard data. */
		fList.flags |= RSSL_ELF_HAS_STANDARD_DATA;
	}

	ASSERT_TRUE(rsslEncodeFieldListInit(&encIter, &fList, pSetDb, 0) == RSSL_RET_SUCCESS);


	/* Encode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		RsslRet expectedRet = (pSetDb && i == pSetDef->count-1) ? RSSL_RET_SET_COMPLETE : RSSL_RET_SUCCESS;

		rsslClearFieldEntry(&fEntry);
		fEntry.fieldId = primitives[i].dataType;
		fEntry.dataType = primitives[i].dataType;

		if (blank)
		{
			ASSERT_TRUE(rsslEncodeFieldEntry(&encIter, &fEntry, NULL) == expectedRet);
		}
		else
		{

			switch (primitives[i].dataType)
			{
				case RSSL_DT_ARRAY:
				{
					/* Encode an int array of 1, 2, 3, 4 */
					RsslArray rArray;
					RsslInt testIntArray[] = {1, 2, 3, 4};
					int j;

					rsslClearArray(&rArray);
					rArray.primitiveType = RSSL_DT_INT;
					rArray.itemLength = 0;
					ASSERT_TRUE(rsslEncodeFieldEntryInit(&encIter, &fEntry, 0) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &rArray) == RSSL_RET_SUCCESS);
					for (j = 0; j < 4; ++j)
						ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, &testIntArray[j]) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rsslEncodeFieldEntryComplete(&encIter, RSSL_TRUE) == expectedRet);
					break;
				}
				default:
					ASSERT_TRUE(rsslEncodeFieldEntry(&encIter, &fEntry, primitives[i].value) == expectedRet);
					break;
			}
		}


	}
	ASSERT_TRUE(rsslEncodeFieldListComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	tbufBig.length = rsslGetEncodedBufferLength(&encIter);

	rsslClearFieldList(&fList);
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

	ASSERT_TRUE(rsslDecodeFieldList(&decIter, &fList, pSetDb) == RSSL_RET_SUCCESS);

	/* Decode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		RsslRet ret;
		rsslClearFieldEntry(&fEntry);


		ASSERT_TRUE(rsslDecodeFieldEntry(&decIter, &fEntry) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(fEntry.fieldId == primitives[i].dataType);

		if (blank)
		{

			/* Test rsslDecodePrimitiveType for blank(array should return failure, buffers will just be empty) */
			ret = rsslDecodePrimitiveType(&decIter, (RsslPrimitiveType)fEntry.fieldId, &primitiveData);
			switch(fEntry.fieldId)
			{
				case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_RMTES_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && fEntry.encData.length == 0); break;
				default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); break;
			}
		}
		else
		{

			switch (primitives[i].dataType)
			{
				case RSSL_DT_ARRAY:
				{
					/* Should be an int array of 1, 2, 3, 4 */
					RsslArray rArray;
					RsslBuffer rArrayEntry;
					RsslInt testIntArray[] = {1, 2, 3, 4};
					RsslInt decInt;
					int j;

					ASSERT_TRUE(rsslDecodeArray(&decIter, &rArray) == RSSL_RET_SUCCESS);
					ASSERT_TRUE(rArray.primitiveType == RSSL_DT_INT);
					ASSERT_TRUE(rArray.itemLength == 0);
					for (j = 0; j < 4; ++j)
					{
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &rArrayEntry) == RSSL_RET_SUCCESS);
						ASSERT_TRUE(rsslDecodeInt(&decIter, &decInt) == RSSL_RET_SUCCESS);
						ASSERT_TRUE(decInt == testIntArray[j]);
					}
						ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &rArrayEntry) == RSSL_RET_END_OF_CONTAINER);
					break;
				}
				default:
					/* Tests that decodePrimitive returns SUCCESS. Doesn't check for matched value. */
					ASSERT_TRUE(rsslDecodePrimitiveType(&decIter, primitives[i].dataType, &primitiveData) == RSSL_RET_SUCCESS);
					break;
			}
		}

		if (g_ToString)
		{
			/* Test printing out the primitive with rsslEncodedPrimitiveToString, along with the name, type, and the return code we got. */
			rsslClearDecodeIterator(&primDecIter);
			rsslSetDecodeIteratorBuffer(&primDecIter, &fEntry.encData);
			primitiveString.length = primitiveStringLen;

			ret = rsslEncodedPrimitiveToString(&primDecIter, fEntry.dataType, &primitiveString);
			if (blank)
			{
				switch(fEntry.dataType)
				{
					case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
					case RSSL_DT_ASCII_STRING:
					case RSSL_DT_RMTES_STRING:
					case RSSL_DT_UTF8_STRING:
					case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && fEntry.encData.length == 0); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
					default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
				}
			}
			else
			{
				switch(fEntry.dataType)
				{
					case RSSL_DT_ARRAY: ASSERT_TRUE(ret == RSSL_RET_FAILURE); break;
					default: ASSERT_TRUE(ret == RSSL_RET_SUCCESS); break;
				}
			}

			//if (ret >= 0)
			//	printf("	<fieldEntry fieldId=\"%i\" dataType=\"%s\" ret=\"%s\" data=\"%.*s\" />", fEntry.fieldId, 
			//			rsslDataTypeToString(fEntry.dataType),
			//			rsslRetCodeToString(ret),
			//			primitiveString.length, primitiveString.data);
		}
	}
	ASSERT_TRUE(rsslDecodeFieldEntry(&decIter, &fEntry) == RSSL_RET_END_OF_CONTAINER);

	tbufBig.length = TEST_BIG_BUF_SIZE;
}

typedef struct
{
	void *value;
	RsslRet ret; /* The return code when encoding this primitive. Some tests are designed to fail at a certain point. */
} TestArrayPrimitive;

void _testPrimitiveArray(TestArrayPrimitive *primitives, RsslUInt primitivesCount, RsslUInt8 primitiveType, RsslUInt8 itemLength, RsslBool blank)
{
	/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

	RsslDecodeIterator primDecIter;
	char primitiveData[32];
	RsslUInt32 primitiveStringLen = 256;
	RsslBuffer primitiveString = { primitiveStringLen, (char*)alloca(primitiveStringLen) };
	RsslArray arr;
	RsslBuffer aEntry;
	int i;

	tbufBig.length = TEST_BIG_BUF_SIZE;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &tbufBig);

	rsslClearArray(&arr);

	arr.itemLength = itemLength;
	arr.primitiveType = primitiveType;


	ASSERT_TRUE(rsslEncodeArrayInit(&encIter, &arr) == RSSL_RET_SUCCESS);


	/* Encode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		rsslClearBuffer(&aEntry);

		if (blank)
		{
#ifdef BLANK_ARRAY_BOTH_NULL
			ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, NULL, NULL) == primitives[i].ret);
#else
			ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, &aEntry, 0) == primitives[i].ret);
#endif

		}
		else
		{
			ASSERT_TRUE(rsslEncodeArrayEntry(&encIter, 0, primitives[i].value) == primitives[i].ret);
		}

		if (primitives[i].ret < 0) return; /* Test was designed to error on encoding. Stop here. */
	}
	ASSERT_TRUE(rsslEncodeArrayComplete(&encIter, RSSL_TRUE) == RSSL_RET_SUCCESS);

	tbufBig.length = rsslGetEncodedBufferLength(&encIter);

	rsslClearArray(&arr);
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);

	ASSERT_TRUE(rsslDecodeArray(&decIter, &arr) == RSSL_RET_SUCCESS);
	ASSERT_TRUE(arr.itemLength == itemLength && arr.primitiveType == primitiveType);

	/* Decode entries */
	for(i = 0; i < primitivesCount; ++i)
	{
		RsslRet ret;

		rsslClearBuffer(&aEntry);

		ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &aEntry) == RSSL_RET_SUCCESS);

		if (blank)
		{

			/* Test rsslDecodePrimitiveType for blank(array should return failure, buffers will just be empty) */
			ret = rsslDecodePrimitiveType(&decIter, arr.primitiveType, &primitiveData); 
			switch(arr.primitiveType)
			{
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_RMTES_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && aEntry.length == 0); break;
				default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); break;
			}
		}
		else
		{
			ASSERT_TRUE(rsslDecodePrimitiveType(&decIter, arr.primitiveType, &primitiveData) == RSSL_RET_SUCCESS);
		}

		if (g_ToString)
		{
			/* Test printing out the primitive with rsslEncodedPrimitiveToString, along with the name, type, and the return code we got. */
			rsslClearDecodeIterator(&primDecIter);
			rsslSetDecodeIteratorBuffer(&primDecIter, &aEntry);
			primitiveString.length = primitiveStringLen;

			ret = rsslEncodedPrimitiveToString(&primDecIter, arr.primitiveType, &primitiveString);
			if (blank)
			{
				switch(arr.primitiveType)
				{
					case RSSL_DT_ASCII_STRING:
					case RSSL_DT_RMTES_STRING:
					case RSSL_DT_UTF8_STRING:
					case RSSL_DT_BUFFER: ASSERT_TRUE(ret == RSSL_RET_SUCCESS && aEntry.length == 0); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
					default: ASSERT_TRUE(ret == RSSL_RET_BLANK_DATA); ASSERT_TRUE(primitiveString.length == 0 && primitiveString.data[0] == '\0'); break;
				}
			}
			else
			{
				ASSERT_TRUE(ret == RSSL_RET_SUCCESS);
			}

			//if (ret >= 0)
			//	printf("	<arrayEntry primitiveType=\"%s\" ret=\"%s\" data=\"%.*s\" />",
			//			rsslDataTypeToString(arr.primitiveType),
			//			rsslRetCodeToString(ret),
			//			primitiveString.length, primitiveString.data);
		}
	}
	ASSERT_TRUE(rsslDecodeArrayEntry(&decIter, &aEntry) == RSSL_RET_END_OF_CONTAINER);

	tbufBig.length = TEST_BIG_BUF_SIZE;
}

TEST(blankTests,blankTests)
{

	RsslInt testInt = -1234;
	RsslUInt testUInt = 2010;
	RsslFloat testFloat = 3.14159f;
	RsslDouble testDouble = 3.14159;
	RsslReal testReal = { RSSL_FALSE, RSSL_RH_EXPONENT_5, 314159 };
	RsslDateTime testDateTime = { {12, 11, 1955}, {22, 4, 0, 0} };
	RsslQos testQos = { RSSL_QOS_TIME_REALTIME, RSSL_QOS_RATE_TICK_BY_TICK, RSSL_FALSE, 0, 0};
	RsslState testState = { RSSL_STREAM_OPEN, RSSL_DATA_NO_CHANGE, RSSL_SC_INVALID_VIEW, STR_TO_RSSLBUF("Illinois") };
	RsslEnum testEnum = 8;
	RsslBuffer testBuffer = STR_TO_RSSLBUF("buffer test");

	TestPrimitive testPrimitives[] =
	{
		{ RSSL_DT_INT, &testInt},
		{ RSSL_DT_UINT, &testUInt},
		{ RSSL_DT_FLOAT, &testFloat},
		{ RSSL_DT_DOUBLE, &testDouble},
		{ RSSL_DT_REAL, &testReal},
		{ RSSL_DT_DATE, &testDateTime.date},
		{ RSSL_DT_TIME, &testDateTime.time},
		{ RSSL_DT_DATETIME, &testDateTime},
		{ RSSL_DT_QOS, &testQos},
		{ RSSL_DT_STATE, &testState},
		{ RSSL_DT_ENUM, &testEnum},
		{ RSSL_DT_ARRAY, 0},
		{ RSSL_DT_BUFFER, &testBuffer},
		{ RSSL_DT_ASCII_STRING, &testBuffer},
		{ RSSL_DT_RMTES_STRING, &testBuffer},
		{ RSSL_DT_UTF8_STRING, &testBuffer} 
	};
	static RsslUInt testPrimitivesCount = sizeof(testPrimitives)/sizeof(TestPrimitive);

	/* STR_TO_RSSLBUF comes from dictionaries.h. Would be nice if this could be in a more generic header. */
	RsslElementSetDefEntry testPrimitiveSet[] =
	{
		{ STR_TO_RSSLBUF("RSSL_DT_INT"), 			RSSL_DT_INT },
		{ STR_TO_RSSLBUF("RSSL_DT_UINT"),			RSSL_DT_UINT },
		{ STR_TO_RSSLBUF("RSSL_DT_FLOAT"),			RSSL_DT_FLOAT },
		{ STR_TO_RSSLBUF("RSSL_DT_DOUBLE"),			RSSL_DT_DOUBLE },
		{ STR_TO_RSSLBUF("RSSL_DT_REAL"),			RSSL_DT_REAL },
		{ STR_TO_RSSLBUF("RSSL_DT_DATE"),			RSSL_DT_DATE },
		{ STR_TO_RSSLBUF("RSSL_DT_TIME"),			RSSL_DT_TIME },
		{ STR_TO_RSSLBUF("RSSL_DT_DATETIME"),		RSSL_DT_DATETIME },
		{ STR_TO_RSSLBUF("RSSL_DT_QOS"),			RSSL_DT_QOS },
		{ STR_TO_RSSLBUF("RSSL_DT_STATE"),			RSSL_DT_STATE },
		{ STR_TO_RSSLBUF("RSSL_DT_ENUM"),			RSSL_DT_ENUM },
		{ STR_TO_RSSLBUF("RSSL_DT_ARRAY"),			RSSL_DT_ARRAY },
		{ STR_TO_RSSLBUF("RSSL_DT_BUFFER"),			RSSL_DT_BUFFER },
		{ STR_TO_RSSLBUF("RSSL_DT_ASCII_STRING"),	RSSL_DT_ASCII_STRING },
		{ STR_TO_RSSLBUF("RSSL_DT_RMTES_STRING"),	RSSL_DT_RMTES_STRING },
		{ STR_TO_RSSLBUF("RSSL_DT_UTF8_STRING"),	RSSL_DT_UTF8_STRING }
	};

	RsslFieldSetDefEntry testFieldSet[] =
	{
		{ RSSL_DT_INT, 			RSSL_DT_INT },
		{ RSSL_DT_UINT,			RSSL_DT_UINT },
		{ RSSL_DT_FLOAT,		RSSL_DT_FLOAT },
		{ RSSL_DT_DOUBLE,		RSSL_DT_DOUBLE },
		{ RSSL_DT_REAL,			RSSL_DT_REAL },
		{ RSSL_DT_DATE,			RSSL_DT_DATE },
		{ RSSL_DT_TIME,			RSSL_DT_TIME },
		{ RSSL_DT_DATETIME,		RSSL_DT_DATETIME },
		{ RSSL_DT_QOS,			RSSL_DT_QOS },
		{ RSSL_DT_STATE,		RSSL_DT_STATE },
		{ RSSL_DT_ENUM,			RSSL_DT_ENUM },
		{ RSSL_DT_ARRAY,		RSSL_DT_ARRAY },
		{ RSSL_DT_BUFFER,		RSSL_DT_BUFFER },
		{ RSSL_DT_ASCII_STRING,	RSSL_DT_ASCII_STRING },
		{ RSSL_DT_RMTES_STRING,	RSSL_DT_RMTES_STRING },
		{ RSSL_DT_UTF8_STRING,	RSSL_DT_UTF8_STRING }
	};

	RsslLocalElementSetDefDb testPrimitiveSetDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ 3, sizeof(testPrimitiveSet)/sizeof(RsslElementSetDefEntry), testPrimitiveSet },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	RsslLocalFieldSetDefDb testFieldPrimitiveSetDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ 3, sizeof(testFieldSet)/sizeof(RsslFieldSetDefEntry), testFieldSet },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	/* Blank element entry tests */
	/* Blank */ _testPrimitiveElementList(testPrimitives, testPrimitivesCount, 0, 0, RSSL_TRUE);
	/* Non-Blank */ _testPrimitiveElementList(testPrimitives, testPrimitivesCount, 0, 0, RSSL_FALSE);
	/* Blank w/ set */ _testPrimitiveElementList(testPrimitives, testPrimitivesCount, &testPrimitiveSetDb, 3, RSSL_TRUE);
	/* NonBlank w/ set */ _testPrimitiveElementList(testPrimitives, testPrimitivesCount, &testPrimitiveSetDb, 3, RSSL_FALSE);

	/* Blank field entry tests */
	/* Blank */ _testPrimitiveFieldList(testPrimitives, testPrimitivesCount, 0, 0, RSSL_TRUE);
	/* Non-Blank */ _testPrimitiveFieldList(testPrimitives, testPrimitivesCount, 0, 0, RSSL_FALSE);
	/* Blank w/ set */ _testPrimitiveFieldList(testPrimitives, testPrimitivesCount, &testFieldPrimitiveSetDb, 3, RSSL_TRUE);
	/* NonBlank w/ set */ _testPrimitiveFieldList(testPrimitives, testPrimitivesCount, &testFieldPrimitiveSetDb, 3, RSSL_FALSE);
}


TEST(arrayUnitTests,arrayUnitTests)
{
	/* Tests encoding arrays of different types and itemLengths, ensuring that it succeeds or fails as appropriate */
	/* TODO value comparison recommended */

	/* Indexed by encoded size(e.g. testInts[4] is a 4-byte value) */
	RsslInt testInts[] = { 0x0, 0x7f, 0x7fff, 0x7fffff, 0x7fffffff, 0x7fffffffffLL, 0x7fffffffffffLL, 0x7fffffffffffffLL, 0x7fffffffffffffffLL };
	RsslUInt testUInts[] = { 0x0, 0xff, 0xffff, 0xffffff, 0xffffffff, 0xffffffffffULL, 0xffffffffffffULL, 0xffffffffffffffULL, 0xffffffffffffffffULL };
	
	RsslReal testReals[] = { 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[0]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[1]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[2]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[3]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[4]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[5]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[6]}, 
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[7]},
		{ RSSL_FALSE, RSSL_RH_EXPONENT_5, testInts[8]}
	};

	RsslFloat testFloat = 3.14159f;
	RsslDouble testDouble = 3.14159;
	RsslDateTime testDateTime = { {12, 11, 1955}, {22, 4, 0, 0, 0, 0} };
	RsslDate testDate = testDateTime.date;
	RsslTime testTime = testDateTime.time;
	RsslQos testQos = { RSSL_QOS_TIME_REALTIME, RSSL_QOS_RATE_TICK_BY_TICK, RSSL_FALSE, 0, 0};
	RsslState testState = { RSSL_STREAM_OPEN, RSSL_DATA_NO_CHANGE, RSSL_SC_INVALID_VIEW, STR_TO_RSSLBUF("Illinois") };
	RsslEnum testEnum = 8;
	RsslBuffer testBuffer = STR_TO_RSSLBUF("felizviernes");

	RsslDataTypes bufferTypes[] = { RSSL_DT_BUFFER, RSSL_DT_ASCII_STRING, RSSL_DT_UTF8_STRING, RSSL_DT_RMTES_STRING } ;
	RsslUInt32 bufferTypesSize = sizeof(bufferTypes)/sizeof(RsslDataTypes);

	/* Empty test */
	_testPrimitiveArray(0, 0, RSSL_DT_INT, 0, RSSL_FALSE);
	_testPrimitiveArray(0, 0, RSSL_DT_INT, 1, RSSL_FALSE);

	/** Ints **/
	{
		/* Basic test */
		TestArrayPrimitive basicInts[] =
		{
			{ &testInts[1], RSSL_RET_SUCCESS }, { &testInts[2], RSSL_RET_SUCCESS }, { &testInts[4], RSSL_RET_SUCCESS }, { &testInts[8], RSSL_RET_SUCCESS }
		};
		RsslUInt basicIntsCount = sizeof(basicInts)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicIntsBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testInts[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicIntsBlankSetFailCount = sizeof(basicIntsBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Ints */
		_testPrimitiveArray(basicInts, basicIntsCount, RSSL_DT_INT, 0, RSSL_TRUE);

		/* NonBlank Ints */
		_testPrimitiveArray(basicInts, basicIntsCount, RSSL_DT_INT, 0, RSSL_FALSE);

		/* NonBlank Ints(Set) */
		_testPrimitiveArray(basicInts, basicIntsCount, RSSL_DT_INT, 8, RSSL_FALSE);

		/* Blank Ints(Set, should fail) */
		_testPrimitiveArray(basicIntsBlankSetFail, basicIntsBlankSetFailCount, RSSL_DT_INT, 4, RSSL_TRUE);
	}

	{
		/* Large value test -- test with a certain item length to see if we get VALUE_OUT_OF_RANGE.*/

		TestArrayPrimitive basicInts1[] = 
		{
			{ &testInts[1], RSSL_RET_SUCCESS }, { &testInts[2], RSSL_RET_VALUE_OUT_OF_RANGE },
		};
		RsslUInt basicInts1Count = sizeof(basicInts1)/sizeof(TestArrayPrimitive);

		TestArrayPrimitive basicInts2[] =
		{
			{ &testInts[1], RSSL_RET_SUCCESS }, { &testInts[2], RSSL_RET_SUCCESS }, { &testInts[4], RSSL_RET_VALUE_OUT_OF_RANGE },
		};
		RsslUInt basicInts2Count = sizeof(basicInts2)/sizeof(TestArrayPrimitive);

		TestArrayPrimitive basicInts4[] =
		{
			{ &testInts[1], RSSL_RET_SUCCESS }, { &testInts[2], RSSL_RET_SUCCESS }, { &testInts[4], RSSL_RET_SUCCESS }, { &testInts[8], RSSL_RET_VALUE_OUT_OF_RANGE }
		};
		RsslUInt basicInts4Count = sizeof(basicInts4)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(basicInts1, 1, RSSL_DT_INT, 1, RSSL_FALSE);
		_testPrimitiveArray(basicInts2, 0, RSSL_DT_INT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicInts2, 1, RSSL_DT_INT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicInts2, 2, RSSL_DT_INT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicInts4, 1, RSSL_DT_INT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicInts4, 2, RSSL_DT_INT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicInts4, 3, RSSL_DT_INT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicInts4, 4, RSSL_DT_INT, 4, RSSL_FALSE);
	}

	{
		/* Invalid itemLength test */

		TestArrayPrimitive invalidInts[] = 
		{
			{ &testInts[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidIntsCount = sizeof(invalidInts)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 3, RSSL_FALSE);
		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 5, RSSL_FALSE);
		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 6, RSSL_FALSE);
		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 7, RSSL_FALSE);
		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 9, RSSL_FALSE);
		_testPrimitiveArray(invalidInts, 1, RSSL_DT_INT, 10, RSSL_FALSE);
	}

	/** UInts **/
	{
		/* Basic test */
		TestArrayPrimitive basicUInts[] =
		{
			{ &testUInts[1], RSSL_RET_SUCCESS }, { &testUInts[2], RSSL_RET_SUCCESS }, { &testUInts[4], RSSL_RET_SUCCESS }, { &testUInts[8], RSSL_RET_SUCCESS }
		};
		RsslUInt basicUIntsCount = sizeof(basicUInts)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicUIntsBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testUInts[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicUIntsBlankSetFailCount = sizeof(basicUIntsBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank UInts */
		_testPrimitiveArray(basicUInts, basicUIntsCount, RSSL_DT_UINT, 0, RSSL_TRUE);

		/* NonBlank UInts */
		_testPrimitiveArray(basicUInts, basicUIntsCount, RSSL_DT_UINT, 0, RSSL_FALSE);

		/* NonBlank UInts(Set) */
		_testPrimitiveArray(basicUInts, basicUIntsCount, RSSL_DT_UINT, 8, RSSL_FALSE);

		/* Blank UInts(Set, should fail) */
		_testPrimitiveArray(basicUIntsBlankSetFail, basicUIntsBlankSetFailCount, RSSL_DT_UINT, 4, RSSL_TRUE);
	}

	{
		/* Large value test -- test with a certain item length to see if we get VALUE_OUT_OF_RANGE.*/

		TestArrayPrimitive basicUInts1[] = 
		{
			{ &testUInts[1], RSSL_RET_SUCCESS }, { &testUInts[2], RSSL_RET_VALUE_OUT_OF_RANGE },
		};
		RsslUInt basicUInts1Count = sizeof(basicUInts1)/sizeof(TestArrayPrimitive);

		TestArrayPrimitive basicUInts2[] =
		{
			{ &testUInts[1], RSSL_RET_SUCCESS }, { &testUInts[2], RSSL_RET_SUCCESS }, { &testUInts[4], RSSL_RET_VALUE_OUT_OF_RANGE },
		};
		RsslUInt basicUInts2Count = sizeof(basicUInts2)/sizeof(TestArrayPrimitive);

		TestArrayPrimitive basicUInts4[] =
		{
			{ &testUInts[1], RSSL_RET_SUCCESS }, { &testUInts[2], RSSL_RET_SUCCESS }, { &testUInts[4], RSSL_RET_SUCCESS }, { &testUInts[8], RSSL_RET_VALUE_OUT_OF_RANGE }
		};
		RsslUInt basicUInts4Count = sizeof(basicUInts4)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(basicUInts1, 1, RSSL_DT_UINT, 1, RSSL_FALSE);
		_testPrimitiveArray(basicUInts2, 0, RSSL_DT_UINT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicUInts2, 1, RSSL_DT_UINT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicUInts2, 2, RSSL_DT_UINT, 2, RSSL_FALSE);
		_testPrimitiveArray(basicUInts4, 1, RSSL_DT_UINT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicUInts4, 2, RSSL_DT_UINT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicUInts4, 3, RSSL_DT_UINT, 4, RSSL_FALSE);
		_testPrimitiveArray(basicUInts4, 4, RSSL_DT_UINT, 4, RSSL_FALSE);
	}

	{
		/* Invalid itemLength test */

		TestArrayPrimitive invalidUInts[] = 
		{
			{ &testUInts[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidUIntsCount = sizeof(invalidUInts)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 3, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 5, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 6, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 7, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 9, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 10, RSSL_FALSE); _testPrimitiveArray(invalidUInts, 1, RSSL_DT_UINT, 10, RSSL_TRUE);
	}
	
	/** Reals **/
	{
		/* Basic test */
		TestArrayPrimitive basicReals[] =
		{
			{ &testReals[1], RSSL_RET_SUCCESS }, { &testReals[2], RSSL_RET_SUCCESS }, { &testReals[4], RSSL_RET_SUCCESS }, { &testReals[8], RSSL_RET_SUCCESS }
		};
		RsslUInt basicRealsCount = sizeof(basicReals)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicRealsBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testReals[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicRealsBlankSetFailCount = sizeof(basicRealsBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Reals */
		_testPrimitiveArray(basicReals, basicRealsCount, RSSL_DT_REAL, 0, RSSL_TRUE);

		/* NonBlank Reals */
		_testPrimitiveArray(basicReals, basicRealsCount, RSSL_DT_REAL, 0, RSSL_FALSE);

		/* Blank Reals(Set, should fail [because no nonzero itemlength is valid for Real]) */
		_testPrimitiveArray(basicRealsBlankSetFail, basicRealsBlankSetFailCount, RSSL_DT_REAL, 4, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* All non-zero itemLengths are invalid for RsslReal */

		TestArrayPrimitive invalidReals[] = 
		{
			{ &testReals[1], RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidRealsCount = sizeof(invalidReals)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 1, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 2, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 3, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 4, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 5, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 6, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 7, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 8, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 9, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 10, RSSL_FALSE); _testPrimitiveArray(invalidReals, 1, RSSL_DT_REAL, 10, RSSL_TRUE);
	}

	/** Floats **/
	{
		/* Basic test */
		TestArrayPrimitive basicFloats[] =
		{
			{ &testFloat, RSSL_RET_SUCCESS }, { &testFloat, RSSL_RET_SUCCESS }, { &testFloat, RSSL_RET_SUCCESS }, { &testFloat, RSSL_RET_SUCCESS }
		};
		RsslUInt basicFloatsCount = sizeof(basicFloats)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicFloatsBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicFloatsBlankSetFailCount = sizeof(basicFloatsBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Floats */
		_testPrimitiveArray(basicFloats, basicFloatsCount, RSSL_DT_FLOAT, 0, RSSL_TRUE);

		/* NonBlank Floats */
		_testPrimitiveArray(basicFloats, basicFloatsCount, RSSL_DT_FLOAT, 0, RSSL_FALSE);

		/* NonBlank Floats(Set) */
		_testPrimitiveArray(basicFloats, basicFloatsCount, RSSL_DT_FLOAT, 4, RSSL_FALSE);

		/* Blank Floats(Set, should fail) */
		_testPrimitiveArray(basicFloatsBlankSetFail, basicFloatsBlankSetFailCount, RSSL_DT_FLOAT, 4, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0 and 4 are the only valid lengths for floats */
		TestArrayPrimitive invalidFloats[] = 
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidFloatsCount = sizeof(invalidFloats)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 1, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 2, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 3, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 5, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 6, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 7, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 8, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 9, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 10, RSSL_FALSE); _testPrimitiveArray(invalidFloats, 1, RSSL_DT_FLOAT, 10, RSSL_TRUE);
	}

	/** Doubles **/
	{
		/* Basic test */
		TestArrayPrimitive basicDoubles[] =
		{
			{ &testDouble, RSSL_RET_SUCCESS }, { &testDouble, RSSL_RET_SUCCESS }, { &testDouble, RSSL_RET_SUCCESS }, { &testDouble, RSSL_RET_SUCCESS }
		};
		RsslUInt basicDoublesCount = sizeof(basicDoubles)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicDoublesBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicDoublesBlankSetFailCount = sizeof(basicDoublesBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Doubles */
		_testPrimitiveArray(basicDoubles, basicDoublesCount, RSSL_DT_DOUBLE, 0, RSSL_TRUE);

		/* NonBlank Doubles */
		_testPrimitiveArray(basicDoubles, basicDoublesCount, RSSL_DT_DOUBLE, 0, RSSL_FALSE);

		/* NonBlank Doubles(Set) */
		_testPrimitiveArray(basicDoubles, basicDoublesCount, RSSL_DT_DOUBLE, 8, RSSL_FALSE);

		/* Blank Doubles(Set, should fail) */
		_testPrimitiveArray(basicDoublesBlankSetFail, basicDoublesBlankSetFailCount, RSSL_DT_DOUBLE, 8, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0 and 8 are the only valid lengths for doubles */
		TestArrayPrimitive invalidDoubles[] = 
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidDoublesCount = sizeof(invalidDoubles)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 1, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 2, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 3, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 4, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 5, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 6, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 7, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 9, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 10, RSSL_FALSE); _testPrimitiveArray(invalidDoubles, 1, RSSL_DT_DOUBLE, 10, RSSL_TRUE);
	}

	/** Dates **/
	{
		/* Basic test */
		TestArrayPrimitive basicDates[] =
		{
			{ &testDate, RSSL_RET_SUCCESS }, { &testDate, RSSL_RET_SUCCESS }, { &testDate, RSSL_RET_SUCCESS }, { &testDate, RSSL_RET_SUCCESS }
		};
		RsslUInt basicDatesCount = sizeof(basicDates)/sizeof(TestArrayPrimitive);

		/* Blank Dates */
		_testPrimitiveArray(basicDates, basicDatesCount, RSSL_DT_DATE, 0, RSSL_TRUE);

		/* NonBlank Dates */
		_testPrimitiveArray(basicDates, basicDatesCount, RSSL_DT_DATE, 0, RSSL_FALSE);

		/* NonBlank Dates(Set) */
		_testPrimitiveArray(basicDates, basicDatesCount, RSSL_DT_DATE, 4, RSSL_FALSE);

		/* Blank Dates(Set) */
		/* TODO This fails because even though it is "blank," the function to decode it returns success. */
		//_testPrimitiveArray(basicDates, basicDatesCount, RSSL_DT_DATE, 4, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0 and 4 are the only valid lengths for dates */
		TestArrayPrimitive invalidDates[] = 
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidDatesCount = sizeof(invalidDates)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 1, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 2, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 3, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 5, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 6, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 7, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 8, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 9, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 10, RSSL_FALSE); _testPrimitiveArray(invalidDates, 1, RSSL_DT_DATE, 10, RSSL_TRUE);
	}

	/** Times **/
	{
		/* Basic test */
		TestArrayPrimitive basicTimes[] =
		{
			{ &testTime, RSSL_RET_SUCCESS }, { &testTime, RSSL_RET_SUCCESS }, { &testTime, RSSL_RET_SUCCESS }, { &testTime, RSSL_RET_SUCCESS }, { &testTime, RSSL_RET_SUCCESS }, { &testTime, RSSL_RET_SUCCESS }
		};
		RsslUInt basicTimesCount = sizeof(basicTimes)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicTimesBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testTime, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicTimesBlankSetFailCount = sizeof(basicTimesBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Times */
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 0, RSSL_TRUE);

		/* NonBlank Times */
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 0, RSSL_FALSE);

		/* NonBlank Times(Set) */
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 3, RSSL_FALSE);
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 5, RSSL_FALSE);
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 7, RSSL_FALSE);
		_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 8, RSSL_FALSE);

		/* Blank Times(Set) */
		/* TODO This fails because even though it is "blank," the function to decode it returns success. */
		//_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 3, RSSL_TRUE);
		//_testPrimitiveArray(basicTimes, basicTimesCount, RSSL_DT_TIME, 5, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0,3 and 5 are the only valid lengths for dates */
		TestArrayPrimitive invalidTimes[] = 
		{
			{ &testFloat, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidTimesCount = sizeof(invalidTimes)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 1, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 2, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 4, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 6, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 9, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 10, RSSL_FALSE); _testPrimitiveArray(invalidTimes, 1, RSSL_DT_TIME, 10, RSSL_TRUE);
	}

	/** DateTimes **/
	{
		/* Basic test */
		TestArrayPrimitive basicDateTimes[] =
		{
			{ &testDateTime, RSSL_RET_SUCCESS }, { &testDateTime, RSSL_RET_SUCCESS }, { &testDateTime, RSSL_RET_SUCCESS }, { &testDateTime, RSSL_RET_SUCCESS }, { &testDateTime, RSSL_RET_SUCCESS }, { &testDateTime, RSSL_RET_SUCCESS }
		};
		RsslUInt basicDateTimesCount = sizeof(basicDateTimes)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicDateTimesBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testDateTime, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicDateTimesBlankSetFailCount = sizeof(basicDateTimesBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank DateTimes */
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 0, RSSL_TRUE);

		/* NonBlank DateTimes */
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 0, RSSL_FALSE);

		/* NonBlank DateTimes(Set) */
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 7, RSSL_FALSE);
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 9, RSSL_FALSE);
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 11, RSSL_FALSE);
		_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 12, RSSL_FALSE);

		/* Blank DateTimes */
		/* TODO This fails because even though it is "blank," the function to decode it returns success. */
		//_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 7, RSSL_TRUE);
		//_testPrimitiveArray(basicDateTimes, basicDateTimesCount, RSSL_DT_DATETIME, 9, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0,7 and 9 are the only valid lengths for datetimes */
		TestArrayPrimitive invalidDateTimes[] = 
		{
			{ &testDateTime, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidDateTimesCount = sizeof(invalidDateTimes)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 1, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 2, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 3, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 4, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 5, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 6, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 8, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 10, RSSL_FALSE); _testPrimitiveArray(invalidDateTimes, 1, RSSL_DT_DATETIME, 10, RSSL_TRUE);
	}

	/** Qoses **/
	{
		/* Basic test */
		TestArrayPrimitive basicQoses[] =
		{
			{ &testQos, RSSL_RET_SUCCESS }, { &testQos, RSSL_RET_SUCCESS }, { &testQos, RSSL_RET_SUCCESS }, { &testQos, RSSL_RET_SUCCESS }
		};
		RsslUInt basicQosesCount = sizeof(basicQoses)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicQosesBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testQos, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicQosesBlankSetFailCount = sizeof(basicQosesBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Qoses */
		_testPrimitiveArray(basicQoses, basicQosesCount, RSSL_DT_QOS, 0, RSSL_TRUE);

		/* NonBlank Qoses */
		_testPrimitiveArray(basicQoses, basicQosesCount, RSSL_DT_QOS, 0, RSSL_FALSE);
	}

	{
		/* Invalid itemLength test */
		/* 0 is the only valid length */
		TestArrayPrimitive invalidQoses[] = 
		{
			{ &testQos, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidQosesCount = sizeof(invalidQoses)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 1, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 2, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 3, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 4, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 5, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 6, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 7, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 8, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 9, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 10, RSSL_FALSE); _testPrimitiveArray(invalidQoses, 1, RSSL_DT_QOS, 10, RSSL_TRUE);
	}

	/** States **/
	{
		/* Basic test */
		TestArrayPrimitive basicStates[] =
		{
			{ &testState, RSSL_RET_SUCCESS }, { &testState, RSSL_RET_SUCCESS }, { &testState, RSSL_RET_SUCCESS }, { &testState, RSSL_RET_SUCCESS }
		};
		RsslUInt basicStatesCount = sizeof(basicStates)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicStatesBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testState, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicStatesBlankSetFailCount = sizeof(basicStatesBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank States */
		_testPrimitiveArray(basicStates, basicStatesCount, RSSL_DT_STATE, 0, RSSL_TRUE);

		/* NonBlank States */
		_testPrimitiveArray(basicStates, basicStatesCount, RSSL_DT_STATE, 0, RSSL_FALSE);
	}

	{
		/* Invalid itemLength test */
		/* 0 is the only valid length */
		TestArrayPrimitive invalidStates[] = 
		{
			{ &testState, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidStatesCount = sizeof(invalidStates)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 1, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 1, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 2, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 2, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 3, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 4, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 5, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 6, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 7, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 8, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 9, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 10, RSSL_FALSE); _testPrimitiveArray(invalidStates, 1, RSSL_DT_STATE, 10, RSSL_TRUE);
	}

	/** Enums **/
	{
		/* Basic test */
		TestArrayPrimitive basicEnums[] =
		{
			{ &testEnum, RSSL_RET_SUCCESS }, { &testEnum, RSSL_RET_SUCCESS }, { &testEnum, RSSL_RET_SUCCESS }, { &testEnum, RSSL_RET_SUCCESS }
		};
		RsslUInt basicEnumsCount = sizeof(basicEnums)/sizeof(TestArrayPrimitive);
	

		TestArrayPrimitive basicEnumsBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
		{
			{ &testEnum, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt basicEnumsBlankSetFailCount = sizeof(basicEnumsBlankSetFail)/sizeof(TestArrayPrimitive);


		/* Blank Enums */
		_testPrimitiveArray(basicEnums, basicEnumsCount, RSSL_DT_ENUM, 0, RSSL_TRUE);

		/* NonBlank Enums */
		_testPrimitiveArray(basicEnums, basicEnumsCount, RSSL_DT_ENUM, 0, RSSL_FALSE);

		/* NonBlank Enums(Set) */
		_testPrimitiveArray(basicEnums, basicEnumsCount, RSSL_DT_ENUM, 1, RSSL_FALSE);
		_testPrimitiveArray(basicEnums, basicEnumsCount, RSSL_DT_ENUM, 2, RSSL_FALSE);

		/* Blank Enums(Set, should fail) */
		_testPrimitiveArray(basicEnumsBlankSetFail, basicEnumsBlankSetFailCount, RSSL_DT_ENUM, 8, RSSL_TRUE);
	}

	{
		/* Invalid itemLength test */
		/* 0,1,2 are the only valid lengths */
		TestArrayPrimitive invalidEnums[] = 
		{
			{ &testEnum, RSSL_RET_INVALID_ARGUMENT }
		};
		RsslUInt invalidEnumsCount = sizeof(invalidEnums)/sizeof(TestArrayPrimitive);

		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 3, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 3, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 4, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 4, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 5, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 5, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 6, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 6, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 7, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 7, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 8, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 8, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 9, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 9, RSSL_TRUE);
		_testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 10, RSSL_FALSE); _testPrimitiveArray(invalidEnums, 1, RSSL_DT_ENUM, 10, RSSL_TRUE);
	}

	/** Buffers(Buffer, AsciiString, UTF8String, RMTESString) **/
	for (RsslUInt32 i = 0; i < bufferTypesSize; ++i)
	{
		RsslDataTypes bufferType = bufferTypes[i];
		RsslBuffer tempTestBuffer = testBuffer;
		RsslInt32 j;

		{
			/* Basic test */
			TestArrayPrimitive basicBuffers[] =
			{
				{ &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS },
				{ &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS },
				{ &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }, { &tempTestBuffer, RSSL_RET_SUCCESS }
			};
			RsslUInt basicBuffersCount = sizeof(basicBuffers)/sizeof(TestArrayPrimitive);


			TestArrayPrimitive basicBuffersBlankSetFail[] = /* Tests trying to set encode a blank int, which should fail */
			{
				{ &tempTestBuffer, RSSL_RET_INVALID_ARGUMENT }
			};
			RsslUInt basicBuffersBlankSetFailCount = sizeof(basicBuffersBlankSetFail)/sizeof(TestArrayPrimitive);

			TestArrayPrimitive invalidBuffers[] = /* Tests trying to set encode a blank int, which should fail */
			{
				{ &tempTestBuffer, RSSL_RET_INVALID_ARGUMENT }
			};
			RsslUInt invalidBuffersCount = sizeof(invalidBuffers)/sizeof(TestArrayPrimitive);


			/* Blank Buffers */
			_testPrimitiveArray(basicBuffers, basicBuffersCount, bufferType, 0, RSSL_TRUE);

			/* NonBlank Buffers */
			_testPrimitiveArray(basicBuffers, basicBuffersCount, bufferType, 0, RSSL_FALSE);

			/* Blank Buffers(Set, should fail) */
			_testPrimitiveArray(basicBuffersBlankSetFail, basicBuffersBlankSetFailCount, bufferType, 8, RSSL_TRUE);

			/* Fixed-length buffers */
			for (j = 1; j <= basicBuffersCount; ++j)
			{
				tempTestBuffer.length = j;
				_testPrimitiveArray(basicBuffers, 2, bufferType, j, RSSL_FALSE);

				/* mismatched length should work (it just copies the needed bytes)*/
				tempTestBuffer.length = j-1;
				_testPrimitiveArray(basicBuffers, 1, bufferType, j, RSSL_FALSE);

				/* greater length asserts */
			}
		}


	}
}

TEST(textFileReaderTest,textFileReaderTest)
{
	TextFileReader textFileReader;
	RsslBuffer errorText = { 255, (char*)alloca(255) };
	int length;
	FILE *testFile;
	char *testString;

	/* Empty file */
	ASSERT_TRUE((testFile = fopen("tmpFile.txt", "w")) != NULL);
	fclose(testFile);
	ASSERT_TRUE((testFile = fopen("tmpFile.txt", "r")) != NULL);
	ASSERT_TRUE(textFileReaderInit(&textFileReader, testFile, &errorText) == 0);
	ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) == 0);
	textFileReaderCleanup(&textFileReader);
	fclose(testFile);

	/* Empty file with newline */
	ASSERT_TRUE((testFile = fopen("tmpFile.txt", "w")) != NULL);
	testString = const_cast<char*>("\n");
	fwrite(testString, 1, 1, testFile);
	fclose(testFile);
	ASSERT_TRUE((testFile = fopen("tmpFile.txt", "r")) != NULL);
	ASSERT_TRUE(textFileReaderInit(&textFileReader, testFile, &errorText) == 0);
	ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) > 0);
	ASSERT_TRUE(textFileReader.currentLineLength == 0);
	ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) == 0);
	textFileReaderCleanup(&textFileReader);
	fclose(testFile);

	/* Test one-line files of increasing length */
	for (length = 1; length < 2048; ++length) 
	{
		int i;

		/* Test with:
		   - no newline
		   - newline
		   - carriage return and newline
		   */
		for (i = 0; i < 3; ++i)
		{
			int pos;

			ASSERT_TRUE((testFile = fopen("tmpFile.txt", "w")) != NULL);

			switch(i)
			{
				case 0: /* Without newline */
					ASSERT_TRUE((testString = (char*)malloc(length)) != NULL);
					memset(testString, 'Z', length);
					fwrite(testString, 1, length, testFile);
					break;

				case 1: /* With newline */
					ASSERT_TRUE((testString = (char*)malloc(length + 1)) != NULL);
					memset(testString, 'Z', length);
					testString[length] = '\n';
					fwrite(testString, 1, length + 1, testFile);
					break;

				case 2: /* With newline and carriage return */
					ASSERT_TRUE((testString = (char*)malloc(length + 2)) != NULL);
					memset(testString, 'Z', length);
					testString[length] = '\r';
					testString[length + 1] = '\n';
					fwrite(testString, 1, length + 2, testFile);
					break;
			}

			fclose(testFile);

			ASSERT_TRUE((testFile = fopen("tmpFile.txt", "r")) != NULL);

			ASSERT_TRUE(textFileReaderInit(&textFileReader, testFile, &errorText) == 0);
			ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) > 0);

			ASSERT_TRUE(textFileReader.currentLineLength == length);
			for(pos = 0; pos < textFileReader.currentLineLength; ++pos)
				ASSERT_TRUE(textFileReader.currentLine[pos] == 'Z');

			ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) == 0);

			textFileReaderCleanup(&textFileReader);

			free(testString);
			fclose(testFile);
		}
	}

	/* Test two-line files of increasing length */
	for (length = 1; length < 2048; ++length) 
	{
		int i;

		/* Test with and without newline */
		for (i = 0; i < 2; ++i)
		{
			int pos;

			ASSERT_TRUE((testFile = fopen("tmpFile.txt", "w")) != NULL);

			switch(i)
			{
				case 0: /* Without newline */
					ASSERT_TRUE((testString = (char*)malloc(length * 2 + 1)) != NULL);
					memset(testString, 'Z', length);
					testString[length] = '\n';
					memset(testString + length + 1, 'X', length);
					fwrite(testString, 1, length * 2 + 1, testFile);
					break;

				case 1: /* With newline */
					ASSERT_TRUE((testString = (char*)malloc(length * 2 + 2)) != NULL);
					memset(testString, 'Z', length);
					testString[length] = '\n';
					memset(testString + length + 1, 'X', length);
					testString[length * 2 + 1] = '\n';
					fwrite(testString, 1, length * 2 + 2, testFile);
					break;

				case 2: /* With newline and carriage return */
					ASSERT_TRUE((testString = (char*)malloc(length * 2 + 4)) != NULL);
					memset(testString, 'Z', length);
					testString[length] = '\r';
					testString[length+1] = '\n';
					memset(testString + length + 2, 'X', length);
					testString[length * 2 + 2] = '\r';
					testString[length * 2 + 3] = '\n';
					fwrite(testString, 1, length * 2 + 4, testFile);
					break;

			}

			free(testString);
			fclose(testFile);

			ASSERT_TRUE((testFile = fopen("tmpFile.txt", "r")) != NULL);

			ASSERT_TRUE(textFileReaderInit(&textFileReader, testFile, &errorText) == 0);

			ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) > 0);
			ASSERT_TRUE(textFileReader.currentLineLength == length);
			for(pos = 0; pos < textFileReader.currentLineLength; ++pos)
				ASSERT_TRUE(textFileReader.currentLine[pos] == 'Z');

			ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) > 0);
			ASSERT_TRUE(textFileReader.currentLineLength == length);
			for(pos = 0; pos < textFileReader.currentLineLength; ++pos)
				ASSERT_TRUE(textFileReader.currentLine[pos] == 'X');

			if (i > 0)
				ASSERT_TRUE(textFileReaderReadLine(&textFileReader, &errorText) == 0);

			textFileReaderCleanup(&textFileReader);

			fclose(testFile);
		}
	}
	remove("tmpFile.txt");
}

TEST(mapEmptySummaryDataTest,mapEmptySummaryDataTest)
{
	RsslMap map = RSSL_INIT_MAP;
	RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslEncodeIterator encodeIter;
	RsslBuffer tmpBuf;

	//printf("Map Empty Summary Data Test:\n\n");

	rsslClearEncodeIterator(&encodeIter);

	tbufBig.length = 1024;
	rsslSetEncodeIteratorBuffer(&encodeIter, &tbufBig);

	/* encode map */
	map.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_KEY_FIELD_ID;
	map.containerType = RSSL_DT_FIELD_LIST;

	/* The map is keyed by the ORDER_ID field. */
	map.keyPrimitiveType = RSSL_DT_BUFFER;
	map.keyFieldId = 3426;

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapInit(&encodeIter, &map, 0, 0)); //rsslEncodeMapInit

	/* Encode empty summary data */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapSummaryDataComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeMapSummaryDataComplete

	/* encode map entry */
	rsslClearMapEntry(&mapEntry);

    mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	mapEntry.flags = RSSL_MPEF_NONE;
	tmpBuf.data = const_cast<char*>("123");
	tmpBuf.length = (RsslUInt32)strlen(tmpBuf.data);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapEntryInit(&encodeIter, &mapEntry, &tmpBuf, 0)); //rsslEncodeMapEntryInit

	/* encode field list */
	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&encodeIter, &fList, NULL, 0)); //rsslEncodeFieldListInit

    /* encode fields */

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 240;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"1")); //rsslEncodeFieldEntry

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 241;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"2")); //rsslEncodeFieldEntry

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 242;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"3")); //rsslEncodeFieldEntry

	/* complete encode field list */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeFieldListComplete

	/* complete map entry */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapEntryComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeMapEntryComplete

	/* complete map */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMapComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeMapComplete

	// decode Map
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
	rsslClearMap(&map);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMap(&decIter, &map)); //rsslDecodeMap

	// decode empty summary data - NO_DATA should be returned
	rsslClearFieldList(&fList);
	ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(&decIter, &fList, NULL)); //rsslDecodeFieldList

	// decode map entry
	rsslClearMapEntry(&mapEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMapEntry(&decIter, &mapEntry, &tmpBuf)); //rsslDecodeMapEntry

	// decode field list
	rsslClearFieldList(&fList);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(&decIter, &fList, NULL)); //rsslDecodeFieldList

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry
}

TEST(seriesEmptySummaryDataTest,seriesEmptySummaryDataTest)
{
	RsslSeries series = RSSL_INIT_SERIES;
	RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslEncodeIterator encodeIter;

	//printf("Series Empty Summary Data Test:\n\n");

	rsslClearEncodeIterator(&encodeIter);

	tbufBig.length = 1024;
	rsslSetEncodeIteratorBuffer(&encodeIter, &tbufBig);

	/* encode series */
	series.flags = RSSL_SRF_HAS_SUMMARY_DATA;
	series.containerType = RSSL_DT_FIELD_LIST;

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesInit(&encodeIter, &series, 0, 0)); //rsslEncodeSeriesInit

	/* Encode empty summary data */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesSummaryDataComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeSeriesSummaryDataComplete

	/* encode series entry */
	rsslClearSeriesEntry(&seriesEntry);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesEntryInit(&encodeIter, &seriesEntry, 0)); //rsslEncodeSeriesEntryInit

	/* encode field list */
	rsslClearFieldList(&fList);
	fList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListInit(&encodeIter, &fList, NULL, 0)); //rsslEncodeFieldListInit

    /* encode fields */

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 240;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"1")); //rsslEncodeFieldEntry

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 241;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"2")); //rsslEncodeFieldEntry

	rsslClearFieldEntry(&fEntry);
	fEntry.fieldId = 242;
	fEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldEntry(&encodeIter, &fEntry, (void*)"3")); //rsslEncodeFieldEntry

	/* complete encode field list */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeFieldListComplete

	/* complete series entry */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesEntryComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeSeriesEntryComplete

	/* complete series */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeSeriesComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeSeriesComplete

	// decode Map
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
	rsslClearSeries(&series);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeSeries(&decIter, &series)); //rsslDecodeSeries

	// decode empty summary data - NO_DATA should be returned
	rsslClearFieldList(&fList);
	ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeFieldList(&decIter, &fList, NULL)); //rsslDecodeFieldList

	// decode series entry
	rsslClearSeriesEntry(&seriesEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeSeriesEntry(&decIter, &seriesEntry)); //rsslDecodeSeriesEntry

	// decode field list
	rsslClearFieldList(&fList);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(&decIter, &fList, NULL)); //rsslDecodeFieldList

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry

	// decode field entry
	rsslClearFieldEntry(&fEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(&decIter, &fEntry)); //rsslDecodeFieldEntry
}

TEST(vectorEmptySummaryDataTest,vectorEmptySummaryDataTest)
{
	RsslVector vector = RSSL_INIT_VECTOR;
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;
	RsslElementList elList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry elEntry = RSSL_INIT_ELEMENT_ENTRY;
	RsslEncodeIterator encodeIter;

	//printf("Vector Empty Summary Data Test:\n\n");

	rsslClearEncodeIterator(&encodeIter);

	tbufBig.length = 1024;
	rsslSetEncodeIteratorBuffer(&encodeIter, &tbufBig);

	/* encode vector */
	vector.flags = RSSL_VTF_HAS_SUMMARY_DATA;
	vector.containerType = RSSL_DT_ELEMENT_LIST;

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorInit(&encodeIter, &vector, 0, 0)); //rsslEncodeVectorInit

	/* Encode empty summary data */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorSummaryDataComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeVectorSummaryDataComplete

	/* encode vector entry */
	rsslClearVectorEntry(&vectorEntry);

	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorEntryInit(&encodeIter, &vectorEntry, 0)); //rsslEncodeVectorEntryInit

	/* encode element list */
	rsslClearElementList(&elList);
	elList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementListInit(&encodeIter, &elList, NULL, 0)); //rsslEncodeElementListInit

    /* encode elements */

	rsslClearElementEntry(&elEntry);
	elEntry.name.data = const_cast<char*>("LINK 1");
	elEntry.name.length = (rtrUInt32)strlen("LINK 1");
	elEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encodeIter, &elEntry, (void*)"1")); //rsslEncodeElementEntry

	rsslClearElementEntry(&elEntry);
	elEntry.name.data = const_cast<char*>("LINK 2");
	elEntry.name.length = (rtrUInt32)strlen("LINK 2");
	elEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encodeIter, &elEntry, (void*)"2")); //rsslEncodeElementEntry

	rsslClearElementEntry(&elEntry);
	elEntry.name.data = const_cast<char*>("LINK 3");
	elEntry.name.length = (rtrUInt32)strlen("LINK 3");
	elEntry.dataType = 14;
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementEntry(&encodeIter, &elEntry, (void*)"3")); //rsslEncodeElementEntry

	/* complete encode element list */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeElementListComplete

	/* complete vector entry */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorEntryComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeVectorEntryComplete

	/* complete vector */
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeVectorComplete(&encodeIter, RSSL_TRUE)); //rsslEncodeVectorComplete

	// decode Map
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &tbufBig);
	rsslClearVector(&vector);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeVector(&decIter, &vector)); //rsslDecodeVector

	// decode empty summary data - NO_DATA should be returned
	rsslClearElementList(&elList);
	ASSERT_TRUE(RSSL_RET_NO_DATA == rsslDecodeElementList(&decIter, &elList, NULL)); //rsslDecodeElementList

	// decode vector entry
	rsslClearVectorEntry(&vectorEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeVectorEntry(&decIter, &vectorEntry)); //rsslDecodeVectorEntry

	// decode element list
	rsslClearElementList(&elList);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementList(&decIter, &elList, NULL)); //rsslDecodeElementList

	// decode element entry
	rsslClearElementEntry(&elEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementEntry(&decIter, &elEntry)); //rsslDecodeElementEntry

	// decode element entry
	rsslClearElementEntry(&elEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementEntry(&decIter, &elEntry)); //rsslDecodeElementEntry

	// decode element entry
	rsslClearElementEntry(&elEntry);
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeElementEntry(&decIter, &elEntry)); //rsslDecodeElementEntry
}

#define DATETIME_TEST_STRLEN 160

TEST(dateTimeToStringFormatTest,dateTimeToStringFormatTest)
{
	/* Date  */ 
	char dateTestName[12][24] = {		
		"Blank Date1", "Blank Date2", "Blank Date3", "Blank Date4", "NonBlank Date",
		"Blank Date6", "Blank Date7", "Blank Date8", "Valid Date1", "Valid Date2",
		"Valid Date3", "Valid Date4"
	};
	RsslDate iDate[12] = {
		{0,0,0}, {0,1,0}, {0,1,2011}, {1,0,0},	{1,1,2011}, 
		{1,0,2011}, {0,0,2011}, {1,1,0}, {25,5,2010}, {29,9,2017},
		{20,01,0001}, {30,10,2010}
	};

	char expDateTestOutRssl[12][48] = {
		"", "   JAN     ","   JAN 2011", "01         ","01 JAN 2011",
		"01     2011", "       2011", "01 JAN     ", "25 MAY 2010", "29 SEP 2017",
		"20 JAN    1", "30 OCT 2010"
	};
	char expDateTestOutIso8601[12][48] = {
		"", "--01", "2011-01", "--  -01","2011-01-01",
		"2011-  -01", "2011", "--01-01", "2010-05-25", "2017-09-29",
		"0001-01-20", "2010-10-30"
	};

	/* Time */
	char timeTestName[23][48] = {		
		"Time AllZeros", "Time AllBlank", "Time HH NotBlank", "Time HH:MM NotBlank", "Time HH:MM:SS NotBlank",
		"Time Micro:Nano Blank", "Time Nano Blank", "Time Trail_0 3Digits", "Time Trail_0 2Digits", "Time 1Digit",
		"Time SecMilMicroNano_0", "Time MilMicroNano_0 SecTrail0", "Time MilMicroNano_0 Sec1Digit", "Time MicroNano_0  Mil3Digit_Trail0", "Time MicroNano_0 Mil2Digit_Trail0",		
		"Time MicroNano_0 Mil1Digit", "Time Nano_0 Micro3Digit_Trail0", "Time Nano_0 Micro2Digit_Trail0", "Time Nano_0 Micro1Digit", "Time Nano3Digits_Trail0",
		"Time Nano2Digits_Trail0","Time Nano1Digit", "Time All 1Digit"
	};
	RsslTime iTime[23] = {
		{0,0,0,0,0,0}, {255,255,255,65535,2047,2047}, {15,255,255,65535,2047,2047}, {12,30,255,65535,2047,2047},  {12,30,6,65535,2047,2047},
		{12,30,56,600,2047,2047}, {12,30,56,809,900,2047}, {12,30,56,800,900,200}, {11,20,30,10,90,40}, {11,1,2,1,9,4}, 
		{15,25,0,0,0,0}, {12,36,40,0,0,0}, {10,20,3,0,0,0}, {12,36,40,200,0,0}, {12,36,40,20,0,0}, 
		{12,36,40,2,0,0}, {11,22,33,400,500,0}, {11,22,33,400,50,0}, {11,22,33,400,5,0}, {11,22,33,400,5,700}, 
		{11,22,33,400,5,70}, {11,22,33,400,5,7}, {2,2,2,2,2,2}
	};

	char expTimeTestOutRssl[23][56] = {
		"00:00:00:000:000:000", "",  "15", "12:30", "12:30:06",
		"12:30:56:600", "12:30:56:809:900", "12:30:56:800:900:200", "11:20:30:010:090:040", "11:01:02:001:009:004",
		"15:25:00:000:000:000", "12:36:40:000:000:000", "10:20:03:000:000:000", "12:36:40:200:000:000", "12:36:40:020:000:000",
		"12:36:40:002:000:000", "11:22:33:400:500:000", "11:22:33:400:050:000", "11:22:33:400:005:000", "11:22:33:400:005:700",
		"11:22:33:400:005:070", "11:22:33:400:005:007", "02:02:02:002:002:002"
	};
	char expTimeTestOutIso8601[23][56] = {
		"00:00:00", "", "15", "12:30", "12:30:06",
		"12:30:56.6", "12:30:56.8099", "12:30:56.8009002", "11:20:30.01009004", "11:01:02.001009004",
		"15:25:00", "12:36:40", "10:20:03", "12:36:40.2", "12:36:40.02",
		"12:36:40.002", "11:22:33.4005", "11:22:33.40005", "11:22:33.400005", "11:22:33.4000057",
		"11:22:33.40000507", "11:22:33.400005007", "02:02:02.002002002"
	};

	RsslDateTime iDateTime;
	int i = 0;
	int j = 0;
	char oDateTime[DATETIME_TEST_STRLEN];
	RsslBuffer oDateTimeBuf;
	char expDateTime[DATETIME_TEST_STRLEN];
	RsslBuffer expDateTimeBuf;
	char testName[DATETIME_TEST_STRLEN];

	oDateTimeBuf.data = &(oDateTime[0]);
	oDateTimeBuf.length = DATETIME_TEST_STRLEN;

	/* Test Date */
	for (i = 0; i < 12; ++i)
	{
		snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format ", dateTestName[i]);
		expDateTimeBuf.data = expDateTestOutRssl[i];
		expDateTimeBuf.length = (RsslUInt32) strlen(expDateTestOutRssl[i]);
		oDateTimeBuf.length = DATETIME_TEST_STRLEN;
		iDateTime.date = iDate[i];
		if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_DATE, &iDateTime, RSSL_STR_DATETIME_RSSL) == RSSL_RET_SUCCESS)
		{
			if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
				ASSERT_TRUE(RSSL_TRUE);
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format -ExpDateString != outputDateString ", dateTestName[i]);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
		else
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format -RSSL_RET_FAIL %s ", dateTestName[i], oDateTimeBuf.data);
			ASSERT_TRUE(RSSL_FALSE);
		}

		/* Test ISO8601 Date */
		snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 ", dateTestName[i]);
		expDateTimeBuf.data = expDateTestOutIso8601[i];
		expDateTimeBuf.length = (RsslUInt32) strlen(expDateTestOutIso8601[i]);
		oDateTimeBuf.length = DATETIME_TEST_STRLEN;
		iDateTime.date = iDate[i];
		if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_DATE, &iDateTime, RSSL_STR_DATETIME_ISO8601) == RSSL_RET_SUCCESS)
		{
			if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
				ASSERT_TRUE(RSSL_TRUE);
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 Format -ExpDateString != outputDateString ", dateTestName[i]);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
		else
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 Format -RSSL_RET_FAIL %s ", dateTestName[i], oDateTimeBuf.data);
			ASSERT_TRUE(RSSL_FALSE);
		}
	}

	/* Test Time */
	for (i = 0; i < 23; ++i)
	{
		snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format ", timeTestName[i]);
		expDateTimeBuf.data = expTimeTestOutRssl[i];
		expDateTimeBuf.length = (RsslUInt32) strlen(expTimeTestOutRssl[i]);
		oDateTimeBuf.length = DATETIME_TEST_STRLEN;
		iDateTime.time = iTime[i];

		if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_TIME, &iDateTime, RSSL_STR_DATETIME_RSSL) == RSSL_RET_SUCCESS)
		{
			if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
				ASSERT_TRUE(RSSL_TRUE);
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format -ExpTimeString != outputTimeString ", timeTestName[i]);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
		else
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s RSSL Format -RSSL_RET_FAIL %s ", timeTestName[i], oDateTimeBuf.data);
			ASSERT_TRUE(RSSL_FALSE);
		}
		 
		/* Test ISO8601 Time */
		snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 ", timeTestName[i]);
		expDateTimeBuf.data = expTimeTestOutIso8601[i];
		expDateTimeBuf.length = (RsslUInt32) strlen(expTimeTestOutIso8601[i]);
		oDateTimeBuf.length = DATETIME_TEST_STRLEN;
		iDateTime.time = iTime[i];
		if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_TIME, &iDateTime, RSSL_STR_DATETIME_ISO8601) == RSSL_RET_SUCCESS)
		{
			if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
				ASSERT_TRUE(RSSL_TRUE);
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 -ExpTimeString != outputTimeString ", timeTestName[i]);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
		else
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s ISO8601 -RSSL_RET_FAIL %s ", timeTestName[i], oDateTimeBuf.data);
			ASSERT_TRUE(RSSL_FALSE);
		}
	}

	/* Test Strf DateTime */
	expDateTimeBuf.data = &(expDateTime[0]);
	for (i = 0; i < 12; ++i)
	{
		iDateTime.date = iDate[i];
		for (j = 0; j < 23; ++j)
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s RSSL Format ", dateTestName[i], timeTestName[j]);
			iDateTime.time = iTime[j];
			expDateTimeBuf.length = snprintf(expDateTime, DATETIME_TEST_STRLEN, "%s %s", expDateTestOutRssl[i], expTimeTestOutRssl[j]);
			if(expDateTimeBuf.length == 1)
				expDateTimeBuf.length = 0; /* Due to space addition between date & time */
			oDateTimeBuf.length = DATETIME_TEST_STRLEN;
			if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_DATETIME, &iDateTime, RSSL_STR_DATETIME_RSSL) == RSSL_RET_SUCCESS)
			{
				if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
					ASSERT_TRUE(RSSL_TRUE);
				else
				{
					snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s RSSL Format -ExpDateTimeString != outputDateTimeString ", dateTestName[i], timeTestName[j]);
					ASSERT_TRUE(RSSL_FALSE);
				}
			}
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s RSSL Format -RSSL_RET_FAIL %s ", dateTestName[i], timeTestName[j], oDateTimeBuf.data);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
	}

	/* Test ISO8601 DateTime */;
	expDateTimeBuf.data = &(expDateTime[0]);
	for (i = 0; i < 12; ++i)
	{
		iDateTime.date = iDate[i];
		for (j = 0; j < 23; ++j)
		{
			snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s ISO8601 ", dateTestName[i], timeTestName[j]);
			iDateTime.time = iTime[j];
			expDateTimeBuf.length = snprintf(expDateTime, DATETIME_TEST_STRLEN, "%sT%s", expDateTestOutIso8601[i], expTimeTestOutIso8601[j]);
			if(expDateTimeBuf.data[expDateTimeBuf.length -1] == 'T')
				expDateTimeBuf.length -= 1; /* Due to T addition between date & time*/
			oDateTimeBuf.length = DATETIME_TEST_STRLEN;
			if( rsslDateTimeToStringFormat(&oDateTimeBuf, RSSL_DT_DATETIME, &iDateTime, RSSL_STR_DATETIME_ISO8601) == RSSL_RET_SUCCESS)
			{
				if(rsslBufferIsEqual(&expDateTimeBuf, &oDateTimeBuf))
					ASSERT_TRUE(RSSL_TRUE);
				else
				{
					snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s ISO8601 -ExpDateTimeString != outputDateTimeString ", dateTestName[i], timeTestName[j]);
					ASSERT_TRUE(RSSL_FALSE);
				}
			}
			else
			{
				snprintf(testName, DATETIME_TEST_STRLEN, "%s_%s ISO8601 -RSSL_RET_FAIL %s ", dateTestName[i], timeTestName[j], oDateTimeBuf.data);
				ASSERT_TRUE(RSSL_FALSE);
			}
		}
	}
}

TEST(dateTimeStringToDateTimeTest,dateTimeStringToDateTimeTest)
{
	/* Date  */ 
	char dateTestName[7][72] = {		
		"Date", "Date w space", "ISO8601 Date", "ISO8601 Year Month", "ISO8601 Date w/o hyphen", "--MM-DD", "--MMDD"
	};
	char dateFormat[7][56] = { 
		"%2d/%2d/%4d", "%2d %2d %4d", "%04d-%02d-%02d", "%4d-%02d", "%04d%02d%02d", "--%2d-%2d", "--%02d%02d"
	};
	RsslDate iDate[7] = {
		{21,10,1978}, {15,12,1998}, {27,6,2008}, {0,1,2009}, {25,5,2010}, {21,10,0}, {13,5,0}
	};
	RsslDate oDate;

	/* Time  */ 
	char timeTestName[28][72] = {		
		"Time w Milli", "Time w/o Sec", "Time w space ", "ISO8601 Time w/o: MinOnly", "ISO8601 Time w/o: SecOnly",
		"ISO8601 DeciTime w/o: 1MilliOnly", "ISO8601 DeciTime w/o: 2MilliOnly", "ISO8601 DeciTime w/o: 3MilliOnly", "ISO8601 DeciTime w/o: 1MicroOnly", "ISO8601 DeciTime w/o: 2MicroOnly",
		"ISO8601 DeciTime w/o: 3MicroOnly", "ISO8601 DeciTime w/o: NanoPadZero", "ISO8601 DeciTime w/o: Nano", "ISO8601 ComaTime w/o: 1MilliOnly", "ISO8601 ComaTime w/o: 2MilliOnly",
		"ISO8601 ComaTime w/o: 3MilliOnly", "ISO8601 ComaTime w/o: 1MicroOnly", "ISO8601 ComaTime w/o: 2MicroOnly", "ISO8601 ComaTime w/o: 3MicroOnly",	"ISO8601 ComaTime w/o: NanoPadZero", 
		"ISO8601 ComaTime w/o: Nano", 	"ISO8601 DeciTime w 1Nano", "ISO8601 DeciTime w 2Nano", "ISO8601 DeciTime w 3Nano", 
		"ISO8601 ComaTime w 1Nano", "ISO8601 ComaTime w 2Nano", "ISO8601 ComaTime w 3Nano", "ISO8601 no micro or nano"
	};
	char timeFormat[28][56] = { 
		"%2d:%02d:%02d:%03d", "%2d:%02d", "%2d %2d %2d %03d %03d %03d", "%02d%02d", "%02d%02d%02d", 
		"%02d%02d%02d.%03d", "%02d%02d%02d.%02d", "%02d%02d%02d.%03d", "%02d%02d%02d.%03d%03d", "%02d%02d%02d.%03d%03d", 
		"%02d%02d%02d.%03d%03d", "%02d%02d%02d.%03d%03d%03d", "%02d%02d%02d.%03d%03d%03d", "%02d%02d%02d,%03d", "%02d%02d%02d,%03d",
		"%02d%02d%02d,%03d", "%02d%02d%02d,%03d%03d", "%02d%02d%02d.%03d%03d",  "%02d%02d%02d.%03d%03d", "%02d%02d%02d,%03d%03d%03d", 
		"%02d%02d%02d,%03d%03d%03d", "%02d%02d%02d.%03d%03d%03d", "%02d%02d%02d.%03d%03d%03d","%02d%02d%02d.%03d%03d%03d", "%02d%02d%02d,%03d%03d%03d", 
		"%02d%02d%02d,%03d%03d%03d","%02d%02d%02d,%03d%03d%03d", "%02d:%02d:%02d"
	};
	RsslTime iTime[28] = {
		{5,02,00,506,0,0}, {10,15,0,0,0,0}, {12,40,36,123,456,989}, {15,25,0,0,0,0}, {12,36,45,0,0,0},
		{11,22,33,400,0,0}, {11,22,33,440,0,0}, {11,22,33,444,0,0}, {9,59,57,006,500,0}, {9,44,57,9,550,0},
		{9,44,57,9,555,0}, {9,59,57,006,050,001}, {11,22,33,444,555,678}, {10,25,15,900,0,0}, {10,25,15,990,0,0},
		{10,25,15,999,0,0}, {00,03,60,002,5,0}, {00,03,60,002,56,0}, {00,03,60,002,568,0},	{9,59,57,070,9,001}, 
		{11,22,33,644,755,677}, {12,40,36,213,456,800},	{05,18,42,312,654,880}, {05,18,42,312,654,888},	{13,43,36,233,456,3},
		{5,16,35,312,654,16},  {9,18,24,312,654,256}, {5,14,13,0,0,0}
	};
	RsslTime oTime;

	int i = 0;
	int j = 0;
	int t = 0;
	char dateTimeStr[256];
	char testNameWithIput[256];
	RsslBuffer dateTimeStrBuf; 	
	RsslDateTime iDateTime;
	RsslDateTime oDateTime;
	char dateTimeFormat[128];

	for (i = 0; i < 7; ++i)
	{ /* Date Tests  */ 
		if( i < 2 )
			dateTimeStrBuf.length=  (RsslUInt) snprintf(dateTimeStr, 256, dateFormat[i], iDate[i].month, iDate[i].day, iDate[i].year);
		else if (i == 5 || i == 6) /* Month & day only */
			dateTimeStrBuf.length=  (RsslUInt) snprintf(dateTimeStr, 256, dateFormat[i], iDate[i].month, iDate[i].day);
		else if(i == 3)
			dateTimeStrBuf.length = (RsslUInt)snprintf(dateTimeStr, 256, dateFormat[i], iDate[i].year, iDate[i].month);
		else
			dateTimeStrBuf.length= (RsslUInt) snprintf(dateTimeStr, 256, dateFormat[i], iDate[i].year, iDate[i].month, iDate[i].day);

		dateTimeStr[dateTimeStrBuf.length] = '\0';
		dateTimeStrBuf.data = &dateTimeStr[0];
		snprintf(testNameWithIput, 256,"%s data=\"%s\"\n", dateTestName[i], dateTimeStr);
		ASSERT_TRUE(rsslDateStringToDate(&oDate, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslDateIsEqual(&(iDate[i]), &oDate));
	}

	for (i = 0; i < 28; ++i)
	{ /* Time Tests  */ 
		dateTimeStrBuf.length=  (RsslUInt) snprintf(dateTimeStr, 256, timeFormat[i], iTime[i].hour, iTime[i].minute, iTime[i].second,
																				iTime[i].millisecond, iTime[i].microsecond,  iTime[i].nanosecond);
		dateTimeStr[dateTimeStrBuf.length] = '\0';
		dateTimeStrBuf.data = &dateTimeStr[0];
		snprintf(testNameWithIput, 256,"%s data=\"%s\"\n", timeTestName[i], dateTimeStr);
		rsslClearTime(&oTime);
		ASSERT_TRUE(rsslTimeStringToTime(&oTime, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslTimeIsEqual(&(iTime[i]), &oTime));

		/* Include the trailing null character */
		dateTimeStrBuf.length++;
		snprintf(testNameWithIput, 256, "%s TruncTrailZeros data=\"%s\"\n", timeTestName[i], dateTimeStr);
		snprintf(testNameWithIput, 256, "%s data=\"%s\"\n", timeTestName[i], dateTimeStr);
		rsslClearTime(&oTime);
		ASSERT_TRUE(rsslTimeStringToTime(&oTime, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslTimeIsEqual(&(iTime[i]), &oTime));
		
		/* Truncate trailing Zeros */
		for (j = dateTimeStrBuf.length -1;  j >=0 ; --j)
		{
			if (dateTimeStrBuf.data[j] == '0')
			{
				dateTimeStrBuf.data[j] = '\0';
				dateTimeStrBuf.length--;
			}
			else
				break;
		}
		snprintf(testNameWithIput, 256,"%s TruncTrailZeros data=\"%s\"\n", timeTestName[i], dateTimeStr);
		snprintf(testNameWithIput, 256, "%s data=\"%s\"\n", timeTestName[i], dateTimeStr);
		rsslClearTime(&oTime);
		ASSERT_TRUE(rsslTimeStringToTime(&oTime, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
		ASSERT_TRUE(rsslTimeIsEqual(&(iTime[i]), &oTime));
	}

	for (i = 0; i < 2; ++i)
	{ /* DateTime Tests Strf */
		rsslClearDateTime(&iDateTime);
		iDateTime.date = iDate[i];
		for (t = 0; t < 3; ++t)
		{
			snprintf(dateTimeFormat,128, "%s %s", dateFormat[i], timeFormat[t]);
			iDateTime.time = iTime[t];
			dateTimeStrBuf.length=  (RsslUInt) snprintf(dateTimeStr, 256, dateTimeFormat, iDateTime.date.month, iDateTime.date.day, iDateTime.date.year,
														iDateTime.time.hour, iDateTime.time.minute, iDateTime.time.second,
														iDateTime.time.millisecond, iDateTime.time.microsecond,  iDateTime.time.nanosecond);
			dateTimeStr[dateTimeStrBuf.length] = '\0';
		    dateTimeStrBuf.data = &dateTimeStr[0];
			snprintf(testNameWithIput, 256,"%s,%s data=\"%s\"\n", dateTestName[i], timeTestName[t], dateTimeStr);			
			
			rsslClearDateTime(&oDateTime);
			ASSERT_TRUE(rsslDateTimeStringToDateTime(&oDateTime, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(rsslDateTimeIsEqual(&(iDateTime), &oDateTime));
		}
	}

	for (i = 2; i < 7; ++i)
	{   /* DateTime Tests ISO8601 */
		rsslClearDateTime(&iDateTime);
		iDateTime.date = iDate[i];
		for (t = 3; t < 27; ++t)
		{
			snprintf(dateTimeFormat,128, "%sT%s", dateFormat[i], timeFormat[t]);
			iDateTime.time = iTime[t];
			if (i == 3)
			{
				dateTimeStrBuf.length = (RsslUInt)snprintf(dateTimeStr, 256, dateTimeFormat, iDateTime.date.year, iDateTime.date.month,
					iDateTime.time.hour, iDateTime.time.minute, iDateTime.time.second,
					iDateTime.time.millisecond, iDateTime.time.microsecond, iDateTime.time.nanosecond);
			}
			else if(i == 5 || i == 6)
				dateTimeStrBuf.length = (RsslUInt)snprintf(dateTimeStr, 256, dateTimeFormat, iDateTime.date.month, iDateTime.date.day,
					iDateTime.time.hour, iDateTime.time.minute, iDateTime.time.second,
					iDateTime.time.millisecond, iDateTime.time.microsecond, iDateTime.time.nanosecond);
			else
				dateTimeStrBuf.length=  (RsslUInt) snprintf(dateTimeStr, 256, dateTimeFormat, iDateTime.date.year, iDateTime.date.month, iDateTime.date.day, 
														iDateTime.time.hour, iDateTime.time.minute, iDateTime.time.second,
														iDateTime.time.millisecond, iDateTime.time.microsecond,  iDateTime.time.nanosecond);
			dateTimeStr[dateTimeStrBuf.length] = '\0';
		    dateTimeStrBuf.data = &dateTimeStr[0];
			snprintf(testNameWithIput, 256,"%s,%s data=\"%s\"\n", dateTestName[i], timeTestName[t], dateTimeStr);			

			rsslClearDateTime(&oDateTime);
			ASSERT_TRUE(rsslDateTimeStringToDateTime(&oDateTime, &dateTimeStrBuf) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(rsslDateTimeIsEqual(&(iDateTime), &oDateTime));
		}
	}	
}

void testCompareDoubleToReal(const RsslReal testReal, const RsslInt testIntVal)
{
	RsslInt diff;
	// maximum difference between MAX_VALUE/MIN_VALUE and real value 0 .. 2048 = 2^11
	// it is the precision difference between long(64bit) and double(53bit)
	if (testReal.value <= testIntVal)
		diff = testIntVal - testReal.value;
	else
		diff = testReal.value - testIntVal;

	if (diff == 0) {
		ASSERT_EQ(testReal.value, testIntVal);
	}
	else {
		ASSERT_TRUE(diff <= 2048);
	}
}

void testDoubleToRealConvert(const RsslDouble dFactor, const RsslRealHints rhExponent)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i / dFactor;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 123. / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testDouble = -123. / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123);

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1e30;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1e30;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testDouble = nextafter(testDouble, 1e30);  // get the next double value
	testDouble = nextafter(testDouble, 1e30);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testDouble = nextafter(testDouble, -1e30);  // get the next double value
	testDouble = nextafter(testDouble, -1e30);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	testCompareDoubleToReal(testReal, (std::numeric_limits<RsslInt>::min)());
}

TEST(realDoubleIntConvertTest, RealHintExponentAllTest)
{
	testDoubleToRealConvert(1e14, RSSL_RH_EXPONENT_14);
	testDoubleToRealConvert(1e13, RSSL_RH_EXPONENT_13);
	testDoubleToRealConvert(1e12, RSSL_RH_EXPONENT_12);
	testDoubleToRealConvert(1e11, RSSL_RH_EXPONENT_11);
	testDoubleToRealConvert(1e10, RSSL_RH_EXPONENT_10);
	testDoubleToRealConvert(1e9, RSSL_RH_EXPONENT_9);
	testDoubleToRealConvert(1e8, RSSL_RH_EXPONENT_8);
	testDoubleToRealConvert(1e7, RSSL_RH_EXPONENT_7);
	testDoubleToRealConvert(1e6, RSSL_RH_EXPONENT_6);
	testDoubleToRealConvert(1e5, RSSL_RH_EXPONENT_5);
	testDoubleToRealConvert(1e4, RSSL_RH_EXPONENT_4);
	testDoubleToRealConvert(1e3, RSSL_RH_EXPONENT_3);
	testDoubleToRealConvert(100., RSSL_RH_EXPONENT_2);
	testDoubleToRealConvert(10., RSSL_RH_EXPONENT_1);

	testDoubleToRealConvert(1., RSSL_RH_EXPONENT0);
	testDoubleToRealConvert(.1, RSSL_RH_EXPONENT1);
	testDoubleToRealConvert(.01, RSSL_RH_EXPONENT2);
	testDoubleToRealConvert(1e-3, RSSL_RH_EXPONENT3);
	testDoubleToRealConvert(1e-4, RSSL_RH_EXPONENT4);
	testDoubleToRealConvert(1e-5, RSSL_RH_EXPONENT5);
	testDoubleToRealConvert(1e-6, RSSL_RH_EXPONENT6);
	testDoubleToRealConvert(1e-7, RSSL_RH_EXPONENT7);

	testDoubleToRealConvert(1., RSSL_RH_FRACTION_1);
	testDoubleToRealConvert(2., RSSL_RH_FRACTION_2);
	testDoubleToRealConvert(4., RSSL_RH_FRACTION_4);
	testDoubleToRealConvert(8., RSSL_RH_FRACTION_8);
	testDoubleToRealConvert(16., RSSL_RH_FRACTION_16);
	testDoubleToRealConvert(32., RSSL_RH_FRACTION_32);
	testDoubleToRealConvert(64., RSSL_RH_FRACTION_64);
	testDoubleToRealConvert(128., RSSL_RH_FRACTION_128);
	testDoubleToRealConvert(256., RSSL_RH_FRACTION_256);
}

void testCompareFloatToReal(const RsslReal testReal, const RsslInt testIntVal)
{
	RsslInt diff;
	// maximum difference between MAX_VALUE/MIN_VALUE and real value 0 .. 2^40
	// it is the precision difference between long(64bit) and float(24bit)
	if (testReal.value <= testIntVal)
		diff = testIntVal - testReal.value;
	else
		diff = testReal.value - testIntVal;

	if (diff == 0) {
		ASSERT_EQ(testReal.value, testIntVal);
	}
	else {
		ASSERT_TRUE(diff <= 1099511627776L);
	}
}

void testFloatToRealConvert(const RsslFloat dFactor, const RsslRealHints rhExponent)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i / dFactor;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 123.f / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testFloat = -123.f / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123);

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1e30f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1e30f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);


	// tests maximum
	// when conversion return success then verify result
	// conversion may return a fail status due rounding errors
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::max)());
	}

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor);
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::max)());
	}

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testFloat = nextafterf(testFloat, 1e30f);  // get the next float value
	testFloat = nextafterf(testFloat, 1e30f);  // get the next float value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::max)());
	}

	// tests minimum
	// when conversion return success then verify result
	// conversion may return a fail status due rounding errors
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::min)());
	}

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor);
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::min)());
	}

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testFloat = nextafterf(testFloat, -1e30f);  // get the next float value
	testFloat = nextafterf(testFloat, -1e30f);  // get the next float value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	if (rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS)
	{
		ASSERT_EQ(testReal.hint, rhExponent);
		testCompareFloatToReal(testReal, (std::numeric_limits<RsslInt>::min)());
	}
}

TEST(realFloatIntConvertTest, RealHintExponentAllTest)
{
	testFloatToRealConvert(1e14f, RSSL_RH_EXPONENT_14);
	testFloatToRealConvert(1e13f, RSSL_RH_EXPONENT_13);
	testFloatToRealConvert(1e12f, RSSL_RH_EXPONENT_12);
	testFloatToRealConvert(1e11f, RSSL_RH_EXPONENT_11);
	testFloatToRealConvert(1e10f, RSSL_RH_EXPONENT_10);
	testFloatToRealConvert(1e9f, RSSL_RH_EXPONENT_9);
	testFloatToRealConvert(1e8f, RSSL_RH_EXPONENT_8);
	testFloatToRealConvert(1e7f, RSSL_RH_EXPONENT_7);
	testFloatToRealConvert(1e6f, RSSL_RH_EXPONENT_6);
	testFloatToRealConvert(1e5f, RSSL_RH_EXPONENT_5);
	testFloatToRealConvert(1e4f, RSSL_RH_EXPONENT_4);
	testFloatToRealConvert(1e3f, RSSL_RH_EXPONENT_3);
	testFloatToRealConvert(100.f, RSSL_RH_EXPONENT_2);
	testFloatToRealConvert(10.f, RSSL_RH_EXPONENT_1);

	testFloatToRealConvert(1.f, RSSL_RH_EXPONENT0);
	testFloatToRealConvert(.1f, RSSL_RH_EXPONENT1);
	testFloatToRealConvert(.01f, RSSL_RH_EXPONENT2);
	testFloatToRealConvert(1e-3f, RSSL_RH_EXPONENT3);
	testFloatToRealConvert(1e-4f, RSSL_RH_EXPONENT4);
	testFloatToRealConvert(1e-5f, RSSL_RH_EXPONENT5);
	testFloatToRealConvert(1e-6f, RSSL_RH_EXPONENT6);
	testFloatToRealConvert(1e-7f, RSSL_RH_EXPONENT7);

	testFloatToRealConvert(1.f, RSSL_RH_FRACTION_1);
	testFloatToRealConvert(2.f, RSSL_RH_FRACTION_2);
	testFloatToRealConvert(4.f, RSSL_RH_FRACTION_4);
	testFloatToRealConvert(8.f, RSSL_RH_FRACTION_8);
	testFloatToRealConvert(16.f, RSSL_RH_FRACTION_16);
	testFloatToRealConvert(32.f, RSSL_RH_FRACTION_32);
	testFloatToRealConvert(64.f, RSSL_RH_FRACTION_64);
	testFloatToRealConvert(128.f, RSSL_RH_FRACTION_128);
	testFloatToRealConvert(256.f, RSSL_RH_FRACTION_256);
}

TEST(realDoubleIntConvertTest, RealHintExponent0Test)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	const RsslRealHints rhExponent = RSSL_RH_EXPONENT0;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests a special user case: conversion should be fail because out of range
	rsslClearReal(&testReal);
	testDouble = 1000000000.0;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, RSSL_RH_EXPONENT_10) == RSSL_RET_FAILURE);

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 1000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000);

	rsslClearReal(&testReal);
	testDouble = -1000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000);

	rsslClearReal(&testReal);
	testDouble = 1000000000000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000000000000);

	rsslClearReal(&testReal);
	testDouble = -1000000000000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000000000000);

	rsslClearReal(&testReal);
	testDouble = 12.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12);

	rsslClearReal(&testReal);
	testDouble = 123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testDouble = 1234.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234);

	rsslClearReal(&testReal);
	testDouble = 12345.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345);

	rsslClearReal(&testReal);
	testDouble = 123456.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456);

	rsslClearReal(&testReal);
	testDouble = 1234567.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567);

	rsslClearReal(&testReal);
	testDouble = 12345678.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678);

	rsslClearReal(&testReal);
	testDouble = 123456789.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789);

	rsslClearReal(&testReal);
	testDouble = 1234567890.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890);

	rsslClearReal(&testReal);
	testDouble = 12345678901.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678901);

	rsslClearReal(&testReal);
	testDouble = 123456789012.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789012);

	rsslClearReal(&testReal);
	testDouble = 1234567890123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890123);

	rsslClearReal(&testReal);
	testDouble = 12345678901234.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678901234);

	rsslClearReal(&testReal);
	testDouble = 123456789012345.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789012345);

	rsslClearReal(&testReal);
	testDouble = 1234567890123456.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890123456);

	rsslClearReal(&testReal);
	testDouble = 12345678901234567.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 12345678901234567); // is not equal: precise of double is lower to int64

	rsslClearReal(&testReal);
	testDouble = 123456789012345678.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 123456789012345678); // is not equal: precise of double is lower to int64

	rsslClearReal(&testReal);
	testDouble = 1234567890123456789.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 1234567890123456789); // is not equal: precise of double is lower to int64

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	testDouble += 1000.;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)()));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.));
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	testDouble = nextafter(testDouble, 1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)() - 1;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	testDouble -= 1000.;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)()));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.));
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	testDouble = nextafter(testDouble, -1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)() + 1;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realDoubleIntConvertTest, RealHintExponent_14Test)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	const RsslInt iFactor = 100000000000000;
	const RsslDouble dFactor = 100000000000000.;
	const RsslRealHints rhExponent = RSSL_RH_EXPONENT_14;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i / dFactor;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123 * iFactor);

	rsslClearReal(&testReal);
	testDouble = -123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123 * iFactor);

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);


	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testDouble = nextafter(testDouble, 1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testDouble = nextafter(testDouble, -1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realDoubleIntConvertTest, RealHintExponent7Test)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	const RsslInt iFactor = 10000000;
	const RsslDouble dFactor = 10000000.;
	const RsslRealHints rhExponent = RSSL_RH_EXPONENT7;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i * dFactor;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 123. * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testDouble = -123. * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123);

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1e30;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1e30;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() + 1000.) * dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() * dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.) * dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() * dFactor;
	testDouble = nextafter(testDouble, 1e30);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() - 1) * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() - 1000.) * dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() * dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.) * dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() * dFactor;
	testDouble = nextafter(testDouble, -1e30);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() + 1) * dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realDoubleIntConvertTest, RealHintFraction_1Test)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	const RsslRealHints rhExponent = RSSL_RH_FRACTION_1;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 1000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000);

	rsslClearReal(&testReal);
	testDouble = -1000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000);

	rsslClearReal(&testReal);
	testDouble = 1000000000000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000000000000);

	rsslClearReal(&testReal);
	testDouble = -1000000000000000000.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000000000000);

	rsslClearReal(&testReal);
	testDouble = 12.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12);

	rsslClearReal(&testReal);
	testDouble = 123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testDouble = 1234.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234);

	rsslClearReal(&testReal);
	testDouble = 12345.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345);

	rsslClearReal(&testReal);
	testDouble = 123456.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456);

	rsslClearReal(&testReal);
	testDouble = 1234567.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567);

	rsslClearReal(&testReal);
	testDouble = 12345678.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678);

	rsslClearReal(&testReal);
	testDouble = 123456789.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789);

	rsslClearReal(&testReal);
	testDouble = 1234567890.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890);

	rsslClearReal(&testReal);
	testDouble = 12345678901.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678901);

	rsslClearReal(&testReal);
	testDouble = 123456789012.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789012);

	rsslClearReal(&testReal);
	testDouble = 1234567890123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890123);

	rsslClearReal(&testReal);
	testDouble = 12345678901234.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678901234);

	rsslClearReal(&testReal);
	testDouble = 123456789012345.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456789012345);

	rsslClearReal(&testReal);
	testDouble = 1234567890123456.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567890123456);

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	testDouble += 1000.;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)()));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.));
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)();
	testDouble = nextafter(testDouble, 1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::max)() - 1;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	testDouble -= 1000.;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)()));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.));
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)();
	testDouble = nextafter(testDouble, -1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = (RsslDouble)(std::numeric_limits<RsslInt>::min)() + 1;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realDoubleIntConvertTest, RealHintFraction_256Test)
{
	RsslDouble testDouble;
	RsslReal testReal;
	int i;

	const RsslInt iFactor = 256;
	const RsslDouble dFactor = 256.;
	const RsslRealHints rhExponent = RSSL_RH_FRACTION_256;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testDouble = i / dFactor;
		ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testDouble = 123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123 * iFactor);

	rsslClearReal(&testReal);
	testDouble = -123.;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123 * iFactor);

	// tests out of range values
	rsslClearReal(&testReal);
	testDouble = 1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = -1e20;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::max)() + 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testDouble = nextafter(testDouble, 1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor;
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testDouble, ((std::numeric_limits<RsslInt>::min)() - 1000.) / dFactor);
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testDouble = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testDouble = nextafter(testDouble, -1e20);  // get the next double value
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testDouble = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	ASSERT_TRUE(rsslDoubleToReal(&testReal, &testDouble, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realFloatIntConvertTest, RealHintExponent0Test)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;

	const RsslRealHints rhExponent = RSSL_RH_EXPONENT0;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 1000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000);

	rsslClearReal(&testReal);
	testFloat = -1000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000);

	rsslClearReal(&testReal);
	testFloat = 1000000000000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_TRUE(testReal.value > 0);
	//ASSERT_EQ(testReal.value, 1000000000000000000); // it's not equal: precise of float is lower to int64

	rsslClearReal(&testReal);
	testFloat = -1000000000000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_TRUE(testReal.value < 0);
	//ASSERT_EQ(testReal.value, -1000000000000000000); // it's not equal: precise of float is lower to int64

	rsslClearReal(&testReal);
	testFloat = 12.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12);

	rsslClearReal(&testReal);
	testFloat = 123.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testFloat = 1234.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234);

	rsslClearReal(&testReal);
	testFloat = 12345.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345);

	rsslClearReal(&testReal);
	testFloat = 123456.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456);

	rsslClearReal(&testReal);
	testFloat = 1234567.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567);

	rsslClearReal(&testReal);
	testFloat = 12345678.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678);

	rsslClearReal(&testReal);
	testFloat = 123456789.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 123456789); // it's not equal: precise of float is lower to int64

	rsslClearReal(&testReal);
	testFloat = 1234567890.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 1234567890); // it's not equal: precise of float is lower to int64

	rsslClearReal(&testReal);
	testFloat = 1234567890123456789.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 1234567890123456789); // is not equal: precise of double is lower to int64

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);


	// tests maximum
	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	testFloat += 1000.f;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)()));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f));
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	testFloat = nextafterf(testFloat, 1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)() - 1;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	testFloat -= 1000.f;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)()));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f));
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	testFloat = nextafterf(testFloat, -1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)() + 1;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realFloatIntConvertTest, RealHintExponent_14Test)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;
	//double d;

	const RsslInt iFactor = 100000000000000;
	const RsslFloat dFactor = 100000000000000.f;
	const RsslRealHints rhExponent = RSSL_RH_EXPONENT_14;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i / dFactor;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 0.0000000123f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (RsslInt)(0.0000000123 * iFactor));

	rsslClearReal(&testReal);
	testFloat = -0.0000000123f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (RsslInt)(-0.0000000123 * iFactor));

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);


	// tests maximum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testFloat = nextafterf(testFloat, 1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testFloat = nextafterf(testFloat, -1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realFloatIntConvertTest, RealHintExponent7Test)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;

	const RsslInt iFactor = 10000000;
	const RsslFloat dFactor = 10000000.f;
	const RsslRealHints rhExponent = RSSL_RH_EXPONENT7;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i * dFactor;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 123.f * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testFloat = -123.f * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123);

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1e30f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1e30f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() + 1000.f) * dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() * dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f) * dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() * dFactor;
	testFloat = nextafterf(testFloat, 1e30f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() - 1) * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() - 1000.f) * dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() * dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f) * dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() * dFactor;
	testFloat = nextafterf(testFloat, -1e30f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() + 1) * dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realFloatIntConvertTest, RealHintFraction_1Test)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;

	const RsslRealHints rhExponent = RSSL_RH_FRACTION_1;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 1000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1000000000);

	rsslClearReal(&testReal);
	testFloat = -1000000000.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -1000000000);

	rsslClearReal(&testReal);
	testFloat = 12.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12);

	rsslClearReal(&testReal);
	testFloat = 123.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123);

	rsslClearReal(&testReal);
	testFloat = 1234.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234);

	rsslClearReal(&testReal);
	testFloat = 12345.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345);

	rsslClearReal(&testReal);
	testFloat = 123456.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123456);

	rsslClearReal(&testReal);
	testFloat = 1234567.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 1234567);

	rsslClearReal(&testReal);
	testFloat = 12345678.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 12345678);

	rsslClearReal(&testReal);
	testFloat = 123456789.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 123456789); // it's not equal: precise of float is lower to int64

	rsslClearReal(&testReal);
	testFloat = 1234567890123456.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	//ASSERT_EQ(testReal.value, 1234567890123456); // it's not equal: precise of float is lower to int64

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	testFloat += 1000.f;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)()));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f));
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)();
	testFloat = nextafterf(testFloat, 1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::max)() - 1;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	testFloat -= 1000.f;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)()));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f));
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)();
	testFloat = nextafterf(testFloat, -1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = (RsslFloat)(std::numeric_limits<RsslInt>::min)() + 1;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(realFloatIntConvertTest, RealHintFraction_256Test)
{
	RsslFloat testFloat;
	RsslReal testReal;
	short i;

	const RsslInt iFactor = 256;
	const RsslFloat dFactor = 256.f;
	const RsslRealHints rhExponent = RSSL_RH_FRACTION_256;

	// tests digits -9 .. 0 .. 9
	for (i = -9; i < 10; i++)
	{
		rsslClearReal(&testReal);
		testFloat = i / dFactor;
		ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
		ASSERT_EQ(testReal.hint, rhExponent);
		ASSERT_EQ(testReal.value, i);
	}

	// tests middle in range values
	rsslClearReal(&testReal);
	testFloat = 123.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, 123 * iFactor);

	rsslClearReal(&testReal);
	testFloat = -123.f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, -123 * iFactor);

	// tests out of range values
	rsslClearReal(&testReal);
	testFloat = 1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = -1e20f;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	// tests maximum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::max)() + 1000.f) / dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::max)() / dFactor;
	testFloat = nextafterf(testFloat, 1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::max)() - 1) / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::max)());

	// tests minimum
	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor;
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() / dFactor));
	ASSERT_EQ(testFloat, ((std::numeric_limits<RsslInt>::min)() - 1000.f) / dFactor);
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());

	rsslClearReal(&testReal);
	testFloat = (std::numeric_limits<RsslInt>::min)() / dFactor;
	testFloat = nextafterf(testFloat, -1e20f);  // get the next double value
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_FAILURE);

	rsslClearReal(&testReal);
	testFloat = ((std::numeric_limits<RsslInt>::min)() + 1) / dFactor;
	ASSERT_TRUE(rsslFloatToReal(&testReal, &testFloat, rhExponent) == RSSL_RET_SUCCESS);
	ASSERT_EQ(testReal.hint, rhExponent);
	ASSERT_EQ(testReal.value, (std::numeric_limits<RsslInt>::min)());
}

TEST(partialUpdateTest, partialUpdateTest)
{
	partialUpdateTest();
}

TEST(controlParseTest, controlParseTest)
{
	controlParseTest();
}

TEST(bufferEdgeCaseTest, bufferEdgeCaseTest)
{
	bufferEdgeCaseTest();
}

TEST(deleteCharTest, deleteCharTest)
{
	deleteCharTest();
}

TEST(overflowTest, overflowTest)
{
	overflowTest();
}

const char
	*argToString = "--to-string";

int main(int argc, char* argv[])
{
	RsslUInt8 iArgs;
	RsslLibraryVersionInfo libVer = RSSL_INIT_LIBRARY_VERSION_INFO;

	/* Parse arguments */
	for ( iArgs = 0; iArgs < argc; ++iArgs)
	{
		if ( 0 == strcmp( argToString, argv[iArgs]))
			g_ToString = RSSL_TRUE;

	}


	rsslQueryDataLibraryVersion(&libVer);
	//printf("Data Library Version:\n \t%s\n \t%s\n \t%s", libVer.productVersion, libVer.internalVersion, libVer.productDate);


	//printf( "[%s] Test ToString(%s)\n",
	//	g_ToString ? "On " : "Off", argToString
	//	);

	rsslClearEncodeIterator(&encIter);
	rsslClearDecodeIterator(&decIter);
	/* took out these tests for assertions */



	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS(); 
	

}
