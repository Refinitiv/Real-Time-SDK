/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"
#include <cstdarg>

#ifndef INSTANTIATE_TEST_SUITE_P
#define INSTANTIATE_TEST_SUITE_P INSTANTIATE_TEST_CASE_P
#endif

using namespace std;
using namespace json;

/* Fixture for VectorTests that has conversion code. */
class VectorTests : public MsgConversionTestBase
{
};

/* Parameters for Vector tests. */
class VectorTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool summaryData;
	bool totalCountHint;
	bool supportSorting;
	bool entryPermData;

	VectorTestParams(RsslJsonProtocolType protocolType, bool summaryData, bool totalCountHint, bool supportSorting, bool entryPermData)
	{
		this->protocolType = protocolType;
		this->summaryData = summaryData;
		this->totalCountHint = totalCountHint;
		this->supportSorting = supportSorting;
		this->entryPermData = entryPermData;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const VectorTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"summaryData:" << (params.summaryData ? "true" : "false") << ","
			"totalCountHint:" << (params.totalCountHint ? "true" : "false") << ","
			"supportSorting:" << (params.supportSorting ? "true" : "false") << ","
			"entryPermData:" << (params.entryPermData ? "true" : "false") 
			<< "]";
		return out;
	}
};

/* Parameters for Vector Entry Action tests. */
class VectorEntryActionsTestParams
{
public:

	RsslVectorEntryActions actionArray[32];
	RsslUInt8 actionArrayCount;

	VectorEntryActionsTestParams(int numActions, ...)
	{
		this->actionArrayCount = numActions;
		va_list arguments;

		va_start(arguments, numActions);
		for (int i = 0; i < numActions; i++)
			this->actionArray[i] = (RsslVectorEntryActions)va_arg(arguments, int);

		va_end(arguments);
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const VectorEntryActionsTestParams& params)
	{
		out << "VectorEntryAction array contains";
		for (int i = 0; i < params.actionArrayCount; i++)
			if (params.actionArray[i] != 0)
				out << " " << rsslVectorEntryActionToOmmString(params.actionArray[i]);
			else
				out << " ";
		return out;
	}
};

class VectorParamTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<VectorTestParams>
{
};

/* Test that converts a Vector with a FieldList from RWF to JSON, and back to RWF. */
TEST_P(VectorParamTestFixture, VectorMembersTest)
{
	VectorTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslVector vector;
	RsslVectorEntry vectorEntry;

	const RsslUInt32 TOTAL_COUNT_HINT = 5;

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
	vector.containerType = RSSL_DT_FIELD_LIST;

	if (params.totalCountHint)
	{
		rsslVectorApplyHasTotalCountHint(&vector);
		vector.totalCountHint = TOTAL_COUNT_HINT;
	}

	if (params.supportSorting)
		rsslVectorApplySupportsSorting(&vector);

	if (params.summaryData)
		rsslVectorApplyHasSummaryData(&vector);

	if (params.entryPermData)
		rsslVectorApplyHasPerEntryPermData(&vector);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));

	if (params.summaryData)
	{
		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorSummaryDataComplete(&_eIter, RSSL_TRUE));
	}
	
	/* Encode an entry. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = RSSL_VTEA_INSERT_ENTRY;
	vectorEntry.index = 1;

	if (params.entryPermData)
	{
		rsslVectorEntryApplyHasPermData(&vectorEntry);
		vectorEntry.permData = PERM_DATA;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
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

			if (params.supportSorting)
			{
				ASSERT_TRUE(_jsonDocument["Vector"].HasMember("SupportSorting"));
				ASSERT_TRUE(_jsonDocument["Vector"]["SupportSorting"].IsBool());
				EXPECT_TRUE(_jsonDocument["Vector"]["SupportSorting"].GetBool());
			}
			else
				EXPECT_FALSE(_jsonDocument["Vector"].HasMember("SupportSorting"));

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Summary"));
				ASSERT_TRUE(_jsonDocument["Vector"]["Summary"].HasMember("Fields"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Vector"]["Summary"]["Fields"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Vector"].HasMember("Summary"));

			if (params.totalCountHint)
			{
				ASSERT_TRUE(_jsonDocument["Vector"].HasMember("CountHint"));
				ASSERT_TRUE(_jsonDocument["Vector"]["CountHint"].IsNumber());
				EXPECT_EQ(TOTAL_COUNT_HINT, _jsonDocument["Vector"]["CountHint"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Vector"].HasMember("CountHint"));

			/* Check VectorEntry */
			ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Vector"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Vector"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Action"));
			ASSERT_TRUE(entries[0]["Action"].IsString());
			EXPECT_STREQ("Insert", entries[0]["Action"].GetString());

			ASSERT_TRUE(entries[0].HasMember("Index"));
			ASSERT_TRUE(entries[0]["Index"].IsNumber());
			EXPECT_EQ(1, entries[0]["Index"].GetInt());

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
			EXPECT_EQ(RSSL_DT_VECTOR - 128, msgBase["f"].GetInt());

			/* Check Vector ContainerType. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_FIELD_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			if (params.supportSorting)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("o"));
				ASSERT_TRUE(_jsonDocument["d"]["o"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["d"]["o"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("o"));

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["s"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["s"].HasMember("d"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"]["s"], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("s"));

			/* Vector Entry */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			/* Action */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("a"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_VTEA_INSERT_ENTRY, _jsonDocument["d"]["d"][0]["a"].GetInt());

			/* Index */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("i"));
			ASSERT_TRUE(_jsonDocument["d"]["d"][0]["i"].IsNumber());
			EXPECT_EQ(1, _jsonDocument["d"]["d"][0]["i"].GetInt());

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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, vector.containerType);

	if (params.summaryData)
	{
		ASSERT_TRUE(rsslVectorCheckHasSummaryData(&vector));
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
	}
	else
		EXPECT_FALSE(rsslVectorCheckHasSummaryData(&vector));

	if (params.totalCountHint)
	{
		ASSERT_TRUE(rsslVectorCheckHasTotalCountHint(&vector));
		EXPECT_EQ(TOTAL_COUNT_HINT, vector.totalCountHint);
	}
	else
		EXPECT_FALSE(rsslVectorCheckHasTotalCountHint(&vector));

	if (params.supportSorting)
		EXPECT_TRUE(rsslVectorCheckSupportsSorting(&vector));
	else
		EXPECT_FALSE(rsslVectorCheckSupportsSorting(&vector));


	EXPECT_EQ(params.entryPermData, rsslVectorCheckHasPerEntryPermData(&vector));

	/* Check VectorEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));

	if (params.entryPermData)
	{
		ASSERT_TRUE(rsslVectorEntryCheckHasPermData(&vectorEntry));
		EXPECT_TRUE(rsslBufferIsEqual(&PERM_DATA, &vectorEntry.permData));
	}
	else
		EXPECT_FALSE(rsslVectorEntryCheckHasPermData(&vectorEntry));

	ASSERT_EQ(RSSL_VTEA_INSERT_ENTRY, vectorEntry.action);
	ASSERT_EQ(1, vectorEntry.index);

	/* Check VectorEntry FieldList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}


INSTANTIATE_TEST_SUITE_P(VectorTests, VectorParamTestFixture, ::testing::Values(

	/* Test with/without SummaryData, TotalCountHint, KeyFieldId, Per-Entry PermData */

	/* Nothing */
	VectorTestParams(RSSL_JSON_JPT_JSON, false, false, false, false),
	VectorTestParams(RSSL_JSON_JPT_JSON2, false, false, false, false),

	/* SummaryData */
	VectorTestParams(RSSL_JSON_JPT_JSON, true, false, false, false),
	VectorTestParams(RSSL_JSON_JPT_JSON2, true, false, false, false),

	/* TotalCountHint */
	//VectorTestParams(RSSL_JSON_JPT_JSON, false, true, false, false), /* JSON1 conversion uses different name for CountHint in each direction */
	VectorTestParams(RSSL_JSON_JPT_JSON2, false, true, false, false),

	/* KeyFieldId */
	VectorTestParams(RSSL_JSON_JPT_JSON, false, false, true, false),
	VectorTestParams(RSSL_JSON_JPT_JSON2, false, false, true, false),

	/* Per-Entry PermData */
	VectorTestParams(RSSL_JSON_JPT_JSON, false, false, false, true),
	VectorTestParams(RSSL_JSON_JPT_JSON2, false, false, false, true)
));


/* Test that converts an empty Vector RWF to JSON, and back to RWF. */
TEST_F(VectorTests, VectorEmptyDataTest)
{
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

	/* Encode an empty vector. */
	rsslClearVector(&vector);
	vector.containerType = RSSL_DT_NO_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Vector. */
	ASSERT_TRUE(_jsonDocument.HasMember("Vector"));
	ASSERT_TRUE(_jsonDocument["Vector"].IsObject());


	/* Check VectorEntry */
	ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));

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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(RSSL_DT_NO_DATA, vector.containerType);
	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}

/* Test that converts a Vector whose entries are each of the different actions from RWF to JSON, and back to RWF. */
TEST_F(VectorTests, VectorEntryActionsTest)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslVector vector;
	RsslVectorEntry vectorEntry;

	const RsslUInt32 TOTAL_COUNT_HINT = 5;
	const RsslUInt32 VECTOR_INDEX_ARRAY[5] = {1,2,3,4,5};
	const RsslUInt8 VECTOR_ACTION_ARRAY[5] = { RSSL_VTEA_INSERT_ENTRY, RSSL_VTEA_UPDATE_ENTRY,
												RSSL_VTEA_SET_ENTRY, RSSL_VTEA_CLEAR_ENTRY, RSSL_VTEA_DELETE_ENTRY };

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
	vector.containerType = RSSL_DT_FIELD_LIST;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));
	
	/* Encode an entry with the INSERT action. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = VECTOR_ACTION_ARRAY[0];
	vectorEntry.index  = VECTOR_INDEX_ARRAY[0];
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry with the UPDATE action. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = VECTOR_ACTION_ARRAY[1];
	vectorEntry.index  = VECTOR_INDEX_ARRAY[1];
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry with the SET action. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = VECTOR_ACTION_ARRAY[2];
	vectorEntry.index  = VECTOR_INDEX_ARRAY[2];
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));

	/* Encode an entry with the CLEAR action. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = VECTOR_ACTION_ARRAY[3];
	vectorEntry.index  = VECTOR_INDEX_ARRAY[3];
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntry(&_eIter, &vectorEntry));

	/* Encode an entry with the DELETE action. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = VECTOR_ACTION_ARRAY[4];
	vectorEntry.index  = VECTOR_INDEX_ARRAY[4];
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntry(&_eIter, &vectorEntry));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Vector. */
	ASSERT_TRUE(_jsonDocument.HasMember("Vector"));
	ASSERT_TRUE(_jsonDocument["Vector"].IsObject());

	/* Check VectorEntries */
	ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Vector"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Vector"]["Entries"];
	ASSERT_EQ(5, entries.Size());

	/* Check Entry with the INSERT action. */
	ASSERT_TRUE(entries[0].HasMember("Index"));
	ASSERT_TRUE(entries[0]["Index"].IsNumber());
	EXPECT_EQ(VECTOR_INDEX_ARRAY[0], entries[0]["Index"].GetInt());

	ASSERT_TRUE(entries[0].HasMember("Action"));
	ASSERT_TRUE(entries[0]["Action"].IsString());
	EXPECT_STREQ("Insert", entries[0]["Action"].GetString());
	ASSERT_TRUE(entries[0].HasMember("Fields"));
	ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(entries[0]["Fields"]));

	/* Check Entry with UPDATE action. */
	ASSERT_TRUE(entries[1].HasMember("Index"));
	ASSERT_TRUE(entries[1]["Index"].IsNumber());
	EXPECT_EQ(VECTOR_INDEX_ARRAY[1], entries[1]["Index"].GetInt());

	ASSERT_TRUE(entries[1].HasMember("Action"));
	ASSERT_TRUE(entries[1]["Action"].IsString());
	EXPECT_STREQ("Update", entries[1]["Action"].GetString());
	ASSERT_TRUE(entries[1].HasMember("Fields"));
	ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(entries[1]["Fields"]));

	/* Check Entry with the SET action. */
	ASSERT_TRUE(entries[2].HasMember("Index"));
	ASSERT_TRUE(entries[2]["Index"].IsNumber());
	EXPECT_EQ(VECTOR_INDEX_ARRAY[2], entries[2]["Index"].GetInt());

	ASSERT_TRUE(entries[2].HasMember("Action"));
	ASSERT_TRUE(entries[2]["Action"].IsString());
	EXPECT_STREQ("Set", entries[2]["Action"].GetString());
	ASSERT_TRUE(entries[2].HasMember("Fields"));

	/* Check Entry with the CLEAR action. */
	ASSERT_TRUE(entries[3].HasMember("Index"));
	ASSERT_TRUE(entries[3]["Index"].IsNumber());
	EXPECT_EQ(VECTOR_INDEX_ARRAY[3], entries[3]["Index"].GetInt());

	ASSERT_TRUE(entries[3].HasMember("Action"));
	ASSERT_TRUE(entries[3]["Action"].IsString());
	EXPECT_STREQ("Clear", entries[3]["Action"].GetString());
	EXPECT_FALSE(entries[3].HasMember("Fields"));

	/* Check Entry with the DELETE action. */
	ASSERT_TRUE(entries[4].HasMember("Index"));
	ASSERT_TRUE(entries[4]["Index"].IsNumber());
	EXPECT_EQ(VECTOR_INDEX_ARRAY[4], entries[4]["Index"].GetInt());

	ASSERT_TRUE(entries[4].HasMember("Action"));
	ASSERT_TRUE(entries[4]["Action"].IsString());
	EXPECT_STREQ("Delete", entries[4]["Action"].GetString());
	EXPECT_FALSE(entries[4].HasMember("Fields"));

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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, vector.containerType);

	/* Check Entry with the INSERT action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	EXPECT_EQ(VECTOR_INDEX_ARRAY[0], vectorEntry.index);
	ASSERT_EQ(VECTOR_ACTION_ARRAY[0], vectorEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));

	/* Check Entry with the UPDATE action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	EXPECT_EQ(VECTOR_INDEX_ARRAY[1], vectorEntry.index);
	ASSERT_EQ(VECTOR_ACTION_ARRAY[1], vectorEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));

	/* Check Entry with the SET action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	EXPECT_EQ(VECTOR_INDEX_ARRAY[2], vectorEntry.index);
	ASSERT_EQ(VECTOR_ACTION_ARRAY[2], vectorEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));

	/* Check Entry with the CLEAR action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	EXPECT_EQ(VECTOR_INDEX_ARRAY[3], vectorEntry.index);
	ASSERT_EQ(VECTOR_ACTION_ARRAY[3], vectorEntry.action);

	/* Check Entry with the DELETE action. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	EXPECT_EQ(VECTOR_INDEX_ARRAY[4], vectorEntry.index);
	ASSERT_EQ(VECTOR_ACTION_ARRAY[4], vectorEntry.action);
	ASSERT_EQ(RSSL_VTEA_DELETE_ENTRY, vectorEntry.action);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}

class VectorContainerTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RsslDataTypeParam> {
};

/* Test that converts a Vector whose entries & summary data are different container types from RWF to JSON, and back to RWF. */
TEST_P(VectorContainerTypesTestFixture, VectorContainerTypesTest)
{
	const RsslDataTypes containerType = GetParam().dataType;
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslVector vector;
	RsslVectorEntry vectorEntry;

	const RsslUInt32 TOTAL_COUNT_HINT = 5;

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
	vector.containerType = containerType;
	rsslVectorApplyHasSummaryData(&vector);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));

	/* Encode summary data with the given container type. */
	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, NULL));
			break;

		default:
			FAIL() << "Attempting to encode unhandled containerType " << containerType;
			break;
	}
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorSummaryDataComplete(&_eIter, RSSL_TRUE));

	
	/* Encode an entry with the given container. */
	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = RSSL_VTEA_INSERT_ENTRY;
	vectorEntry.index  = 1;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));

	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter, NULL));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, NULL));
			break;

		default:
			FAIL() << "Attempting to encode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Vector. */
	ASSERT_TRUE(_jsonDocument.HasMember("Vector"));
	ASSERT_TRUE(_jsonDocument["Vector"].IsObject());

	/* Check Summary Data. */
	ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Summary"));
	
	switch (containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_TRUE(_jsonDocument["Vector"]["Summary"].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Vector"]["Summary"]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(_jsonDocument["Vector"]["Summary"].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["Vector"]["Summary"]["Elements"]));
			break;

		default:
			FAIL() << "Attempting to search for unhandled containerType " << containerType;
			break;
	}

	/* Check VectorEntry */
	ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Vector"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Vector"]["Entries"];
	ASSERT_EQ(1, entries.Size());

	ASSERT_TRUE(entries[0].HasMember("Action"));
	ASSERT_TRUE(entries[0]["Action"].IsString());
	EXPECT_STREQ("Insert", entries[0]["Action"].GetString());

	ASSERT_TRUE(entries[0].HasMember("Index"));
	ASSERT_TRUE(entries[0]["Index"].IsNumber());
	EXPECT_EQ(1, entries[0]["Index"].GetInt());

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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(containerType, vector.containerType);
	ASSERT_TRUE(rsslVectorCheckHasSummaryData(&vector));

	/* Check Summary Data. */
	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		default:
			FAIL() << "Attempting to decode unhandled containerType " << containerType;
			break;
	}

	/* Check VectorEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
	ASSERT_EQ(RSSL_VTEA_INSERT_ENTRY, vectorEntry.action);
	ASSERT_EQ(1, vectorEntry.index);

	switch(containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(RSSL_JSON_JPT_JSON2, &_dIter));
			break;

		default:
			FAIL() << "Attempting to decode unhandled containerType " << containerType;
			break;
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}

INSTANTIATE_TEST_SUITE_P(VectorTests, VectorContainerTypesTestFixture, ::testing::Values(
	RsslDataTypeParam(RSSL_DT_FIELD_LIST),
	RsslDataTypeParam(RSSL_DT_ELEMENT_LIST)
));

class VectorEntryActionsTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<VectorEntryActionsTestParams> {
};

/* Test that converts a Vector whose entries are each of the different actions from RWF to JSON, and back to RWF. */
TEST_P(VectorEntryActionsTestFixture, VectorEntryActionsTest)
{
	VectorEntryActionsTestParams const &params = GetParam();

	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslVector vector;
	RsslVectorEntry vectorEntry;
	RsslBool foundUnknownAction = false;

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
	vector.containerType = RSSL_DT_FIELD_LIST;
	
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(&_eIter, &vector, 0, 0));

	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Encode an entry with the action. */
		rsslClearVectorEntry(&vectorEntry);

		if (params.actionArray[i] == 0)
			foundUnknownAction = true;

		vectorEntry.action = params.actionArray[i];
		vectorEntry.index = i + 1;
		if (vectorEntry.action == RSSL_VTEA_DELETE_ENTRY || vectorEntry.action == RSSL_VTEA_CLEAR_ENTRY)
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntry(&_eIter, &vectorEntry));
		else
		{
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(&_eIter, &vectorEntry, 0));
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(&_eIter, RSSL_TRUE));
		}
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(&_eIter, RSSL_TRUE));

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

	/* Check Vector. */
	ASSERT_TRUE(_jsonDocument.HasMember("Vector"));
	ASSERT_TRUE(_jsonDocument["Vector"].IsObject());

	/* Check VectorEntries */
	ASSERT_TRUE(_jsonDocument["Vector"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Vector"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Vector"]["Entries"];
	ASSERT_EQ(params.actionArrayCount, entries.Size());

	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Check Entry with the action. */
		ASSERT_TRUE(entries[i].HasMember("Index"));
		ASSERT_TRUE(entries[i]["Index"].IsNumber());
		EXPECT_EQ(i + 1, entries[i]["Index"].GetInt());

		ASSERT_TRUE(entries[i].HasMember("Action"));
		ASSERT_TRUE(entries[i]["Action"].IsString());
		EXPECT_STREQ(rsslVectorEntryActionToOmmString(params.actionArray[i]), entries[i]["Action"].GetString());
		if ( (strncmp(entries[i]["Action"].GetString(), RSSL_OMMSTR_VTEA_DELETE_ENTRY.data, RSSL_OMMSTR_VTEA_DELETE_ENTRY.length) != 0) &&
			(strncmp(entries[i]["Action"].GetString(), RSSL_OMMSTR_VTEA_CLEAR_ENTRY.data, RSSL_OMMSTR_VTEA_CLEAR_ENTRY.length) != 0) )
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
	EXPECT_EQ(RSSL_DT_VECTOR, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Vector. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(&_dIter, &vector));
	ASSERT_EQ(params.actionArrayCount == 3 ? RSSL_DT_FIELD_LIST : RSSL_DT_NO_DATA, vector.containerType);

	/* Check VectorEntries. */
	for (int i = 0; i < params.actionArrayCount; i++)
	{
		/* Check Entry with the action. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
		ASSERT_EQ(params.actionArray[i], vectorEntry.action);
		if ( (vectorEntry.action != RSSL_VTEA_DELETE_ENTRY) && (vectorEntry.action != RSSL_VTEA_CLEAR_ENTRY) )
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, &_dIter));
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(&_dIter, &vectorEntry));
}

INSTANTIATE_TEST_SUITE_P(VectorTests, VectorEntryActionsTestFixture, ::testing::Values(
	VectorEntryActionsTestParams(2, 0, 0),
	VectorEntryActionsTestParams(3, RSSL_VTEA_INSERT_ENTRY, RSSL_VTEA_UPDATE_ENTRY, RSSL_VTEA_DELETE_ENTRY),
	VectorEntryActionsTestParams(3, RSSL_VTEA_DELETE_ENTRY, RSSL_VTEA_UPDATE_ENTRY, RSSL_VTEA_SET_ENTRY),
	VectorEntryActionsTestParams(2, RSSL_VTEA_DELETE_ENTRY, RSSL_VTEA_DELETE_ENTRY),
	VectorEntryActionsTestParams(3, RSSL_VTEA_CLEAR_ENTRY, RSSL_VTEA_SET_ENTRY, RSSL_VTEA_UPDATE_ENTRY),
	VectorEntryActionsTestParams(2, RSSL_VTEA_CLEAR_ENTRY, RSSL_VTEA_CLEAR_ENTRY)
));

TEST_F(VectorTests, InvalidVectorTests)
{
	/* Action missing from VectorEntry. */
	setJsonBufferToString("{\"Type\": \"Generic\", \"ID\" : 2, \"Domain\" : 128, \"SeqNumber\" : 3,\"Vector\" : {\"Summary\": {\"Fields\": {\"BID\": 45.01,\"BIDSIZE\" : 18}},\"CountHint\": 2,\"Entries\" : [ { \"Index\":1, \"Fields\": {\"BID\": 55.55,\"BIDSIZE\" : 28,\"ASK\" : 55.57,\"ASKSIZE\" : 29}},{ \"Index\":2, \"Fields\": {\"BID\": 66.66,\"BIDSIZE\" : 30,\"ASK\" : 66.57,\"ASKSIZE\" : 56}}]}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Action' for 'Entries'"));

	/* Index missing from VectorEntry. */
	setJsonBufferToString("{\"Type\": \"Generic\", \"ID\" : 2, \"Domain\" : 128, \"SeqNumber\" : 3,\"Vector\" : {\"Summary\": {\"Fields\": {\"BID\": 45.01,\"BIDSIZE\" : 18}},\"CountHint\": 2,\"Entries\" : [ {\"Action\":\"Update\",\"Fields\": {\"BID\": 55.55,\"BIDSIZE\" : 28,\"ASK\" : 55.57,\"ASKSIZE\" : 29}},{ \"Index\":2, \"Action\":\"Update\",\"Fields\": {\"BID\": 66.66,\"BIDSIZE\" : 30,\"ASK\" : 66.57,\"ASKSIZE\" : 56}}]}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Index' for 'Entries'"));
}
