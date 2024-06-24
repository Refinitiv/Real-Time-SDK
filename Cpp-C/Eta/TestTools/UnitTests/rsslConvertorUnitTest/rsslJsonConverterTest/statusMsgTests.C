/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 LSEG. All rights reserved.      –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json; 

/* Parameters for StatusMsg tests. */
class StatusMsgMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool permData;
	bool key;
	bool groupId;
	bool state;
	bool clearCache;
	bool privateStream;
	bool postUserInfo;
	bool reqKey;
	bool qualifiedStream;

	StatusMsgMembersTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool permData, bool key, bool groupId, bool state, bool clearCache,
			bool privateStream, bool postUserInfo, bool reqKey, bool qualifiedStream)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->permData = permData;
		this->key = key;
		this->groupId = groupId;
		this->state = state;
		this->clearCache = clearCache;
		this->privateStream = privateStream;
		this->postUserInfo = postUserInfo;
		this->reqKey = reqKey;
		this->qualifiedStream = qualifiedStream;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const StatusMsgMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"permData:" << (params.permData ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"groupId:" << (params.groupId ? "true" : "false") << ","
			"state:" << (params.state ? "true" : "false") << ","
			"clearCache:" << (params.clearCache ? "true" : "false") << ","
			"privateStream:" << (params.privateStream ? "true" : "false") << ","
			"postUserInfo:" << (params.postUserInfo ? "true" : "false") << ","
			"reqKey:" << (params.reqKey ? "true" : "false") << ","
			"qualifiedStream:" << (params.qualifiedStream ? "true" : "false")
			<< "]";
		return out;
	}
};

class StatusMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<StatusMsgMembersTestParams>
{
};

/* Test that converts a StatusMsg from RWF to JSON, and back to RWF. */
TEST_P(StatusMsgMembersTestFixture, StatusMsgMembersTest)
{
	StatusMsgMembersTestParams const &params = GetParam();
	RsslStatusMsg statusMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple StatusMsg. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = 5;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_BY_ORDER;

	if (params.extendedHeader)
	{
		rsslStatusMsgApplyHasExtendedHdr(&statusMsg);
		statusMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.permData)
	{
		rsslStatusMsgApplyHasPermData(&statusMsg);
		statusMsg.permData = PERM_DATA;
	}

	if (params.key)
	{
		rsslStatusMsgApplyHasMsgKey(&statusMsg);
		rsslMsgKeyApplyHasName(&statusMsg.msgBase.msgKey);
		statusMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&statusMsg.msgBase.msgKey);
		statusMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.groupId)
	{
		rsslStatusMsgApplyHasGroupId(&statusMsg);
		statusMsg.groupId = GROUP_ID;
	}

	if (params.state)
	{
		rsslStatusMsgApplyHasState(&statusMsg);
		statusMsg.state.streamState = RSSL_STREAM_CLOSED;
		statusMsg.state.dataState = RSSL_DATA_SUSPECT;
		statusMsg.state.code = RSSL_SC_NOT_FOUND;
		statusMsg.state.text = STATE_TEXT;
	}

	if (params.clearCache)
		rsslStatusMsgApplyClearCache(&statusMsg);

	if (params.privateStream)
		rsslStatusMsgApplyPrivateStream(&statusMsg);

	if (params.reqKey)
	{
		statusMsg.flags |= RSSL_STMF_HAS_REQ_MSG_KEY;
		rsslMsgKeyApplyHasName(&statusMsg.reqMsgKey);
		statusMsg.reqMsgKey.name = REQ_MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&statusMsg.reqMsgKey);
		statusMsg.reqMsgKey.serviceId = REQ_MSGKEY_SVC_ID;
	}

	if (params.postUserInfo)
	{
		rsslStatusMsgApplyHasPostUserInfo(&statusMsg);
		statusMsg.postUserInfo.postUserAddr = IP_ADDRESS_UINT;
		statusMsg.postUserInfo.postUserId = USER_ID;
	}

	if (params.qualifiedStream)
		rsslStatusMsgApplyQualifiedStream(&statusMsg);

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&statusMsg));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Status", _jsonDocument["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

			/* Check Domain. */
			ASSERT_TRUE(_jsonDocument.HasMember("Domain"));
			ASSERT_TRUE(_jsonDocument["Domain"].IsString());
			EXPECT_STREQ("MarketByOrder", _jsonDocument["Domain"].GetString());

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

				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Service"));
				ASSERT_TRUE(_jsonDocument["Key"]["Service"].IsString());
				EXPECT_STREQ(SERVICE_NAME.data, _jsonDocument["Key"]["Service"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Key"));

			/* Ignore GroupId. It shouldn't be present and there's no keyword defined for it. It will
			 * be verified to not be present after conversion back to RWF. */

			/* Check State. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("State"));
				ASSERT_TRUE(_jsonDocument["State"].IsObject());
				ASSERT_TRUE(_jsonDocument["State"].HasMember("Stream"));
				ASSERT_TRUE(_jsonDocument["State"]["Stream"].IsString());
				EXPECT_STREQ("Closed", _jsonDocument["State"]["Stream"].GetString());
				ASSERT_TRUE(_jsonDocument["State"].HasMember("Data"));
				ASSERT_TRUE(_jsonDocument["State"]["Data"].IsString());
				EXPECT_STREQ("Suspect", _jsonDocument["State"]["Data"].GetString());
				ASSERT_TRUE(_jsonDocument["State"].HasMember("Code"));
				ASSERT_TRUE(_jsonDocument["State"]["Code"].IsString());
				EXPECT_STREQ("NotFound", _jsonDocument["State"]["Code"].GetString());
				ASSERT_TRUE(_jsonDocument["State"].HasMember("Text"));
				ASSERT_TRUE(_jsonDocument["State"]["Text"].IsString());
				EXPECT_STREQ(STATE_TEXT.data, _jsonDocument["State"]["Text"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("State"));

			/* Check ClearCache flag. */
			if (params.clearCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ClearCache"));
				ASSERT_TRUE(_jsonDocument["ClearCache"].IsBool());
				EXPECT_TRUE(_jsonDocument["ClearCache"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ClearCache"));

			/* Check PrivateStream flag. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Private"));
				ASSERT_TRUE(_jsonDocument["Private"].IsBool());
				EXPECT_TRUE(_jsonDocument["Private"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Private"));

			/* Check PostUserInfo */
			if (params.postUserInfo)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PostUserInfo"));
				ASSERT_TRUE(_jsonDocument["PostUserInfo"].IsObject());

				ASSERT_TRUE(_jsonDocument["PostUserInfo"].HasMember("Address"));
				ASSERT_TRUE(_jsonDocument["PostUserInfo"]["Address"].IsString());
				EXPECT_STREQ(IP_ADDRESS_STR, _jsonDocument["PostUserInfo"]["Address"].GetString());

				ASSERT_TRUE(_jsonDocument["PostUserInfo"].HasMember("UserID"));
				ASSERT_TRUE(_jsonDocument["PostUserInfo"]["UserID"].IsNumber());
				EXPECT_EQ(USER_ID, _jsonDocument["PostUserInfo"]["UserID"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PostUserInfo"));

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

			/* Qualified */
			if (params.qualifiedStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Qualified"));
				ASSERT_TRUE(_jsonDocument["Qualified"].IsBool());
				EXPECT_TRUE(_jsonDocument["Qualified"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Qualified"));

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
			EXPECT_EQ(RSSL_MC_STATUS, msgBase["c"].GetInt());

			/* Check Stream ID. */
			ASSERT_TRUE(msgBase.HasMember("s"));
			ASSERT_TRUE(msgBase["s"].IsNumber());
			EXPECT_EQ(5, msgBase["s"].GetInt());

			/* Check Domain. */
			ASSERT_TRUE(msgBase.HasMember("t"));
			ASSERT_TRUE(msgBase["t"].IsNumber());
			EXPECT_EQ(RSSL_DMT_MARKET_BY_ORDER, msgBase["t"].GetInt());

			/* Check ContainerType. */
			ASSERT_TRUE(msgBase.HasMember("f"));
			ASSERT_TRUE(msgBase["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_NO_DATA - 128, msgBase["f"].GetInt());


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

			/* Check Group. */
			if (params.groupId)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("g"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&GROUP_ID, _jsonDocument["g"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("g"));

			/* Check State. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("s"));
				ASSERT_TRUE(_jsonDocument["s"].IsObject());
				ASSERT_TRUE(_jsonDocument["s"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["s"]["s"].IsNumber());
				EXPECT_EQ(RSSL_STREAM_CLOSED, _jsonDocument["s"]["s"].GetInt());
				ASSERT_TRUE(_jsonDocument["s"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["s"]["d"].IsNumber());
				EXPECT_EQ(RSSL_DATA_SUSPECT, _jsonDocument["s"]["d"].GetInt());
				ASSERT_TRUE(_jsonDocument["s"].HasMember("c"));
				ASSERT_TRUE(_jsonDocument["s"]["c"].IsNumber());
				EXPECT_EQ(RSSL_SC_NOT_FOUND, _jsonDocument["s"]["c"].GetInt());
				ASSERT_TRUE(_jsonDocument["s"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["s"]["t"].IsString());
				EXPECT_STREQ(STATE_TEXT.data, _jsonDocument["s"]["t"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("s"));

			/* Check ClearCache flag. */
			if (params.clearCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("t"));
				ASSERT_TRUE(_jsonDocument["t"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["t"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("t"));

			/* Check PrivateStream flag. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("u"));
				ASSERT_TRUE(_jsonDocument["u"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["u"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("u"));

			/* Check PostUserInfo */
			if (params.postUserInfo)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("i"));
				ASSERT_TRUE(_jsonDocument["i"].IsObject());

				ASSERT_TRUE(_jsonDocument["i"].HasMember("a"));
				ASSERT_TRUE(_jsonDocument["i"]["a"].IsNumber());
				EXPECT_EQ(IP_ADDRESS_UINT, _jsonDocument["i"]["a"].GetInt());

				ASSERT_TRUE(_jsonDocument["i"].HasMember("u"));
				ASSERT_TRUE(_jsonDocument["i"]["u"].IsNumber());
				EXPECT_EQ(USER_ID, _jsonDocument["i"]["u"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("i"));

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

			/* Qualified */
			if (params.qualifiedStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("o"));
				ASSERT_TRUE(_jsonDocument["o"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["o"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("o"));

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

	/* Verify that RsslStatusMsg is correct. */
	EXPECT_EQ(RSSL_MC_STATUS, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_BY_ORDER, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_NO_DATA, rsslMsg.msgBase.containerType);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslStatusMsgCheckHasExtendedHdr(&rsslMsg.statusMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.statusMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslStatusMsgCheckHasExtendedHdr(&rsslMsg.statusMsg));

	/* Check PermData. */
	if (params.permData)
	{
		ASSERT_TRUE(rsslStatusMsgCheckHasPermData(&rsslMsg.statusMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&PERM_DATA, &rsslMsg.statusMsg.permData));
	}
	else
		EXPECT_FALSE(rsslStatusMsgCheckHasPermData(&rsslMsg.statusMsg));

	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslStatusMsgCheckHasMsgKey(&rsslMsg.statusMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslStatusMsgCheckHasMsgKey(&rsslMsg.statusMsg));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			/* GroupId is not used in JSON2, so it shouldn't appear even when converted back to RWF. */
			EXPECT_FALSE(rsslStatusMsgCheckHasGroupId(&rsslMsg.statusMsg));
			break;

		case RSSL_JSON_JPT_JSON:
			if (params.groupId)
			{
				ASSERT_TRUE(rsslStatusMsgCheckHasGroupId(&rsslMsg.statusMsg));
				ASSERT_TRUE(rsslMsg.statusMsg.groupId.data != NULL);
				EXPECT_TRUE(rsslBufferIsEqual(&GROUP_ID, &rsslMsg.statusMsg.groupId));
			}
			break;

		default:
			FAIL() << "Unknown protocol type" << params.protocolType;
			break;
	}

	/* Check State. */
	if (params.state)
	{
		ASSERT_TRUE(rsslStatusMsgCheckHasState(&rsslMsg.statusMsg));
		EXPECT_EQ(RSSL_STREAM_CLOSED, rsslMsg.statusMsg.state.streamState);
		EXPECT_EQ(RSSL_DATA_SUSPECT, rsslMsg.statusMsg.state.dataState);
		EXPECT_EQ(RSSL_SC_NOT_FOUND, rsslMsg.statusMsg.state.code);
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&STATE_TEXT, &rsslMsg.statusMsg.state.text));
	}
	else
		EXPECT_FALSE(rsslStatusMsgCheckHasState(&rsslMsg.statusMsg));

	/* Check ClearCache flag. */
	EXPECT_EQ(params.clearCache, rsslStatusMsgCheckClearCache(&rsslMsg.statusMsg));

	/* Check PrivateStream flag. */
	EXPECT_EQ(params.privateStream, rsslStatusMsgCheckPrivateStream(&rsslMsg.statusMsg));

	/* Check PostUserInfo. */
	if (params.postUserInfo)
	{
		ASSERT_TRUE(rsslStatusMsgCheckHasPostUserInfo(&rsslMsg.statusMsg));
		EXPECT_EQ(IP_ADDRESS_UINT, rsslMsg.statusMsg.postUserInfo.postUserAddr);
		EXPECT_EQ(USER_ID, rsslMsg.statusMsg.postUserInfo.postUserId);
	}
	else
		EXPECT_FALSE(rsslStatusMsgCheckHasPostUserInfo(&rsslMsg.statusMsg));

	/* Check ReqKey. */
	if (params.reqKey)
	{
		ASSERT_TRUE(rsslMsg.statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY);

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.statusMsg.reqMsgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&REQ_MSG_KEY_NAME, &rsslMsg.statusMsg.reqMsgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.statusMsg.reqMsgKey));
		EXPECT_EQ(REQ_MSGKEY_SVC_ID, rsslMsg.statusMsg.reqMsgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslMsg.statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY);

	/* Check Qualified flag. */
	EXPECT_EQ(params.qualifiedStream, rsslStatusMsgCheckQualifiedStream(&rsslMsg.statusMsg));
}


INSTANTIATE_TEST_SUITE_P(StatusMsgTests, StatusMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, PermData, MsgKey, GroupId, State, ClearCache, PrivateStream, PostUserInfo, ReqKey, Qualified */

	/* Defaults */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, false, false, false),

	/* ExtendedHeader */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, true, false, false, false, false, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false, false, false, false, false, false, false),

	/* PermData */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, false, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, false, false, false, false, false, false),

	/* MsgKey */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, false, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, false, false, false, false, false, false),

	/* GroupId (Group ID isn't used in JSON so it shouldn't appear) */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, false, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, false, false, false, false, false, false),

	/* State */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, false, false, false),

	/* ClearCache */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, true, false, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, true, false, false, false, false),

	/* PrivateStream */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, true, false, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, true, false, false, false),

	/* PostUserInfo */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, true, false, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, true, false, false),

	/* ReqKey */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, false, true, false),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, false, true, false),

	/* Qualified */
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, false, false, false, false, true),
	StatusMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, false, false, false, false, true)

));

/* Parameters for StatusMsg State Code tests. */
class StatusMsgStateCodeTestParams
{
	public:

	/* State code for testing. */
	RsslUInt8 stateCode;
	const RsslBuffer *stateCodeString;

	StatusMsgStateCodeTestParams(RsslUInt8 stateCode, const RsslBuffer *stateCodeString)
	{
		this->stateCode = stateCode;
		this->stateCodeString = stateCodeString;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const StatusMsgStateCodeTestParams& params)
	{
		out << "["
			"stateCode:" << (params.stateCode)
			<< "]";
		return out;
	}
};

class StatusMsgStateCodeParamFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<StatusMsgStateCodeTestParams>
{
};

/* Test that converts a StatusMsg from RWF to JSON, and back to RWF. */
TEST_P(StatusMsgStateCodeParamFixture, StatusMsgStateCodeParamTest)
{
	StatusMsgStateCodeTestParams const &params = GetParam();
	RsslStatusMsg statusMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple StatusMsg. */
	rsslClearStatusMsg(&statusMsg);
	statusMsg.msgBase.streamId = 5;
	statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	statusMsg.msgBase.domainType = RSSL_DMT_MARKET_BY_ORDER;

	rsslStatusMsgApplyHasState(&statusMsg);
	statusMsg.state.streamState = RSSL_STREAM_OPEN;
	statusMsg.state.dataState = RSSL_DATA_OK;
	statusMsg.state.code = params.stateCode;
	statusMsg.state.text = STATE_TEXT;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&statusMsg));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message type. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Status", _jsonDocument["Type"].GetString());

	/* Check Stream ID. */
	ASSERT_TRUE(_jsonDocument.HasMember("ID"));
	ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
	EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

	/* Check Domain. */
	ASSERT_TRUE(_jsonDocument.HasMember("Domain"));
	ASSERT_TRUE(_jsonDocument["Domain"].IsString());
	EXPECT_STREQ("MarketByOrder", _jsonDocument["Domain"].GetString());

	/* Ignore GroupId. It shouldn't be present and there's no keyword defined for it. It will
	 * be verified to not be present after conversion back to RWF. */

	/* Check State. */
	ASSERT_TRUE(_jsonDocument.HasMember("State"));
	ASSERT_TRUE(_jsonDocument["State"].IsObject());
	if (params.stateCode > RSSL_SC_NONE)
	{
		ASSERT_TRUE(_jsonDocument["State"].HasMember("Code"));
		ASSERT_TRUE(_jsonDocument["State"]["Code"].IsString());
		EXPECT_STREQ(params.stateCodeString->data, _jsonDocument["State"]["Code"].GetString());
	}
	else
	{
		EXPECT_FALSE(_jsonDocument["State"].HasMember("Code"));
	}

	/* Convert back to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslStatusMsg is correct. */
	EXPECT_EQ(RSSL_MC_STATUS, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_BY_ORDER, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_NO_DATA, rsslMsg.msgBase.containerType);

	/* Check GroupId. GroupId is not used in JSON, so it shouldn't appear even when converted back to RWF. */
	EXPECT_FALSE(rsslStatusMsgCheckHasGroupId(&rsslMsg.statusMsg));

	EXPECT_EQ(params.stateCode, rsslMsg.statusMsg.state.code);
}

INSTANTIATE_TEST_SUITE_P(StatusMsgStateCodeFlagTests, StatusMsgStateCodeParamFixture, ::testing::Values(
	StatusMsgStateCodeTestParams(RSSL_SC_NONE, &RSSL_OMMSTR_SC_NONE),
	StatusMsgStateCodeTestParams(RSSL_SC_NOT_FOUND, &RSSL_OMMSTR_SC_NOT_FOUND),
	StatusMsgStateCodeTestParams(RSSL_SC_TIMEOUT, &RSSL_OMMSTR_SC_TIMEOUT),
	StatusMsgStateCodeTestParams(RSSL_SC_NOT_ENTITLED, &RSSL_OMMSTR_SC_NOT_ENTITLED),
	StatusMsgStateCodeTestParams(RSSL_SC_INVALID_ARGUMENT, &RSSL_OMMSTR_SC_INVALID_ARGUMENT),
	StatusMsgStateCodeTestParams(RSSL_SC_USAGE_ERROR, &RSSL_OMMSTR_SC_USAGE_ERROR),
	StatusMsgStateCodeTestParams(RSSL_SC_PREEMPTED, &RSSL_OMMSTR_SC_PREEMPTED),
	StatusMsgStateCodeTestParams(RSSL_SC_JIT_CONFLATION_STARTED, &RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED),
	StatusMsgStateCodeTestParams(RSSL_SC_REALTIME_RESUMED, &RSSL_OMMSTR_SC_REALTIME_RESUMED),
	StatusMsgStateCodeTestParams(RSSL_SC_FAILOVER_STARTED, &RSSL_OMMSTR_SC_FAILOVER_STARTED),
	StatusMsgStateCodeTestParams(RSSL_SC_FAILOVER_COMPLETED, &RSSL_OMMSTR_SC_FAILOVER_COMPLETED),
	StatusMsgStateCodeTestParams(RSSL_SC_GAP_DETECTED, &RSSL_OMMSTR_SC_GAP_DETECTED),
	StatusMsgStateCodeTestParams(RSSL_SC_NO_RESOURCES, &RSSL_OMMSTR_SC_NO_RESOURCES),
	StatusMsgStateCodeTestParams(RSSL_SC_TOO_MANY_ITEMS, &RSSL_OMMSTR_SC_TOO_MANY_ITEMS),
	StatusMsgStateCodeTestParams(RSSL_SC_ALREADY_OPEN, &RSSL_OMMSTR_SC_ALREADY_OPEN),
	StatusMsgStateCodeTestParams(RSSL_SC_SOURCE_UNKNOWN, &RSSL_OMMSTR_SC_SOURCE_UNKNOWN),
	StatusMsgStateCodeTestParams(RSSL_SC_NOT_OPEN, &RSSL_OMMSTR_SC_NOT_OPEN),
	StatusMsgStateCodeTestParams(RSSL_SC_NON_UPDATING_ITEM, &RSSL_OMMSTR_SC_NON_UPDATING_ITEM),
	StatusMsgStateCodeTestParams(RSSL_SC_UNSUPPORTED_VIEW_TYPE, &RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE),
	StatusMsgStateCodeTestParams(RSSL_SC_INVALID_VIEW, &RSSL_OMMSTR_SC_INVALID_VIEW),
	StatusMsgStateCodeTestParams(RSSL_SC_FULL_VIEW_PROVIDED, &RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED),
	StatusMsgStateCodeTestParams(RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH, &RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH),
	StatusMsgStateCodeTestParams(RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ, &RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ),
	StatusMsgStateCodeTestParams(RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER, &RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER),
	StatusMsgStateCodeTestParams(RSSL_SC_ERROR, &RSSL_OMMSTR_SC_ERROR),
	StatusMsgStateCodeTestParams(RSSL_SC_DACS_DOWN, &RSSL_OMMSTR_SC_DACS_DOWN),
	StatusMsgStateCodeTestParams(RSSL_SC_USER_UNKNOWN_TO_PERM_SYS, &RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS),
	StatusMsgStateCodeTestParams(RSSL_SC_DACS_MAX_LOGINS_REACHED, &RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED),
	StatusMsgStateCodeTestParams(RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED, &RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED),
	StatusMsgStateCodeTestParams(RSSL_SC_GAP_FILL, &RSSL_OMMSTR_SC_GAP_FILL),
	StatusMsgStateCodeTestParams(RSSL_SC_APP_AUTHORIZATION_FAILED, &RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED)
));
