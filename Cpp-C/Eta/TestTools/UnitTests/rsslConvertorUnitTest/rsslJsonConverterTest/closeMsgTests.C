/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license
*| and is provided AS IS with no warranty or guarantee of fit for purpose.
*| See the project's LICENSE.md for details.
*| Copyright (C) 2020, 2024 LSEG. All rights reserved.
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json; 

/* Fixture for CloseMsgTests that has conversion code. */
class CloseMsgTests : public MsgConversionTestBase
{
};

/* Parameters for CloseMsg tests. */
class CloseMsgMemberTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool ack;
	bool batch;
	int dummy; /* This class is small; silence a Valgrind warning when GoogleTest uses it. */

	CloseMsgMemberTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool ack, bool batch)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->ack = ack;
		this->batch = batch;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const CloseMsgMemberTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"ack:" << (params.ack ? "true" : "false") << ","
			"batch:" << (params.batch ? "true" : "false") 
			<< "]";
		return out;
	}
};

class CloseMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<CloseMsgMemberTestParams>
{
};

/* Test that converts a CloseMsg from RWF to JSON, and back to RWF. */
TEST_P(CloseMsgMembersTestFixture, CloseMsgMembersTest)
{
	CloseMsgMemberTestParams const &params = GetParam();
	RsslCloseMsg closeMsg;
	RsslMsg rsslMsg;

	RsslInt BATCH_STREAM_IDS[3] = {5, 8, 13};

	/* Create and encode a simple CloseMsg. */
	rsslClearCloseMsg(&closeMsg);

	closeMsg.msgBase.streamId = 5;
	closeMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	if (params.extendedHeader)
	{
		closeMsg.flags |= RSSL_CLMF_HAS_EXTENDED_HEADER;
		closeMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.ack)
		closeMsg.flags |= RSSL_CLMF_ACK;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	if (params.batch)
	{
		/* Encode a payload containing the Stream ID batch. */
		RsslElementList elementList;
		RsslElementEntry elementEntry;
		RsslArray streamIdArray;

		closeMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
		closeMsg.flags |= RSSL_CLMF_HAS_BATCH;

		ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&closeMsg, 0));

		rsslClearElementList(&elementList);
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(&_eIter, &elementList, NULL, 0));

		/* Encode StreamIdList Entry. */
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = RSSL_ENAME_BATCH_STREAMID_LIST;
		elementEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		/* Encode StreamId Array. */
		rsslClearArray(&streamIdArray);
		streamIdArray.primitiveType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &streamIdArray));

		/* Encode stream IDs. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_STREAM_IDS[0]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_STREAM_IDS[1]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_STREAM_IDS[2]));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(&_eIter, RSSL_TRUE));
	}
	else
	{
		closeMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&closeMsg));
	}

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch (params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Close", _jsonDocument["Type"].GetString());

			/* Check stream id(s). */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));

			if (!params.batch)
			{
				/* In Non-Batch, ID element should be a single integer. */
				ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
				EXPECT_EQ(5, _jsonDocument["ID"].GetInt());
			}
			else /* Check Batch Stream ID List. */
			{
				/* In Batch, ID element should be an array of integers. */
				ASSERT_TRUE(_jsonDocument["ID"].IsArray());

				/* Check Stream ID Array. */
				const Value& streamIdList = _jsonDocument["ID"];
				ASSERT_EQ(3, streamIdList.Size());
				ASSERT_TRUE(streamIdList[0].IsNumber());
				ASSERT_EQ(BATCH_STREAM_IDS[0], streamIdList[0].GetInt());
				ASSERT_TRUE(streamIdList[1].IsNumber());
				ASSERT_EQ(BATCH_STREAM_IDS[1], streamIdList[1].GetInt());
				ASSERT_TRUE(streamIdList[2].IsNumber());
				ASSERT_EQ(BATCH_STREAM_IDS[2], streamIdList[2].GetInt());
			}

			/* Check Domain. */
			ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ExtHdr"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["ExtHdr"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ExtHdr"));

			/* Check Ack flag. */
			if (params.ack)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Ack"));
				ASSERT_TRUE(_jsonDocument["Ack"].IsBool());
				ASSERT_TRUE(_jsonDocument["Ack"].GetBool());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("Ack"));

			break;
		}

		case RSSL_JSON_JPT_JSON:
		{

			/* Get message base. */
			ASSERT_TRUE(_jsonDocument.HasMember("b"));
			ASSERT_TRUE(_jsonDocument["b"].IsObject());
			const Value &msgBase = _jsonDocument["b"];

			/* Check message class. */
			ASSERT_TRUE(msgBase.HasMember("c"));
			ASSERT_TRUE(msgBase["c"].IsNumber());
			EXPECT_EQ(RSSL_MC_CLOSE, msgBase["c"].GetInt());

			/* Check Stream ID. */
			ASSERT_TRUE(msgBase.HasMember("s"));
			ASSERT_TRUE(msgBase["s"].IsNumber());
			EXPECT_EQ(5, msgBase["s"].GetInt());

			/* Check Domain. */
			ASSERT_TRUE(msgBase.HasMember("t"));
			ASSERT_TRUE(msgBase["t"].IsNumber());
			EXPECT_EQ(RSSL_DMT_MARKET_PRICE, msgBase["t"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("e"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["e"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("e"));

			/* Check Ack flag. */
			if (params.ack)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("a"));
				ASSERT_TRUE(_jsonDocument["a"].IsNumber());
				ASSERT_EQ(1, _jsonDocument["a"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("a"));

			/* Check ContainerType. */
			ASSERT_TRUE(msgBase.HasMember("f"));
			ASSERT_TRUE(msgBase["f"].IsNumber());
			if (!params.batch)
			{
				EXPECT_EQ(RSSL_DT_NO_DATA - 128, msgBase["f"].GetInt());
				EXPECT_FALSE(msgBase.HasMember("d"));
			}
			else
			{
				EXPECT_EQ(RSSL_DT_ELEMENT_LIST - 128, msgBase["f"].GetInt());
				ASSERT_TRUE(_jsonDocument.HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			}

			/* Check Batch (container and type already checked). */
			if (params.batch)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
				ASSERT_TRUE(_jsonDocument["d"]["d"].Size() == 1);

				ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][0]["n"].IsString());

				if (0 == strcmp(":StreamIdList", _jsonDocument["d"]["d"][0]["n"].GetString()))
				{
					/* Element type should be Array */
					ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("t"));
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["t"].IsNumber());
					EXPECT_EQ(RSSL_DT_ARRAY, _jsonDocument["d"]["d"][0]["t"].GetInt());

					ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].IsObject());

					/* Array type should be Int. */
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].HasMember("t"));
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["t"].IsNumber());
					EXPECT_EQ(RSSL_DT_INT, _jsonDocument["d"]["d"][0]["d"]["t"].GetInt());

					/* Data should be an array of three numbers. */
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].HasMember("d"));
					ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["d"].IsArray());
					const Value &streamList = _jsonDocument["d"]["d"][0]["d"]["d"];
					ASSERT_EQ(3, streamList.Size());
					ASSERT_TRUE(streamList[0].IsNumber());
					EXPECT_EQ(BATCH_STREAM_IDS[0], streamList[0].GetInt());
					ASSERT_TRUE(streamList[1].IsNumber());
					EXPECT_EQ(BATCH_STREAM_IDS[1], streamList[1].GetInt());
					ASSERT_TRUE(streamList[2].IsNumber());
					EXPECT_EQ(BATCH_STREAM_IDS[2], streamList[2].GetInt());
				}
				else
					FAIL() << "Unrecognized Element Name " << _jsonDocument["d"]["d"][0]["n"].GetString();
			}

			break;
		}

		default:
			FAIL() << "Unknown protocol type" << params.protocolType;
			break;

	}

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(params.protocolType));

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslCloseMsg is correct. */
	EXPECT_EQ(RSSL_MC_CLOSE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(BATCH_STREAM_IDS[0], rsslMsg.msgBase.streamId);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslCloseMsgCheckHasExtendedHdr(&rsslMsg.closeMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.closeMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslCloseMsgCheckHasExtendedHdr(&rsslMsg.closeMsg));

	/* Check Ack flag. */
	EXPECT_EQ(params.ack, rsslCloseMsgCheckAck(&rsslMsg.closeMsg));

	if (params.batch)
	{
		/* Decode the Stream ID batch. */
		RsslElementList elementList;
		RsslElementEntry elementEntry;
		RsslArray streamIdArray;
		RsslBuffer arrayEntry;
		RsslInt arrayValue;

		EXPECT_TRUE(rsslCloseMsgCheckHasBatch(&rsslMsg.closeMsg));
		EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

		/* Decode Stream ID List Entry. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementEntry(&_dIter, &elementEntry));
		ASSERT_TRUE(rsslBufferIsEqual(&RSSL_ENAME_BATCH_STREAMID_LIST, &elementEntry.name));
		ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &streamIdArray));
		ASSERT_EQ(RSSL_DT_INT, streamIdArray.primitiveType);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
		EXPECT_EQ(BATCH_STREAM_IDS[0], arrayValue);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
		EXPECT_EQ(BATCH_STREAM_IDS[1], arrayValue);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
		EXPECT_EQ(BATCH_STREAM_IDS[2], arrayValue);

		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));

		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeElementEntry(&_dIter, &elementEntry));
	}
	else
		EXPECT_EQ(RSSL_DT_NO_DATA, rsslMsg.msgBase.containerType);
}

INSTANTIATE_TEST_SUITE_P(CloseMsgTests, CloseMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, Ack, Batch */

	/* Defaults */
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false),
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false),

	/* ExtendedHeader */
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON, true, false, false),
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON2, true, false, false),

	/* Ack */
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, true, false),
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, true, false),

	/* Batch */
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, true),
	CloseMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, true)

));


/* Test using a Stream ID array in the JSON batch close, and converting back to RWF. */
TEST_F(CloseMsgTests, BatchStreamIdArrayConversionTest)
{
	RsslMsg rsslMsg;

	RsslInt BATCH_STREAM_IDS[3] = {5, 8, 13};
	_jsonBuffer.data = (char*)"{\"Type\":\"Close\",\"ID\":[5,8,13]}";
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslCloseMsg is correct. */
	EXPECT_EQ(RSSL_MC_CLOSE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_TRUE(rsslCloseMsgCheckHasBatch(&rsslMsg.closeMsg));
	EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(BATCH_STREAM_IDS[0], rsslMsg.msgBase.streamId);

	/* Decode the Stream ID element in the dataBody. */
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslArray streamIdArray;
	RsslBuffer arrayEntry;
	RsslInt arrayValue;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementEntry(&_dIter, &elementEntry));
	ASSERT_TRUE(rsslBufferIsEqual(&RSSL_ENAME_BATCH_STREAMID_LIST, &elementEntry.name));
	ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

	/* Decode the Stream ID array. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &streamIdArray));
	ASSERT_EQ(RSSL_DT_INT, streamIdArray.primitiveType);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
	EXPECT_EQ(BATCH_STREAM_IDS[0], arrayValue);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
	EXPECT_EQ(BATCH_STREAM_IDS[1], arrayValue);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &arrayValue));
	EXPECT_EQ(BATCH_STREAM_IDS[2], arrayValue);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeElementEntry(&_dIter, &elementEntry));
}

TEST_F(CloseMsgTests, BatchStreamIdArrayWithNullConversionTest)
{
	RsslMsg rsslMsg;
	const int MAX_ENTRIES = 8;
	RsslRet BATCH_DECODE_RETURN[MAX_ENTRIES] = {
						RSSL_RET_BLANK_DATA, RSSL_RET_SUCCESS, RSSL_RET_BLANK_DATA,
						RSSL_RET_SUCCESS, RSSL_RET_SUCCESS,
						RSSL_RET_BLANK_DATA, RSSL_RET_BLANK_DATA, RSSL_RET_BLANK_DATA };
	RsslInt BATCH_STREAM_IDS[MAX_ENTRIES] = { 0, 5, 0, 8, 13, 0, 0, 0 };
	_jsonBuffer.data = (char*)"{\"Type\":\"Close\",\"ID\":[null,5,null,8,13,null,null,null]}";
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslCloseMsg is correct. */
	EXPECT_EQ(RSSL_MC_CLOSE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_TRUE(rsslCloseMsgCheckHasBatch(&rsslMsg.closeMsg));
	EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);

	/* Decode the Stream ID element in the dataBody. */
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslArray streamIdArray;
	RsslBuffer arrayEntry;
	RsslInt arrayValue;
	RsslRet ret;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementEntry(&_dIter, &elementEntry));
	ASSERT_TRUE(rsslBufferIsEqual(&RSSL_ENAME_BATCH_STREAMID_LIST, &elementEntry.name));
	ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

	/* Decode the Stream ID array. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &streamIdArray));
	ASSERT_EQ(RSSL_DT_INT, streamIdArray.primitiveType);

	for (int i = 0; i < MAX_ENTRIES; ++i)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry)) << "i: " << i;
		ret = rsslDecodeInt(&_dIter, &arrayValue);
		ASSERT_EQ(BATCH_DECODE_RETURN[i], ret) << "i: " << i << "; ret: " << ret << "; expected ret: " << BATCH_DECODE_RETURN[i];
		if (ret != RSSL_RET_SUCCESS)
			continue;
		EXPECT_EQ(BATCH_STREAM_IDS[i], arrayValue) << "i: " << i << "; arrayValue: " << arrayValue << "; expected val: " << BATCH_STREAM_IDS[i];
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeElementEntry(&_dIter, &elementEntry));
}

TEST_F(CloseMsgTests, BatchStreamIdNullArray1FailTest)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	/* ID with null data array */
	_jsonBuffer.data = (char*)"{\"ID\":[null],\"Type\":\"Close\"}";
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

	/* Expected an error - should be at least one non-zero streamId. */
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_NE(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) ;
	ASSERT_NE(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Check the error. */
	ASSERT_STREQ("JSON Unexpected Value. Received \'[null]\' for key \'ID\'", converterError.text);
}

TEST_F(CloseMsgTests, BatchStreamIdNullArray2FailTest)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	/* ID with null data array */
	_jsonBuffer.data = (char*)"{\"ID\":[null,null],\"Type\":\"Close\"}";
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

	/* Expected an error - should be at least one non-zero streamId */
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS);
	ASSERT_NE(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS);
	ASSERT_NE(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#endif

	/* Check the error. */
	ASSERT_STREQ("JSON Unexpected Value. Received \'[null,null]\' for key \'ID\'", converterError.text);
}

TEST_F(CloseMsgTests, BatchStreamIdNullFailTest)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;

	/* ID with null */
	_jsonBuffer.data = (char*)"{\"ID\":null,\"Type\":\"Close\"}";
	_jsonBuffer.length = (RsslUInt32)strlen(_jsonBuffer.data);

	/* Convert back to RWF. Error: Expected array. */
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS);
	ASSERT_NE(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError));
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS);
	ASSERT_NE(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError)) << "converterError. rsslErrorId: " << converterError.rsslErrorId << "; text: " << converterError.text;
#endif

	/* Check the error. */
	ASSERT_STREQ("JSON Converter Token Type error: Expected \'PRIMITIVE\' for key \'ID\' Received \'PRIMITIVE\'", converterError.text);
}
