/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslJsonConverterTestBase.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json; 

/* Fixture for RequestMsgTests that has conversion code. */
class RequestMsgTests : public MsgConversionTestBase
{
};

/* Parameters for RequestMsg tests. */
class RequestMsgMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool extendedHeader;
	bool priority;
	bool streaming;
	bool keyInUpdates;
	bool confInfoInUpdates;
	bool noRefresh;
	bool qos;
	bool worstQos;
	bool privateStream;
	bool pause;
	bool batch;
	bool view;
	bool qualified;
    bool keyIdentifier;
	bool keyNameEscChar;

	RequestMsgMembersTestParams(RsslJsonProtocolType protocolType, bool extendedHeader, bool priority, bool streaming, bool keyInUpdates, bool confInfoInUpdates,
			bool noRefresh, bool qos, bool worstQos, bool privateStream, bool pause, bool batch, bool view, 
			bool qualified, bool keyIdentifier, bool keyNameEscChar = false)
	{
		this->protocolType = protocolType;
		this->extendedHeader = extendedHeader;
		this->priority = priority;
		this->streaming = streaming;
		this->keyInUpdates = keyInUpdates;
		this->confInfoInUpdates = confInfoInUpdates;
		this->noRefresh = noRefresh;
		this->qos = qos;
		this->worstQos = worstQos;
		this->privateStream = privateStream;
		this->pause = pause;
		this->batch = batch;
		this->view = view;
		this->qualified = qualified;
        this->keyIdentifier = keyIdentifier;
		this->keyNameEscChar = keyNameEscChar;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const RequestMsgMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"extendedHeader:" << (params.extendedHeader ? "true" : "false") << ","
			"priority:" << (params.priority ? "true" : "false") << ","
			"streaming:" << (params.streaming ? "true" : "false") << ","
			"keyInUpdates:" << (params.keyInUpdates ? "true" : "false") << ","
			"confInfoInUpdates:" << (params.confInfoInUpdates ? "true" : "false") << ","
			"noRefresh:" << (params.noRefresh ? "true" : "false") << ","
			"qos:" << (params.qos ? "true" : "false") << ","
			"worstQos:" << (params.worstQos ? "true" : "false") << ","
			"privateStream:" << (params.privateStream ? "true" : "false") << ","
			"pause:" << (params.pause ? "true" : "false") << ","
			"batch:" << (params.batch ? "true" : "false") << ","
			"view:" << (params.view ? "true" : "false") << ","
			"qualified:" << (params.qualified ? "true" : "false") << "," 
			"keyIdentifier:" << (params.keyIdentifier ? "true" : "false") << ","
			"keyNameEscChar:" << (params.keyNameEscChar ? "true" : "false")
			<< "]";
		return out;
	}
};

class RequestMsgMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RequestMsgMembersTestParams>
{
};

/* Test that converts a RequestMsg from RWF to JSON, and back to RWF. */
TEST_P(RequestMsgMembersTestFixture, RequestMsgMembersTest)
{
	RequestMsgMembersTestParams const &params = GetParam();
	RsslRequestMsg requestMsg;
	RsslMsg rsslMsg;
	RsslQos qos, worstQos;

	RsslUInt8 PRIORITY_CLASS = 2;
	RsslUInt16 PRIORITY_COUNT = 65535;

	RsslBuffer BATCH_ITEM_NAMES[3] = {{ 3, (char*)"SIX"}, {6, (char*)"TWELVE"}, {8, (char*)"FOURTEEN"}};

	RsslInt VIEW_FIDS[2] = {22, 25};

	/* Create and encode a simple RequestMsg. */
	rsslClearRequestMsg(&requestMsg);

	requestMsg.msgBase.streamId = 5;
	requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;

	rsslClearQos(&qos);
	qos.timeliness = RSSL_QOS_TIME_REALTIME;
	qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&worstQos);
	worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
	worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;

	if (params.extendedHeader)
	{
		rsslRequestMsgApplyHasExtendedHdr(&requestMsg);
		requestMsg.extendedHeader = EXTENDED_HEADER;
	}

	if (params.priority)
	{
		rsslRequestMsgApplyHasPriority(&requestMsg);
		requestMsg.priorityClass = PRIORITY_CLASS;
		requestMsg.priorityCount = PRIORITY_COUNT;
	}

	if (params.streaming)
		rsslRequestMsgApplyStreaming(&requestMsg);

	if (params.keyInUpdates)
		rsslRequestMsgApplyMsgKeyInUpdates(&requestMsg);

	if (params.confInfoInUpdates)
		rsslRequestMsgApplyConfInfoInUpdates(&requestMsg);

	if (params.noRefresh)
		rsslRequestMsgApplyNoRefresh(&requestMsg);

	if (params.qos)
	{
		rsslRequestMsgApplyHasQos(&requestMsg);
		requestMsg.qos = qos;
	}

	if (params.worstQos)
	{
		rsslRequestMsgApplyHasWorstQos(&requestMsg);
		requestMsg.worstQos = worstQos;
	}

	if (params.privateStream)
		rsslRequestMsgApplyPrivateStream(&requestMsg);

	if (params.pause)
		rsslRequestMsgApplyPause(&requestMsg);

	rsslMsgKeyApplyHasServiceId(&requestMsg.msgBase.msgKey);
	requestMsg.msgBase.msgKey.serviceId = MSGKEY_SVC_ID;

    if (params.keyIdentifier)
    {
        rsslMsgKeyApplyHasIdentifier(&requestMsg.msgBase.msgKey);
        requestMsg.msgBase.msgKey.identifier = MSGKEY_IDENTIFIER;
    }

	if (params.batch)
		rsslRequestMsgApplyHasBatch(&requestMsg);
	else
	{
		/* Use a single item name. */
		rsslMsgKeyApplyHasName(&requestMsg.msgBase.msgKey);
		if (params.keyNameEscChar)
			requestMsg.msgBase.msgKey.name = MSG_KEY_NAME_ESC_CHAR;
		else
			requestMsg.msgBase.msgKey.name = MSG_KEY_NAME;
	}

	if (params.view)
		rsslRequestMsgApplyHasView(&requestMsg);

	if (params.qualified)
		rsslRequestMsgApplyQualifiedStream(&requestMsg);

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	if (params.batch || params.view)
	{
		/* Encode a payload containing the batch and/or view */
		RsslElementList elementList;
		RsslElementEntry elementEntry;
		RsslArray itemNameArray;

		requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

		ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&requestMsg, 0));

		rsslClearElementList(&elementList);
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(&_eIter, &elementList, NULL, 0));

		if (params.batch)
		{
			/* Encode ItemList Entry. */
			rsslClearElementEntry(&elementEntry);
			elementEntry.name = RSSL_ENAME_BATCH_ITEM_LIST;
			elementEntry.dataType = RSSL_DT_ARRAY;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

			/* Encode ItemList Array. */
			rsslClearArray(&itemNameArray);
			itemNameArray.primitiveType = RSSL_DT_ASCII_STRING;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &itemNameArray));

			/* Encode item names. */
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_ITEM_NAMES[0]));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_ITEM_NAMES[1]));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_ITEM_NAMES[2]));

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
		}

		if (params.view)
		{
			/* Encode ViewType Entry. */
			RsslUInt viewType = RDM_VIEW_TYPE_FIELD_ID_LIST;
			rsslClearElementEntry(&elementEntry);
			elementEntry.name = RSSL_ENAME_VIEW_TYPE;
			elementEntry.dataType = RSSL_DT_UINT;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &viewType));

			/* Encode ViewData Entry. */
			rsslClearElementEntry(&elementEntry);
			elementEntry.name = RSSL_ENAME_VIEW_DATA;
			elementEntry.dataType = RSSL_DT_ARRAY;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

			/* Encode FID Array. */
			rsslClearArray(&itemNameArray);
			itemNameArray.primitiveType = RSSL_DT_INT;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &itemNameArray));

			/* Encode FIDs */
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &VIEW_FIDS[0]));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &VIEW_FIDS[1]));

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
		}

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(&_eIter, RSSL_TRUE));
	}
	else
	{
		requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsg(&_eIter, (RsslMsg*)&requestMsg));
	}

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	
	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message type. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Request", _jsonDocument["Type"].GetString());

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

			/* Check Priority. */
			if (params.priority)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Priority"));
				ASSERT_TRUE(_jsonDocument["Priority"].IsObject());
				ASSERT_TRUE(_jsonDocument["Priority"].HasMember("Class"));
				ASSERT_TRUE(_jsonDocument["Priority"]["Class"].IsNumber());
				EXPECT_EQ(PRIORITY_CLASS, _jsonDocument["Priority"]["Class"].GetInt());
				ASSERT_TRUE(_jsonDocument["Priority"].HasMember("Count"));
				ASSERT_TRUE(_jsonDocument["Priority"]["Count"].IsNumber());
				EXPECT_EQ(PRIORITY_COUNT, _jsonDocument["Priority"]["Count"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("Priority"));

			/* Check Streaming flag. */
			if (params.streaming)
				EXPECT_FALSE(_jsonDocument.HasMember("Streaming"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Streaming"));
				ASSERT_TRUE(_jsonDocument["Streaming"].IsBool());
				EXPECT_FALSE(_jsonDocument["Streaming"].GetBool());
			}

			/* Check KeyInUpdates flag. */
			if (params.keyInUpdates)
				EXPECT_FALSE(_jsonDocument.HasMember("KeyInUpdates"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("KeyInUpdates"));
				ASSERT_TRUE(_jsonDocument["KeyInUpdates"].IsBool());
				EXPECT_FALSE(_jsonDocument["KeyInUpdates"].GetBool());
			}

			/* Check ConfInfoInUpdates flag. */
			if (params.confInfoInUpdates)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("ConfInfoInUpdates"));
				ASSERT_TRUE(_jsonDocument["ConfInfoInUpdates"].IsBool());
				EXPECT_TRUE(_jsonDocument["ConfInfoInUpdates"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("ConfInfoInUpdates"));

			/* Check Refresh flag. */
			if (params.noRefresh)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Refresh"));
				ASSERT_TRUE(_jsonDocument["Refresh"].IsBool());
				EXPECT_FALSE(_jsonDocument["Refresh"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Refresh"));

			/* Check Qos. */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Qos"));
				ASSERT_TRUE(_jsonDocument["Qos"].IsObject());
				ASSERT_TRUE(_jsonDocument["Qos"].HasMember("Timeliness"));
				ASSERT_TRUE(_jsonDocument["Qos"]["Timeliness"].IsString());
				EXPECT_STREQ("Realtime", _jsonDocument["Qos"]["Timeliness"].GetString());
				ASSERT_TRUE(_jsonDocument["Qos"].HasMember("Rate"));
				ASSERT_TRUE(_jsonDocument["Qos"]["Rate"].IsString());
				EXPECT_STREQ("TickByTick", _jsonDocument["Qos"]["Rate"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Qos"));

			/* Check WorstQos. */
			if (params.worstQos)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("WorstQos"));
				ASSERT_TRUE(_jsonDocument["WorstQos"].IsObject());
				ASSERT_TRUE(_jsonDocument["WorstQos"].HasMember("Timeliness"));
				ASSERT_TRUE(_jsonDocument["WorstQos"]["Timeliness"].IsString());
				EXPECT_STREQ("DelayedUnknown", _jsonDocument["WorstQos"]["Timeliness"].GetString());
				ASSERT_TRUE(_jsonDocument["WorstQos"].HasMember("Rate"));
				ASSERT_TRUE(_jsonDocument["WorstQos"]["Rate"].IsString());
				EXPECT_STREQ("JitConflated", _jsonDocument["WorstQos"]["Rate"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("WorstQos"));

			/* Check Private flag. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Private"));
				ASSERT_TRUE(_jsonDocument["Private"].IsBool());
				EXPECT_TRUE(_jsonDocument["Private"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Private"));

			/* Check Pause flag. */
			if (params.pause)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("Pause"));
				ASSERT_TRUE(_jsonDocument["Pause"].IsBool());
				EXPECT_TRUE(_jsonDocument["Pause"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("Pause"));

			/* Check MsgKey & Batch */
			ASSERT_TRUE(_jsonDocument.HasMember("Key"));
			ASSERT_TRUE(_jsonDocument["Key"].IsObject());

			ASSERT_TRUE(_jsonDocument["Key"].HasMember("Service"));
			ASSERT_TRUE(_jsonDocument["Key"]["Service"].IsString());
			EXPECT_STREQ(SERVICE_NAME.data, _jsonDocument["Key"]["Service"].GetString());

			ASSERT_TRUE(_jsonDocument["Key"].HasMember("Name"));
			if (params.batch)
			{
				/* In Batch, Name element should be an array of strings. */
				ASSERT_TRUE(_jsonDocument["Key"]["Name"].IsArray());

				const Value& itemList = _jsonDocument["Key"]["Name"];

				ASSERT_EQ(3, itemList.Size());
				ASSERT_TRUE(itemList[0].IsString());
				EXPECT_STREQ(BATCH_ITEM_NAMES[0].data, itemList[0].GetString());
				ASSERT_TRUE(itemList[1].IsString());
				EXPECT_STREQ(BATCH_ITEM_NAMES[1].data, itemList[1].GetString());
				ASSERT_TRUE(itemList[2].IsString());
				EXPECT_STREQ(BATCH_ITEM_NAMES[2].data, itemList[2].GetString());
			}
			else
			{
				/* If not a batch request, Name element should be a single string. */
				ASSERT_TRUE(_jsonDocument["Key"]["Name"].IsString());
				if (params.keyNameEscChar)
					EXPECT_STREQ(MSG_KEY_NAME_ESC_CHAR.data, _jsonDocument["Key"]["Name"].GetString());
				else
					EXPECT_STREQ(MSG_KEY_NAME.data, _jsonDocument["Key"]["Name"].GetString());
			}

			/* Check View. */
			if (params.view)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("View"));
				ASSERT_TRUE(_jsonDocument["View"].IsArray());

				const Value& fidList = _jsonDocument["View"];

				ASSERT_EQ(2, fidList.Size());

				ASSERT_TRUE(fidList[0].IsNumber());
				EXPECT_EQ(VIEW_FIDS[0], fidList[0].GetInt());

				ASSERT_TRUE(fidList[1].IsNumber());
				EXPECT_EQ(VIEW_FIDS[1], fidList[1].GetInt());
			}
			else
			{
				ASSERT_FALSE(_jsonDocument.HasMember("View"));
			}

			if (params.keyIdentifier)
			{
				ASSERT_TRUE(_jsonDocument["Key"].HasMember("Identifier"));
				ASSERT_TRUE(_jsonDocument["Key"]["Identifier"].IsNumber());
				EXPECT_EQ(MSGKEY_IDENTIFIER, _jsonDocument["Key"]["Identifier"].GetInt());
			}
			else
			{
				ASSERT_FALSE(_jsonDocument["Key"].HasMember("Identifier"));
			}

			/* Check Qualified flag. */
			if (params.qualified)
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
			EXPECT_EQ(RSSL_MC_REQUEST, msgBase["c"].GetInt());

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
			if (!params.batch && !params.view)
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
			
			/* Check ExtendedHeader */
			if (params.extendedHeader)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("e"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&EXTENDED_HEADER, _jsonDocument["e"]));
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("e"));

			/* Check Priority. */
			if (params.priority)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("p"));
				ASSERT_TRUE(_jsonDocument["p"].IsObject());
				ASSERT_TRUE(_jsonDocument["p"].HasMember("c"));
				ASSERT_TRUE(_jsonDocument["p"]["c"].IsNumber());
				EXPECT_EQ(PRIORITY_CLASS, _jsonDocument["p"]["c"].GetInt());
				ASSERT_TRUE(_jsonDocument["p"].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["p"]["n"].IsNumber());
				EXPECT_EQ(PRIORITY_COUNT, _jsonDocument["p"]["n"].GetInt());
			}
			else
				ASSERT_FALSE(_jsonDocument.HasMember("p"));

			/* Check Streaming flag. */
			if (params.streaming)
				EXPECT_FALSE(_jsonDocument.HasMember("s"));
			else
			{
				ASSERT_TRUE(_jsonDocument.HasMember("s"));
				ASSERT_TRUE(_jsonDocument["s"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["s"].GetInt());
			}

			/* Check KeyInUpdates flag. */
			if (params.keyInUpdates)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("i"));
				ASSERT_TRUE(_jsonDocument["i"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["i"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("i"));

			/* Check ConfInfoInUpdates flag. */
			if (params.confInfoInUpdates)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("c"));
				ASSERT_TRUE(_jsonDocument["c"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["c"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("c"));

			/* Check Refresh flag. */
			if (params.noRefresh)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("n"));
				ASSERT_TRUE(_jsonDocument["n"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["n"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("n"));

			/* Check Qos. */
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

			/* Check WorstQos. */
			if (params.worstQos)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("w"));
				ASSERT_TRUE(_jsonDocument["w"].IsObject());
				ASSERT_TRUE(_jsonDocument["w"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["w"]["t"].IsNumber());
				EXPECT_EQ(RSSL_QOS_TIME_DELAYED_UNKNOWN, _jsonDocument["w"]["t"].GetInt());
				ASSERT_TRUE(_jsonDocument["w"].HasMember("r"));
				ASSERT_TRUE(_jsonDocument["w"]["r"].IsNumber());
				EXPECT_EQ(RSSL_QOS_RATE_JIT_CONFLATED, _jsonDocument["w"]["r"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("w"));

			/* Check Private flag. */
			if (params.privateStream)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("u"));
				ASSERT_TRUE(_jsonDocument["u"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["u"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("u"));

			/* Check Pause flag. */
			if (params.pause)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("h"));
				ASSERT_TRUE(_jsonDocument["h"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["h"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("h"));

			/* Check MsgKey */
			ASSERT_TRUE(_jsonDocument.HasMember("k"));
			ASSERT_TRUE(_jsonDocument["k"].IsObject());

			if (params.batch)
				EXPECT_FALSE(_jsonDocument["k"].HasMember("n"));
			else
			{
				ASSERT_TRUE(_jsonDocument["k"].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["k"]["n"].IsString());
				if (params.keyNameEscChar)
					EXPECT_STREQ(MSG_KEY_NAME_ESC_CHAR.data, _jsonDocument["k"]["n"].GetString());
				else
					EXPECT_STREQ(MSG_KEY_NAME.data, _jsonDocument["k"]["n"].GetString());
			}

			ASSERT_TRUE(_jsonDocument["k"].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["k"]["s"].IsNumber());
			EXPECT_EQ(MSGKEY_SVC_ID, _jsonDocument["k"]["s"].GetInt());

			if (params.keyIdentifier)
			{
				ASSERT_TRUE(_jsonDocument["k"].HasMember("i"));
				ASSERT_TRUE(_jsonDocument["k"]["i"].IsNumber());
				EXPECT_EQ(MSGKEY_IDENTIFIER, _jsonDocument["k"]["i"].GetInt());
			}
			else
			{
				ASSERT_FALSE(_jsonDocument["k"].HasMember("i"));
			}

			/* Check Qualified flag. */
			if (params.qualified)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("o"));
				ASSERT_TRUE(_jsonDocument["o"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["o"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("o"));

			/* Check view flag. */
			if (params.view)
			{
				ASSERT_TRUE(_jsonDocument.HasMember("v"));
				ASSERT_TRUE(_jsonDocument["v"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["v"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument.HasMember("v"));

			/* Check Batch/View (container and type already checked). */
			if (params.batch || params.view)
			{
				bool foundBatch = false, foundViewType = false, foundViewData = false;

				ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());

				if (!params.view) /* Batch only */
					ASSERT_TRUE(_jsonDocument["d"]["d"].Size() == 1);
				else if (!params.batch) /* View only */
					ASSERT_TRUE(_jsonDocument["d"]["d"].Size() == 2);
				else /* Both Batch and View */
					ASSERT_TRUE(_jsonDocument["d"]["d"].Size() == 3);
				
				int size = _jsonDocument["d"]["d"].Size(); 
				for (int i = 0; i < size; ++i)
				{
					ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("n"));
					ASSERT_TRUE(_jsonDocument["d"]["d"][i]["n"].IsString());

					if (0 == strcmp(":ItemList", _jsonDocument["d"]["d"][i]["n"].GetString()))
					{
						ASSERT_TRUE(params.batch);
						foundBatch = true;

						/* Element type should be Array */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("t"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["t"].IsNumber());
						EXPECT_EQ(RSSL_DT_ARRAY, _jsonDocument["d"]["d"][i]["t"].GetInt());

						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("d"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].IsObject());

						/* Array type should be AsciiString. */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].HasMember("t"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"]["t"].IsNumber());
						EXPECT_EQ(RSSL_DT_ASCII_STRING, _jsonDocument["d"]["d"][i]["d"]["t"].GetInt());

						/* Data should be an array of three names. */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].HasMember("d"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"]["d"].IsArray());
						const Value &itemList = _jsonDocument["d"]["d"][i]["d"]["d"];
						ASSERT_EQ(3, itemList.Size());
						ASSERT_TRUE(itemList[0].IsString());
						EXPECT_STREQ(BATCH_ITEM_NAMES[0].data, itemList[0].GetString());
						ASSERT_TRUE(itemList[1].IsString());
						EXPECT_STREQ(BATCH_ITEM_NAMES[1].data, itemList[1].GetString());
						ASSERT_TRUE(itemList[2].IsString());
						EXPECT_STREQ(BATCH_ITEM_NAMES[2].data, itemList[2].GetString());
					}
					else if (0 == strcmp(":ViewType", _jsonDocument["d"]["d"][i]["n"].GetString()))
					{
						ASSERT_TRUE(params.view);
						foundViewType = true;

						/* Element type should be UInt */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("t"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["t"].IsNumber());
						EXPECT_EQ(RSSL_DT_UINT, _jsonDocument["d"]["d"][i]["t"].GetInt());

						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("d"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].IsNumber());
						EXPECT_EQ(RDM_VIEW_TYPE_FIELD_ID_LIST, _jsonDocument["d"]["d"][i]["d"].GetInt());
					}
					else if (0 == strcmp(":ViewData", _jsonDocument["d"]["d"][i]["n"].GetString()))
					{
						ASSERT_TRUE(params.view);
						foundViewData = true;

						/* Element type should be Array */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("t"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["t"].IsNumber());
						EXPECT_EQ(RSSL_DT_ARRAY, _jsonDocument["d"]["d"][i]["t"].GetInt());

						ASSERT_TRUE(_jsonDocument["d"]["d"][i].HasMember("d"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].IsObject());

						/* Array type should be Int. */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].HasMember("t"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"]["t"].IsNumber());
						EXPECT_EQ(RSSL_DT_INT, _jsonDocument["d"]["d"][i]["d"]["t"].GetInt());

						/* Data should be an array of three names. */
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"].HasMember("d"));
						ASSERT_TRUE(_jsonDocument["d"]["d"][i]["d"]["d"].IsArray());
						const Value &viewData = _jsonDocument["d"]["d"][i]["d"]["d"];
						ASSERT_EQ(2, viewData.Size());
						ASSERT_TRUE(viewData[0].IsNumber());
						EXPECT_EQ(VIEW_FIDS[0], viewData[0].GetInt());
						ASSERT_TRUE(viewData[1].IsNumber());
						EXPECT_EQ(VIEW_FIDS[1], viewData[1].GetInt());
					}
					else
						FAIL() << "Unrecognized Element Name " << _jsonDocument["d"]["d"][i]["n"].GetString();

				}

				ASSERT_EQ(params.batch, foundBatch);
				ASSERT_EQ(params.view, foundViewType);
				ASSERT_EQ(params.view, foundViewData);
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

	/* Verify that RsslRequestMsg is correct. */
	EXPECT_EQ(RSSL_MC_REQUEST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);

	/* Check ExtendedHeader. */
	if (params.extendedHeader)
	{
		ASSERT_TRUE(rsslRequestMsgCheckHasExtendedHdr(&rsslMsg.requestMsg));
		EXPECT_EQ(RSSL_TRUE, rsslBufferIsEqual(&EXTENDED_HEADER, &rsslMsg.requestMsg.extendedHeader));
	}
	else
		EXPECT_FALSE(rsslRequestMsgCheckHasExtendedHdr(&rsslMsg.requestMsg));

	/* Check Priority. */
	if (params.priority)
	{
		ASSERT_TRUE(rsslRequestMsgCheckHasPriority(&rsslMsg.requestMsg));
		EXPECT_EQ(PRIORITY_CLASS, rsslMsg.requestMsg.priorityClass);
		EXPECT_EQ(PRIORITY_COUNT, rsslMsg.requestMsg.priorityCount);
	}
	else
		EXPECT_FALSE(rsslRequestMsgCheckHasPriority(&rsslMsg.requestMsg));

	/* Check Streaming flag */
	EXPECT_EQ(params.streaming, rsslRequestMsgCheckStreaming(&rsslMsg.requestMsg));

	/* Check KeyInUpdates flag */
	EXPECT_EQ(params.keyInUpdates, rsslRequestMsgCheckMsgKeyInUpdates(&rsslMsg.requestMsg));

	/* Check ConfInfoInUpdates flag. */
	EXPECT_EQ(params.confInfoInUpdates, rsslRequestMsgCheckConfInfoInUpdates(&rsslMsg.requestMsg));

	/* Check HasBatch flag. */
	EXPECT_EQ(params.batch, rsslRequestMsgCheckHasBatch(&rsslMsg.requestMsg));

	/* Check HasView flag. */
	EXPECT_EQ(params.view, rsslRequestMsgCheckHasView(&rsslMsg.requestMsg));

	/* Check NoRefresh flag. */
	EXPECT_EQ(params.noRefresh, rsslRequestMsgCheckNoRefresh(&rsslMsg.requestMsg));

    /* Check key.identifier */
    if (params.keyIdentifier)
    {
        ASSERT_TRUE(rsslMsgKeyCheckHasIdentifier(&rsslMsg.msgBase.msgKey));
        EXPECT_EQ(MSGKEY_IDENTIFIER, rsslMsg.msgBase.msgKey.identifier);
    }
    else
        EXPECT_FALSE(rsslMsgKeyCheckHasIdentifier(&rsslMsg.msgBase.msgKey));

	if (params.batch || params.view)
	{
		bool foundBatchElement = false, foundViewTypeElement = false, foundViewDataElement = false;
		RsslRet ret;

		EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);

		/* Decode the DataBody for any Batch- or View-related elements. */
		RsslElementList elementList;
		RsslElementEntry elementEntry;
		RsslArray rsslArray;
		RsslBuffer arrayEntry;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

		while ((ret = rsslDecodeElementEntry(&_dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (rsslBufferIsEqual(&RSSL_ENAME_BATCH_ITEM_LIST, &elementEntry.name))
			{
				/* Decode Item List. */
				RsslBuffer itemName;

				foundBatchElement = true;

				ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &rsslArray));
				ASSERT_EQ(RSSL_DT_ASCII_STRING, rsslArray.primitiveType);

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &itemName));
				EXPECT_TRUE(rsslBufferIsEqual(&BATCH_ITEM_NAMES[0], &itemName));

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &itemName));
				EXPECT_TRUE(rsslBufferIsEqual(&BATCH_ITEM_NAMES[1], &itemName));

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &itemName));
				EXPECT_TRUE(rsslBufferIsEqual(&BATCH_ITEM_NAMES[2], &itemName));


				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
			}
			else if (rsslBufferIsEqual(&RSSL_ENAME_VIEW_TYPE, &elementEntry.name))
			{
				RsslInt viewType;

				foundViewTypeElement = true;

				ASSERT_EQ(RSSL_DT_UINT, elementEntry.dataType);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &viewType));
				EXPECT_EQ(RDM_VIEW_TYPE_FIELD_ID_LIST, viewType);
			}
			else if (rsslBufferIsEqual(&RSSL_ENAME_VIEW_DATA, &elementEntry.name))
			{
				RsslInt viewFid;

				foundViewDataElement = true;

				ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &rsslArray));
				ASSERT_EQ(RSSL_DT_INT, rsslArray.primitiveType);

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &viewFid));
				EXPECT_EQ(VIEW_FIDS[0], viewFid);

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &viewFid));
				EXPECT_EQ(VIEW_FIDS[1], viewFid);

				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
			}
			else
			{
				string elemName(elementEntry.name.data, elementEntry.name.length);
				FAIL() << "rsslDecodeElementEntry decoded unknown ElementEntry name: " << elemName;
			}
		}

		EXPECT_EQ(params.batch, foundBatchElement);
		EXPECT_EQ(params.view, foundViewTypeElement);
		EXPECT_EQ(params.view, foundViewDataElement);
	}
	else
		EXPECT_EQ(RSSL_DT_NO_DATA, rsslMsg.msgBase.containerType);

	EXPECT_EQ(params.qualified, rsslRequestMsgCheckQualifiedStream(&rsslMsg.requestMsg));
}

INSTANTIATE_TEST_SUITE_P(RequestMsgTests, RequestMsgMembersTestFixture, ::testing::Values(
	/* Test with/without ExtendedHeader, Priority, Streaming, KeyInUpdates, ConfInfoInUpdates, NoRefresh, Qos, WorstQos, 
	 * PrivateStream, Pause, Batch, View, Qualified, KeyIdentifier */

	/* Defaults */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, false, false, false, false),

	/* ExtendedHeader */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, true, false, true, true, false, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, true, true, false, false, false, false, false, false, false, false, false, false),

	/* Priority */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, true, true, true, false, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, true, true, false, false, false, false, false, false, false, false, false, false),

	/* Streaming (false), */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, true, false, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true, false, false, false, false, false, false, false, false, false, false),

	/* KeyInUpdates (false), */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, false, false, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false, false, false, false, false, false, false, false, false, false, false),

	/* ConfInfoInUpdates */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, true, false, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, true, false, false, false, false, false, false, false, false, false),

	/* NoRefresh */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, true, false, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, true, false, false, false, false, false, false, false, false),

	/* Qos */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, true, false, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, true, false, false, false, false, false, false, false),

	/* WorstQoS */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, true, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, true, false, false, false, false, false, false),

	/* Qos and WorstQos */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, true, true, false, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, true, true, false, false, false, false, false, false),

	/* PrivateStream */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, true, false, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, true, false, false, false, false, false),

	/* Pause */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, true, false, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, true, false, false, false, false),

	/* Batch */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, true, false, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, true, false, false, false),

	/* View */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, false, true, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, false, true, false, false),

	/* Batch and View */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, true, true, false, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, true, true, false, false),

	/* Qualified */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, false, false, true, false),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, false, false, true, false),
	
    /* KeyIdentifier */
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, false, false, false, true),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, false, false, false, true),

	/* KeyNameEscChars*/
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, true, false, false, false, false, false, false, false, false, false, false, true),
	RequestMsgMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, true, false, false, false, false, false, false, false, false, false, false, true)
));

#define BATCH_ITEM_COUNT 2000
TEST_F(RequestMsgTests, RequestMsgBatchTest)
{
	/* Convert a varying-size batch request from RWF to JSON, and back to RWF. */

	char BATCH_ITEM_NAME_STRINGS[BATCH_ITEM_COUNT][9];
	RsslBuffer BATCH_ITEM_NAMES[BATCH_ITEM_COUNT];

	RsslRequestMsg requestMsg;
	RsslMsg rsslMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslArray itemNameArray;
	RsslBuffer arrayEntry;

	/* Build list of item names. */
	for(int i = 0; i < BATCH_ITEM_COUNT; ++i)
	{
		BATCH_ITEM_NAMES[i].length = snprintf(BATCH_ITEM_NAME_STRINGS[i], 9, "ITEM%04d", i);
		BATCH_ITEM_NAMES[i].data = BATCH_ITEM_NAME_STRINGS[i];
	}

	for (int i = 0; i <= BATCH_ITEM_COUNT; ++i)
	{
		/* Encode the RsslRequestMsg. */
		rsslClearRequestMsg(&requestMsg);

		requestMsg.msgBase.streamId = 5;
		requestMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
		requestMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;

		rsslRequestMsgApplyStreaming(&requestMsg);
		rsslRequestMsgApplyMsgKeyInUpdates(&requestMsg);
		rsslRequestMsgApplyHasBatch(&requestMsg);

		rsslClearEncodeIterator(&_eIter);
		rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
		rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&requestMsg, 0));

		rsslClearElementList(&elementList);
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(&_eIter, &elementList, NULL, 0));

		/* Encode ItemList Entry. */
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = RSSL_ENAME_BATCH_ITEM_LIST;
		elementEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		/* Encode ItemList Array. */
		rsslClearArray(&itemNameArray);
		itemNameArray.primitiveType = RSSL_DT_ASCII_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &itemNameArray));

		/* Encode item names. */
		for (int j = 0; j < i; ++j)
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &BATCH_ITEM_NAMES[j]));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

		ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

		/* Check message type. */
		ASSERT_TRUE(_jsonDocument.HasMember("Type"));
		ASSERT_TRUE(_jsonDocument["Type"].IsString());
		EXPECT_STREQ("Request", _jsonDocument["Type"].GetString());

		ASSERT_TRUE(_jsonDocument.HasMember("ID"));
		ASSERT_TRUE(_jsonDocument["ID"].IsNumber());
		EXPECT_EQ(5, _jsonDocument["ID"].GetInt());

		/* Check Domain. */
		ASSERT_FALSE(_jsonDocument.HasMember("Domain")); /* MarketPrice */

		/* Check MsgKey & Batch */
		ASSERT_TRUE(_jsonDocument.HasMember("Key"));
		ASSERT_TRUE(_jsonDocument["Key"].IsObject());
		ASSERT_TRUE(_jsonDocument["Key"].HasMember("Name"));

		/* In Batch, Name element should be an array of strings. */
		ASSERT_TRUE(_jsonDocument["Key"]["Name"].IsArray());

		const Value& itemList = _jsonDocument["Key"]["Name"];

		ASSERT_EQ(i, itemList.Size());

		for (int j = 0; j < i; ++j)
		{
			ASSERT_TRUE(itemList[j].IsString());
			EXPECT_STREQ(BATCH_ITEM_NAMES[j].data, itemList[j].GetString());
		}

		/* Convert back to RWF. */
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Verify that RsslRequestMsg is correct. */
		EXPECT_EQ(RSSL_MC_REQUEST, rsslMsg.msgBase.msgClass);
		EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
		EXPECT_TRUE(rsslRequestMsgCheckHasBatch(&rsslMsg.requestMsg));
		EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);

		/* Decode the ItemList. */

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementEntry(&_dIter, &elementEntry));
		ASSERT_TRUE(rsslBufferIsEqual(&RSSL_ENAME_BATCH_ITEM_LIST, &elementEntry.name));

		/* Decode Item List. */
		RsslBuffer itemName;

		ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &itemNameArray));
		ASSERT_EQ(RSSL_DT_ASCII_STRING, itemNameArray.primitiveType);

		for (int j = 0; j < i; ++j)
		{
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &itemName));
			EXPECT_TRUE(rsslBufferIsEqual(&BATCH_ITEM_NAMES[j], &itemName));
		}

		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &arrayEntry));
		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeElementEntry(&_dIter, &elementEntry));
	}
}
