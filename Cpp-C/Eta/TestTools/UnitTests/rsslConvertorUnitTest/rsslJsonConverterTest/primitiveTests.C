/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"
#include <limits.h>
#include <math.h>

#include <stdlib.h>

#include <float.h>
#include <ctype.h>

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
INSTANTIATE_TEST_CASE_P(PrimitiveTests, RealStringValueTestFixture, ::testing::Values(
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

	RmtesStringTestParams(RsslBuffer rmtesBuffer, RsslBuffer utf8Buffer)
	{
		this->rmtesBuffer = rmtesBuffer;
		this->utf8Buffer = utf8Buffer;
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
		EXPECT_TRUE(rsslBufferIsEqual(&params.utf8Buffer, &decodeBuffer));
	}
}

const RsslBuffer rmtesStringBuffers[][2] =
{
	/* First buffer is the Rmtes string, second is the corresponding Utf8 string */

	/* Value is Ascii-only. UTF8 string should be identical */
	{{ 4, (char*)"TEST" }, {4, (char*)"TEST"}},

	/* RmtesString is blank. Utf8 result should be null*/
	{{ 0, (char*)"" }, {0, (char*)NULL}},

	/* RmtesString contains only null bytes. Those will be discarded, so Utf8 result should be null. */
	{{ 4, (char*)"\0\0\0\0" }, {0, (char*)NULL}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 8, (char*)"TEST\0\0\0\0" }, {4, (char*)"TEST"}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 12, (char*)"TEST\0\0\0\0TEST" }, {8, (char*)"TESTTEST"}},

	/* Value contains null bytes. UTF8 should drop them. */
	{{ 8, (char*)"\0\0\0\0TEST" }, {4, (char*)"TEST"}}
};

/* Test cases for Rmtes strings. */
INSTANTIATE_TEST_CASE_P(PrimitiveTests, RmtesStringTestFixture, ::testing::Values(
	RmtesStringTestParams(rmtesStringBuffers[0][0], rmtesStringBuffers[0][1]),
	RmtesStringTestParams(rmtesStringBuffers[1][0], rmtesStringBuffers[1][1]),
	RmtesStringTestParams(rmtesStringBuffers[2][0], rmtesStringBuffers[2][1]),
	RmtesStringTestParams(rmtesStringBuffers[3][0], rmtesStringBuffers[3][1]),
	RmtesStringTestParams(rmtesStringBuffers[4][0], rmtesStringBuffers[4][1]),
	RmtesStringTestParams(rmtesStringBuffers[5][0], rmtesStringBuffers[5][1])
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
	else if (isnan(rsslFloat))
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

	if (isnan(rsslFloat))
		EXPECT_TRUE(isnan(decodeFloat));
	else
	{
		/* Use an exact equality match. The test values include the infinity values, and comparing them 
		 * for 'nearness' involves subtraction which results in a difference of NaN. The values tested here
		 * do result in their exact same value after conversion. */
		EXPECT_EQ(decodeFloat, rsslFloat);
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_CASE_P(PrimitiveTests, FloatTestFixture, ::testing::Values(
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
	else if (isnan(rsslDouble))
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

	if (isnan(rsslDouble))
		EXPECT_TRUE(isnan(decodeDouble));
	else
	{
		/* Use an exact equality match. The test values include the infinity values, and comparing them 
		 * for 'nearness' involves subtraction which results in a difference of NaN. The values tested here
		 * do result in their exact same value after conversion. */
		EXPECT_EQ(decodeDouble, rsslDouble);
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
}

INSTANTIATE_TEST_CASE_P(PrimitiveTests, DoubleTestFixture, ::testing::Values(
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

INSTANTIATE_TEST_CASE_P(PrimitiveTests, InvalidFloatDoubleStringTestFixture, ::testing::Values(
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

INSTANTIATE_TEST_CASE_P(PrimitiveTests, HugeFloatDoubleTestFixture, ::testing::Values(
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
INSTANTIATE_TEST_CASE_P(PrimitiveTests, InvalidUintStringTestFixture, ::testing::Values(
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
INSTANTIATE_TEST_CASE_P(PrimitiveTests, InvalidUintParserTestFixture, ::testing::Values(
	"$",
	"#",
	"A"
));
