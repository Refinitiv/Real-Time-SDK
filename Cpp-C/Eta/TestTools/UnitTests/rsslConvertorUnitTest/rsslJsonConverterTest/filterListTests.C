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

/* Fixture for FilterListTests that has conversion code. */
class FilterListTests : public MsgConversionTestBase
{
};

/* Parameters for FilterList tests. */
class FilterListMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool totalCountHint;
	bool entryPermData;
	bool setEntryType;

	FilterListMembersTestParams(RsslJsonProtocolType protocolType, bool totalCountHint, bool entryPermData, bool setEntryType)
	{
		this->protocolType = protocolType;
		this->totalCountHint = totalCountHint;
		this->entryPermData = entryPermData;
		this->setEntryType = setEntryType;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const FilterListMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"totalCountHint:" << (params.totalCountHint ? "true" : "false") << ","
			"entryPermData:" << (params.entryPermData ? "true" : "false") << ","
			"setEntryType:" << (params.setEntryType ? "true" : "false") 
			<< "]";
		return out;
	}
};

class FilterListMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<FilterListMembersTestParams>
{
};

/* Test that converts a FilterList with an ElementList from RWF to JSON, and back to RWF. */
TEST_P(FilterListMembersTestFixture, FilterListMembersTest)
{
	FilterListMembersTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;

	const RsslUInt8 FILTER_ENTRY_ID = 7;
	const RsslUInt32 TOTAL_COUNT_HINT = 5;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FILTER_LIST;
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

	rsslClearFilterList(&filterList);

	/* If testing FilterEntry.containerType, set the FilterList container to something other than the entry's actual payload. */
	if (params.setEntryType)
		filterList.containerType = RSSL_DT_MAP;
	else
		filterList.containerType = RSSL_DT_ELEMENT_LIST;

	if (params.totalCountHint)
	{
		rsslFilterListApplyHasTotalCountHint(&filterList);
		filterList.totalCountHint = TOTAL_COUNT_HINT;
	}

	if (params.entryPermData)
		rsslFilterListApplyHasPerEntryPermData(&filterList);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(&_eIter, &filterList));

	/* Encode an entry. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = FILTER_ENTRY_ID;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;

	/* Test with/without setting FilterEntry.containerType in RWF.
	 * The converted result should be the same either way (the payload of the entry, e.g. "Elements", already defines the container type of that entry). */
	if (params.setEntryType)
	{
		rsslFilterEntryApplyHasContainerType(&filterEntry);
		filterEntry.containerType = RSSL_DT_ELEMENT_LIST;
	}

	if (params.entryPermData)
	{
		rsslFilterEntryApplyHasPermData(&filterEntry);
		filterEntry.permData = PERM_DATA;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(&_eIter, &filterEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(&_eIter, RSSL_TRUE));

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

			/* Check FilterList. */
			ASSERT_TRUE(_jsonDocument.HasMember("FilterList"));
			ASSERT_TRUE(_jsonDocument["FilterList"].IsObject());

			if (params.totalCountHint)
			{
				ASSERT_TRUE(_jsonDocument["FilterList"].HasMember("CountHint"));
				ASSERT_TRUE(_jsonDocument["FilterList"]["CountHint"].IsNumber());
				EXPECT_EQ(TOTAL_COUNT_HINT, _jsonDocument["FilterList"]["CountHint"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["FilterList"].HasMember("CountHint"));

			/* Check FilterEntry */
			ASSERT_TRUE(_jsonDocument["FilterList"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["FilterList"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["FilterList"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("ID"));
			ASSERT_TRUE(entries[0]["ID"].IsNumber());
			EXPECT_EQ(FILTER_ENTRY_ID, entries[0]["ID"].GetInt());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Set", entries[0]["Action"].GetString());

			if (params.entryPermData)
			{
				ASSERT_TRUE(entries[0].HasMember("PermData"));
				ASSERT_TRUE(entries[0]["PermData"].IsString());
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&PERM_DATA, entries[0]["PermData"]));
			}
			else
				EXPECT_FALSE(entries[0].HasMember("PermData"));

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

			/* Check ContainerType. */
			ASSERT_TRUE(msgBase.HasMember("f"));
			ASSERT_TRUE(msgBase["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_FILTER_LIST - 128, msgBase["f"].GetInt());

			/* Check FilterList ContainerType. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			if (params.setEntryType)
				EXPECT_EQ(RSSL_DT_MAP - 128, _jsonDocument["d"]["f"].GetInt());
			else
				EXPECT_EQ(RSSL_DT_ELEMENT_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			/* Filter Entry */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			/* Action */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_FTEA_SET_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());

			/* ID */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["i"].IsNumber());
			EXPECT_EQ(FILTER_ENTRY_ID, _jsonDocument["d"]["d"][0]["i"].GetInt());

			if (params.entryPermData)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("p"));
				ASSERT_TRUE(_jsonDocument["d"]["p"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["d"]["p"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("p"));

			/* Entry Payload */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["d"]["d"][0]["d"], params.protocolType));
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
	EXPECT_EQ(RSSL_DT_FILTER_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FilterList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(&_dIter, &filterList));

	if (params.protocolType == RSSL_JSON_JPT_JSON2)
		ASSERT_EQ(RSSL_DT_ELEMENT_LIST, filterList.containerType); /* JSON2 doesn't require explicit overall containerType, and will use ElementList */
	else
	{
		if (params.setEntryType)
			ASSERT_EQ(RSSL_DT_MAP, filterList.containerType);
		else
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST, filterList.containerType);
	}

	if (params.totalCountHint)
	{
		ASSERT_TRUE(rsslFilterListCheckHasTotalCountHint(&filterList));
		EXPECT_EQ(TOTAL_COUNT_HINT, filterList.totalCountHint);
	}
	else
		EXPECT_FALSE(rsslFilterListCheckHasTotalCountHint(&filterList));

	EXPECT_EQ(params.entryPermData, rsslFilterListCheckHasPerEntryPermData(&filterList));

	/* Check FilterEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &filterEntry));
	EXPECT_EQ(FILTER_ENTRY_ID, filterEntry.id);

	if (params.entryPermData)
	{
		ASSERT_TRUE(rsslFilterEntryCheckHasPermData(&filterEntry));
		EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &filterEntry.permData));
	}
	else
		EXPECT_FALSE(rsslFilterEntryCheckHasPermData(&filterEntry));

	ASSERT_EQ(RSSL_FTEA_SET_ENTRY, filterEntry.action);

	/* Check FilterEntry ElementList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(&_dIter, &filterEntry));
}


INSTANTIATE_TEST_CASE_P(FilterListTests, FilterListMembersTestFixture, ::testing::Values(

	/* Test with/without TotalCountHint, FilterEntry.ContainerType in RWF, Per-Entry PermData */

	/* Nothing */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON, false, false, false),
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, false),

	/* FilterEntry.ContainerType in RWF */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON, false, false, true),
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, false, false, true),

	/* TotalCountHint */
	//FilterListMembersTestParams(RSSL_JSON_JPT_JSON, true, false, false), /* JSON1 conversion uses different name for CountHint in each direction */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, true, false, false),

	/* Per-Entry PermData */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON, false, true, false),
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, false, true, false),

	/* TotalCountHint, Per-Entry PermData */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, true, true, false),

	/* All */
	FilterListMembersTestParams(RSSL_JSON_JPT_JSON2, true, true, true)
));


/* Test that converts an empty FilterList RWF to JSON, and back to RWF. */
TEST_F(FilterListTests, EmptyFilterListTest)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FILTER_LIST;
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

	/* Encode an empty filterList. */
	rsslClearFilterList(&filterList);
	filterList.containerType = RSSL_DT_ELEMENT_LIST;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(&_eIter, &filterList));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FilterList. */
	ASSERT_TRUE(_jsonDocument.HasMember("FilterList"));
	ASSERT_TRUE(_jsonDocument["FilterList"].IsObject());

	/* Check FilterEntry */
	ASSERT_TRUE(_jsonDocument["FilterList"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["FilterList"]["Entries"].IsArray());
	EXPECT_EQ(0, _jsonDocument["FilterList"]["Entries"].Size());

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
	EXPECT_EQ(RSSL_DT_FILTER_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FilterList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(&_dIter, &filterList));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, filterList.containerType);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(&_dIter, &filterEntry));
}

/* Test that converts a FilterList whose entries are each of the different actions from RWF to JSON, and back to RWF. */
TEST_F(FilterListTests, FilterEntryActionsTest)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;

	const RsslUInt8 FILTER_ENTRY_IDS[3] = {6,7,8};

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FILTER_LIST;
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

	rsslClearFilterList(&filterList);
	filterList.containerType = RSSL_DT_ELEMENT_LIST;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(&_eIter, &filterList));
	
	/* Encode an entry with the SET action. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = FILTER_ENTRY_IDS[0];
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(&_eIter, &filterEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry with the UPDATE action. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = FILTER_ENTRY_IDS[1];
	filterEntry.action = RSSL_FTEA_UPDATE_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(&_eIter, &filterEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry with the CLEAR action. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = FILTER_ENTRY_IDS[2];
	filterEntry.action = RSSL_FTEA_CLEAR_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntry(&_eIter, &filterEntry));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FilterList. */
	ASSERT_TRUE(_jsonDocument.HasMember("FilterList"));
	ASSERT_TRUE(_jsonDocument["FilterList"].IsObject());

	/* Check FilterEntries */
	ASSERT_TRUE(_jsonDocument["FilterList"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["FilterList"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["FilterList"]["Entries"];
	ASSERT_EQ(3, entries.Size());

	/* Check Entry with the SET action. */
	ASSERT_TRUE(entries[0].HasMember("ID"));
	ASSERT_TRUE(entries[0]["ID"].IsNumber());
	EXPECT_EQ(FILTER_ENTRY_IDS[0], entries[0]["ID"].GetInt());

	ASSERT_TRUE(entries[0].HasMember("Action"));
	ASSERT_TRUE(entries[0]["Action"].IsString());
	EXPECT_STREQ("Set", entries[0]["Action"].GetString());
	ASSERT_TRUE(entries[0].HasMember("Elements"));
	ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));

	/* Check Entry with the UPDATE action. */
	ASSERT_TRUE(entries[1].HasMember("ID"));
	ASSERT_TRUE(entries[1]["ID"].IsNumber());
	EXPECT_EQ(FILTER_ENTRY_IDS[1], entries[1]["ID"].GetInt());

	ASSERT_TRUE(entries[1].HasMember("Action"));
	ASSERT_TRUE(entries[1]["Action"].IsString());
	EXPECT_STREQ("Update", entries[1]["Action"].GetString());
	ASSERT_TRUE(entries[1].HasMember("Elements"));
	ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[1]["Elements"]));

	/* Check Entry with the CLEAR action. */
	ASSERT_TRUE(entries[2].HasMember("ID"));
	ASSERT_TRUE(entries[2]["ID"].IsNumber());
	EXPECT_EQ(FILTER_ENTRY_IDS[2], entries[2]["ID"].GetInt());

	ASSERT_TRUE(entries[2].HasMember("Action"));
	ASSERT_TRUE(entries[2]["Action"].IsString());
	EXPECT_STREQ("Clear", entries[2]["Action"].GetString());
	EXPECT_FALSE(entries[2].HasMember("Fields"));

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
	EXPECT_EQ(RSSL_DT_FILTER_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FilterList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(&_dIter, &filterList));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, filterList.containerType);

	/* Check FilterEntries. */

	/* Check Entry with the SET action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &filterEntry));
	EXPECT_EQ(FILTER_ENTRY_IDS[0], filterEntry.id);
	ASSERT_EQ(RSSL_FTEA_SET_ENTRY, filterEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));

	/* Check Entry with the UPDATE action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &filterEntry));
	EXPECT_EQ(FILTER_ENTRY_IDS[1], filterEntry.id);
	ASSERT_EQ(RSSL_FTEA_UPDATE_ENTRY, filterEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));

	/* Check Entry with the CLEAR action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &filterEntry));
	EXPECT_EQ(FILTER_ENTRY_IDS[2], filterEntry.id);
	ASSERT_EQ(RSSL_FTEA_CLEAR_ENTRY, filterEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(&_dIter, &filterEntry));
}

class FilterListContainerTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RsslDataTypeParam> {
};

/* Test that converts a FilterList whose entries & summary data are a given containertype from RWF to JSON, and back to RWF.  */
TEST_P(FilterListContainerTypesTestFixture, FilterListContainerTypesTest)
{
	const RsslDataTypes containerType = GetParam().dataType;
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;

	const RsslUInt FILTER_ENTRY_ID = 6;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FILTER_LIST;
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

	rsslClearFilterList(&filterList);
	filterList.containerType = containerType;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(&_eIter, &filterList));
	
	/* Encode an entry with the given container. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = FILTER_ENTRY_ID;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(&_eIter, &filterEntry, 0));

	switch(containerType)
	{
		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));
			break;
		case RSSL_DT_MAP:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslMap(&_eIter));
			break;
		default:
			FAIL() << "Attempting to encode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check FilterList. */
	ASSERT_TRUE(_jsonDocument.HasMember("FilterList"));
	ASSERT_TRUE(_jsonDocument["FilterList"].IsObject());

	/* Check FilterEntry */
	ASSERT_TRUE(_jsonDocument["FilterList"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["FilterList"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["FilterList"]["Entries"];
	ASSERT_EQ(1, entries.Size());

	ASSERT_TRUE(entries[0].HasMember("Action"));
	ASSERT_TRUE(entries[0]["Action"].IsString());
	EXPECT_STREQ("Set", entries[0]["Action"].GetString());

	switch (containerType)
	{
		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(entries[0]["Elements"]));
			break;
		case RSSL_DT_MAP:
			ASSERT_TRUE(entries[0].HasMember("Map"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonMap(entries[0]["Map"]));
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
	EXPECT_EQ(RSSL_DT_FILTER_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FilterList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(&_dIter, &filterList));

	/* Check FilterEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &filterEntry));
	EXPECT_EQ(FILTER_ENTRY_ID, filterEntry.id);
	ASSERT_EQ(RSSL_FTEA_SET_ENTRY, filterEntry.action);
	ASSERT_TRUE(rsslFilterEntryCheckHasContainerType(&filterEntry));
	ASSERT_EQ(containerType, filterEntry.containerType);

	switch(containerType)
	{
		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;
		case RSSL_DT_MAP:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslMap(RSSL_JSON_JPT_JSON2, &_dIter));
			break;
		default:
			FAIL() << "Attempting to decode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(&_dIter, &filterEntry));
}

INSTANTIATE_TEST_CASE_P(FilterListTests, FilterListContainerTypesTestFixture, ::testing::Values(
	RsslDataTypeParam(RSSL_DT_ELEMENT_LIST),
	RsslDataTypeParam(RSSL_DT_MAP)
));

