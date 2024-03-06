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

/* Parameters for AckMsg tests. */
class AckMsgMemberTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool text;
	bool privateStream;
	bool seqNum;
	bool key;
	bool nakCode;
	bool qualified;
	int dummy; /* This class is small; silence a Valgrind warning when GoogleTest uses it. */

	AckMsgMemberTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool text, bool privateStream, bool seqNum, bool key, bool nakCode, bool qualified)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->text = text;
		this->privateStream = privateStream;
		this->seqNum = seqNum;
		this->key = key;
		this->nakCode = nakCode;
		this->qualified = qualified;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const AckMsgMemberTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"text:" << (params.text ? "true" : "false") << ","
			"privateStream:" << (params.privateStream ? "true" : "false") << ","
			"seqNum:" << (params.seqNum ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"nakCode:" << (params.nakCode ? "true" : "false") << ","
			"qualified:" << (params.qualified ? "true" : "false")
			<< "]";
		return out;
	}
};

class AckMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<AckMsgMemberTestParams>
{
};

/* Test that converts a AckMsg from RWF to JSON, and back to RWF. */
TEST_P(AckMsgMembersTestFixture, AckMsgMembersTest)
{
	AckMsgMemberTestParams const &params = GetParam();
	RsslAckMsg ackMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple AckMsg. */
	rsslClearAckMsg(&ackMsg);

	ackMsg.msgBase.streamId = 5;
	ackMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	ackMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	ackMsg.ackId = 7;

	if (params.extendedHeader)
	{
		rsslAckMsgApplyHasExtendedHdr(&ackMsg);
		ackMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.text)
	{
		rsslAckMsgApplyHasText(&ackMsg);
		ackMsg.text = ACK_TEXT;
	}

	if (params.privateStream)
		rsslAckMsgApplyPrivateStream(&ackMsg);

	if (params.seqNum)
	{
		rsslAckMsgApplyHasSeqNum(&ackMsg);
		ackMsg.seqNum = SEQ_NUM;
	}

	if (params.key)
	{
		rsslAckMsgApplyHasMsgKey(&ackMsg);
		rsslMsgKeyApplyHasName(&ackMsg.msgBase.msgKey);
		ackMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&ackMsg.msgBase.msgKey);
		ackMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.nakCode)
	{
		rsslAckMsgApplyHasNakCode(&ackMsg);
		ackMsg.nakCode = RSSL_NAKC_ACCESS_DENIED;
	}

	if (params.qualified)
		ackMsg.flags |= RSSL_AKMF_QUALIFIED_STREAM;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&ackMsg));
	
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{

			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Ack", _jsonDocument["Type"].GetString());

			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

			/* Check Domain. */
			ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */


			/* Check AckId. */
			ASSERT_TRUE(_jsonDocument.HasMember("AckID"));
			ASSERT_TRUE(_jsonDocument["AckID"].IsNumber());
			EXPECT_EQ(7, _jsonDocument["AckID"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ExtHdr"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["ExtHdr"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ExtHdr"));

			/* Check Text */
			if (params.text)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Text"));
				ASSERT_TRUE(_jsonDocument["Text"].IsString());
				EXPECT_STREQ(ACK_TEXT.data, _jsonDocument["Text"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Text"));

			/* Check PrivateStream. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Private"));
				ASSERT_TRUE(_jsonDocument["Private"].IsBool());
				ASSERT_TRUE(_jsonDocument["Private"].GetBool());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("Private"));

			/* Check SeqNum. */
			if (params.seqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("SeqNumber"));
				ASSERT_TRUE(_jsonDocument["SeqNumber"].IsNumber());
				ASSERT_EQ(SEQ_NUM, _jsonDocument["SeqNumber"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("SeqNumber"));

			/* Check MsgKey */
			if (params.key)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Key"));
				ASSERT_TRUE(_jsonDocument["Key"].IsObject());

				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Name"));
				ASSERT_TRUE(_jsonDocument["Key"]["Name"].IsString());
				EXPECT_STREQ(MSG_KEY_NAME.data, _jsonDocument["Key"]["Name"].GetString());

				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Service"));
				ASSERT_TRUE(_jsonDocument["Key"]["Service"].IsString());
				EXPECT_STREQ(SERVICE_NAME.data, _jsonDocument["Key"]["Service"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Key"));

			/* Check NakCode. */
			if (params.nakCode)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("NakCode"));
				ASSERT_TRUE(_jsonDocument["NakCode"].IsString());
				EXPECT_STREQ("AccessDenied", _jsonDocument["NakCode"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("NakCode"));

			/* Check Qualified. */
			if (params.qualified)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Qualified"));
				ASSERT_TRUE(_jsonDocument["Qualified"].IsBool());
				ASSERT_TRUE(_jsonDocument["Qualified"].GetBool());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("Qualified"));
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
			EXPECT_EQ(RSSL_MC_ACK, msgBase["c"].GetInt());

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
			EXPECT_EQ(RSSL_DT_NO_DATA - 128, msgBase["f"].GetInt());

			/* Check AckId. */
			ASSERT_TRUE(_jsonDocument.HasMember("i"));
			ASSERT_TRUE(_jsonDocument["i"].IsNumber());
			EXPECT_EQ(7, _jsonDocument["i"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("e"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["e"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("e"));

			/* Check Text */
			if (params.text)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("t"));
				ASSERT_TRUE(_jsonDocument["t"].IsString());
				EXPECT_STREQ(ACK_TEXT.data, _jsonDocument["t"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("t"));

			/* Check PrivateStream. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("u"));
				ASSERT_TRUE(_jsonDocument["u"].IsNumber());
				ASSERT_EQ(1, _jsonDocument["u"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("u"));

			/* Check SeqNum. */
			if (params.seqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("n"));
				ASSERT_TRUE(_jsonDocument["n"].IsNumber());
				ASSERT_EQ(SEQ_NUM, _jsonDocument["n"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("n"));

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

			/* Check NakCode. */
			if (params.nakCode)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"].IsNumber());
				EXPECT_EQ(RSSL_NAKC_ACCESS_DENIED, _jsonDocument["c"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("c"));

			/* Check Qualified. */
			if (params.qualified)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("o"));
				ASSERT_TRUE(_jsonDocument["o"].IsNumber());
				ASSERT_EQ(1, _jsonDocument["o"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("o"));
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

	/* Verify that RsslAckMsg is correct. */
	EXPECT_EQ(RSSL_MC_ACK, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_NO_DATA, rsslMsg.msgBase.containerType);

	/* Check AckId. */
	EXPECT_EQ(7, rsslMsg.ackMsg.ackId);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslAckMsgCheckHasExtendedHdr(&rsslMsg.ackMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.ackMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslAckMsgCheckHasExtendedHdr(&rsslMsg.ackMsg));

	/* Check Text. */
	if (params.text)
	{
		ASSERT_TRUE(rsslAckMsgCheckHasText(&rsslMsg.ackMsg));
		EXPECT_TRUE(rsslBufferIsEqual(&ACK_TEXT, &rsslMsg.ackMsg.text));
	}
	else
		EXPECT_FALSE(rsslAckMsgCheckHasText(&rsslMsg.ackMsg));

	/* Check PrivateStream. */
	ASSERT_EQ(params.privateStream, rsslAckMsgCheckPrivateStream(&rsslMsg.ackMsg));

	/* Check SeqNum. */
	if (params.seqNum)
	{
		ASSERT_TRUE(rsslAckMsgCheckHasSeqNum(&rsslMsg.ackMsg));
		EXPECT_EQ(SEQ_NUM, rsslMsg.ackMsg.seqNum);
	}
	else
		EXPECT_FALSE(rsslAckMsgCheckHasSeqNum(&rsslMsg.ackMsg));
	
	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslAckMsgCheckHasMsgKey(&rsslMsg.ackMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslAckMsgCheckHasMsgKey(&rsslMsg.ackMsg));

	/* Check NakCode. */
	if (params.nakCode)
	{
		ASSERT_TRUE(rsslAckMsgCheckHasNakCode(&rsslMsg.ackMsg));
		EXPECT_EQ(RSSL_NAKC_ACCESS_DENIED, rsslMsg.ackMsg.nakCode);
	}
	else
		EXPECT_FALSE(rsslAckMsgCheckHasNakCode(&rsslMsg.ackMsg));

	/* Check Qualified. */
	ASSERT_EQ(params.qualified, rsslAckMsgCheckQualifiedStream(&rsslMsg.ackMsg));
}

INSTANTIATE_TEST_SUITE_P(AckMsgTests, AckMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, Text, PrivateStream, SeqNum, MsgKey, NakCode, Qualified. */

	/* Defaults */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false),

	/* ExtendedHeader */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, true, false, false, false, false, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false, false, false, false),

	/* Text */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, false, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, false, false, false),

	/* PrivateStream */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, false, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, false, false, false),

	/* SeqNum */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, false, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, false, false, false),

	/* MsgKey */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false),

	/* NakCode */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, true, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, true, false),

	/* Text and NakCode */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, false, true, false),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, false, true, false),

	/* Qualified */
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, true),
	AckMsgMemberTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, true)

));

