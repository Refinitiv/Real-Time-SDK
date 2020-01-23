#include "rsslJsonConverterTestBase.h"
#include <cstdarg>

using namespace std;
using namespace rapidjson; 

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
			out << " " << rsslMapEntryActionToOmmString(params.actionArray[i]);
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
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["Map"]["Summary"]["Fields"]));
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
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, &entries[0]["PermData"]));
			}
			else
				EXPECT_FALSE(entries[0].HasMember("PermData"));
			
			ASSERT_TRUE(entries[0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&entries[0]["Fields"]));
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
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["d"]["s"], params.protocolType));
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
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, &_jsonDocument["d"]["d"][0]["p"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"][0].HasMember("p"));

			/* Entry Payload */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["d"]["d"][0]["d"], params.protocolType));
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


INSTANTIATE_TEST_CASE_P(MapTests, MapMembersTestFixture, ::testing::Values(

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
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&entries[i]["Fields"]));
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
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);
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

INSTANTIATE_TEST_CASE_P(MapTests, MapEntryActionsTestFixture, ::testing::Values(
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
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["Map"]["Summary"]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["Map"]["Summary"]["Elements"]));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Map"]["Summary"].HasMember("FilterList"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(&_jsonDocument["Map"]["Summary"]["FilterList"]));
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
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&entries[0]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&entries[0]["Elements"]));
			break;

		case RSSL_DT_FILTER_LIST:
			ASSERT_TRUE(entries[0].HasMember("FilterList"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(&entries[0]["FilterList"]));
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

INSTANTIATE_TEST_CASE_P(MapTests, MapContainerTypesTestFixture, ::testing::Values(
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
