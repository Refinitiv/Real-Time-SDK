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

/* Parameters for RefreshMsg tests. */
class RefreshMsgMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool permData;
	bool key;
	bool seqNum;
	bool solicited;
	bool complete;
	bool qos;
	bool clearCache;
	bool doNotCache;
	bool privateStream;
	bool postUserInfo;
	bool reqKey;
	bool qualifiedStream;
    bool partNum;

	RefreshMsgMembersTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool permData, bool key, bool seqNum, bool solicited, bool complete, bool qos, 
			bool clearCache, bool doNotCache, bool privateStream, bool postUserInfo, bool reqKey, bool qualifiedStream, bool partNum)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->permData = permData;
		this->key = key;
		this->seqNum = seqNum;
		this->solicited = solicited;
		this->complete = complete;
		this->qos = qos;
		this->clearCache = clearCache;
		this->doNotCache = doNotCache;
		this->privateStream = privateStream;
		this->postUserInfo = postUserInfo;
		this->reqKey = reqKey;
		this->qualifiedStream = qualifiedStream;
        this->partNum = partNum;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const RefreshMsgMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"permData:" << (params.permData ? "true" : "false") << ","
			"key:" << (params.key ? "true" : "false") << ","
			"seqNum:" << (params.seqNum ? "true" : "false") << ","
			"solicited:" << (params.solicited ? "true" : "false") << ","
			"qos:" << (params.qos ? "true" : "false") << ","
			"clearCache:" << (params.clearCache ? "true" : "false") << ","
			"doNotCache:" << (params.doNotCache ? "true" : "false") << ","
			"privateStream:" << (params.privateStream ? "true" : "false") << ","
			"postUserInfo:" << (params.postUserInfo ? "true" : "false") << ","
			"reqKey:" << (params.reqKey ? "true" : "false") << ","
			"qualifiedStream:" << (params.qualifiedStream ? "true" : "false") << ","
			"partNum:" << (params.partNum ? "true" : "false") 
			<< "]";
		return out;
	}
};

class RefreshMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RefreshMsgMembersTestParams>
{
};

/* Test that converts a RefreshMsg with a FieldList from RWF to JSON, and back to RWF. */
TEST_P(RefreshMsgMembersTestFixture, RefreshMsgMembersTest)
{
	RefreshMsgMembersTestParams const &params = GetParam();
	RsslRefreshMsg refreshMsg;
	RsslMsg rsslMsg;

	/* Create and encode a simple RefreshMsg. */
	rsslClearRefreshMsg(&refreshMsg);

	refreshMsg.msgBase.streamId = 5;
	refreshMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	refreshMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.dataState = RSSL_DATA_OK;
	refreshMsg.state.text = STATE_TEXT;

	/* Set a non-default Group ID. Group ID is not used in JSON so this should not appear even after converting back to RWF. */
	refreshMsg.groupId = GROUP_ID;

	if (params.extendedHeader)
	{
		rsslRefreshMsgApplyHasExtendedHdr(&refreshMsg);
		refreshMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.permData)
	{
		rsslRefreshMsgApplyHasPermData(&refreshMsg);
		refreshMsg.permData = PERM_DATA;
	}

	if (params.key)
	{
		rsslRefreshMsgApplyHasMsgKey(&refreshMsg);
		rsslMsgKeyApplyHasName(&refreshMsg.msgBase.msgKey);
		refreshMsg.msgBase.msgKey.name = MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&refreshMsg.msgBase.msgKey);
		refreshMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;
	}

	if (params.seqNum)
	{
		rsslRefreshMsgApplyHasSeqNum(&refreshMsg);
		refreshMsg.seqNum = SEQ_NUM;
	}

	if (params.solicited)
		rsslRefreshMsgApplySolicited(&refreshMsg);

	if (params.complete)
		rsslRefreshMsgApplyRefreshComplete(&refreshMsg);

	if (params.qos)
	{
		rsslRefreshMsgApplyHasQos(&refreshMsg);
		refreshMsg.qos.timeliness = RSSL_QOS_TIME_REALTIME;
		refreshMsg.qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
	}

	if (params.clearCache)
		rsslRefreshMsgApplyClearCache(&refreshMsg);

	if (params.doNotCache)
		rsslRefreshMsgApplyDoNotCache(&refreshMsg);

	if (params.privateStream)
		rsslRefreshMsgApplyPrivateStream(&refreshMsg);

	if (params.postUserInfo)
	{
		rsslRefreshMsgApplyHasPostUserInfo(&refreshMsg);
		refreshMsg.postUserInfo.postUserAddr = IP_ADDRESS_UINT;
		refreshMsg.postUserInfo.postUserId = USER_ID;
	}

	if (params.reqKey)
	{
		refreshMsg.flags |= RSSL_RFMF_HAS_REQ_MSG_KEY;
		rsslMsgKeyApplyHasName(&refreshMsg.reqMsgKey);
		refreshMsg.reqMsgKey.name = REQ_MSG_KEY_NAME;
		rsslMsgKeyApplyHasServiceId(&refreshMsg.reqMsgKey);
		refreshMsg.reqMsgKey.serviceId = REQ_MSGKEY_SVC_ID;
	}

	if (params.qualifiedStream)
		rsslRefreshMsgApplyQualifiedStream(&refreshMsg);

    if(params.partNum)
    {
        refreshMsg.flags |= RSSL_RFMF_HAS_PART_NUM;
        refreshMsg.partNum = PART_NUM;
    }

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&refreshMsg, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType, params.solicited));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Refresh", _jsonDocument["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(_jsonDocument.HasMember("ID"));
			ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
			EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

			/* Check Domain. */
			ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */

			/* Check State. */
			ASSERT_TRUE(_jsonDocument.HasMember("State"));
			ASSERT_TRUE(_jsonDocument["State"].IsObject());
			ASSERT_TRUE(_jsonDocument["State"].HasMember("Stream"));
			ASSERT_TRUE(_jsonDocument["State"]["Stream"].IsString());
			EXPECT_STREQ("Open", _jsonDocument["State"]["Stream"].GetString());
			ASSERT_TRUE(_jsonDocument["State"].HasMember("Data"));
			ASSERT_TRUE(_jsonDocument["State"]["Data"].IsString());
			EXPECT_STREQ("Ok", _jsonDocument["State"]["Data"].GetString());
			ASSERT_FALSE(_jsonDocument["State"].HasMember("Code"));
			ASSERT_TRUE(_jsonDocument["State"].HasMember("Text"));
			ASSERT_TRUE(_jsonDocument["State"]["Text"].IsString());
			EXPECT_STREQ(STATE_TEXT.data, _jsonDocument["State"]["Text"].GetString());

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

			/* Check Solicited flag */
			if (params.solicited)
				EXPECT_FALSE(_jsonDocument.HasMember("Solicited"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Solicited"));
				ASSERT_TRUE(_jsonDocument["Solicited"].IsBool());
				EXPECT_FALSE(_jsonDocument["Solicited"].GetBool());
			}

			/* Check Complete flag. */
			if (params.complete)
				EXPECT_FALSE(_jsonDocument.HasMember("Complete"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Complete"));
				ASSERT_TRUE(_jsonDocument["Complete"].IsBool());
				EXPECT_FALSE(_jsonDocument["Complete"].GetBool());
			}

			/* Check QoS */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Qos"));
				ASSERT_TRUE(_jsonDocument["Qos"].IsObject());
				ASSERT_TRUE(_jsonDocument["Qos"].HasMember("Timeliness"));
				ASSERT_TRUE(_jsonDocument["Qos"]["Timeliness"].IsString());
				EXPECT_STREQ("Realtime", _jsonDocument["Qos"]["Timeliness"].GetString());
				ASSERT_TRUE(_jsonDocument["Qos"].IsObject());
				ASSERT_TRUE(_jsonDocument["Qos"].HasMember("Rate"));
				ASSERT_TRUE(_jsonDocument["Qos"]["Rate"].IsString());
				EXPECT_STREQ("TickByTick", _jsonDocument["Qos"]["Rate"].GetString());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("Qos"));

			/* Check ClearCache flag. */
			if (params.clearCache)
				EXPECT_FALSE(_jsonDocument.HasMember("ClearCache"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ClearCache"));
				ASSERT_TRUE(_jsonDocument["ClearCache"].IsBool());
				EXPECT_FALSE(_jsonDocument["ClearCache"].GetBool());
			}

			/* Check DoNotCache flag. */
			if (params.doNotCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("DoNotCache"));
				ASSERT_TRUE(_jsonDocument["DoNotCache"].IsBool());
				EXPECT_TRUE(_jsonDocument["DoNotCache"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("DoNotCache"));

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

			/* Check Part Num */
			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("PartNumber"));
				ASSERT_TRUE(_jsonDocument["PartNumber"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["PartNumber"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("PartNumber"));


			/* Check Field List. */
			ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Fields"], params.protocolType));
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
			EXPECT_EQ(RSSL_MC_REFRESH, msgBase["c"].GetInt());

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

			/* Check State. */
			ASSERT_TRUE(_jsonDocument.HasMember("s"));
			ASSERT_TRUE(_jsonDocument["s"].IsObject());
			ASSERT_TRUE(_jsonDocument["s"].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["s"]["s"].IsNumber());
			EXPECT_EQ(RSSL_STREAM_OPEN, _jsonDocument["s"]["s"].GetInt());
			ASSERT_TRUE(_jsonDocument["s"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["s"]["d"].IsNumber());
			EXPECT_EQ(RSSL_DATA_OK, _jsonDocument["s"]["d"].GetInt());
			ASSERT_FALSE(_jsonDocument["s"].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["s"].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["s"]["t"].IsString());
			EXPECT_STREQ(STATE_TEXT.data, _jsonDocument["s"]["t"].GetString());

			/* Check Group. */
			ASSERT_TRUE(_jsonDocument.HasMember("g"));
			ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&GROUP_ID, _jsonDocument["g"]));

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

			/* Check Solicited flag (JSON1 always sends this flag). */
			ASSERT_TRUE(_jsonDocument.HasMember("a"));
			ASSERT_TRUE(_jsonDocument["a"].IsNumber());

			if (params.solicited)
				EXPECT_EQ(1, _jsonDocument["a"].GetInt());
			else
				EXPECT_EQ(0, _jsonDocument["a"].GetInt());

			/* Check Complete flag. */
			if (params.complete)
				EXPECT_FALSE(_jsonDocument.HasMember("c"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["c"].GetInt());
			}

			/* Check QoS */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("q"));
				ASSERT_TRUE(_jsonDocument["q"].IsObject());
				ASSERT_TRUE(_jsonDocument["q"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["q"]["t"].IsNumber());
				EXPECT_EQ(RSSL_QOS_TIME_REALTIME, _jsonDocument["q"]["t"].GetInt());
				ASSERT_TRUE(_jsonDocument["q"].HasMember("r"));
				ASSERT_TRUE(_jsonDocument["q"]["r"].IsNumber());
				EXPECT_EQ(RSSL_QOS_RATE_TICK_BY_TICK, _jsonDocument["q"]["r"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("q"));

			/* Check ClearCache flag. */
			if (params.clearCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("t"));
				ASSERT_TRUE(_jsonDocument["t"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["t"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("t"));

			/* Check DoNotCache flag. */
			if (params.doNotCache)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("x"));
				ASSERT_TRUE(_jsonDocument["x"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["x"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("x"));

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

			/* Check Part Num */
			if (params.partNum)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("m"));
				ASSERT_TRUE(_jsonDocument["m"].IsNumber());
				EXPECT_EQ(PART_NUM, _jsonDocument["m"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("m"));


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
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslRefreshMsg is correct. */
	EXPECT_EQ(RSSL_MC_REFRESH, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			/* Check GroupId (should be the default group -- JSON2 does not convert group) */
			ASSERT_TRUE(rsslMsg.refreshMsg.groupId.data != NULL);
			EXPECT_EQ(2, rsslMsg.refreshMsg.groupId.length);
			EXPECT_EQ(0, rsslMsg.refreshMsg.groupId.data[0]);
			EXPECT_EQ(0, rsslMsg.refreshMsg.groupId.data[1]);
			break;

		case RSSL_JSON_JPT_JSON:
			ASSERT_TRUE(rsslMsg.refreshMsg.groupId.data != NULL);
			EXPECT_TRUE(rsslBufferIsEqual(&GROUP_ID, &rsslMsg.refreshMsg.groupId));
			break;

		default:
			FAIL() << "Unknown protocol type" << params.protocolType;
			break;
	}


	/* Check State. */
	EXPECT_EQ(RSSL_STREAM_OPEN, rsslMsg.refreshMsg.state.streamState);
	EXPECT_EQ(RSSL_DATA_OK, rsslMsg.refreshMsg.state.dataState);
	EXPECT_EQ(RSSL_SC_NONE, rsslMsg.refreshMsg.state.code);
	EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&STATE_TEXT, &rsslMsg.refreshMsg.state.text));

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasExtendedHdr(&rsslMsg.refreshMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.refreshMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasExtendedHdr(&rsslMsg.refreshMsg));

	/* Check PermData. */
	if (params.permData)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasPermData(&rsslMsg.refreshMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&PERM_DATA, &rsslMsg.refreshMsg.permData));
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasPermData(&rsslMsg.refreshMsg));

	/* Check MsgKey. */
	if (params.key)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasMsgKey(&rsslMsg.refreshMsg));

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&MSG_KEY_NAME, &rsslMsg.msgBase.msgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.msgBase.msgKey));
		EXPECT_EQ(MSGKEY_SVC_ID, rsslMsg.msgBase.msgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasMsgKey(&rsslMsg.refreshMsg));

	/* Check SeqNum. */
	if (params.seqNum)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasSeqNum(&rsslMsg.refreshMsg));
		EXPECT_EQ(SEQ_NUM, rsslMsg.refreshMsg.seqNum);
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasSeqNum(&rsslMsg.refreshMsg));

	/* Check Solicited */
	EXPECT_EQ(params.solicited, rsslRefreshMsgCheckSolicited(&rsslMsg.refreshMsg));

	/* Check Complete */
	EXPECT_EQ(params.complete, rsslRefreshMsgCheckRefreshComplete(&rsslMsg.refreshMsg));

	/* Check Qos. */
	if (params.qos)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasQos(&rsslMsg.refreshMsg));
		EXPECT_EQ(RSSL_QOS_TIME_REALTIME, rsslMsg.refreshMsg.qos.timeliness);
		EXPECT_EQ(RSSL_QOS_RATE_TICK_BY_TICK, rsslMsg.refreshMsg.qos.rate);
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasQos(&rsslMsg.refreshMsg));

	/* Check ClearCache flag. */
	EXPECT_EQ(params.clearCache, rsslRefreshMsgCheckClearCache(&rsslMsg.refreshMsg));

	/* Check DoNotCache flag. */
	EXPECT_EQ(params.doNotCache, rsslRefreshMsgCheckDoNotCache(&rsslMsg.refreshMsg));

	/* Check PrivateStream flag. */
	EXPECT_EQ(params.privateStream, rsslRefreshMsgCheckPrivateStream(&rsslMsg.refreshMsg));

	/* Check PostUserInfo. */
	if (params.postUserInfo)
	{
		ASSERT_TRUE(rsslRefreshMsgCheckHasPostUserInfo(&rsslMsg.refreshMsg));
		EXPECT_EQ(IP_ADDRESS_UINT, rsslMsg.refreshMsg.postUserInfo.postUserAddr);
		EXPECT_EQ(USER_ID, rsslMsg.refreshMsg.postUserInfo.postUserId);
	}
	else
		EXPECT_FALSE(rsslRefreshMsgCheckHasPostUserInfo(&rsslMsg.refreshMsg));

	/* Check ReqKey. */
	if (params.reqKey)
	{
		ASSERT_TRUE(rsslMsg.refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY);

		ASSERT_TRUE(rsslMsgKeyCheckHasName(&rsslMsg.refreshMsg.reqMsgKey));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&REQ_MSG_KEY_NAME, &rsslMsg.refreshMsg.reqMsgKey.name));

		ASSERT_TRUE(rsslMsgKeyCheckHasServiceId(&rsslMsg.refreshMsg.reqMsgKey));
		EXPECT_EQ(REQ_MSGKEY_SVC_ID, rsslMsg.refreshMsg.reqMsgKey.serviceId);
	}
	else
		EXPECT_FALSE(rsslMsg.refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY);

	/* Check Qualified flag. */
	EXPECT_EQ(params.qualifiedStream, rsslRefreshMsgCheckQualifiedStream(&rsslMsg.refreshMsg));

    /* Check Part Num */
    if(params.partNum)
    {
        ASSERT_TRUE(rsslMsg.refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM);
        EXPECT_EQ(PART_NUM, rsslMsg.refreshMsg.partNum);
    }
    else
        EXPECT_FALSE(rsslMsg.refreshMsg.flags & RSSL_RFMF_HAS_PART_NUM);

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
}



INSTANTIATE_TEST_SUITE_P(RefreshMsgTests, RefreshMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, PermData, MsgKey, SeqNum, Solicited, Complete, Qos, ClearCache, DoNotCache, PrivateStream, PostUserInfo, ReqKey, Qualified */

	/* Defaults */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, false, false, false, false, false),

	/* ExtendedHeader */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, true, false, false, false, true, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false, true, true, false, true, false, false, false, false, false, false),

	/* PermData */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false, false, true, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false, true, true, false, true, false, false, false, false, false, false),

	/* MsgKey */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, true, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, true, true, false, true, false, false, false, false, false, false),

	/* SeqNum */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, true, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, true, true, false, true, false, false, false, false, false, false),

	/* Solicited (false) */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, false, true, false, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, false, true, false, true, false, false, false, false, false, false),

	/* Complete (false) */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, false, false, true, false, false, false, false, false, true),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, false, false, true, false, false, false, false, false, true),

	/* Qos */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, true, true, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, true, true, false, false, false, false, false, false),

	/* ClearCache (false)*/
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, false, false, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, false, false, false, false, false, false, false),

	/* DoNotCache */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, true, false, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, true, false, false, false, false, false),

	/* PrivateStream */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, true, false, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, true, false, false, false, false),

	/* PostUserInfo */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, false, true, false, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, false, true, false, false, false),

	/* ReqKey */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, false, false, true, false, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, false, false, true, false, false),

	/* Qualified */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, false, false, false, true, false),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, false, false, false, true, false),

    /* Part Number */
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false, true, true, false, true, false, false, false, false, false, true),
	RefreshMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false, true, true, false, true, false, false, false, false, false, true)

));

