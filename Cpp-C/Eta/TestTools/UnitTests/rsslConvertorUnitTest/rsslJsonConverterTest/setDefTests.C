/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rsslJsonConverterTestBase.h"

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json;

/* Fixture for SetDefTests that has conversion code. */
class SetDefTests : public MsgConversionTestBase
{
};

/* Parameters for SetDef tests. */
class SetDefTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	SetDefTestParams(RsslJsonProtocolType protocolType)
	{
		this->protocolType = protocolType;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const SetDefTestParams& params)
	{
		out << "["
			<< "protocolType: " << jsonProtocolTypeToString(params.protocolType)
			<< "]";
		return out;
	}
};

class SetDefParamTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<SetDefTestParams>
{
};

/* Test that converts a Map with a FieldList and setDef set from RWF to JSON, and back to RWF. */
TEST_P(SetDefParamTestFixture, MapFieldSetDefTest)
{
	SetDefTestParams const &params = GetParam();
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
	map.flags = RSSL_MPF_HAS_SET_DEFS;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));

	/* Set Defs */

	RsslFieldSetDefEntry fields[] =
	{
		{22, RSSL_DT_REAL_4RB},
		{25, RSSL_DT_REAL_4RB}
	};

	RsslLocalFieldSetDefDb fieldListSetDefDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ 0, 2, fields},
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 },
			{ RSSL_FIELD_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	/* Encode Set Definitions */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeLocalFieldSetDefDb(&_eIter, &fieldListSetDefDb));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapSetDefsComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry. */
	rsslClearMapEntry(&mapEntry);
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &MAP_ENTRY_KEY, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, &fieldListSetDefDb));

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

			/* Check MapEntry */
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Map"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Add", entries[0]["Action"].GetString());

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
			EXPECT_EQ(RSSL_DT_MAP - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

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

			/* Check SetDb. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("l"));
			ASSERT_TRUE(_jsonDocument["d"]["l"].IsObject());

			/* Check SetDef count */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["c"].IsNumber());
			EXPECT_EQ(1, _jsonDocument["d"]["l"]["c"].GetInt());

			/* Check SetDef */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["l"]["d"].Size());

			/* Check Set ID */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["l"]["d"][0]["i"].GetInt());

			/* Check Set Entry count */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["c"].IsNumber());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["c"].GetInt());

			/* Check set entries */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["s"].Size());

			/* First set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["f"].IsNumber());
			EXPECT_EQ(22, _jsonDocument["d"]["l"]["d"][0]["s"][0]["f"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_REAL_4RB, _jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].GetInt());

			/* Second set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["f"].IsNumber());
			EXPECT_EQ(25, _jsonDocument["d"]["l"]["d"][0]["s"][1]["f"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_REAL_4RB, _jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].GetInt());

			/* Map Entry */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("k"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["k"].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["k"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));

			/* Check (defined) FieldList. */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["d"][0]["d"]["s"]["i"].GetInt());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["d"][0]["d"]["s"]["d"].Size());

			/* First Field */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].HasMember("h"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0]["h"].IsNumber());
			ASSERT_EQ(RSSL_RH_FRACTION_256, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][0]["h"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].HasMember("v"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0]["v"].IsNumber());
			ASSERT_EQ(30396, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][0]["v"].GetInt());

			/* Second Field */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].HasMember("h"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1]["h"].IsNumber());
			ASSERT_EQ(RSSL_RH_EXPONENT_2, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][1]["h"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].HasMember("v"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1]["v"].IsNumber());
			ASSERT_EQ(3906, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][1]["v"].GetInt());

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
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	/* Standard JSON uses a set def */
	RsslLocalFieldSetDefDb decodeSetDb, *pDecodeSetDb = NULL;
	if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		ASSERT_TRUE(rsslMapCheckHasSetDefs(&map));
		rsslClearLocalFieldSetDefDb(&decodeSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalFieldSetDefDb(&_dIter, &decodeSetDb));
		pDecodeSetDb = &decodeSetDb;
	}

	/* Check MapEntry. */
	RsslUInt mapEntryKey;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(MAP_ENTRY_KEY, mapEntryKey);

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check MapEntry FieldList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter, pDecodeSetDb));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}


/* Test that converts a Map with an ElementList and setDef set from RWF to JSON, and back to RWF. */
TEST_P(SetDefParamTestFixture, MapElementSetDefTest)
{
	SetDefTestParams const &params = GetParam();
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
	map.containerType = RSSL_DT_ELEMENT_LIST;
	map.keyPrimitiveType = RSSL_DT_UINT;
	map.flags = RSSL_MPF_HAS_SET_DEFS;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));

	/* Set Defs */

	RsslElementSetDefEntry elements[] =
	{
		{{3, (char*)"ONE"}, RSSL_DT_INT_1},
		{{3, (char*)"TWO"}, RSSL_DT_INT_1}
	};

	RsslLocalElementSetDefDb elementListSetDefDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ 0, 2, elements},
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	/* Encode Set Definitions */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeLocalElementSetDefDb(&_eIter, &elementListSetDefDb));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapSetDefsComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry. */
	rsslClearMapEntry(&mapEntry);
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &MAP_ENTRY_KEY, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, &elementListSetDefDb));

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

			/* Check MapEntry */
			ASSERT_TRUE(_jsonDocument["Map"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Map"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Map"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Add", entries[0]["Action"].GetString());

			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));
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
			EXPECT_EQ(RSSL_DT_MAP - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check Map. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST - 128, _jsonDocument["d"]["f"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"].HasMember("k"));
			ASSERT_TRUE(_jsonDocument["d"]["k"].IsNumber());
			ASSERT_EQ(RSSL_DT_UINT, _jsonDocument["d"]["k"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			/* Check SetDb. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("l"));
			ASSERT_TRUE(_jsonDocument["d"]["l"].IsObject());

			/* Check SetDef count */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["c"].IsNumber());
			EXPECT_EQ(1, _jsonDocument["d"]["l"]["c"].GetInt());

			/* Check SetDef */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["l"]["d"].Size());

			/* Check Set ID */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["l"]["d"][0]["i"].GetInt());

			/* Check Set Entry count */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["c"].IsNumber());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["c"].GetInt());

			/* Check set entries */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["s"].Size());

			/* First set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].IsString());
			EXPECT_STREQ("ONE", _jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].GetInt());

			/* Second set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].IsString());
			EXPECT_STREQ("TWO", _jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].GetInt());

			/* Map Entry */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("k"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["k"].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["k"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));

			/* Check (defined) ElementList. */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["d"][0]["d"]["s"]["i"].GetInt());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["d"][0]["d"]["s"]["d"].Size());

			/* First Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].IsNumber());
			EXPECT_EQ(5, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].GetInt());

			/* Second Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].GetInt());
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
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	/* Check Map. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &map));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, map.containerType);
	ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

	/* Standard JSON uses a set def */
	RsslLocalElementSetDefDb decodeSetDb, *pDecodeSetDb = NULL;
	if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		ASSERT_TRUE(rsslMapCheckHasSetDefs(&map));
		rsslClearLocalElementSetDefDb(&decodeSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalElementSetDefDb(&_dIter, &decodeSetDb));
		pDecodeSetDb = &decodeSetDb;
	}

	/* Check MapEntry. */
	RsslUInt mapEntryKey;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
	EXPECT_EQ(MAP_ENTRY_KEY, mapEntryKey);

	ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);

	/* Check MapEntry ElementList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter, pDecodeSetDb));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &mapEntry, &mapEntryKey));
}

/* Test that converts a Vector with an ElementList and setDef set from RWF to JSON, and back to RWF. */
TEST_P(SetDefParamTestFixture, VectorElementSetDefTest)
{
	SetDefTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslVector vector;
	RsslVectorEntry vectorEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_VECTOR;
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

	rsslClearVector(&vector);
	vector.containerType = RSSL_DT_ELEMENT_LIST;
	vector.flags = RSSL_VTF_HAS_SET_DEFS;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));

	/* Set Defs */

	RsslElementSetDefEntry elements[] =
	{
		{{3, (char*)"ONE"}, RSSL_DT_INT_1},
		{{3, (char*)"TWO"}, RSSL_DT_INT_1}
	};

	RsslLocalElementSetDefDb elementListSetDefDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ 0, 2, elements},
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	/* Encode Set Definitions */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeLocalElementSetDefDb(&_eIter, &elementListSetDefDb));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorSetDefsComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = RSSL_VTEA_INSERT_ENTRY;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, &elementListSetDefDb));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(&_eIter, RSSL_TRUE));

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

			/* Check Vector. */
			ASSERT_TRUE(_jsonDocument.HasMember("Vector"));
			ASSERT_TRUE(_jsonDocument["Vector"].IsObject());

			/* Check vectorEntry */
			ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Vector"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Vector"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Insert", entries[0]["Action"].GetString());

			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));
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
			EXPECT_EQ(RSSL_DT_VECTOR - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check Vector. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			/* Check SetDb. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("l"));
			ASSERT_TRUE(_jsonDocument["d"]["l"].IsObject());

			/* Check SetDef count */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["c"].IsNumber());
			EXPECT_EQ(1, _jsonDocument["d"]["l"]["c"].GetInt());

			/* Check SetDef */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["l"]["d"].Size());

			/* Check Set ID */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["l"]["d"][0]["i"].GetInt());

			/* Check Set Entry count */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["c"].IsNumber());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["c"].GetInt());

			/* Check set entries */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["s"].Size());

			/* First set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].IsString());
			EXPECT_STREQ("ONE", _jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].GetInt());

			/* Second set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].IsString());
			EXPECT_STREQ("TWO", _jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].GetInt());

			/* Vector Entry */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_VTEA_INSERT_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["i"].IsNumber());
			EXPECT_EQ(0, _jsonDocument["d"]["d"][0]["i"].GetInt());
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));

			/* Check (defined) ElementList. */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["d"][0]["d"]["s"]["i"].GetInt());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["d"][0]["d"]["s"]["d"].Size());

			/* First Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].IsNumber());
			EXPECT_EQ(5, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][0].GetInt());

			/* Second Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["d"]["s"]["d"][1].GetInt());
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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, vector.containerType);

	/* Standard JSON uses a set def */
	RsslLocalElementSetDefDb decodeSetDb, *pDecodeSetDb = NULL;
	if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		ASSERT_TRUE(rsslVectorCheckHasSetDefs(&vector));
		rsslClearLocalElementSetDefDb(&decodeSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalElementSetDefDb(&_dIter, &decodeSetDb));
		pDecodeSetDb = &decodeSetDb;
	}

	/* Check vectorEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));

	ASSERT_EQ(RSSL_VTEA_INSERT_ENTRY, vectorEntry.action);

	/* Check vectorEntry ElementList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter, pDecodeSetDb));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}

/* Test that converts a Series with a ElementList and SetDefs set from RWF to JSON, and back to RWF. */
TEST_P(SetDefParamTestFixture, SeriesElementSetDefTest)
{
	SetDefTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslSeries series;
	RsslSeriesEntry seriesEntry;

	const RsslUInt32 TOTAL_COUNT_HINT = 5;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_SERIES;
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

	rsslClearSeries(&series);
	series.containerType = RSSL_DT_ELEMENT_LIST;
	series.flags = RSSL_SRF_HAS_SET_DEFS;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(&_eIter, &series, 0, 0));

	/* Set Defs */

	RsslElementSetDefEntry elements[] =
	{
		{{3, (char*)"ONE"}, RSSL_DT_INT_1},
		{{3, (char*)"TWO"}, RSSL_DT_INT_1}
	};

	RsslLocalElementSetDefDb elementListSetDefDb =
	{
		{
			/* {setId, entry count, entries array} */
			{ 0, 2, elements},
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 },
			{ RSSL_ELEMENT_SET_BLANK_ID, 0, 0 }
		},
		{0, 0}
	};

	/* Encode Set Definitions */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeLocalElementSetDefDb(&_eIter, &elementListSetDefDb));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesSetDefsComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry. */
	rsslClearSeriesEntry(&seriesEntry);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryInit(&_eIter, &seriesEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, &elementListSetDefDb));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesComplete(&_eIter, RSSL_TRUE));

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

			/* Check Series. */
			ASSERT_TRUE(_jsonDocument.HasMember("Series"));
			ASSERT_TRUE(_jsonDocument["Series"].IsObject());

			/* Check SeriesEntry */
			ASSERT_TRUE(_jsonDocument["Series"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Series"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Series"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));
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
			EXPECT_EQ(RSSL_DT_SERIES - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check Series. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			/* Check SetDb. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("l"));
			ASSERT_TRUE(_jsonDocument["d"]["l"].IsObject());

			/* Check SetDef count */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["c"].IsNumber());
			EXPECT_EQ(1, _jsonDocument["d"]["l"]["c"].GetInt());

			/* Check SetDef */
			ASSERT_TRUE(_jsonDocument["d"]["l"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["l"]["d"].Size());

			/* Check Set ID */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["l"]["d"][0]["i"].GetInt());

			/* Check Set Entry count */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("c"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["c"].IsNumber());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["c"].GetInt());

			/* Check set entries */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["l"]["d"][0]["s"].Size());

			/* First set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].IsString());
			EXPECT_STREQ("ONE", _jsonDocument["d"]["l"]["d"][0]["s"][0]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][0]["t"].GetInt());

			/* Second set entry */
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].IsObject());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("n"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].IsString());
			EXPECT_STREQ("TWO", _jsonDocument["d"]["l"]["d"][0]["s"][1]["n"].GetString());
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1].HasMember("t"));
			ASSERT_TRUE(_jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT_1, _jsonDocument["d"]["l"]["d"][0]["s"][1]["t"].GetInt());

			/* Series Entry */

			/* Check (defined) ElementList. */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("s"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"]["i"].IsNumber());
			ASSERT_EQ(0, _jsonDocument["d"]["d"][0]["s"]["i"].GetInt());

			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"]["d"].IsArray());
			ASSERT_EQ(2, _jsonDocument["d"]["d"][0]["s"]["d"].Size());

			/* First Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"]["d"][0].IsNumber());
			EXPECT_EQ(5, _jsonDocument["d"]["d"][0]["s"]["d"][0].GetInt());

			/* Second Element */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["s"]["d"][1].IsNumber());
			EXPECT_EQ(6, _jsonDocument["d"]["d"][0]["s"]["d"][1].GetInt());
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
	EXPECT_EQ(RSSL_DT_SERIES, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	/* Check Series. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, series.containerType);

	/* Standard JSON uses a set def */
	RsslLocalElementSetDefDb decodeSetDb, *pDecodeSetDb = NULL;
	if (params.protocolType == RSSL_JSON_JPT_JSON)
	{
		ASSERT_TRUE(rsslSeriesCheckHasSetDefs(&series));
		rsslClearLocalElementSetDefDb(&decodeSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalElementSetDefDb(&_dIter, &decodeSetDb));
		pDecodeSetDb = &decodeSetDb;
	}

	/* Check SeriesEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));

	/* Check SeriesEntry ElementList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter, pDecodeSetDb));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
}
INSTANTIATE_TEST_SUITE_P(SetDefTests, SetDefParamTestFixture, ::testing::Values(
	SetDefTestParams(RSSL_JSON_JPT_JSON2),
	SetDefTestParams(RSSL_JSON_JPT_JSON)
));

TEST_F(SetDefTests, Json1SetDefOrderTest)
{
	/* Test some cases of ordering with JSON1 set definitions and dat */
	RsslMsg rsslMsg;
	RsslSeries series;
	RsslSeriesEntry seriesEntry;

	const char *seriesWithElementListMsgs [2] = {
		/* SetDef after data */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":10},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"f\":5,\"d\":[{\"s\":{\"i\":0,\"d\":[5,6]},\"d\":[]}],\"l\":{\"c\":1,\"d\":[{\"i\":0,\"c\":2,\"s\":[{\"n\":\"ONE\",\"t\":64},{\"n\":\"TWO\",\"t\":64}]}]}}}",

		/* Defined data with no standard data */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":10},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"f\":5,\"l\":{\"c\":1,\"d\":[{\"i\":0,\"c\":2,\"s\":[{\"n\":\"ONE\",\"t\":64},{\"n\":\"TWO\",\"t\":64}]}]},\"d\":[{\"s\":{\"i\":0,\"d\":[5,6]}}]}}"
	};

	for (int i = 0 ; i < 2; ++i)
	{
		setJsonBufferToString(seriesWithElementListMsgs[i]);
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(RSSL_JSON_JPT_JSON));

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Verify that RsslUpdateMsg is correct. */
		EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
		EXPECT_EQ(5, rsslMsg.msgBase.streamId);
		EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
		EXPECT_EQ(RSSL_DT_SERIES, rsslMsg.msgBase.containerType);
		EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

		/* Check Series. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
		ASSERT_EQ(RSSL_DT_ELEMENT_LIST, series.containerType);

		/* Standard JSON uses a set def */
		RsslLocalElementSetDefDb elementSetDb;
		ASSERT_TRUE(rsslSeriesCheckHasSetDefs(&series));
		rsslClearLocalElementSetDefDb(&elementSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalElementSetDefDb(&_dIter, &elementSetDb));

		/* Check SeriesEntry. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));

		/* Check SeriesEntry ElementList. */
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON, &_dIter, &elementSetDb));

		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
	}

	const char *seriesWithFieldListMsgs [2] = {
		/* SetDef after data */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":10},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"f\":4,\"d\":[{\"s\":{\"i\":0,\"d\":[{\"h\":30,\"v\":30396},{\"h\":12,\"v\":3906}]},\"d\":{}}],\"l\":{\"c\":1,\"d\":[{\"i\":0,\"c\":2,\"s\":[{\"f\":22,\"t\":74},{\"f\":25,\"t\":74}]}]}}}",

		/* Defined data with no standard data */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":10},\"u\":1,\"k\":{\"s\":555,\"n\":\"TINY\"},\"d\":{\"f\":4,\"l\":{\"c\":1,\"d\":[{\"i\":0,\"c\":2,\"s\":[{\"f\":22,\"t\":74},{\"f\":25,\"t\":74}]}]},\"d\":[{\"s\":{\"i\":0,\"d\":[{\"h\":30,\"v\":30396},{\"h\":12,\"v\":3906}]}}]}}"
	};

	for (int i = 0; i < 2; ++i)
	{
		setJsonBufferToString(seriesWithFieldListMsgs[i]);
		ASSERT_NO_FATAL_FAILURE(convertJsonToRssl(RSSL_JSON_JPT_JSON));

		/* Decode the message. */
		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		/* Verify that RsslUpdateMsg is correct. */
		EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
		EXPECT_EQ(5, rsslMsg.msgBase.streamId);
		EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
		EXPECT_EQ(RSSL_DT_SERIES, rsslMsg.msgBase.containerType);
		EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

		/* Check Series. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
		ASSERT_EQ(RSSL_DT_FIELD_LIST, series.containerType);

		/* Standard JSON uses a set def */
		RsslLocalFieldSetDefDb fieldSetDb;
		ASSERT_TRUE(rsslSeriesCheckHasSetDefs(&series));
		rsslClearLocalFieldSetDefDb(&fieldSetDb);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeLocalFieldSetDefDb(&_dIter, &fieldSetDb));

		/* Check SeriesEntry. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));

		/* Check SeriesEntry FieldList. */
		/* This is */
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON, &_dIter, &fieldSetDb));

		ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
	}
}

