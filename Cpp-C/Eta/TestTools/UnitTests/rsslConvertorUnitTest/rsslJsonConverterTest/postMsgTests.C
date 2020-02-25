/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace json; 

/* Parameters for PostMsg tests. */
class PostMsgMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool postId;
	bool key;
	bool seqNum;
	bool complete;
	bool ack;
	bool permData;
	bool partNum;
	bool postUserRights;

	PostMsgMembersTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool postId, bool key, bool seqNum, bool complete, bool ack, bool permData,
			bool partNum, bool postUserRights)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->postId = postId;
		this->key = key;
		this->seqNum = seqNum;
		this->complete = complete;
		this->ack = ack;
		this->permData = permData;
		this->partNum = partNum;
		this->postUserRights = postUserRights;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const PostMsgMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"postId:" << (params.postId ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"seqNum:" << (params.seqNum ? "true" : "false") << ","
			"complete:" << (params.complete ? "true" : "false") << ","
			"ack:" << (params.ack ? "true" : "false") << ","
			"permData:" << (params.permData ? "true" : "false") << ","
			"partNum:" << (params.partNum ? "true" : "false") << ","
			"postUserRights:" << (params.postUserRights ? "true" : "false")
			<< "]";
		return out;
	}
};

class PostMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<PostMsgMembersTestParams>
{
};

/* Test that converts a PostMsg with an UpdateMsg payload from RWF to JSON, and back to RWF. */
TEST_P(PostMsgMembersTestFixture, PostMsgMembersTest)
{
	PostMsgMembersTestParams const &params = GetParam();
	RsslPostMsg postMsg;
	RsslMsg rsslMsg;
	RsslUInt32 POST_ID = 1;

	/* Create and encode a simple PostMsg. */
	rsslClearPostMsg(&postMsg);

	postMsg.msgBase.streamId = 5;
	postMsg.msgBase.containerType = RSSL_DT_MSG;
	postMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	postMsg.postUserInfo.postUserAddr = IP_ADDRESS_UINT;
	postMsg.postUserInfo.postUserId = USER_ID;

	if (params.extendedHeader)
	{
		rsslPostMsgApplyHasExtendedHdr(&postMsg);
		postMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.key)
	{
		rsslPostMsgApplyHasMsgKey(&postMsg);
		rsslMsgKeyApplyHasName(&postMsg.msgBase.msgKey);
		postMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&postMsg.msgBase.msgKey);
		postMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.postId)
	{
		rsslPostMsgApplyHasPostId(&postMsg);
		postMsg.postId = POST_ID;
	}

	if (params.seqNum)
	{
		rsslPostMsgApplyHasSeqNum(&postMsg);
		postMsg.seqNum = SEQ_NUM;
	}

	if (params.complete)
		rsslPostMsgApplyPostComplete(&postMsg);

	if (params.ack)
		rsslPostMsgApplyAck(&postMsg);

	if (params.permData)
	{
		rsslPostMsgApplyHasPermData(&postMsg);
		postMsg.permData = PERM_DATA;
	}

	if (params.partNum)
	{
		rsslPostMsgApplyHasPartNum(&postMsg);
		postMsg.partNum = PART_NUM;
	}

	if (params.postUserRights)
	{
		rsslPostMsgApplyHasPostUserRights(&postMsg);
		postMsg.postUserRights = RSSL_PSUR_CREATE | RSSL_PSUR_DELETE;
	}

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&postMsg, 0));

	/* Encode the nested update */
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslUpdateMsg(&_eIter));

	/* Complete message encoding. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Post", _jsonDocument["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

			/* Check Domain. */
			ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */

			/* Check PostUserInfo. */
			ASSERT_TRUE(_jsonDocument.HasMember("PostUserInfo"));
			ASSERT_TRUE(_jsonDocument["PostUserInfo"].IsObject());

			ASSERT_TRUE(_jsonDocument["PostUserInfo"].HasMember("Address"));
			ASSERT_TRUE(_jsonDocument["PostUserInfo"]["Address"].IsString());
			EXPECT_STREQ(IP_ADDRESS_STR, _jsonDocument["PostUserInfo"]["Address"].GetString());

			ASSERT_TRUE(_jsonDocument["PostUserInfo"].HasMember("UserID"));
			ASSERT_TRUE(_jsonDocument["PostUserInfo"]["UserID"].IsNumber());
			EXPECT_EQ(USER_ID, _jsonDocument["PostUserInfo"]["UserID"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ExtHdr"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["ExtHdr"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ExtHdr"));

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

			/* Check Ack flag. */
			if (params.ack)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Ack"));
				ASSERT_TRUE(_jsonDocument["Ack"].IsBool());
				EXPECT_TRUE(_jsonDocument["Ack"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Ack"));

			/* Check PermData */
			if (params.permData)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PermData"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, _jsonDocument["PermData"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PermData"));

			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PartNumber"));
				ASSERT_TRUE(_jsonDocument["PartNumber"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["PartNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PartNumber"));

			/* Check PostUserRights */
			if (params.postUserRights)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PostUserRights"));
				ASSERT_TRUE(_jsonDocument["PostUserRights"].IsNumber());
				EXPECT_EQ(RSSL_PSUR_CREATE | RSSL_PSUR_DELETE, _jsonDocument["PostUserRights"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PostUserRights"));

			/* Check nested UpdateMsg. */
			ASSERT_TRUE(_jsonDocument.HasMember("Message"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(_jsonDocument["Message"], params.protocolType));

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
			EXPECT_EQ(RSSL_MC_POST, msgBase["c"].GetInt());

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
			EXPECT_EQ(RSSL_DT_MSG - 128, msgBase["f"].GetInt());

			/* Check PostUserInfo. */
			ASSERT_TRUE(_jsonDocument.HasMember("i"));
			ASSERT_TRUE(_jsonDocument["i"].IsObject());

			ASSERT_TRUE(_jsonDocument["i"].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["i"]["a"].IsNumber());
			EXPECT_EQ(IP_ADDRESS_UINT, _jsonDocument["i"]["a"].GetInt());

			ASSERT_TRUE(_jsonDocument["i"].HasMember("u"));
			ASSERT_TRUE(_jsonDocument["i"]["u"].IsNumber());
			EXPECT_EQ(USER_ID, _jsonDocument["i"]["u"].GetInt());

			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("e"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["e"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("e"));

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

			/* Check Ack flag. */
			if (params.ack)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("a"));
				ASSERT_TRUE(_jsonDocument["a"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["a"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("a"));

			/* Check PermData */
			if (params.permData)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("p"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, _jsonDocument["p"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("p"));

			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("m"));
				ASSERT_TRUE(_jsonDocument["m"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["m"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("m"));

			/* Check PostUserRights */
			if (params.postUserRights)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("r"));
				ASSERT_TRUE(_jsonDocument["r"].IsNumber());
				EXPECT_EQ(RSSL_PSUR_CREATE | RSSL_PSUR_DELETE, _jsonDocument["r"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("r"));

			/* Check nested UpdateMsg. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(_jsonDocument["d"], params.protocolType));
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

	/* Verify that RsslPostMsg is correct. */
	EXPECT_EQ(RSSL_MC_POST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_MSG, rsslMsg.msgBase.containerType);

	/* Check PostUserInfo. */
	EXPECT_EQ(IP_ADDRESS_UINT, rsslMsg.postMsg.postUserInfo.postUserAddr);
	EXPECT_EQ(USER_ID, rsslMsg.postMsg.postUserInfo.postUserId);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasExtendedHdr(&rsslMsg.postMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.postMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasExtendedHdr(&rsslMsg.postMsg));

	/* Check PostId */
	if (params.postId)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasPostId(&rsslMsg.postMsg));
		EXPECT_EQ(POST_ID, rsslMsg.postMsg.postId);
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasPostId(&rsslMsg.postMsg));

	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasMsgKey(&rsslMsg.postMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasMsgKey(&rsslMsg.postMsg));

	/* Check SeqNum. */
	if (params.seqNum)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasSeqNum(&rsslMsg.postMsg));
		EXPECT_EQ(SEQ_NUM, rsslMsg.postMsg.seqNum);
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasSeqNum(&rsslMsg.postMsg));

	/* Check Complete flag. */
	EXPECT_EQ(params.complete, rsslPostMsgCheckPostComplete(&rsslMsg.postMsg));

	/* Check Ack flag. */
	EXPECT_EQ(params.ack, rsslPostMsgCheckAck(&rsslMsg.postMsg));

	/* Check PermData. */
	if (params.permData)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasPermData(&rsslMsg.postMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&PERM_DATA, &rsslMsg.postMsg.permData));
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasPermData(&rsslMsg.postMsg));

	/* Check PartNum. */
	if (params.partNum)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasPartNum(&rsslMsg.postMsg));
		EXPECT_EQ(PART_NUM, rsslMsg.postMsg.partNum);
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasPartNum(&rsslMsg.postMsg));

	/* Check PostUserRights. */
	if (params.postUserRights)
	{
		ASSERT_TRUE(rsslPostMsgCheckHasPostUserRights(&rsslMsg.postMsg));
		EXPECT_EQ(RSSL_PSUR_CREATE | RSSL_PSUR_DELETE, rsslMsg.postMsg.postUserRights);
	}
	else
		EXPECT_FALSE(rsslPostMsgCheckHasPostUserRights(&rsslMsg.postMsg));

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslUpdateMsg(params.protocolType, &_dIter));
}



INSTANTIATE_TEST_CASE_P(PostMsgTests, PostMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, PostId, MsgKey, SeqNum, Complete, Ack, PermData, PartNum, PostUserRights */

	/* Defaults */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, false, false),

	/* ExtendedHeader */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, true, false, false, false, true, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false, true, false, false, false, false),

	/* PostId */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, true, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, true, false, false, false, false),

	/* MsgKey */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, true, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, true, false, false, false, false),

	/* SeqNum */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, true, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, true, false, false, false, false),

	/* Complete (false), */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, false, false),

	/* Ack */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, false, false),

	/* PermData */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, true, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, true, false, false),

	/* PartNum */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, true, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, true, false),

	/* PostUserRights */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, false, true),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, false, true),

	/* PostId, SeqNum, Ack, and Complete (false) */
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false, true, false, true, false, false, false),
	PostMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false, true, false, true, false, false, false)
));


