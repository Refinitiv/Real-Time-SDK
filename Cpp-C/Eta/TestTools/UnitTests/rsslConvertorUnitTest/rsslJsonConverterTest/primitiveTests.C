/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslJsonConverterTestBase.h"
#include <limits.h>
#include <math.h>

#include <stdlib.h>

#include <float.h>
#include <ctype.h>
#include <rtr/rtratoi.h>

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

/* Suppress warning C4756: overflow in constant arithmetic that occurs only on VS2013 */
#if defined(WIN32) &&  _MSC_VER == 1800
#pragma warning( disable : 4056 4756)
#endif

#ifdef WIN32
	#ifndef INFINITY
		#define INFINITY HUGE_VAL
		#define HUGE_VALF  ((float)INFINITY)
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -HUGE_VAL
		#endif
	#else
		#ifndef NEG_INFINITY
			#define NEG_INFINITY -INFINITY
		#endif
	#endif
	#ifndef NAN
        static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
        #define NAN (*(const double *) __nan)
    #endif
#else
	#define NEG_INFINITY -INFINITY
#endif

#if defined(WIN32) &&  _MSC_VER == 1700
#define isnan(x) _isnan(x)
#endif

using namespace std;
using namespace json; 

/* Fixture for PrimitiveTests that has conversion code. */
class PrimitiveTests : public MsgConversionTestBase
{
};

/* Parameters for Real string values. */
class RealStringValueTestParams
{
	public:

	RsslRealHints realHint; /* The value used in the RsslReal. */
	const char * stringValue; /* The equivalent string of the value used. */

	RealStringValueTestParams(RsslRealHints realHint, const char *stringValue)
	{
		this->realHint = realHint;
		this->stringValue = stringValue;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const RealStringValueTestParams& params)
	{
		out << "[Value:" << params.stringValue << "]";
		return out;
	}
};

class RealStringValueTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RealStringValueTestParams>
{
};

/* Test conversion of string values used in JSON for Reals, e.g. Inf, -Inf, and NaN. */
TEST_P(RealStringValueTestFixture, RealStringValueTests)
{
	RealStringValueTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslReal real, decodeReal;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	
	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	/* Encode a field list with different RsslReals. */
	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));

	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 22; /* BID */
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = params.realHint;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &real));

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FieldList. */
	ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
	ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

	/* Check correct string value for the real. */
	ASSERT_TRUE(_jsonDocument["Fields"].HasMember("BID"));
	ASSERT_TRUE(_jsonDocument["Fields"]["BID"].IsString());
	EXPECT_STREQ(params.stringValue, _jsonDocument["Fields"]["BID"].GetString());

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

	/* Check correct value of real. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(&_dIter, &decodeReal));
	EXPECT_EQ(RSSL_FALSE, decodeReal.isBlank);
	EXPECT_EQ(params.realHint, decodeReal.hint);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

/* Test cases for real string values. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, RealStringValueTestFixture, ::testing::Values(
	RealStringValueTestParams(RSSL_RH_INFINITY, "Inf"),
	RealStringValueTestParams(RSSL_RH_NEG_INFINITY, "-Inf"),
	RealStringValueTestParams(RSSL_RH_NOT_A_NUMBER, "NaN")
));


/* Parameters for Rmtes string values. */
class RmtesStringTestParams
{
	public:

	RsslBuffer rmtesBuffer; /* RMTES buffer. */
	RsslBuffer utf8Buffer; /* Corresponding UTF8 buffer (utf8Buffer.data will be NULL if the converted value in JSON should be null) */
	RsslBuffer convertedUtf8Buffer; /* Corresponding UTF8 buffer after json->rwf conversion (utf8Buffer.data will be NULL if the converted value in JSON should be null) */

	RmtesStringTestParams(RsslBuffer rmtesBuffer, RsslBuffer utf8Buffer, RsslBuffer convertedUtf8Buffer)
	{
		this->rmtesBuffer = rmtesBuffer;
		this->utf8Buffer = utf8Buffer;
		this->convertedUtf8Buffer = convertedUtf8Buffer;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const RmtesStringTestParams& params)
	{
		if (params.rmtesBuffer.data != NULL)
			out << "[Value (may include non-displayable characters or null-terminators) : Rmtes: \"" << params.rmtesBuffer.data;
		else
			out << "[Value (may include non-displayable characters or null-terminators) : Rmtes: \"NULL";
		
		if(params.utf8Buffer.data != NULL)
			out << "\", Utf8: \"" << params.utf8Buffer.data << "\"]";
		else
			out << "\", Utf8: \"NULL\"]";

		return out;
	}
};

class RmtesStringTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RmtesStringTestParams>
{
};


/* Test a field list that encodes different Rmtes Strings. */
TEST_P(RmtesStringTestFixture, RmtesStringTest)
{
	RmtesStringTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	
	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));
	
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = RMTES_STRING_FIELD.fieldId;
	fieldEntry.dataType = RSSL_DT_RMTES_STRING;
	fieldEntry.encData = params.rmtesBuffer;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FieldList. */
	ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
	ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

	/* Check RMTES string field. */
	ASSERT_TRUE(_jsonDocument["Fields"].HasMember(RMTES_STRING_FIELD.fieldName.data));

	if (params.utf8Buffer.data == NULL)
		EXPECT_TRUE(_jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].IsNull());
	else
	{
		ASSERT_TRUE(_jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].IsString());
		EXPECT_STREQ(params.utf8Buffer.data, _jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].GetString());
	}

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the sample string. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, RMTES_STRING_FIELD.fieldId);

	RsslBuffer decodeBuffer;
	if (params.utf8Buffer.data == NULL)
		EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
	else
	{
		EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
		/* Conversion from UTF8 to RMTES not available, so we should see the UTF8 string again. */
		EXPECT_TRUE(rsslBufferIsEqual(&params.convertedUtf8Buffer, &decodeBuffer));
	}
}

const unsigned char partialUpdate1[] = { 0x1B, 0x5B, '1', '0', 0x60, 'm', 'n', 'o', '\0'};
const unsigned char partialUpdate2[] = { 'a', 'b', 'c', 0x1B, 0x5B, '2', 0x60, 'd', 'e', 0x1B, 0x5B, '3', 0x62, '\0' };

const unsigned char specialChar[] = { '0', '\\', '\b', '\f', '\n', '\r', '\t', 0x1B, 0x5B, '2', 0x60, '/', '\0' };



const RsslBuffer rmtesStringBuffers[][3] =
{
	/* First buffer is the Rmtes string, second is the corresponding Utf8 string */

	/* Value is Ascii-only. UTF8 string should be identical */
	{{ 4, (char*)"TEST" }, {4, (char*)"TEST"}, {4, (char*)"TEST"} },

	/* RmtesString is blank. Utf8 result should be null*/
	{{ 0, (char*)"" }, {0, (char*)NULL}, {0, (char*)NULL}},

	/* RmtesString contains only null bytes. Those will be discarded, so Utf8 result should be null. */
	{{ 4, (char*)"\0\0\0\0" }, {0, (char*)NULL}, {0, (char*)NULL}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 8, (char*)"TEST\0\0\0\0" }, {4, (char*)"TEST"}, {4, (char*)"TEST"}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 12, (char*)"TEST\0\0\0\0TEST" }, {8, (char*)"TESTTEST"},  {8, (char*)"TESTTEST"}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 8, (char*)"\0\0\0\0TEST" }, {4, (char*)"TEST"}, {4, (char*)"TEST"}},

	/* Value contains a partial update. UTF8 should convert it to \u001b. */
	{ { 8, (char*)partialUpdate1 }, {8, (char*)partialUpdate1}, { 8, (char*)partialUpdate1 }},

	/* Value contains a partial update. UTF8 should convert it to \u001b. */
	{ { 13, (char*)partialUpdate2 }, {13, (char*)partialUpdate2}, { 13, (char*)partialUpdate2 }},
	/* Value contains special characters */
	{ { 12, (char*)specialChar }, {12, (char*)specialChar}, { 12, (char*)specialChar }}
};

/* Test cases for Rmtes strings. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, RmtesStringTestFixture, ::testing::Values(
	RmtesStringTestParams(rmtesStringBuffers[0][0], rmtesStringBuffers[0][1], rmtesStringBuffers[0][2]),
	RmtesStringTestParams(rmtesStringBuffers[1][0], rmtesStringBuffers[1][1], rmtesStringBuffers[1][2]),
	RmtesStringTestParams(rmtesStringBuffers[2][0], rmtesStringBuffers[2][1], rmtesStringBuffers[2][2]),
	RmtesStringTestParams(rmtesStringBuffers[3][0], rmtesStringBuffers[3][1], rmtesStringBuffers[3][2]),
	RmtesStringTestParams(rmtesStringBuffers[4][0], rmtesStringBuffers[4][1], rmtesStringBuffers[4][2]),
	RmtesStringTestParams(rmtesStringBuffers[5][0], rmtesStringBuffers[5][1], rmtesStringBuffers[5][2]),
	RmtesStringTestParams(rmtesStringBuffers[6][0], rmtesStringBuffers[6][1], rmtesStringBuffers[6][2]),
	RmtesStringTestParams(rmtesStringBuffers[7][0], rmtesStringBuffers[7][1], rmtesStringBuffers[7][2]),
	RmtesStringTestParams(rmtesStringBuffers[8][0], rmtesStringBuffers[8][1], rmtesStringBuffers[8][2])
));

class FloatTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<float>
{
};


/* Test converting different float values, including Inf/-Inf/NaN */
TEST_P(FloatTestFixture, FloatTest)
{
	RsslFloat const &rsslFloat = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	
	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));
	
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = FLOAT_FIELD.fieldId;
	fieldEntry.dataType = RSSL_DT_FLOAT;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslFloat));

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FieldList. */
	ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
	ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

	/* Check float field. */
	ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));

	if (rsslFloat == HUGE_VALF || rsslFloat == INFINITY)
	{
		ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("Inf", _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetString());
	}
	else if (rsslFloat == -HUGE_VALF || rsslFloat == -INFINITY)
	{
		ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("-Inf", _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetString());
	}
	else if (::isnan(rsslFloat))
	{
		ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("NaN", _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetString());
	}
	else
	{
		ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsNumber());
		EXPECT_NEAR(rsslFloat, _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetFloat(), 0.099);
	}

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the float. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, FLOAT_FIELD.fieldId);

	RsslFloat decodeFloat;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));

	if (::isnan(rsslFloat))
		EXPECT_TRUE(::isnan(decodeFloat));
	else
	{
		/* Use an exact equality match. The test values include the infinity values, and comparing them 
		 * for 'nearness' involves subtraction which results in a difference of NaN. The values tested here
		 * do result in their exact same value after conversion. */
		EXPECT_EQ(decodeFloat, rsslFloat);
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_SUITE_P(PrimitiveTests, FloatTestFixture, ::testing::Values(
			0.0f,
			1.0f,
			-1.0f,
			HUGE_VALF,  /* Results in "Inf" in JSON */
			-HUGE_VALF,  /* Results in "-Inf" in JSON */
			INFINITY,
			-INFINITY,
			NAN
));

class DoubleTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<double>
{
};


/* Test converting different double values, including Inf/-Inf/NaN */
TEST_P(DoubleTestFixture, DoubleTest)
{
	RsslDouble const &rsslDouble = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	
	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));
	
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = DOUBLE_FIELD.fieldId;
	fieldEntry.dataType = RSSL_DT_DOUBLE;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslDouble));

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FieldList. */
	ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
	ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

	/* Check double field. */
	ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DOUBLE_FIELD.fieldName.data));

	if (rsslDouble == HUGE_VAL || rsslDouble == INFINITY)
	{
		ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("Inf", _jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].GetString());
	}
	else if (rsslDouble == -HUGE_VAL || rsslDouble == -INFINITY)
	{
		ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("-Inf", _jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].GetString());
	}
	else if (::isnan(rsslDouble))
	{
		ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsString());
		EXPECT_STREQ("NaN", _jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].GetString());
	}
	else
	{
		ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsNumber());
		EXPECT_NEAR(rsslDouble, _jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].GetDouble(), 0.099);
	}

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the double. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, DOUBLE_FIELD.fieldId);

	RsslDouble decodeDouble;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDouble(&_dIter, &decodeDouble));

	if (::isnan(rsslDouble))
		EXPECT_TRUE(::isnan(decodeDouble));
	else
	{
		/* Use an exact equality match. The test values include the infinity values, and comparing them 
		 * for 'nearness' involves subtraction which results in a difference of NaN. The values tested here
		 * do result in their exact same value after conversion. */
		EXPECT_EQ(decodeDouble, rsslDouble);
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_SUITE_P(PrimitiveTests, DoubleTestFixture, ::testing::Values(
			0.0,
			1.0,
			-1.0,
			HUGE_VAL, /* Results in "Inf" in JSON*/
			-HUGE_VAL, /* Results in "-Inf" in JSON */
			(double)INFINITY,
			(double)-INFINITY,
			(double)NAN
));

class InvalidFloatDoubleStringTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<const char*>
{
};

/* Test invalid string values for floats & doubles. */
TEST_P(InvalidFloatDoubleStringTestFixture, InvalidFloatDoubleStringTests)
{
	const char* floatString = GetParam();

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;
	   
	/* Build message containing the invalid float string. */

	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"FLOAT\":\"");
	jsonStringStream << floatString << "\"}}";
	
	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected Value."));

	/* Now test the same invalid string in a double. */

	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"DOUBLE\":\"");
	jsonStringStream << floatString << "\"}}";

	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected Value."));
}

INSTANTIATE_TEST_SUITE_P(PrimitiveTests, InvalidFloatDoubleStringTestFixture, ::testing::Values(
			"X",
			"I",
			"Infi",
			"InfinityWar",
			"inf",
			"-I",
			"-Infi",
			"-InfinityWar",
			"-inf"
			"N",
			"Nani",
			"nan",
			"-NaN"
));

/* Parameters for Float/Double string values. */
class HugeFloatDoubleTestParams
{
	public:

		const char* stringValue;	/* JSON string of the huge value */
		float rwfValue;				/* Resulting value in RWF (should be INFINITY or -INFINITY for this test) */


	HugeFloatDoubleTestParams(const char *stringValue, float rwfValue)
	{
		this->stringValue = stringValue;
		this->rwfValue = rwfValue;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const HugeFloatDoubleTestParams& params)
	{
		out << "[String:" << params.stringValue << ", Value:" << params.rwfValue << "]";
		return out;
	}
};

class HugeFloatDoubleTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<HugeFloatDoubleTestParams>
{
};


/* Test large values of floats/doubles from JSON. */
TEST_P(HugeFloatDoubleTestFixture, HugeFloatDoubleTest)
{
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	HugeFloatDoubleTestParams const &params = GetParam();
	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	/* Confirm that the number in JSON is large enough that strtof will actually indicate overflow */
	if (params.rwfValue < 0.0f)
		EXPECT_EQ(-HUGE_VAL, strtod(params.stringValue, NULL));
	else
		EXPECT_EQ(HUGE_VAL, strtod(params.stringValue, NULL));

	/* Build JSON string and convert to RWF. */
	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"FLOAT\":");
	jsonStringStream << params.stringValue << "}}";
	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslGenericMsg is correct. */
	EXPECT_EQ(RSSL_MC_GENERIC, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(2, rsslMsg.msgBase.streamId);
	EXPECT_EQ(128, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the float. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, FLOAT_FIELD.fieldId);

	/* Converter should've encoded INFINITY or -INFINITY. */
	RsslFloat decodeFloat;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));
	EXPECT_EQ(params.rwfValue, decodeFloat);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

	/*** Repeat test for Double. ***/

	/* Confirm that the number in JSON is large enough that strtod will actually indicate overflow */
	if (params.rwfValue < 0.0f)
		EXPECT_EQ(-HUGE_VAL, strtod(params.stringValue, NULL));
	else
		EXPECT_EQ(HUGE_VAL, strtod(params.stringValue, NULL));

	/* Build JSON string and convert to RWF. */
	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"DOUBLE\":");
	jsonStringStream << params.stringValue << "}}";
	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslGenericMsg is correct. */
	EXPECT_EQ(RSSL_MC_GENERIC, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(2, rsslMsg.msgBase.streamId);
	EXPECT_EQ(128, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the double. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, DOUBLE_FIELD.fieldId);

	/* Converter should've encoded INFINITY or -INFINITY. */
	RsslDouble decodeDouble;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDouble(&_dIter, &decodeDouble));
	EXPECT_EQ(params.rwfValue, decodeDouble);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_SUITE_P(PrimitiveTests, HugeFloatDoubleTestFixture, ::testing::Values(
	HugeFloatDoubleTestParams("9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", (float)INFINITY),
	HugeFloatDoubleTestParams("-9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999", (float)(-INFINITY))
));


class InvalidUintStringTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<const char*>
{
};

/* Test invalid values for UINTs. */
TEST_P(InvalidUintStringTestFixture, InvalidUintStringTests)
{
	const char* uintString = GetParam();

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	/* Build message containing the invalid UINT64 value. */

	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"EUROCLR_NO\":");
	jsonStringStream << uintString << "}}";

	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Unexpected Value."));
}

/* negative values correct for numbers but incorrect for UINTs */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, InvalidUintStringTestFixture, ::testing::Values(
	"-1",
	"-2",
	"-3",
	"-42",
	"-100500"
));

class InvalidUintParserTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<const char*>
{
};

/* Test invalid JSON Parser values $, #, A for UINTs. */
TEST_P(InvalidUintParserTestFixture, InvalidUintParserTests)
{
	RsslJsonConverterError converterError;

	const char* uintString = GetParam();

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	/* Build message containing the invalid UINT64 value. */

	jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"EUROCLR_NO\":");
	jsonStringStream << uintString << "}}";

	jsonString = jsonStringStream.str();
	setJsonBufferToString(jsonString.c_str());
	EXPECT_NO_FATAL_FAILURE(getJsonToRsslParserError(&converterError));

	ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON parser error:"));
}

/* Added additional test inputs for others invalid UInt type such as $, #, A as well. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, InvalidUintParserTestFixture, ::testing::Values(
	"$",
	"#",
	"A"
));

class EnumValueTestParam
{
	public:
		RsslJsonProtocolType protocolType;
		const char *stringValue;
		bool isOverflow;

		EnumValueTestParam (RsslJsonProtocolType protocolType, const char * stringValue, bool isOverflow)
		{
			this->protocolType = protocolType;
			this->stringValue = stringValue;
			this->isOverflow = isOverflow;
		}

		/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
		friend ostream &operator<<(ostream &out, const EnumValueTestParam& params)
		{
			std::ios::fmtflags f(out.flags());

			out << "[protocolType: "<< params.protocolType
				<< ", stringValue:" << params.stringValue
				<< ", isOverflow:" << std::boolalpha << params .isOverflow << "]";

			out.flags(f);

			return out;
		}
};

class OverflowEnumTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<EnumValueTestParam>
{
};

/* Test enum overflow */
TEST_P(OverflowEnumTestFixture, InvalidUintParserTests)
{
	EnumValueTestParam const &params = GetParam();

	RsslJsonConverterError converterError;

	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	if (params.protocolType == RSSL_JSON_JPT_JSON2)
	{
		jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"RDN_EXCHID\":");
		jsonStringStream << params.stringValue << "}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		jsonStringStream.str("{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":4},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"d\":{\"4\":");
		jsonStringStream << params.stringValue << "}}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else
		FAIL() << "Wrong protocolType: " << params.protocolType;

	if(params.isOverflow)
	{
		EXPECT_NO_FATAL_FAILURE(getJsonToRsslError(params.protocolType, &converterError));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON Unexpected Value. Received"));
	}
	else
	{
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(params.protocolType));

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Check FieldList. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

		/* FieldList should contain one field containing the double. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

		RsslEnum decodeEnum;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeEnum(&_dIter, &decodeEnum));

		ASSERT_EQ(decodeEnum, 65535);
	}
}

/* Added additional test inputs for others invalid UInt type such as $, #, A as well. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, OverflowEnumTestFixture, ::testing::Values(
	EnumValueTestParam(RSSL_JSON_JPT_JSON2, "-100", true),
	EnumValueTestParam(RSSL_JSON_JPT_JSON, "-100", true),
	EnumValueTestParam(RSSL_JSON_JPT_JSON2,"65536", true),
	EnumValueTestParam(RSSL_JSON_JPT_JSON,"65536", true),
	EnumValueTestParam(RSSL_JSON_JPT_JSON2,"65535", false),
	EnumValueTestParam(RSSL_JSON_JPT_JSON,"65535", false)
));


class IntValueTestParam
{
public:
	RsslJsonProtocolType protocolType;
	const char *stringValue;
	bool isOverflow;
	int checkMaxMin;

	IntValueTestParam(RsslJsonProtocolType protocolType, const char * stringValue, bool isOverflow, int checkMaxMin)
	{
		this->protocolType = protocolType;
		this->stringValue = stringValue;
		this->isOverflow = isOverflow;
		this->checkMaxMin = checkMaxMin;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const IntValueTestParam& params)
	{
		std::ios::fmtflags f(out.flags());

		out << "[protocolType: " << params.protocolType
			<< ", stringValue:" << params.stringValue
			<< ", isOverflow:" << std::boolalpha << params.isOverflow
			<< ", checkMaxMin:" << params.checkMaxMin
			<< "]";

		out.flags(f);

		return out;
	}
};

class OverflowIntTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<IntValueTestParam>
{
};

/* Test int (long long) overflow */
TEST_P(OverflowIntTestFixture, InvalidUintParserTests)
{
	IntValueTestParam const &params = GetParam();

	RsslJsonConverterError converterError;

	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	if (params.protocolType == RSSL_JSON_JPT_JSON2)
	{
		jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"SENTIMENT\":");
		jsonStringStream << params.stringValue << "}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		jsonStringStream.str("{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":4},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"d\":{\"5170\":");
		jsonStringStream << params.stringValue << "}}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else
		FAIL() << "Wrong protocolType: " << params.protocolType;

	if(params.isOverflow)
	{
		EXPECT_NO_FATAL_FAILURE(getJsonToRsslError(params.protocolType, &converterError));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON Unexpected Value. Received"));
	}
	else
	{
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(params.protocolType));

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Check FieldList. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

		/* FieldList should contain one field containing the double. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

		RsslInt decodeInt;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &decodeInt));

		if (params.checkMaxMin == 1)
			ASSERT_EQ(decodeInt, 9223372036854775807LL);
		else if (params.checkMaxMin == 2)
			ASSERT_EQ(decodeInt, -9223372036854775807LL-1);
		else
			FAIL() << "Wrong checkMaxMin param";
	}
}

/* Added additional test inputs for others invalid UInt type such as $, #, A as well. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, OverflowIntTestFixture, ::testing::Values(
	IntValueTestParam(RSSL_JSON_JPT_JSON2, "-1234567891234567891234567891234567891234567890", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON, "-1234567891234567891234567891234567891234567890", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON2, "6553734534444444444444444444444444444444444444344444444", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON, "6553734534444444444444444444444444444444444444344444444", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON2, "9223372036854775808", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON, "9223372036854775808", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON2,"-9223372036854775809", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON,"-9223372036854775809", true, 0),
	IntValueTestParam(RSSL_JSON_JPT_JSON2,"9223372036854775807", false, 1),
	IntValueTestParam(RSSL_JSON_JPT_JSON,"9223372036854775807", false, 1),
	IntValueTestParam(RSSL_JSON_JPT_JSON2, "-9223372036854775808", false, 2),
	IntValueTestParam(RSSL_JSON_JPT_JSON, "-9223372036854775808", false, 2)
));


class UIntValueTestParam
{
public:
	RsslJsonProtocolType protocolType;
	const char *stringValue;
	bool isOverflow;

	UIntValueTestParam(RsslJsonProtocolType protocolType, const char * stringValue, bool isOverflow)
	{
		this->protocolType = protocolType;
		this->stringValue = stringValue;
		this->isOverflow = isOverflow;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const UIntValueTestParam& params)
	{
		std::ios::fmtflags f(out.flags());

		out << "[protocolType: " << params.protocolType
			<< ", stringValue:" << params.stringValue
			<< ", isOverflow:" << std::boolalpha << params.isOverflow << "]";

		out.flags(f);

		return out;
	}
};

class OverflowUIntTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<UIntValueTestParam>
{
};

/* Test int (long long) overflow */
TEST_P(OverflowUIntTestFixture, InvalidUintParserTests)
{
	UIntValueTestParam const &params = GetParam();

	RsslJsonConverterError converterError;

	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	std::ostringstream jsonStringStream(std::ostringstream::ate);
	std::string jsonString;

	if (params.protocolType == RSSL_JSON_JPT_JSON2)
	{
		jsonStringStream.str("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Fields\":{\"PROD_PERM\":");
		jsonStringStream << params.stringValue << "}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		jsonStringStream.str("{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":4},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"d\":{\"1\":");
		jsonStringStream << params.stringValue << "}}}";
		jsonString = jsonStringStream.str();
		setJsonBufferToString(jsonString.c_str());
	}
	else
		FAIL() << "Wrong protocolType: " << params.protocolType;

	if (params.isOverflow)
	{
		EXPECT_NO_FATAL_FAILURE(getJsonToRsslError(params.protocolType, &converterError));
		ASSERT_TRUE(::testing::internal::RE::PartialMatch(converterError.text, "JSON Unexpected Value. Received"));
	}
	else
	{
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(params.protocolType));

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Check FieldList. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

		/* FieldList should contain one field containing the double. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

		RsslUInt decodeUInt;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeUInt(&_dIter, &decodeUInt));

		ASSERT_EQ(decodeUInt, 18446744073709551615ULL);
	}
}

/* Added additional test inputs for others invalid UInt type such as $, #, A as well. */
INSTANTIATE_TEST_SUITE_P(PrimitiveTests, OverflowUIntTestFixture, ::testing::Values(
	UIntValueTestParam(RSSL_JSON_JPT_JSON2, "-131233333333333333333333333333333339345394583434534535", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON, "-131233333333333333333333333333333339345394583434534535", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON2, "6553734534444444444444444444444444444444444444344444444", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON, "6553734534444444444444444444444444444444444444344444444", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON2, "18446744073709551616", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON, "18446744073709551616", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON2, "-1844674407370955161", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON, "-1844674407370955161", true),
	UIntValueTestParam(RSSL_JSON_JPT_JSON2, "18446744073709551615", false),
	UIntValueTestParam(RSSL_JSON_JPT_JSON, "18446744073709551615", false)
));


TEST(OverflowIntTest, ConversionI8ToStrTests)
{
	/*Max Values*/
	const char* i8MaxVal = "127";

	char *begin = NULL;
	char *end = NULL;

	RsslInt8 i8res;

	begin = (char*)i8MaxVal;
	end = (char*)begin + strlen(i8MaxVal);

	ASSERT_EQ(rtr_atoi8_size_check(begin, end, &i8res), end);
	ASSERT_EQ(i8res, 127);

	const char* i8MinVal = "-128";

	begin = (char*)i8MinVal;
	end = (char*)begin + strlen(i8MinVal);

	ASSERT_EQ(rtr_atoi8_size_check(begin, end, &i8res), end);
	ASSERT_EQ(i8res, -128);

	/* Overflow */

	const char* i8MMaxOverVal = "128";

	begin = (char*)i8MMaxOverVal;
	end = (char*)begin + strlen(i8MMaxOverVal);

	ASSERT_NE(rtr_atoi8_size_check(begin, end, &i8res), end);

	const char* i8MMinUnderVal = "-129";

	begin = (char*)i8MMinUnderVal;
	end = (char*)begin + strlen(i8MMinUnderVal);

	ASSERT_NE(rtr_atoi8_size_check(begin, end, &i8res), end);
}

TEST(OverflowIntTest, ConversionUI8ToStrTests)
{
	/*Max Values*/
	const char* ui8MaxVal = "255";

	char *begin = NULL;
	char *end = NULL;

	RsslUInt8 i8res;

	begin = (char*)ui8MaxVal;
	end = (char*)begin + strlen(ui8MaxVal);

	ASSERT_EQ(rtr_atoui8_size_check(begin, end, &i8res), end);
	ASSERT_EQ(i8res, 255);

	/* Overflow */

	const char* ui8OMaxOverVal = "256";

	begin = (char*)ui8OMaxOverVal;
	end = (char*)begin + strlen(ui8OMaxOverVal);

	ASSERT_NE(rtr_atoui8_size_check(begin, end, &i8res), end);
}

TEST(OverflowIntTest, ConversionI16ToStrTests)
{
	/*Max Values*/
	const char* i16MaxVal = "32767";

	char *begin = NULL;
	char *end = NULL;

	RsslInt16 i16res;

	begin = (char*)i16MaxVal;
	end = (char*)begin + strlen(i16MaxVal);

	ASSERT_EQ(rtr_atoi16_size_check(begin, end, &i16res), end);
	ASSERT_EQ(i16res, 32767);

	const char* i16MinVal = "-32768";

	begin = (char*)i16MinVal;
	end = (char*)begin + strlen(i16MinVal);

	ASSERT_EQ(rtr_atoi16_size_check(begin, end, &i16res), end);
	ASSERT_EQ(i16res, -32768);

	/* Overflow */

	const char* i16MMaxOverVal = "32768";

	begin = (char*)i16MMaxOverVal;
	end = (char*)begin + strlen(i16MMaxOverVal);

	ASSERT_NE(rtr_atoi16_size_check(begin, end, &i16res), end);

	const char* i16MMinUnderVal = "-32769";

	begin = (char*)i16MMinUnderVal;
	end = (char*)begin + strlen(i16MMinUnderVal);

	ASSERT_NE(rtr_atoi16_size_check(begin, end, &i16res), end);
}

TEST(OverflowIntTest, ConversionUI16ToStrTests)
{
	/*Max Values*/
	const char* ui16MaxVal = "65535";

	char *begin = NULL;
	char *end = NULL;

	RsslUInt16 iu16res;

	begin = (char*)ui16MaxVal;
	end = (char*)begin + strlen(ui16MaxVal);

	ASSERT_EQ(rtr_atoui16_size_check(begin, end, &iu16res), end);
	ASSERT_EQ(iu16res, 65535);

	/* Overflow */

	const char* ui16OMaxOverVal = "65536";

	begin = (char*)ui16OMaxOverVal;
	end = (char*)begin + strlen(ui16OMaxOverVal);

	ASSERT_NE(rtr_atoui16_size_check(begin, end, &iu16res), end);
}

TEST(OverflowIntTest, ConversionI32ToStrTests)
{
	/*Max Values*/
	const char* i32MaxVal = "2147483647";

	char *begin = NULL;
	char *end = NULL;

	RsslInt32 i32res;

	begin = (char*)i32MaxVal;
	end = (char*)begin + strlen(i32MaxVal);

	ASSERT_EQ(rtr_atoi32_size_check(begin, end, &i32res), end);
	ASSERT_EQ(i32res, INT_MAX);

	const char* i32MinVal = "-2147483648";

	begin = (char*)i32MinVal;
	end = (char*)begin + strlen(i32MinVal);

	ASSERT_EQ(rtr_atoi32_size_check(begin, end, &i32res), end);
	ASSERT_EQ(i32res, INT_MIN);

	/* Overflow */

	const char* i32MMaxOverVal = "2147483648";

	begin = (char*)i32MMaxOverVal;
	end = (char*)begin + strlen(i32MMaxOverVal);

	ASSERT_NE(rtr_atoi32_size_check(begin, end, &i32res), end);

	const char* i32MMinUnderVal = "-2147483649";

	begin = (char*)i32MMinUnderVal;
	end = (char*)begin + strlen(i32MMinUnderVal);

	ASSERT_NE(rtr_atoi32_size_check(begin, end, &i32res), end);
}

TEST(OverflowIntTest, ConversionUI32ToStrTests)
{
	/*Max Values*/
	const char* ui32MaxVal = "4294967295";

	char *begin = NULL;
	char *end = NULL;

	RsslUInt32 iu32res;

	begin = (char*)ui32MaxVal;
	end = (char*)begin + strlen(ui32MaxVal);

	ASSERT_EQ(rtr_atoui32_size_check(begin, end, &iu32res), end);
	ASSERT_EQ(iu32res, 4294967295);

	/* Overflow */

	const char* ui32OMaxOverVal = "4294967296";

	begin = (char*)ui32OMaxOverVal;
	end = (char*)begin + strlen(ui32OMaxOverVal);

	ASSERT_NE(rtr_atoui32_size_check(begin, end, &iu32res), end);
}


TEST(OverflowIntTest, ConversionI64ToStrTests)
{
	/*Max Values*/
	const char* i64MaxVal = "9223372036854775807";

	char *begin = NULL;
	char *end = NULL;

	RsslInt64 i64res;

	begin = (char*)i64MaxVal;
	end = (char*)begin + strlen(i64MaxVal);

	ASSERT_EQ(rtr_atoi64_size_check(begin, end, &i64res), end);
	ASSERT_EQ(i64res, 9223372036854775807);

	const char* i64MinVal = "-9223372036854775808";

	begin = (char*)i64MinVal;
	end = (char*)begin + strlen(i64MinVal);

	ASSERT_EQ(rtr_atoi64_size_check(begin, end, &i64res), end);
	ASSERT_EQ(i64res, -9223372036854775807LL-1);

	/* Overflow */

	const char* i64MMaxOverVal = "9223372036854775808";

	begin = (char*)i64MMaxOverVal;
	end = (char*)begin + strlen(i64MMaxOverVal);

	ASSERT_NE(rtr_atoi64_size_check(begin, end, &i64res), end);

	const char* i64MMinUnderVal = "-9223372036854775809";

	begin = (char*)i64MMinUnderVal;
	end = (char*)begin + strlen(i64MMinUnderVal);

	ASSERT_NE(rtr_atoi64_size_check(begin, end, &i64res), end);
}

TEST(OverflowIntTest, ConversinUI64ToStrTests)
{
	/*Max Values*/
	const char* ui64MaxVal = "18446744073709551615";

	char *begin = NULL;
	char *end = NULL;

	RsslUInt64 iu64res;

	begin = (char*)ui64MaxVal;
	end = (char*)begin + strlen(ui64MaxVal);

	ASSERT_EQ(rtr_atoui64_size_check(begin, end, &iu64res), end);
	ASSERT_EQ(iu64res, 18446744073709551615ULL);

	/* Overflow */

	const char* ui64OMaxOverVal = "18446744073709551616";

	begin = (char*)ui64OMaxOverVal;
	end = (char*)begin + strlen(ui64OMaxOverVal);

	ASSERT_NE(rtr_atoui64_size_check(begin, end, &iu64res), end);
}


class StreamIdTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RsslInt32>
{
};

/* Test on edge cases streamId */
TEST_P(StreamIdTestFixture, StreamIdTest)
{
	RsslInt32 const& streamId_testValue = GetParam();
	RsslFloat const rsslFloat = 123.45f;
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = streamId_testValue;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	rsslUpdateMsgApplyHasMsgKey(&updateMsg);
	rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
	updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));

	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = FLOAT_FIELD.fieldId;
	fieldEntry.dataType = RSSL_DT_FLOAT;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslFloat));

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FieldList. */
	ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
	ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

	/* Check float field. */
	ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));

	ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsNumber());
	EXPECT_NEAR(rsslFloat, _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetFloat(), 0.099);

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(streamId_testValue, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	/* FieldList should contain one field containing the float. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(fieldEntry.fieldId, FLOAT_FIELD.fieldId);

	RsslFloat decodeFloat;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));

	if (::isnan(rsslFloat))
		EXPECT_TRUE(::isnan(decodeFloat));
	else
	{
		/* Use an exact equality match. The test values include the infinity values, and comparing them
		 * for 'nearness' involves subtraction which results in a difference of NaN. The values tested here
		 * do result in their exact same value after conversion. */
		EXPECT_EQ(decodeFloat, rsslFloat);
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_SUITE_P(StreamIdEdgeCases, StreamIdTestFixture, ::testing::Values(
	0,
	1,
	-1,
	1000000,
	-1000000,
	1000000000,
	-1000000000,
	2000000000,
	-2000000000,
	INT_MAX,
	INT_MIN
));
