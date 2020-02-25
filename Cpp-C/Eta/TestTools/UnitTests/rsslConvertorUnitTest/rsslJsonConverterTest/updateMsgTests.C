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

/* Parameters for UpdateMsg tests. */
class UpdateMsgMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool permData;
	bool key;
	bool seqNum;
	bool conflationInfo;
	bool doNotCache;
	bool doNotConflate;
	bool doNotRipple;
	bool postUserInfo;
	bool discardable;
	RsslDataTypes containerType; /* Should be RSSL_DT_FIELD_LIST or RSSL_DT_NO_DATA */

	UpdateMsgMembersTestParams(RsslJsonProtocolType protocolType, RsslDataTypes containerType, bool extendedHeader, bool permData, bool key, bool seqNum, bool conflationInfo, bool doNotCache,
			bool doNotConflate, bool doNotRipple, bool postUserInfo, bool discardable)
	{
		this->protocolType = protocolType;
		this->containerType = containerType;
		this->extendedHeader = extendedHeader;
		this->permData = permData;
		this->key = key;
		this->seqNum = seqNum;
		this->conflationInfo = conflationInfo;
		this->doNotCache = doNotCache;
		this->doNotConflate = doNotConflate;
		this->doNotRipple = doNotRipple;
		this->postUserInfo = postUserInfo;
		this->discardable = discardable;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const UpdateMsgMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"containerType: " << rsslDataTypeToOmmString(params.containerType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"permData:" << (params.permData ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"seqNum:" << (params.seqNum ? "true" : "false") << ","
			"conflationInfo:" << (params.conflationInfo ? "true" : "false") << ","
			"doNotCache:" << (params.doNotCache ? "true" : "false") << ","
			"doNotConflate:" << (params.doNotConflate ? "true" : "false") << ","
			"doNotRipple:" << (params.doNotRipple ? "true" : "false") << ","
			"postUserInfo:" << (params.postUserInfo ? "true" : "false") << ","
			"discardable:" << (params.discardable ? "true" : "false")
			<< "]";
		return out;
	}
};

class UpdateMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<UpdateMsgMembersTestParams>
{
};

/* Test that converts a UpdateMsg with a FieldList from RWF to JSON, and back to RWF. */
TEST_P(UpdateMsgMembersTestFixture, UpdateMsgMembersTest)
{
	UpdateMsgMembersTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;

	RsslUInt16 CONFLATION_COUNT = 15;
	RsslUInt16 CONFLATION_TIME = 32768;

	/* Create and encode a simple UpdateMsg. */
	rsslClearUpdateMsg(&updateMsg);

	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.containerType = params.containerType;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	if (params.extendedHeader)
	{
		rsslUpdateMsgApplyHasExtendedHdr(&updateMsg);
		updateMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.permData)
	{
		rsslUpdateMsgApplyHasPermData(&updateMsg);
		updateMsg.permData = PERM_DATA;
	}

	if (params.key)
	{
		rsslUpdateMsgApplyHasMsgKey(&updateMsg);
		rsslMsgKeyApplyHasName(&updateMsg.msgBase.msgKey);
		updateMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&updateMsg.msgBase.msgKey);
		updateMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.seqNum)
	{
		rsslUpdateMsgApplyHasSeqNum(&updateMsg);
		updateMsg.seqNum = SEQ_NUM;
	}

	if (params.conflationInfo)
	{
		rsslUpdateMsgApplyHasConfInfo(&updateMsg);
		updateMsg.conflationCount = CONFLATION_COUNT;
		updateMsg.conflationTime = CONFLATION_TIME;
	}

	if (params.doNotCache)
		rsslUpdateMsgApplyDoNotCache(&updateMsg);

	if (params.doNotConflate)
		rsslUpdateMsgApplyDoNotConflate(&updateMsg);

	if (params.doNotRipple)
		rsslUpdateMsgApplyDoNotRipple(&updateMsg);

	if (params.postUserInfo)
	{
		rsslUpdateMsgApplyHasPostUserInfo(&updateMsg);
		updateMsg.postUserInfo.postUserAddr = IP_ADDRESS_UINT;
		updateMsg.postUserInfo.postUserId = USER_ID;
	}

	if (params.discardable)
		rsslUpdateMsgApplyDiscardable(&updateMsg);

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	switch(params.containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
			break;
		case RSSL_DT_NO_DATA:
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&updateMsg));
			break;
		default:
			FAIL() << "Unhandled container type" << params.containerType;
			break;
	}


	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{

			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

			/* Check Domain. */
			ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("UpdateType"));
			ASSERT_TRUE(_jsonDocument["UpdateType"].IsString());
			EXPECT_STREQ("Quote", _jsonDocument["UpdateType"].GetString());

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

			/* Check SeqNum */
			if (params.seqNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("SeqNumber"));
				ASSERT_TRUE(_jsonDocument["SeqNumber"].IsNumber());
				EXPECT_EQ(SEQ_NUM, _jsonDocument["SeqNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("SeqNumber"));

			/* Check Conflation Info */
			if (params.conflationInfo)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ConflationInfo"));
				ASSERT_TRUE(_jsonDocument["ConflationInfo"].IsObject());
				ASSERT_TRUE(_jsonDocument["ConflationInfo"].HasMember("Time"));
				ASSERT_TRUE(_jsonDocument["ConflationInfo"]["Time"].IsNumber());
				EXPECT_EQ(CONFLATION_TIME, _jsonDocument["ConflationInfo"]["Time"].GetInt());
				ASSERT_TRUE(_jsonDocument["ConflationInfo"].HasMember("Count"));
				ASSERT_TRUE(_jsonDocument["ConflationInfo"]["Count"].IsNumber());
				EXPECT_EQ(CONFLATION_COUNT, _jsonDocument["ConflationInfo"]["Count"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ConflationInfo"));

			/* Check DoNotCache flag. */
			if (params.doNotCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("DoNotCache"));
				ASSERT_TRUE(_jsonDocument["DoNotCache"].IsBool());
				EXPECT_TRUE(_jsonDocument["DoNotCache"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("DoNotCache"));

			/* Check DoNotRipple flag. */
			if (params.doNotRipple)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("DoNotRipple"));
				ASSERT_TRUE(_jsonDocument["DoNotRipple"].IsBool());
				EXPECT_TRUE(_jsonDocument["DoNotRipple"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("DoNotRipple"));

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

			/* Check Discardable flag. */
			if (params.discardable)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Discardable"));
				ASSERT_TRUE(_jsonDocument["Discardable"].IsBool());
				EXPECT_TRUE(_jsonDocument["Discardable"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Discardable"));

			/* Check Field List. */
			if (params.containerType == RSSL_DT_FIELD_LIST)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Fields"], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Fields"));
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
			EXPECT_EQ(RSSL_MC_UPDATE, msgBase["c"].GetInt());

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
			EXPECT_EQ(params.containerType - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

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

			/* Check Conflation Info */
			if (params.conflationInfo)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"].IsObject());
				ASSERT_TRUE(_jsonDocument["c"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["c"]["t"].IsNumber());
				EXPECT_EQ(CONFLATION_TIME, _jsonDocument["c"]["t"].GetInt());
				ASSERT_TRUE(_jsonDocument["c"].HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"]["c"].IsNumber());
				EXPECT_EQ(CONFLATION_COUNT, _jsonDocument["c"]["c"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("c"));

			/* Check DoNotCache flag. */
			if (params.doNotCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("x"));
				ASSERT_TRUE(_jsonDocument["x"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["x"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("x"));

			/* Check DoNotRipple flag. */
			if (params.doNotRipple)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("r"));
				ASSERT_TRUE(_jsonDocument["r"].IsInt());
				EXPECT_EQ(1, _jsonDocument["r"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("r"));

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

			/* Check Discardable flag. */
			if (params.discardable)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("o"));
				ASSERT_TRUE(_jsonDocument["o"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["o"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("o"));

			/* Check Field List. */
			if (params.containerType == RSSL_DT_FIELD_LIST)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("d"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("d"));
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

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(params.containerType, rsslMsg.msgBase.containerType);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslUpdateMsgCheckHasExtendedHdr(&rsslMsg.updateMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.updateMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslUpdateMsgCheckHasExtendedHdr(&rsslMsg.updateMsg));

	/* Check PermData. */
	if (params.permData)
	{
		ASSERT_TRUE(rsslUpdateMsgCheckHasPermData(&rsslMsg.updateMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&PERM_DATA, &rsslMsg.updateMsg.permData));
	}
	else
		EXPECT_FALSE(rsslUpdateMsgCheckHasPermData(&rsslMsg.updateMsg));

	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslUpdateMsgCheckHasMsgKey(&rsslMsg.updateMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslUpdateMsgCheckHasMsgKey(&rsslMsg.updateMsg));

	/* Check SeqNum. */
	if (params.seqNum)
	{
		ASSERT_TRUE(rsslUpdateMsgCheckHasSeqNum(&rsslMsg.updateMsg));
		EXPECT_EQ(SEQ_NUM, rsslMsg.updateMsg.seqNum);
	}

	/* Check ConflationInfo. */
	if (params.conflationInfo)
	{
		EXPECT_TRUE(rsslUpdateMsgCheckHasConfInfo(&rsslMsg.updateMsg));
		EXPECT_EQ(CONFLATION_TIME, rsslMsg.updateMsg.conflationTime);
		EXPECT_EQ(CONFLATION_COUNT, rsslMsg.updateMsg.conflationCount);
	}
	else
		EXPECT_FALSE(rsslUpdateMsgCheckHasConfInfo(&rsslMsg.updateMsg));

	/* Check DoNotCache flag. */
	EXPECT_EQ(params.doNotCache, rsslUpdateMsgCheckDoNotCache(&rsslMsg.updateMsg));

	/* Check DoNotConflate flag. */
	EXPECT_EQ(params.doNotConflate, rsslUpdateMsgCheckDoNotConflate(&rsslMsg.updateMsg));

	/* Check DoNotRipple flag. */
	EXPECT_EQ(params.doNotRipple, rsslUpdateMsgCheckDoNotRipple(&rsslMsg.updateMsg));

	/* Check PostUserInfo. */
	if (params.postUserInfo)
	{
		ASSERT_TRUE(rsslUpdateMsgCheckHasPostUserInfo(&rsslMsg.updateMsg));
		EXPECT_EQ(IP_ADDRESS_UINT, rsslMsg.updateMsg.postUserInfo.postUserAddr);
		EXPECT_EQ(USER_ID, rsslMsg.updateMsg.postUserInfo.postUserId);
	}
	else
		EXPECT_FALSE(rsslUpdateMsgCheckHasPostUserInfo(&rsslMsg.updateMsg));

	/* Check Discardable flag. */
	EXPECT_EQ(params.discardable, rsslUpdateMsgCheckHasDiscardable(&rsslMsg.updateMsg));

	/* Check FieldList. */
	if (params.containerType == RSSL_DT_FIELD_LIST)
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
}



INSTANTIATE_TEST_CASE_P(UpdateMsgTests, UpdateMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, PermData, MsgKey, SeqNum, ConflationInfo, DoNotCache, DoNotRipple, PostUserInfo, Discardable */

	/* Defaults */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, false, false),

	/* ExtendedHeader */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, true, false, false, false, false, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, true, false, false, false, false, false, false, false, false, false),

	/* PermData */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, true, false, false, false, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, true, false, false, false, false, false, false, false, false),

	/* MsgKey */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, true, false, false, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, true, false, false, false, false, false, false, false),

	/* SeqNum */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, true, false, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, true, false, false, false, false, false, false),

	/* ConflationInfo */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, true, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, true, false, false, false, false, false),

	/* ConflationInfo and no payload (tests parsing conflation info when it is the last member in the message) */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_NO_DATA, false, false, false, false, true, false, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_NO_DATA, false, false, false, false, true, false, false, false, false, false),

	/* DoNotCache */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, true, false, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, true, false, false, false, false),

	/* DoNotConflate */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, true, false, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, true, false, false, false),

	/* DoNotRipple */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, true, false, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, true, false, false),

	/* PostUserInfo */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, true, false),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, true, false),

	/* Discardable */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, false, true),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false, false, false, false, false, false, false, false, false, true),

	/* Everything */
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, true, true, true, true, true, true, true, true, true, true),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, true, true, true, true, true, true, true, true, true, true),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_NO_DATA, true, true, true, true, true, true, true, true, true, true),
	UpdateMsgMembersTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_NO_DATA, true, true, true, true, true, true, true, true, true, true)

));

