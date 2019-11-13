///*
// *|---------------------------------------------------------------
// *|		Copyright (C) 2019 Refinitiv,			  --
// *| All rights reserved. Duplication or distribution prohibited --
// *|---------------------------------------------------------------
// */

/* rsslMessageTest
 * Provides functionality and performance testing of the RSSL Message structures.
 * This test is actually an RSSL port of the Data test. It was made for two purposes:
 *   1. Provide a performance comparison
 *   2. Provide functional testing for the expected public release of RSSL.
 * To do functionality testing, simply run the test.
 * Command line options can be found in the main() function at the bottom.
 * Total time for each test is always provided. 
 * TEST is still used as the enumeration base name, but the values have been changed to match RSSL.
 */


#include <malloc.h>
#include <math.h>

#include "rtr/rsslRDM.h"
#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslDataUtils.h"

#ifdef WIN32
#define _VARIADIC_MAX 10 /* GoogleTest needs this to build properly w/ Visual Studio*/
#endif
#include "gtest/gtest.h"


/* For payload */

#include "rtr/rsslFieldList.h"
#include "rtr/encoderTools.h"

#ifdef WIN32
#include <windows.h>
#endif

/* Report Mode */ 

typedef enum
{
	TEST_REPORT_ALL,
	TEST_REPORT_FAIL,
	TEST_PERF_MODE
} reportModes;

reportModes g_reportMode;

RsslUInt64 failedTests = 0, testId = 0;

/* Buffer sizes */
const RsslUInt32 c_TestDataBufferSize = 1024;
const RsslUInt32 c_TestMsgBufferSize  = 1024 + 512;
const RsslUInt32 c_TestMsgCopyBufferSize = 2048;

/* Base msg */
RsslMsg msg;

/* Encode nested Msg */
RsslUInt8 g_dataFormat;

/* Skip Decoding if desired */
RsslBool g_testDecoding; 
RsslBool g_changeData = RSSL_FALSE;

/* ExtraActions 
 * - Additional flag set for testing situations additionally not covered by flags */
typedef enum {
	TEST_ACTION_POST_PAYLOAD  = 0x0001, /* Add post-encoded payload to the message (if PRE_PAYLOAD is not set) */
	TEST_ACTION_PRE_PAYLOAD   = 0x0002, /* Add a pre-encoded payload to the message */
	TEST_ACTION_4BYTE_SEQ     = 0x0004,  /* Test a sequence number with 4 bytes instead of 2 */ /* TODO is this needed in the RSSL version? */
	TEST_ACTION_TRIM_DATA_BUF = 0x0008, /* Test trimming the data buffer size to the actual encoded length */
	TEST_ACTION_POST_OPAQUE   = 0x0010, /* Test post encoding of opaque - only applies in EncodeMsgInit cases */
	TEST_ACTION_POST_EXTENDED = 0x0020, /* Test post encoding of extended header - only applies in EncodeMsgInit cases */
	TEST_ACTION_POST_REQKEY   = 0x0040  /* Test post encoding of opaque - only applies in EncodeMsgInit cases */
} ExtraAction;
#define TEST_ACTION_PAYLOAD (TEST_ACTION_POST_PAYLOAD | TEST_ACTION_PRE_PAYLOAD) /* For Decoder -- indicates payload whether pre or post-encoded */

#define TEST_MSGKEY_MAX_MASK 0x3F /* Highest flag == 0x20 */
#define TEST_STATE_MAX_MASK 0x3 /* Highest Flag: 0x02 */

/* Iterators and Buffers for writing/reading */
RsslBuffer encBuf, encDataBuf, encNestedBuf, copyMsgBuf, encTempBuf;
RsslEncodeIterator encIter, encDataIter;
RsslDecodeIterator decIter, decDataIter;

RsslUInt32 *masks, masksSize, *actions, actionsSize, *keyMasks, keyMasksSize;
RsslUInt32 masksIter, actionsIter, keyMasksIter;

/* Test values */

RsslInt32 streamId = 2;

char extendedHeader[] = "extendedHeader";
RsslUInt32 extendedHeaderLen = sizeof(extendedHeader)/sizeof(char);

char exchangeId[] = "NYSE";
RsslUInt32 exchangeIdLen = sizeof(exchangeId)/sizeof(char);

char permissionData[] = "permission";
RsslUInt32 permissionDataLen = sizeof(permissionData)/sizeof(char);

RsslUInt16 permissionEntity = 42;

char text[] = "Acknowledged";
RsslUInt32 textLen = sizeof(text)/sizeof(char);

char groupId[] = "3";
RsslUInt32 groupIdLen = sizeof(groupId)/sizeof(char);

char stateText[] = "Source Unavailable";
RsslUInt32 stateTextLen = sizeof(stateText)/sizeof(char);

RsslUInt32 seqNum = 12345;

RsslUInt32 postId = 123456;

/* Performance counter */
/* This code exists in both DataTest and MessageTest, so please copy any changes to the other */

RsslUInt64 g_totalTime = 0;
RsslBool g_success;

#if defined(WIN32)

#include <time.h>

LARGE_INTEGER g_freq, g_timeStart, g_timeEnd;
LARGE_INTEGER timeStart, timeEnd;

#define setTimer(timer) QueryPerformanceCounter(&(timer))
#define getTimer(timer) (timer).QuadPart
#define setFrequency(freq) QueryPerformanceFrequency(&freq);
#define getFrequency(freq) (freq).QuadPart


#elif defined(LINUX)

#include <time.h>

struct timespec g_timeStart, g_timeEnd;
struct timespec timeStart, timeEnd;
RsslDouble g_freq;

#define setTimer(timer) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &(timer))
#define getTimer(timer) ((timer).tv_sec * 1000000000ULL + timer.tv_nsec)
#define setFrequency(freq) freq = 1000000000;
#define getFrequency(freq) (freq)

#else /* Solaris */

#include <sys/time.h>

hrtime_t g_freq, g_timeStart, g_timeEnd;
hrtime_t timeStart, timeEnd;

#define setTimer(timer) ((timer) = gethrtime())
#define getTimer(timer) (timer)
#define setFrequency(freq) freq = 1000000000;
#define getFrequency(freq) (freq)

#endif

RTR_C_ALWAYS_INLINE void startTimer() 
{ 
	setTimer(g_timeStart); 
}

RTR_C_ALWAYS_INLINE void endTimerAndPrint()
{
	setTimer(g_timeEnd);
	g_totalTime = getTimer(g_timeEnd) - getTimer(g_timeStart);
	//printf("	Total time: %.9fs\n", (RsslDouble)g_totalTime / (RsslDouble)getFrequency(g_freq) );
}

/* test report functions */

void testReportFull(RsslBool success, char *title)
{
		++testId;

		if (success)
		{
				//if ( g_reportMode == TEST_REPORT_ALL )
					//printf( RTR_LLU "	Pass	%s\n", testId, title);

			//if ( !(testId & 0xFFFFFFF))
			//{
				//printf("...\n"); /* Keep printing -something- so we know the test is still going */
			//}
		}
		else
		{
			//if ( g_reportMode == TEST_REPORT_FAIL || g_reportMode == TEST_REPORT_ALL )		
				//printf( RTR_LLU "	Fail	%s\n", testId, title);
				

			++failedTests;
		}
}

//#define ASSERT_TRUE(exp, title) if ( g_reportMode == TEST_PERF_MODE ) (exp); else testReportFull(exp, title);
#define str(exp) #exp
//#define ASSERT_TRUE(exp) ASSERT_TRUE(exp, str(exp) )

void endReport()
{
	//printf("Test Results: " RTR_LLU " total, " RTR_LLU "  failed\n", testId, failedTests);
	//if (failedTests > 0)
		//printf("FAILED\n");
	//else
		//printf("PASSED\n");
}

void _setupEncodeIterator()
{
	encBuf.length = c_TestMsgBufferSize;
	rsslClearEncodeIterator(&encIter);
	rsslSetEncodeIteratorBuffer(&encIter, &encBuf);
}

void _setupEncodeDataIterator()
{
	encDataBuf.length = c_TestDataBufferSize;
	rsslClearEncodeIterator(&encDataIter);
	rsslSetEncodeIteratorBuffer(&encDataIter, &encDataBuf);
}

void _setupDecodeIterator()
{
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorBuffer(&decIter, &encBuf);
}

void _setupDecodeDataIterator()
{
	rsslClearDecodeIterator(&decDataIter);
}

/***** Decode/Encode Field List *****/
/* Copied from DataTest -- for use as a message payload */

void _encodeFieldList(RsslEncodeIterator* pEncIter);
void _decodeFieldList(RsslDecodeIterator *pDecIter);
void _encodeMsgKey(RsslMsgKey *pKey, char *opaque, RsslUInt32 opaqueLen);
void _decodeMsgKey(RsslDecodeIterator *pDecIter, RsslMsgKey *pKey);

/* FIDs and primitives for field list */
RsslFieldId fieldListFids[] =
{
	1,
	2,
	245,
	255,
	256,
	32767
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
RsslTime fieldListTime = { 23, 59, 59, 999 };
RsslDateTime fieldListDateTime = { {8, 3, 1892}, {23, 59, 59, 999} };

#define TEST_FIELD_LIST_MAX_ENTRIES 6 /* Using all primitives makes the top container too big to test nested structure to maximum depth */

char payloadName[] = "TRI.N";
RsslUInt32 payloadNameLen = sizeof(payloadName)/sizeof(char);

char msgKeyAttrib[1000];
RsslUInt32 msgKeyAttribLen = 1000;

char reqKeyAttrib[1000];
RsslUInt32 reqKeyAttribLen = 1000;

/* Encode a payload that is actually another message */
void _encodeNestedMsg(RsslEncodeIterator* pEncIter)
{
    RsslUpdateMsg updMsg;

    rsslClearUpdateMsg(&updMsg);

    updMsg.msgBase.msgClass = RSSL_MC_UPDATE;
    updMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
    updMsg.msgBase.streamId = streamId;
    updMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
    updMsg.flags = RSSL_UPMF_HAS_EXTENDED_HEADER | RSSL_UPMF_HAS_MSG_KEY;
    updMsg.updateType = 3;

    updMsg.extendedHeader.length = extendedHeaderLen;
    updMsg.extendedHeader.data = extendedHeader;

    updMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_IDENTIFIER | RSSL_MKF_HAS_ATTRIB;
    _encodeMsgKey(&updMsg.msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);

	
    ASSERT_TRUE(RSSL_RET_ENCODE_CONTAINER == rsslEncodeMsgInit(pEncIter, (RsslMsg*)&updMsg, 0)); //rsslEncodeMsgInit
    //ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeExtendedHeader(pEncIter, &extHdrBuf)); //rsslEncodeExtendedHeader
    _encodeFieldList(pEncIter);


    ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(pEncIter, RSSL_TRUE)); //rsslEncodeMsgComplete
}

void _decodeNestedMsg(RsslDecodeIterator *pDecIter)
{
    RsslMsg msg;

    rsslClearMsg(&msg);
    ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(pDecIter, &msg)); //nestedMsg DecodeMsg
    ASSERT_TRUE(msg.msgBase.containerType == RSSL_DT_FIELD_LIST); //nestedMsg containerType

    ASSERT_TRUE(msg.msgBase.msgClass == RSSL_MC_UPDATE); //nestedMsg msgClass
    ASSERT_TRUE(msg.msgBase.containerType == RSSL_DT_FIELD_LIST); //nestedMsg containerType
    ASSERT_TRUE(msg.msgBase.streamId == streamId); //nestedMsg streamId
    ASSERT_TRUE(msg.msgBase.domainType == RSSL_DMT_MARKET_PRICE); //nestedMsg domainType
    ASSERT_TRUE(msg.updateMsg.flags == (RSSL_UPMF_HAS_EXTENDED_HEADER | RSSL_UPMF_HAS_MSG_KEY)); //nestedMsg flags
    ASSERT_TRUE(msg.updateMsg.extendedHeader.length == extendedHeaderLen
            && 0 == strcmp(msg.updateMsg.extendedHeader.data, extendedHeader)); //extendedHeader
    ASSERT_TRUE(msg.updateMsg.updateType == 3); //nestedMsg updateType

    ASSERT_TRUE(msg.msgBase.msgKey.flags == (RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_IDENTIFIER | RSSL_MKF_HAS_ATTRIB)); //nestedMsg key flags
    _decodeMsgKey(pDecIter, &msg.msgBase.msgKey);

    _decodeFieldList(pDecIter);

}


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
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListInit(pEncIter, &fieldList, 0, 0)); //rsslEncodeFieldListInit

	/* add entries */
	for(iiEntry = 0; iiEntry < TEST_FIELD_LIST_MAX_ENTRIES ; ++iiEntry)
	{
		rsslClearFieldEntry(&entry);
		entry.fieldId = fieldListFids[iiEntry];
		entry.dataType = fieldListDataTypes[iiEntry];

		switch(fieldListDataTypes[iiEntry])
		{
			case RSSL_DT_INT:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListInt)); break; //rsslEncodeFieldEntry 
			case RSSL_DT_DOUBLE:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListFloat)); break; //rsslEncodeFieldEntry 
			case RSSL_DT_REAL:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListReal)); break; //rsslEncodeFieldEntry 
			case RSSL_DT_DATE:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListDate)); break; //rsslEncodeFieldEntry 
			case RSSL_DT_TIME:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListTime)); break; //rsslEncodeFieldEntry 
			case RSSL_DT_DATETIME:
				ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldEntry(pEncIter, &entry, &fieldListDateTime)); break; //rsslEncodeFieldEntry 
			default:
				ASSERT_TRUE(0);  break; //Error in _encodeFieldList()
		}

	}


	/* finish encoding */
	ASSERT_TRUE( RSSL_RET_SUCCESS == rsslEncodeFieldListComplete(pEncIter, RSSL_TRUE)); //rsslEncodeFieldListComplete
}

void _encodePayload (RsslEncodeIterator* pEncIter)
{
    if (g_dataFormat == RSSL_DT_MSG)
        _encodeNestedMsg(pEncIter);
    else // FIELD_LIST
        _encodeFieldList(pEncIter);
}


/* Msg Key */

void _encodeMsgKey(RsslMsgKey *pKey, char *opaque, RsslUInt32 opaqueLen)
{
	RsslMsgKeyFlags mask = (RsslMsgKeyFlags)pKey->flags;

	/* Add Service ID */
	if ( mask & RSSL_MKF_HAS_SERVICE_ID )
		pKey->serviceId = 7;

	/* Add Name */
	if ( mask & RSSL_MKF_HAS_NAME )
	{
		pKey->name.data = payloadName;
		pKey->name.length = payloadNameLen;
	}

	/* Add Name Type*/
	if ( mask & RSSL_MKF_HAS_NAME_TYPE )
		pKey->nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;

	/* Add Filter */
	if ( mask & RSSL_MKF_HAS_FILTER )
		pKey->filter = 4294967294;

	/* Add ID */
	if ( mask & RSSL_MKF_HAS_IDENTIFIER )
		pKey->identifier = 9001;

	/* Add Attrib ContainerType/Data */ /* Rssl calls this "opaque" */
	if ( mask & RSSL_MKF_HAS_ATTRIB )
	{
		RsslEncodeIterator encIter = RSSL_INIT_ENCODE_ITERATOR;
		pKey->encAttrib.data = opaque;
		pKey->encAttrib.length = opaqueLen;
		rsslSetEncodeIteratorBuffer(&encIter, &pKey->encAttrib);
		_encodeFieldList(&encIter);
		pKey->encAttrib.length = rsslGetEncodedBufferLength(&encIter);
		pKey->attribContainerType = RSSL_DT_FIELD_LIST;
	}	
}


/* used by no data test - it encodes the attrib the same way so it comes out equivelant */
void _encodeMsgKeyNoAttrib(RsslMsgKey *pKey)
{
	RsslMsgKeyFlags mask = (RsslMsgKeyFlags)pKey->flags;

	/* Add Service ID */
	if ( mask & RSSL_MKF_HAS_SERVICE_ID )
		pKey->serviceId = 7;

	/* Add Name */
	if ( mask & RSSL_MKF_HAS_NAME )
	{
		pKey->name.data = payloadName;
		pKey->name.length = payloadNameLen;
	}

	/* Add Name Type*/
	if ( mask & RSSL_MKF_HAS_NAME_TYPE )
		pKey->nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;

	/* Add Filter */
	if ( mask & RSSL_MKF_HAS_FILTER )
		pKey->filter = 4294967294;

	/* Add ID */
	if ( mask & RSSL_MKF_HAS_IDENTIFIER )
		pKey->identifier = 9001;


	pKey->attribContainerType = RSSL_DT_FIELD_LIST;
	/* attrib is encoded outside */
	
}


void _decodeMsgKey(RsslDecodeIterator *pDecIter, RsslMsgKey *pKey)
{
	RsslMsgKeyFlags mask = (RsslMsgKeyFlags)pKey->flags;
	/* Check Service ID */
	if ( mask & RSSL_MKF_HAS_SERVICE_ID )
		ASSERT_TRUE( pKey->serviceId == 7); //Correct Service ID in MsgKey

	/* Check Name */
	if ( mask & RSSL_MKF_HAS_NAME  )
		ASSERT_TRUE(
			pKey->name.length == payloadNameLen
			&& 0 == memcmp(pKey->name.data, payloadName, payloadNameLen)); //Correct Name in MsgKey

	/* Check Name Type*/
	if ( mask & RSSL_MKF_HAS_NAME_TYPE )
		ASSERT_TRUE( pKey->nameType == RDM_INSTRUMENT_NAME_TYPE_RIC ); // Correct Name Type in MsgKey

	/* Check Filter */
	if ( mask & RSSL_MKF_HAS_FILTER )
		ASSERT_TRUE( pKey->filter == 4294967294); //Correct Filter in MsgKey

	/* Check ID */
	if ( mask & RSSL_MKF_HAS_IDENTIFIER )
		ASSERT_TRUE( pKey->identifier == 9001); //Correct ID in MsgKey

	/* check opaque */
	if (mask & RSSL_MKF_HAS_ATTRIB)
	{
		ASSERT_TRUE(pKey->attribContainerType == RSSL_DT_FIELD_LIST); //Correct attribContainerType
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsgKeyAttrib(pDecIter, pKey)); //rsslDecodeMsgKeyAttrib
		_decodeFieldList(pDecIter);
	}
}

/* Remove flags that the encoder should have removed because the test is(intentionally) trying to do something wrong */
RsslMsgKeyFlags _fixDecodeMsgKeyMask(RsslMsgKeyFlags mask)
{
	if ((mask & RSSL_MKF_HAS_NAME_TYPE) && !(mask & RSSL_MKF_HAS_NAME)) /* NameType without Name */ /* Rssl DOES do this optimization */
		mask = (RsslMsgKeyFlags)(mask & ~RSSL_MKF_HAS_NAME_TYPE);

	return mask;
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
	ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldList(pDecIter, &container, 0)); //rsslDecodeFieldList

	ASSERT_TRUE(
		container.flags == RSSL_FLF_HAS_STANDARD_DATA
		); //FieldList is correct

	// Decode entries
	for(iiEntry = 0; iiEntry < TEST_FIELD_LIST_MAX_ENTRIES ; ++iiEntry)
	{
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeFieldEntry(pDecIter, &entry)); //rsslDecodeFieldEntry
						
			ASSERT_TRUE(entry.fieldId == fieldListFids[iiEntry] && entry.dataType == RSSL_DT_UNKNOWN
						); //Decoded entry is correct

			switch(fieldListDataTypes[iiEntry])
			{
				case RSSL_DT_INT:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeInt(pDecIter, &decInt)
							&& decInt == fieldListInt); //Integer correctly decoded
					break;
				case RSSL_DT_DOUBLE:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDouble(pDecIter, &decFloat)
						&& decFloat == fieldListFloat); // Float correctly decoded /* not rounded inside encoding/decoding, so this should match exactly */
					break;
				case RSSL_DT_REAL:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeReal(pDecIter, &decReal)
							&& decReal.isBlank == 0
							&& decReal.hint == fieldListReal.hint
							&& decReal.value == fieldListReal.value); //Real correctly decoded
					break;
				case RSSL_DT_DATE:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDate(pDecIter, &decDate)
							&& decDate.day == fieldListDate.day
							&& decDate.month == fieldListDate.month
							&& decDate.year == fieldListDate.year); //Date correctly decoded
					break;
				case RSSL_DT_TIME:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeTime(pDecIter, &decTime)
						&& decTime.hour == fieldListTime.hour
						&& decTime.minute == fieldListTime.minute
						&& decTime.second == fieldListTime.second
						&& decTime.millisecond == fieldListTime.millisecond); //Time correctly decoded

					break;
				case RSSL_DT_DATETIME:
					ASSERT_TRUE( RSSL_RET_SUCCESS == rsslDecodeDateTime(pDecIter, &decDateTime)
							&& decDateTime.date.day == fieldListDateTime.date.day
							&& decDateTime.date.month == fieldListDateTime.date.month
							&& decDateTime.date.year == fieldListDateTime.date.year
							&& decDateTime.time.hour == fieldListDateTime.time.hour
							&& decDateTime.time.minute == fieldListDateTime.time.minute
							&& decDateTime.time.second == fieldListDateTime.time.second
							&& decDateTime.time.millisecond == fieldListDateTime.time.millisecond); //DateTime correctly decoded
					break;
				default:
					ASSERT_TRUE(0); //Error in _decodeFieldList()
			}
	}
	ASSERT_TRUE(RSSL_RET_END_OF_CONTAINER == rsslDecodeFieldEntry(pDecIter, &entry)); //rsslDecodeFieldEntry(end of container)

}

void _decodePayload(RsslDecodeIterator *pDecIter)
{
    if (g_dataFormat == RSSL_DT_MSG)
        _decodeNestedMsg(pDecIter);
    else
        _decodeFieldList(pDecIter);
}

/* does the work for encoding opaque and/or extended header after MsgInit is called */
RTR_C_INLINE void _postEncodeMsg(RsslEncodeIterator *pEncIter, RsslMsg *pMsg, ExtraAction extraAction, RsslBuffer *pExtHdr, RsslMsgKey *reqKey, RsslUInt16 msgFlags, RsslUInt16 extHdrFlags, RsslUInt16 reqKeyFlags )
{
	RsslRet expectedRet = RSSL_RET_ENCODE_CONTAINER;
	RsslRet ret;
	
	if ((extraAction & TEST_ACTION_POST_REQKEY) && reqKey && (msgFlags & reqKeyFlags) && (reqKey->flags & RSSL_MKF_HAS_ATTRIB))
	{
		/* if we are post encoding the request key, make it a field list */
		reqKey->encAttrib.length = 0;
		reqKey->encAttrib.data = 0;
		reqKey->attribContainerType = RSSL_DT_FIELD_LIST;
		expectedRet = RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;	// set this to the value that rsslEncodeMsgInit will return
	}

	if ((extraAction & TEST_ACTION_POST_EXTENDED) && (msgFlags & extHdrFlags) && pExtHdr)
	{
		/* if we are also post encoding our extended header, unset its values */
		pExtHdr->length = 0;
		pExtHdr->data = 0;
		expectedRet = RSSL_RET_ENCODE_EXTENDED_HEADER; // set this to the value that rsslEncodeMsgInit will return
	}
	
	if ((extraAction & TEST_ACTION_POST_OPAQUE) && (pMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB))
	{
		/* if we are post encoding the opaque, make it a field list */
		pMsg->msgBase.msgKey.encAttrib.length = 0;
		pMsg->msgBase.msgKey.encAttrib.data = 0;
		pMsg->msgBase.msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		expectedRet = RSSL_RET_ENCODE_MSG_KEY_OPAQUE; // set this to the value that rsslEncodeMsgInit will return
	}

	ASSERT_TRUE(expectedRet == (ret = rsslEncodeMsgInit(pEncIter, pMsg, 0))); //rsslEncodeMsgInit

	if (expectedRet == RSSL_RET_ENCODE_MSG_KEY_OPAQUE)
	{
		/* we need to encode our opaque here, should be a field list */
		_encodeFieldList(pEncIter);

		if ((extraAction & TEST_ACTION_POST_EXTENDED) && (msgFlags & extHdrFlags) && pExtHdr)
			expectedRet = RSSL_RET_ENCODE_EXTENDED_HEADER;
		else if ((extraAction & TEST_ACTION_POST_REQKEY) && reqKey && (msgFlags & reqKeyFlags) && (reqKey->flags & RSSL_MKF_HAS_ATTRIB))
			expectedRet = RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
		else
			expectedRet = RSSL_RET_ENCODE_CONTAINER;

		ASSERT_TRUE(expectedRet == (ret = rsslEncodeMsgKeyAttribComplete(pEncIter, RSSL_TRUE))); //rsslEncodeMsgKeyAttribComplete
	}

	if ((expectedRet == RSSL_RET_ENCODE_EXTENDED_HEADER) && pExtHdr)
	{	
		RsslBuffer buffer = RSSL_INIT_BUFFER;

		/* we must encode our extended header now */
		/* just hack copy it onto the wire */
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeNonRWFDataTypeInit(pEncIter, &buffer)); //rsslEncodeNonRWFDataTypeInit
		ASSERT_TRUE(buffer.length >= extendedHeaderLen); //Length Failure
		MemCopyByInt(buffer.data, extendedHeader, extendedHeaderLen);
		buffer.length = extendedHeaderLen;
		ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeNonRWFDataTypeComplete(pEncIter, &buffer, RSSL_TRUE)); //rsslEncodeNonRWFDataTypeComplete

		if ((extraAction & TEST_ACTION_POST_REQKEY) && reqKey && (msgFlags & reqKeyFlags) && (reqKey->flags & RSSL_MKF_HAS_ATTRIB))
			expectedRet = RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB;
		else
			expectedRet = RSSL_RET_ENCODE_CONTAINER;

		ASSERT_TRUE(expectedRet == rsslEncodeExtendedHeaderComplete(pEncIter, RSSL_TRUE)); //rsslEncodeExtendedHeaderComplete
	}
	
	if (expectedRet == RSSL_RET_ENCODE_REQMSG_KEY_ATTRIB)
	{
		/* we need to encode our request key attribs here with a field list */
		_encodeFieldList(pEncIter);
		ASSERT_TRUE(RSSL_RET_ENCODE_CONTAINER == rsslEncodeMsgReqKeyAttribComplete(pEncIter, RSSL_TRUE)); //rsslEncodeMsgReqKeyAttribComplete
	}
}



/***** Message Tests *****/
void ackMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslAckFlags ackMask;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;

	RsslAckMsg *pAcknMsg = &msg.ackMsg;

	RsslUInt8 domainType = RSSL_DMT_TRANSACTION;


	//printf("ackMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			ackMask = (RsslAckFlags)masks[masksIter];
			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (ackMask & RSSL_AKMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
				_setupEncodeIterator();

				/* Encode msg */

				rsslClearAckMsg(pAcknMsg);

				pAcknMsg->msgBase.msgClass = RSSL_MC_ACK;
				pAcknMsg->msgBase.streamId = streamId;
				pAcknMsg->msgBase.domainType = domainType;

				pAcknMsg->flags = ackMask;
				pAcknMsg->ackId = ackMask;

				/* Add nakCode */
				if (ackMask & RSSL_AKMF_HAS_NAK_CODE )
				{
					pAcknMsg->nakCode = (RsslUInt8)(ackMask + 1);

				}

				if (ackMask & RSSL_AKMF_HAS_TEXT)
				{
					pAcknMsg->text.length = textLen;
					pAcknMsg->text.data = text;
				}

					if (ackMask & RSSL_AKMF_HAS_SEQ_NUM)
						pAcknMsg->seqNum = seqNum;

					/* Add msgKey */
					if (ackMask & RSSL_AKMF_HAS_MSG_KEY)
					{
						pAcknMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pAcknMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}

				/* Add Extended Header */
				/* always set it here to cover our pre-encoded data/rsslEncodeMsg cases.  */
				if (ackMask & RSSL_AKMF_HAS_EXTENDED_HEADER) 
				{
					pAcknMsg->extendedHeader.length = extendedHeaderLen;
					pAcknMsg->extendedHeader.data = extendedHeader;
				}

				/* Add Payload */
				if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
				{
					msg.msgBase.encDataBody.data = encDataBuf.data;
					msg.msgBase.containerType = g_dataFormat;

					/* Trim payload length if desired */
					msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
				}
				else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
				{
					RsslRet ret = RSSL_RET_ENCODE_CONTAINER;
					msg.msgBase.containerType = g_dataFormat;

					/* if our key opaque and our extended header are pre-encoded, EncodeMsgInit should
					   tell us to encode our payload/container */

					_postEncodeMsg(&encIter, &msg, extraAction, &pAcknMsg->extendedHeader, NULL, pAcknMsg->flags, RSSL_AKMF_HAS_EXTENDED_HEADER, 0 );

					_encodePayload(&encIter);
					encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
					
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
				}
				else
				{
					msg.msgBase.encDataBody.data = 0;
					msg.msgBase.encDataBody.length = 0;
					msg.msgBase.containerType = RSSL_DT_NO_DATA;
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
				}

				encBuf.length = rsslGetEncodedBufferLength(&encIter);

				if (g_testDecoding == RSSL_FALSE) continue;

				/* Decode msg */
				decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

				rsslClearAckMsg(pAcknMsg);
				_setupDecodeIterator();
				_setupEncodeIterator();

				if (g_changeData)
				{
					RsslUInt32 extractSeqNum;

					/* extract the msgClass */
					ASSERT_TRUE(RSSL_MC_ACK == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
					_setupDecodeIterator();

					/* extract the domainType */
					ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
					_setupDecodeIterator();

					/* replace the domainType */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
					_setupEncodeIterator();

					/* extract the domainType */
					ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
					_setupDecodeIterator();

					/* extract streamId */
					ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
					_setupDecodeIterator();

					/* replace the streamId */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
					_setupEncodeIterator();

					/* extract the streamId */
					ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
					_setupDecodeIterator();

					/* extract the seqNum */
					ASSERT_TRUE(
						((ackMask & RSSL_AKMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
						== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
					if (ackMask & RSSL_AKMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum); //extractSeqNum
					_setupDecodeIterator();

					/* replace the seqNum */
					ASSERT_TRUE( 
						((ackMask & RSSL_AKMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
						== rsslReplaceSeqNum(&encIter, seqNum + 1)
						); //rsslReplaceSeqNum
					_setupEncodeIterator();

					/* extract the new seqNum */
					ASSERT_TRUE(
						((ackMask & RSSL_AKMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
						== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
					if (ackMask & RSSL_AKMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum + 1); //extractSeqNum
					_setupDecodeIterator();
				}

				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
				ASSERT_TRUE(
					msg.msgBase.msgClass == RSSL_MC_ACK
					&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
					&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
					&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

				ASSERT_TRUE(
					pAcknMsg->flags == ackMask); //Correct Mask

				/* copy test */
				{
					RsslMsg *copyMsg;
					RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
					int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
					copyBuffer.data = (char*)malloc(msgSize);
					copyBuffer.length = msgSize;
					copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
					ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
					ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
							0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
					ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
					ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
							0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
					ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
					ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
							0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
					ASSERT_TRUE(!msg.ackMsg.extendedHeader.data || copyMsg->ackMsg.extendedHeader.data != msg.ackMsg.extendedHeader.data); //extendedHeader deep copied
					ASSERT_TRUE(copyMsg->ackMsg.extendedHeader.length == msg.ackMsg.extendedHeader.length &&
							0 == memcmp(copyMsg->ackMsg.extendedHeader.data, msg.ackMsg.extendedHeader.data, msg.ackMsg.extendedHeader.length)); //extendedHeader is correct
					ASSERT_TRUE(!msg.ackMsg.text.data || copyMsg->ackMsg.text.data != msg.ackMsg.text.data); //text deep copied
					ASSERT_TRUE(copyMsg->ackMsg.text.length == msg.ackMsg.text.length &&
							0 == memcmp(copyMsg->ackMsg.text.data, msg.ackMsg.text.data, msg.ackMsg.text.length)); //text is correct
					free(copyBuffer.data);
				}

				/* Check status */
				if (ackMask & RSSL_AKMF_HAS_TEXT)
				{
					ASSERT_TRUE(
						pAcknMsg->text.length == textLen
						&& 0 == memcmp(pAcknMsg->text.data, text, textLen)); //Text is correct
				}

				if (ackMask & RSSL_AKMF_HAS_NAK_CODE)
				{
					ASSERT_TRUE(
						pAcknMsg->nakCode == (RsslUInt8)(ackMask + 1)); //Correct NakCode
				}

					if (ackMask & RSSL_AKMF_HAS_SEQ_NUM)
					{
						ASSERT_TRUE(
								pAcknMsg->seqNum == (g_changeData ? seqNum + 1 : seqNum)); //SeqNum is correct
					}
			
					
					/* Check msgKey */
					if (ackMask & RSSL_AKMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( pAcknMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pAcknMsg->msgBase.msgKey);

					}


				/* Check Extended Header */
				if (ackMask & RSSL_AKMF_HAS_EXTENDED_HEADER)
				{
					ASSERT_TRUE( 
						pAcknMsg->extendedHeader.length == extendedHeaderLen
						&& 0 == memcmp(pAcknMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
				}
				

				/* Check Payload */
				if ( extraAction & TEST_ACTION_PAYLOAD )
				{
					_decodePayload(&decIter);
				}
				}
			}
		}
	}

	endTimerAndPrint();
	//printf("\n");
}

void closeMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslCloseFlags closeMask;

	RsslCloseMsg *pCloseMsg = &msg.closeMsg;

	RsslUInt8 domainType = RSSL_DMT_DICTIONARY;

	//printf("closeMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			closeMask = (RsslCloseFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				/* No key for closeMsg in Rssl */
				/*for ( msgKeyMask = 0; msgKeyMask <= TEST_MSGKEY_MAX_MASK; ++msgKeyMask )
				{*/
				_setupEncodeIterator();

				/* Encode msg */

				rsslClearCloseMsg(pCloseMsg);

				pCloseMsg->msgBase.msgClass = RSSL_MC_CLOSE;
				pCloseMsg->msgBase.streamId = streamId;
				pCloseMsg->msgBase.domainType = domainType;

				pCloseMsg->flags = closeMask;

				/* Add Extended Header */
				if (closeMask & RSSL_CLMF_HAS_EXTENDED_HEADER)
				{
					pCloseMsg->extendedHeader.length = extendedHeaderLen;
					pCloseMsg->extendedHeader.data = extendedHeader;
				}

				/* Add Payload */
				if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
				{
					msg.msgBase.encDataBody.data = encDataBuf.data;
					msg.msgBase.containerType = g_dataFormat;

					/* Trim payload length if desired */
					msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
				}
				else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
				{
					msg.msgBase.containerType = g_dataFormat;

					_postEncodeMsg(&encIter, &msg, extraAction, &pCloseMsg->extendedHeader, NULL, pCloseMsg->flags, RSSL_CLMF_HAS_EXTENDED_HEADER, 0 );

					_encodePayload(&encIter);
					encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
					
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
				}
				else
				{
					msg.msgBase.encDataBody.data = 0;
					msg.msgBase.encDataBody.length = 0;
					msg.msgBase.containerType = RSSL_DT_NO_DATA;
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
				}

				encBuf.length = rsslGetEncodedBufferLength(&encIter);

				if (g_testDecoding == RSSL_FALSE) continue;

				rsslClearCloseMsg(pCloseMsg);
				_setupDecodeIterator();
				_setupEncodeIterator();

				if (g_changeData)
				{
					/* extract the msgClass */
					ASSERT_TRUE(RSSL_MC_CLOSE == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
					_setupDecodeIterator();

					/* extract the domainType */
					ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
					_setupDecodeIterator();

					/* replace the domainType */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
					_setupEncodeIterator();

					/* extract the domainType */
					ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
					_setupDecodeIterator();

					/* extract streamId */
					ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
					_setupDecodeIterator();

					/* replace the streamId */
					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
					_setupEncodeIterator();

					/* extract the streamId */
					ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
					_setupDecodeIterator();
				}

				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
				ASSERT_TRUE(
					msg.msgBase.msgClass == RSSL_MC_CLOSE
					&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
					&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
					&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

				ASSERT_TRUE(
					pCloseMsg->flags == closeMask); //Correct Mask

				/* copy test */
				{
					RsslMsg *copyMsg;
					RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
					int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
					copyBuffer.data = (char*)malloc(msgSize);
					copyBuffer.length = msgSize;
					copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
					ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
					ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
							0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
					ASSERT_TRUE(!msg.closeMsg.extendedHeader.data || copyMsg->closeMsg.extendedHeader.data != msg.closeMsg.extendedHeader.data); //extendedHeader deep copied
					ASSERT_TRUE(copyMsg->closeMsg.extendedHeader.length == msg.closeMsg.extendedHeader.length &&
							0 == memcmp(copyMsg->closeMsg.extendedHeader.data, msg.closeMsg.extendedHeader.data, msg.closeMsg.extendedHeader.length)); //extendedHeader is correct
					free(copyBuffer.data);
				}

				/* Check Extended Header */
				if (closeMask & RSSL_CLMF_HAS_EXTENDED_HEADER)
				{
					ASSERT_TRUE( 
						pCloseMsg->extendedHeader.length == extendedHeaderLen
						&& 0 == memcmp(pCloseMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
				}

				/* Check Payload */
				if ( extraAction & TEST_ACTION_PAYLOAD )
				{
					_decodePayload(&decIter);
				}
				/*}*/
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}


void genericMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslGenericFlags genericMask;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;

	RsslGenericMsg *pGenMsg = &msg.genericMsg;

	RsslUInt8 domainType = RSSL_DMT_STORY;

	//printf("genericMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			genericMask = (RsslGenericFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (genericMask & RSSL_GNMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					_setupEncodeIterator();

					/* Encode msg */

					rsslClearGenericMsg(pGenMsg);

					pGenMsg->msgBase.msgClass = RSSL_MC_GENERIC;
					pGenMsg->msgBase.streamId = streamId;
					pGenMsg->msgBase.domainType = domainType;
					pGenMsg->flags = genericMask;

					/* Add msgKey */
					if (genericMask & RSSL_GNMF_HAS_MSG_KEY)
					{
						pGenMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pGenMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}

					/* Add request msgKey */
					if (genericMask & RSSL_GNMF_HAS_REQ_MSG_KEY)
					{
						pGenMsg->reqMsgKey.flags = msgKeyMask;
						_encodeMsgKey(&pGenMsg->reqMsgKey, reqKeyAttrib, reqKeyAttribLen);
					}

					/* Add Permission Info */ 
					if (genericMask & RSSL_GNMF_HAS_PERM_DATA)
					{
						pGenMsg->permData.length = permissionDataLen;
						pGenMsg->permData.data = permissionData;
					}

					/* Add Item Sequence Number */
					if (genericMask & RSSL_GNMF_HAS_SEQ_NUM)
					{
						seqNum = (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32)(genericMask + 0xFFFF): (RsslUInt32)genericMask;
						pGenMsg->seqNum = seqNum;
					}

					if (genericMask & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM)
					{
						pGenMsg->secondarySeqNum = (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32) (genericMask + 0xFFFF): (RsslUInt32)genericMask;
					}

					if (genericMask & RSSL_GNMF_HAS_PART_NUM)
					{
						pGenMsg->partNum = 0x100f;
					}

					/* Add Extended Header */
					if (genericMask & RSSL_GNMF_HAS_EXTENDED_HEADER)
					{
						pGenMsg->extendedHeader.length = extendedHeaderLen;
						pGenMsg->extendedHeader.data = extendedHeader;
					}
					
					/* test rsslSetGenericCompleteFlag()/rsslUnsetGenericCompleteFlag(), remove/add it here first, then add/remove later*/
					if ( genericMask & RSSL_GNMF_MESSAGE_COMPLETE )
						pGenMsg->flags &= ~RSSL_GNMF_MESSAGE_COMPLETE;
					else
						pGenMsg->flags |= RSSL_GNMF_MESSAGE_COMPLETE;

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
						
						/* Call rsslSetGenericCompleteFlag()/rsslUnsetGenericCompleteFlag() to reset back to the original genericMask*/
						if ( genericMask & RSSL_GNMF_MESSAGE_COMPLETE )
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslSetGenericCompleteFlag(&encIter)); //rsslSetGenericCompleteFlag
						if ( !(genericMask & RSSL_GNMF_MESSAGE_COMPLETE) )
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslUnsetGenericCompleteFlag(&encIter)); //rsslUnsetGenericCompleteFlag
					
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;

						_postEncodeMsg(&encIter, &msg, extraAction, &pGenMsg->extendedHeader, &pGenMsg->reqMsgKey, pGenMsg->flags, RSSL_GNMF_HAS_EXTENDED_HEADER, RSSL_GNMF_HAS_REQ_MSG_KEY );

						_encodePayload(&encIter);
						
						/* Call rsslSetGenericCompleteFlag()/rsslUnsetGenericCompleteFlag() to reset back to the original genericMask*/
						if (genericMask & RSSL_GNMF_MESSAGE_COMPLETE)
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslSetGenericCompleteFlag(&encIter)); //rsslSetGenericCompleteFlag
						if (!(genericMask & RSSL_GNMF_MESSAGE_COMPLETE))
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslUnsetGenericCompleteFlag(&encIter)); //rsslUnsetGenericCompleteFlag
						
						encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
						
						/* Call rsslSetGenericCompleteFlag()/rsslUnsetGenericCompleteFlag() to reset back to the original genericMask*/
						if (genericMask & RSSL_GNMF_MESSAGE_COMPLETE)
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslSetGenericCompleteFlag(&encIter)); //rsslSetGenericCompleteFlag
						if (!(genericMask & RSSL_GNMF_MESSAGE_COMPLETE))
							ASSERT_TRUE(RSSL_RET_SUCCESS == rsslUnsetGenericCompleteFlag(&encIter)); //rsslUnsetGenericCompleteFlag

					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearGenericMsg(pGenMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						RsslUInt32 extractSeqNum;

						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_GENERIC == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* extract the seqNum */
						ASSERT_TRUE(
							((genericMask & RSSL_GNMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (genericMask & RSSL_GNMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum); //extractSeqNum
						_setupDecodeIterator();

						/* replace the seqNum */
						ASSERT_TRUE( 
							((genericMask & RSSL_GNMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceSeqNum(&encIter, seqNum + 1)
							); //rsslReplaceSeqNum
						_setupEncodeIterator();

						/* extract the new seqNum */
						ASSERT_TRUE(
							((genericMask & RSSL_GNMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (genericMask & RSSL_GNMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum + 1); //extractSeqNum
						_setupDecodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_GENERIC
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pGenMsg->flags == genericMask); //Correct Mask

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.genericMsg.reqMsgKey.encAttrib.data || copyMsg->genericMsg.reqMsgKey.encAttrib.data != msg.genericMsg.reqMsgKey.encAttrib.data); //reqMsgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->genericMsg.reqMsgKey.encAttrib.length == msg.genericMsg.reqMsgKey.encAttrib.length &&
								0 == memcmp(copyMsg->genericMsg.reqMsgKey.encAttrib.data, msg.genericMsg.reqMsgKey.encAttrib.data, msg.genericMsg.reqMsgKey.encAttrib.length)); //reqMsgKey.encAttrib is correct
						ASSERT_TRUE(!msg.genericMsg.reqMsgKey.name.data || copyMsg->genericMsg.reqMsgKey.name.data != msg.genericMsg.reqMsgKey.name.data); //reqMsgKey.name deep copied
						ASSERT_TRUE(copyMsg->genericMsg.reqMsgKey.name.length == msg.genericMsg.reqMsgKey.name.length &&
								0 == memcmp(copyMsg->genericMsg.reqMsgKey.name.data, msg.genericMsg.reqMsgKey.name.data, msg.genericMsg.reqMsgKey.name.length)); //reqMsgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.genericMsg.permData.data || copyMsg->genericMsg.permData.data != msg.genericMsg.permData.data); //permData deep copied
						ASSERT_TRUE(copyMsg->genericMsg.permData.length == msg.genericMsg.permData.length &&
								0 == memcmp(copyMsg->genericMsg.permData.data, msg.genericMsg.permData.data, msg.genericMsg.permData.length)); //permData is correct
						ASSERT_TRUE(!msg.genericMsg.extendedHeader.data || copyMsg->genericMsg.extendedHeader.data != msg.genericMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->genericMsg.extendedHeader.length == msg.genericMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->genericMsg.extendedHeader.data, msg.genericMsg.extendedHeader.data, msg.genericMsg.extendedHeader.length)); //extendedHeader is correct
						free(copyBuffer.data);
					}

					/* Check msgKey */
					if (genericMask & RSSL_GNMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( pGenMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pGenMsg->msgBase.msgKey);
					}

					/* Check Permission Info */
					if (genericMask & RSSL_GNMF_HAS_PERM_DATA)
					{
						ASSERT_TRUE( 
							pGenMsg->permData.length == permissionDataLen 
							&& 0 == memcmp(pGenMsg->permData.data, permissionData, permissionDataLen)); //Correct Permission Info
					}

					/* Check Item Sequence Number */
					if (genericMask & RSSL_GNMF_HAS_SEQ_NUM)
					{
						ASSERT_TRUE( 
							pGenMsg->seqNum == (g_changeData ? seqNum + 1 : seqNum)); //Correct Item Sequence Number
					}

					/* Check Item Secondary Sequence Number */
					if (genericMask & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM)
					{
						ASSERT_TRUE( 
							pGenMsg->secondarySeqNum == (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32)(genericMask + 0xFFFF): (RsslUInt32)genericMask); //Correct Item Secondary Sequence Number
					}

					if (genericMask & RSSL_GNMF_HAS_PART_NUM)
					{
						ASSERT_TRUE(pGenMsg->partNum == 0x100f); //Correct PartNum
					}

					/* Check Extended Header */
					if (genericMask & RSSL_GNMF_HAS_EXTENDED_HEADER)
					{
						ASSERT_TRUE( 
							pGenMsg->extendedHeader.length == extendedHeaderLen
							&& 0 == memcmp(pGenMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
				}
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}

void postMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslPostFlags postMask;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;

	RsslPostMsg *pPostMsg = &msg.postMsg;

	RsslUInt8 domainType = RSSL_DMT_MARKET_PRICE;

	//printf("postMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			postMask = (RsslPostFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (postMask & RSSL_PSMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					_setupEncodeIterator();

					/* Encode msg */

					rsslClearPostMsg(pPostMsg);

					pPostMsg->msgBase.msgClass = RSSL_MC_POST;
					pPostMsg->msgBase.streamId = streamId;
					pPostMsg->msgBase.domainType = domainType;
					pPostMsg->flags = postMask;

					/* Add msgKey */
					if (postMask & RSSL_PSMF_HAS_MSG_KEY)
					{
						pPostMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pPostMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}

					/* Add Post ID */ 
					if (postMask & RSSL_PSMF_HAS_POST_ID)
					{
						pPostMsg->postId = postId;
						
					}

					if(postMask & RSSL_PSMF_HAS_PERM_DATA)
					{
						pPostMsg->permData.length = permissionDataLen;
						pPostMsg->permData.data = permissionData;
					}

					/* Set up user information */
					pPostMsg->postUserInfo.postUserAddr = 0xCCAA551D;
					pPostMsg->postUserInfo.postUserId = 0xCCAA551D;

					/* Add Post Sequence Number */
					if (postMask & RSSL_PSMF_HAS_SEQ_NUM)
					{
						seqNum = (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32)(postMask + 0xFFFF): (RsslUInt32)postMask;
						pPostMsg->seqNum = seqNum;
					}

					if (postMask & RSSL_PSMF_HAS_PART_NUM)
					{
						pPostMsg->partNum = 0x100f;
					}
					
					if (postMask & RSSL_PSMF_HAS_POST_USER_RIGHTS)
					{
						pPostMsg->postUserRights = 0x0001;
					}

					/* Add Extended Header */
					if (postMask & RSSL_PSMF_HAS_EXTENDED_HEADER)
					{
						pPostMsg->extendedHeader.length = extendedHeaderLen;
						pPostMsg->extendedHeader.data = extendedHeader;
					}

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg(&encIter, &msg)); //rsslEncodeMsg
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;

						_postEncodeMsg(&encIter, &msg, extraAction, &pPostMsg->extendedHeader, NULL, pPostMsg->flags, RSSL_PSMF_HAS_EXTENDED_HEADER, 0 );

						_encodePayload(&encIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg(&encIter, &msg)); //rsslEncodeMsg
					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearPostMsg(pPostMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						RsslUInt32 extractSeqNum;
						RsslUInt32 extractPostId;

						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_POST == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* extract the seqNum */
						ASSERT_TRUE(
							((postMask & RSSL_PSMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (postMask & RSSL_PSMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum); //extractSeqNum
						_setupDecodeIterator();

						/* replace the seqNum */
						ASSERT_TRUE( 
							((postMask & RSSL_PSMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceSeqNum(&encIter, seqNum + 1)
							); //rsslReplaceSeqNum
						_setupEncodeIterator();

						/* extract the new seqNum */
						ASSERT_TRUE(
							((postMask & RSSL_PSMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (postMask & RSSL_PSMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum + 1); //extractSeqNum
						_setupDecodeIterator();

						/* extract the postId */
						ASSERT_TRUE(
							((postMask & RSSL_PSMF_HAS_POST_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractPostId(&decIter, &extractPostId)); //rsslExtractPostId
						if (postMask & RSSL_PSMF_HAS_POST_ID) ASSERT_TRUE(extractPostId == postId); //extractPostId
						_setupDecodeIterator();

						/* replace the postId */
						ASSERT_TRUE( 
							((postMask & RSSL_PSMF_HAS_POST_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplacePostId(&encIter, postId + 1)
							); //rsslReplacePostId
						_setupEncodeIterator();

						/* extract the new postId */
						ASSERT_TRUE(
							((postMask & RSSL_PSMF_HAS_POST_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractPostId(&decIter, &extractPostId)); //rsslExtractPostId
						if (postMask & RSSL_PSMF_HAS_POST_ID) ASSERT_TRUE(extractPostId == postId + 1); //extractPostId
						_setupDecodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //rsslDecodeMsg
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_POST
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pPostMsg->flags == postMask); //Correct Mask

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.postMsg.permData.data || copyMsg->postMsg.permData.data != msg.postMsg.permData.data); //permData deep copied
						ASSERT_TRUE(copyMsg->postMsg.permData.length == msg.postMsg.permData.length &&
								0 == memcmp(copyMsg->postMsg.permData.data, msg.postMsg.permData.data, msg.postMsg.permData.length)); //permData is correct
						ASSERT_TRUE(!msg.postMsg.extendedHeader.data || copyMsg->postMsg.extendedHeader.data != msg.postMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->postMsg.extendedHeader.length == msg.postMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->postMsg.extendedHeader.data, msg.postMsg.extendedHeader.data, msg.postMsg.extendedHeader.length)); //extendedHeader is correct
						free(copyBuffer.data);
					}

					/* Check msgKey */
					if (postMask & RSSL_PSMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( pPostMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pPostMsg->msgBase.msgKey);
					}

					/* Check Post ID */
					if (postMask & RSSL_PSMF_HAS_POST_ID)
					{
						ASSERT_TRUE( 
							pPostMsg->postId == (g_changeData ? postId + 1 : postId)); //Correct Permission Info
					}

					/* Check Post User Info */
					ASSERT_TRUE(pPostMsg->postUserInfo.postUserAddr == 0xCCAA551D); //Correct Post User Address

					ASSERT_TRUE(pPostMsg->postUserInfo.postUserId == 0xCCAA551D); //Correct Post User ID								
					

					/* Check Item Sequence Number */
					if (postMask & RSSL_PSMF_HAS_SEQ_NUM)
					{
						ASSERT_TRUE( 
							pPostMsg->seqNum == (g_changeData ? seqNum + 1 : seqNum)); //Correct Item Sequence Number
					}

					if (postMask & RSSL_PSMF_HAS_PART_NUM)
					{
						ASSERT_TRUE(pPostMsg->partNum == 0x100f); //Correct PartNum
					}
					
					/* Check Post User Rights*/
					if (postMask & RSSL_PSMF_HAS_POST_USER_RIGHTS)
					{
						ASSERT_TRUE(pPostMsg->postUserRights == 0x0001); //Correct Post User Rights
					}

					/* Check Permission Info */
					if (postMask & RSSL_PSMF_HAS_PERM_DATA)
					{
						ASSERT_TRUE( 
							pPostMsg->permData.length == permissionDataLen 
							&& 0 == memcmp(pPostMsg->permData.data, permissionData, permissionDataLen)); //Correct Permission Info
					}

					/* Check Extended Header */
					if (postMask & RSSL_PSMF_HAS_EXTENDED_HEADER)
					{
						ASSERT_TRUE( 
							pPostMsg->extendedHeader.length == extendedHeaderLen
							&& 0 == memcmp(pPostMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
				}
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}

void statusMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslStatusFlags statusMask;

	RsslStatusMsg *pStatMsg = &msg.statusMsg;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;
	/*RsslState stateMask;*/

	RsslUInt8 domainType = RSSL_DMT_MARKET_PRICE;

	//printf("statusMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			statusMask = (RsslStatusFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (statusMask & RSSL_STMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					/* Rssl does not have state flags */
					//for ( stateMask = 0; stateMask <= TEST_STATE_MAX_MASK; ++stateMask )
					//{
					//	if (stateMask && !(statusMask & RSSL_STMF_HAS_STATE) ) /* Skip redundant stateMask runs */
					//		continue;

					_setupEncodeIterator();

					/* Encode msg */

					rsslClearStatusMsg(pStatMsg);

					pStatMsg->msgBase.msgClass = RSSL_MC_STATUS;
					pStatMsg->msgBase.streamId = streamId;
					pStatMsg->msgBase.domainType = domainType;

					pStatMsg->state.code = RSSL_SC_MAX_RESERVED;
					pStatMsg->flags = statusMask;

					/* Add Item State */
					if (statusMask & RSSL_STMF_HAS_STATE)
					{
						pStatMsg->state.text.length = stateTextLen;
						pStatMsg->state.text.data = stateText;

						pStatMsg->state.streamState = RSSL_STREAM_OPEN;
						pStatMsg->state.dataState = RSSL_DATA_SUSPECT;
						pStatMsg->state.code = RSSL_SC_NO_RESOURCES;
					}

					/* Add Group ID */
					if (statusMask & RSSL_STMF_HAS_GROUP_ID)
					{
						pStatMsg->groupId.data = groupId;
						pStatMsg->groupId.length = groupIdLen;
					}

					/* Add msgKey */
					if (statusMask & RSSL_STMF_HAS_MSG_KEY)
					{
						pStatMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pStatMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}

					/* Add request msgKey */
					if (statusMask & RSSL_STMF_HAS_REQ_MSG_KEY)
					{
						pStatMsg->reqMsgKey.flags = msgKeyMask;
						_encodeMsgKey(&pStatMsg->reqMsgKey, reqKeyAttrib, reqKeyAttribLen);
					}

					/* Add Permission Info */
					if (statusMask & RSSL_STMF_HAS_PERM_DATA)
					{
						pStatMsg->permData.length = permissionDataLen;
						pStatMsg->permData.data = permissionData;
					}

					if (statusMask & RSSL_STMF_HAS_POST_USER_INFO)
					{
						pStatMsg->postUserInfo.postUserAddr = 0xCCAA551D;
						pStatMsg->postUserInfo.postUserId = 0xCCAA551D;
					}

					/* Add Extended Header */
					if (statusMask & RSSL_STMF_HAS_EXTENDED_HEADER)
					{
						pStatMsg->extendedHeader.length = extendedHeaderLen;
						pStatMsg->extendedHeader.data = extendedHeader;
					}

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;

						_postEncodeMsg(&encIter, &msg, extraAction, &pStatMsg->extendedHeader,  &pStatMsg->reqMsgKey, pStatMsg->flags, RSSL_STMF_HAS_EXTENDED_HEADER, RSSL_STMF_HAS_REQ_MSG_KEY );

						_encodePayload(&encIter);
						encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearStatusMsg(pStatMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						RsslBuffer extractGroupId, newGroupId;
						char groupIdData[32];
						newGroupId.data = groupId;
						newGroupId.length = groupIdLen;
						extractGroupId.data = groupIdData;
						extractGroupId.length = sizeof(groupIdData);

						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_STATUS == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* extract the groupId */
						ASSERT_TRUE(
							((statusMask & RSSL_STMF_HAS_GROUP_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractGroupId(&decIter, &extractGroupId)); //rsslExtractGroupId
						if (statusMask & RSSL_STMF_HAS_GROUP_ID)
							ASSERT_TRUE((extractGroupId.length == groupIdLen
									&& 0 == memcmp(extractGroupId.data, groupId, groupIdLen))); //extractGroupId
						_setupDecodeIterator();

						/* replace the groupId */
						groupId[0] = '4';
						ASSERT_TRUE( 
							((statusMask & RSSL_STMF_HAS_GROUP_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceGroupId(&encIter, newGroupId)
							); //rsslReplaceGroupId
						_setupEncodeIterator();

						/* extract the new groupId */
						ASSERT_TRUE(
							((statusMask & RSSL_STMF_HAS_GROUP_ID) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractGroupId(&decIter, &extractGroupId)); //rsslExtractGroupId
						if (statusMask & RSSL_STMF_HAS_GROUP_ID)
							ASSERT_TRUE((extractGroupId.length == newGroupId.length
									&& 0 == memcmp(extractGroupId.data, newGroupId.data, newGroupId.length))); //extractGroupId
						_setupDecodeIterator();

						/* replace the streamState */
						ASSERT_TRUE( 
							((statusMask & RSSL_STMF_HAS_STATE) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceStreamState(&encIter, RSSL_STREAM_REDIRECTED)
							); //rsslReplaceStreamState
						_setupEncodeIterator();

						/* replace the dataState */
						ASSERT_TRUE( 
							((statusMask & RSSL_STMF_HAS_STATE) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceDataState(&encIter, RSSL_DATA_OK)
							); //rsslReplaceDataState
						_setupEncodeIterator();

						/* replace the stateCode */
						ASSERT_TRUE( 
							((statusMask & RSSL_STMF_HAS_STATE) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceStateCode(&encIter, RSSL_SC_INVALID_VIEW)
							); //rsslReplaceStateCode
						_setupEncodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_STATUS
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pStatMsg->flags == statusMask); //Correct Mask

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.statusMsg.reqMsgKey.encAttrib.data || copyMsg->statusMsg.reqMsgKey.encAttrib.data != msg.statusMsg.reqMsgKey.encAttrib.data); //reqMsgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->statusMsg.reqMsgKey.encAttrib.length == msg.statusMsg.reqMsgKey.encAttrib.length &&
								0 == memcmp(copyMsg->statusMsg.reqMsgKey.encAttrib.data, msg.statusMsg.reqMsgKey.encAttrib.data, msg.statusMsg.reqMsgKey.encAttrib.length)); //reqMsgKey.encAttrib is correct
						ASSERT_TRUE(!msg.statusMsg.reqMsgKey.name.data || copyMsg->statusMsg.reqMsgKey.name.data != msg.statusMsg.reqMsgKey.name.data); //reqMsgKey.name deep copied
						ASSERT_TRUE(copyMsg->statusMsg.reqMsgKey.name.length == msg.statusMsg.reqMsgKey.name.length &&
								0 == memcmp(copyMsg->statusMsg.reqMsgKey.name.data, msg.statusMsg.reqMsgKey.name.data, msg.statusMsg.reqMsgKey.name.length)); //reqMsgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.statusMsg.permData.data || copyMsg->statusMsg.permData.data != msg.statusMsg.permData.data); //permData deep copied
						ASSERT_TRUE(copyMsg->statusMsg.permData.length == msg.statusMsg.permData.length &&
								0 == memcmp(copyMsg->statusMsg.permData.data, msg.statusMsg.permData.data, msg.statusMsg.permData.length)); //permData is correct
						ASSERT_TRUE(!msg.statusMsg.extendedHeader.data || copyMsg->statusMsg.extendedHeader.data != msg.statusMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->statusMsg.extendedHeader.length == msg.statusMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->statusMsg.extendedHeader.data, msg.statusMsg.extendedHeader.data, msg.statusMsg.extendedHeader.length)); //extendedHeader is correct
						ASSERT_TRUE(!msg.statusMsg.state.text.data || copyMsg->statusMsg.state.text.data != msg.statusMsg.state.text.data); //state text deep copied
						ASSERT_TRUE(copyMsg->statusMsg.state.text.length == msg.statusMsg.state.text.length &&
								0 == memcmp(copyMsg->statusMsg.state.text.data, msg.statusMsg.state.text.data, msg.statusMsg.state.text.length)); //state text is correct
						ASSERT_TRUE(!msg.statusMsg.groupId.data || copyMsg->statusMsg.groupId.data != msg.statusMsg.groupId.data); //groupId deep copied
						ASSERT_TRUE(copyMsg->statusMsg.groupId.length == msg.statusMsg.groupId.length &&
								0 == memcmp(copyMsg->statusMsg.groupId.data, msg.statusMsg.groupId.data, msg.statusMsg.groupId.length)); //groupId is correct
						free(copyBuffer.data);
					}

					/* Check Item State */
					if (statusMask & RSSL_STMF_HAS_STATE)
					{
						ASSERT_TRUE( 
							pStatMsg->state.streamState == (g_changeData ? RSSL_STREAM_REDIRECTED : RSSL_STREAM_OPEN)); //Correct Stream State

						ASSERT_TRUE(
							pStatMsg->state.dataState == (g_changeData ? RSSL_DATA_OK : RSSL_DATA_SUSPECT)
							&& pStatMsg->state.code == (g_changeData ? RSSL_SC_INVALID_VIEW : RSSL_SC_NO_RESOURCES)
							&& pStatMsg->state.text.length == stateTextLen && 0 == memcmp(pStatMsg->state.text.data, stateText, stateTextLen)
							); //Correct Item State
					}

					if (statusMask & RSSL_STMF_HAS_GROUP_ID)
					{
						ASSERT_TRUE(
							(pStatMsg->groupId.length == groupIdLen
							&& 0 == memcmp(pStatMsg->groupId.data, groupId, groupIdLen))); //Correct Group ID
					}

					/* Check msgKey */
					if (statusMask & RSSL_STMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( 
							pStatMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pStatMsg->msgBase.msgKey);
					}

					/* Check Permission Info */
					if (statusMask & RSSL_STMF_HAS_PERM_DATA)
					{
						ASSERT_TRUE( 
							pStatMsg->permData.length == permissionDataLen 
							&& 0 == memcmp(pStatMsg->permData.data, permissionData, permissionDataLen)); //Correct Permission Info
					}

					if (statusMask & RSSL_STMF_HAS_POST_USER_INFO)
					{
						ASSERT_TRUE(
							pStatMsg->postUserInfo.postUserAddr == 0xCCAA551D); //Correct Post User Addr

						ASSERT_TRUE( pStatMsg->postUserInfo.postUserId == 0xCCAA551D); //Correct Post User ID
					}


					/* Check Extended Header */
					if (statusMask & RSSL_STMF_HAS_EXTENDED_HEADER)
					{
						ASSERT_TRUE( 
							pStatMsg->extendedHeader.length == extendedHeaderLen
							&& 0 == memcmp(pStatMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
					/*}*/
				}
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}

void requestMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslRequestFlags requestMask;
	RsslRequestMsg *pReqMsg = &msg.requestMsg;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;

	RsslUInt8 domainType = RSSL_DMT_DICTIONARY;

	//printf("requestMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			requestMask = (RsslRequestFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < keyMasksSize /*reqMsg always has key*/; ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					_setupEncodeIterator();

					/* Encode msg */
					rsslClearRequestMsg(pReqMsg);

					pReqMsg->msgBase.msgClass = RSSL_MC_REQUEST;
					pReqMsg->msgBase.streamId = streamId;
					pReqMsg->msgBase.domainType = domainType;

					pReqMsg->flags = requestMask;

					/* Add msg key */
					pReqMsg->msgBase.msgKey.flags = msgKeyMask;
					_encodeMsgKey(&pReqMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);

					/* Add priority info */
					if ( requestMask & RSSL_RQMF_HAS_PRIORITY )
					{
						pReqMsg->priorityClass = 3;
						pReqMsg->priorityCount = 4;
					}

					/* Add best QoS */
					if ( requestMask & RSSL_RQMF_HAS_QOS )
					{
						pReqMsg->qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
						pReqMsg->qos.rateInfo = 65535;
						pReqMsg->qos.timeliness = RSSL_QOS_TIME_DELAYED;
						pReqMsg->qos.timeInfo = 65534;
					}

					/* Add worst QoS */
					if ( requestMask & RSSL_RQMF_HAS_WORST_QOS )
					{
						pReqMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
						pReqMsg->worstQos.rateInfo = 65533;
						pReqMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
						pReqMsg->worstQos.timeInfo = 65532;
					}

					/* Add extended header */
					if ( requestMask & RSSL_RQMF_HAS_EXTENDED_HEADER)
					{
						pReqMsg->extendedHeader.length = extendedHeaderLen;
						pReqMsg->extendedHeader.data = extendedHeader;
					}

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;

						_postEncodeMsg(&encIter, &msg, extraAction, &pReqMsg->extendedHeader, NULL, pReqMsg->flags, RSSL_RQMF_HAS_EXTENDED_HEADER, 0 );

						_encodePayload(&encIter);
						encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearRequestMsg(pReqMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_REQUEST == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg

					/* Check mask and msgBase */
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_REQUEST
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pReqMsg->flags == requestMask); //Correct Mask

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.requestMsg.extendedHeader.data || copyMsg->requestMsg.extendedHeader.data != msg.requestMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->requestMsg.extendedHeader.length == msg.requestMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->requestMsg.extendedHeader.data, msg.requestMsg.extendedHeader.data, msg.requestMsg.extendedHeader.length)); //extendedHeader is correct
						free(copyBuffer.data);
					}

					/* Check msg key */
					ASSERT_TRUE(pReqMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Key 
					_decodeMsgKey(&decIter, &pReqMsg->msgBase.msgKey);

					/* Check priority info */
					if ( requestMask & RSSL_RQMF_HAS_PRIORITY )
					{
						ASSERT_TRUE(pReqMsg->priorityClass == 3
							&& pReqMsg->priorityCount == 4); //Correct Priority
					}

					/* Check best QoS */
					if ( requestMask & RSSL_RQMF_HAS_QOS )
					{
						ASSERT_TRUE(
							pReqMsg->qos.rate == RSSL_QOS_RATE_TIME_CONFLATED
							&& pReqMsg->qos.rateInfo == 65535
							&& pReqMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED
							&& pReqMsg->qos.timeInfo == 65534); //Correct Best QoS
					}

					/* Check worst QoS */
					if ( requestMask & RSSL_RQMF_HAS_WORST_QOS )
					{
						ASSERT_TRUE(
							pReqMsg->worstQos.rate == RSSL_QOS_RATE_TIME_CONFLATED 
							&& pReqMsg->worstQos.rateInfo == 65533
							&& pReqMsg->worstQos.timeliness == RSSL_QOS_TIME_DELAYED
							&& pReqMsg->worstQos.timeInfo == 65532); //Correct Worst QoS
					}

					/* Check extended header */
					if ( requestMask & RSSL_RQMF_HAS_EXTENDED_HEADER )
					{
						ASSERT_TRUE(
							pReqMsg->extendedHeader.length = extendedHeaderLen
							&& 0 == memcmp(pReqMsg->extendedHeader.data,  extendedHeader, pReqMsg->extendedHeader.length)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
				}
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}

void refreshMsgTest(RsslUInt32 repeat)
{
	ExtraAction extraAction;

	RsslRefreshFlags responseMask;

	RsslRefreshMsg *pRespMsg = &msg.refreshMsg;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;
	/*StateMasks stateMask;*/

	RsslUInt8 domainType = RSSL_DMT_LOGIN;

	//printf("refreshMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			responseMask = (RsslRefreshFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (responseMask & RSSL_RFMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					/* Rssl States do not have flags */
					/*for ( stateMask = 0; stateMask <= TEST_STATE_MAX_MASK; ++stateMask)
					{*/
					_setupEncodeIterator();

					/* Encode msg */

					rsslClearRefreshMsg(pRespMsg);

					pRespMsg->msgBase.msgClass = RSSL_MC_REFRESH;
					pRespMsg->msgBase.streamId = streamId;
					pRespMsg->msgBase.domainType = domainType;

					pRespMsg->flags = responseMask;

					pRespMsg->state.streamState = RSSL_STREAM_OPEN;
					pRespMsg->state.dataState = RSSL_DATA_SUSPECT;
					pRespMsg->state.code = RSSL_SC_NO_RESOURCES;
					pRespMsg->state.text.length = stateTextLen;
					pRespMsg->state.text.data = stateText;

					pRespMsg->groupId.data = groupId;
					pRespMsg->groupId.length = groupIdLen;

					/* Add msgKey */
					if (responseMask & RSSL_RFMF_HAS_MSG_KEY)
					{
						pRespMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pRespMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}
					
					/* Add request msgKey */
					if (responseMask & RSSL_RFMF_HAS_REQ_MSG_KEY)
					{
						pRespMsg->reqMsgKey.flags = msgKeyMask;
						_encodeMsgKey(&pRespMsg->reqMsgKey, reqKeyAttrib, reqKeyAttribLen);
					}

					/* Add Permission Info */
					if (responseMask & RSSL_RFMF_HAS_PERM_DATA)
					{
						pRespMsg->permData.length = permissionDataLen;
						pRespMsg->permData.data = permissionData;
					}

					/* Add Item Sequence Number */
					if (responseMask & RSSL_RFMF_HAS_SEQ_NUM)
					{
						seqNum = (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32)(responseMask + 0xFFFF): (RsslUInt32)responseMask;
						pRespMsg->seqNum = seqNum;
					}

					if (responseMask & RSSL_RFMF_HAS_POST_USER_INFO)
					{
						pRespMsg->postUserInfo.postUserAddr = 0xCCAA551D;
						pRespMsg->postUserInfo.postUserId = 0xCCAA551D;
					}

					if (responseMask & RSSL_RFMF_HAS_PART_NUM)
					{
						pRespMsg->partNum = 0x100f;
					}

					/* Add QoS */
					if (responseMask & RSSL_RFMF_HAS_QOS)
					{
						pRespMsg->qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
						pRespMsg->qos.rateInfo = 65535;
						pRespMsg->qos.timeliness = RSSL_QOS_TIME_DELAYED;
						pRespMsg->qos.timeInfo = 65534;
					}

					/* Add Extended Header */
					if (responseMask & RSSL_RFMF_HAS_EXTENDED_HEADER)
					{
						pRespMsg->extendedHeader.length = extendedHeaderLen;
						pRespMsg->extendedHeader.data = extendedHeader;
					}

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;

						_postEncodeMsg(&encIter, &msg, extraAction, &pRespMsg->extendedHeader,  &pRespMsg->reqMsgKey, pRespMsg->flags, RSSL_RFMF_HAS_EXTENDED_HEADER, RSSL_RFMF_HAS_REQ_MSG_KEY );

						_encodePayload(&encIter);
						encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearRefreshMsg(pRespMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						RsslUInt32 extractSeqNum;
						RsslBuffer extractGroupId, newGroupId;
						char groupIdData[32];
						newGroupId.data = groupId;
						newGroupId.length = groupIdLen;
						extractGroupId.data = groupIdData;
						extractGroupId.length = sizeof(groupIdData);

						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_REFRESH == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* extract the seqNum */
						ASSERT_TRUE(
							((responseMask & RSSL_RFMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (responseMask & RSSL_RFMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum); //extractSeqNum
						_setupDecodeIterator();

						/* replace the seqNum */
						ASSERT_TRUE( 
							((responseMask & RSSL_RFMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceSeqNum(&encIter, seqNum + 1)
							); //rsslReplaceSeqNum
						_setupEncodeIterator();

						/* extract the new seqNum */
						ASSERT_TRUE(
							((responseMask & RSSL_RFMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (responseMask & RSSL_RFMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum + 1); //extractSeqNum
						_setupDecodeIterator();

						/* extract the groupId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslExtractGroupId(&decIter, &extractGroupId)); //rsslExtractGroupId
						ASSERT_TRUE((extractGroupId.length == groupIdLen
									&& 0 == memcmp(extractGroupId.data, groupId, groupIdLen))); //extractGroupId
						_setupDecodeIterator();

						/* replace the groupId */
						groupId[0] = '4';
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceGroupId(&encIter, newGroupId)); //rsslReplaceGroupId
						_setupEncodeIterator();

						/* extract the new groupId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslExtractGroupId(&decIter, &extractGroupId)); //rsslExtractGroupId
						ASSERT_TRUE((extractGroupId.length == newGroupId.length
									&& 0 == memcmp(extractGroupId.data, newGroupId.data, newGroupId.length))); //extractGroupId
						_setupDecodeIterator();

						/* replace the streamState */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamState(&encIter, RSSL_STREAM_REDIRECTED)); //rsslReplaceStreamState
						_setupEncodeIterator();

						/* replace the dataState */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDataState(&encIter, RSSL_DATA_OK)); //rsslReplaceDataState
						_setupEncodeIterator();

						/* replace the stateCode */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStateCode(&encIter, RSSL_SC_INVALID_VIEW)); //rsslReplaceStateCode
						_setupEncodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_REFRESH
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pRespMsg->flags == responseMask); //Correct Mask

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.refreshMsg.reqMsgKey.encAttrib.data || copyMsg->refreshMsg.reqMsgKey.encAttrib.data != msg.refreshMsg.reqMsgKey.encAttrib.data); //reqMsgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.reqMsgKey.encAttrib.length == msg.refreshMsg.reqMsgKey.encAttrib.length &&
								0 == memcmp(copyMsg->refreshMsg.reqMsgKey.encAttrib.data, msg.refreshMsg.reqMsgKey.encAttrib.data, msg.refreshMsg.reqMsgKey.encAttrib.length)); //reqMsgKey.encAttrib is correct
						ASSERT_TRUE(!msg.refreshMsg.reqMsgKey.name.data || copyMsg->refreshMsg.reqMsgKey.name.data != msg.refreshMsg.reqMsgKey.name.data); //reqMsgKey.name deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.reqMsgKey.name.length == msg.refreshMsg.reqMsgKey.name.length &&
								0 == memcmp(copyMsg->refreshMsg.reqMsgKey.name.data, msg.refreshMsg.reqMsgKey.name.data, msg.refreshMsg.reqMsgKey.name.length)); //reqMsgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.refreshMsg.permData.data || copyMsg->refreshMsg.permData.data != msg.refreshMsg.permData.data); //permData deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.permData.length == msg.refreshMsg.permData.length &&
								0 == memcmp(copyMsg->refreshMsg.permData.data, msg.refreshMsg.permData.data, msg.refreshMsg.permData.length)); //permData is correct
						ASSERT_TRUE(!msg.refreshMsg.extendedHeader.data || copyMsg->refreshMsg.extendedHeader.data != msg.refreshMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.extendedHeader.length == msg.refreshMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->refreshMsg.extendedHeader.data, msg.refreshMsg.extendedHeader.data, msg.refreshMsg.extendedHeader.length)); //extendedHeader is correct
						ASSERT_TRUE(!msg.refreshMsg.state.text.data || copyMsg->refreshMsg.state.text.data != msg.refreshMsg.state.text.data); //state text deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.state.text.length == msg.refreshMsg.state.text.length &&
								0 == memcmp(copyMsg->refreshMsg.state.text.data, msg.refreshMsg.state.text.data, msg.refreshMsg.state.text.length)); //state text is correct
						ASSERT_TRUE(!msg.refreshMsg.groupId.data || copyMsg->refreshMsg.groupId.data != msg.refreshMsg.groupId.data); //groupId deep copied
						ASSERT_TRUE(copyMsg->refreshMsg.groupId.length == msg.refreshMsg.groupId.length &&
								0 == memcmp(copyMsg->refreshMsg.groupId.data, msg.refreshMsg.groupId.data, msg.refreshMsg.groupId.length)); //groupId is correct
						free(copyBuffer.data);
					}

					/* Check msgKey */
					if (responseMask & RSSL_RFMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( pRespMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pRespMsg->msgBase.msgKey);
					}

					/* Check State */
					ASSERT_TRUE( 
						pRespMsg->state.streamState == (g_changeData ? RSSL_STREAM_REDIRECTED : RSSL_STREAM_OPEN)); //Correct Stream State

					ASSERT_TRUE(
						pRespMsg->state.dataState == (g_changeData ? RSSL_DATA_OK : RSSL_DATA_SUSPECT)
						&& pRespMsg->state.code == (g_changeData ? RSSL_SC_INVALID_VIEW : RSSL_SC_NO_RESOURCES)
						&& pRespMsg->groupId.length == groupIdLen
						&& 0 == memcmp(pRespMsg->groupId.data, groupId, groupIdLen)
						&& pRespMsg->state.text.length == stateTextLen && 0 == memcmp(pRespMsg->state.text.data, stateText, stateTextLen)
						); //Correct Item State

					/* Check Permission Info */ 
					if (responseMask & RSSL_RFMF_HAS_PERM_DATA)
					{
						ASSERT_TRUE( 
							pRespMsg->permData.length == permissionDataLen 
							&& 0 == memcmp(pRespMsg->permData.data, permissionData, permissionDataLen)); //Correct Permission Info
					}

					/* Check Item Sequence Number */
					if (responseMask & RSSL_RFMF_HAS_SEQ_NUM)
					{
						ASSERT_TRUE( 
							pRespMsg->seqNum == (g_changeData ? seqNum + 1 : seqNum)); //Correct Item Sequence Number
					}

					if (responseMask & RSSL_RFMF_HAS_POST_USER_INFO)
					{
						ASSERT_TRUE(
							pRespMsg->postUserInfo.postUserAddr == 0xCCAA551D); //Correct Post User Addr

						ASSERT_TRUE( pRespMsg->postUserInfo.postUserId == 0xCCAA551D); //Correct Post User ID
					}

					if (responseMask & RSSL_RFMF_HAS_PART_NUM)
					{
						ASSERT_TRUE(pRespMsg->partNum == 0x100f); //Correct PartNum
					}

					/* Check QoS */
					if (responseMask & RSSL_RFMF_HAS_QOS)
					{
						ASSERT_TRUE( 
							pRespMsg->qos.rate == RSSL_QOS_RATE_TIME_CONFLATED
							&& pRespMsg->qos.rateInfo == 65535
							&& pRespMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED
							&& pRespMsg->qos.timeInfo == 65534); //Correct QoS
					}

					/* Check Extended Header */
					if (responseMask & RSSL_RFMF_HAS_EXTENDED_HEADER)
					{
						ASSERT_TRUE( 
							pRespMsg->extendedHeader.length == extendedHeaderLen
							&& 0 == memcmp(pRespMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
					/*}*/
				}
			}
		}
	}

	endTimerAndPrint();

	//printf("\n");
}

void updateMsgTest(RsslUInt32 repeat)
{
	RsslUpdateFlags updateMask;

	ExtraAction extraAction;

	RsslUpdateMsg *pUpdMsg = &msg.updateMsg;

	RsslMsgKeyFlags msgKeyMask;
	RsslMsgKeyFlags decodeMsgKeyMask;

	RsslUInt8 domainType = RSSL_DMT_LOGIN;

	//printf("updateMsg Tests:\n");

	startTimer();

	for (; repeat > 0; --repeat)
	{
		for ( masksIter = 0; masksIter < masksSize; ++masksIter )
		{
			updateMask = (RsslUpdateFlags)masks[masksIter];

			for ( actionsIter = 0; actionsIter < actionsSize; ++actionsIter )
			{
				extraAction = (ExtraAction)actions[actionsIter];
				for ( keyMasksIter = 0; keyMasksIter < (updateMask & RSSL_UPMF_HAS_MSG_KEY ? keyMasksSize : 1); ++keyMasksIter )
				{
					msgKeyMask = (RsslMsgKeyFlags)keyMasks[keyMasksIter];
					_setupEncodeIterator();

					/* Encode msg */

					rsslClearUpdateMsg(pUpdMsg);

					pUpdMsg->msgBase.msgClass = RSSL_MC_UPDATE;
					pUpdMsg->msgBase.streamId = streamId;
					pUpdMsg->msgBase.domainType = domainType;

					pUpdMsg->flags = updateMask;
					pUpdMsg->updateType = 3;

					/* Add msgKey */
					if (updateMask & RSSL_UPMF_HAS_MSG_KEY)
					{
						pUpdMsg->msgBase.msgKey.flags = msgKeyMask;
						_encodeMsgKey(&pUpdMsg->msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
					}

					/* Add Permission Info */ 
					if (updateMask & RSSL_UPMF_HAS_PERM_DATA)
					{
						pUpdMsg->permData.length = permissionDataLen;
						pUpdMsg->permData.data = permissionData;
					}

					/* Add Item Sequence Number */
					if (updateMask & RSSL_UPMF_HAS_SEQ_NUM)
					{
						seqNum = (extraAction & TEST_ACTION_4BYTE_SEQ ) ? (RsslUInt32)(updateMask + 0xFFFF): (RsslUInt32)updateMask;
						pUpdMsg->seqNum = seqNum;
					}

					/* Add Conflation Info */
					if (updateMask & RSSL_UPMF_HAS_CONF_INFO)
					{
						pUpdMsg->conflationCount= (RsslUInt16)updateMask+2; 
						pUpdMsg->conflationTime = (RsslUInt16)updateMask+3; 
					}


					if (updateMask & RSSL_UPMF_HAS_POST_USER_INFO)
					{
						pUpdMsg->postUserInfo.postUserAddr = 0xCCAA551D;
						pUpdMsg->postUserInfo.postUserId = 0xCCAA551D;
					}

					/* Add Extended Header */
					if (updateMask & RSSL_UPMF_HAS_EXTENDED_HEADER)
					{
						pUpdMsg->extendedHeader.length = extendedHeaderLen;
						pUpdMsg->extendedHeader.data = extendedHeader;
					}

					/* Add Payload */
					if ( extraAction & TEST_ACTION_PRE_PAYLOAD )
					{
						msg.msgBase.encDataBody.data = encDataBuf.data;
						msg.msgBase.containerType = g_dataFormat;

						/* Trim payload length if desired */
						msg.msgBase.encDataBody.length = (extraAction & TEST_ACTION_TRIM_DATA_BUF) ? rsslGetEncodedBufferLength(&encDataIter) : encDataBuf.length;

						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}
					else if ( extraAction & TEST_ACTION_POST_PAYLOAD )
					{
						msg.msgBase.containerType = g_dataFormat;
	
						_postEncodeMsg(&encIter, &msg, extraAction, &pUpdMsg->extendedHeader, NULL, pUpdMsg->flags, RSSL_UPMF_HAS_EXTENDED_HEADER, 0 );

						_encodePayload(&encIter);
						encDataBuf.length = rsslGetEncodedBufferLength(&encDataIter);
						
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
					}
					else
					{
						msg.msgBase.encDataBody.data = 0;
						msg.msgBase.encDataBody.length = 0;
						msg.msgBase.containerType = RSSL_DT_NO_DATA;
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsg( &encIter, &msg)); //rsslEncodeMsg
					}

					encBuf.length = rsslGetEncodedBufferLength(&encIter);

					if (g_testDecoding == RSSL_FALSE) continue;

					/* Decode msg */
					decodeMsgKeyMask = _fixDecodeMsgKeyMask(msgKeyMask);

					rsslClearUpdateMsg(pUpdMsg);
					_setupDecodeIterator();
					_setupEncodeIterator();

					if (g_changeData)
					{
						RsslUInt32 extractSeqNum;

						/* extract the msgClass */
						ASSERT_TRUE(RSSL_MC_UPDATE == rsslExtractMsgClass(&decIter)); //rsslExtractMsgClass
						_setupDecodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* replace the domainType */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceDomainType(&encIter, domainType + 1)); //rsslReplaceDomainType
						_setupEncodeIterator();

						/* extract the domainType */
						ASSERT_TRUE(domainType + 1 == rsslExtractDomainType(&decIter)); //rsslExtractDomainType
						_setupDecodeIterator();

						/* extract streamId */
						ASSERT_TRUE(streamId == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* replace the streamId */
						ASSERT_TRUE(RSSL_RET_SUCCESS == rsslReplaceStreamId(&encIter, streamId + 1)); //rsslReplaceStreamId
						_setupEncodeIterator();

						/* extract the streamId */
						ASSERT_TRUE(streamId + 1 == rsslExtractStreamId(&decIter)); //rsslExtractStreamId
						_setupDecodeIterator();

						/* extract the seqNum */
						ASSERT_TRUE(
							((updateMask & RSSL_UPMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (updateMask & RSSL_UPMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum); //extractSeqNum
						_setupDecodeIterator();

						/* replace the seqNum */
						ASSERT_TRUE( 
							((updateMask & RSSL_UPMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslReplaceSeqNum(&encIter, seqNum + 1)
							); //rsslReplaceSeqNum
						_setupEncodeIterator();

						/* extract the new seqNum */
						ASSERT_TRUE(
							((updateMask & RSSL_UPMF_HAS_SEQ_NUM) ? RSSL_RET_SUCCESS : RSSL_RET_FAILURE)
							== rsslExtractSeqNum(&decIter, &extractSeqNum)); //rsslExtractSeqNum
						if (updateMask & RSSL_UPMF_HAS_SEQ_NUM) ASSERT_TRUE(extractSeqNum == seqNum + 1); //extractSeqNum
						_setupDecodeIterator();
					}

					ASSERT_TRUE(RSSL_RET_SUCCESS == rsslDecodeMsg(&decIter, &msg)); //DecodeMsg
					ASSERT_TRUE(
						msg.msgBase.msgClass == RSSL_MC_UPDATE
						&& msg.msgBase.containerType == ((extraAction & TEST_ACTION_PAYLOAD ) ? g_dataFormat : RSSL_DT_NO_DATA)
						&& msg.msgBase.streamId == (g_changeData ? streamId + 1 : streamId)
						&& msg.msgBase.domainType == (g_changeData ? domainType + 1 : domainType)); //Correct MsgBase

					ASSERT_TRUE(
						pUpdMsg->flags == updateMask); //Correct Mask

					ASSERT_TRUE(
						pUpdMsg->updateType == 3); //Correct update Type

					/* copy test */
					{
						RsslMsg *copyMsg;
						RsslBuffer copyBuffer = RSSL_INIT_BUFFER;
						int msgSize = rsslSizeOfMsg(&msg, RSSL_CMF_ALL_FLAGS);
						copyBuffer.data = (char*)malloc(msgSize);
						copyBuffer.length = msgSize;
						copyMsg = rsslCopyMsg(&msg, RSSL_CMF_ALL_FLAGS, 0, &copyBuffer);
						ASSERT_TRUE(!msg.msgBase.msgKey.encAttrib.data || copyMsg->msgBase.msgKey.encAttrib.data != msg.msgBase.msgKey.encAttrib.data); //msgKey.encAttrib deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.encAttrib.length == msg.msgBase.msgKey.encAttrib.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.data, msg.msgBase.msgKey.encAttrib.length)); //msgKey.encAttrib is correct
						ASSERT_TRUE(!msg.msgBase.msgKey.name.data || copyMsg->msgBase.msgKey.name.data != msg.msgBase.msgKey.name.data); //msgKey.name deep copied
						ASSERT_TRUE(copyMsg->msgBase.msgKey.name.length == msg.msgBase.msgKey.name.length &&
								0 == memcmp(copyMsg->msgBase.msgKey.name.data, msg.msgBase.msgKey.name.data, msg.msgBase.msgKey.name.length)); //msgKey.name is correct
						ASSERT_TRUE(!msg.msgBase.encDataBody.data || copyMsg->msgBase.encDataBody.data != msg.msgBase.encDataBody.data); //encDataBody deep copied encode
						ASSERT_TRUE(copyMsg->msgBase.encDataBody.length == msg.msgBase.encDataBody.length &&
								0 == memcmp(copyMsg->msgBase.encDataBody.data, msg.msgBase.encDataBody.data, msg.msgBase.encDataBody.length)); //encDataBody is correct
						ASSERT_TRUE(!msg.updateMsg.permData.data || copyMsg->updateMsg.permData.data != msg.updateMsg.permData.data); //permData deep copied
						ASSERT_TRUE(copyMsg->updateMsg.permData.length == msg.updateMsg.permData.length &&
								0 == memcmp(copyMsg->updateMsg.permData.data, msg.updateMsg.permData.data, msg.updateMsg.permData.length)); //permData is correct
						ASSERT_TRUE(!msg.updateMsg.extendedHeader.data || copyMsg->updateMsg.extendedHeader.data != msg.updateMsg.extendedHeader.data); //extendedHeader deep copied
						ASSERT_TRUE(copyMsg->updateMsg.extendedHeader.length == msg.updateMsg.extendedHeader.length &&
								0 == memcmp(copyMsg->updateMsg.extendedHeader.data, msg.updateMsg.extendedHeader.data, msg.updateMsg.extendedHeader.length)); //extendedHeader is correct
						free(copyBuffer.data);
					}

					/* Check msgKey */
					if (updateMask & RSSL_UPMF_HAS_MSG_KEY)
					{
						ASSERT_TRUE( 
							pUpdMsg->msgBase.msgKey.flags == decodeMsgKeyMask); //Correct Msg Key Mask
						_decodeMsgKey(&decIter, &pUpdMsg->msgBase.msgKey);
					}

					/* Check Permission Info */
					if (updateMask & RSSL_UPMF_HAS_PERM_DATA)
					{
						ASSERT_TRUE( 
							pUpdMsg->permData.length == permissionDataLen 
							&& 0 == memcmp(pUpdMsg->permData.data, permissionData, permissionDataLen)); //Correct Permission Info
					}

					/* Check Item Sequence Number */
					if (updateMask & RSSL_UPMF_HAS_SEQ_NUM)
					{
						ASSERT_TRUE( 
							pUpdMsg->seqNum == (g_changeData ? seqNum + 1 : seqNum)); //Correct Item Sequence Number
					}

					/* Check Conflation Info */
					if (updateMask & RSSL_UPMF_HAS_CONF_INFO)
					{
						ASSERT_TRUE( 
							pUpdMsg->conflationCount == (RsslUInt16)updateMask+2
							&& pUpdMsg->conflationTime == (RsslUInt16)updateMask+3); //Correct Conflation Info
					}

					if (updateMask & RSSL_UPMF_HAS_POST_USER_INFO)
					{
						ASSERT_TRUE(
							pUpdMsg->postUserInfo.postUserAddr == 0xCCAA551D); //Correct Post User Addr

						ASSERT_TRUE( pUpdMsg->postUserInfo.postUserId == 0xCCAA551D); //Correct Post User ID
					}


					/* Check Extended Header */
					if (updateMask & RSSL_UPMF_HAS_EXTENDED_HEADER)
					{
						ASSERT_TRUE( 
							pUpdMsg->extendedHeader.length == extendedHeaderLen
							&& 0 == memcmp(pUpdMsg->extendedHeader.data, extendedHeader, extendedHeaderLen)); //Correct Extended Header
					}

					/* Check Payload */
					if ( extraAction & TEST_ACTION_PAYLOAD )
					{
						_decodePayload(&decIter);
					}
				}
			}

		}
	}


	endTimerAndPrint();

	//printf("\n");
}

/* _allocateFlagCombinations()
 *   Pass in an array of masks(masksBase), this will allocate 'masks' with a block containing all combinations of those flags 
 *     (including all & none).
 */
RsslUInt32 _allocateFlagCombinations(RsslUInt32** dstMasks, const RsslUInt32* srcMasks, RsslUInt32 srcMasksSize, RsslBool skipZero)
{
	RsslUInt32 skip = skipZero? 1 : 0;
	RsslUInt32 dstMasksSize = (RsslUInt32)pow((double)2, (int)srcMasksSize) - skip;
	RsslUInt32 srcMasksIter, dstMasksIter;

	*dstMasks =  (RsslUInt32*)malloc(dstMasksSize * sizeof(RsslUInt32));

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

/* Adapted from RFA examples' CtrlBreakHandler */
#ifdef WIN32
void sleepTest( long msec )
{
	startTimer();
	Sleep(msec);
	endTimerAndPrint();
}
#else
#define MILLISEC 1000
struct timespec sleeptime;
void sleepTest( long msec )
{
	sleeptime.tv_sec = msec / MILLISEC;
	sleeptime.tv_nsec = (msec % MILLISEC) * 1000000;
	startTimer();
	nanosleep(&sleeptime,0);
	endTimerAndPrint();
}
#endif

void copyKeyTest()
{
	char nameSource[32], attribSource[32], nameDest[32], attribDest[32];
	RsslMsgKey keySource = RSSL_INIT_MSG_KEY;
	RsslMsgKey keyDest = RSSL_INIT_MSG_KEY;

	//printf("rsslCopyMsgKey Tests:\n");
	startTimer();

	strcpy(nameSource, "name from source");
	strcpy(attribSource, "attribute from source");

	keySource.name.data = nameSource;
	keyDest.name.data = 0;
	keySource.name.length = sizeof(nameSource);
	keyDest.name.length = 0;

	keySource.attribContainerType = RSSL_DT_OPAQUE;
	keySource.encAttrib.data = attribSource;
	keyDest.encAttrib.data = 0;
	keySource.encAttrib.length = sizeof(attribSource);
	keyDest.encAttrib.length = 0;

	/* pass - no flags set */
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_SUCCESS); //copyKeyTest with no flags set failed
	ASSERT_TRUE(rsslCompareMsgKeys(&keyDest, &keySource) == 0); //copyKeyTest with no flags set failed

	/* fail - 0 length name in destination key */
	keySource.flags = 0;
	keySource.flags |= RSSL_MKF_HAS_NAME;
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_BUFFER_TOO_SMALL); //copyKeyTest with 0 length name in destination key failed

	/* fail - 0 length attrib in destination key */
	keySource.flags = 0;
	keySource.flags |= RSSL_MKF_HAS_ATTRIB;
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_BUFFER_TOO_SMALL); //copyKeyTest with 0 length attrib in destination key failed

	/* fail - null name in destination key */
	keySource.flags = 0;
	keySource.flags |= RSSL_MKF_HAS_NAME;
	keyDest.name.length = sizeof(nameDest);;
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_FAILURE); //copyKeyTest with null name in destination key failed

	/* fail - null attrib in destination key */
	keySource.flags = 0;
	keySource.flags |= RSSL_MKF_HAS_ATTRIB;
	keyDest.encAttrib.length = sizeof(attribDest);
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_FAILURE); //copyKeyTest with null attrib in destination key failed

	/* pass - sufficient memory for destination key name and attrib */
	keySource.flags = 0;
	keySource.flags |= RSSL_MKF_HAS_NAME;
	keySource.flags |= RSSL_MKF_HAS_ATTRIB;
	keyDest.name.data = nameDest;
	keyDest.name.length = sizeof(nameDest);
	keyDest.encAttrib.data = attribDest;
	keyDest.encAttrib.length = sizeof(attribDest);
	ASSERT_TRUE(rsslCopyMsgKey(&keyDest, &keySource) == RSSL_RET_SUCCESS); //copyKeyTest with sufficient memory for destination key name and attrib failed
	ASSERT_TRUE(rsslCompareMsgKeys(&keyDest, &keySource) == 0); //copyKeyTest with sufficient memory for destination key name and attrib failed

	endTimerAndPrint();
	//printf("\n");
}

void copyAckMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, ackId = 255;
	RsslUInt8 nameType = 255, nakCode = RSSL_NAKC_INVALID_CONTENT;
	RsslAckMsg ackMsg = RSSL_INIT_ACK_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, textBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	textBuf.data = text;
	textBuf.length = sizeof(text);
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	ackMsg.msgBase.msgClass = RSSL_MC_ACK;
	ackMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	ackMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	ackMsg.msgBase.streamId = streamId;
	rsslAckMsgApplyHasMsgKey(&ackMsg);
	rsslMsgKeyApplyHasServiceId(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	ackMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.identifier = identifier;
	ackMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&ackMsg.msgBase.msgKey);
	ackMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	ackMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	ackMsg.msgBase.encDataBody.data = encDataBuf.data;
	ackMsg.msgBase.encDataBody.length = encDataBuf.length;
	ackMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	ackMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslAckMsg */
	ackMsg.ackId = ackId;
	rsslAckMsgApplyHasNakCode(&ackMsg);
	ackMsg.nakCode = nakCode;
	rsslAckMsgApplyHasSeqNum(&ackMsg);
	ackMsg.seqNum = seqNum;
	rsslAckMsgApplyHasText(&ackMsg);
	ackMsg.text.data = textBuf.data;
	ackMsg.text.length = textBuf.length;
	rsslAckMsgApplyHasExtendedHdr(&ackMsg);
	ackMsg.extendedHeader.data = extendedHdrBuf.data;
	ackMsg.extendedHeader.length = extendedHdrBuf.length;

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslAckMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslAckMsg*)rsslCopyMsg((RsslMsg *)&ackMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslAckMsg*)rsslCopyMsg((RsslMsg *)&ackMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_ACK);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslAckMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != ackMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(rsslAckMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslAckMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != ackMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(rsslAckMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != ackMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslAckMsg */
			ASSERT_TRUE(pCopiedMsg->ackId == ackId);
			ASSERT_TRUE(rsslAckMsgCheckHasNakCode(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->nakCode == nakCode);
			ASSERT_TRUE(rsslAckMsgCheckHasSeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->seqNum == seqNum);
			if (copyFlags & RSSL_CMF_NAK_TEXT)
			{
				ASSERT_TRUE(rsslAckMsgCheckHasText(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->text.data && pCopiedMsg->text.data != ackMsg.text.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->text, &textBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslAckMsgCheckHasText(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->text.data == 0);
			}
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslAckMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != ackMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslAckMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyCloseMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslCloseMsg closeMsg = RSSL_INIT_CLOSE_MSG;
	RsslBuffer encDataBuf, encMsgBuf, extendedHdrBuf;
	char encData[] = "encData";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	closeMsg.msgBase.msgClass = RSSL_MC_CLOSE;
	closeMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	closeMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	closeMsg.msgBase.streamId = streamId;
	closeMsg.msgBase.encDataBody.data = encDataBuf.data;
	closeMsg.msgBase.encDataBody.length = encDataBuf.length;
	closeMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	closeMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslCloseMsg */
	rsslCloseMsgSetHasExtendedHdr(&closeMsg);
	closeMsg.extendedHeader.data = extendedHdrBuf.data;
	closeMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslCloseMsgSetAck(&closeMsg);

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslCloseMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslCloseMsg*)rsslCopyMsg((RsslMsg *)&closeMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslCloseMsg*)rsslCopyMsg((RsslMsg *)&closeMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_CLOSE);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != closeMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslCloseMsg */
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslCloseMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != closeMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslCloseMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslCloseMsgCheckAck(pCopiedMsg));

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyPostMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt16 partNum = 33333;
	RsslUInt16 postUserRights = 0x0001;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, postUserAddr = 1234, postUserId = 5678;
	RsslUInt8 nameType = 255;
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, permDataBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	permDataBuf.data = permissionData;
	permDataBuf.length = permissionDataLen;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	postMsg.msgBase.msgClass = RSSL_MC_POST;
	postMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	postMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	postMsg.msgBase.streamId = streamId;
	rsslPostMsgApplyHasMsgKey(&postMsg);
	rsslMsgKeyApplyHasServiceId(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	postMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.identifier = identifier;
	postMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&postMsg.msgBase.msgKey);
	postMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	postMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	postMsg.msgBase.encDataBody.data = encDataBuf.data;
	postMsg.msgBase.encDataBody.length = encDataBuf.length;
	postMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	postMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslPostMsg */
	rsslPostMsgApplyHasPostUserRights(&postMsg);
	postMsg.postUserRights = postUserRights;
	rsslPostMsgApplyHasPartNum(&postMsg);
	postMsg.partNum = partNum;
	rsslPostMsgApplyHasSeqNum(&postMsg);
	postMsg.seqNum = seqNum;
	rsslPostMsgApplyHasPostId(&postMsg);
	postMsg.postId = postId;
	postMsg.postUserInfo.postUserAddr = postUserAddr;
	postMsg.postUserInfo.postUserId = postUserId;
	rsslPostMsgApplyHasPermData(&postMsg);
	postMsg.permData.data = permDataBuf.data;
	postMsg.permData.length = permDataBuf.length;
	rsslPostMsgApplyHasExtendedHdr(&postMsg);
	postMsg.extendedHeader.data = extendedHdrBuf.data;
	postMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslPostMsgApplyPostComplete(&postMsg);
	rsslPostMsgApplyAck(&postMsg);

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslPostMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslPostMsg*)rsslCopyMsg((RsslMsg *)&postMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslPostMsg*)rsslCopyMsg((RsslMsg *)&postMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_POST);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslPostMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != postMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(rsslPostMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslPostMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != postMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(rsslPostMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != postMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslPostMsg */
			ASSERT_TRUE(rsslPostMsgCheckHasPostUserRights(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->postUserRights == postUserRights);
			ASSERT_TRUE(rsslPostMsgCheckHasPartNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->partNum == partNum);
			ASSERT_TRUE(rsslPostMsgCheckHasSeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->seqNum == seqNum);
			ASSERT_TRUE(rsslPostMsgCheckHasPostId(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->postId == postId);
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserAddr == postUserAddr);
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserId == postUserId);
			if (copyFlags & RSSL_CMF_PERM_DATA)
			{
				ASSERT_TRUE(rsslPostMsgCheckHasPermData(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->permData.data && pCopiedMsg->permData.data != postMsg.permData.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->permData, &permDataBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslPostMsgCheckHasPermData(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->permData.data == 0);
			}
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslPostMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != postMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslPostMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslPostMsgCheckPostComplete(pCopiedMsg));
			ASSERT_TRUE(rsslPostMsgCheckAck(pCopiedMsg));

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyGenericMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt16 partNum = 33333;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, secondarySeqNum = 56789;
	RsslUInt8 nameType = 255;
	RsslGenericMsg genericMsg = RSSL_INIT_GENERIC_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, permDataBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	permDataBuf.data = permissionData;
	permDataBuf.length = permissionDataLen;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	genericMsg.msgBase.msgClass = RSSL_MC_GENERIC;
	genericMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	genericMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	genericMsg.msgBase.streamId = streamId;
	rsslGenericMsgApplyHasMsgKey(&genericMsg);
	rsslMsgKeyApplyHasServiceId(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	genericMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.identifier = identifier;
	genericMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&genericMsg.msgBase.msgKey);
	genericMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	genericMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	genericMsg.msgBase.encDataBody.data = encDataBuf.data;
	genericMsg.msgBase.encDataBody.length = encDataBuf.length;
	genericMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	genericMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslGenericMsg */
	rsslGenericMsgApplyHasPartNum(&genericMsg);
	genericMsg.partNum = partNum;
	rsslGenericMsgApplyHasSeqNum(&genericMsg);
	genericMsg.seqNum = seqNum;
	rsslGenericMsgApplyHasSecondarySeqNum(&genericMsg);
	genericMsg.secondarySeqNum = secondarySeqNum;
	rsslGenericMsgApplyHasPermData(&genericMsg);
	genericMsg.permData.data = permDataBuf.data;
	genericMsg.permData.length = permDataBuf.length;
	rsslGenericMsgApplyHasExtendedHdr(&genericMsg);
	genericMsg.extendedHeader.data = extendedHdrBuf.data;
	genericMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslGenericMsgApplyMessageComplete(&genericMsg);

	/* ReqMsgKey */
	rsslGenericMsgApplyHasReqMsgKey(&genericMsg);
	rsslMsgKeyApplyHasServiceId(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.serviceId = serviceId + 1;
	rsslMsgKeyApplyHasNameType(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.nameType = nameType - 1;
	rsslMsgKeyApplyHasName(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.name.data = keyNameBuf.data;
	genericMsg.reqMsgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.filter = filter + 1;
	rsslMsgKeyApplyHasIdentifier(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.identifier = identifier + 1;
	genericMsg.reqMsgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&genericMsg.reqMsgKey);
	genericMsg.reqMsgKey.encAttrib.data = keyAttribBuf.data;
	genericMsg.reqMsgKey.encAttrib.length = keyAttribBuf.length;

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslGenericMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslGenericMsg*)rsslCopyMsg((RsslMsg *)&genericMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslGenericMsg*)rsslCopyMsg((RsslMsg *)&genericMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_GENERIC);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslGenericMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != genericMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(rsslGenericMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslGenericMsgCheckHasMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != genericMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != genericMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslGenericMsg */
			ASSERT_TRUE(rsslGenericMsgCheckHasPartNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->partNum == partNum);
			ASSERT_TRUE(rsslGenericMsgCheckHasSeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->seqNum == seqNum);
			ASSERT_TRUE(rsslGenericMsgCheckHasSecondarySeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->secondarySeqNum == secondarySeqNum);
			if (copyFlags & RSSL_CMF_PERM_DATA)
			{
				ASSERT_TRUE(rsslGenericMsgCheckHasPermData(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->permData.data && pCopiedMsg->permData.data != genericMsg.permData.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->permData, &permDataBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslGenericMsgCheckHasPermData(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->permData.data == 0);
			}
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslGenericMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != genericMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslGenericMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslGenericMsgCheckMessageComplete(pCopiedMsg));

			/* ReqMsgKey */
			ASSERT_TRUE(rsslGenericMsgCheckHasReqMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.serviceId == serviceId + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.nameType == nameType - 1);
			if (copyFlags & RSSL_CMF_REQ_KEY_NAME)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data && pCopiedMsg->reqMsgKey.name.data != genericMsg.reqMsgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.filter == filter + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.identifier == identifier + 1);
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_REQ_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data && pCopiedMsg->reqMsgKey.encAttrib.data != genericMsg.reqMsgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data == 0);
			}

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyStatusMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, postUserAddr = 1234, postUserId = 5678;
	RsslUInt8 nameType = 255;
	RsslStatusMsg statusMsg = RSSL_INIT_STATUS_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, stateTextBuf, groupIdBuf, permDataBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	stateTextBuf.data = stateText;
	stateTextBuf.length = stateTextLen;
	groupIdBuf.data = groupId;
	groupIdBuf.length = groupIdLen;
	permDataBuf.data = permissionData;
	permDataBuf.length = permissionDataLen;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
	statusMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	statusMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	statusMsg.msgBase.streamId = streamId;
	rsslStatusMsgApplyHasMsgKey(&statusMsg);
	rsslMsgKeyApplyHasServiceId(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	statusMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.identifier = identifier;
	statusMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&statusMsg.msgBase.msgKey);
	statusMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	statusMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	statusMsg.msgBase.encDataBody.data = encDataBuf.data;
	statusMsg.msgBase.encDataBody.length = encDataBuf.length;
	statusMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	statusMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslStatusMsg */
	rsslStatusMsgApplyHasState(&statusMsg);
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_SUSPECT;
	statusMsg.state.code = RSSL_SC_NO_RESOURCES;
	statusMsg.state.text.data = stateTextBuf.data;
	statusMsg.state.text.length = stateTextBuf.length;
	rsslStatusMsgApplyHasGroupId(&statusMsg);
	statusMsg.groupId.data = groupIdBuf.data;
	statusMsg.groupId.length = groupIdBuf.length;
	rsslStatusMsgApplyHasPermData(&statusMsg);
	statusMsg.permData.data = permDataBuf.data;
	statusMsg.permData.length = permDataBuf.length;
	rsslStatusMsgApplyHasPostUserInfo(&statusMsg);
	statusMsg.postUserInfo.postUserAddr = postUserAddr;
	statusMsg.postUserInfo.postUserId = postUserId;
	rsslStatusMsgApplyHasExtendedHdr(&statusMsg);
	statusMsg.extendedHeader.data = extendedHdrBuf.data;
	statusMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslStatusMsgApplyClearCache(&statusMsg);

	/* ReqMsgKey */
	rsslStatusMsgApplyHasReqMsgKey(&statusMsg);
	rsslMsgKeyApplyHasServiceId(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.serviceId = serviceId + 1;
	rsslMsgKeyApplyHasNameType(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.nameType = nameType - 1;
	rsslMsgKeyApplyHasName(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.name.data = keyNameBuf.data;
	statusMsg.reqMsgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.filter = filter + 1;
	rsslMsgKeyApplyHasIdentifier(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.identifier = identifier + 1;
	statusMsg.reqMsgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&statusMsg.reqMsgKey);
	statusMsg.reqMsgKey.encAttrib.data = keyAttribBuf.data;
	statusMsg.reqMsgKey.encAttrib.length = keyAttribBuf.length;
	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslStatusMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslStatusMsg*)rsslCopyMsg((RsslMsg *)&statusMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslStatusMsg*)rsslCopyMsg((RsslMsg *)&statusMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_STATUS);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslStatusMsgCheckHasMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != statusMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != statusMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != statusMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslStatusMsg */
			ASSERT_TRUE(rsslStatusMsgCheckHasState(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pCopiedMsg->state.dataState == RSSL_DATA_SUSPECT);
			ASSERT_TRUE(pCopiedMsg->state.code == RSSL_SC_NO_RESOURCES);
			if (copyFlags & RSSL_CMF_STATE_TEXT)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->state.text.data && pCopiedMsg->state.text.data != statusMsg.state.text.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->state.text, &stateTextBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->state.text.data == 0);
			}
			if (copyFlags & RSSL_CMF_GROUP_ID)
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasGroupId(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->groupId.data && pCopiedMsg->groupId.data != statusMsg.groupId.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->groupId, &groupIdBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslStatusMsgCheckHasGroupId(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->groupId.data == 0);
			}
			if (copyFlags & RSSL_CMF_PERM_DATA)
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasPermData(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->permData.data && pCopiedMsg->permData.data != statusMsg.permData.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->permData, &permDataBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslStatusMsgCheckHasPermData(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->permData.data == 0);
			}
			ASSERT_TRUE(rsslStatusMsgCheckHasPostUserInfo(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserAddr == postUserAddr);
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserId == postUserId);
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != statusMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslStatusMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslStatusMsgCheckClearCache(pCopiedMsg));

			/* ReqMsgKey */
			ASSERT_TRUE(rsslStatusMsgCheckHasReqMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.serviceId == serviceId + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.nameType == nameType - 1);
			if (copyFlags & RSSL_CMF_REQ_KEY_NAME)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data && pCopiedMsg->reqMsgKey.name.data != statusMsg.reqMsgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.filter == filter + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.identifier == identifier + 1);
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_REQ_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data && pCopiedMsg->reqMsgKey.encAttrib.data != statusMsg.reqMsgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data == 0);
			}

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyRefreshMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt16 partNum = 33333;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, postUserAddr = 1234, postUserId = 5678;
	RsslUInt8 nameType = 255;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, stateTextBuf, groupIdBuf, permDataBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	stateTextBuf.data = stateText;
	stateTextBuf.length = stateTextLen;
	groupIdBuf.data = groupId;
	groupIdBuf.length = groupIdLen;
	permDataBuf.data = permissionData;
	permDataBuf.length = permissionDataLen;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	refreshMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	refreshMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	refreshMsg.msgBase.streamId = streamId;
	rsslRefreshMsgApplyHasMsgKey(&refreshMsg);
	rsslMsgKeyApplyHasServiceId(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	refreshMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.identifier = identifier;
	refreshMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&refreshMsg.msgBase.msgKey);
	refreshMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	refreshMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	refreshMsg.msgBase.encDataBody.data = encDataBuf.data;
	refreshMsg.msgBase.encDataBody.length = encDataBuf.length;
	refreshMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	refreshMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslRefreshMsg */
	rsslRefreshMsgApplyHasPartNum(&refreshMsg);
	refreshMsg.partNum = partNum;
	rsslRefreshMsgApplyHasSeqNum(&refreshMsg);
	refreshMsg.seqNum = seqNum;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_SUSPECT;
	refreshMsg.state.code = RSSL_SC_NO_RESOURCES;
	refreshMsg.state.text.data = stateTextBuf.data;
	refreshMsg.state.text.length = stateTextBuf.length;
	refreshMsg.groupId.data = groupIdBuf.data;
	refreshMsg.groupId.length = groupIdBuf.length;
	rsslRefreshMsgApplyHasPermData(&refreshMsg);
	refreshMsg.permData.data = permDataBuf.data;
	refreshMsg.permData.length = permDataBuf.length;
	rsslRefreshMsgApplyHasPostUserInfo(&refreshMsg);
	refreshMsg.postUserInfo.postUserAddr = postUserAddr;
	refreshMsg.postUserInfo.postUserId = postUserId;
	rsslRefreshMsgApplyHasQoS(&refreshMsg);
	refreshMsg.qos.dynamic = RSSL_TRUE;
	refreshMsg.qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
	refreshMsg.qos.rateInfo = 65535;
	refreshMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED;
	refreshMsg.qos.timeInfo = 65534;
	rsslRefreshMsgApplyHasExtendedHdr(&refreshMsg);
	refreshMsg.extendedHeader.data = extendedHdrBuf.data;
	refreshMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslRefreshMsgApplySolicited(&refreshMsg);
	rsslRefreshMsgApplyRefreshComplete(&refreshMsg);
	rsslRefreshMsgApplyClearCache(&refreshMsg);
	rsslRefreshMsgApplyDoNotCache(&refreshMsg);

	/* ReqMsgKey */
	rsslRefreshMsgApplyHasReqMsgKey(&refreshMsg);
	rsslMsgKeyApplyHasServiceId(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.serviceId = serviceId + 1;
	rsslMsgKeyApplyHasNameType(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.nameType = nameType - 1;
	rsslMsgKeyApplyHasName(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.name.data = keyNameBuf.data;
	refreshMsg.reqMsgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.filter = filter + 1;
	rsslMsgKeyApplyHasIdentifier(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.identifier = identifier + 1;
	refreshMsg.reqMsgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&refreshMsg.reqMsgKey);
	refreshMsg.reqMsgKey.encAttrib.data = keyAttribBuf.data;
	refreshMsg.reqMsgKey.encAttrib.length = keyAttribBuf.length;
	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslRefreshMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslRefreshMsg*)rsslCopyMsg((RsslMsg *)&refreshMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslRefreshMsg*)rsslCopyMsg((RsslMsg *)&refreshMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_REFRESH);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslRefreshMsgCheckHasMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != refreshMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslRefreshMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != refreshMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(rsslRefreshMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != refreshMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslRefreshMsg */
			ASSERT_TRUE(rsslRefreshMsgCheckHasPartNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->partNum == partNum);
			ASSERT_TRUE(rsslRefreshMsgCheckHasSeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->seqNum == seqNum);
			ASSERT_TRUE(pCopiedMsg->state.streamState == RSSL_STREAM_OPEN);
			ASSERT_TRUE(pCopiedMsg->state.dataState == RSSL_DATA_SUSPECT);
			ASSERT_TRUE(pCopiedMsg->state.code == RSSL_SC_NO_RESOURCES);
			if (copyFlags & RSSL_CMF_STATE_TEXT)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->state.text.data && pCopiedMsg->state.text.data != refreshMsg.state.text.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->state.text, &stateTextBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->state.text.data == 0);
			}
			if (copyFlags & RSSL_CMF_GROUP_ID)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->groupId.data && pCopiedMsg->groupId.data != refreshMsg.groupId.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->groupId, &groupIdBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->groupId.data == 0);
			}
			if (copyFlags & RSSL_CMF_PERM_DATA)
			{
				ASSERT_TRUE(rsslRefreshMsgCheckHasPermData(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->permData.data && pCopiedMsg->permData.data != refreshMsg.permData.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->permData, &permDataBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslRefreshMsgCheckHasPermData(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->permData.data == 0);
			}
			ASSERT_TRUE(rsslRefreshMsgCheckHasPostUserInfo(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserAddr == postUserAddr);
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserId == postUserId);
			ASSERT_TRUE(rsslRefreshMsgCheckHasQoS(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->qos.dynamic == RSSL_TRUE);
			ASSERT_TRUE(pCopiedMsg->qos.rate == RSSL_QOS_RATE_TIME_CONFLATED);
			ASSERT_TRUE(pCopiedMsg->qos.rateInfo == 65535);
			ASSERT_TRUE(pCopiedMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED);
			ASSERT_TRUE(pCopiedMsg->qos.timeInfo == 65534);
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslRefreshMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != refreshMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslRefreshMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslRefreshMsgCheckSolicited(pCopiedMsg));
			ASSERT_TRUE(rsslRefreshMsgCheckRefreshComplete(pCopiedMsg));
			ASSERT_TRUE(rsslRefreshMsgCheckClearCache(pCopiedMsg));
			ASSERT_TRUE(rsslRefreshMsgCheckDoNotCache(pCopiedMsg));

			/* ReqMsgKey */
			ASSERT_TRUE(rsslRefreshMsgCheckHasReqMsgKey(pCopiedMsg));
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.serviceId == serviceId + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.nameType == nameType - 1);
			if (copyFlags & RSSL_CMF_REQ_KEY_NAME)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data && pCopiedMsg->reqMsgKey.name.data != refreshMsg.reqMsgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.filter == filter + 1);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->reqMsgKey));
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.identifier == identifier + 1);
			ASSERT_TRUE(pCopiedMsg->reqMsgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_REQ_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data && pCopiedMsg->reqMsgKey.encAttrib.data != refreshMsg.reqMsgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->reqMsgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->reqMsgKey));
				ASSERT_TRUE(pCopiedMsg->reqMsgKey.encAttrib.data == 0);
			}

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyRequestMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslRequestMsg requestMsg = RSSL_INIT_REQUEST_MSG;
	RsslBuffer encDataBuf, encMsgBuf, extendedHdrBuf;
	char encData[] = "encData";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	requestMsg.msgBase.msgClass = RSSL_MC_REQUEST;
	requestMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	requestMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	requestMsg.msgBase.streamId = streamId;
	requestMsg.msgBase.encDataBody.data = encDataBuf.data;
	requestMsg.msgBase.encDataBody.length = encDataBuf.length;
	requestMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	requestMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslRequestMsg */
	rsslRequestMsgApplyHasPriority(&requestMsg);
	requestMsg.priorityClass = 3;
	requestMsg.priorityCount = 4;
	rsslRequestMsgApplyHasQos(&requestMsg);
	requestMsg.qos.dynamic = RSSL_TRUE;
	requestMsg.qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
	requestMsg.qos.rateInfo = 65535;
	requestMsg.qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	requestMsg.qos.timeInfo = 65534;
	rsslRequestMsgApplyHasWorstQos(&requestMsg);
	requestMsg.worstQos.dynamic = RSSL_TRUE;
	requestMsg.worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
	requestMsg.worstQos.rateInfo = 65534;
	requestMsg.worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
	requestMsg.worstQos.timeInfo = 65533;
	rsslRequestMsgApplyHasExtendedHdr(&requestMsg);
	requestMsg.extendedHeader.data = extendedHdrBuf.data;
	requestMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslRequestMsgApplyStreaming(&requestMsg);
	rsslRequestMsgApplyMsgKeyInUpdates(&requestMsg);
	rsslRequestMsgApplyConfInfoInUpdates(&requestMsg);
	rsslRequestMsgApplyNoRefresh(&requestMsg);
	rsslRequestMsgApplyPause(&requestMsg);
	rsslRequestMsgApplyHasView(&requestMsg);
	rsslRequestMsgApplyHasBatch(&requestMsg);

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslRequestMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslRequestMsg*)rsslCopyMsg((RsslMsg *)&requestMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslRequestMsg*)rsslCopyMsg((RsslMsg *)&requestMsg, copyFlags, 0, NULL)) != NULL);
			}


			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_REQUEST);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != requestMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslRequestMsg */
			ASSERT_TRUE(rsslRequestMsgCheckHasPriority(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->priorityClass == 3);
			ASSERT_TRUE(pCopiedMsg->priorityCount == 4);
			ASSERT_TRUE(rsslRequestMsgCheckHasQoS(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->qos.dynamic == RSSL_TRUE);
			ASSERT_TRUE(pCopiedMsg->qos.rate == RSSL_QOS_RATE_JIT_CONFLATED);
			ASSERT_TRUE(pCopiedMsg->qos.rateInfo == 65535);
			ASSERT_TRUE(pCopiedMsg->qos.timeliness == RSSL_QOS_TIME_DELAYED_UNKNOWN);
			ASSERT_TRUE(pCopiedMsg->qos.timeInfo == 65534);
			ASSERT_TRUE(rsslRequestMsgCheckHasWorstQoS(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->worstQos.dynamic == RSSL_TRUE);
			ASSERT_TRUE(pCopiedMsg->worstQos.rate == RSSL_QOS_RATE_TIME_CONFLATED);
			ASSERT_TRUE(pCopiedMsg->worstQos.rateInfo == 65534);
			ASSERT_TRUE(pCopiedMsg->worstQos.timeliness == RSSL_QOS_TIME_DELAYED);
			ASSERT_TRUE(pCopiedMsg->worstQos.timeInfo == 65533);
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslRequestMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != requestMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslRequestMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslRequestMsgCheckStreaming(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckMsgKeyInUpdates(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckConfInfoInUpdates(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckNoRefresh(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckPause(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckHasView(pCopiedMsg));
			ASSERT_TRUE(rsslRequestMsgCheckHasBatch(pCopiedMsg));

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyUpdateMsgTest()
{
	int copyFlags;
	int inlineCopy;
	RsslUInt8 nameType = 255, updateType = 2;
	RsslUInt16 conflationCount = 3, conflationTime = 4;
	RsslUInt32 serviceId = 9876, filter = 123, identifier = 456, postUserAddr = 1234, postUserId = 5678;
	RsslUpdateMsg updateMsg = RSSL_INIT_UPDATE_MSG;
	RsslBuffer encDataBuf, encMsgBuf, keyNameBuf, keyAttribBuf, permDataBuf, extendedHdrBuf;
	char encData[] = "encData", keyName[] = "keyName", keyAttrib[] = "keyAttrib";

	encDataBuf.data = encData;
	encDataBuf.length = sizeof(encData);
	encMsgBuf.data = 0;
	encMsgBuf.length = 0;
	keyNameBuf.data = keyName;
	keyNameBuf.length = sizeof(keyName);
	keyAttribBuf.data = keyAttrib;
	keyAttribBuf.length = sizeof(keyAttrib);
	permDataBuf.data = permissionData;
	permDataBuf.length = permissionDataLen;
	extendedHdrBuf.data = extendedHeader;
	extendedHdrBuf.length = sizeof(extendedHeader);

	/* RsslMsgBase */
	updateMsg.msgBase.msgClass = RSSL_MC_UPDATE;
	updateMsg.msgBase.domainType = RSSL_DMT_TRANSACTION;
	updateMsg.msgBase.containerType = RSSL_DT_OPAQUE;
	updateMsg.msgBase.streamId = streamId;
	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = serviceId;
	rsslMsgKeyApplyHasNameType(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.nameType = nameType;
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name.data = keyNameBuf.data;
	updateMsg.msgBase.msgKey.name.length = keyNameBuf.length;
	rsslMsgKeyApplyHasFilter(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.filter = filter;
	rsslMsgKeyApplyHasIdentifier(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.identifier = identifier;
	updateMsg.msgBase.msgKey.attribContainerType = RSSL_DT_OPAQUE;
	rsslMsgKeyApplyHasAttrib(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.encAttrib.data = keyAttribBuf.data;
	updateMsg.msgBase.msgKey.encAttrib.length = keyAttribBuf.length;
	updateMsg.msgBase.encDataBody.data = encDataBuf.data;
	updateMsg.msgBase.encDataBody.length = encDataBuf.length;
	updateMsg.msgBase.encMsgBuffer.data = encMsgBuf.data;
	updateMsg.msgBase.encMsgBuffer.length = encMsgBuf.length;
	/* RsslUpdateMsg */
	updateMsg.updateType = updateType;
	rsslUpdateMsgApplyHasSeqNum(&updateMsg);
	updateMsg.seqNum = seqNum;
	rsslUpdateMsgApplyHasConfInfo(&updateMsg);
	updateMsg.conflationCount = conflationCount;
	updateMsg.conflationTime = conflationTime;
	rsslUpdateMsgApplyHasPermData(&updateMsg);
	updateMsg.permData.data = permDataBuf.data;
	updateMsg.permData.length = permDataBuf.length;
	rsslUpdateMsgApplyHasPostUserInfo(&updateMsg);
	updateMsg.postUserInfo.postUserAddr = postUserAddr;
	updateMsg.postUserInfo.postUserId = postUserId;
	rsslUpdateMsgApplyHasExtendedHdr(&updateMsg);
	updateMsg.extendedHeader.data = extendedHdrBuf.data;
	updateMsg.extendedHeader.length = extendedHdrBuf.length;
	rsslUpdateMsgApplyDoNotCache(&updateMsg);
	rsslUpdateMsgApplyDoNotConflate(&updateMsg);
	rsslUpdateMsgApplyDoNotRipple(&updateMsg);

	
	for (inlineCopy = 0; inlineCopy < 2; ++inlineCopy)
	{
		for (copyFlags = 0; copyFlags < 0x1000; copyFlags++)
		{
			RsslUpdateMsg *pCopiedMsg;

			if (inlineCopy)
			{
				/* Inline copy into existing buffer.*/
				copyMsgBuf.length = c_TestMsgCopyBufferSize; /* reset buffer size */
				memset(copyMsgBuf.data, 0, copyMsgBuf.length);
				ASSERT_TRUE((pCopiedMsg = (RsslUpdateMsg*)rsslCopyMsg((RsslMsg *)&updateMsg, copyFlags, 0, &copyMsgBuf)) != NULL);
			}
			else
			{
				/* Copy allocated by API. */
				ASSERT_TRUE((pCopiedMsg = (RsslUpdateMsg*)rsslCopyMsg((RsslMsg *)&updateMsg, copyFlags, 0, NULL)) != NULL);
			}

			/* RsslMsgBase */
			ASSERT_TRUE(pCopiedMsg->msgBase.msgClass == RSSL_MC_UPDATE);
			ASSERT_TRUE(pCopiedMsg->msgBase.domainType == RSSL_DMT_TRANSACTION);
			ASSERT_TRUE(pCopiedMsg->msgBase.containerType == RSSL_DT_OPAQUE);
			ASSERT_TRUE(pCopiedMsg->msgBase.streamId == streamId);
			ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.serviceId == serviceId);
			ASSERT_TRUE(rsslMsgKeyCheckHasNameType(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.nameType == nameType);
			if (copyFlags & RSSL_CMF_KEY_NAME)
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data && pCopiedMsg->msgBase.msgKey.name.data != updateMsg.msgBase.msgKey.name.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.name, &keyNameBuf));
			}
			else
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasName(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.name.data == 0);
			}
			ASSERT_TRUE(rsslMsgKeyCheckHasFilter(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.filter == filter);
			ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&pCopiedMsg->msgBase.msgKey));
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.identifier == identifier);
			ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.attribContainerType == RSSL_DT_OPAQUE);
			if (copyFlags & RSSL_CMF_KEY_ATTRIB)
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data && pCopiedMsg->msgBase.msgKey.encAttrib.data != updateMsg.msgBase.msgKey.encAttrib.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.msgKey.encAttrib, &keyAttribBuf));
			}
			else
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasMsgKey(pCopiedMsg));
				ASSERT_TRUE(!rsslMsgKeyCheckHasAttrib(&pCopiedMsg->msgBase.msgKey));
				ASSERT_TRUE(pCopiedMsg->msgBase.msgKey.encAttrib.data == 0);
			}
			if (copyFlags & RSSL_CMF_DATA_BODY)
			{
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data && pCopiedMsg->msgBase.encDataBody.data != updateMsg.msgBase.encDataBody.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->msgBase.encDataBody, &encDataBuf));
			}
			else
			{
				ASSERT_TRUE(pCopiedMsg->msgBase.encDataBody.data == 0);
			}
			ASSERT_TRUE(pCopiedMsg->msgBase.encMsgBuffer.data == 0);
			/* RsslUpdateMsg */
			ASSERT_TRUE(pCopiedMsg->updateType == updateType);
			ASSERT_TRUE(rsslUpdateMsgCheckHasSeqNum(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->seqNum == seqNum);
			ASSERT_TRUE(rsslUpdateMsgCheckHasConfInfo(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->conflationCount == conflationCount);
			ASSERT_TRUE(pCopiedMsg->conflationTime == conflationTime);
			if (copyFlags & RSSL_CMF_PERM_DATA)
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasPermData(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->permData.data && pCopiedMsg->permData.data != updateMsg.permData.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->permData, &permDataBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslUpdateMsgCheckHasPermData(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->permData.data == 0);
			}
			ASSERT_TRUE(rsslUpdateMsgCheckHasPostUserInfo(pCopiedMsg));
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserAddr == postUserAddr);
			ASSERT_TRUE(pCopiedMsg->postUserInfo.postUserId == postUserId);
			if (copyFlags & RSSL_CMF_EXTENDED_HEADER)
			{
				ASSERT_TRUE(rsslUpdateMsgCheckHasExtendedHdr(pCopiedMsg));
				/* Make sure all buffer copies are deep copies -- pointers should not match our given buffers */
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data && pCopiedMsg->extendedHeader.data != updateMsg.extendedHeader.data);
				ASSERT_TRUE(rsslBufferIsEqual(&pCopiedMsg->extendedHeader, &extendedHdrBuf));
			}
			else
			{
				ASSERT_TRUE(!rsslUpdateMsgCheckHasExtendedHdr(pCopiedMsg));
				ASSERT_TRUE(pCopiedMsg->extendedHeader.data == 0);
			}
			ASSERT_TRUE(rsslUpdateMsgCheckDoNotCache(pCopiedMsg));
			ASSERT_TRUE(rsslUpdateMsgCheckDoNotConflate(pCopiedMsg));
			ASSERT_TRUE(rsslUpdateMsgCheckDoNotRipple(pCopiedMsg));

			if (!inlineCopy)
			{
				/* Copy allocated by API. Free it. */
				rsslReleaseCopiedMsg((RsslMsg*)pCopiedMsg);
			}
		}
	}
}

void copyMsgTest()
{
	//printf("rsslCopyMsg Tests:\n");
	startTimer();

	copyAckMsgTest();
	copyCloseMsgTest();
	copyPostMsgTest();
	copyGenericMsgTest();
	copyStatusMsgTest();
	copyRefreshMsgTest();
	copyRequestMsgTest();
	copyUpdateMsgTest();

	endTimerAndPrint();
	//printf("\n");
}

void rsslArrayClear(RsslArray *pArray)
{
	pArray->primitiveType = 0;
	pArray->itemLength = 0;
	pArray->encData.length = 0;
	pArray->encData.data = 0;
	
}

void arrayClearMemSetTest()
{
	RsslArray rsslArray[4];
	RsslArray dirtyArray;
	int i;

	rsslArray[0].itemLength = 1;
	rsslArray[0].primitiveType = RSSL_DT_INT;
	rsslArray[0].encData.data = (char*)1;
	rsslArray[0].encData.length = 2;

	rsslArray[1].itemLength = 3;
	rsslArray[1].primitiveType = RSSL_DT_FLOAT;
	rsslArray[1].encData.data = (char*)2;
	rsslArray[1].encData.length = 4;

	rsslArray[2].itemLength = 7;
	rsslArray[2].primitiveType = RSSL_DT_DOUBLE;
	rsslArray[2].encData.data = (char*)3;
	rsslArray[2].encData.length = 8;

	rsslArray[3].itemLength = 15;
	rsslArray[3].primitiveType = RSSL_DT_REAL;
	rsslArray[3].encData.data = (char*)4;
	rsslArray[3].encData.length = 32;

	//Test setting individual values with rsslArrayClear
	startTimer();
	for(i = 0; i < 100000000; i++)
	{
		dirtyArray.itemLength = rsslArray[i % 4].itemLength;
		dirtyArray.primitiveType = rsslArray[i % 4].primitiveType;
		dirtyArray.encData.data = rsslArray[i % 4].encData.data;
		dirtyArray.encData.length = rsslArray[i % 4].encData.length;
		rsslArrayClear(&dirtyArray);
	}
	//printf("\tArray Clear Test:");
	endTimerAndPrint();

	//Test using memset 
	startTimer();
	for(i = 0; i < 100000000; i++)
	{
		dirtyArray.itemLength = rsslArray[i % 4].itemLength;
		dirtyArray.primitiveType = rsslArray[i % 4].primitiveType;
		dirtyArray.encData.data = rsslArray[i % 4].encData.data;
		dirtyArray.encData.length = rsslArray[i % 4].encData.length;
		memset(&dirtyArray, 0, sizeof(RsslArray));
	}
	//printf("\tArray Memset Test:");
	endTimerAndPrint();
}

void rsslVectorClear(RsslVector *pVector)
{
	pVector->flags = 0;
	pVector->containerType = 0;
	pVector->encEntries.length = 0;
	pVector->encEntries.data = 0;
	pVector->encSetDefs.length = 0;
	pVector->encSetDefs.data = 0;
	pVector->encSummaryData.length = 0;
	pVector->encSummaryData.data = 0;
	pVector->totalCountHint = 0;

}

void vectorClearMemSetTest()
{
	RsslVector rsslVector[4];
	RsslVector dirtyVector;
	int i;

	rsslVector[0].containerType = RSSL_DT_INT;
	rsslVector[0].flags = 0x00;
	rsslVector[0].totalCountHint = 1;
	rsslVector[0].encEntries.length = 1;
	rsslVector[0].encEntries.data = (char*)1;
	rsslVector[0].encSetDefs.length = 1;
	rsslVector[0].encSetDefs.data = (char*)1;
	rsslVector[0].encSummaryData.length = 1;
	rsslVector[0].encSummaryData.data = (char*)1;


	rsslVector[1].containerType = RSSL_DT_DOUBLE;
	rsslVector[1].flags = 0x01;
	rsslVector[1].totalCountHint = 3;
	rsslVector[1].encEntries.length = 2;
	rsslVector[1].encEntries.data = (char*)2;
	rsslVector[1].encSetDefs.length = 2;
	rsslVector[1].encSetDefs.data = (char*)2;
	rsslVector[1].encSummaryData.length = 2;
	rsslVector[1].encSummaryData.data = (char*)2;

	rsslVector[2].containerType = RSSL_DT_FLOAT;
	rsslVector[2].flags = 0x02;
	rsslVector[2].totalCountHint = 7;
	rsslVector[2].encEntries.length = 3;
	rsslVector[2].encEntries.data = (char*)3;
	rsslVector[2].encSetDefs.length = 3;
	rsslVector[2].encSetDefs.data = (char*)3;
	rsslVector[2].encSummaryData.length = 3;
	rsslVector[2].encSummaryData.data = (char*)3;

	rsslVector[3].containerType = RSSL_DT_REAL;
	rsslVector[3].flags = 0x03;
	rsslVector[3].totalCountHint = 15;
	rsslVector[3].encEntries.length = 4;
	rsslVector[3].encEntries.data = (char*)4;
	rsslVector[3].encSetDefs.length = 4;
	rsslVector[3].encSetDefs.data = (char*)4;
	rsslVector[3].encSummaryData.length = 4;
	rsslVector[3].encSummaryData.data = (char*)4;

	//Test setting individual values with rsslVectorClear
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyVector.containerType = rsslVector[i % 4].containerType;
		dirtyVector.flags= rsslVector[i % 4].flags;
		dirtyVector.totalCountHint = rsslVector[i % 4].totalCountHint;
		dirtyVector.encEntries.length = rsslVector[i % 4].encEntries.length;
		dirtyVector.encEntries.data = rsslVector[i % 4].encEntries.data;
		dirtyVector.encSetDefs.length = rsslVector[i % 4].encSetDefs.length;
		dirtyVector.encSetDefs.data = rsslVector[i % 4].encSetDefs.data;
		dirtyVector.encSummaryData.length = rsslVector[i % 4].encSummaryData.length;
		dirtyVector.encSummaryData.data = rsslVector[i % 4].encSummaryData.data;
		
		rsslVectorClear(&dirtyVector);
	}
	//printf("\tVector Clear Test:");
	endTimerAndPrint();
	
	//Test using memset
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyVector.containerType = rsslVector[i % 4].containerType;
		dirtyVector.flags= rsslVector[i % 4].flags;
		dirtyVector.totalCountHint = rsslVector[i % 4].totalCountHint;
		dirtyVector.encEntries.length = rsslVector[i % 4].encEntries.length;
		dirtyVector.encEntries.data = rsslVector[i % 4].encEntries.data;
		dirtyVector.encSetDefs.length = rsslVector[i % 4].encSetDefs.length;
		dirtyVector.encSetDefs.data = rsslVector[i % 4].encSetDefs.data;
		dirtyVector.encSummaryData.length = rsslVector[i % 4].encSummaryData.length;
		dirtyVector.encSummaryData.data = rsslVector[i % 4].encSummaryData.data;
		
		memset(&dirtyVector, 0, sizeof(RsslVector));
	}
	//printf("\tVector Memset Test:");
	endTimerAndPrint();
}

void rsslAckMsgClear(RsslAckMsg *pAckMsg)
{
	pAckMsg->msgBase.msgClass = 0;
	pAckMsg->msgBase.domainType = 0;
	pAckMsg->msgBase.containerType = 0;
	pAckMsg->msgBase.streamId = 0;
	pAckMsg->msgBase.msgKey.flags = 0;
	pAckMsg->msgBase.msgKey.serviceId = 0;
	pAckMsg->msgBase.msgKey.nameType = 0;
	pAckMsg->msgBase.msgKey.name.length = 0;
	pAckMsg->msgBase.msgKey.name.data = 0;
	pAckMsg->msgBase.msgKey.filter = 0;
	pAckMsg->msgBase.msgKey.identifier = 0;
	pAckMsg->msgBase.msgKey.attribContainerType = 0;
	pAckMsg->msgBase.msgKey.encAttrib.length = 0;
	pAckMsg->msgBase.msgKey.encAttrib.data = 0;
	pAckMsg->msgBase.encDataBody.length = 0;
	pAckMsg->msgBase.encDataBody.data = 0;
	pAckMsg->msgBase.encMsgBuffer.length = 0;
	pAckMsg->msgBase.encMsgBuffer.data = 0;
	pAckMsg->flags = 0;
	pAckMsg->ackId = 0;
	pAckMsg->nakCode = 0;
	pAckMsg->seqNum = 0;
	pAckMsg->text.length = 0;
	pAckMsg->text.data = 0;
	pAckMsg->extendedHeader.length = 0;
	pAckMsg->extendedHeader.data = 0;
}

void ackMsgClearMemSetTest()
{
	RsslAckMsg rsslAckMsg[4];
	RsslAckMsg dirtyAckMsg;
	int i;

	rsslAckMsg[0].msgBase.msgClass = 1;
	rsslAckMsg[0].msgBase.domainType = 1;
	rsslAckMsg[0].msgBase.containerType = 1;
	rsslAckMsg[0].msgBase.streamId = 1;
	rsslAckMsg[0].msgBase.msgKey.flags = 1;
	rsslAckMsg[0].msgBase.msgKey.serviceId = 1;
	rsslAckMsg[0].msgBase.msgKey.nameType = 1;
	rsslAckMsg[0].msgBase.msgKey.name.length = 1;
	rsslAckMsg[0].msgBase.msgKey.name.data = (char*)1;
	rsslAckMsg[0].msgBase.msgKey.filter = 1;
	rsslAckMsg[0].msgBase.msgKey.identifier = 1;
	rsslAckMsg[0].msgBase.msgKey.attribContainerType = 1;
	rsslAckMsg[0].msgBase.msgKey.encAttrib.length = 1;
	rsslAckMsg[0].msgBase.msgKey.encAttrib.data = (char*)1;
	rsslAckMsg[0].msgBase.encDataBody.length = 1;
	rsslAckMsg[0].msgBase.encDataBody.data = (char*)1;
	rsslAckMsg[0].msgBase.encMsgBuffer.length = 1;
	rsslAckMsg[0].msgBase.encMsgBuffer.data = (char*)1;
	rsslAckMsg[0].flags = 1;
	rsslAckMsg[0].ackId = 1;
	rsslAckMsg[0].nakCode = 1;
	rsslAckMsg[0].seqNum = 1;
	rsslAckMsg[0].text.length = 1;
	rsslAckMsg[0].text.data = (char*)1;
	rsslAckMsg[0].extendedHeader.length = 1;
	rsslAckMsg[0].extendedHeader.data = (char*)1;

	rsslAckMsg[1].msgBase.msgClass = 2;
	rsslAckMsg[1].msgBase.domainType = 2;
	rsslAckMsg[1].msgBase.containerType = 2;
	rsslAckMsg[1].msgBase.streamId = 2;
	rsslAckMsg[1].msgBase.msgKey.flags = 2;
	rsslAckMsg[1].msgBase.msgKey.serviceId = 2;
	rsslAckMsg[1].msgBase.msgKey.nameType = 2;
	rsslAckMsg[1].msgBase.msgKey.name.length = 2;
	rsslAckMsg[1].msgBase.msgKey.name.data = (char*)2;
	rsslAckMsg[1].msgBase.msgKey.filter = 2;
	rsslAckMsg[1].msgBase.msgKey.identifier = 2;
	rsslAckMsg[1].msgBase.msgKey.attribContainerType = 2;
	rsslAckMsg[1].msgBase.msgKey.encAttrib.length = 2;
	rsslAckMsg[1].msgBase.msgKey.encAttrib.data = (char*)2;
	rsslAckMsg[1].msgBase.encDataBody.length = 2;
	rsslAckMsg[1].msgBase.encDataBody.data = (char*)2;
	rsslAckMsg[1].msgBase.encMsgBuffer.length = 2;
	rsslAckMsg[1].msgBase.encMsgBuffer.data = (char*)2;
	rsslAckMsg[1].flags = 2;
	rsslAckMsg[1].ackId = 2;
	rsslAckMsg[1].nakCode = 2;
	rsslAckMsg[1].seqNum = 2;
	rsslAckMsg[1].text.length = 2;
	rsslAckMsg[1].text.data = (char*)2;
	rsslAckMsg[1].extendedHeader.length = 2;
	rsslAckMsg[1].extendedHeader.data = (char*)2;

	rsslAckMsg[2].msgBase.msgClass = 3;
	rsslAckMsg[2].msgBase.domainType = 3;
	rsslAckMsg[2].msgBase.containerType = 3;
	rsslAckMsg[2].msgBase.streamId = 3;
	rsslAckMsg[2].msgBase.msgKey.flags = 3;
	rsslAckMsg[2].msgBase.msgKey.serviceId = 3;
	rsslAckMsg[2].msgBase.msgKey.nameType = 3;
	rsslAckMsg[2].msgBase.msgKey.name.length = 3;
	rsslAckMsg[2].msgBase.msgKey.name.data = (char*)3;
	rsslAckMsg[2].msgBase.msgKey.filter = 3;
	rsslAckMsg[2].msgBase.msgKey.identifier = 3;
	rsslAckMsg[2].msgBase.msgKey.attribContainerType = 3;
	rsslAckMsg[2].msgBase.msgKey.encAttrib.length = 3;
	rsslAckMsg[2].msgBase.msgKey.encAttrib.data = (char*)3;
	rsslAckMsg[2].msgBase.encDataBody.length = 3;
	rsslAckMsg[2].msgBase.encDataBody.data = (char*)3;
	rsslAckMsg[2].msgBase.encMsgBuffer.length = 3;
	rsslAckMsg[2].msgBase.encMsgBuffer.data = (char*)3;
	rsslAckMsg[2].flags = 3;
	rsslAckMsg[2].ackId = 3;
	rsslAckMsg[2].nakCode = 3;
	rsslAckMsg[2].seqNum = 3;
	rsslAckMsg[2].text.length = 3;
	rsslAckMsg[2].text.data = (char*)3;
	rsslAckMsg[2].extendedHeader.length = 3;
	rsslAckMsg[2].extendedHeader.data = (char*)3;

	rsslAckMsg[3].msgBase.msgClass = 4;
	rsslAckMsg[3].msgBase.domainType = 4;
	rsslAckMsg[3].msgBase.containerType = 4;
	rsslAckMsg[3].msgBase.streamId = 4;
	rsslAckMsg[3].msgBase.msgKey.flags = 4;
	rsslAckMsg[3].msgBase.msgKey.serviceId = 4;
	rsslAckMsg[3].msgBase.msgKey.nameType = 4;
	rsslAckMsg[3].msgBase.msgKey.name.length = 4;
	rsslAckMsg[3].msgBase.msgKey.name.data = (char*)4;
	rsslAckMsg[3].msgBase.msgKey.filter = 4;
	rsslAckMsg[3].msgBase.msgKey.identifier = 4;
	rsslAckMsg[3].msgBase.msgKey.attribContainerType = 4;
	rsslAckMsg[3].msgBase.msgKey.encAttrib.length = 4;
	rsslAckMsg[3].msgBase.msgKey.encAttrib.data = (char*)4;
	rsslAckMsg[3].msgBase.encDataBody.length = 4;
	rsslAckMsg[3].msgBase.encDataBody.data = (char*)4;
	rsslAckMsg[3].msgBase.encMsgBuffer.length = 4;
	rsslAckMsg[3].msgBase.encMsgBuffer.data = (char*)4;
	rsslAckMsg[3].flags = 4;
	rsslAckMsg[3].ackId = 4;
	rsslAckMsg[3].nakCode = 4;
	rsslAckMsg[3].seqNum = 4;
	rsslAckMsg[3].text.length = 4;
	rsslAckMsg[3].text.data = (char*)4;
	rsslAckMsg[3].extendedHeader.length = 4;
	rsslAckMsg[3].extendedHeader.data = (char*)4;

	//Test setting individual values with rsslAckMsgClear
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyAckMsg.msgBase.msgClass = rsslAckMsg[i % 4].msgBase.msgClass;
		dirtyAckMsg.msgBase.domainType = rsslAckMsg[i % 4].msgBase.domainType;
		dirtyAckMsg.msgBase.containerType = rsslAckMsg[i % 4].msgBase.containerType;
		dirtyAckMsg.msgBase.streamId = rsslAckMsg[i % 4].msgBase.streamId;
		dirtyAckMsg.msgBase.msgKey.flags = rsslAckMsg[i % 4].msgBase.msgKey.flags;
		dirtyAckMsg.msgBase.msgKey.serviceId = rsslAckMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyAckMsg.msgBase.msgKey.nameType = rsslAckMsg[i % 4].msgBase.msgKey.nameType;
		dirtyAckMsg.msgBase.msgKey.name.length = rsslAckMsg[i % 4].msgBase.msgKey.name.length;
		dirtyAckMsg.msgBase.msgKey.name.data = rsslAckMsg[i % 4].msgBase.msgKey.name.data;
		dirtyAckMsg.msgBase.msgKey.filter = rsslAckMsg[i % 4].msgBase.msgKey.filter;
		dirtyAckMsg.msgBase.msgKey.identifier = rsslAckMsg[i % 4].msgBase.msgKey.identifier;
		dirtyAckMsg.msgBase.msgKey.attribContainerType = rsslAckMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyAckMsg.msgBase.msgKey.encAttrib.length = rsslAckMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyAckMsg.msgBase.msgKey.encAttrib.data = rsslAckMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyAckMsg.msgBase.encDataBody.length = rsslAckMsg[i % 4].msgBase.encDataBody.length;
		dirtyAckMsg.msgBase.encDataBody.data = rsslAckMsg[i % 4].msgBase.encDataBody.data;
		dirtyAckMsg.msgBase.encMsgBuffer.length = rsslAckMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyAckMsg.msgBase.encMsgBuffer.data = rsslAckMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyAckMsg.flags = rsslAckMsg[i % 4].flags;
		dirtyAckMsg.ackId = rsslAckMsg[i % 4].ackId;
		dirtyAckMsg.nakCode = rsslAckMsg[i % 4].nakCode;
		dirtyAckMsg.seqNum = rsslAckMsg[i % 4].seqNum;
		dirtyAckMsg.text.length = rsslAckMsg[i % 4].text.length;
		dirtyAckMsg.text.data = rsslAckMsg[i % 4].text.data;
		dirtyAckMsg.extendedHeader.length = rsslAckMsg[i % 4].extendedHeader.length;
		dirtyAckMsg.extendedHeader.data = rsslAckMsg[i % 4].extendedHeader.data;
	
		rsslAckMsgClear(&dirtyAckMsg);
	}
	//printf("\tAckMsg Clear Test:");
	endTimerAndPrint();

	//Test using memset
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyAckMsg.msgBase.msgClass = rsslAckMsg[i % 4].msgBase.msgClass;
		dirtyAckMsg.msgBase.domainType = rsslAckMsg[i % 4].msgBase.domainType;
		dirtyAckMsg.msgBase.containerType = rsslAckMsg[i % 4].msgBase.containerType;
		dirtyAckMsg.msgBase.streamId = rsslAckMsg[i % 4].msgBase.streamId;
		dirtyAckMsg.msgBase.msgKey.flags = rsslAckMsg[i % 4].msgBase.msgKey.flags;
		dirtyAckMsg.msgBase.msgKey.serviceId = rsslAckMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyAckMsg.msgBase.msgKey.nameType = rsslAckMsg[i % 4].msgBase.msgKey.nameType;
		dirtyAckMsg.msgBase.msgKey.name.length = rsslAckMsg[i % 4].msgBase.msgKey.name.length;
		dirtyAckMsg.msgBase.msgKey.name.data = rsslAckMsg[i % 4].msgBase.msgKey.name.data;
		dirtyAckMsg.msgBase.msgKey.filter = rsslAckMsg[i % 4].msgBase.msgKey.filter;
		dirtyAckMsg.msgBase.msgKey.identifier = rsslAckMsg[i % 4].msgBase.msgKey.identifier;
		dirtyAckMsg.msgBase.msgKey.attribContainerType = rsslAckMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyAckMsg.msgBase.msgKey.encAttrib.length = rsslAckMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyAckMsg.msgBase.msgKey.encAttrib.data = rsslAckMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyAckMsg.msgBase.encDataBody.length = rsslAckMsg[i % 4].msgBase.encDataBody.length;
		dirtyAckMsg.msgBase.encDataBody.data = rsslAckMsg[i % 4].msgBase.encDataBody.data;
		dirtyAckMsg.msgBase.encMsgBuffer.length = rsslAckMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyAckMsg.msgBase.encMsgBuffer.data = rsslAckMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyAckMsg.flags = rsslAckMsg[i % 4].flags;
		dirtyAckMsg.ackId = rsslAckMsg[i % 4].ackId;
		dirtyAckMsg.nakCode = rsslAckMsg[i % 4].nakCode;
		dirtyAckMsg.seqNum = rsslAckMsg[i % 4].seqNum;
		dirtyAckMsg.text.length = rsslAckMsg[i % 4].text.length;
		dirtyAckMsg.text.data = rsslAckMsg[i % 4].text.data;
		dirtyAckMsg.extendedHeader.length = rsslAckMsg[i % 4].extendedHeader.length;
		dirtyAckMsg.extendedHeader.data = rsslAckMsg[i % 4].extendedHeader.data;
	
		memset(&dirtyAckMsg, 0, sizeof(RsslAckMsg));
	}
	//printf("\tAckMsg Memset Test:");
	endTimerAndPrint();
}

void rsslUpdateMsgClear(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->msgBase.msgClass = 0;
	pUpdateMsg->msgBase.domainType = 0;
	pUpdateMsg->msgBase.containerType = 0;
	pUpdateMsg->msgBase.streamId = 0;
	pUpdateMsg->msgBase.msgKey.flags = 0;
	pUpdateMsg->msgBase.msgKey.serviceId = 0;
	pUpdateMsg->msgBase.msgKey.nameType = 0;
	pUpdateMsg->msgBase.msgKey.name.length = 0;
	pUpdateMsg->msgBase.msgKey.name.data = 0;
	pUpdateMsg->msgBase.msgKey.filter = 0;
	pUpdateMsg->msgBase.msgKey.identifier = 0;
	pUpdateMsg->msgBase.msgKey.attribContainerType = 0;
	pUpdateMsg->msgBase.msgKey.encAttrib.length = 0;
	pUpdateMsg->msgBase.msgKey.encAttrib.data = 0;
	pUpdateMsg->msgBase.encDataBody.length = 0;
	pUpdateMsg->msgBase.encDataBody.data = 0;
	pUpdateMsg->msgBase.encMsgBuffer.length = 0;
	pUpdateMsg->msgBase.encMsgBuffer.data = 0;
	pUpdateMsg->flags = 0;
	pUpdateMsg->updateType = 0;
	pUpdateMsg->seqNum = 0;
	pUpdateMsg->conflationCount = 0;
	pUpdateMsg->conflationTime = 0;
	pUpdateMsg->permData.length = 0;
	pUpdateMsg->permData.data = 0;
	pUpdateMsg->postUserInfo.postUserAddr = 0;
	pUpdateMsg->postUserInfo.postUserId = 0;
	pUpdateMsg->extendedHeader.length = 0;
	pUpdateMsg->extendedHeader.data = 0;
}

void updateMsgClearMemSetTest()
{
	RsslUpdateMsg rsslUpdateMsg[4];
	RsslUpdateMsg dirtyUpdateMsg;
	int i;

	rsslUpdateMsg[0].msgBase.msgClass = 1;
	rsslUpdateMsg[0].msgBase.domainType = 1;
	rsslUpdateMsg[0].msgBase.containerType = 1;
	rsslUpdateMsg[0].msgBase.streamId = 1;
	rsslUpdateMsg[0].msgBase.msgKey.flags = 1;
	rsslUpdateMsg[0].msgBase.msgKey.serviceId = 1;
	rsslUpdateMsg[0].msgBase.msgKey.nameType = 1;
	rsslUpdateMsg[0].msgBase.msgKey.name.length = 1;
	rsslUpdateMsg[0].msgBase.msgKey.name.data = (char*)1;
	rsslUpdateMsg[0].msgBase.msgKey.filter = 1;
	rsslUpdateMsg[0].msgBase.msgKey.identifier = 1;
	rsslUpdateMsg[0].msgBase.msgKey.attribContainerType = 1;
	rsslUpdateMsg[0].msgBase.msgKey.encAttrib.length = 1;
	rsslUpdateMsg[0].msgBase.msgKey.encAttrib.data = (char*)1;
	rsslUpdateMsg[0].msgBase.encDataBody.length = 1;
	rsslUpdateMsg[0].msgBase.encDataBody.data = (char*)1;
	rsslUpdateMsg[0].msgBase.encMsgBuffer.length = 1;
	rsslUpdateMsg[0].msgBase.encMsgBuffer.data = (char*)1;
	rsslUpdateMsg[0].flags = 1;
	rsslUpdateMsg[0].updateType = 1;
	rsslUpdateMsg[0].seqNum = 1;
	rsslUpdateMsg[0].conflationCount = 1;
	rsslUpdateMsg[0].conflationTime = 1;
	rsslUpdateMsg[0].permData.length = 1;
	rsslUpdateMsg[0].permData.data = (char*)1;
	rsslUpdateMsg[0].postUserInfo.postUserAddr = 1;
	rsslUpdateMsg[0].postUserInfo.postUserId = 1;
	rsslUpdateMsg[0].extendedHeader.length = 1;
	rsslUpdateMsg[0].extendedHeader.data = (char*)1;

	rsslUpdateMsg[1].msgBase.msgClass = 2;
	rsslUpdateMsg[1].msgBase.domainType = 2;
	rsslUpdateMsg[1].msgBase.containerType = 2;
	rsslUpdateMsg[1].msgBase.streamId = 2;
	rsslUpdateMsg[1].msgBase.msgKey.flags = 2;
	rsslUpdateMsg[1].msgBase.msgKey.serviceId = 2;
	rsslUpdateMsg[1].msgBase.msgKey.nameType = 2;
	rsslUpdateMsg[1].msgBase.msgKey.name.length = 2;
	rsslUpdateMsg[1].msgBase.msgKey.name.data = (char*)2;
	rsslUpdateMsg[1].msgBase.msgKey.filter = 2;
	rsslUpdateMsg[1].msgBase.msgKey.identifier = 2;
	rsslUpdateMsg[1].msgBase.msgKey.attribContainerType = 2;
	rsslUpdateMsg[1].msgBase.msgKey.encAttrib.length = 2;
	rsslUpdateMsg[1].msgBase.msgKey.encAttrib.data = (char*)2;
	rsslUpdateMsg[1].msgBase.encDataBody.length = 2;
	rsslUpdateMsg[1].msgBase.encDataBody.data = (char*)2;
	rsslUpdateMsg[1].msgBase.encMsgBuffer.length = 2;
	rsslUpdateMsg[1].msgBase.encMsgBuffer.data = (char*)2;
	rsslUpdateMsg[1].flags = 2;
	rsslUpdateMsg[1].updateType = 2;
	rsslUpdateMsg[1].seqNum = 2;
	rsslUpdateMsg[1].conflationCount = 2;
	rsslUpdateMsg[1].conflationTime = 2;
	rsslUpdateMsg[1].permData.length = 2;
	rsslUpdateMsg[1].permData.data = (char*)2;
	rsslUpdateMsg[1].postUserInfo.postUserAddr = 2;
	rsslUpdateMsg[1].postUserInfo.postUserId = 2;
	rsslUpdateMsg[1].extendedHeader.length = 2;
	rsslUpdateMsg[1].extendedHeader.data = (char*)2;

	rsslUpdateMsg[2].msgBase.msgClass = 3;
	rsslUpdateMsg[2].msgBase.domainType = 3;
	rsslUpdateMsg[2].msgBase.containerType = 3;
	rsslUpdateMsg[2].msgBase.streamId = 3;
	rsslUpdateMsg[2].msgBase.msgKey.flags = 3;
	rsslUpdateMsg[2].msgBase.msgKey.serviceId = 3;
	rsslUpdateMsg[2].msgBase.msgKey.nameType = 3;
	rsslUpdateMsg[2].msgBase.msgKey.name.length = 3;
	rsslUpdateMsg[2].msgBase.msgKey.name.data = (char*)3;
	rsslUpdateMsg[2].msgBase.msgKey.filter = 3;
	rsslUpdateMsg[2].msgBase.msgKey.identifier = 3;
	rsslUpdateMsg[2].msgBase.msgKey.attribContainerType = 3;
	rsslUpdateMsg[2].msgBase.msgKey.encAttrib.length = 3;
	rsslUpdateMsg[2].msgBase.msgKey.encAttrib.data = (char*)3;
	rsslUpdateMsg[2].msgBase.encDataBody.length = 3;
	rsslUpdateMsg[2].msgBase.encDataBody.data = (char*)3;
	rsslUpdateMsg[2].msgBase.encMsgBuffer.length = 3;
	rsslUpdateMsg[2].msgBase.encMsgBuffer.data = (char*)3;
	rsslUpdateMsg[2].flags = 3;
	rsslUpdateMsg[2].updateType = 3;
	rsslUpdateMsg[2].seqNum = 3;
	rsslUpdateMsg[2].conflationCount = 3;
	rsslUpdateMsg[2].conflationTime = 3;
	rsslUpdateMsg[2].permData.length = 3;
	rsslUpdateMsg[2].permData.data = (char*)3;
	rsslUpdateMsg[2].postUserInfo.postUserAddr = 3;
	rsslUpdateMsg[2].postUserInfo.postUserId = 3;
	rsslUpdateMsg[2].extendedHeader.length = 3;
	rsslUpdateMsg[2].extendedHeader.data = (char*)3;

	rsslUpdateMsg[3].msgBase.msgClass = 4;
	rsslUpdateMsg[3].msgBase.domainType = 4;
	rsslUpdateMsg[3].msgBase.containerType = 4;
	rsslUpdateMsg[3].msgBase.streamId = 4;
	rsslUpdateMsg[3].msgBase.msgKey.flags = 4;
	rsslUpdateMsg[3].msgBase.msgKey.serviceId = 4;
	rsslUpdateMsg[3].msgBase.msgKey.nameType = 4;
	rsslUpdateMsg[3].msgBase.msgKey.name.length = 4;
	rsslUpdateMsg[3].msgBase.msgKey.name.data = (char*)4;
	rsslUpdateMsg[3].msgBase.msgKey.filter = 4;
	rsslUpdateMsg[3].msgBase.msgKey.identifier = 4;
	rsslUpdateMsg[3].msgBase.msgKey.attribContainerType = 4;
	rsslUpdateMsg[3].msgBase.msgKey.encAttrib.length = 4;
	rsslUpdateMsg[3].msgBase.msgKey.encAttrib.data = (char*)4;
	rsslUpdateMsg[3].msgBase.encDataBody.length = 4;
	rsslUpdateMsg[3].msgBase.encDataBody.data = (char*)4;
	rsslUpdateMsg[3].msgBase.encMsgBuffer.length = 4;
	rsslUpdateMsg[3].msgBase.encMsgBuffer.data = (char*)4;
	rsslUpdateMsg[3].flags = 4;
	rsslUpdateMsg[3].updateType = 4;
	rsslUpdateMsg[3].seqNum = 4;
	rsslUpdateMsg[3].conflationCount = 4;
	rsslUpdateMsg[3].conflationTime = 4;
	rsslUpdateMsg[3].permData.length = 4;
	rsslUpdateMsg[3].permData.data = (char*)4;
	rsslUpdateMsg[3].postUserInfo.postUserAddr = 4;
	rsslUpdateMsg[3].postUserInfo.postUserId = 4;
	rsslUpdateMsg[3].extendedHeader.length = 4;
	rsslUpdateMsg[3].extendedHeader.data = (char*)4;

	//Test setting individual values with rsslUpdateMsgClear
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyUpdateMsg.msgBase.msgClass = rsslUpdateMsg[i % 4].msgBase.msgClass;
		dirtyUpdateMsg.msgBase.domainType = rsslUpdateMsg[i % 4].msgBase.domainType;
		dirtyUpdateMsg.msgBase.containerType = rsslUpdateMsg[i % 4].msgBase.containerType;
		dirtyUpdateMsg.msgBase.streamId = rsslUpdateMsg[i % 4].msgBase.streamId;
		dirtyUpdateMsg.msgBase.msgKey.flags = rsslUpdateMsg[i % 4].msgBase.msgKey.flags;
		dirtyUpdateMsg.msgBase.msgKey.serviceId = rsslUpdateMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyUpdateMsg.msgBase.msgKey.nameType = rsslUpdateMsg[i % 4].msgBase.msgKey.nameType;
		dirtyUpdateMsg.msgBase.msgKey.name.length = rsslUpdateMsg[i % 4].msgBase.msgKey.name.length;
		dirtyUpdateMsg.msgBase.msgKey.name.data = rsslUpdateMsg[i % 4].msgBase.msgKey.name.data;
		dirtyUpdateMsg.msgBase.msgKey.filter = rsslUpdateMsg[i % 4].msgBase.msgKey.filter;
		dirtyUpdateMsg.msgBase.msgKey.identifier = rsslUpdateMsg[i % 4].msgBase.msgKey.identifier;
		dirtyUpdateMsg.msgBase.msgKey.attribContainerType = rsslUpdateMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyUpdateMsg.msgBase.msgKey.encAttrib.length = rsslUpdateMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyUpdateMsg.msgBase.msgKey.encAttrib.data = rsslUpdateMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyUpdateMsg.msgBase.encDataBody.length = rsslUpdateMsg[i % 4].msgBase.encDataBody.length;
		dirtyUpdateMsg.msgBase.encDataBody.data = rsslUpdateMsg[i % 4].msgBase.encDataBody.data;
		dirtyUpdateMsg.msgBase.encMsgBuffer.length = rsslUpdateMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyUpdateMsg.msgBase.encMsgBuffer.data = rsslUpdateMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyUpdateMsg.flags = rsslUpdateMsg[i % 4].flags;
		dirtyUpdateMsg.updateType = rsslUpdateMsg[i % 4].updateType;
		dirtyUpdateMsg.seqNum = rsslUpdateMsg[i % 4].seqNum;
		dirtyUpdateMsg.conflationCount = rsslUpdateMsg[i % 4].conflationCount;
		dirtyUpdateMsg.conflationTime = rsslUpdateMsg[i % 4].conflationTime;
		dirtyUpdateMsg.permData.length = rsslUpdateMsg[i % 4].permData.length;
		dirtyUpdateMsg.permData.data = rsslUpdateMsg[i % 4].permData.data;
		dirtyUpdateMsg.postUserInfo.postUserAddr = rsslUpdateMsg[i % 4].postUserInfo.postUserAddr;
		dirtyUpdateMsg.postUserInfo.postUserId = rsslUpdateMsg[i % 4].postUserInfo.postUserId;
		dirtyUpdateMsg.extendedHeader.length = rsslUpdateMsg[i % 4].extendedHeader.length;
		dirtyUpdateMsg.extendedHeader.data = rsslUpdateMsg[i % 4].extendedHeader.data;

		rsslUpdateMsgClear(&dirtyUpdateMsg);
	}
	//printf("\tUpdateMsg Clear Test:");
	endTimerAndPrint();

	//Test setting individual values with rsslUpdateMsgClear
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyUpdateMsg.msgBase.msgClass = rsslUpdateMsg[i % 4].msgBase.msgClass;
		dirtyUpdateMsg.msgBase.domainType = rsslUpdateMsg[i % 4].msgBase.domainType;
		dirtyUpdateMsg.msgBase.containerType = rsslUpdateMsg[i % 4].msgBase.containerType;
		dirtyUpdateMsg.msgBase.streamId = rsslUpdateMsg[i % 4].msgBase.streamId;
		dirtyUpdateMsg.msgBase.msgKey.flags = rsslUpdateMsg[i % 4].msgBase.msgKey.flags;
		dirtyUpdateMsg.msgBase.msgKey.serviceId = rsslUpdateMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyUpdateMsg.msgBase.msgKey.nameType = rsslUpdateMsg[i % 4].msgBase.msgKey.nameType;
		dirtyUpdateMsg.msgBase.msgKey.name.length = rsslUpdateMsg[i % 4].msgBase.msgKey.name.length;
		dirtyUpdateMsg.msgBase.msgKey.name.data = rsslUpdateMsg[i % 4].msgBase.msgKey.name.data;
		dirtyUpdateMsg.msgBase.msgKey.filter = rsslUpdateMsg[i % 4].msgBase.msgKey.filter;
		dirtyUpdateMsg.msgBase.msgKey.identifier = rsslUpdateMsg[i % 4].msgBase.msgKey.identifier;
		dirtyUpdateMsg.msgBase.msgKey.attribContainerType = rsslUpdateMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyUpdateMsg.msgBase.msgKey.encAttrib.length = rsslUpdateMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyUpdateMsg.msgBase.msgKey.encAttrib.data = rsslUpdateMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyUpdateMsg.msgBase.encDataBody.length = rsslUpdateMsg[i % 4].msgBase.encDataBody.length;
		dirtyUpdateMsg.msgBase.encDataBody.data = rsslUpdateMsg[i % 4].msgBase.encDataBody.data;
		dirtyUpdateMsg.msgBase.encMsgBuffer.length = rsslUpdateMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyUpdateMsg.msgBase.encMsgBuffer.data = rsslUpdateMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyUpdateMsg.flags = rsslUpdateMsg[i % 4].flags;
		dirtyUpdateMsg.updateType = rsslUpdateMsg[i % 4].updateType;
		dirtyUpdateMsg.seqNum = rsslUpdateMsg[i % 4].seqNum;
		dirtyUpdateMsg.conflationCount = rsslUpdateMsg[i % 4].conflationCount;
		dirtyUpdateMsg.conflationTime = rsslUpdateMsg[i % 4].conflationTime;
		dirtyUpdateMsg.permData.length = rsslUpdateMsg[i % 4].permData.length;
		dirtyUpdateMsg.permData.data = rsslUpdateMsg[i % 4].permData.data;
		dirtyUpdateMsg.postUserInfo.postUserAddr = rsslUpdateMsg[i % 4].postUserInfo.postUserAddr;
		dirtyUpdateMsg.postUserInfo.postUserId = rsslUpdateMsg[i % 4].postUserInfo.postUserId;
		dirtyUpdateMsg.extendedHeader.length = rsslUpdateMsg[i % 4].extendedHeader.length;
		dirtyUpdateMsg.extendedHeader.data = rsslUpdateMsg[i % 4].extendedHeader.data;

		memset(&dirtyUpdateMsg, 0, sizeof(RsslUpdateMsg));
	}
	//printf("\tUpdateMsg Memset Test:");
	endTimerAndPrint();
}

void rsslRefreshMsgClear(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->msgBase.msgClass = 0;
	pRefreshMsg->msgBase.domainType = 0;
	pRefreshMsg->msgBase.containerType = 0;
	pRefreshMsg->msgBase.streamId = 0;
	pRefreshMsg->msgBase.msgKey.flags = 0;
	pRefreshMsg->msgBase.msgKey.serviceId = 0;
	pRefreshMsg->msgBase.msgKey.nameType = 0;
	pRefreshMsg->msgBase.msgKey.name.length = 0;
	pRefreshMsg->msgBase.msgKey.name.data = 0;
	pRefreshMsg->msgBase.msgKey.filter = 0;
	pRefreshMsg->msgBase.msgKey.identifier = 0;
	pRefreshMsg->msgBase.msgKey.attribContainerType = 0;
	pRefreshMsg->msgBase.msgKey.encAttrib.length = 0;
	pRefreshMsg->msgBase.msgKey.encAttrib.data = 0;
	pRefreshMsg->msgBase.encDataBody.length = 0;
	pRefreshMsg->msgBase.encDataBody.data = 0;
	pRefreshMsg->msgBase.encMsgBuffer.length = 0;
	pRefreshMsg->msgBase.encMsgBuffer.data = 0;
	pRefreshMsg->flags = 0;
	pRefreshMsg->partNum = 0;
	pRefreshMsg->seqNum = 0;
	pRefreshMsg->state.streamState = 0;
	pRefreshMsg->state.dataState = 0;
	pRefreshMsg->state.code = 0;
	pRefreshMsg->state.text.length = 0;
	pRefreshMsg->state.text.data = 0;
	pRefreshMsg->groupId.length = 0;
	pRefreshMsg->groupId.data = 0;
	pRefreshMsg->permData.length = 0;
	pRefreshMsg->permData.data = 0;
	pRefreshMsg->postUserInfo.postUserAddr = 0;
	pRefreshMsg->postUserInfo.postUserId = 0;
	pRefreshMsg->qos.timeliness = 0;
	pRefreshMsg->qos.rate = 0;
	pRefreshMsg->qos.dynamic = 0;
	pRefreshMsg->qos.timeInfo = 0;
	pRefreshMsg->qos.rateInfo = 0;
	pRefreshMsg->extendedHeader.length = 0;
	pRefreshMsg->extendedHeader.data = 0;
}

void refreshMsgClearMemSetTest()
{
	RsslRefreshMsg rsslRefreshMsg[4];
	RsslRefreshMsg dirtyRefreshMsg;
	int i;
	RsslUInt8 dynamic = 1;

	rsslRefreshMsg[0].msgBase.msgClass = 1;
	rsslRefreshMsg[0].msgBase.domainType = 1;
	rsslRefreshMsg[0].msgBase.containerType = 1;
	rsslRefreshMsg[0].msgBase.streamId = 1;
	rsslRefreshMsg[0].msgBase.msgKey.flags = 1;
	rsslRefreshMsg[0].msgBase.msgKey.serviceId = 1;
	rsslRefreshMsg[0].msgBase.msgKey.nameType = 1;
	rsslRefreshMsg[0].msgBase.msgKey.name.length = 1;
	rsslRefreshMsg[0].msgBase.msgKey.name.data = (char*)1;
	rsslRefreshMsg[0].msgBase.msgKey.filter = 1;
	rsslRefreshMsg[0].msgBase.msgKey.identifier = 1;
	rsslRefreshMsg[0].msgBase.msgKey.attribContainerType = 1;
	rsslRefreshMsg[0].msgBase.msgKey.encAttrib.length = 1;
	rsslRefreshMsg[0].msgBase.msgKey.encAttrib.data = (char*)1;
	rsslRefreshMsg[0].msgBase.encDataBody.length = 0;
	rsslRefreshMsg[0].msgBase.encDataBody.data = (char*)1;
	rsslRefreshMsg[0].msgBase.encMsgBuffer.length = 1;
	rsslRefreshMsg[0].msgBase.encMsgBuffer.data = (char*)1;
	rsslRefreshMsg[0].flags = 1;
	rsslRefreshMsg[0].partNum = 1;
	rsslRefreshMsg[0].seqNum = 1;
	rsslRefreshMsg[0].state.streamState = 1;
	rsslRefreshMsg[0].state.dataState = 1;
	rsslRefreshMsg[0].state.code = 1;
	rsslRefreshMsg[0].state.text.length = 1;
	rsslRefreshMsg[0].state.text.data = (char*)1;
	rsslRefreshMsg[0].groupId.length = 1;
	rsslRefreshMsg[0].groupId.data = (char*)1;
	rsslRefreshMsg[0].permData.length = 1;
	rsslRefreshMsg[0].permData.data = (char*)1;
	rsslRefreshMsg[0].postUserInfo.postUserAddr = 1;
	rsslRefreshMsg[0].postUserInfo.postUserId = 1;
	rsslRefreshMsg[0].qos.timeliness = 1;
	rsslRefreshMsg[0].qos.rate = 1;
	rsslRefreshMsg[0].qos.dynamic = dynamic;
	rsslRefreshMsg[0].qos.timeInfo = 1;
	rsslRefreshMsg[0].qos.rateInfo = 1;
	rsslRefreshMsg[0].extendedHeader.length = 1;
	rsslRefreshMsg[0].extendedHeader.data = (char*)1;

	rsslRefreshMsg[1].msgBase.msgClass = 2;
	rsslRefreshMsg[1].msgBase.domainType = 2;
	rsslRefreshMsg[1].msgBase.containerType = 2;
	rsslRefreshMsg[1].msgBase.streamId = 2;
	rsslRefreshMsg[1].msgBase.msgKey.flags = 2;
	rsslRefreshMsg[1].msgBase.msgKey.serviceId = 2;
	rsslRefreshMsg[1].msgBase.msgKey.nameType = 2;
	rsslRefreshMsg[1].msgBase.msgKey.name.length = 2;
	rsslRefreshMsg[1].msgBase.msgKey.name.data = (char*)2;
	rsslRefreshMsg[1].msgBase.msgKey.filter = 2;
	rsslRefreshMsg[1].msgBase.msgKey.identifier = 2;
	rsslRefreshMsg[1].msgBase.msgKey.attribContainerType = 2;
	rsslRefreshMsg[1].msgBase.msgKey.encAttrib.length = 2;
	rsslRefreshMsg[1].msgBase.msgKey.encAttrib.data = (char*)2;
	rsslRefreshMsg[1].msgBase.encDataBody.length = 2;
	rsslRefreshMsg[1].msgBase.encDataBody.data = (char*)2;
	rsslRefreshMsg[1].msgBase.encMsgBuffer.length = 2;
	rsslRefreshMsg[1].msgBase.encMsgBuffer.data = (char*)2;
	rsslRefreshMsg[1].flags = 2;
	rsslRefreshMsg[1].partNum = 2;
	rsslRefreshMsg[1].seqNum = 2;
	rsslRefreshMsg[1].state.streamState = 2;
	rsslRefreshMsg[1].state.dataState = 2;
	rsslRefreshMsg[1].state.code = 2;
	rsslRefreshMsg[1].state.text.length = 2;
	rsslRefreshMsg[1].state.text.data = (char*)2;
	rsslRefreshMsg[1].groupId.length = 2;
	rsslRefreshMsg[1].groupId.data = (char*)2;
	rsslRefreshMsg[1].permData.length = 2;
	rsslRefreshMsg[1].permData.data = (char*)2;
	rsslRefreshMsg[1].postUserInfo.postUserAddr = 2;
	rsslRefreshMsg[1].postUserInfo.postUserId = 2;
	rsslRefreshMsg[1].qos.timeliness = 2;
	rsslRefreshMsg[1].qos.rate = 2;
	dynamic = 2;
	rsslRefreshMsg[1].qos.dynamic = dynamic;
	rsslRefreshMsg[1].qos.timeInfo = 2;
	rsslRefreshMsg[1].qos.rateInfo = 2;
	rsslRefreshMsg[1].extendedHeader.length = 2;
	rsslRefreshMsg[1].extendedHeader.data = (char*)2;

	rsslRefreshMsg[2].msgBase.msgClass = 3;
	rsslRefreshMsg[2].msgBase.domainType = 3;
	rsslRefreshMsg[2].msgBase.containerType = 3;
	rsslRefreshMsg[2].msgBase.streamId = 3;
	rsslRefreshMsg[2].msgBase.msgKey.flags = 3;
	rsslRefreshMsg[2].msgBase.msgKey.serviceId = 3;
	rsslRefreshMsg[2].msgBase.msgKey.nameType = 3;
	rsslRefreshMsg[2].msgBase.msgKey.name.length = 3;
	rsslRefreshMsg[2].msgBase.msgKey.name.data = (char*)3;
	rsslRefreshMsg[2].msgBase.msgKey.filter = 3;
	rsslRefreshMsg[2].msgBase.msgKey.identifier = 3;
	rsslRefreshMsg[2].msgBase.msgKey.attribContainerType = 3;
	rsslRefreshMsg[2].msgBase.msgKey.encAttrib.length = 3;
	rsslRefreshMsg[2].msgBase.msgKey.encAttrib.data = (char*)3;
	rsslRefreshMsg[2].msgBase.encDataBody.length = 3;
	rsslRefreshMsg[2].msgBase.encDataBody.data = (char*)3;
	rsslRefreshMsg[2].msgBase.encMsgBuffer.length = 3;
	rsslRefreshMsg[2].msgBase.encMsgBuffer.data = (char*)3;
	rsslRefreshMsg[2].flags = 3;
	rsslRefreshMsg[2].partNum = 3;
	rsslRefreshMsg[2].seqNum = 3;
	rsslRefreshMsg[2].state.streamState = 3;
	rsslRefreshMsg[2].state.dataState = 3;
	rsslRefreshMsg[2].state.code = 3;
	rsslRefreshMsg[2].state.text.length = 3;
	rsslRefreshMsg[2].state.text.data = (char*)3;
	rsslRefreshMsg[2].groupId.length = 3;
	rsslRefreshMsg[2].groupId.data = (char*)3;
	rsslRefreshMsg[2].permData.length = 3;
	rsslRefreshMsg[2].permData.data = (char*)3;
	rsslRefreshMsg[2].postUserInfo.postUserAddr = 3;
	rsslRefreshMsg[2].postUserInfo.postUserId = 3;
	rsslRefreshMsg[2].qos.timeliness = 3;
	rsslRefreshMsg[2].qos.rate = 3;
	dynamic = 3;
	rsslRefreshMsg[2].qos.dynamic = dynamic;
	rsslRefreshMsg[2].qos.timeInfo = 3;
	rsslRefreshMsg[2].qos.rateInfo = 3;
	rsslRefreshMsg[2].extendedHeader.length = 3;
	rsslRefreshMsg[2].extendedHeader.data = (char*)3;

	rsslRefreshMsg[3].msgBase.msgClass = 4;
	rsslRefreshMsg[3].msgBase.domainType = 4;
	rsslRefreshMsg[3].msgBase.containerType = 4;
	rsslRefreshMsg[3].msgBase.streamId = 4;
	rsslRefreshMsg[3].msgBase.msgKey.flags = 4;
	rsslRefreshMsg[3].msgBase.msgKey.serviceId = 4;
	rsslRefreshMsg[3].msgBase.msgKey.nameType = 4;
	rsslRefreshMsg[3].msgBase.msgKey.name.length = 4;
	rsslRefreshMsg[3].msgBase.msgKey.name.data = (char*)4;
	rsslRefreshMsg[3].msgBase.msgKey.filter = 4;
	rsslRefreshMsg[3].msgBase.msgKey.identifier = 4;
	rsslRefreshMsg[3].msgBase.msgKey.attribContainerType = 4;
	rsslRefreshMsg[3].msgBase.msgKey.encAttrib.length = 4;
	rsslRefreshMsg[3].msgBase.msgKey.encAttrib.data = (char*)4;
	rsslRefreshMsg[3].msgBase.encDataBody.length = 4;
	rsslRefreshMsg[3].msgBase.encDataBody.data = (char*)4;
	rsslRefreshMsg[3].msgBase.encMsgBuffer.length = 4;
	rsslRefreshMsg[3].msgBase.encMsgBuffer.data = (char*)4;
	rsslRefreshMsg[3].flags = 4;
	rsslRefreshMsg[3].partNum = 4;
	rsslRefreshMsg[3].seqNum = 4;
	rsslRefreshMsg[3].state.streamState = 4;
	rsslRefreshMsg[3].state.dataState = 4;
	rsslRefreshMsg[3].state.code = 4;
	rsslRefreshMsg[3].state.text.length = 4;
	rsslRefreshMsg[3].state.text.data = (char*)4;
	rsslRefreshMsg[3].groupId.length = 4;
	rsslRefreshMsg[3].groupId.data = (char*)4;
	rsslRefreshMsg[3].permData.length = 4;
	rsslRefreshMsg[3].permData.data = (char*)4;
	rsslRefreshMsg[3].postUserInfo.postUserAddr = 4;
	rsslRefreshMsg[3].postUserInfo.postUserId = 4;
	rsslRefreshMsg[3].qos.timeliness = 4;
	rsslRefreshMsg[3].qos.rate = 4;
	dynamic = 4;
	rsslRefreshMsg[3].qos.dynamic = dynamic;
	rsslRefreshMsg[3].qos.timeInfo = 4;
	rsslRefreshMsg[3].qos.rateInfo = 4;
	rsslRefreshMsg[3].extendedHeader.length = 4;
	rsslRefreshMsg[3].extendedHeader.data = (char*)4;

	//Test setting individual values with rsslRefreshMsgClear
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyRefreshMsg.msgBase.msgClass = rsslRefreshMsg[i % 4].msgBase.msgClass;
		dirtyRefreshMsg.msgBase.domainType= rsslRefreshMsg[i % 4].msgBase.domainType;
		dirtyRefreshMsg.msgBase.containerType = rsslRefreshMsg[i % 4].msgBase.containerType;
		dirtyRefreshMsg.msgBase.streamId = rsslRefreshMsg[i % 4].msgBase.streamId;
		dirtyRefreshMsg.msgBase.msgKey.flags = rsslRefreshMsg[i % 4].msgBase.msgKey.flags;
		dirtyRefreshMsg.msgBase.msgKey.serviceId = rsslRefreshMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyRefreshMsg.msgBase.msgKey.nameType = rsslRefreshMsg[i % 4].msgBase.msgKey.nameType;
		dirtyRefreshMsg.msgBase.msgKey.name.length = rsslRefreshMsg[i % 4].msgBase.msgKey.name.length;
		dirtyRefreshMsg.msgBase.msgKey.name.data = rsslRefreshMsg[i % 4].msgBase.msgKey.name.data;
		dirtyRefreshMsg.msgBase.msgKey.filter = rsslRefreshMsg[i % 4].msgBase.msgKey.filter;
		dirtyRefreshMsg.msgBase.msgKey.identifier = rsslRefreshMsg[i % 4].msgBase.msgKey.identifier;
		dirtyRefreshMsg.msgBase.msgKey.attribContainerType = rsslRefreshMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyRefreshMsg.msgBase.msgKey.encAttrib.length = rsslRefreshMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyRefreshMsg.msgBase.msgKey.encAttrib.data = rsslRefreshMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyRefreshMsg.msgBase.encDataBody.length = rsslRefreshMsg[i % 4].msgBase.encDataBody.length;
		dirtyRefreshMsg.msgBase.encDataBody.data = rsslRefreshMsg[i % 4].msgBase.encDataBody.data;
		dirtyRefreshMsg.msgBase.encMsgBuffer.length = rsslRefreshMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyRefreshMsg.msgBase.encMsgBuffer.data = rsslRefreshMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyRefreshMsg.flags = rsslRefreshMsg[i % 4].flags;
		dirtyRefreshMsg.partNum = rsslRefreshMsg[i % 4].partNum;
		dirtyRefreshMsg.seqNum = rsslRefreshMsg[i % 4].seqNum;
		dirtyRefreshMsg.state.streamState = rsslRefreshMsg[i % 4].state.streamState;
		dirtyRefreshMsg.state.dataState = rsslRefreshMsg[i % 4].state.dataState;
		dirtyRefreshMsg.state.code = rsslRefreshMsg[i % 4].state.code;
		dirtyRefreshMsg.state.text.length = rsslRefreshMsg[i % 4].state.text.length;
		dirtyRefreshMsg.state.text.data = rsslRefreshMsg[i % 4].state.text.data;
		dirtyRefreshMsg.groupId.length = rsslRefreshMsg[i % 4].groupId.length;
		dirtyRefreshMsg.groupId.data = rsslRefreshMsg[i % 4].groupId.data;
		dirtyRefreshMsg.permData.length = rsslRefreshMsg[i % 4].permData.length;
		dirtyRefreshMsg.permData.data = rsslRefreshMsg[i % 4].permData.data;
		dirtyRefreshMsg.postUserInfo.postUserAddr = rsslRefreshMsg[i % 4].postUserInfo.postUserAddr;
		dirtyRefreshMsg.postUserInfo.postUserId = rsslRefreshMsg[i % 4].postUserInfo.postUserId;
		dirtyRefreshMsg.qos.timeliness = rsslRefreshMsg[i % 4].qos.timeliness;
		dirtyRefreshMsg.qos.rate = rsslRefreshMsg[i % 4].qos.rate;
		dirtyRefreshMsg.qos.dynamic = rsslRefreshMsg[i % 4].qos.dynamic;
		dirtyRefreshMsg.qos.timeInfo = rsslRefreshMsg[i % 4].qos.timeInfo;
		dirtyRefreshMsg.qos.rateInfo = rsslRefreshMsg[i % 4].qos.rateInfo;
		dirtyRefreshMsg.extendedHeader.length = rsslRefreshMsg[i % 4].extendedHeader.length;
		dirtyRefreshMsg.extendedHeader.data = rsslRefreshMsg[i % 4].extendedHeader.data;

		rsslRefreshMsgClear(&dirtyRefreshMsg);
	}
	//printf("\tRefreshMsg Clear Test:");
	endTimerAndPrint();

	//Test using memset
	startTimer();
	for(i = 0; i < 100000000; i++) 
	{
		dirtyRefreshMsg.msgBase.msgClass = rsslRefreshMsg[i % 4].msgBase.msgClass;
		dirtyRefreshMsg.msgBase.domainType= rsslRefreshMsg[i % 4].msgBase.domainType;
		dirtyRefreshMsg.msgBase.containerType = rsslRefreshMsg[i % 4].msgBase.containerType;
		dirtyRefreshMsg.msgBase.streamId = rsslRefreshMsg[i % 4].msgBase.streamId;
		dirtyRefreshMsg.msgBase.msgKey.flags = rsslRefreshMsg[i % 4].msgBase.msgKey.flags;
		dirtyRefreshMsg.msgBase.msgKey.serviceId = rsslRefreshMsg[i % 4].msgBase.msgKey.serviceId;
		dirtyRefreshMsg.msgBase.msgKey.nameType = rsslRefreshMsg[i % 4].msgBase.msgKey.nameType;
		dirtyRefreshMsg.msgBase.msgKey.name.length = rsslRefreshMsg[i % 4].msgBase.msgKey.name.length;
		dirtyRefreshMsg.msgBase.msgKey.name.data = rsslRefreshMsg[i % 4].msgBase.msgKey.name.data;
		dirtyRefreshMsg.msgBase.msgKey.filter = rsslRefreshMsg[i % 4].msgBase.msgKey.filter;
		dirtyRefreshMsg.msgBase.msgKey.identifier = rsslRefreshMsg[i % 4].msgBase.msgKey.identifier;
		dirtyRefreshMsg.msgBase.msgKey.attribContainerType = rsslRefreshMsg[i % 4].msgBase.msgKey.attribContainerType;
		dirtyRefreshMsg.msgBase.msgKey.encAttrib.length = rsslRefreshMsg[i % 4].msgBase.msgKey.encAttrib.length;
		dirtyRefreshMsg.msgBase.msgKey.encAttrib.data = rsslRefreshMsg[i % 4].msgBase.msgKey.encAttrib.data;
		dirtyRefreshMsg.msgBase.encDataBody.length = rsslRefreshMsg[i % 4].msgBase.encDataBody.length;
		dirtyRefreshMsg.msgBase.encDataBody.data = rsslRefreshMsg[i % 4].msgBase.encDataBody.data;
		dirtyRefreshMsg.msgBase.encMsgBuffer.length = rsslRefreshMsg[i % 4].msgBase.encMsgBuffer.length;
		dirtyRefreshMsg.msgBase.encMsgBuffer.data = rsslRefreshMsg[i % 4].msgBase.encMsgBuffer.data;
		dirtyRefreshMsg.flags = rsslRefreshMsg[i % 4].flags;
		dirtyRefreshMsg.partNum = rsslRefreshMsg[i % 4].partNum;
		dirtyRefreshMsg.seqNum = rsslRefreshMsg[i % 4].seqNum;
		dirtyRefreshMsg.state.streamState = rsslRefreshMsg[i % 4].state.streamState;
		dirtyRefreshMsg.state.dataState = rsslRefreshMsg[i % 4].state.dataState;
		dirtyRefreshMsg.state.code = rsslRefreshMsg[i % 4].state.code;
		dirtyRefreshMsg.state.text.length = rsslRefreshMsg[i % 4].state.text.length;
		dirtyRefreshMsg.state.text.data = rsslRefreshMsg[i % 4].state.text.data;
		dirtyRefreshMsg.groupId.length = rsslRefreshMsg[i % 4].groupId.length;
		dirtyRefreshMsg.groupId.data = rsslRefreshMsg[i % 4].groupId.data;
		dirtyRefreshMsg.permData.length = rsslRefreshMsg[i % 4].permData.length;
		dirtyRefreshMsg.permData.data = rsslRefreshMsg[i % 4].permData.data;
		dirtyRefreshMsg.postUserInfo.postUserAddr = rsslRefreshMsg[i % 4].postUserInfo.postUserAddr;
		dirtyRefreshMsg.postUserInfo.postUserId = rsslRefreshMsg[i % 4].postUserInfo.postUserId;
		dirtyRefreshMsg.qos.timeliness = rsslRefreshMsg[i % 4].qos.timeliness;
		dirtyRefreshMsg.qos.rate = rsslRefreshMsg[i % 4].qos.rate;
		dirtyRefreshMsg.qos.dynamic = rsslRefreshMsg[i % 4].qos.dynamic;
		dirtyRefreshMsg.qos.timeInfo = rsslRefreshMsg[i % 4].qos.timeInfo;
		dirtyRefreshMsg.qos.rateInfo = rsslRefreshMsg[i % 4].qos.rateInfo;
		dirtyRefreshMsg.extendedHeader.length = rsslRefreshMsg[i % 4].extendedHeader.length;
		dirtyRefreshMsg.extendedHeader.data = rsslRefreshMsg[i % 4].extendedHeader.data;

		memset(&dirtyRefreshMsg, 0, sizeof(RsslRefreshMsg));
	}
	//printf("\tRefreshMsg Memset Test:");
	endTimerAndPrint();
}

void clearMemSetTest()
{
	//printf("rsslClearMemSet Tests:\n");
	arrayClearMemSetTest();
	//printf("\n");
	vectorClearMemSetTest();
	//printf("\n");
	ackMsgClearMemSetTest();
	//printf("\n");
	updateMsgClearMemSetTest();
	//printf("\n");
	refreshMsgClearMemSetTest();
}

/* Used to defined scenarios for testing converting flag values to strings */
typedef struct
{
	RsslUInt32 flags;				/* Flag values to convert */
	RsslUInt32 outputBufferLength;	/* Length of output buffer to use */
	const char * flagString;		/* Expected output string from conversion call */
	RsslRet retCode;				/* Expected return code from conversion call */
} FlagToStringTest;

void ommStringTest()
{
	char tmpChar[255];
	RsslBuffer tmpBuf = {sizeof(tmpChar), tmpChar};
	int i;

	FlagToStringTest requestFlagTests[] =
	{
		{RSSL_RQMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_PRIORITY, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_PRIORITY.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_STREAMING, sizeof(tmpChar), RSSL_OMMSTR_RQMF_STREAMING.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_MSG_KEY_IN_UPDATES, sizeof(tmpChar), RSSL_OMMSTR_RQMF_MSG_KEY_IN_UPDATES.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_CONF_INFO_IN_UPDATES, sizeof(tmpChar), RSSL_OMMSTR_RQMF_CONF_INFO_IN_UPDATES.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_NO_REFRESH, sizeof(tmpChar), RSSL_OMMSTR_RQMF_NO_REFRESH.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_QOS, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_QOS.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_WORST_QOS, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_WORST_QOS.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_PRIVATE_STREAM, sizeof(tmpChar), RSSL_OMMSTR_RQMF_PRIVATE_STREAM.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_PAUSE, sizeof(tmpChar), RSSL_OMMSTR_RQMF_PAUSE.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_VIEW, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_VIEW.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_HAS_BATCH, sizeof(tmpChar), RSSL_OMMSTR_RQMF_HAS_BATCH.data, RSSL_RET_SUCCESS},
		{RSSL_RQMF_QUALIFIED_STREAM, sizeof(tmpChar), RSSL_OMMSTR_RQMF_QUALIFIED_STREAM.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_RQMF_HAS_EXTENDED_HEADER | RSSL_RQMF_HAS_PRIORITY | RSSL_RQMF_STREAMING | RSSL_RQMF_MSG_KEY_IN_UPDATES | RSSL_RQMF_CONF_INFO_IN_UPDATES | RSSL_RQMF_NO_REFRESH | RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS | RSSL_RQMF_PRIVATE_STREAM | RSSL_RQMF_PAUSE | RSSL_RQMF_HAS_VIEW | RSSL_RQMF_HAS_BATCH | RSSL_RQMF_QUALIFIED_STREAM,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPriority|Streaming|MsgKeyInUpdates|ConfInfoInUpdates|NoRefresh|HasQos|HasWorstQos|PrivateStream|Pause|HasView|HasBatch|QualifiedStream",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_RQMF_STREAMING, (RsslUInt32)strlen("Streaming") + 1, "Streaming", RSSL_RET_SUCCESS},
		{RSSL_RQMF_STREAMING | RSSL_RQMF_PAUSE, (RsslUInt32)strlen("Streaming|Pause") + 1, "Streaming|Pause", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_RQMF_STREAMING, (RsslUInt32)strlen("Streaming"), "Streaming", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_RQMF_STREAMING | RSSL_RQMF_PAUSE, (RsslUInt32)strlen("Streaming|Pause"), "Streaming|Pause", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest refreshFlagTests[] =
	{
		{RSSL_RFMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_SOLICITED, sizeof(tmpChar), RSSL_OMMSTR_RFMF_SOLICITED.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_REFRESH_COMPLETE, sizeof(tmpChar), RSSL_OMMSTR_RFMF_REFRESH_COMPLETE.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_QOS, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_QOS.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_CLEAR_CACHE, sizeof(tmpChar), RSSL_OMMSTR_RFMF_CLEAR_CACHE.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_DO_NOT_CACHE, sizeof(tmpChar), RSSL_OMMSTR_RFMF_DO_NOT_CACHE.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_PRIVATE_STREAM, sizeof(tmpChar), RSSL_OMMSTR_RFMF_PRIVATE_STREAM.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_POST_USER_INFO, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_POST_USER_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_PART_NUM, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_PART_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_HAS_REQ_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_RFMF_HAS_REQ_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_RFMF_QUALIFIED_STREAM, sizeof(tmpChar), RSSL_OMMSTR_RFMF_QUALIFIED_STREAM.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_RFMF_HAS_EXTENDED_HEADER | RSSL_RFMF_HAS_PERM_DATA | RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_HAS_SEQ_NUM | RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_HAS_QOS | RSSL_RFMF_CLEAR_CACHE | RSSL_RFMF_DO_NOT_CACHE | RSSL_RFMF_PRIVATE_STREAM | RSSL_RFMF_HAS_POST_USER_INFO | RSSL_RFMF_HAS_PART_NUM | RSSL_RFMF_HAS_REQ_MSG_KEY | RSSL_RFMF_QUALIFIED_STREAM,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPermData|HasMsgKey|HasSeqNum|Solicited|RefreshComplete|HasQos|ClearCache|DoNotCache|PrivateStream|HasPostUserInfo|HasPartNum|HasReqMsgKey|QualifiedStream",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_RFMF_SOLICITED, (RsslUInt32)strlen("Solicited") + 1, "Solicited", RSSL_RET_SUCCESS},
		{RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE, (RsslUInt32)strlen("Solicited|RefreshComplete") + 1, "Solicited|RefreshComplete", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_RFMF_SOLICITED, (RsslUInt32)strlen("Solicited"), "Solicited", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_RFMF_SOLICITED | RSSL_RFMF_REFRESH_COMPLETE, (RsslUInt32)strlen("Solicited|RefreshComplete"), "Solicited|RefreshComplete", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest statusFlagTests[] =
	{
		{RSSL_STMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_GROUP_ID, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_GROUP_ID.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_STATE, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_STATE.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_CLEAR_CACHE, sizeof(tmpChar), RSSL_OMMSTR_STMF_CLEAR_CACHE.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_PRIVATE_STREAM, sizeof(tmpChar), RSSL_OMMSTR_STMF_PRIVATE_STREAM.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_POST_USER_INFO, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_POST_USER_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_REQ_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_STMF_HAS_REQ_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_STMF_QUALIFIED_STREAM, sizeof(tmpChar), RSSL_OMMSTR_STMF_QUALIFIED_STREAM.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_STMF_HAS_EXTENDED_HEADER | RSSL_STMF_HAS_PERM_DATA | RSSL_STMF_HAS_MSG_KEY | RSSL_STMF_HAS_GROUP_ID | RSSL_STMF_HAS_STATE | RSSL_STMF_CLEAR_CACHE | RSSL_STMF_PRIVATE_STREAM | RSSL_STMF_HAS_POST_USER_INFO | RSSL_STMF_HAS_REQ_MSG_KEY | RSSL_STMF_QUALIFIED_STREAM,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPermData|HasMsgKey|HasGroupID|HasState|ClearCache|PrivateStream|HasPostUserInfo|HasReqMsgKey|QualifiedStream",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_STMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey") + 1, "HasMsgKey", RSSL_RET_SUCCESS},
		{RSSL_STMF_HAS_MSG_KEY | RSSL_STMF_HAS_STATE, (RsslUInt32)strlen("HasMsgKey|HasState") + 1, "HasMsgKey|HasState", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_STMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey"), "HasMsgKey", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_STMF_HAS_MSG_KEY | RSSL_STMF_HAS_STATE, (RsslUInt32)strlen("HasMsgKey|HasState"), "HasMsgKey|HasState", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest updateFlagTests[] =
	{
		{RSSL_UPMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_CONF_INFO, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_CONF_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_DO_NOT_CACHE, sizeof(tmpChar), RSSL_OMMSTR_UPMF_DO_NOT_CACHE.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_DO_NOT_CONFLATE, sizeof(tmpChar), RSSL_OMMSTR_UPMF_DO_NOT_CONFLATE.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_DO_NOT_RIPPLE, sizeof(tmpChar), RSSL_OMMSTR_UPMF_DO_NOT_RIPPLE.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_POST_USER_INFO, sizeof(tmpChar), RSSL_OMMSTR_UPMF_HAS_POST_USER_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_UPMF_DISCARDABLE, sizeof(tmpChar), RSSL_OMMSTR_UPMF_DISCARDABLE.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_UPMF_HAS_EXTENDED_HEADER|RSSL_UPMF_HAS_PERM_DATA|RSSL_UPMF_HAS_MSG_KEY|RSSL_UPMF_HAS_SEQ_NUM|RSSL_UPMF_HAS_CONF_INFO|RSSL_UPMF_DO_NOT_CACHE|RSSL_UPMF_DO_NOT_CONFLATE|RSSL_UPMF_DO_NOT_RIPPLE|RSSL_UPMF_HAS_POST_USER_INFO|RSSL_UPMF_DISCARDABLE,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPermData|HasMsgKey|HasSeqNum|HasConfInfo|DoNotCache|DoNotConflate|DoNotRipple|HasPostUserInfo|Discardable",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_UPMF_HAS_SEQ_NUM, (RsslUInt32)strlen("HasSeqNum") + 1, "HasSeqNum", RSSL_RET_SUCCESS},
		{RSSL_UPMF_HAS_SEQ_NUM | RSSL_UPMF_DO_NOT_CACHE, (RsslUInt32)strlen("HasSeqNum|DoNotCache") + 1, "HasSeqNum|DoNotCache", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_UPMF_HAS_SEQ_NUM, (RsslUInt32)strlen("HasSeqNum"), "HasSeqNum", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_UPMF_HAS_SEQ_NUM | RSSL_UPMF_DO_NOT_CACHE, (RsslUInt32)strlen("HasSeqNum|DoNotCache"), "HasSeqNum|DoNotCache", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest closeFlagTests[] =
	{
		{RSSL_CLMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_CLMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_CLMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_CLMF_ACK, sizeof(tmpChar), RSSL_OMMSTR_CLMF_ACK.data, RSSL_RET_SUCCESS},
		{RSSL_CLMF_HAS_BATCH, sizeof(tmpChar), RSSL_OMMSTR_CLMF_HAS_BATCH.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_CLMF_HAS_EXTENDED_HEADER | RSSL_CLMF_ACK | RSSL_CLMF_HAS_BATCH,
			sizeof(tmpChar),
			"HasExtendedHeader|Ack|HasBatch",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_CLMF_HAS_EXTENDED_HEADER, (RsslUInt32)strlen("HasExtendedHeader") + 1, "HasExtendedHeader", RSSL_RET_SUCCESS},
		{RSSL_CLMF_HAS_EXTENDED_HEADER | RSSL_CLMF_HAS_BATCH, (RsslUInt32)strlen("HasExtendedHeader|HasBatch") + 1, "HasExtendedHeader|HasBatch", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_CLMF_HAS_EXTENDED_HEADER, (RsslUInt32)strlen("HasExtendedHeader"), "HasExtendedHeader", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_CLMF_HAS_EXTENDED_HEADER | RSSL_CLMF_HAS_BATCH, (RsslUInt32)strlen("HasExtendedHeader|HasBatch"), "HasExtendedHeader|HasBatch", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest ackFlagTests[] =
	{
		{RSSL_AKMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_AKMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_TEXT, sizeof(tmpChar), RSSL_OMMSTR_AKMF_HAS_TEXT.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_PRIVATE_STREAM, sizeof(tmpChar), RSSL_OMMSTR_AKMF_PRIVATE_STREAM.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_AKMF_HAS_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_AKMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_NAK_CODE, sizeof(tmpChar), RSSL_OMMSTR_AKMF_HAS_NAK_CODE.data, RSSL_RET_SUCCESS},
		{RSSL_AKMF_QUALIFIED_STREAM, sizeof(tmpChar), RSSL_OMMSTR_AKMF_QUALIFIED_STREAM.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_AKMF_HAS_EXTENDED_HEADER | RSSL_AKMF_HAS_TEXT | RSSL_AKMF_PRIVATE_STREAM | RSSL_AKMF_HAS_SEQ_NUM | RSSL_AKMF_HAS_MSG_KEY | RSSL_AKMF_HAS_NAK_CODE | RSSL_AKMF_QUALIFIED_STREAM,
			sizeof(tmpChar),
			"HasExtendedHeader|HasText|PrivateStream|HasSeqNum|HasMsgKey|HasNakCode|QualifiedStream",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_AKMF_HAS_TEXT, (RsslUInt32)strlen("HasText") + 1, "HasText", RSSL_RET_SUCCESS},
		{RSSL_AKMF_HAS_TEXT | RSSL_AKMF_HAS_NAK_CODE, (RsslUInt32)strlen("HasText|HasNakCode") + 1, "HasText|HasNakCode", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_AKMF_HAS_TEXT, (RsslUInt32)strlen("HasText"), "HasText", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_AKMF_HAS_TEXT | RSSL_AKMF_HAS_NAK_CODE, (RsslUInt32)strlen("HasText|HasNakCode"), "HasText|HasNakCode", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest genericFlagTests[] =
	{
		{RSSL_GNMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_MESSAGE_COMPLETE, sizeof(tmpChar), RSSL_OMMSTR_GNMF_MESSAGE_COMPLETE.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_SECONDARY_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_SECONDARY_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_PART_NUM, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_PART_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_REQ_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_GNMF_HAS_REQ_MSG_KEY.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_GNMF_HAS_EXTENDED_HEADER | RSSL_GNMF_HAS_PERM_DATA | RSSL_GNMF_HAS_MSG_KEY | RSSL_GNMF_HAS_SEQ_NUM | RSSL_GNMF_MESSAGE_COMPLETE | RSSL_GNMF_HAS_SECONDARY_SEQ_NUM | RSSL_GNMF_HAS_PART_NUM | RSSL_GNMF_HAS_REQ_MSG_KEY,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPermData|HasMsgKey|HasSeqNum|MessageComplete|HasSecondarySeqNum|HasPartNum|HasReqMsgKey",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_GNMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey") + 1, "HasMsgKey", RSSL_RET_SUCCESS},
		{RSSL_GNMF_HAS_MSG_KEY | RSSL_GNMF_MESSAGE_COMPLETE, (RsslUInt32)strlen("HasMsgKey|MessageComplete") + 1, "HasMsgKey|MessageComplete", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_GNMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey"), "HasMsgKey", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_GNMF_HAS_MSG_KEY | RSSL_GNMF_MESSAGE_COMPLETE, (RsslUInt32)strlen("HasMsgKey|MessageComplete"), "HasMsgKey|MessageComplete", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest postFlagTests[] =
	{
		{RSSL_PSMF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_EXTENDED_HEADER, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_EXTENDED_HEADER.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_POST_ID, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_POST_ID.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_MSG_KEY, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_MSG_KEY.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_SEQ_NUM, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_SEQ_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_POST_COMPLETE, sizeof(tmpChar), RSSL_OMMSTR_PSMF_POST_COMPLETE.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_ACK, sizeof(tmpChar), RSSL_OMMSTR_PSMF_ACK.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_PART_NUM, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_PART_NUM.data, RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_POST_USER_RIGHTS, sizeof(tmpChar), RSSL_OMMSTR_PSMF_HAS_POST_USER_RIGHTS.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_PSMF_HAS_EXTENDED_HEADER | RSSL_PSMF_HAS_POST_ID | RSSL_PSMF_HAS_MSG_KEY | RSSL_PSMF_HAS_SEQ_NUM | RSSL_PSMF_POST_COMPLETE | RSSL_PSMF_ACK | RSSL_PSMF_HAS_PERM_DATA | RSSL_PSMF_HAS_PART_NUM | RSSL_PSMF_HAS_POST_USER_RIGHTS,
			sizeof(tmpChar),
			"HasExtendedHeader|HasPostID|HasMsgKey|HasSeqNum|PostComplete|Ack|HasPermData|HasPartNum|HasPostUserRights",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_PSMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey") + 1, "HasMsgKey", RSSL_RET_SUCCESS},
		{RSSL_PSMF_HAS_MSG_KEY | RSSL_PSMF_POST_COMPLETE, (RsslUInt32)strlen("HasMsgKey|PostComplete") + 1, "HasMsgKey|PostComplete", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_PSMF_HAS_MSG_KEY, (RsslUInt32)strlen("HasMsgKey"), "HasMsgKey", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_PSMF_HAS_MSG_KEY | RSSL_PSMF_POST_COMPLETE, (RsslUInt32)strlen("HasMsgKey|PostComplete"), "HasMsgKey|PostComplete", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest postUserRightsTests[] =
	{
		{RSSL_PSUR_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_PSUR_CREATE, sizeof(tmpChar), RSSL_OMMSTR_PSUR_CREATE.data, RSSL_RET_SUCCESS},
		{RSSL_PSUR_DELETE, sizeof(tmpChar), RSSL_OMMSTR_PSUR_DELETE.data, RSSL_RET_SUCCESS},
		{RSSL_PSUR_MODIFY_PERM, sizeof(tmpChar), RSSL_OMMSTR_PSUR_MODIFY_PERM.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_PSUR_CREATE| RSSL_PSUR_DELETE| RSSL_PSUR_MODIFY_PERM,
			sizeof(tmpChar),
			"Create|Delete|ModifyPerm",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_PSUR_CREATE, (RsslUInt32)strlen("Create") + 1, "Create", RSSL_RET_SUCCESS},
		{RSSL_PSUR_CREATE | RSSL_PSUR_MODIFY_PERM, (RsslUInt32)strlen("Create|ModifyPerm") + 1, "Create|ModifyPerm", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_PSUR_CREATE, (RsslUInt32)strlen("Create"), "Create", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_PSUR_CREATE | RSSL_PSUR_MODIFY_PERM, (RsslUInt32)strlen("Create|ModifyPerm"), "Create|ModifyPerm", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest msgKeyFlagTests[] =
	{
		{RSSL_MKF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_SERVICE_ID, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_SERVICE_ID.data, RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_NAME, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_NAME.data, RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_NAME_TYPE, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_NAME_TYPE.data, RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_FILTER, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_FILTER.data, RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_IDENTIFIER, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_IDENTIFIER.data, RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_ATTRIB, sizeof(tmpChar), RSSL_OMMSTR_MKF_HAS_ATTRIB.data, RSSL_RET_SUCCESS},


		/* All flags test */
		{
			RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_IDENTIFIER | RSSL_MKF_HAS_ATTRIB,
			sizeof(tmpChar),
			"HasServiceID|HasName|HasNameType|HasFilter|HasIdentifier|HasAttrib",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_MKF_HAS_NAME, (RsslUInt32)strlen("HasName") + 1, "HasName", RSSL_RET_SUCCESS},
		{RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE, (RsslUInt32)strlen("HasName|HasNameType") + 1, "HasName|HasNameType", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_MKF_HAS_NAME, (RsslUInt32)strlen("HasName"), "HasName", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE, (RsslUInt32)strlen("HasName|HasNameType"), "HasName|HasNameType", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest mapFlagTests[] =
	{
		{RSSL_MPF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_SET_DEFS, sizeof(tmpChar), RSSL_OMMSTR_MPF_HAS_SET_DEFS.data, RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_SUMMARY_DATA, sizeof(tmpChar), RSSL_OMMSTR_MPF_HAS_SUMMARY_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_PER_ENTRY_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_MPF_HAS_PER_ENTRY_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_TOTAL_COUNT_HINT, sizeof(tmpChar), RSSL_OMMSTR_MPF_HAS_TOTAL_COUNT_HINT.data, RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_KEY_FIELD_ID, sizeof(tmpChar), RSSL_OMMSTR_MPF_HAS_KEY_FIELD_ID.data, RSSL_RET_SUCCESS},


		/* All flags test */
		{
			RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_PER_ENTRY_PERM_DATA | RSSL_MPF_HAS_TOTAL_COUNT_HINT | RSSL_MPF_HAS_KEY_FIELD_ID,
			sizeof(tmpChar),
			"HasSetDefs|HasSummaryData|HasPerEntryPermData|HasTotalCountHint|HasKeyFieldID",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_MPF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs") + 1, "HasSetDefs", RSSL_RET_SUCCESS},
		{RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData") + 1, "HasSetDefs|HasSummaryData", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_MPF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs"), "HasSetDefs", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_MPF_HAS_SET_DEFS | RSSL_MPF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData"), "HasSetDefs|HasSummaryData", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest mapEntryFlagTests[] =
	{
		{RSSL_MPEF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_MPEF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_MPEF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_MPEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData") + 1, "HasPermData", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_MPEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData"), "HasPermData", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest vectorFlagTests[] =
	{

		{RSSL_VTF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_VTF_HAS_SET_DEFS, sizeof(tmpChar), RSSL_OMMSTR_VTF_HAS_SET_DEFS.data, RSSL_RET_SUCCESS},
		{RSSL_VTF_HAS_SUMMARY_DATA, sizeof(tmpChar), RSSL_OMMSTR_VTF_HAS_SUMMARY_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_VTF_HAS_PER_ENTRY_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_VTF_HAS_PER_ENTRY_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_VTF_HAS_TOTAL_COUNT_HINT, sizeof(tmpChar), RSSL_OMMSTR_VTF_HAS_TOTAL_COUNT_HINT.data, RSSL_RET_SUCCESS},
		{RSSL_VTF_SUPPORTS_SORTING, sizeof(tmpChar), RSSL_OMMSTR_VTF_SUPPORTS_SORTING.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_VTF_HAS_SET_DEFS | RSSL_VTF_HAS_SUMMARY_DATA | RSSL_VTF_HAS_PER_ENTRY_PERM_DATA | RSSL_VTF_HAS_TOTAL_COUNT_HINT | RSSL_VTF_SUPPORTS_SORTING,
			sizeof(tmpChar),
			"HasSetDefs|HasSummaryData|HasPerEntryPermData|HasTotalCountHint|SupportsSorting",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_VTF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs") + 1, "HasSetDefs", RSSL_RET_SUCCESS},
		{RSSL_VTF_HAS_SET_DEFS | RSSL_VTF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData") + 1, "HasSetDefs|HasSummaryData", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_VTF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs"), "HasSetDefs", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_VTF_HAS_SET_DEFS | RSSL_VTF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData"), "HasSetDefs|HasSummaryData", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest vectorEntryFlagTests[] =
	{
		{RSSL_VTEF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_VTEF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_VTEF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_VTEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData") + 1, "HasPermData", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_VTEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData"), "HasPermData", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest seriesFlagTests[] =
	{
		{RSSL_SRF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_SRF_HAS_SET_DEFS, sizeof(tmpChar), RSSL_OMMSTR_SRF_HAS_SET_DEFS.data, RSSL_RET_SUCCESS},
		{RSSL_SRF_HAS_SUMMARY_DATA, sizeof(tmpChar), RSSL_OMMSTR_SRF_HAS_SUMMARY_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_SRF_HAS_TOTAL_COUNT_HINT, sizeof(tmpChar), RSSL_OMMSTR_SRF_HAS_TOTAL_COUNT_HINT.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_SRF_HAS_SET_DEFS | RSSL_SRF_HAS_SUMMARY_DATA | RSSL_SRF_HAS_TOTAL_COUNT_HINT,
			sizeof(tmpChar),
			"HasSetDefs|HasSummaryData|HasTotalCountHint",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_SRF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs") + 1, "HasSetDefs", RSSL_RET_SUCCESS},
		{RSSL_SRF_HAS_SET_DEFS | RSSL_SRF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData") + 1, "HasSetDefs|HasSummaryData", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_SRF_HAS_SET_DEFS, (RsslUInt32)strlen("HasSetDefs"), "HasSetDefs", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_SRF_HAS_SET_DEFS | RSSL_SRF_HAS_SUMMARY_DATA, (RsslUInt32)strlen("HasSetDefs|HasSummaryData"), "HasSetDefs|HasSummaryData", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest filterListFlagTests[] =
	{
		{RSSL_FTF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_FTF_HAS_PER_ENTRY_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_FTF_HAS_PER_ENTRY_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_FTF_HAS_TOTAL_COUNT_HINT, sizeof(tmpChar), RSSL_OMMSTR_FTF_HAS_TOTAL_COUNT_HINT.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_FTF_HAS_PER_ENTRY_PERM_DATA | RSSL_FTF_HAS_TOTAL_COUNT_HINT,
			sizeof(tmpChar),
			"HasPerEntryPermData|HasTotalCountHint",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_FTF_HAS_PER_ENTRY_PERM_DATA, (RsslUInt32)strlen("HasPerEntryPermData") + 1, "HasPerEntryPermData", RSSL_RET_SUCCESS},
		{RSSL_FTF_HAS_PER_ENTRY_PERM_DATA | RSSL_FTF_HAS_TOTAL_COUNT_HINT, (RsslUInt32)strlen("HasPerEntryPermData|HasTotalCountHint") + 1, "HasPerEntryPermData|HasTotalCountHint", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_FTF_HAS_PER_ENTRY_PERM_DATA, (RsslUInt32)strlen("HasPerEntryPermData"), "HasPerEntryPermData", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_FTF_HAS_PER_ENTRY_PERM_DATA | RSSL_FTF_HAS_TOTAL_COUNT_HINT, (RsslUInt32)strlen("HasPerEntryPermData|HasTotalCountHint"), "HasPerEntryPermData|HasTotalCountHint", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest filterEntryFlagTests[] =
	{
		{RSSL_FTEF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_FTEF_HAS_PERM_DATA, sizeof(tmpChar), RSSL_OMMSTR_FTEF_HAS_PERM_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_FTEF_HAS_CONTAINER_TYPE, sizeof(tmpChar), RSSL_OMMSTR_FTEF_HAS_CONTAINER_TYPE.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_FTEF_HAS_PERM_DATA | RSSL_FTEF_HAS_CONTAINER_TYPE,
			sizeof(tmpChar),
			"HasPermData|HasContainerType",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_FTEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData") + 1, "HasPermData", RSSL_RET_SUCCESS},
		{RSSL_FTEF_HAS_PERM_DATA | RSSL_FTEF_HAS_CONTAINER_TYPE, (RsslUInt32)strlen("HasPermData|HasContainerType") + 1, "HasPermData|HasContainerType", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_FTEF_HAS_PERM_DATA, (RsslUInt32)strlen("HasPermData"), "HasPermData", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_FTEF_HAS_PERM_DATA | RSSL_FTEF_HAS_CONTAINER_TYPE, (RsslUInt32)strlen("HasPermData|HasContainerType"), "HasPermData|HasContainerType", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest fieldListFlagTests[] =
	{
		{RSSL_FLF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_FLF_HAS_FIELD_LIST_INFO, sizeof(tmpChar), RSSL_OMMSTR_FLF_HAS_FIELD_LIST_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_FLF_HAS_SET_DATA, sizeof(tmpChar), RSSL_OMMSTR_FLF_HAS_SET_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_FLF_HAS_SET_ID, sizeof(tmpChar), RSSL_OMMSTR_FLF_HAS_SET_ID.data, RSSL_RET_SUCCESS},
		{RSSL_FLF_HAS_STANDARD_DATA, sizeof(tmpChar), RSSL_OMMSTR_FLF_HAS_STANDARD_DATA.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_FLF_HAS_FIELD_LIST_INFO| RSSL_FLF_HAS_SET_DATA| RSSL_FLF_HAS_SET_ID| RSSL_FLF_HAS_STANDARD_DATA,
			sizeof(tmpChar),
			"HasFieldListInfo|HasSetData|HasSetID|HasStandardData",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_FLF_HAS_SET_DATA, (RsslUInt32)strlen("HasSetData") + 1, "HasSetData", RSSL_RET_SUCCESS},
		{RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID, (RsslUInt32)strlen("HasSetData|HasSetID") + 1, "HasSetData|HasSetID", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_FLF_HAS_SET_DATA, (RsslUInt32)strlen("HasSetData"), "HasSetData", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID, (RsslUInt32)strlen("HasSetData|HasSetID"), "HasSetData|HasSetID", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest elementListFlagTests[] =
	{
		{RSSL_ELF_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RSSL_ELF_HAS_ELEMENT_LIST_INFO, sizeof(tmpChar), RSSL_OMMSTR_ELF_HAS_ELEMENT_LIST_INFO.data, RSSL_RET_SUCCESS},
		{RSSL_ELF_HAS_SET_DATA, sizeof(tmpChar), RSSL_OMMSTR_ELF_HAS_SET_DATA.data, RSSL_RET_SUCCESS},
		{RSSL_ELF_HAS_SET_ID, sizeof(tmpChar), RSSL_OMMSTR_ELF_HAS_SET_ID.data, RSSL_RET_SUCCESS},
		{RSSL_ELF_HAS_STANDARD_DATA, sizeof(tmpChar), RSSL_OMMSTR_ELF_HAS_STANDARD_DATA.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RSSL_ELF_HAS_ELEMENT_LIST_INFO| RSSL_ELF_HAS_SET_DATA| RSSL_ELF_HAS_SET_ID| RSSL_ELF_HAS_STANDARD_DATA,
			sizeof(tmpChar),
			"HasElementListInfo|HasSetData|HasSetID|HasStandardData",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RSSL_ELF_HAS_SET_DATA, (RsslUInt32)strlen("HasSetData") + 1, "HasSetData", RSSL_RET_SUCCESS},
		{RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID, (RsslUInt32)strlen("HasSetData|HasSetID") + 1, "HasSetData|HasSetID", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RSSL_ELF_HAS_SET_DATA, (RsslUInt32)strlen("HasSetData"), "HasSetData", RSSL_RET_BUFFER_TOO_SMALL},
		{RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID, (RsslUInt32)strlen("HasSetData|HasSetID"), "HasSetData|HasSetID", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest loginBatchSupportFlagTests[] =
	{
		{RDM_LOGIN_BATCH_NONE, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RDM_LOGIN_BATCH_SUPPORT_REQUESTS, sizeof(tmpChar), RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REQUESTS.data, RSSL_RET_SUCCESS},
		{RDM_LOGIN_BATCH_SUPPORT_REISSUES, sizeof(tmpChar), RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REISSUES.data, RSSL_RET_SUCCESS},
		{RDM_LOGIN_BATCH_SUPPORT_CLOSES, sizeof(tmpChar), RDM_OMMSTR_LOGIN_BATCH_SUPPORT_CLOSES.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_REISSUES | RDM_LOGIN_BATCH_SUPPORT_CLOSES,
			sizeof(tmpChar),
			"Requests|Reissues|Closes",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RDM_LOGIN_BATCH_SUPPORT_REQUESTS, (RsslUInt32)strlen("Requests") + 1, "Requests", RSSL_RET_SUCCESS},
		{RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_CLOSES, (RsslUInt32)strlen("Requests|Closes") + 1, "Requests|Closes", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RDM_LOGIN_BATCH_SUPPORT_REQUESTS, (RsslUInt32)strlen("Requests"), "Requests", RSSL_RET_BUFFER_TOO_SMALL},
		{RDM_LOGIN_BATCH_SUPPORT_REQUESTS | RDM_LOGIN_BATCH_SUPPORT_CLOSES, (RsslUInt32)strlen("Requests|Closes"), "Requests|Closes", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest serviceFilterFlagTests[] =
	{
		{0 /* No "None" value exists for service filter flags. */, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_INFO_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_INFO_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_STATE_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_STATE_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_GROUP_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_LOAD_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_DATA_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_DATA_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_LINK_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_LINK_FILTER.data, RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER, sizeof(tmpChar), RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_FILTER.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER | RDM_DIRECTORY_SERVICE_GROUP_FILTER | RDM_DIRECTORY_SERVICE_LOAD_FILTER | RDM_DIRECTORY_SERVICE_DATA_FILTER | RDM_DIRECTORY_SERVICE_LINK_FILTER | RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER,
			sizeof(tmpChar),
			"Info|State|Group|Load|Data|Link|SeqMcast",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RDM_DIRECTORY_SERVICE_INFO_FILTER, (RsslUInt32)strlen("Info") + 1, "Info", RSSL_RET_SUCCESS},
		{RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER, (RsslUInt32)strlen("Info|State") + 1, "Info|State", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RDM_DIRECTORY_SERVICE_INFO_FILTER, (RsslUInt32)strlen("Info"), "Info", RSSL_RET_BUFFER_TOO_SMALL},
		{RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER, (RsslUInt32)strlen("Info|State"), "Info|State", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest symbolListSupportFlagTests[] =
	{
		{RDM_SYMBOL_LIST_SUPPORT_NAMES_ONLY, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS, sizeof(tmpChar), RDM_OMMSTR_SYMBOL_LIST_SUPPORT_DATA_STREAMS.data, RSSL_RET_SUCCESS},

		/* Exact size tests (string plus null-terminator) */
		{RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS, (RsslUInt32)strlen("DataStreams") + 1, "DataStreams", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS, (RsslUInt32)strlen("DataStreams"), "DataStreams", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest symbolListDataStreamRequestFlagTests[] =
	{
		{RDM_SYMBOL_LIST_NAMES_ONLY, sizeof(tmpChar), "", RSSL_RET_SUCCESS},
		{RDM_SYMBOL_LIST_DATA_STREAMS, sizeof(tmpChar), RDM_OMMSTR_SYMBOL_LIST_DATA_STREAMS.data, RSSL_RET_SUCCESS},
		{RDM_SYMBOL_LIST_DATA_SNAPSHOTS, sizeof(tmpChar), RDM_OMMSTR_SYMBOL_LIST_DATA_SNAPSHOTS.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS,
			sizeof(tmpChar),
			"DataStreams|DataSnapshots",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RDM_SYMBOL_LIST_DATA_STREAMS, (RsslUInt32)strlen("DataStreams") + 1, "DataStreams", RSSL_RET_SUCCESS},
		{RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS, (RsslUInt32)strlen("DataStreams|DataSnapshots") + 1, "DataStreams|DataSnapshots", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RDM_SYMBOL_LIST_DATA_STREAMS, (RsslUInt32)strlen("DataStreams"), "DataStreams", RSSL_RET_BUFFER_TOO_SMALL},
		{RDM_SYMBOL_LIST_DATA_STREAMS | RDM_SYMBOL_LIST_DATA_SNAPSHOTS, (RsslUInt32)strlen("DataStreams|DataSnapshots"), "DataStreams|DataSnapshots", RSSL_RET_BUFFER_TOO_SMALL},
	};

	FlagToStringTest cosFilterFlagTests[] =
	{
		{RDM_COS_COMMON_PROPERTIES_FLAG, sizeof(tmpChar), RDM_OMMSTR_COS_COMMON_PROPERTIES_FLAG.data, RSSL_RET_SUCCESS},
		{RDM_COS_AUTHENTICATION_FLAG, sizeof(tmpChar), RDM_OMMSTR_COS_AUTHENTICATION_FLAG.data, RSSL_RET_SUCCESS},
		{RDM_COS_FLOW_CONTROL_FLAG, sizeof(tmpChar), RDM_OMMSTR_COS_FLOW_CONTROL_FLAG.data, RSSL_RET_SUCCESS},
		{RDM_COS_DATA_INTEGRITY_FLAG, sizeof(tmpChar), RDM_OMMSTR_COS_DATA_INTEGRITY_FLAG.data, RSSL_RET_SUCCESS},
		{RDM_COS_GUARANTEE_FLAG, sizeof(tmpChar), RDM_OMMSTR_COS_GUARANTEE_FLAG.data, RSSL_RET_SUCCESS},

		/* All flags test */
		{
			RDM_COS_COMMON_PROPERTIES_FLAG | RDM_COS_AUTHENTICATION_FLAG | RDM_COS_FLOW_CONTROL_FLAG | RDM_COS_DATA_INTEGRITY_FLAG | RDM_COS_GUARANTEE_FLAG,
			sizeof(tmpChar),
			"CommonProperties|Authentication|FlowControl|DataIntegrity|Guarantee",
			RSSL_RET_SUCCESS
		},

		/* Exact size tests (string plus null-terminator) */
		{RDM_COS_COMMON_PROPERTIES_FLAG, (RsslUInt32)strlen("CommonProperties") + 1, "CommonProperties", RSSL_RET_SUCCESS},
		{RDM_COS_COMMON_PROPERTIES_FLAG | RDM_COS_AUTHENTICATION_FLAG, (RsslUInt32)strlen("CommonProperties|Authentication") + 1, "CommonProperties|Authentication", RSSL_RET_SUCCESS},

		/* Buffer too small tests */
		{RDM_COS_COMMON_PROPERTIES_FLAG, (RsslUInt32)strlen("CommonProperties"), "CommonProperties", RSSL_RET_BUFFER_TOO_SMALL},
		{RDM_COS_COMMON_PROPERTIES_FLAG | RDM_COS_AUTHENTICATION_FLAG, (RsslUInt32)strlen("CommonProperties|Authentication"), "CommonProperties|Authentication", RSSL_RET_BUFFER_TOO_SMALL},
	};

	/* Test general OMM string functions. */
	//printf("OMM String Tests:\n");

	/* RequestFlags */
	for (i = 0; i < sizeof(requestFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = requestFlagTests[i].outputBufferLength;

		if (requestFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRequestFlagsToOmmString(&tmpBuf, requestFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == requestFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", requestFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRequestFlagsToOmmString(&tmpBuf, requestFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(requestFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, requestFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", requestFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(requestFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_RQMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_PRIORITY.data) == RSSL_OMMSTR_RQMF_HAS_PRIORITY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_STREAMING.data) == RSSL_OMMSTR_RQMF_STREAMING.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_MSG_KEY_IN_UPDATES.data) == RSSL_OMMSTR_RQMF_MSG_KEY_IN_UPDATES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_CONF_INFO_IN_UPDATES.data) == RSSL_OMMSTR_RQMF_CONF_INFO_IN_UPDATES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_NO_REFRESH.data) == RSSL_OMMSTR_RQMF_NO_REFRESH.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_QOS.data) == RSSL_OMMSTR_RQMF_HAS_QOS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_WORST_QOS.data) == RSSL_OMMSTR_RQMF_HAS_WORST_QOS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_PRIVATE_STREAM.data) == RSSL_OMMSTR_RQMF_PRIVATE_STREAM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_PAUSE.data) == RSSL_OMMSTR_RQMF_PAUSE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_VIEW.data) == RSSL_OMMSTR_RQMF_HAS_VIEW.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_HAS_BATCH.data) == RSSL_OMMSTR_RQMF_HAS_BATCH.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RQMF_QUALIFIED_STREAM.data) == RSSL_OMMSTR_RQMF_QUALIFIED_STREAM.length);

	/* RefreshFlags */
	for (i = 0; i < sizeof(refreshFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = refreshFlagTests[i].outputBufferLength;

		if (refreshFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRefreshFlagsToOmmString(&tmpBuf, refreshFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == refreshFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", refreshFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRefreshFlagsToOmmString(&tmpBuf, refreshFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(refreshFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, refreshFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", refreshFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(refreshFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_RFMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_PERM_DATA.data) == RSSL_OMMSTR_RFMF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_RFMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_SEQ_NUM.data) == RSSL_OMMSTR_RFMF_HAS_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_SOLICITED.data) == RSSL_OMMSTR_RFMF_SOLICITED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_REFRESH_COMPLETE.data) == RSSL_OMMSTR_RFMF_REFRESH_COMPLETE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_QOS.data) == RSSL_OMMSTR_RFMF_HAS_QOS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_CLEAR_CACHE.data) == RSSL_OMMSTR_RFMF_CLEAR_CACHE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_DO_NOT_CACHE.data) == RSSL_OMMSTR_RFMF_DO_NOT_CACHE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_PRIVATE_STREAM.data) == RSSL_OMMSTR_RFMF_PRIVATE_STREAM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_POST_USER_INFO.data) == RSSL_OMMSTR_RFMF_HAS_POST_USER_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_PART_NUM.data) == RSSL_OMMSTR_RFMF_HAS_PART_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_HAS_REQ_MSG_KEY.data) == RSSL_OMMSTR_RFMF_HAS_REQ_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RFMF_QUALIFIED_STREAM.data) == RSSL_OMMSTR_RFMF_QUALIFIED_STREAM.length);

	/* StatusFlags */
	for (i = 0; i < sizeof(statusFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = statusFlagTests[i].outputBufferLength;

		if (statusFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslStatusFlagsToOmmString(&tmpBuf, statusFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == statusFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", statusFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslStatusFlagsToOmmString(&tmpBuf, statusFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(statusFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, statusFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", statusFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(statusFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_STMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_PERM_DATA.data) == RSSL_OMMSTR_STMF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_STMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_GROUP_ID.data) == RSSL_OMMSTR_STMF_HAS_GROUP_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_STATE.data) == RSSL_OMMSTR_STMF_HAS_STATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_CLEAR_CACHE.data) == RSSL_OMMSTR_STMF_CLEAR_CACHE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_PRIVATE_STREAM.data) == RSSL_OMMSTR_STMF_PRIVATE_STREAM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_POST_USER_INFO.data) == RSSL_OMMSTR_STMF_HAS_POST_USER_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_HAS_REQ_MSG_KEY.data) == RSSL_OMMSTR_STMF_HAS_REQ_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STMF_QUALIFIED_STREAM.data) == RSSL_OMMSTR_STMF_QUALIFIED_STREAM.length);

	/* UpdateFlags */
	for (i = 0; i < sizeof(updateFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = updateFlagTests[i].outputBufferLength;

		if (updateFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslUpdateFlagsToOmmString(&tmpBuf, updateFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == updateFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", updateFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslUpdateFlagsToOmmString(&tmpBuf, updateFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(updateFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, updateFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", updateFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(updateFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_UPMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_PERM_DATA.data) == RSSL_OMMSTR_UPMF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_UPMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_SEQ_NUM.data) == RSSL_OMMSTR_UPMF_HAS_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_CONF_INFO.data) == RSSL_OMMSTR_UPMF_HAS_CONF_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_DO_NOT_CACHE.data) == RSSL_OMMSTR_UPMF_DO_NOT_CACHE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_DO_NOT_CONFLATE.data) == RSSL_OMMSTR_UPMF_DO_NOT_CONFLATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_DO_NOT_RIPPLE.data) == RSSL_OMMSTR_UPMF_DO_NOT_RIPPLE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_HAS_POST_USER_INFO.data) == RSSL_OMMSTR_UPMF_HAS_POST_USER_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_UPMF_DISCARDABLE.data) == RSSL_OMMSTR_UPMF_DISCARDABLE.length);

	/* CloseFlags */
	for (i = 0; i < sizeof(closeFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = closeFlagTests[i].outputBufferLength;

		if (closeFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslCloseFlagsToOmmString(&tmpBuf, closeFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == closeFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", closeFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslCloseFlagsToOmmString(&tmpBuf, closeFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(closeFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, closeFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", closeFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(closeFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_CLMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_CLMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_CLMF_ACK.data) == RSSL_OMMSTR_CLMF_ACK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_CLMF_HAS_BATCH.data) == RSSL_OMMSTR_CLMF_HAS_BATCH.length);

	/* AckFlags */
	for (i = 0; i < sizeof(ackFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = ackFlagTests[i].outputBufferLength;

		if (ackFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslAckFlagsToOmmString(&tmpBuf, ackFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == ackFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", ackFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslAckFlagsToOmmString(&tmpBuf, ackFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(ackFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, ackFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", ackFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(ackFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_AKMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_HAS_TEXT.data) == RSSL_OMMSTR_AKMF_HAS_TEXT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_PRIVATE_STREAM.data) == RSSL_OMMSTR_AKMF_PRIVATE_STREAM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_HAS_SEQ_NUM.data) == RSSL_OMMSTR_AKMF_HAS_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_AKMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_HAS_NAK_CODE.data) == RSSL_OMMSTR_AKMF_HAS_NAK_CODE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_AKMF_QUALIFIED_STREAM.data) == RSSL_OMMSTR_AKMF_QUALIFIED_STREAM.length);

	/* GenericFlags */
	for (i = 0; i < sizeof(genericFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = genericFlagTests[i].outputBufferLength;

		if (genericFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslGenericFlagsToOmmString(&tmpBuf, genericFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == genericFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", genericFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslGenericFlagsToOmmString(&tmpBuf, genericFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(genericFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, genericFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", genericFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(genericFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_GNMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_PERM_DATA.data) == RSSL_OMMSTR_GNMF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_GNMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_SEQ_NUM.data) == RSSL_OMMSTR_GNMF_HAS_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_MESSAGE_COMPLETE.data) == RSSL_OMMSTR_GNMF_MESSAGE_COMPLETE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_SECONDARY_SEQ_NUM.data) == RSSL_OMMSTR_GNMF_HAS_SECONDARY_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_PART_NUM.data) == RSSL_OMMSTR_GNMF_HAS_PART_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_GNMF_HAS_REQ_MSG_KEY.data) == RSSL_OMMSTR_GNMF_HAS_REQ_MSG_KEY.length);


	/* PostFlags */
	for (i = 0; i < sizeof(postFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = postFlagTests[i].outputBufferLength;

		if (postFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslPostFlagsToOmmString(&tmpBuf, postFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == postFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", postFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslPostFlagsToOmmString(&tmpBuf, postFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(postFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, postFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", postFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(postFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_EXTENDED_HEADER.data) == RSSL_OMMSTR_PSMF_HAS_EXTENDED_HEADER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_POST_ID.data) == RSSL_OMMSTR_PSMF_HAS_POST_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_MSG_KEY.data) == RSSL_OMMSTR_PSMF_HAS_MSG_KEY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_SEQ_NUM.data) == RSSL_OMMSTR_PSMF_HAS_SEQ_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_POST_COMPLETE.data) == RSSL_OMMSTR_PSMF_POST_COMPLETE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_ACK.data) == RSSL_OMMSTR_PSMF_ACK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_PERM_DATA.data) == RSSL_OMMSTR_PSMF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_PART_NUM.data) == RSSL_OMMSTR_PSMF_HAS_PART_NUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSMF_HAS_POST_USER_RIGHTS.data) == RSSL_OMMSTR_PSMF_HAS_POST_USER_RIGHTS.length);

	/* PostUserRights */
	for (i = 0; i < sizeof(postUserRightsTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = postUserRightsTests[i].outputBufferLength;

		if (postUserRightsTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslPostUserRightsToOmmString(&tmpBuf, postUserRightsTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == postUserRightsTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", postUserRightsTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslPostUserRightsToOmmString(&tmpBuf, postUserRightsTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(postUserRightsTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, postUserRightsTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", postUserRightsTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(postUserRightsTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSUR_CREATE.data) == RSSL_OMMSTR_PSUR_CREATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSUR_DELETE.data) == RSSL_OMMSTR_PSUR_DELETE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_PSUR_MODIFY_PERM.data) == RSSL_OMMSTR_PSUR_MODIFY_PERM.length);

	/* MsgKeyFlags */
	for (i = 0; i < sizeof(msgKeyFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = msgKeyFlagTests[i].outputBufferLength;

		if (msgKeyFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslMsgKeyFlagsToOmmString(&tmpBuf, msgKeyFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == msgKeyFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", msgKeyFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslMsgKeyFlagsToOmmString(&tmpBuf, msgKeyFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(msgKeyFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, msgKeyFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", msgKeyFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(msgKeyFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_SERVICE_ID.data) == RSSL_OMMSTR_MKF_HAS_SERVICE_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_NAME.data) == RSSL_OMMSTR_MKF_HAS_NAME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_NAME_TYPE.data) == RSSL_OMMSTR_MKF_HAS_NAME_TYPE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_FILTER.data) == RSSL_OMMSTR_MKF_HAS_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_IDENTIFIER.data) == RSSL_OMMSTR_MKF_HAS_IDENTIFIER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MKF_HAS_ATTRIB.data) == RSSL_OMMSTR_MKF_HAS_ATTRIB.length);

	/* MapFlags */
	for (i = 0; i < sizeof(mapFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = mapFlagTests[i].outputBufferLength;

		if (mapFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslMapFlagsToOmmString(&tmpBuf, mapFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == mapFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", mapFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslMapFlagsToOmmString(&tmpBuf, mapFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(mapFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, mapFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", mapFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(mapFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPF_HAS_SET_DEFS.data) == RSSL_OMMSTR_MPF_HAS_SET_DEFS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPF_HAS_SUMMARY_DATA.data) == RSSL_OMMSTR_MPF_HAS_SUMMARY_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPF_HAS_PER_ENTRY_PERM_DATA.data) == RSSL_OMMSTR_MPF_HAS_PER_ENTRY_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPF_HAS_TOTAL_COUNT_HINT.data) == RSSL_OMMSTR_MPF_HAS_TOTAL_COUNT_HINT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPF_HAS_KEY_FIELD_ID.data) == RSSL_OMMSTR_MPF_HAS_KEY_FIELD_ID.length);

	/* MapEntryFlags */
	for (i = 0; i < sizeof(mapEntryFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = mapEntryFlagTests[i].outputBufferLength;

		if (mapEntryFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslMapEntryFlagsToOmmString(&tmpBuf, mapEntryFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == mapEntryFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", mapEntryFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslMapEntryFlagsToOmmString(&tmpBuf, mapEntryFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(mapEntryFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, mapEntryFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", mapEntryFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(mapEntryFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPEF_HAS_PERM_DATA.data) == RSSL_OMMSTR_MPEF_HAS_PERM_DATA.length);

	/* VectorFlags */
	for (i = 0; i < sizeof(vectorFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = vectorFlagTests[i].outputBufferLength;

		if (vectorFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslVectorFlagsToOmmString(&tmpBuf, vectorFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == vectorFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", vectorFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslVectorFlagsToOmmString(&tmpBuf, vectorFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(vectorFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, vectorFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", vectorFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(vectorFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTF_HAS_SET_DEFS.data) == RSSL_OMMSTR_VTF_HAS_SET_DEFS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTF_HAS_SUMMARY_DATA.data) == RSSL_OMMSTR_VTF_HAS_SUMMARY_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTF_HAS_PER_ENTRY_PERM_DATA.data) == RSSL_OMMSTR_VTF_HAS_PER_ENTRY_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTF_HAS_TOTAL_COUNT_HINT.data) == RSSL_OMMSTR_VTF_HAS_TOTAL_COUNT_HINT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTF_SUPPORTS_SORTING.data) == RSSL_OMMSTR_VTF_SUPPORTS_SORTING.length);

	/* VectorEntryFlags */
	for (i = 0; i < sizeof(vectorEntryFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = vectorEntryFlagTests[i].outputBufferLength;

		if (vectorEntryFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslVectorEntryFlagsToOmmString(&tmpBuf, vectorEntryFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == vectorEntryFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", vectorEntryFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslVectorEntryFlagsToOmmString(&tmpBuf, vectorEntryFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(vectorEntryFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, vectorEntryFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", vectorEntryFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(vectorEntryFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEF_HAS_PERM_DATA.data) == RSSL_OMMSTR_VTEF_HAS_PERM_DATA.length);

	/* SeriesFlags */
	for (i = 0; i < sizeof(seriesFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = seriesFlagTests[i].outputBufferLength;

		if (seriesFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslSeriesFlagsToOmmString(&tmpBuf, seriesFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == seriesFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", seriesFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslSeriesFlagsToOmmString(&tmpBuf, seriesFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(seriesFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, seriesFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", seriesFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(seriesFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SRF_HAS_SET_DEFS.data) == RSSL_OMMSTR_SRF_HAS_SET_DEFS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SRF_HAS_SUMMARY_DATA.data) == RSSL_OMMSTR_SRF_HAS_SUMMARY_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SRF_HAS_TOTAL_COUNT_HINT.data) == RSSL_OMMSTR_SRF_HAS_TOTAL_COUNT_HINT.length);

	/* FilterListFlags */
	for (i = 0; i < sizeof(filterListFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = filterListFlagTests[i].outputBufferLength;

		if (filterListFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslFilterListFlagsToOmmString(&tmpBuf, filterListFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == filterListFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", filterListFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslFilterListFlagsToOmmString(&tmpBuf, filterListFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(filterListFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, filterListFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", filterListFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(filterListFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTF_HAS_PER_ENTRY_PERM_DATA.data) == RSSL_OMMSTR_FTF_HAS_PER_ENTRY_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTF_HAS_TOTAL_COUNT_HINT.data) == RSSL_OMMSTR_FTF_HAS_TOTAL_COUNT_HINT.length);

	/* FilterEntryFlags */
	for (i = 0; i < sizeof(filterEntryFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = filterEntryFlagTests[i].outputBufferLength;

		if (filterEntryFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslFilterEntryFlagsToOmmString(&tmpBuf, filterEntryFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == filterEntryFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", filterEntryFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslFilterEntryFlagsToOmmString(&tmpBuf, filterEntryFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(filterEntryFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, filterEntryFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", filterEntryFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(filterEntryFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTEF_HAS_PERM_DATA.data) == RSSL_OMMSTR_FTEF_HAS_PERM_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTEF_HAS_CONTAINER_TYPE.data) == RSSL_OMMSTR_FTEF_HAS_CONTAINER_TYPE.length);

	/* FieldListFlags */
	for (i = 0; i < sizeof(fieldListFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = fieldListFlagTests[i].outputBufferLength;

		if (fieldListFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslFieldListFlagsToOmmString(&tmpBuf, fieldListFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == fieldListFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", fieldListFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslFieldListFlagsToOmmString(&tmpBuf, fieldListFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(fieldListFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, fieldListFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", fieldListFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(fieldListFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FLF_HAS_FIELD_LIST_INFO.data) == RSSL_OMMSTR_FLF_HAS_FIELD_LIST_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FLF_HAS_SET_DATA.data) == RSSL_OMMSTR_FLF_HAS_SET_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FLF_HAS_SET_ID.data) == RSSL_OMMSTR_FLF_HAS_SET_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FLF_HAS_STANDARD_DATA.data) == RSSL_OMMSTR_FLF_HAS_STANDARD_DATA.length);

	/* ElementListFlags */
	for (i = 0; i < sizeof(elementListFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = elementListFlagTests[i].outputBufferLength;

		if (elementListFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslElementListFlagsToOmmString(&tmpBuf, elementListFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == elementListFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", elementListFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslElementListFlagsToOmmString(&tmpBuf, elementListFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(elementListFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, elementListFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", elementListFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(elementListFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_ELF_HAS_ELEMENT_LIST_INFO.data) == RSSL_OMMSTR_ELF_HAS_ELEMENT_LIST_INFO.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_ELF_HAS_SET_DATA.data) == RSSL_OMMSTR_ELF_HAS_SET_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_ELF_HAS_SET_ID.data) == RSSL_OMMSTR_ELF_HAS_SET_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_ELF_HAS_STANDARD_DATA.data) == RSSL_OMMSTR_ELF_HAS_STANDARD_DATA.length);


	/* MsgClass */
	ASSERT_TRUE(rsslMsgClassToOmmString(0) == NULL);
	ASSERT_TRUE(rsslMsgClassToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_REQUEST.data, rsslMsgClassToOmmString(RSSL_MC_REQUEST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_REFRESH.data, rsslMsgClassToOmmString(RSSL_MC_REFRESH)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_UPDATE.data, rsslMsgClassToOmmString(RSSL_MC_UPDATE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_STATUS.data, rsslMsgClassToOmmString(RSSL_MC_STATUS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_ACK.data, rsslMsgClassToOmmString(RSSL_MC_ACK)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_GENERIC.data, rsslMsgClassToOmmString(RSSL_MC_GENERIC)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MC_POST.data, rsslMsgClassToOmmString(RSSL_MC_POST)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_REQUEST.data) == RSSL_OMMSTR_MC_REQUEST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_REFRESH.data) == RSSL_OMMSTR_MC_REFRESH.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_UPDATE.data) == RSSL_OMMSTR_MC_UPDATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_STATUS.data) == RSSL_OMMSTR_MC_STATUS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_ACK.data) == RSSL_OMMSTR_MC_ACK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_GENERIC.data) == RSSL_OMMSTR_MC_GENERIC.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MC_POST.data) == RSSL_OMMSTR_MC_POST.length);

	/* DataType */
	ASSERT_TRUE(rsslDataTypeToOmmString(1) == NULL);
	ASSERT_TRUE(rsslDataTypeToOmmString(2) == NULL);
	ASSERT_TRUE(rsslDataTypeToOmmString(225) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UNKNOWN.data, rsslDataTypeToOmmString(RSSL_DT_UNKNOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_INT.data, rsslDataTypeToOmmString(RSSL_DT_INT)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UINT.data, rsslDataTypeToOmmString(RSSL_DT_UINT)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_FLOAT.data, rsslDataTypeToOmmString(RSSL_DT_FLOAT)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DOUBLE.data, rsslDataTypeToOmmString(RSSL_DT_DOUBLE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATE.data, rsslDataTypeToOmmString(RSSL_DT_DATE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_TIME.data, rsslDataTypeToOmmString(RSSL_DT_TIME)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATETIME.data, rsslDataTypeToOmmString(RSSL_DT_DATETIME)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_QOS.data, rsslDataTypeToOmmString(RSSL_DT_QOS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_STATE.data, rsslDataTypeToOmmString(RSSL_DT_STATE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_ENUM.data, rsslDataTypeToOmmString(RSSL_DT_ENUM)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_ARRAY.data, rsslDataTypeToOmmString(RSSL_DT_ARRAY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_BUFFER.data, rsslDataTypeToOmmString(RSSL_DT_BUFFER)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_ASCII_STRING.data, rsslDataTypeToOmmString(RSSL_DT_ASCII_STRING)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UTF8_STRING.data, rsslDataTypeToOmmString(RSSL_DT_UTF8_STRING)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_RMTES_STRING.data, rsslDataTypeToOmmString(RSSL_DT_RMTES_STRING)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_INT_1.data, rsslDataTypeToOmmString(RSSL_DT_INT_1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UINT_1.data, rsslDataTypeToOmmString(RSSL_DT_UINT_1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_INT_2.data, rsslDataTypeToOmmString(RSSL_DT_INT_2)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_INT_4.data, rsslDataTypeToOmmString(RSSL_DT_INT_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UINT_4.data, rsslDataTypeToOmmString(RSSL_DT_UINT_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_INT_8.data, rsslDataTypeToOmmString(RSSL_DT_INT_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_UINT_8.data, rsslDataTypeToOmmString(RSSL_DT_UINT_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_FLOAT_4.data, rsslDataTypeToOmmString(RSSL_DT_FLOAT_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DOUBLE_8.data, rsslDataTypeToOmmString(RSSL_DT_DOUBLE_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_REAL_4RB.data, rsslDataTypeToOmmString(RSSL_DT_REAL_4RB)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_REAL_8RB.data, rsslDataTypeToOmmString(RSSL_DT_REAL_8RB)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATE_4.data, rsslDataTypeToOmmString(RSSL_DT_DATE_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_TIME_3.data, rsslDataTypeToOmmString(RSSL_DT_TIME_3)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_TIME_5.data, rsslDataTypeToOmmString(RSSL_DT_TIME_5)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATETIME_7.data, rsslDataTypeToOmmString(RSSL_DT_DATETIME_7)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATETIME_9.data, rsslDataTypeToOmmString(RSSL_DT_DATETIME_9)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATETIME_11.data, rsslDataTypeToOmmString(RSSL_DT_DATETIME_11)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_DATETIME_12.data, rsslDataTypeToOmmString(RSSL_DT_DATETIME_12)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_TIME_7.data, rsslDataTypeToOmmString(RSSL_DT_TIME_7)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_TIME_8.data, rsslDataTypeToOmmString(RSSL_DT_TIME_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_NO_DATA.data, rsslDataTypeToOmmString(RSSL_DT_NO_DATA)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_XML.data, rsslDataTypeToOmmString(RSSL_DT_XML)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_FIELD_LIST.data, rsslDataTypeToOmmString(RSSL_DT_FIELD_LIST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_ELEMENT_LIST.data, rsslDataTypeToOmmString(RSSL_DT_ELEMENT_LIST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_ANSI_PAGE.data, rsslDataTypeToOmmString(RSSL_DT_ANSI_PAGE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_FILTER_LIST.data, rsslDataTypeToOmmString(RSSL_DT_FILTER_LIST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_VECTOR.data, rsslDataTypeToOmmString(RSSL_DT_VECTOR)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_MAP.data, rsslDataTypeToOmmString(RSSL_DT_MAP)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_FILTER_LIST.data, rsslDataTypeToOmmString(RSSL_DT_FILTER_LIST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_VECTOR.data, rsslDataTypeToOmmString(RSSL_DT_VECTOR)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_MAP.data, rsslDataTypeToOmmString(RSSL_DT_MAP)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_SERIES.data, rsslDataTypeToOmmString(RSSL_DT_SERIES)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_MSG.data, rsslDataTypeToOmmString(RSSL_DT_MSG)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DT_JSON.data, rsslDataTypeToOmmString(RSSL_DT_JSON)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UNKNOWN.data) == RSSL_OMMSTR_DT_UNKNOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_INT.data) == RSSL_OMMSTR_DT_INT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UINT.data) == RSSL_OMMSTR_DT_UINT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_FLOAT.data) == RSSL_OMMSTR_DT_FLOAT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DOUBLE.data) == RSSL_OMMSTR_DT_DOUBLE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATE.data) == RSSL_OMMSTR_DT_DATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_TIME.data) == RSSL_OMMSTR_DT_TIME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATETIME.data) == RSSL_OMMSTR_DT_DATETIME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_QOS.data) == RSSL_OMMSTR_DT_QOS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_STATE.data) == RSSL_OMMSTR_DT_STATE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_ENUM.data) == RSSL_OMMSTR_DT_ENUM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_ARRAY.data) == RSSL_OMMSTR_DT_ARRAY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_BUFFER.data) == RSSL_OMMSTR_DT_BUFFER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_ASCII_STRING.data) == RSSL_OMMSTR_DT_ASCII_STRING.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UTF8_STRING.data) == RSSL_OMMSTR_DT_UTF8_STRING.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_RMTES_STRING.data) == RSSL_OMMSTR_DT_RMTES_STRING.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_INT_1.data) == RSSL_OMMSTR_DT_INT_1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UINT_1.data) == RSSL_OMMSTR_DT_UINT_1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_INT_2.data) == RSSL_OMMSTR_DT_INT_2.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_INT_4.data) == RSSL_OMMSTR_DT_INT_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UINT_4.data) == RSSL_OMMSTR_DT_UINT_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_INT_8.data) == RSSL_OMMSTR_DT_INT_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_UINT_8.data) == RSSL_OMMSTR_DT_UINT_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_FLOAT_4.data) == RSSL_OMMSTR_DT_FLOAT_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DOUBLE_8.data) == RSSL_OMMSTR_DT_DOUBLE_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_REAL_4RB.data) == RSSL_OMMSTR_DT_REAL_4RB.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_REAL_8RB.data) == RSSL_OMMSTR_DT_REAL_8RB.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATE_4.data) == RSSL_OMMSTR_DT_DATE_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_TIME_3.data) == RSSL_OMMSTR_DT_TIME_3.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_TIME_5.data) == RSSL_OMMSTR_DT_TIME_5.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATETIME_7.data) == RSSL_OMMSTR_DT_DATETIME_7.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATETIME_9.data) == RSSL_OMMSTR_DT_DATETIME_9.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATETIME_11.data) == RSSL_OMMSTR_DT_DATETIME_11.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_DATETIME_12.data) == RSSL_OMMSTR_DT_DATETIME_12.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_TIME_7.data) == RSSL_OMMSTR_DT_TIME_7.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_TIME_8.data) == RSSL_OMMSTR_DT_TIME_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_NO_DATA.data) == RSSL_OMMSTR_DT_NO_DATA.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_XML.data) == RSSL_OMMSTR_DT_XML.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_FIELD_LIST.data) == RSSL_OMMSTR_DT_FIELD_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_ELEMENT_LIST.data) == RSSL_OMMSTR_DT_ELEMENT_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_ANSI_PAGE.data) == RSSL_OMMSTR_DT_ANSI_PAGE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_FILTER_LIST.data) == RSSL_OMMSTR_DT_FILTER_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_VECTOR.data) == RSSL_OMMSTR_DT_VECTOR.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_MAP.data) == RSSL_OMMSTR_DT_MAP.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_FILTER_LIST.data) == RSSL_OMMSTR_DT_FILTER_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_VECTOR.data) == RSSL_OMMSTR_DT_VECTOR.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_MAP.data) == RSSL_OMMSTR_DT_MAP.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_SERIES.data) == RSSL_OMMSTR_DT_SERIES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_MSG.data) == RSSL_OMMSTR_DT_MSG.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DT_JSON.data) == RSSL_OMMSTR_DT_JSON.length);

	/* DomainType */
	ASSERT_TRUE(rsslDomainTypeToOmmString(0) == NULL);
	ASSERT_TRUE(rsslDomainTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_LOGIN.data, rsslDomainTypeToOmmString(RSSL_DMT_LOGIN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_SOURCE.data, rsslDomainTypeToOmmString(RSSL_DMT_SOURCE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_DICTIONARY.data, rsslDomainTypeToOmmString(RSSL_DMT_DICTIONARY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_MARKET_PRICE.data, rsslDomainTypeToOmmString(RSSL_DMT_MARKET_PRICE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_MARKET_BY_ORDER.data, rsslDomainTypeToOmmString(RSSL_DMT_MARKET_BY_ORDER)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_MARKET_BY_PRICE.data, rsslDomainTypeToOmmString(RSSL_DMT_MARKET_BY_PRICE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_MARKET_MAKER.data, rsslDomainTypeToOmmString(RSSL_DMT_MARKET_MAKER)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_SYMBOL_LIST.data, rsslDomainTypeToOmmString(RSSL_DMT_SYMBOL_LIST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS.data, rsslDomainTypeToOmmString(RSSL_DMT_SERVICE_PROVIDER_STATUS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_HISTORY.data, rsslDomainTypeToOmmString(RSSL_DMT_HISTORY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_HEADLINE.data, rsslDomainTypeToOmmString(RSSL_DMT_HEADLINE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_STORY.data, rsslDomainTypeToOmmString(RSSL_DMT_STORY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_REPLAYHEADLINE.data, rsslDomainTypeToOmmString(RSSL_DMT_REPLAYHEADLINE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_REPLAYSTORY.data, rsslDomainTypeToOmmString(RSSL_DMT_REPLAYSTORY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_TRANSACTION.data, rsslDomainTypeToOmmString(RSSL_DMT_TRANSACTION)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_YIELD_CURVE.data, rsslDomainTypeToOmmString(RSSL_DMT_YIELD_CURVE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_CONTRIBUTION.data, rsslDomainTypeToOmmString(RSSL_DMT_CONTRIBUTION)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_PROVIDER_ADMIN.data, rsslDomainTypeToOmmString(RSSL_DMT_PROVIDER_ADMIN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_ANALYTICS.data, rsslDomainTypeToOmmString(RSSL_DMT_ANALYTICS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_REFERENCE.data, rsslDomainTypeToOmmString(RSSL_DMT_REFERENCE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS.data, rsslDomainTypeToOmmString(RSSL_DMT_NEWS_TEXT_ANALYTICS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR.data, rsslDomainTypeToOmmString(RSSL_DMT_ECONOMIC_INDICATOR)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_POLL.data, rsslDomainTypeToOmmString(RSSL_DMT_POLL)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_FORECAST.data, rsslDomainTypeToOmmString(RSSL_DMT_FORECAST)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_MARKET_BY_TIME.data, rsslDomainTypeToOmmString(RSSL_DMT_MARKET_BY_TIME)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DMT_SYSTEM.data, rsslDomainTypeToOmmString(RSSL_DMT_SYSTEM)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_LOGIN.data) == RSSL_OMMSTR_DMT_LOGIN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_SOURCE.data) == RSSL_OMMSTR_DMT_SOURCE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_DICTIONARY.data) == RSSL_OMMSTR_DMT_DICTIONARY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_MARKET_PRICE.data) == RSSL_OMMSTR_DMT_MARKET_PRICE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_MARKET_BY_ORDER.data) == RSSL_OMMSTR_DMT_MARKET_BY_ORDER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_MARKET_BY_PRICE.data) == RSSL_OMMSTR_DMT_MARKET_BY_PRICE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_MARKET_MAKER.data) == RSSL_OMMSTR_DMT_MARKET_MAKER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_SYMBOL_LIST.data) == RSSL_OMMSTR_DMT_SYMBOL_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS.data) == RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_HISTORY.data) == RSSL_OMMSTR_DMT_HISTORY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_HEADLINE.data) == RSSL_OMMSTR_DMT_HEADLINE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_STORY.data) == RSSL_OMMSTR_DMT_STORY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_REPLAYHEADLINE.data) == RSSL_OMMSTR_DMT_REPLAYHEADLINE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_REPLAYSTORY.data) == RSSL_OMMSTR_DMT_REPLAYSTORY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_TRANSACTION.data) == RSSL_OMMSTR_DMT_TRANSACTION.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_YIELD_CURVE.data) == RSSL_OMMSTR_DMT_YIELD_CURVE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_CONTRIBUTION.data) == RSSL_OMMSTR_DMT_CONTRIBUTION.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_PROVIDER_ADMIN.data) == RSSL_OMMSTR_DMT_PROVIDER_ADMIN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_ANALYTICS.data) == RSSL_OMMSTR_DMT_ANALYTICS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_REFERENCE.data) == RSSL_OMMSTR_DMT_REFERENCE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS.data) == RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR.data) == RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_POLL.data) == RSSL_OMMSTR_DMT_POLL.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_FORECAST.data) == RSSL_OMMSTR_DMT_FORECAST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_MARKET_BY_TIME.data) == RSSL_OMMSTR_DMT_MARKET_BY_TIME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DMT_SYSTEM.data) == RSSL_OMMSTR_DMT_SYSTEM.length);

	/* InstrumentNameType */
	ASSERT_TRUE(rsslRDMInstrumentNameTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED.data, rsslRDMInstrumentNameTypeToOmmString(RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC.data, rsslRDMInstrumentNameTypeToOmmString(RDM_INSTRUMENT_NAME_TYPE_RIC)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR.data, rsslRDMInstrumentNameTypeToOmmString(RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED.data) == RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC.data) == RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR.data) == RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR.length);

	/* UpdateEventType */
	ASSERT_TRUE(rsslRDMUpdateEventTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_UNSPECIFIED)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_QUOTE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_TRADE.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_TRADE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_NEWS_ALERT)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_VOLUME_ALERT)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_ORDER_INDICATION)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_CLOSING_RUN)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_CORRECTION)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_MARKET_DIGEST)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_QUOTES_TRADE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_MULTIPLE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY.data, rsslRDMUpdateEventTypeToOmmString(RDM_UPD_EVENT_TYPE_VERIFY)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED.data) == RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE.data) == RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_TRADE.data) == RDM_OMMSTR_UPD_EVENT_TYPE_TRADE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT.data) == RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT.data) == RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION.data) == RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN.data) == RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION.data) == RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST.data) == RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE.data) == RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE.data) == RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY.data) == RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY.length);

	/* ViewType */
	ASSERT_TRUE(rsslRDMViewTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_VIEW_TYPE_FIELD_ID_LIST.data, rsslRDMViewTypeToOmmString(RDM_VIEW_TYPE_FIELD_ID_LIST)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_VIEW_TYPE_ELEMENT_NAME_LIST.data, rsslRDMViewTypeToOmmString(RDM_VIEW_TYPE_ELEMENT_NAME_LIST)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_VIEW_TYPE_FIELD_ID_LIST.data) == RDM_OMMSTR_VIEW_TYPE_FIELD_ID_LIST.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_VIEW_TYPE_ELEMENT_NAME_LIST.data) == RDM_OMMSTR_VIEW_TYPE_ELEMENT_NAME_LIST.length);

	/* LoginRoleType */
	ASSERT_TRUE(rsslRDMLoginRoleTypeToOmmString(128) == NULL);
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_ROLE_CONS.data, rsslRDMLoginRoleTypeToOmmString(RDM_LOGIN_ROLE_CONS)));
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_ROLE_PROV.data, rsslRDMLoginRoleTypeToOmmString(RDM_LOGIN_ROLE_PROV)));

    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_ROLE_CONS.data) == RDM_OMMSTR_LOGIN_ROLE_CONS.length);
    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_ROLE_PROV.data) == RDM_OMMSTR_LOGIN_ROLE_PROV.length);

	/* LoginUserIdType */
	ASSERT_TRUE(rsslRDMLoginUserIdTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_USER_NAME.data, rsslRDMLoginUserIdTypeToOmmString(RDM_LOGIN_USER_NAME)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS.data, rsslRDMLoginUserIdTypeToOmmString(RDM_LOGIN_USER_EMAIL_ADDRESS)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_USER_TOKEN.data, rsslRDMLoginUserIdTypeToOmmString(RDM_LOGIN_USER_TOKEN)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_USER_COOKIE.data, rsslRDMLoginUserIdTypeToOmmString(RDM_LOGIN_USER_COOKIE)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_USER_NAME.data) == RDM_OMMSTR_LOGIN_USER_NAME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS.data) == RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_USER_TOKEN.data) == RDM_OMMSTR_LOGIN_USER_TOKEN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_USER_COOKIE.data) == RDM_OMMSTR_LOGIN_USER_COOKIE.length);

	/* LoginServerType */
	ASSERT_TRUE(rsslRDMLoginServerTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_SERVER_TYPE_ACTIVE.data, rsslRDMLoginServerTypeToOmmString(RDM_LOGIN_SERVER_TYPE_ACTIVE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_LOGIN_SERVER_TYPE_STANDBY.data, rsslRDMLoginServerTypeToOmmString(RDM_LOGIN_SERVER_TYPE_STANDBY)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_SERVER_TYPE_ACTIVE.data) == RDM_OMMSTR_LOGIN_SERVER_TYPE_ACTIVE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_SERVER_TYPE_STANDBY.data) == RDM_OMMSTR_LOGIN_SERVER_TYPE_STANDBY.length);

	/* LoginBatchSupportFlags */
	for (i = 0; i < sizeof(loginBatchSupportFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = loginBatchSupportFlagTests[i].outputBufferLength;

		if (loginBatchSupportFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRDMLoginBatchSupportFlagsToOmmString(&tmpBuf, loginBatchSupportFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == loginBatchSupportFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", loginBatchSupportFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRDMLoginBatchSupportFlagsToOmmString(&tmpBuf, loginBatchSupportFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(loginBatchSupportFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, loginBatchSupportFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", loginBatchSupportFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(loginBatchSupportFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REQUESTS.data) == RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REQUESTS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REISSUES.data) == RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REISSUES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_LOGIN_BATCH_SUPPORT_CLOSES.data) == RDM_OMMSTR_LOGIN_BATCH_SUPPORT_CLOSES.length);

	/* DictionaryType */
	ASSERT_TRUE(rsslRDMDictionaryTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_FIELD_DEFINITIONS.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_FIELD_DEFINITIONS)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_ENUM_TABLES.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_ENUM_TABLES)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_RECORD_TEMPLATES.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_RECORD_TEMPLATES)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_DISPLAY_TEMPLATES.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_DISPLAY_TEMPLATES)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_DATA_DEFINITIONS.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_DATA_DEFINITIONS)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_STYLE_SHEET.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_STYLE_SHEET)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_REFERENCE.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_REFERENCE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_FIELD_SET_DEFINITION.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_FIELD_SET_DEFINITION)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_ELEMENT_SET_DEFINITION.data, rsslRDMDictionaryTypeToOmmString(RDM_DICTIONARY_ELEMENT_SET_DEFINITION)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_FIELD_DEFINITIONS.data) == RDM_OMMSTR_DICTIONARY_FIELD_DEFINITIONS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_ENUM_TABLES.data) == RDM_OMMSTR_DICTIONARY_ENUM_TABLES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_RECORD_TEMPLATES.data) == RDM_OMMSTR_DICTIONARY_RECORD_TEMPLATES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_DISPLAY_TEMPLATES.data) == RDM_OMMSTR_DICTIONARY_DISPLAY_TEMPLATES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_DATA_DEFINITIONS.data) == RDM_OMMSTR_DICTIONARY_DATA_DEFINITIONS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_STYLE_SHEET.data) == RDM_OMMSTR_DICTIONARY_STYLE_SHEET.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_REFERENCE.data) == RDM_OMMSTR_DICTIONARY_REFERENCE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_FIELD_SET_DEFINITION.data) == RDM_OMMSTR_DICTIONARY_FIELD_SET_DEFINITION.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_ELEMENT_SET_DEFINITION.data) == RDM_OMMSTR_DICTIONARY_ELEMENT_SET_DEFINITION.length);

	/* DictionaryVerbosityValue */
	ASSERT_TRUE(rsslRDMDictionaryVerbosityValueToOmmString(128) == NULL);
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_INFO.data, rsslRDMDictionaryVerbosityValueToOmmString(RDM_DICTIONARY_INFO)));
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_MINIMAL.data, rsslRDMDictionaryVerbosityValueToOmmString(RDM_DICTIONARY_MINIMAL)));
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_NORMAL.data, rsslRDMDictionaryVerbosityValueToOmmString(RDM_DICTIONARY_NORMAL)));
    ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DICTIONARY_VERBOSE.data, rsslRDMDictionaryVerbosityValueToOmmString(RDM_DICTIONARY_VERBOSE)));

    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_INFO.data) == RDM_OMMSTR_DICTIONARY_INFO.length);
    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_MINIMAL.data) == RDM_OMMSTR_DICTIONARY_MINIMAL.length);
    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_NORMAL.data) == RDM_OMMSTR_DICTIONARY_NORMAL.length);
    ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DICTIONARY_VERBOSE.data) == RDM_OMMSTR_DICTIONARY_VERBOSE.length);

	/* ServiceFilterFlags */
	for (i = 0; i < sizeof(serviceFilterFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = serviceFilterFlagTests[i].outputBufferLength;

		if (serviceFilterFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRDMDirectoryServiceFilterFlagsToOmmString(&tmpBuf, serviceFilterFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == serviceFilterFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", serviceFilterFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRDMDirectoryServiceFilterFlagsToOmmString(&tmpBuf, serviceFilterFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(serviceFilterFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, serviceFilterFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", serviceFilterFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(loginBatchSupportFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_INFO_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_INFO_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_STATE_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_DATA_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_DATA_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_LINK_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_LINK_FILTER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_FILTER.data) == RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_FILTER.length);

	/* ServiceFilterId */
	ASSERT_TRUE(rsslRDMDirectoryServiceFilterIdToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_INFO_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_INFO_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_STATE_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_GROUP_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_LOAD_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_DATA_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_DATA_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_LINK_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_LINK_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_ID.data, rsslRDMDirectoryServiceFilterIdToOmmString(RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_INFO_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_INFO_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_STATE_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_DATA_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_DATA_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_LINK_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_LINK_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_ID.data) == RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_ID.length);

	/* ServiceState */
	ASSERT_TRUE(rsslRDMDirectoryServiceStateToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_DOWN.data, rsslRDMDirectoryServiceStateToOmmString(RDM_DIRECTORY_SERVICE_STATE_DOWN)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_UP.data, rsslRDMDirectoryServiceStateToOmmString(RDM_DIRECTORY_SERVICE_STATE_UP)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_DOWN.data) == RDM_OMMSTR_DIRECTORY_SERVICE_STATE_DOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SERVICE_STATE_UP.data) == RDM_OMMSTR_DIRECTORY_SERVICE_STATE_UP.length);

	/* SourceMirroringMode */
	ASSERT_TRUE(rsslRDMDirectorySourceMirroringModeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY.data, rsslRDMDirectorySourceMirroringModeToOmmString(RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY.data, rsslRDMDirectorySourceMirroringModeToOmmString(RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY.data, rsslRDMDirectorySourceMirroringModeToOmmString(RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY.data) == RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY.data) == RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY.data) == RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY.length);

	/* DirectoryDataType */
	ASSERT_TRUE(rsslRDMDirectoryDataTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_DATA_TYPE_NONE.data, rsslRDMDirectoryDataTypeToOmmString(RDM_DIRECTORY_DATA_TYPE_NONE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_DATA_TYPE_TIME.data, rsslRDMDirectoryDataTypeToOmmString(RDM_DIRECTORY_DATA_TYPE_TIME)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_DATA_TYPE_ALERT.data, rsslRDMDirectoryDataTypeToOmmString(RDM_DIRECTORY_DATA_TYPE_ALERT)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_DATA_TYPE_HEADLINE.data, rsslRDMDirectoryDataTypeToOmmString(RDM_DIRECTORY_DATA_TYPE_HEADLINE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_DATA_TYPE_STATUS.data, rsslRDMDirectoryDataTypeToOmmString(RDM_DIRECTORY_DATA_TYPE_STATUS)));
	
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_DATA_TYPE_NONE.data) == RDM_OMMSTR_DIRECTORY_DATA_TYPE_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_DATA_TYPE_TIME.data) == RDM_OMMSTR_DIRECTORY_DATA_TYPE_TIME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_DATA_TYPE_ALERT.data) == RDM_OMMSTR_DIRECTORY_DATA_TYPE_ALERT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_DATA_TYPE_HEADLINE.data) == RDM_OMMSTR_DIRECTORY_DATA_TYPE_HEADLINE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_DATA_TYPE_STATUS.data) == RDM_OMMSTR_DIRECTORY_DATA_TYPE_STATUS.length);

	/* LinkType */
	ASSERT_TRUE(rsslRDMDirectoryLinkTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_TYPE_INTERACTIVE.data, rsslRDMDirectoryLinkTypeToOmmString(RDM_DIRECTORY_LINK_TYPE_INTERACTIVE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_TYPE_BROADCAST.data, rsslRDMDirectoryLinkTypeToOmmString(RDM_DIRECTORY_LINK_TYPE_BROADCAST)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_TYPE_INTERACTIVE.data) == RDM_OMMSTR_DIRECTORY_LINK_TYPE_INTERACTIVE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_TYPE_BROADCAST.data) == RDM_OMMSTR_DIRECTORY_LINK_TYPE_BROADCAST.length);

	/* LinkState */
	ASSERT_TRUE(rsslRDMDirectoryLinkStateToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_STATE_DOWN.data, rsslRDMDirectoryLinkStateToOmmString(RDM_DIRECTORY_LINK_STATE_DOWN)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_STATE_UP.data, rsslRDMDirectoryLinkStateToOmmString(RDM_DIRECTORY_LINK_STATE_UP)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_STATE_DOWN.data) == RDM_OMMSTR_DIRECTORY_LINK_STATE_DOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_STATE_UP.data) == RDM_OMMSTR_DIRECTORY_LINK_STATE_UP.length);

	/* LinkCode */
	ASSERT_TRUE(rsslRDMDirectoryLinkCodeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_CODE_NONE.data, rsslRDMDirectoryLinkCodeToOmmString(RDM_DIRECTORY_LINK_CODE_NONE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_CODE_OK.data, rsslRDMDirectoryLinkCodeToOmmString(RDM_DIRECTORY_LINK_CODE_OK)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_STARTED.data, rsslRDMDirectoryLinkCodeToOmmString(RDM_DIRECTORY_LINK_CODE_RECOVERY_STARTED)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED.data, rsslRDMDirectoryLinkCodeToOmmString(RDM_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_CODE_NONE.data) == RDM_OMMSTR_DIRECTORY_LINK_CODE_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_CODE_OK.data) == RDM_OMMSTR_DIRECTORY_LINK_CODE_OK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_STARTED.data) == RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_STARTED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED.data) == RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED.length);

	/* SymbolListSupportFlags */
	for (i = 0; i < sizeof(symbolListSupportFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = symbolListSupportFlagTests[i].outputBufferLength;

		if (symbolListSupportFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRDMEnhancedSymbolListSupportFlagsToOmmString(&tmpBuf, symbolListSupportFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == symbolListSupportFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", symbolListSupportFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRDMEnhancedSymbolListSupportFlagsToOmmString(&tmpBuf, symbolListSupportFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(symbolListSupportFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, symbolListSupportFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", symbolListSupportFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(symbolListSupportFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_SYMBOL_LIST_SUPPORT_DATA_STREAMS.data) == RDM_OMMSTR_SYMBOL_LIST_SUPPORT_DATA_STREAMS.length);

	/* SymbolListDataStreamRequestFlags */
	for (i = 0; i < sizeof(symbolListDataStreamRequestFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = symbolListDataStreamRequestFlagTests[i].outputBufferLength;

		if (symbolListDataStreamRequestFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRDMSymbolListDataStreamRequestFlagsToOmmString(&tmpBuf, symbolListDataStreamRequestFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == symbolListDataStreamRequestFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", symbolListDataStreamRequestFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRDMSymbolListDataStreamRequestFlagsToOmmString(&tmpBuf, symbolListDataStreamRequestFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(symbolListDataStreamRequestFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, symbolListDataStreamRequestFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", symbolListDataStreamRequestFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(symbolListDataStreamRequestFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_SYMBOL_LIST_DATA_STREAMS.data) == RDM_OMMSTR_SYMBOL_LIST_DATA_STREAMS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_SYMBOL_LIST_DATA_SNAPSHOTS.data) == RDM_OMMSTR_SYMBOL_LIST_DATA_SNAPSHOTS.length);

	/* CosFilterId */
	ASSERT_TRUE(rsslRDMCosFilterIdToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_COMMON_PROPERTIES_ID.data, rsslRDMCosFilterIdToOmmString(RDM_COS_COMMON_PROPERTIES_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_AUTHENTICATION_ID.data, rsslRDMCosFilterIdToOmmString(RDM_COS_AUTHENTICATION_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_FLOW_CONTROL_ID.data, rsslRDMCosFilterIdToOmmString(RDM_COS_FLOW_CONTROL_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_DATA_INTEGRITY_ID.data, rsslRDMCosFilterIdToOmmString(RDM_COS_DATA_INTEGRITY_ID)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_GUARANTEE_ID.data, rsslRDMCosFilterIdToOmmString(RDM_COS_GUARANTEE_ID)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_COMMON_PROPERTIES_ID.data) == RDM_OMMSTR_COS_COMMON_PROPERTIES_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_AUTHENTICATION_ID.data) == RDM_OMMSTR_COS_AUTHENTICATION_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_FLOW_CONTROL_ID.data) == RDM_OMMSTR_COS_FLOW_CONTROL_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_DATA_INTEGRITY_ID.data) == RDM_OMMSTR_COS_DATA_INTEGRITY_ID.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_GUARANTEE_ID.data) == RDM_OMMSTR_COS_GUARANTEE_ID.length);

	/* CosFilterFlags */
	for (i = 0; i < sizeof(cosFilterFlagTests)/sizeof(FlagToStringTest); ++i)
	{
		tmpBuf.length = cosFilterFlagTests[i].outputBufferLength;

		if (cosFilterFlagTests[i].retCode == RSSL_RET_BUFFER_TOO_SMALL)
		{
			ASSERT_TRUE(rsslRDMCosFilterFlagsToOmmString(&tmpBuf, cosFilterFlagTests[i].flags) == RSSL_RET_BUFFER_TOO_SMALL);
			ASSERT_TRUE(tmpBuf.length == cosFilterFlagTests[i].outputBufferLength);
			//printf("<DEBUG> Buffer was too small for %s\n", cosFilterFlagTests[i].flagString);
		}
		else
		{
			ASSERT_TRUE(rsslRDMCosFilterFlagsToOmmString(&tmpBuf, cosFilterFlagTests[i].flags) == RSSL_RET_SUCCESS);
			ASSERT_TRUE(tmpBuf.length == (RsslUInt32)strlen(cosFilterFlagTests[i].flagString));
			ASSERT_TRUE(0 == memcmp(tmpBuf.data, cosFilterFlagTests[i].flagString, tmpBuf.length));
			//printf("<DEBUG> 0x%x %.*s (%d characters)\n", cosFilterFlagTests[i].flags, tmpBuf.length, tmpBuf.data, strlen(cosFilterFlagTests[i].flagString));
		}
	}

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_COMMON_PROPERTIES_FLAG.data) == RDM_OMMSTR_COS_COMMON_PROPERTIES_FLAG.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_AUTHENTICATION_FLAG.data) == RDM_OMMSTR_COS_AUTHENTICATION_FLAG.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_FLOW_CONTROL_FLAG.data) == RDM_OMMSTR_COS_FLOW_CONTROL_FLAG.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_DATA_INTEGRITY_FLAG.data) == RDM_OMMSTR_COS_DATA_INTEGRITY_FLAG.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_GUARANTEE_FLAG.data) == RDM_OMMSTR_COS_GUARANTEE_FLAG.length);

	/* CosAuthenticationType */
	ASSERT_TRUE(rsslRDMCosAuthenticationTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_AU_NOT_REQUIRED.data, rsslRDMCosAuthenticationTypeToOmmString(RDM_COS_AU_NOT_REQUIRED)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_AU_OMM_LOGIN.data, rsslRDMCosAuthenticationTypeToOmmString(RDM_COS_AU_OMM_LOGIN)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_AU_NOT_REQUIRED.data) == RDM_OMMSTR_COS_AU_NOT_REQUIRED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_AU_OMM_LOGIN.data) == RDM_OMMSTR_COS_AU_OMM_LOGIN.length);

	/* CosFlowControlType */
	ASSERT_TRUE(rsslRDMCosFlowControlTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_FC_NONE.data, rsslRDMCosFlowControlTypeToOmmString(RDM_COS_FC_NONE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_FC_BIDIRECTIONAL.data, rsslRDMCosFlowControlTypeToOmmString(RDM_COS_FC_BIDIRECTIONAL)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_FC_NONE.data) == RDM_OMMSTR_COS_FC_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_FC_BIDIRECTIONAL.data) == RDM_OMMSTR_COS_FC_BIDIRECTIONAL.length);

	/* CosDataIntegrityType */
	ASSERT_TRUE(rsslRDMCosDataIntegrityTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_DI_BEST_EFFORT.data, rsslRDMCosDataIntegrityTypeToOmmString(RDM_COS_DI_BEST_EFFORT)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_DI_RELIABLE.data, rsslRDMCosDataIntegrityTypeToOmmString(RDM_COS_DI_RELIABLE)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_DI_BEST_EFFORT.data) == RDM_OMMSTR_COS_DI_BEST_EFFORT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_DI_RELIABLE.data) == RDM_OMMSTR_COS_DI_RELIABLE.length);

	/* CosGuaranteeType */
	ASSERT_TRUE(rsslRDMCosGuaranteeTypeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_GU_NONE.data, rsslRDMCosGuaranteeTypeToOmmString(RDM_COS_GU_NONE)));
	ASSERT_TRUE(0 == strcmp(RDM_OMMSTR_COS_GU_PERSISTENT_QUEUE.data, rsslRDMCosGuaranteeTypeToOmmString(RDM_COS_GU_PERSISTENT_QUEUE)));

	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_GU_NONE.data) == RDM_OMMSTR_COS_GU_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RDM_OMMSTR_COS_GU_PERSISTENT_QUEUE.data) == RDM_OMMSTR_COS_GU_PERSISTENT_QUEUE.length);


	/* StreamStates */
	ASSERT_TRUE(rsslStreamStateToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_UNSPECIFIED.data, rsslStreamStateToOmmString(RSSL_STREAM_UNSPECIFIED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_OPEN.data, rsslStreamStateToOmmString(RSSL_STREAM_OPEN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_NON_STREAMING.data, rsslStreamStateToOmmString(RSSL_STREAM_NON_STREAMING)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_CLOSED_RECOVER.data, rsslStreamStateToOmmString(RSSL_STREAM_CLOSED_RECOVER)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_CLOSED.data, rsslStreamStateToOmmString(RSSL_STREAM_CLOSED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_STREAM_REDIRECTED.data, rsslStreamStateToOmmString(RSSL_STREAM_REDIRECTED)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_UNSPECIFIED.data) == RSSL_OMMSTR_STREAM_UNSPECIFIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_OPEN.data) == RSSL_OMMSTR_STREAM_OPEN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_NON_STREAMING.data) == RSSL_OMMSTR_STREAM_NON_STREAMING.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_CLOSED_RECOVER.data) == RSSL_OMMSTR_STREAM_CLOSED_RECOVER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_CLOSED.data) == RSSL_OMMSTR_STREAM_CLOSED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_STREAM_REDIRECTED.data) == RSSL_OMMSTR_STREAM_REDIRECTED.length);

	/* DataStates */
	ASSERT_TRUE(rsslDataStateToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DATA_NO_CHANGE.data, rsslDataStateToOmmString(RSSL_DATA_NO_CHANGE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DATA_OK.data, rsslDataStateToOmmString(RSSL_DATA_OK)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_DATA_SUSPECT.data, rsslDataStateToOmmString(RSSL_DATA_SUSPECT)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DATA_NO_CHANGE.data) == RSSL_OMMSTR_DATA_NO_CHANGE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DATA_OK.data) == RSSL_OMMSTR_DATA_OK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_DATA_SUSPECT.data) == RSSL_OMMSTR_DATA_SUSPECT.length);

	/* StateCodes */
	ASSERT_TRUE(rsslStateCodeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NONE.data, rsslStateCodeToOmmString(RSSL_SC_NONE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NOT_FOUND.data, rsslStateCodeToOmmString(RSSL_SC_NOT_FOUND)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_TIMEOUT.data, rsslStateCodeToOmmString(RSSL_SC_TIMEOUT)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NOT_ENTITLED.data, rsslStateCodeToOmmString(RSSL_SC_NOT_ENTITLED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_INVALID_ARGUMENT.data, rsslStateCodeToOmmString(RSSL_SC_INVALID_ARGUMENT)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_USAGE_ERROR.data, rsslStateCodeToOmmString(RSSL_SC_USAGE_ERROR)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_PREEMPTED.data, rsslStateCodeToOmmString(RSSL_SC_PREEMPTED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED.data, rsslStateCodeToOmmString(RSSL_SC_JIT_CONFLATION_STARTED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_REALTIME_RESUMED.data, rsslStateCodeToOmmString(RSSL_SC_REALTIME_RESUMED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_FAILOVER_STARTED.data, rsslStateCodeToOmmString(RSSL_SC_FAILOVER_STARTED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_FAILOVER_COMPLETED.data, rsslStateCodeToOmmString(RSSL_SC_FAILOVER_COMPLETED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_GAP_DETECTED.data, rsslStateCodeToOmmString(RSSL_SC_GAP_DETECTED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NO_RESOURCES.data, rsslStateCodeToOmmString(RSSL_SC_NO_RESOURCES)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_TOO_MANY_ITEMS.data, rsslStateCodeToOmmString(RSSL_SC_TOO_MANY_ITEMS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_ALREADY_OPEN.data, rsslStateCodeToOmmString(RSSL_SC_ALREADY_OPEN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_SOURCE_UNKNOWN.data, rsslStateCodeToOmmString(RSSL_SC_SOURCE_UNKNOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NOT_OPEN.data, rsslStateCodeToOmmString(RSSL_SC_NOT_OPEN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NON_UPDATING_ITEM.data, rsslStateCodeToOmmString(RSSL_SC_NON_UPDATING_ITEM)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE.data, rsslStateCodeToOmmString(RSSL_SC_UNSUPPORTED_VIEW_TYPE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_INVALID_VIEW.data, rsslStateCodeToOmmString(RSSL_SC_INVALID_VIEW)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED.data, rsslStateCodeToOmmString(RSSL_SC_FULL_VIEW_PROVIDED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH.data, rsslStateCodeToOmmString(RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ.data, rsslStateCodeToOmmString(RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER.data, rsslStateCodeToOmmString(RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_ERROR.data, rsslStateCodeToOmmString(RSSL_SC_ERROR)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_DACS_DOWN.data, rsslStateCodeToOmmString(RSSL_SC_DACS_DOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS.data, rsslStateCodeToOmmString(RSSL_SC_USER_UNKNOWN_TO_PERM_SYS)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED.data, rsslStateCodeToOmmString(RSSL_SC_DACS_MAX_LOGINS_REACHED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED.data, rsslStateCodeToOmmString(RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_GAP_FILL.data, rsslStateCodeToOmmString(RSSL_SC_GAP_FILL)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED.data, rsslStateCodeToOmmString(RSSL_SC_APP_AUTHORIZATION_FAILED)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NONE.data) == RSSL_OMMSTR_SC_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NOT_FOUND.data) == RSSL_OMMSTR_SC_NOT_FOUND.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_TIMEOUT.data) == RSSL_OMMSTR_SC_TIMEOUT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NOT_ENTITLED.data) == RSSL_OMMSTR_SC_NOT_ENTITLED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_INVALID_ARGUMENT.data) == RSSL_OMMSTR_SC_INVALID_ARGUMENT.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_USAGE_ERROR.data) == RSSL_OMMSTR_SC_USAGE_ERROR.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_PREEMPTED.data) == RSSL_OMMSTR_SC_PREEMPTED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED.data) == RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_REALTIME_RESUMED.data) == RSSL_OMMSTR_SC_REALTIME_RESUMED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_FAILOVER_STARTED.data) == RSSL_OMMSTR_SC_FAILOVER_STARTED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_FAILOVER_COMPLETED.data) == RSSL_OMMSTR_SC_FAILOVER_COMPLETED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_GAP_DETECTED.data) == RSSL_OMMSTR_SC_GAP_DETECTED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NO_RESOURCES.data) == RSSL_OMMSTR_SC_NO_RESOURCES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_TOO_MANY_ITEMS.data) == RSSL_OMMSTR_SC_TOO_MANY_ITEMS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_ALREADY_OPEN.data) == RSSL_OMMSTR_SC_ALREADY_OPEN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_SOURCE_UNKNOWN.data) == RSSL_OMMSTR_SC_SOURCE_UNKNOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NOT_OPEN.data) == RSSL_OMMSTR_SC_NOT_OPEN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NON_UPDATING_ITEM.data) == RSSL_OMMSTR_SC_NON_UPDATING_ITEM.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE.data) == RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_INVALID_VIEW.data) == RSSL_OMMSTR_SC_INVALID_VIEW.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED.data) == RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH.data) == RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ.data) == RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER.data) == RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_ERROR.data) == RSSL_OMMSTR_SC_ERROR.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_DACS_DOWN.data) == RSSL_OMMSTR_SC_DACS_DOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS.data) == RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED.data) == RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED.data) == RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_GAP_FILL.data) == RSSL_OMMSTR_SC_GAP_FILL.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED.data) == RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED.length);

	/* Entry Actions */
	ASSERT_TRUE(rsslMapEntryActionToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MPEA_UPDATE_ENTRY.data, rsslMapEntryActionToOmmString(RSSL_MPEA_UPDATE_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MPEA_ADD_ENTRY.data, rsslMapEntryActionToOmmString(RSSL_MPEA_ADD_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_MPEA_DELETE_ENTRY.data, rsslMapEntryActionToOmmString(RSSL_MPEA_DELETE_ENTRY)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPEA_UPDATE_ENTRY.data) == RSSL_OMMSTR_MPEA_UPDATE_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPEA_ADD_ENTRY.data) == RSSL_OMMSTR_MPEA_ADD_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_MPEA_DELETE_ENTRY.data) == RSSL_OMMSTR_MPEA_DELETE_ENTRY.length);

	ASSERT_TRUE(rsslVectorEntryActionToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_VTEA_UPDATE_ENTRY.data, rsslVectorEntryActionToOmmString(RSSL_VTEA_UPDATE_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_VTEA_SET_ENTRY.data, rsslVectorEntryActionToOmmString(RSSL_VTEA_SET_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_VTEA_CLEAR_ENTRY.data, rsslVectorEntryActionToOmmString(RSSL_VTEA_CLEAR_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_VTEA_INSERT_ENTRY.data, rsslVectorEntryActionToOmmString(RSSL_VTEA_INSERT_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_VTEA_DELETE_ENTRY.data, rsslVectorEntryActionToOmmString(RSSL_VTEA_DELETE_ENTRY)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEA_UPDATE_ENTRY.data) == RSSL_OMMSTR_VTEA_UPDATE_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEA_SET_ENTRY.data) == RSSL_OMMSTR_VTEA_SET_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEA_CLEAR_ENTRY.data) == RSSL_OMMSTR_VTEA_CLEAR_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEA_INSERT_ENTRY.data) == RSSL_OMMSTR_VTEA_INSERT_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_VTEA_DELETE_ENTRY.data) == RSSL_OMMSTR_VTEA_DELETE_ENTRY.length);

	ASSERT_TRUE(rsslFilterEntryActionToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_FTEA_UPDATE_ENTRY.data, rsslFilterEntryActionToOmmString(RSSL_FTEA_UPDATE_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_FTEA_SET_ENTRY.data, rsslFilterEntryActionToOmmString(RSSL_FTEA_SET_ENTRY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_FTEA_CLEAR_ENTRY.data, rsslFilterEntryActionToOmmString(RSSL_FTEA_CLEAR_ENTRY)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTEA_UPDATE_ENTRY.data) == RSSL_OMMSTR_FTEA_UPDATE_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTEA_SET_ENTRY.data) == RSSL_OMMSTR_FTEA_SET_ENTRY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_FTEA_CLEAR_ENTRY.data) == RSSL_OMMSTR_FTEA_CLEAR_ENTRY.length);

	/* Qos Rate/Timeliness */
	ASSERT_TRUE(rsslQosTimelinessToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_TIME_UNSPECIFIED.data, rsslQosTimelinessToOmmString(RSSL_QOS_TIME_UNSPECIFIED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_TIME_REALTIME.data, rsslQosTimelinessToOmmString(RSSL_QOS_TIME_REALTIME)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN.data, rsslQosTimelinessToOmmString(RSSL_QOS_TIME_DELAYED_UNKNOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_TIME_DELAYED.data, rsslQosTimelinessToOmmString(RSSL_QOS_TIME_DELAYED)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_TIME_UNSPECIFIED.data) == RSSL_OMMSTR_QOS_TIME_UNSPECIFIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_TIME_REALTIME.data) == RSSL_OMMSTR_QOS_TIME_REALTIME.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN.data) == RSSL_OMMSTR_QOS_TIME_DELAYED_UNKNOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_TIME_DELAYED.data) == RSSL_OMMSTR_QOS_TIME_DELAYED.length);

	ASSERT_TRUE(rsslQosRateToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_RATE_UNSPECIFIED.data, rsslQosRateToOmmString(RSSL_QOS_RATE_UNSPECIFIED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK.data, rsslQosRateToOmmString(RSSL_QOS_RATE_TICK_BY_TICK)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED.data, rsslQosRateToOmmString(RSSL_QOS_RATE_JIT_CONFLATED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED.data, rsslQosRateToOmmString(RSSL_QOS_RATE_TIME_CONFLATED)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_RATE_UNSPECIFIED.data) == RSSL_OMMSTR_QOS_RATE_UNSPECIFIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK.data) == RSSL_OMMSTR_QOS_RATE_TICK_BY_TICK.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED.data) == RSSL_OMMSTR_QOS_RATE_JIT_CONFLATED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED.data) == RSSL_OMMSTR_QOS_RATE_TIME_CONFLATED.length);

	/* Real hints */
	ASSERT_TRUE(rsslRealHintToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_14.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_14)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_13.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_13)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_12.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_12)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_11.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_11)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_10.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_10)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_9.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_9)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_8.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_7.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_7)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_6.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_6)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_5.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_5)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_4.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_3.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_3)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_2.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_2)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_1.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT_1.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT_1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT1.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT2.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT2)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT3.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT3)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT4.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT5.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT5)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT6.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT6)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_EXPONENT7.data, rsslRealHintToOmmString(RSSL_RH_EXPONENT7)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_1.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_1)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_2.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_2)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_4.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_4)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_8.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_8)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_16.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_16)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_32.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_32)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_64.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_64)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_128.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_128)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_FRACTION_256.data, rsslRealHintToOmmString(RSSL_RH_FRACTION_256)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_INFINITY.data, rsslRealHintToOmmString(RSSL_RH_INFINITY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_NEG_INFINITY.data, rsslRealHintToOmmString(RSSL_RH_NEG_INFINITY)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_RH_NOT_A_NUMBER.data, rsslRealHintToOmmString(RSSL_RH_NOT_A_NUMBER)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_14.data) == RSSL_OMMSTR_RH_EXPONENT_14.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_13.data) == RSSL_OMMSTR_RH_EXPONENT_13.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_12.data) == RSSL_OMMSTR_RH_EXPONENT_12.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_11.data) == RSSL_OMMSTR_RH_EXPONENT_11.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_10.data) == RSSL_OMMSTR_RH_EXPONENT_10.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_9.data) == RSSL_OMMSTR_RH_EXPONENT_9.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_8.data) == RSSL_OMMSTR_RH_EXPONENT_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_7.data) == RSSL_OMMSTR_RH_EXPONENT_7.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_6.data) == RSSL_OMMSTR_RH_EXPONENT_6.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_5.data) == RSSL_OMMSTR_RH_EXPONENT_5.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_4.data) == RSSL_OMMSTR_RH_EXPONENT_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_3.data) == RSSL_OMMSTR_RH_EXPONENT_3.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_2.data) == RSSL_OMMSTR_RH_EXPONENT_2.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_1.data) == RSSL_OMMSTR_RH_EXPONENT_1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT_1.data) == RSSL_OMMSTR_RH_EXPONENT_1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT1.data) == RSSL_OMMSTR_RH_EXPONENT1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT2.data) == RSSL_OMMSTR_RH_EXPONENT2.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT3.data) == RSSL_OMMSTR_RH_EXPONENT3.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT4.data) == RSSL_OMMSTR_RH_EXPONENT4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT5.data) == RSSL_OMMSTR_RH_EXPONENT5.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT6.data) == RSSL_OMMSTR_RH_EXPONENT6.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_EXPONENT7.data) == RSSL_OMMSTR_RH_EXPONENT7.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_1.data) == RSSL_OMMSTR_RH_FRACTION_1.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_2.data) == RSSL_OMMSTR_RH_FRACTION_2.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_4.data) == RSSL_OMMSTR_RH_FRACTION_4.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_8.data) == RSSL_OMMSTR_RH_FRACTION_8.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_16.data) == RSSL_OMMSTR_RH_FRACTION_16.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_32.data) == RSSL_OMMSTR_RH_FRACTION_32.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_64.data) == RSSL_OMMSTR_RH_FRACTION_64.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_128.data) == RSSL_OMMSTR_RH_FRACTION_128.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_FRACTION_256.data) == RSSL_OMMSTR_RH_FRACTION_256.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_INFINITY.data) == RSSL_OMMSTR_RH_INFINITY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_NEG_INFINITY.data) == RSSL_OMMSTR_RH_NEG_INFINITY.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_RH_NOT_A_NUMBER.data) == RSSL_OMMSTR_RH_NOT_A_NUMBER.length);

	/* Nak Codes */
	ASSERT_TRUE(rsslNakCodeToOmmString(8) == NULL);
	ASSERT_TRUE(rsslNakCodeToOmmString(9) == NULL);
	ASSERT_TRUE(rsslNakCodeToOmmString(128) == NULL);
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_NONE.data, rsslNakCodeToOmmString(RSSL_NAKC_NONE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_ACCESS_DENIED.data, rsslNakCodeToOmmString(RSSL_NAKC_ACCESS_DENIED)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_DENIED_BY_SRC.data, rsslNakCodeToOmmString(RSSL_NAKC_DENIED_BY_SRC)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_SOURCE_DOWN.data, rsslNakCodeToOmmString(RSSL_NAKC_SOURCE_DOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN.data, rsslNakCodeToOmmString(RSSL_NAKC_SOURCE_UNKNOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_NO_RESOURCES.data, rsslNakCodeToOmmString(RSSL_NAKC_NO_RESOURCES)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_NO_RESPONSE.data, rsslNakCodeToOmmString(RSSL_NAKC_NO_RESPONSE)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_GATEWAY_DOWN.data, rsslNakCodeToOmmString(RSSL_NAKC_GATEWAY_DOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN.data, rsslNakCodeToOmmString(RSSL_NAKC_SYMBOL_UNKNOWN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_NOT_OPEN.data, rsslNakCodeToOmmString(RSSL_NAKC_NOT_OPEN)));
	ASSERT_TRUE(0 == strcmp(RSSL_OMMSTR_NAKC_INVALID_CONTENT.data, rsslNakCodeToOmmString(RSSL_NAKC_INVALID_CONTENT)));

	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_NONE.data) == RSSL_OMMSTR_NAKC_NONE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_ACCESS_DENIED.data) == RSSL_OMMSTR_NAKC_ACCESS_DENIED.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_DENIED_BY_SRC.data) == RSSL_OMMSTR_NAKC_DENIED_BY_SRC.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_SOURCE_DOWN.data) == RSSL_OMMSTR_NAKC_SOURCE_DOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN.data) == RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_NO_RESOURCES.data) == RSSL_OMMSTR_NAKC_NO_RESOURCES.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_NO_RESPONSE.data) == RSSL_OMMSTR_NAKC_NO_RESPONSE.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_GATEWAY_DOWN.data) == RSSL_OMMSTR_NAKC_GATEWAY_DOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN.data) == RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_NOT_OPEN.data) == RSSL_OMMSTR_NAKC_NOT_OPEN.length);
	ASSERT_TRUE((RsslUInt32)strlen(RSSL_OMMSTR_NAKC_INVALID_CONTENT.data) == RSSL_OMMSTR_NAKC_INVALID_CONTENT.length);

	//printf("Done.\n\n");
}


/* masks to be used in the tests */
/* Cast as UInt32 so they can be set up with a common function. 32 should be big enough */

RsslUInt32 ackMasksAll[] =
{
	RSSL_AKMF_HAS_NAK_CODE,
	RSSL_AKMF_HAS_TEXT,
	RSSL_AKMF_HAS_EXTENDED_HEADER,
	RSSL_AKMF_PRIVATE_STREAM,
	RSSL_AKMF_HAS_SEQ_NUM,
	RSSL_AKMF_HAS_MSG_KEY
};

RsslUInt32 ackMasksCommon[] =
{
	RSSL_AKMF_HAS_NAK_CODE, /*MSG_AKM_MSK_HAS_NAK_CODE | MSG_AKM_MSK_HAS_TEXT, */
	RSSL_AKMF_HAS_TEXT,
	RSSL_AKMF_HAS_MSG_KEY,
	RSSL_AKMF_HAS_EXTENDED_HEADER,
	RSSL_AKMF_QUALIFIED_STREAM
};

RsslUInt32 closeMasksAll[] =
{
	RSSL_CLMF_HAS_EXTENDED_HEADER,
	RSSL_CLMF_ACK, 
	RSSL_CLMF_HAS_BATCH
};

RsslUInt32 closeMasksCommon[] =
{
	RSSL_CLMF_HAS_EXTENDED_HEADER
};

RsslUInt32 genericMasksAll[] =
{
	RSSL_GNMF_HAS_EXTENDED_HEADER,
	RSSL_GNMF_HAS_PERM_DATA,
	RSSL_GNMF_HAS_MSG_KEY,
	RSSL_GNMF_HAS_SEQ_NUM,
	RSSL_GNMF_MESSAGE_COMPLETE,
	RSSL_GNMF_HAS_SECONDARY_SEQ_NUM,
	RSSL_GNMF_HAS_PART_NUM,
	RSSL_GNMF_HAS_REQ_MSG_KEY
};

RsslUInt32 genericMasksCommon[] =
{
	RSSL_GNMF_MESSAGE_COMPLETE,
	RSSL_GNMF_HAS_MSG_KEY,
	RSSL_GNMF_HAS_PERM_DATA,
	RSSL_GNMF_HAS_SEQ_NUM,
	RSSL_GNMF_HAS_EXTENDED_HEADER
};

/* Rssl doesn't have Group message -- status msg takes that role */

RsslUInt32 requestMasksAll[] =
{
	RSSL_RQMF_HAS_EXTENDED_HEADER,
	RSSL_RQMF_HAS_PRIORITY,
	RSSL_RQMF_STREAMING,
	RSSL_RQMF_MSG_KEY_IN_UPDATES,
 	RSSL_RQMF_CONF_INFO_IN_UPDATES,
	RSSL_RQMF_NO_REFRESH,
	RSSL_RQMF_HAS_QOS,
	RSSL_RQMF_HAS_WORST_QOS,
	RSSL_RQMF_PRIVATE_STREAM,
	RSSL_RQMF_QUALIFIED_STREAM
};

RsslUInt32 requestMasksCommon[] =
{
	RSSL_RQMF_NO_REFRESH,
	RSSL_RQMF_STREAMING,
	RSSL_RQMF_HAS_PRIORITY,
	RSSL_RQMF_HAS_QOS,
	RSSL_RQMF_HAS_WORST_QOS,
	RSSL_RQMF_HAS_EXTENDED_HEADER,
	RSSL_RQMF_MSG_KEY_IN_UPDATES,
	RSSL_RQMF_CONF_INFO_IN_UPDATES
};

RsslUInt32 responseMasksAll[] =
{
	RSSL_RFMF_HAS_EXTENDED_HEADER,
	RSSL_RFMF_HAS_PERM_DATA,
	RSSL_RFMF_HAS_MSG_KEY,
	RSSL_RFMF_HAS_SEQ_NUM,
	RSSL_RFMF_SOLICITED,
	RSSL_RFMF_REFRESH_COMPLETE,
	RSSL_RFMF_HAS_QOS,
	RSSL_RFMF_CLEAR_CACHE,
	RSSL_RFMF_DO_NOT_CACHE,
	RSSL_RFMF_PRIVATE_STREAM,
	RSSL_RFMF_HAS_POST_USER_INFO,
	RSSL_RFMF_HAS_PART_NUM,
	RSSL_RFMF_HAS_REQ_MSG_KEY,
	RSSL_RFMF_QUALIFIED_STREAM
};

RsslUInt32 responseMasksCommon[] =
{
	RSSL_RFMF_DO_NOT_CACHE,
	RSSL_RFMF_CLEAR_CACHE,
	RSSL_RFMF_REFRESH_COMPLETE,
	RSSL_RFMF_HAS_MSG_KEY,
	RSSL_RFMF_HAS_PERM_DATA,
	RSSL_RFMF_HAS_SEQ_NUM,
	RSSL_RFMF_HAS_QOS,
	RSSL_RFMF_HAS_EXTENDED_HEADER
};

RsslUInt32 statusMasksAll[] =
{
	RSSL_STMF_HAS_EXTENDED_HEADER,
	RSSL_STMF_HAS_PERM_DATA,
	RSSL_STMF_HAS_MSG_KEY,
	RSSL_STMF_HAS_GROUP_ID,
	RSSL_STMF_HAS_STATE,
	RSSL_STMF_CLEAR_CACHE,
	RSSL_STMF_PRIVATE_STREAM,
	RSSL_STMF_HAS_POST_USER_INFO,
	RSSL_STMF_HAS_REQ_MSG_KEY,
	RSSL_STMF_QUALIFIED_STREAM
};

RsslUInt32 statusMasksCommon[] =
{
	RSSL_STMF_CLEAR_CACHE,
	RSSL_STMF_HAS_STATE,
	RSSL_STMF_HAS_PERM_DATA,
	RSSL_STMF_HAS_EXTENDED_HEADER
};

RsslUInt32 updateMasksAll[] =
{
	RSSL_UPMF_HAS_EXTENDED_HEADER,
	RSSL_UPMF_HAS_PERM_DATA,
	RSSL_UPMF_HAS_MSG_KEY,
	RSSL_UPMF_HAS_SEQ_NUM,
	RSSL_UPMF_HAS_CONF_INFO,
	RSSL_UPMF_DO_NOT_CACHE,
	RSSL_UPMF_DO_NOT_CONFLATE,
	RSSL_UPMF_DO_NOT_RIPPLE,
	RSSL_UPMF_HAS_POST_USER_INFO,
	RSSL_UPMF_DISCARDABLE
};

RsslUInt32 updateMasksCommon[] = 
{
	RSSL_UPMF_DO_NOT_RIPPLE,
	RSSL_UPMF_DO_NOT_CACHE,
	RSSL_UPMF_HAS_MSG_KEY,
	RSSL_UPMF_HAS_PERM_DATA, 
	RSSL_UPMF_HAS_SEQ_NUM,
	RSSL_UPMF_HAS_CONF_INFO
};

RsslUInt32 postMasksAll[] =
{
	RSSL_PSMF_HAS_EXTENDED_HEADER,
	RSSL_PSMF_HAS_POST_ID,
	RSSL_PSMF_HAS_MSG_KEY,
    RSSL_PSMF_HAS_SEQ_NUM,
   	RSSL_PSMF_POST_COMPLETE,
	RSSL_PSMF_ACK,
	RSSL_PSMF_HAS_PERM_DATA,
	RSSL_PSMF_HAS_PART_NUM,
	RSSL_PSMF_HAS_POST_USER_RIGHTS
};


RsslUInt32 postMasksCommon[] =
{
	RSSL_PSMF_HAS_POST_ID,
	RSSL_PSMF_HAS_MSG_KEY,
    RSSL_PSMF_ACK,
	RSSL_PSMF_HAS_PERM_DATA,
	RSSL_PSMF_HAS_POST_USER_RIGHTS
};

RsslUInt32 actionsAll[] =
{
	TEST_ACTION_POST_PAYLOAD,
	TEST_ACTION_PRE_PAYLOAD,
	TEST_ACTION_4BYTE_SEQ,
	TEST_ACTION_TRIM_DATA_BUF,
	TEST_ACTION_POST_OPAQUE,
	TEST_ACTION_POST_EXTENDED,
	TEST_ACTION_POST_REQKEY
};

RsslUInt32 actionsCommon[] =
{
	TEST_ACTION_POST_PAYLOAD,
	TEST_ACTION_PRE_PAYLOAD,
	TEST_ACTION_4BYTE_SEQ,
	TEST_ACTION_POST_OPAQUE,
	TEST_ACTION_POST_EXTENDED,
	TEST_ACTION_POST_REQKEY
};

RsslUInt32 keyMasksAll[] =
{
	RSSL_MKF_HAS_SERVICE_ID,
	RSSL_MKF_HAS_NAME,
	RSSL_MKF_HAS_NAME_TYPE,
	RSSL_MKF_HAS_FILTER,
	RSSL_MKF_HAS_IDENTIFIER,
	RSSL_MKF_HAS_ATTRIB
};

RsslUInt32 keyMasksCommon[] =
{
	RSSL_MKF_HAS_SERVICE_ID,
	RSSL_MKF_HAS_NAME,
	RSSL_MKF_HAS_NAME_TYPE,
	RSSL_MKF_HAS_ATTRIB
};

/***** Main *****/

const char
	*argPerfMode = "--perf",
	*argNoDecodeTests = "--no-decode",
	*argNestedMsg = "--nested-msg",
	*argChangeData = "--change-data";
	
RsslUInt32 repeatCount = 150;

TEST(noDataTest, noDataTest)
{
	/* only the message is prone to an issue where using encodeMsgInit/Complete 
	 * may still allow user to encode payload.  */
  RsslUpdateMsg updMsg;
	RsslBuffer tempBuf;
	char space[1024];

	RsslEncodeIterator encIter;
	int i;
	/* 0x8 is key, 0x1 is eh */
	int noDataFlags[] = {0, 8, 1, 9, 8, 1, 9, 9, 9};

	tempBuf.data = space;
	tempBuf.length = 1024;

	/* want to make sure this is ok when we have key and extended header as well */
	/* run 0 is just msg
	 * run 1 is msg and preenc key
	 * run 2 is msg and preenc eh
	 * run 3 is msg, preenc key, and preenc eh
	 * run 4 is msg, key 
	 * run 5 is msg, eh
	 * run 6 is msg, preenc key, eh
	 * run 7 is msg, key, preenc eh 
	 * run 8 is msg, key, eh 
	 */
	for (i = 0; i < 9; i++)
	{
		rsslClearUpdateMsg(&updMsg);
		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, &tempBuf);
		updMsg.msgBase.msgClass = RSSL_MC_UPDATE;
		updMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		updMsg.msgBase.streamId = 1;
		updMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		updMsg.flags = noDataFlags[i];
		updMsg.updateType = 3;


		if ((updMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER) && (i != 6 && i != 5 && i != 8))
		{
			updMsg.extendedHeader.length = extendedHeaderLen;
			updMsg.extendedHeader.data = extendedHeader;
		}

		updMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_SERVICE_ID | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_FILTER | RSSL_MKF_HAS_IDENTIFIER | RSSL_MKF_HAS_ATTRIB;
		if (updMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
		{
			if (i != 4 && i != 7 && i != 8)
				_encodeMsgKey(&updMsg.msgBase.msgKey, msgKeyAttrib, msgKeyAttribLen);
			else
				_encodeMsgKeyNoAttrib(&updMsg.msgBase.msgKey);
		}
		
		/* everything in these cases is pre-encoded.  This should tell us success */
		if (i < 4)  
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgInit(&encIter, (RsslMsg*)&updMsg, 0)); //rsslEncodeMsgInit, pre-enc

		if (i == 4 || i == 7 || i == 8)  /* key opaque is next */
			ASSERT_TRUE(RSSL_RET_ENCODE_MSG_KEY_OPAQUE == rsslEncodeMsgInit(&encIter, (RsslMsg*)&updMsg, 0)); //rsslEncodeMsgInit, key is next

		if (i == 5 || i == 6)
			ASSERT_TRUE(RSSL_RET_ENCODE_EXTENDED_HEADER == rsslEncodeMsgInit(&encIter, (RsslMsg*)&updMsg, 0)); //rsslEncodeMsgInit, ext header is next

		if (i == 4 || i == 7 || i == 8) 
		{
			/* need to encode key opaque */
			_encodeFieldList(&encIter);
			
			if (i == 8) /* need to do extended header as well  */
				ASSERT_TRUE( RSSL_RET_ENCODE_EXTENDED_HEADER == rsslEncodeMsgKeyAttribComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgKeyAttribComplete, eh is next
			
			if (i == 4 || i == 7)
				ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeMsgKeyAttribComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgKeyAttribComplete
		}

		if (i == 5 || i == 6 || i == 8)
		{
			/* need to encode extended header */
			RsslBuffer buffer = RSSL_INIT_BUFFER;

			/* we must encode our extended header now */
			/* just hack copy it onto the wire */
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeNonRWFDataTypeInit(&encIter, &buffer)); //rsslEncodeNonRWFDataTypeInit
			ASSERT_TRUE(buffer.length >= extendedHeaderLen); //Length Failure
			MemCopyByInt(buffer.data, extendedHeader, extendedHeaderLen);
			buffer.length = extendedHeaderLen;
			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeNonRWFDataTypeComplete(&encIter, &buffer, RSSL_TRUE)); //rsslEncodeNonRWFDataTypeComplete

			ASSERT_TRUE(RSSL_RET_SUCCESS == rsslEncodeExtendedHeaderComplete(&encIter, RSSL_TRUE)); //rsslEncodeExtendedHeaderComplete
		}

		/* now try to encode a field list */

		_encodeFieldList(&encIter);

		ASSERT_TRUE(RSSL_RET_INVALID_DATA == rsslEncodeMsgComplete(&encIter, RSSL_TRUE)); //rsslEncodeMsgComplete
	}
}

TEST(simpleRollback, simpleRollback)
{

	RsslMap rsslMap = RSSL_INIT_MAP;
	RsslUInt8 count =0;
	RsslEncodeIterator encIter = RSSL_INIT_ENCODE_ITERATOR;
	RsslDecodeIterator decIter = RSSL_INIT_DECODE_ITERATOR;
	RsslMapEntry mapEnt = RSSL_INIT_MAP_ENTRY;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslBuffer tempBuffer;
	RsslMsg decMsg;
	RsslInt8 retVal = 0;
	RsslMap decMap = RSSL_INIT_MAP;
	RsslMapEntry decMapEntry = RSSL_INIT_MAP_ENTRY;
	RsslInt mapKey = 0;
	char workspace[500];

	tempBuffer.data = workspace;
	tempBuffer.length = 500;
	rsslSetEncodeIteratorBuffer(&encIter, &tempBuffer);
	
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	refreshMsg.msgBase.streamId = 10;
	refreshMsg.msgBase.containerType = RSSL_DT_MAP;
	refreshMsg.flags = RSSL_RFMF_REFRESH_COMPLETE | RSSL_RFMF_SOLICITED;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.code = RSSL_SC_NONE;
	refreshMsg.state.text.length = 0;

	rsslEncodeMsgInit(&encIter, (RsslMsg*)&refreshMsg, 0);
	//	printf("failed rollback encodeMsgInit\n");

	rsslMap.containerType = RSSL_DT_NO_DATA;
	rsslMap.keyPrimitiveType = RSSL_DT_INT;
	rsslMap.flags = 0;
	rsslEncodeMapInit(&encIter, &rsslMap, 0, 0);
	//	printf("failed rollback encodeMapInit\n");
	
	mapKey = 1;
	mapEnt.flags = 0;
	mapEnt.action = RSSL_MPEA_ADD_ENTRY;
	rsslEncodeMapEntryInit(&encIter, &mapEnt, &mapKey, 0);
	//	printf("encodeMapEntryInit failed\n");

	rsslEncodeMapEntryComplete(&encIter, RSSL_TRUE);
	//	printf("encodeMapEntryComplete failed\n");

	mapKey = 2;
	/* roll back second entry */
	rsslEncodeMapEntryInit(&encIter, &mapEnt, &mapKey, 0);
	//	printf("encodeMapEntryInit failed\n");

	rsslEncodeMapEntryComplete(&encIter, RSSL_FALSE);
	//	printf("encodeMapEntryComplete failed\n");

	rsslEncodeMapComplete(&encIter, RSSL_TRUE);
	//	printf("encodeMapComplete failed\n");

	rsslEncodeMsgComplete(&encIter, RSSL_TRUE);
	//	printf("encodeMsgComplete failed\n");

	tempBuffer.length = rsslGetEncodedBufferLength(&encIter);

	rsslSetDecodeIteratorBuffer(&decIter, &tempBuffer);

	/* now decode */
	rsslDecodeMsg(&decIter, &decMsg);
	//	printf("DecodeMsg failed\n");

	rsslDecodeMap(&decIter, &decMap);
	//	printf("DecodeMap failed\n");

	while ((retVal = rsslDecodeMapEntry(&decIter, &decMapEntry, &mapKey)) != RSSL_RET_END_OF_CONTAINER)
	{
		//if (retVal < RSSL_RET_SUCCESS)
		//	printf("decodeMapEntry failed\n");

		count++;

	}

	//if (count > 1)
	//	printf("too many map entries in map for rollback test\n");
}

TEST(ommStringTest,ommStringTest)
{
	ommStringTest();
}

TEST(initCommonTests, initCommonTests)
{
	actionsSize = _allocateFlagCombinations(&actions, actionsCommon, sizeof(actionsCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	keyMasksSize = _allocateFlagCombinations(&keyMasks, keyMasksCommon, sizeof(keyMasksCommon)/sizeof(RsslUInt32), RSSL_TRUE);
}

TEST(ackMsgTestCommon, ackMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, ackMasksCommon, sizeof(ackMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	ackMsgTest(repeatCount);
	free(masks);
}

TEST(closeMsgTestCommon, closeMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, closeMasksCommon, sizeof(closeMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	closeMsgTest(repeatCount);
	free(masks);
}

TEST(genericMsgTestCommon, genericMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, genericMasksCommon, sizeof(genericMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	genericMsgTest(repeatCount);
	free(masks);
}

TEST(postMsgTestCommon, postMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, postMasksCommon, sizeof(postMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	postMsgTest(repeatCount);
	free(masks);
}

TEST(statusMsgTestCommon, statusMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, statusMasksCommon, sizeof(statusMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	statusMsgTest(repeatCount);
	free(masks);
}

TEST(refreshMsgTestCommon, refreshMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, responseMasksCommon, sizeof(responseMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	refreshMsgTest(repeatCount);
	free(masks);
}

TEST(requestMsgTestCommon, requestMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, requestMasksCommon, sizeof(requestMasksCommon)/sizeof(RsslUInt32), RSSL_TRUE);
	requestMsgTest(repeatCount);
	free(masks);
}

TEST(updateMsgTestCommon, updateMsgTestCommon)
{
	masksSize = _allocateFlagCombinations(&masks, updateMasksCommon, sizeof(updateMasksCommon)/sizeof(RsslUInt32), RSSL_FALSE);
	updateMsgTest(3*repeatCount);
	free(masks);
}

TEST(closeCommonTests, closeCommonTests)
{
	free(actions);
	free(keyMasks);
}

TEST(initFullTests, initFullTests)
{
	actionsSize = _allocateFlagCombinations(&actions, actionsAll, sizeof(actionsAll)/sizeof(RsslUInt32), RSSL_FALSE);
	keyMasksSize = _allocateFlagCombinations(&keyMasks, keyMasksAll, sizeof(keyMasksAll)/sizeof(RsslUInt32), RSSL_TRUE);
}

TEST(ackMsgTestFull, ackMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, ackMasksAll, sizeof(ackMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	ackMsgTest(1);
	free(masks);
}

TEST(closeMsgTestFull, closeMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, closeMasksAll, sizeof(closeMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	closeMsgTest(1);
	free(masks);
}
TEST(postMsgTestFull, postMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, postMasksAll, sizeof(postMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	postMsgTest(1);
	free(masks);
}

TEST(genericMsgTestFull, genericMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, genericMasksAll, sizeof(genericMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	genericMsgTest(1);
	free(masks);
}

TEST(statusMsgTestFull, statusMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, statusMasksAll, sizeof(statusMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	statusMsgTest(1);
	free(masks);
}

TEST(refreshMsgTestFull, refreshMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, responseMasksAll, sizeof(responseMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	refreshMsgTest(1);
	free(masks);
}

TEST(requestMsgTestFull, requestMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, requestMasksAll, sizeof(requestMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	requestMsgTest(1);
	free(masks);
}

TEST(updateMsgTestFull, updateMsgTestFull)
{
	masksSize = _allocateFlagCombinations(&masks, updateMasksAll, sizeof(updateMasksAll)/sizeof(RsslUInt32), RSSL_FALSE);
	updateMsgTest(1);
	free(masks);
}

TEST(closeFullTests, closeFullTests)
{
	free(actions);
	free(keyMasks);
}

TEST(copyKeyTest, copyKeyTest)
{
	copyKeyTest();
}

TEST(copyMsgTest, copyMsgTest)
{
	copyMsgTest();
}

TEST(clearMemSetTest, clearMemSetTest)
{
	clearMemSetTest();
}

int main(int argc, char* argv[])
{
	/* repeat count for Common tests -- helps lessen the impact of any kind of cache miss on the results */
	RsslUInt8 iArgs;
	RsslLibraryVersionInfo libVer = RSSL_INIT_LIBRARY_VERSION_INFO;
	int ret;

	/* Parse arguments */
	g_reportMode = TEST_REPORT_FAIL;
	g_testDecoding = RSSL_TRUE;
	g_dataFormat = RSSL_DT_FIELD_LIST;
	
	
	for ( iArgs = 0; iArgs < argc; ++iArgs)
	{
		if ( 0 == strcmp( argPerfMode, argv[iArgs]))
			g_reportMode = TEST_PERF_MODE;
		else if (0 == strcmp(argNoDecodeTests, argv[iArgs]))
			g_testDecoding = RSSL_FALSE;
		else if (0 == strcmp(argNestedMsg, argv[iArgs]))
			g_dataFormat = RSSL_DT_MSG;
		else if(0 == strcmp(argChangeData, argv[iArgs]))
			g_changeData = RSSL_TRUE;
	}

	//printf( "[%s] Skip Decoding Tests(%s) \n[%s] Perf Test(%s) \n[%s] Nested Msg Payload(%s) \n[%s] Change encoded data w/ helper functions(e.g. seqNum)(%s) \n\n",
	//	(!g_testDecoding) ? "On " : "Off", argNoDecodeTests,
	//	g_reportMode == TEST_PERF_MODE ? "On " : "Off", argPerfMode,
	//	g_dataFormat == RSSL_DT_MSG ? "On" : "Off", argNestedMsg,
	//	g_changeData ? "On " : "Off", argChangeData
	//	);

	setFrequency(g_freq);

	//printf("Timer Accuracy Check (using sleep)\n");
	//printf("\t500ms:");		
	sleepTest(500);
	//printf("\t250ms:");		
	sleepTest(250);
	//printf("\t125ms:");		
	sleepTest(125);
	//printf("\t5ms:");			
	sleepTest(5);

	//printf("\n");


	rsslQueryMessagesLibraryVersion(&libVer);
	//printf("Messages Library Version:\n \t%s\n \t%s\n \t%s\n", libVer.productVersion, libVer.internalVersion, libVer.productDate);

	/* Initialize global members */
	rsslClearMsg(&msg);

	encBuf.data = (char*)malloc(c_TestMsgBufferSize*sizeof(char));
	encBuf.length = c_TestMsgBufferSize;

	encDataBuf.data = (char*)malloc(c_TestDataBufferSize*sizeof(char));
	encDataBuf.length = c_TestDataBufferSize;

	copyMsgBuf.data = (char*)malloc(c_TestMsgCopyBufferSize*sizeof(char));
	copyMsgBuf.length = c_TestMsgCopyBufferSize;

	/* Encode Payload Data for pre-encoded data tests( a nested msg or fieldList, depending on cmdline )*/
	_setupEncodeDataIterator();
	_encodePayload(&encDataIter);
	
	::testing::InitGoogleTest(&argc, argv);
	ret = RUN_ALL_TESTS();

	free(encBuf.data);
	free(encDataBuf.data);
	free(copyMsgBuf.data);

	return ret;
}
