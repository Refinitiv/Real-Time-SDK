/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json; 

/* Fixture for GenericMsgTests that has conversion code. */
class GenericMsgTests : public MsgConversionTestBase
{
   public:
	   GenericMsgTests() {}
};

/* Parameters for GenericMsg tests. */
class GenericMsgTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool permData;
	bool key;
	bool seqNum;
	bool complete;
	bool secondarySeqNum;
	bool partNum;
	bool reqKey;

	GenericMsgTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool permData, bool key, bool seqNum, bool complete,
			bool secondarySeqNum, bool partNum, bool reqKey)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->permData = permData;
		this->key = key;
		this->seqNum = seqNum;
		this->complete = complete;
		this->secondarySeqNum = secondarySeqNum;
		this->partNum = partNum;
		this->reqKey = reqKey;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const GenericMsgTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"permData:" << (params.permData ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"seqNum:" << (params.seqNum ? "true" : "false") << ","
			"complete:" << (params.complete ? "true" : "false") << ","
			"secondarySeqNum:" << (params.secondarySeqNum ? "true" : "false") << ","
			"partNum:" << (params.partNum ? "true" : "false") << ","
			"reqKey:" << (params.reqKey ? "true" : "false")
			<< "]";
		return out;
	}
};

class GenericMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<GenericMsgTestParams>
{
  public:
	  GenericMsgMembersTestFixture() {}
};

/* Test that converts a GenericMsg with a FieldList from RWF to JSON, and back to RWF. */
TEST_P(GenericMsgMembersTestFixture, GenericMsgMembersTest)
{
	GenericMsgTestParams const &params = GetParam();
	RsslGenericMsg genericMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple GenericMsg. */
	rsslClearGenericMsg(&genericMsg);

	genericMsg.msgBase.streamId = 5;
	genericMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	genericMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	if (params.extendedHeader)
	{
		rsslGenericMsgApplyHasExtendedHdr(&genericMsg);
		genericMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.permData)
	{
		rsslGenericMsgApplyHasPermData(&genericMsg);
		genericMsg.permData = PERM_DATA;
	}

	if (params.key)
	{
		rsslGenericMsgApplyHasMsgKey(&genericMsg);
		rsslMsgKeyApplyHasName(&genericMsg.msgBase.msgKey);
		genericMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&genericMsg.msgBase.msgKey);
		genericMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.seqNum)
	{
		rsslGenericMsgApplyHasSeqNum(&genericMsg);
		genericMsg.seqNum = SEQ_NUM;
	}

	if (params.complete)
		rsslGenericMsgApplyMessageComplete(&genericMsg);

	if (params.secondarySeqNum)
	{
		rsslGenericMsgApplyHasSecondarySeqNum(&genericMsg);
		genericMsg.secondarySeqNum = SECONDARY_SEQ_NUM;
	}

	if (params.partNum)
	{
		rsslGenericMsgApplyHasPartNum(&genericMsg);
		genericMsg.partNum = PART_NUM;
	}

	if (params.reqKey)
	{
		genericMsg.flags |= RSSL_GNMF_HAS_REQ_MSG_KEY;
		rsslMsgKeyApplyHasName(&genericMsg.reqMsgKey);
		genericMsg.reqMsgKey.name = REQ_MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&genericMsg.reqMsgKey);
		genericMsg.reqMsgKey.serviceId = REQ_MSGKEY_SVC_ID;
	}

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&genericMsg, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Generic", _jsonDocument["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

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

			/* Check PermData */
			if (params.permData)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PermData"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, _jsonDocument["PermData"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PermData"));

			/* Check MsgKey */
			if (params.key)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Key"));
				ASSERT_TRUE(_jsonDocument["Key"].IsObject());

				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Name"));
				ASSERT_TRUE(_jsonDocument["Key"]["Name"].IsString());
				EXPECT_STREQ(MSG_KEY_NAME.data, _jsonDocument["Key"]["Name"].GetString());

				/* GenericMsg.Key.Service ID shouldn't be translated to name. */
				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Service"));
				ASSERT_TRUE(_jsonDocument["Key"]["Service"].IsNumber());
				EXPECT_EQ(MSGKEY_SVC_ID, _jsonDocument["Key"]["Service"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Key"));

			/* Check SeqNum */
			if (params.seqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("SeqNumber"));
				ASSERT_TRUE(_jsonDocument["SeqNumber"].IsNumber());
				EXPECT_EQ(SEQ_NUM, _jsonDocument["SeqNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("SeqNumber"));

			/* Check Complete flag. */
			if (params.complete)
				EXPECT_FALSE(_jsonDocument.HasMember("Complete"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Complete"));
				ASSERT_TRUE(_jsonDocument["Complete"].IsBool());
				EXPECT_FALSE(_jsonDocument["Complete"].GetBool());
			}

			/* Check SecondarySeqNum */
			if (params.secondarySeqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("SecSeqNumber"));
				ASSERT_TRUE(_jsonDocument["SecSeqNumber"].IsNumber());
				EXPECT_EQ(SECONDARY_SEQ_NUM, _jsonDocument["SecSeqNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("SecSeqNumber"));

			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PartNumber"));
				ASSERT_TRUE(_jsonDocument["PartNumber"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["PartNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PartNumber"));

			/* Check ReqMsgKey. */
			if (params.reqKey)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ReqKey"));
				ASSERT_TRUE(_jsonDocument["ReqKey"].IsObject());

				ASSERT_TRUE(_jsonDocument["ReqKey"].HasMember("Name"));
				ASSERT_TRUE(_jsonDocument["ReqKey"]["Name"].IsString());
				EXPECT_STREQ(REQ_MSG_KEY_NAME.data, _jsonDocument["ReqKey"]["Name"].GetString());

				ASSERT_TRUE(_jsonDocument["ReqKey"].HasMember("Service"));
				ASSERT_TRUE(_jsonDocument["ReqKey"]["Service"].IsString());
				EXPECT_STREQ(SERVICE_NAME.data, _jsonDocument["ReqKey"]["Service"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ReqKey"));

			/* Check Field List. */
			ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Fields"]));

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
			EXPECT_EQ(RSSL_MC_GENERIC, msgBase["c"].GetInt());

			/* Check Stream ID. */
			ASSERT_TRUE(msgBase.HasMember("s"));
			ASSERT_TRUE(msgBase["s"].IsNumber());
			EXPECT_EQ(5, msgBase["s"].GetInt());

			/* Check Domain. */
			ASSERT_TRUE(msgBase.HasMember("t"));
			ASSERT_TRUE(msgBase["t"].IsNumber());
			EXPECT_EQ(RSSL_DMT_MARKET_PRICE, msgBase["t"].GetInt());

			/* Check ContainerType. */
			ASSERT_TRUE(msgBase.HasMember("f"));
			ASSERT_TRUE(msgBase["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_FIELD_LIST - 128, msgBase["f"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("e"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["e"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("e"));

			/* Check PermData */
			if (params.permData)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("p"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, _jsonDocument["p"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("p"));

			/* Check MsgKey */
			if (params.key)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("k"));
				ASSERT_TRUE(_jsonDocument["k"].IsObject());

				ASSERT_TRUE(_jsonDocument["k"].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["k"]["n"].IsString());
				EXPECT_STREQ(MSG_KEY_NAME.data, _jsonDocument["k"]["n"].GetString());

				ASSERT_TRUE(_jsonDocument["k"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["k"]["s"].IsNumber());
				EXPECT_EQ(MSGKEY_SVC_ID, _jsonDocument["k"]["s"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("k"));

			/* Check SeqNum */
			if (params.seqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("n"));
				ASSERT_TRUE(_jsonDocument["n"].IsNumber());
				EXPECT_EQ(SEQ_NUM, _jsonDocument["n"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("n"));

			/* Check Complete flag. */
			if (params.complete)
				EXPECT_FALSE(_jsonDocument.HasMember("c"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["c"].GetInt());
			}

			/* Check SecondarySeqNum */
			if (params.secondarySeqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("s"));
				ASSERT_TRUE(_jsonDocument["s"].IsNumber());
				EXPECT_EQ(SECONDARY_SEQ_NUM, _jsonDocument["s"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("s"));

			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("m"));
				ASSERT_TRUE(_jsonDocument["m"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["m"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("m"));

			/* Check ReqMsgKey. */
			if (params.reqKey)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("r"));
				ASSERT_TRUE(_jsonDocument["r"].IsObject());

				ASSERT_TRUE(_jsonDocument["r"].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["r"]["n"].IsString());
				EXPECT_STREQ(REQ_MSG_KEY_NAME.data, _jsonDocument["r"]["n"].GetString());

				ASSERT_TRUE(_jsonDocument["r"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["r"]["s"].IsNumber());
				EXPECT_EQ(MSGKEY_SVC_ID, _jsonDocument["r"]["s"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("r"));

			/* Check Field List. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"], params.protocolType));
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
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer); rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslGenericMsg is correct. */
	EXPECT_EQ(RSSL_MC_GENERIC, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasExtendedHdr(&rsslMsg.genericMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.genericMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasExtendedHdr(&rsslMsg.genericMsg));

	/* Check PermData. */
	if (params.permData)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasPermData(&rsslMsg.genericMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&PERM_DATA, &rsslMsg.genericMsg.permData));
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasPermData(&rsslMsg.genericMsg));

	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasMsgKey(&rsslMsg.genericMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasMsgKey(&rsslMsg.genericMsg));

	/* Check SeqNum. */
	if (params.seqNum)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasSeqNum(&rsslMsg.genericMsg));
		EXPECT_EQ(SEQ_NUM, rsslMsg.genericMsg.seqNum);
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasSeqNum(&rsslMsg.genericMsg));

	/* Check Complete flag. */
	EXPECT_EQ(params.complete, rsslGenericMsgCheckMessageComplete(&rsslMsg.genericMsg));

	/* Check SecondarySeqNum. */
	if (params.secondarySeqNum)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasSecondarySeqNum(&rsslMsg.genericMsg));
		EXPECT_EQ(SECONDARY_SEQ_NUM, rsslMsg.genericMsg.secondarySeqNum);
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasSecondarySeqNum(&rsslMsg.genericMsg));

	/* Check PartNum. */
	if (params.partNum)
	{
		ASSERT_TRUE(rsslGenericMsgCheckHasPartNum(&rsslMsg.genericMsg));
		EXPECT_EQ(PART_NUM, rsslMsg.genericMsg.partNum);
	}
	else
		EXPECT_FALSE(rsslGenericMsgCheckHasPartNum(&rsslMsg.genericMsg));

	/* Check ReqKey. */
	if (params.reqKey)
	{
		ASSERT_TRUE(rsslMsg.genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY);

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.genericMsg.reqMsgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&REQ_MSG_KEY_NAME, &rsslMsg.genericMsg.reqMsgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.genericMsg.reqMsgKey));
		EXPECT_EQ(REQ_MSGKEY_SVC_ID, rsslMsg.genericMsg.reqMsgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslMsg.genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY);
	
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
}



INSTANTIATE_TEST_SUITE_P(GenericMsgTests, GenericMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, PermData, MsgKey, SeqNum, Complete, SecondarySeqNum, PartNum, ReqMsgKey */

	/* Defaults */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, false),

	/* ExtendedHeader */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, true, false, false, false, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false, true, false, false, false),

	/* PermData */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, true, false, false, false),

	/* MsgKey */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, true, false, false, false),

	/* SeqNum */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, true, false, false, false),

	/* Complete (false), */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, false),

	/* MsgKey, SeqNum */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, true, false, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, true, false, false, false),

	/* SecondarySeqNum */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, false),

	/* MsgKey, SeqNum, SecondarySeqNum */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, true, true, false, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, true, true, false, false),

	/* PartNum */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, true, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, true, false),

	/* PartNum and Complete (false), */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, true, false),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, true, false),

	/* ReqMsgKey */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, true),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, true),

	/* All optional members. */
	GenericMsgTestParams(RSSL_JSON_JPT_JSON, true, true, true, true, false, true, true, true),
	GenericMsgTestParams(RSSL_JSON_JPT_JSON2, true, true, true, true, false, true, true, true)
));

/* Test a GenericMsg with a nested message. */
TEST_F(GenericMsgTests, GenericMsgWithMsgPayloadTest)
{
	RsslGenericMsg genericMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple GenericMsg. */
	rsslClearGenericMsg(&genericMsg);

	genericMsg.msgBase.streamId = 5;
	genericMsg.msgBase.containerType = RSSL_DT_MSG;
	genericMsg.msgBase.domainType = 129;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&genericMsg, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslUpdateMsg(&_eIter));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message type. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Generic", _jsonDocument["Type"].GetString());

	/* Check Stream ID. */
	ASSERT_TRUE(_jsonDocument.HasMember("ID"));
	ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
	EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

	/* Check Domain. */
	ASSERT_TRUE(_jsonDocument.HasMember("Domain")); 
	ASSERT_TRUE(_jsonDocument["Domain"].IsNumber());
	EXPECT_EQ(129, _jsonDocument["Domain"].GetInt());

	/* Check Payload. */
	ASSERT_TRUE(_jsonDocument.HasMember("Message"));
	ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(_jsonDocument["Message"]));

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer); rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslGenericMsg is correct. */
	EXPECT_EQ(RSSL_MC_GENERIC, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(129, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_MSG, rsslMsg.msgBase.containerType);
	
	/* Check FieldList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslUpdateMsg(RSSL_JSON_JPT_JSON2, &_dIter));
}
