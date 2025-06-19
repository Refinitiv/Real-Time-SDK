/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslJsonConverterTestBase.h"
#include <cstdarg>

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json; 

/* Fixture for MapTests that has conversion code. */
class MapTests : public MsgConversionTestBase
{
};

/* Parameters for Map Entry Action tests. */
class MapEntryActionsTestParams
{
	public:

	RsslMapEntryActions actionArray[32];
	RsslUInt8 actionArrayCount;

	MapEntryActionsTestParams(int numActions, ...)
	{
		this->actionArrayCount = numActions;
		va_list arguments;

		va_start(arguments, numActions);
		for (int i = 0; i < numActions; i++)
			this->actionArray[i] = (RsslMapEntryActions)va_arg(arguments, int);

		va_end(arguments);
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const MapEntryActionsTestParams& params)
	{
		out << "MapEntryAction array contains";
		for (int i = 0; i < params.actionArrayCount; i++)
			if (params.actionArray[i] != 0)
				out << " " << rsslMapEntryActionToOmmString(params.actionArray[i]);
			else
				out << "";
		return out;
	}
};

/* Parameters for Map tests. */
class MapMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool summaryData;
	bool totalCountHint;
	bool keyFieldId;
	bool entryPermData;

	MapMembersTestParams(RsslJsonProtocolType protocolType, bool summaryData, bool totalCountHint, bool keyFieldId, bool entryPermData)
	{
		this->protocolType = protocolType;
		this->summaryData = summaryData;
		this->totalCountHint = totalCountHint;
		this->keyFieldId = keyFieldId;
		this->entryPermData = entryPermData;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const MapMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"summaryData:" << (params.summaryData ? "true" : "false") << ","
			"totalCountHint:" << (params.totalCountHint ? "true" : "false") << ","
			"keyFieldId:" << (params.keyFieldId ? "true" : "false") << ","
			"entryPermData:" << (params.entryPermData ? "true" : "false") 
			<< "]";
		return out;
	}
};

class MapMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<MapMembersTestParams>
{
};

/* Test that converts a Map with a FieldList from RWF to JSON, and back to RWF. */
TEST_P(MapMembersTestFixture, MapMembersTest)
{
	MapMembersTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;

	const RsslUInt32 TOTAL_COUNT_HINT = 5;
	const RsslUInt MAP_ENTRY_KEY = 6;
	const RsslFieldId KEY_FIELD_ID = UINT_FIELD.fieldId;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_MAP;
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

	rsslClearMap(&map);
	map.containerType = RSSL_DT_FIELD_LIST;
	map.keyPrimitiveType = RSSL_DT_UINT;

	if (params.totalCountHint)
	{
		rsslMapApplyHasTotalCountHint(&map);
		map.totalCountHint = TOTAL_COUNT_HINT;
	}

	if (params.keyFieldId)
	{
		rsslMapApplyHasKeyFieldId(&map);
		map.keyFieldId = KEY_FIELD_ID;
	}

	if (params.summaryData)
		rsslMapApplyHasSummaryData(&map);

	if (params.entryPermData)
		rsslMapApplyHasPerEntryPermData(&map);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));

	if (params.summaryData)
	{
		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapSummaryDataComplete(&_eIter, RSSL_TRUE));
	}
	
	/* Encode an entry. */
	rsslClearMapEntry(&mapEntry);
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;

	if (params.entryPermData)
	{
		rsslMapEntryApplyHasPermData(&mapEntry);
		mapEntry.permData = PERM_DATA;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &MAP_ENTRY_KEY, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch(params.protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
		{
			/* Check message. */
			ASSERT_TRUE(_jsonDocument.HasMember("Type"));
			ASSERT_TRUE(_jsonDocument["Type"].IsString());
			EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

			/* Check Map. */
			ASSERT_TRUE(_jsonDocument.HasMember("Map"));
			ASSERT_TRUE(_jsonDocument["Map"].IsObject());

			ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyType"));
			ASSERT_TRUE(_jsonDocument["Map"]["KeyType"].IsString());
			EXPECT_STREQ("UInt", _jsonDocument["Map"]["KeyType"].GetString());

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
				ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("Fields"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Map"]["Summary"]["Fields"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Map"].HasMember("Summary"));

			if (params.totalCountHint)
			{
				ASSERT_TRUE(_jsonDocument["Map"].HasMember("CountHint"));
				ASSERT_TRUE(_jsonDocument["Map"]["CountHint"].IsNumber());
				EXPECT_EQ(TOTAL_COUNT_HINT, _jsonDocument["Map"]["CountHint"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Map"].HasMember("CountHint"));

			if (params.keyFieldId)
			{
				ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyFieldID"));
				ASSERT_TRUE(_jsonDocument["Map"]["KeyFieldID"].IsNumber());
				EXPECT_EQ(KEY_FIELD_ID, _jsonDocument["Map"]["KeyFieldID"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Map"].HasMember("KeyFieldID"));

			/* Check MapEntry */
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Map"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Add", entries[0]["Action"].GetString());

			if (params.entryPermData)
			{
				ASSERT_TRUE(entries[0].HasMember("PermData"));
				ASSERT_TRUE(entries[0]["PermData"].IsString());
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, entries[0]["PermData"]));
			}
			else
				EXPECT_FALSE(entries[0].HasMember("PermData"));
			
			ASSERT_TRUE(entries[0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(entries[0]["Fields"]));
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

			/* Check ContainerType. */
			ASSERT_TRUE(msgBase.HasMember("f"));
			ASSERT_TRUE(msgBase["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_MAP - 128, msgBase["f"].GetInt());

			/* Check Map ContainerType. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_FIELD_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			/* Check Map. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_FIELD_LIST - 128, _jsonDocument["d"]["f"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"].HasMember("k"));
			ASSERT_TRUE(_jsonDocument["d"]["k"].IsNumber());
			ASSERT_EQ(RSSL_DT_UINT, _jsonDocument["d"]["k"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["s"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["s"].HasMember("d"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"]["s"], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("s"));

			if (params.totalCountHint)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["h"].IsNumber());
				EXPECT_EQ(TOTAL_COUNT_HINT, _jsonDocument["d"]["h"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("h"));

			if (params.keyFieldId)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("i"));
				ASSERT_TRUE(_jsonDocument["d"]["i"].IsNumber());
				EXPECT_EQ(KEY_FIELD_ID, _jsonDocument["d"]["i"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("i"));

			if (params.entryPermData)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("p"));
				ASSERT_TRUE(_jsonDocument["d"]["p"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["d"]["p"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("p"));


			/* Map Entry */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			/* Action */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());

			/* Key */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("k"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["k"].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["k"].GetInt());

			/* PermData */
			if (params.entryPermData)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("p"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][0]["p"].IsString());
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, _jsonDocument["d"]["d"][0]["p"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"][0].HasMember("p"));

			/* Entry Payload */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"]["d"][0]["d"], params.protocolType));
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
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	if (params.summaryData)
	{
		ASSERT_TRUE(rsslMapCheckHasSummaryData(&map));
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
	}
	else
		EXPECT_FALSE(rsslMapCheckHasSummaryData(&map));

	if (params.totalCountHint)
	{
		ASSERT_TRUE(rsslMapCheckHasTotalCountHint(&map));
		EXPECT_EQ(TOTAL_COUNT_HINT, map.totalCountHint);
	}
	else
		EXPECT_FALSE(rsslMapCheckHasTotalCountHint(&map));

	if (params.keyFieldId)
	{
		ASSERT_TRUE(rsslMapCheckHasKeyFieldId(&map));
		EXPECT_EQ(KEY_FIELD_ID, map.keyFieldId);
	}
	else
		EXPECT_FALSE(rsslMapCheckHasKeyFieldId(&map));

	EXPECT_EQ(params.entryPermData, rsslMapCheckHasPerEntryPermData(&map));

	/* Check MapEntry. */
	RsslUInt mapEntryKey;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(MAP_ENTRY_KEY, mapEntryKey);

	if (params.entryPermData)
	{
		ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
		EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData));
	}
	else
		EXPECT_FALSE(rsslMapEntryCheckHasPermData(&mapEntry));

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check MapEntry FieldList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}


INSTANTIATE_TEST_SUITE_P(MapTests, MapMembersTestFixture, ::testing::Values(

	/* Test with/without SummaryData, TotalCountHint, KeyFieldId, Per-Entry PermData */

	/* Nothing */
	MapMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, false),
	MapMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false),

	/* SummaryData */
	MapMembersTestParams(RSSL_JSON_JPT_JSON, true, false, false, false),
	MapMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false),

	/* TotalCountHint */
	MapMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false, false),
	MapMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false),

	/* KeyFieldId */
	MapMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true, false),
	MapMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false),

	/* Per-Entry PermData */
	MapMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false, true),
	MapMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true)
));


/* Test that converts an empty Map RWF to JSON, and back to RWF. */
TEST_F(MapTests, EmptyMapTest /* This test triggers an assertion failure in map encoding when converting from JSON to RWF. */)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_MAP;
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

	/* Encode an empty map. */
	rsslClearMap(&map);
	map.containerType = RSSL_DT_FIELD_LIST;
	map.keyPrimitiveType = RSSL_DT_UINT;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Map. */
	ASSERT_TRUE(_jsonDocument.HasMember("Map"));
	ASSERT_TRUE(_jsonDocument["Map"].IsObject());

	ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyType"));
	ASSERT_TRUE(_jsonDocument["Map"]["KeyType"].IsString());
	EXPECT_STREQ("UInt", _jsonDocument["Map"]["KeyType"].GetString());

	/* Check MapEntry */
	ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());
	EXPECT_EQ(0, _jsonDocument["Map"]["Entries"].Size());

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
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_NO_DATA, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	RsslUInt mapEntryKey;
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}

class MapEntryActionsTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<MapEntryActionsTestParams> {
};

/* Test that converts a Map whose entries are each of the different actions from RWF to JSON, and back to RWF. */
TEST_P(MapEntryActionsTestFixture, MapEntryActionsTest)
{
	MapEntryActionsTestParams const &params = GetParam();

	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslBool foundUnknownAction = false;

	const RsslUInt MAP_ENTRY_KEYS[5] = {6,7,8,9,10};
	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_MAP;
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

	rsslClearMap(&map);
	map.containerType = RSSL_DT_FIELD_LIST;
	map.keyPrimitiveType = RSSL_DT_UINT;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));
	
	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Encode an entry with the action. */
		rsslClearMapEntry(&mapEntry);

		if(params.actionArray[i] == 0)
			foundUnknownAction = true;

		mapEntry.action = params.actionArray[i];
		if (mapEntry.action == RSSL_MPEA_DELETE_ENTRY)
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntry(&_eIter, &mapEntry, &MAP_ENTRY_KEYS[i]));
		else
		{
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &MAP_ENTRY_KEYS[i], 0));
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(&_eIter, RSSL_TRUE));
		}
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	if (foundUnknownAction)
	{
		convertRsslToJson(RSSL_JSON_JPT_JSON2, true, RSSL_RET_FAILURE);
		return;
	}
	else
		ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Map. */
	ASSERT_TRUE(_jsonDocument.HasMember("Map"));
	ASSERT_TRUE(_jsonDocument["Map"].IsObject());

	ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyType"));
	ASSERT_TRUE(_jsonDocument["Map"]["KeyType"].IsString());
	EXPECT_STREQ("UInt", _jsonDocument["Map"]["KeyType"].GetString());

	/* Check MapEntries */
	ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Map"]["Entries"];
	ASSERT_EQ(params.actionArrayCount, entries.Size());

	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Check Entry with the action. */
		ASSERT_TRUE(entries[i].HasMember("Key"));
		ASSERT_TRUE(entries[i]["Key"].IsNumber());
		EXPECT_EQ(MAP_ENTRY_KEYS[i], entries[i]["Key"].GetInt());

		ASSERT_TRUE(entries[i].HasMember("Action"));
		ASSERT_TRUE(entries[i]["Action"].IsString());
		EXPECT_STREQ(rsslMapEntryActionToOmmString(params.actionArray[i]), entries[i]["Action"].GetString());
		if (strncmp(entries[i]["Action"].GetString(), RSSL_OMMSTR_MPEA_DELETE_ENTRY.data, RSSL_OMMSTR_MPEA_DELETE_ENTRY.length) != 0)
		{
			ASSERT_TRUE(entries[i].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(entries[i]["Fields"]));
		}
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
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(params.actionArrayCount == 3 ? RSSL_DT_FIELD_LIST : RSSL_DT_NO_DATA, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	/* Check MapEntries. */
	RsslUInt mapEntryKey;

	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Check Entry with the action. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
		EXPECT_EQ(MAP_ENTRY_KEYS[i], mapEntryKey);
		ASSERT_EQ(params.actionArray[i], mapEntry.action);
		if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}

INSTANTIATE_TEST_SUITE_P(MapTests, MapEntryActionsTestFixture, ::testing::Values(
	MapEntryActionsTestParams(2, 0, 0),
	MapEntryActionsTestParams(3, RSSL_MPEA_ADD_ENTRY, RSSL_MPEA_UPDATE_ENTRY, RSSL_MPEA_DELETE_ENTRY),
	MapEntryActionsTestParams(3, RSSL_MPEA_DELETE_ENTRY, RSSL_MPEA_UPDATE_ENTRY, RSSL_MPEA_ADD_ENTRY),
	MapEntryActionsTestParams(2, RSSL_MPEA_DELETE_ENTRY, RSSL_MPEA_DELETE_ENTRY)
));

class MapContainerTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RsslDataTypeParam> {
};

/* Test that converts a Map whose entries & summary data are each of the different actions from RWF to JSON, and back to RWF. */
TEST_P(MapContainerTypesTestFixture, MapContainerTypesTest)
{
	const RsslDataTypes containerType = GetParam().dataType;
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;

	const RsslUInt MAP_ENTRY_KEY = 6;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_MAP;
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

	rsslClearMap(&map);
	map.containerType = containerType;
	map.keyPrimitiveType = RSSL_DT_UINT;
	rsslMapApplyHasSummaryData(&map);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));

	/* Encode summary data with the given container type. */
	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFilterList(&_eIter));
			break;

		default:
			FAIL() << "Attempting to encode unhandled containerType " << containerType;
			break;
	}
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapSummaryDataComplete(&_eIter, RSSL_TRUE));

	
	/* Encode an entry with the given container. */
	rsslClearMapEntry(&mapEntry);
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &MAP_ENTRY_KEY, 0));

	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFilterList(&_eIter));
			break;

		default:
			FAIL() << "Attempting to encode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Map. */
	ASSERT_TRUE(_jsonDocument.HasMember("Map"));
	ASSERT_TRUE(_jsonDocument["Map"].IsObject());

	ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyType"));
	ASSERT_TRUE(_jsonDocument["Map"]["KeyType"].IsString());
	EXPECT_STREQ("UInt", _jsonDocument["Map"]["KeyType"].GetString());

	/* Check Summary Data. */
	switch (containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Map"]["Summary"]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["Map"]["Summary"]["Elements"]));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("FilterList"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(_jsonDocument["Map"]["Summary"]["FilterList"]));
			break;

		default:
			FAIL() << "Attempting to search for unhandled containerType " << containerType;
			break;
	}

	/* Check MapEntry */
	ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Map"]["Entries"];
	ASSERT_EQ(1, entries.Size());

	ASSERT_TRUE(entries[0].HasMember("Action"));
	ASSERT_TRUE(entries[0]["Action"].IsString());
	EXPECT_STREQ("Add", entries[0]["Action"].GetString());

	switch (containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_TRUE(entries[0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(entries[0]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_TRUE(entries[0].HasMember("FilterList"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(entries[0]["FilterList"]));
			break;

		default:
			FAIL() << "Attempting to search for unhandled containerType " << containerType;
			break;
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
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(containerType, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);
	ASSERT_TRUE(rsslMapCheckHasSummaryData(&map));

	/* Check Summary Data. */
	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFilterList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		default:
			FAIL() << "Attempting to decode unhandled containerType " << containerType;
			break;
	}

	/* Check MapEntry. */
	RsslUInt mapEntryKey;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(MAP_ENTRY_KEY, mapEntryKey);
	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFilterList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		default:
			FAIL() << "Attempting to decode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}

INSTANTIATE_TEST_SUITE_P(MapTests, MapContainerTypesTestFixture, ::testing::Values(
	RsslDataTypeParam(RSSL_DT_FIELD_LIST),
	RsslDataTypeParam(RSSL_DT_ELEMENT_LIST),
	RsslDataTypeParam(RSSL_DT_FILTER_LIST)
));

TEST_F(MapTests, InvalidMapTests)
{
	/* Action and Key missing from MapEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Map\":{\"KeyFieldID\":3,\"KeyType\":\"AsciiString\",\"Entries\":[{}] } }");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key "));

	/* Action missing from MapEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Map\":{\"KeyFieldID\":3,\"KeyType\":\"AsciiString\",\"Entries\":[{\"Key\":\"Maker\"}] } }");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Action' for 'Entries'"));

	/* Key missing from MapEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Map\":{\"KeyFieldID\":3,\"KeyType\":\"AsciiString\",\"Entries\":[{\"Action\":\"Add\"}] } }");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Key' for 'Entries'"));
}

/* Test that converts an empty Map RWF to JSON, and back to RWF. */
TEST_F(MapTests, MapEmptyDataTest)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_MAP;
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

	/* Encode an empty map. */
	rsslClearMap(&map);
	map.containerType = RSSL_DT_NO_DATA;
	map.keyPrimitiveType = RSSL_DT_UINT;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Map. */
	ASSERT_TRUE(_jsonDocument.HasMember("Map"));
	ASSERT_TRUE(_jsonDocument["Map"].IsObject());

	ASSERT_TRUE(_jsonDocument["Map"].HasMember("KeyType"));
	ASSERT_TRUE(_jsonDocument["Map"]["KeyType"].IsString());
	EXPECT_STREQ("UInt", _jsonDocument["Map"]["KeyType"].GetString());

	/* Check MapEntry */
	ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));

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
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_NO_DATA, map.containerType);
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry,NULL));
}

TEST_F(MapTests, MapPostOrderBufferKeyPermDataTest)
{
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;

	const RsslBuffer ENTRY_KEY_1234 = { 4, (char*)"1234" };
	const RsslBuffer ENTRY_KEY_1235 = { 4, (char*)"1235" };

	//////////////////////////
	/* Convert back to RWF. */
	//////////////////////////

	setJsonBufferToString(
		"{\"Type\":\"Post\",\"Message\":{\"Solicited\":false,\"Type\":\"Refresh\",\"State\":{\"Stream\":\"Open\",\"Data\":\"Ok\",\"Text\":\"Post Refresh OK\",\"Code\":\"None\"},\"Domain\":\"MarketPrice\",\"ID\":2,\
		\"Map\":{\"KeyFieldID\":3426,\"KeyType\":\"Buffer\",\"CountHint\":2,\"Entries\":[{\"Action\":\"Add\",\"Fields\":{\"ORDER_SIZE\":1000,\"ORDER_PRC\":66.66,\"ORDER_SIDE\":6,\"QUOTIM_MS\":666},\
		\"PermData\":\"UGVyTWlzc2lvbg==\",\"Key\":\"MTIzNA==\"},\
		{\"Action\":\"Add\",\"Fields\":{\"ORDER_SIZE\":1000,\"ORDER_PRC\":66.66,\"ORDER_SIDE\":6,\"QUOTIM_MS\":666},\"Key\":\"MTIzNQ==\",\"PermData\":\"UGVyTWlzc2lvbg==\"}]},\"Key\":{\"Service\":60000,\"Name\":\"TRI.N\"}},\
		\"PermData\":\"AwO9ZWLA\",\"Ack\":true,\"PostUserInfo\":{\"Address\":806749471,\"UserID\":1},\"ID\":1,\"Domain\":\"MarketPrice\",\"Key\":{\"Service\":259,\"Name\":\"TRI.N\"},\"PostID\":1}");

	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslPostMsg is correct. */
	EXPECT_EQ(RSSL_MC_POST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);

	RsslMsg rsslRefreshMsg;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslRefreshMsg));

	EXPECT_EQ(RSSL_MC_REFRESH, rsslRefreshMsg.msgBase.msgClass);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslRefreshMsg.msgBase.domainType);

	/* Check Map. */
	rsslClearMap(&map);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);

	ASSERT_EQ(RSSL_DT_BUFFER, map.keyPrimitiveType);

	EXPECT_FALSE(rsslMapCheckHasSummaryData(&map));
	EXPECT_TRUE(rsslMapCheckHasTotalCountHint(&map));
	EXPECT_TRUE(rsslMapCheckHasKeyFieldId(&map));
	EXPECT_TRUE(rsslMapCheckHasPerEntryPermData(&map));

	EXPECT_EQ(3426, map.keyFieldId);
	EXPECT_EQ(2, map.totalCountHint);

	/* Check 0-th MapEntry. */
	rsslClearMapEntry(&mapEntry);
	RsslBuffer mapBufferKey;
	rsslClearBuffer(&mapBufferKey);


	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapBufferKey));

	EXPECT_TRUE(rsslBufferIsEqual(&ENTRY_KEY_1234, &mapBufferKey)) << "mapBufferKey: " << mapBufferKey.data;

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check 1-st MapEntry. */
	rsslClearMapEntry(&mapEntry);
	rsslClearBuffer(&mapBufferKey);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapBufferKey));
	EXPECT_TRUE(rsslBufferIsEqual(&ENTRY_KEY_1235, &mapBufferKey)) << "mapBufferKey: " << mapBufferKey.data;

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, NULL));
}

TEST_F(MapTests, OrderKeyBufferPermDataMapTest)
{
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslBuffer mapEntryKey;

	const RsslBuffer ENTRY_KEY_1234 = { 4, (char*)"1234" };
	const RsslBuffer ENTRY_KEY_1235 = { 4, (char*)"1235" };
	const RsslBuffer ENTRY_KEY_1234567 = { 7, (char*)"1234567" };


	setJsonBufferToString(
		"{\"ID\":5,\"Type\":\"Post\",\"Key\":{\"Service\":\"DUCK_FEED\",\"Name\":\"TINY\"},\"Complete\":false,\
		\"Map\":{\"KeyType\":\"Buffer\",\"Entries\":[\
		{\"Action\":\"Add\",\"PermData\":\"UGVyTWlzc2lvbg==\",\"Key\":\"MTIzNDU2Nw==\",\"Fields\":{\"BID\":118.734375}},\
		{\"Action\":\"Delete\",\"Key\":\"MTIzNQ==\",\"PermData\":\"UGVyTWlzc2lvbg==\",\"Fields\":{\"BID\":118.734375}},\
		{\"PermData\":\"UGVyTWlzc2lvbg==\",\"Action\":\"Update\",\"Key\":\"MTIzNA==\",\"Fields\":{\"BID\":118.734375}}\
		]}}");

	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslPostMsg is correct. */
	EXPECT_EQ(RSSL_MC_POST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);

	/* Check Map. */
	rsslClearMap(&map);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);

	ASSERT_EQ(RSSL_DT_BUFFER, map.keyPrimitiveType);

	EXPECT_FALSE(rsslMapCheckHasSummaryData(&map));
	EXPECT_FALSE(rsslMapCheckHasTotalCountHint(&map));
	EXPECT_FALSE(rsslMapCheckHasKeyFieldId(&map));

	EXPECT_TRUE(rsslMapCheckHasPerEntryPermData(&map));

	/* Check MapEntry. */
	rsslClearMapEntry(&mapEntry);
	rsslClearBuffer(&mapEntryKey);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_TRUE(rsslBufferIsEqual(&ENTRY_KEY_1234567, &mapEntryKey));

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData));

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check 1-st MapEntry. */
	rsslClearMapEntry(&mapEntry);
	rsslClearBuffer(&mapEntryKey);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_TRUE(rsslBufferIsEqual(&ENTRY_KEY_1235, &mapEntryKey));

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_DELETE_ENTRY, mapEntry.action);


	/* Check 2-nd MapEntry. */
	rsslClearMapEntry(&mapEntry);
	rsslClearBuffer(&mapEntryKey);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_TRUE(rsslBufferIsEqual(&ENTRY_KEY_1234, &mapEntryKey));

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_UPDATE_ENTRY, mapEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, NULL));
}

TEST_F(MapTests, OrderKeyUIntPermDataMapTest)
{
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslUInt mapEntryKey;

	const RsslUInt UIENTRY_KEY1 = 75932212;
	const RsslUInt UIENTRY_KEY2 = 3388123;
	const RsslUInt UIENTRY_KEY3 = 904183;


	setJsonBufferToString(
		"{\"ID\":5,\"Type\":\"Post\",\"Key\":{\"Service\":\"DUCK_FEED\",\"Name\":\"TINY\"},\"Complete\":false,\
		\"Map\":{\"KeyType\":\"UInt\",\"Entries\":[\
		{\"Action\":\"Add\",\"PermData\":\"UGVyTWlzc2lvbg==\",\"Key\":75932212,\"Fields\":{\"BID\":118.734375}},\
		{\"Action\":\"Add\",\"Key\":3388123,\"PermData\":\"UGVyTWlzc2lvbg==\",\"Fields\":{\"BID\":118.734375}},\
		{\"PermData\":\"UGVyTWlzc2lvbg==\",\"Action\":\"Add\",\"Key\":904183,\"Fields\":{\"BID\":118.734375}}\
		]}}");

	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslPostMsg is correct. */
	EXPECT_EQ(RSSL_MC_POST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);

	/* Check Map. */
	rsslClearMap(&map);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);

	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	EXPECT_FALSE(rsslMapCheckHasSummaryData(&map));
	EXPECT_FALSE(rsslMapCheckHasTotalCountHint(&map));
	EXPECT_FALSE(rsslMapCheckHasKeyFieldId(&map));

	EXPECT_TRUE(rsslMapCheckHasPerEntryPermData(&map));

	/* Check MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(UIENTRY_KEY1, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData));

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check 1-st MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(UIENTRY_KEY2, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);


	/* Check 2-nd MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(UIENTRY_KEY3, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, NULL));
}

TEST_F(MapTests, OrderKeyDoublePermDataMapTest)
{
	RsslPostMsg postMsg = RSSL_INIT_POST_MSG;
	RsslMsg rsslMsg;
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslDouble mapEntryKey;

	const RsslDouble DENTRY_KEY1 = 3627864.567;
	const RsslDouble DENTRY_KEY2 = -643864386.0053;
	const RsslDouble DENTRY_KEY3 = 9731249.2;


	setJsonBufferToString(
		"{\"ID\":5,\"Type\":\"Post\",\"Key\":{\"Service\":\"DUCK_FEED\",\"Name\":\"TINY\"},\"Complete\":false,\
		\"Map\":{\"KeyType\":\"Double\",\"Entries\":[\
		{\"Action\":\"Add\",\"PermData\":\"UGVyTWlzc2lvbg==\",\"Key\":3627864.567,\"Fields\":{\"BID\":118.734375}},\
		{\"Action\":\"Add\",\"Key\":-643864386.0053,\"PermData\":\"UGVyTWlzc2lvbg==\",\"Fields\":{\"BID\":118.734375}},\
		{\"PermData\":\"UGVyTWlzc2lvbg==\",\"Action\":\"Add\",\"Key\":9731249.2,\"Fields\":{\"BID\":118.734375}}\
		]}}");

	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslPostMsg is correct. */
	EXPECT_EQ(RSSL_MC_POST, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_MAP, rsslMsg.msgBase.containerType);

	/* Check Map. */
	rsslClearMap(&map);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);

	ASSERT_EQ(RSSL_DT_DOUBLE, map.keyPrimitiveType);

	EXPECT_FALSE(rsslMapCheckHasSummaryData(&map));
	EXPECT_FALSE(rsslMapCheckHasTotalCountHint(&map));
	EXPECT_FALSE(rsslMapCheckHasKeyFieldId(&map));

	EXPECT_TRUE(rsslMapCheckHasPerEntryPermData(&map));

	/* Check MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0.;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(DENTRY_KEY1, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData));

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check 1-st MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0.;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(DENTRY_KEY2, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);


	/* Check 2-nd MapEntry. */
	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 0.;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(DENTRY_KEY3, mapEntryKey);

	ASSERT_TRUE(rsslMapEntryCheckHasPermData(&mapEntry));
	EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &mapEntry.permData)) << "permData: " << mapEntry.permData.data;

	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, NULL));
}
