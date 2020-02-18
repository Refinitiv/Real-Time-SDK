#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace rapidjson;

/* Fixture for SeriesTests that has conversion code. */
class SeriesTests : public MsgConversionTestBase
{
};

/* Parameters for Series tests. */
class SeriesMembersTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters for testing optional members. */
	bool summaryData;
	bool totalCountHint;

	SeriesMembersTestParams(RsslJsonProtocolType protocolType, bool summaryData, bool totalCountHint)
	{
		this->protocolType = protocolType;
		this->summaryData = summaryData;
		this->totalCountHint = totalCountHint;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const SeriesMembersTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			"summaryData:" << (params.summaryData ? "true" : "false") << ","
			"totalCountHint:" << (params.totalCountHint ? "true" : "false")
			<< "]";
		return out;
	}
};

class SeriesMembersTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<SeriesMembersTestParams>
{
};

/* Test that converts a Series with a ElementList from RWF to JSON, and back to RWF. */
TEST_P(SeriesMembersTestFixture, SeriesMembersTest)
{
	SeriesMembersTestParams const &params = GetParam();
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

	if (params.totalCountHint)
	{
		rsslSeriesApplyHasTotalCountHint(&series);
		series.totalCountHint = TOTAL_COUNT_HINT;
	}

	if (params.summaryData)
		rsslSeriesApplyHasSummaryData(&series);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(&_eIter, &series, 0, 0));

	if (params.summaryData)
	{
		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, NULL));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesSummaryDataComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode an entry. */
	rsslClearSeriesEntry(&seriesEntry);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryInit(&_eIter, &seriesEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter, NULL));
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

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["Series"].HasMember("Summary"));
				ASSERT_TRUE(_jsonDocument["Series"]["Summary"].HasMember("Elements"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["Series"]["Summary"]["Elements"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Series"].HasMember("Summary"));

			if (params.totalCountHint)
			{
				ASSERT_TRUE(_jsonDocument["Series"].HasMember("CountHint"));
				ASSERT_TRUE(_jsonDocument["Series"]["CountHint"].IsNumber());
				EXPECT_EQ(TOTAL_COUNT_HINT, _jsonDocument["Series"]["CountHint"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Series"].HasMember("CountHint"));

			/* Check SeriesEntry */
			ASSERT_TRUE(_jsonDocument["Series"].HasMember("Entries"));
			ASSERT_TRUE(_jsonDocument["Series"]["Entries"].IsArray());

			const Value& entries = _jsonDocument["Series"]["Entries"];
			ASSERT_EQ(1, entries.Size());

			ASSERT_TRUE(entries[0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&entries[0]["Elements"]));
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
			EXPECT_EQ(RSSL_DT_SERIES - 128, msgBase["f"].GetInt());

			/* Check Series ContainerType. */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("f"));
			ASSERT_TRUE(_jsonDocument["d"]["f"].IsNumber());
			EXPECT_EQ(RSSL_DT_ELEMENT_LIST - 128, _jsonDocument["d"]["f"].GetInt());

			if (params.summaryData)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["s"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["s"].HasMember("d"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["d"]["s"], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"].HasMember("s"));

			/* Series Entry */
			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(1, _jsonDocument["d"]["d"].Size());

			/* Entry Payload */
			ASSERT_TRUE(_jsonDocument["d"]["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["d"]["d"][0], params.protocolType));
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
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Series. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
	ASSERT_EQ(RSSL_DT_ELEMENT_LIST, series.containerType);

	if (params.summaryData)
	{
		ASSERT_TRUE(rsslSeriesCheckHasSummaryData(&series));
		ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));
	}
	else
		EXPECT_FALSE(rsslSeriesCheckHasSummaryData(&series));

	if (params.totalCountHint)
	{
		ASSERT_TRUE(rsslSeriesCheckHasTotalCountHint(&series));
		EXPECT_EQ(TOTAL_COUNT_HINT, series.totalCountHint);
	}
	else
		EXPECT_FALSE(rsslSeriesCheckHasTotalCountHint(&series));

	/* Check SeriesEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));

	/* Check SeriesEntry ElementList. */
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
}


INSTANTIATE_TEST_CASE_P(SeriesTests, SeriesMembersTestFixture, ::testing::Values(

	/* Test with/without SummaryData, TotalCountHint */

	/* Nothing */
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON, false, false),
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON2, false, false),

	/* SummaryData */
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON, true, false),
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON2, true, false),

	/* TotalCountHint */
	//SeriesMembersTestParams(RSSL_JSON_JPT_JSON, false, true), /* JSON1 conversion uses different name for CountHint in each direction */
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON2, false, true),

	/* Both */
	SeriesMembersTestParams(RSSL_JSON_JPT_JSON2, true, true)
));


/* Test that converts an empty Series RWF to JSON, and back to RWF. */
TEST_F(SeriesTests, SeriesEmptyDataTest)
{
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslSeries series;
	RsslSeriesEntry seriesEntry;

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

	/* Encode an empty series. */
	rsslClearSeries(&series);
	series.containerType = RSSL_DT_FIELD_LIST;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(&_eIter, &series, 0, 0));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Series. */
	ASSERT_TRUE(_jsonDocument.HasMember("Series"));
	ASSERT_TRUE(_jsonDocument["Series"].IsObject());

	/* Check Entries (should be empty) */
	ASSERT_TRUE(_jsonDocument["Series"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Series"]["Entries"].IsArray());
	EXPECT_EQ(0, _jsonDocument["Series"]["Entries"].Size());

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
	EXPECT_EQ(RSSL_DT_SERIES, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Series. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
	ASSERT_EQ(RSSL_DT_NO_DATA, series.containerType);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
}

class SeriesContainerTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<RsslDataTypeParam> {
};

/* Test that converts a Series whose entries & summary data are each of the different actions from RWF to JSON, and back to RWF. */
TEST_P(SeriesContainerTypesTestFixture, SeriesContainerTypesTest)
{
	const RsslDataTypes containerType = GetParam().dataType;
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslSeries series;
	RsslSeriesEntry seriesEntry;

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
	series.containerType = containerType;
	rsslSeriesApplyHasSummaryData(&series);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(&_eIter, &series, 0, 0));

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
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesSummaryDataComplete(&_eIter, RSSL_TRUE));


	/* Encode an entry with the given container. */
	rsslClearSeriesEntry(&seriesEntry);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryInit(&_eIter, &seriesEntry, 0));

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

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesComplete(&_eIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	/* Check message. */
	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	/* Check Series. */
	ASSERT_TRUE(_jsonDocument.HasMember("Series"));
	ASSERT_TRUE(_jsonDocument["Series"].IsObject());

	/* Check Summary Data. */
	switch (containerType)
	{
		case RSSL_DT_FIELD_LIST:
			ASSERT_TRUE(_jsonDocument["Series"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Series"]["Summary"].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["Series"]["Summary"]["Fields"]));
			break;

		case RSSL_DT_ELEMENT_LIST:
			ASSERT_TRUE(_jsonDocument["Series"].HasMember("Summary"));
			ASSERT_TRUE(_jsonDocument["Series"]["Summary"].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["Series"]["Summary"]["Elements"]));
			break;

		default:
			FAIL() << "Attempting to search for unhandled containerType " << containerType;
			break;
	}

	/* Check SeriesEntry */
	ASSERT_TRUE(_jsonDocument["Series"].HasMember("Entries"));
	ASSERT_TRUE(_jsonDocument["Series"]["Entries"].IsArray());

	const Value& entries = _jsonDocument["Series"]["Entries"];
	ASSERT_EQ(1, entries.Size());

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
	EXPECT_EQ(RSSL_DT_SERIES, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check Series. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(&_dIter, &series));
	ASSERT_EQ(containerType, series.containerType);
	ASSERT_TRUE(rsslSeriesCheckHasSummaryData(&series));

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

	/* Check SeriesEntry. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));

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

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(&_dIter, &seriesEntry));
}

INSTANTIATE_TEST_CASE_P(SeriesTests, SeriesContainerTypesTestFixture, ::testing::Values(
	RsslDataTypeParam(RSSL_DT_FIELD_LIST),
	RsslDataTypeParam(RSSL_DT_ELEMENT_LIST)
));

