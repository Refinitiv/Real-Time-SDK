#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace rapidjson; 

/* Parameters for nested container tests. */
class NestedContainerTestParams
{
	public:

	RsslUInt8 outerContainerType;
	RsslUInt8 outerMsgClass;
	RsslUInt8 innerContainerType;
	bool encodeSummaryData;

	NestedContainerTestParams(RsslUInt8 outerContainerType, RsslUInt8 outerMsgClass, RsslUInt8 innerContainerType, bool encodeSummaryData)
	{
		this->outerMsgClass = outerMsgClass;
		this->outerContainerType = outerContainerType;
		this->innerContainerType = innerContainerType;
		this->encodeSummaryData = encodeSummaryData;
	}

	NestedContainerTestParams(RsslUInt8 outerContainerType, RsslUInt8 innerContainerType, bool encodeSummaryData)
	{
		this->outerMsgClass = RSSL_MC_UPDATE;
		this->outerContainerType = outerContainerType;
		this->innerContainerType = innerContainerType;
		this->encodeSummaryData = encodeSummaryData;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const NestedContainerTestParams& params)
	{
		out << "[Outer:" << rsslDataTypeToOmmString(params.outerContainerType) << "(" << (unsigned) params.outerContainerType
			<< "), MsgClass: " << rsslMsgClassToOmmString(params.outerMsgClass) << "(" << (unsigned)params.outerMsgClass
			<< "), Inner:" << rsslDataTypeToOmmString(params.innerContainerType) << "(" << (unsigned) params.innerContainerType 
			<< ") SummaryData: " << params.encodeSummaryData << "]";
		return out;
	}
};


static const RsslUInt MAP_ENTRY_KEY = 3;
static const RsslUInt32 VECTOR_ENTRY_INDEX = 4;
static const RsslUInt32 FILTER_ENTRY_ID = 5;

class NestedContainerTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<NestedContainerTestParams>
{

	public:

	/* Encode a container with the given type.
	 * outerContainerType specifies the container to encode.
	 * msgClass is the class of message to encode, if outerContainerType is a message.
	 * innerContainerType specifies the container to encode inside that container. */
	void encodeContainer(RsslEncodeIterator *pIter, RsslUInt8 outerContainerType, RsslUInt8 msgClass, RsslUInt8 innerContainerType, bool encodeSummaryData, bool isInnerContainer)
	{
		RsslMap map;
		RsslMapEntry mapEntry;
		RsslUInt mapEntryKey = MAP_ENTRY_KEY;
		RsslSeries series;
		RsslSeriesEntry seriesEntry;
		RsslVector vector;
		RsslVectorEntry vectorEntry;
		RsslFilterList filterList;
		RsslFilterEntry filterEntry;
		RsslBuffer buffer;
		RsslMsg msg;

		/* Encode outer container */
		switch(outerContainerType)
		{
			case RSSL_DT_MAP:
			{
				rsslClearMap(&map);
				map.containerType = innerContainerType;
				map.keyPrimitiveType = RSSL_DT_UINT;

				if (encodeSummaryData)
					map.flags |= RSSL_MPF_HAS_SUMMARY_DATA;

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(pIter, &map, 0, 0));

				if (encodeSummaryData)
				{
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));
					}
					else
						ASSERT_NO_FATAL_FAILURE(encodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, encodeSummaryData, true));

					ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapSummaryDataComplete(pIter, RSSL_TRUE));
				}
				
				rsslClearMapEntry(&mapEntry);
				mapEntry.action = RSSL_MPEA_ADD_ENTRY;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(pIter, &mapEntry, (void*)&mapEntryKey, 0));
				break;
			}

			case RSSL_DT_SERIES:
			{
				rsslClearSeries(&series);
				series.containerType = innerContainerType;

				if (encodeSummaryData)
					series.flags |= RSSL_SRF_HAS_SUMMARY_DATA;

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(pIter, &series, 0, 0));

				if (encodeSummaryData)
				{
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));
					}
					else
						ASSERT_NO_FATAL_FAILURE(encodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, encodeSummaryData, true));

					ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesSummaryDataComplete(pIter, RSSL_TRUE));
				}
				
				rsslClearSeriesEntry(&seriesEntry);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryInit(pIter, &seriesEntry, 0));
				break;
			}

			case RSSL_DT_VECTOR:
			{
				rsslClearVector(&vector);
				vector.containerType = innerContainerType;

				if (encodeSummaryData)
					vector.flags |= RSSL_VTF_HAS_SUMMARY_DATA;

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(pIter, &vector, 0, 0));

				if (encodeSummaryData)
				{
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));
					}
					else
						ASSERT_NO_FATAL_FAILURE(encodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, encodeSummaryData, true));

					ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorSummaryDataComplete(pIter, RSSL_TRUE));
				}
				
				rsslClearVectorEntry(&vectorEntry);
				vectorEntry.action = RSSL_VTEA_SET_ENTRY;
				vectorEntry.index = VECTOR_ENTRY_INDEX;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(pIter, &vectorEntry, 0));
				break;
			}

			case RSSL_DT_FILTER_LIST:
			{
				rsslClearFilterList(&filterList);
				filterList.containerType = innerContainerType;

				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(pIter, &filterList));
				
				rsslClearFilterEntry(&filterEntry);
				filterEntry.action = RSSL_FTEA_SET_ENTRY;
				filterEntry.id = FILTER_ENTRY_ID;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(pIter, &filterEntry, 0));
				break;
			}

			case RSSL_DT_OPAQUE:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeInit(pIter, &buffer));
				ASSERT_LE(OPAQUE_BUFFER.length, buffer.length);
				buffer.length = OPAQUE_BUFFER.length;
				memcpy(buffer.data, OPAQUE_BUFFER.data, OPAQUE_BUFFER.length);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeComplete(pIter, &buffer, RSSL_TRUE));
				return; /* Not encoding anything in this. */
			}

			case RSSL_DT_XML:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeInit(pIter, &buffer));
				ASSERT_LE(XML_BUFFER.length, buffer.length);
				buffer.length = XML_BUFFER.length;
				memcpy(buffer.data, XML_BUFFER.data, XML_BUFFER.length);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeComplete(pIter, &buffer, RSSL_TRUE));
				return; /* Not encoding anything in this. */
			}

			case RSSL_DT_JSON:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeInit(pIter, &buffer));
				ASSERT_LE(JSON_BUFFER.length, buffer.length);
				buffer.length = JSON_BUFFER.length;
				memcpy(buffer.data, JSON_BUFFER.data, JSON_BUFFER.length);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeNonRWFDataTypeComplete(pIter, &buffer, RSSL_TRUE));
				return; /* Not encoding anything in this. */
			}

			case RSSL_DT_MSG:
			{
				rsslClearMsg(&msg);

				msg.msgBase.msgClass = msgClass;
				msg.msgBase.streamId = 5;
				msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
				msg.msgBase.containerType = innerContainerType;

				switch (msgClass)
				{
					case RSSL_MC_REFRESH:
						msg.refreshMsg.state.streamState = RSSL_STREAM_OPEN;
						msg.refreshMsg.state.dataState = RSSL_DATA_OK;
						break;

					case RSSL_MC_STATUS:
						rsslStatusMsgApplyHasState(&msg.statusMsg);
						msg.statusMsg.state.streamState = RSSL_STREAM_OPEN;
						msg.statusMsg.state.dataState = RSSL_DATA_OK;
						break;
				}

				ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(pIter, &msg, 0));
				break;
			}

			default:
				FAIL() << "Unknown container type " << outerContainerType;
				break;
		}

		/* Encode corresponding inner container */
		if (isInnerContainer)
		{
			ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
			ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));
		}
		else
			ASSERT_NO_FATAL_FAILURE(encodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, encodeSummaryData, true));


		/* Complete encoding outer container */
		switch(outerContainerType)
		{
			case RSSL_DT_MAP:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(pIter, RSSL_TRUE));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(pIter, RSSL_TRUE));
				break;

			case RSSL_DT_SERIES:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryComplete(pIter, RSSL_TRUE));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesComplete(pIter, RSSL_TRUE));
				break;

			case RSSL_DT_VECTOR:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(pIter, RSSL_TRUE));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(pIter, RSSL_TRUE));
				break;

			case RSSL_DT_FILTER_LIST:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(pIter, RSSL_TRUE));
				break;

			case RSSL_DT_MSG:
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(pIter, RSSL_TRUE));
				break;

			default:
				FAIL() << "Unknown container type " << outerContainerType;
				break;
		}
	}

	/* Test the JSON-encoded container for correct values */
	void checkJsonContainer(Value *document, RsslUInt8 outerContainerType, RsslUInt8 msgClass, RsslUInt8 innerContainerType, bool hasSummaryData, bool isInnerContainer)
	{
		Value *innerContainer = NULL;

		/* Check outer container */
		switch(outerContainerType)
		{
			case RSSL_DT_MAP:

				ASSERT_TRUE(document->HasMember("Map"));

				if (hasSummaryData)
				{
					ASSERT_TRUE((*document)["Map"].HasMember("Summary"));
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_TRUE((*document)["Map"]["Summary"].HasMember("Fields"));
						ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&(*document)["Map"]["Summary"]["Fields"]));
					}
					else
					{
						ASSERT_NO_FATAL_FAILURE(checkJsonContainer(&(*document)["Map"]["Summary"], innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, hasSummaryData, true));
					}
				}

				innerContainer = &(*document)["Map"]["Entries"][0];
				ASSERT_TRUE(innerContainer->HasMember("Action"));
				ASSERT_TRUE((*innerContainer)["Action"].IsString());
				EXPECT_STREQ("Add", (*innerContainer)["Action"].GetString());

				ASSERT_TRUE(innerContainer->HasMember("Key"));
				ASSERT_TRUE((*innerContainer)["Key"].IsNumber());
				EXPECT_EQ(MAP_ENTRY_KEY, (*innerContainer)["Key"].GetInt());
				break;

			case RSSL_DT_SERIES:

				ASSERT_TRUE(document->HasMember("Series"));

				if (hasSummaryData)
				{
					ASSERT_TRUE((*document)["Series"].HasMember("Summary"));
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_TRUE((*document)["Series"]["Summary"].HasMember("Fields"));
						ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&(*document)["Series"]["Summary"]["Fields"]));
					}
					else
					{
						ASSERT_NO_FATAL_FAILURE(checkJsonContainer(&(*document)["Series"]["Summary"], innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, hasSummaryData, true));
					}
				}

				innerContainer = &(*document)["Series"]["Entries"][0];
				break;

			case RSSL_DT_VECTOR:

				ASSERT_TRUE(document->HasMember("Vector"));

				if (hasSummaryData)
				{
					ASSERT_TRUE((*document)["Vector"].HasMember("Summary"));
					if (isInnerContainer)
					{
						ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
						ASSERT_TRUE((*document)["Vector"]["Summary"].HasMember("Fields"));
						ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&(*document)["Vector"]["Summary"]["Fields"]));
					}
					else
					{
						ASSERT_NO_FATAL_FAILURE(checkJsonContainer(&(*document)["Vector"]["Summary"], innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, hasSummaryData, true));
					}
				}

				innerContainer = &(*document)["Vector"]["Entries"][0];
				ASSERT_TRUE(innerContainer->HasMember("Action"));
				ASSERT_TRUE((*innerContainer)["Action"].IsString());
				EXPECT_STREQ("Set", (*innerContainer)["Action"].GetString());

				ASSERT_TRUE(innerContainer->HasMember("Index"));
				ASSERT_TRUE((*innerContainer)["Index"].IsNumber());
				EXPECT_EQ(VECTOR_ENTRY_INDEX, (*innerContainer)["Index"].GetInt());
				break;

			case RSSL_DT_FILTER_LIST:

				ASSERT_TRUE(document->HasMember("FilterList"));
				ASSERT_FALSE((*document)["FilterList"].HasMember("Summary"));

				innerContainer = &(*document)["FilterList"]["Entries"][0];
				ASSERT_TRUE(innerContainer->HasMember("Action"));
				ASSERT_TRUE((*innerContainer)["Action"].IsString());
				EXPECT_STREQ("Set", (*innerContainer)["Action"].GetString());

				ASSERT_TRUE(innerContainer->HasMember("ID"));
				ASSERT_TRUE((*innerContainer)["ID"].IsNumber());
				EXPECT_EQ(FILTER_ENTRY_ID, (*innerContainer)["ID"].GetInt());
				break;

			case RSSL_DT_OPAQUE:
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(document->HasMember("Opaque"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, &(*document)["Opaque"]));
				return; /* This doesn't contain anything else */

			case RSSL_DT_XML:
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(document->HasMember("Xml"));
				ASSERT_STREQ(XML_BUFFER.data, (*document)["Xml"].GetString());
				return; /* This doesn't contain anything else */

			case RSSL_DT_JSON:
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(document->HasMember("Json"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonObject(&(*document)["Json"]));
				return; /* This doesn't contain anything else */

			case RSSL_DT_MSG:
				ASSERT_TRUE(document->HasMember("Message"));
				innerContainer = &(*document)["Message"];
				ASSERT_TRUE((*innerContainer)["Type"].IsString());
				EXPECT_STREQ(rsslMsgClassToOmmString(msgClass), (*innerContainer)["Type"].GetString());

				break;


			default:
				FAIL() << "Unknown container type " << outerContainerType;
				break;
		}

		/* Check inner container */
		if (isInnerContainer)
		{
			ASSERT_TRUE(innerContainerType == RSSL_DT_FIELD_LIST);
			ASSERT_TRUE(innerContainer->HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&(*innerContainer)["Fields"]));
		}
		else
		{
			ASSERT_NO_FATAL_FAILURE(checkJsonContainer(innerContainer, innerContainerType, RSSL_MC_UPDATE, RSSL_DT_FIELD_LIST, hasSummaryData, true));
		}
	}

	void decodeContainer(RsslDecodeIterator *pIter, RsslUInt8 outerContainerType, RsslUInt8 msgClass, RsslBuffer *pInnerBuffer, RsslUInt8 innerContainerType, bool hasSummaryData, bool isInnerContainer)
	{
		RsslMap map;
		RsslMapEntry mapEntry;
		RsslUInt decodeMapKey;
		RsslSeries series;
		RsslSeriesEntry seriesEntry;
		RsslVector vector;
		RsslVectorEntry vectorEntry;
		RsslFilterList filterList;
		RsslFilterEntry filterEntry;
		RsslMsg rsslMsg;
		RsslBuffer *pInnerEncData;

		/* Decode outer container */
		switch(outerContainerType)
		{
			case RSSL_DT_MAP:
			{
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(pIter, &map));
				ASSERT_EQ(innerContainerType, map.containerType);
				ASSERT_EQ(RSSL_DT_UINT, map.keyPrimitiveType);

				if (hasSummaryData)
				{
					ASSERT_TRUE(rsslMapCheckHasSummaryData(&map));

					if (isInnerContainer)
						ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, pIter));
					else
						ASSERT_NO_FATAL_FAILURE(decodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, &map.encSummaryData, RSSL_DT_FIELD_LIST, hasSummaryData, true));
				}
				
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(pIter, &mapEntry, (void*)&decodeMapKey));
				ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, mapEntry.action);
				ASSERT_EQ(MAP_ENTRY_KEY, decodeMapKey);
				pInnerEncData = &mapEntry.encData;
				break;
			}

			case RSSL_DT_SERIES:
			{
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(pIter, &series));
				ASSERT_EQ(innerContainerType, series.containerType);

				if (hasSummaryData)
				{
					ASSERT_TRUE(rsslSeriesCheckHasSummaryData(&series));

					if (isInnerContainer)
						ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, pIter));
					else
						ASSERT_NO_FATAL_FAILURE(decodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, &series.encSummaryData, RSSL_DT_FIELD_LIST, hasSummaryData, true));
				}
				
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(pIter, &seriesEntry));
				pInnerEncData = &seriesEntry.encData;
				break;
			}

			case RSSL_DT_VECTOR:
			{
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(pIter, &vector));
				ASSERT_EQ(innerContainerType, vector.containerType);

				if (hasSummaryData)
				{
					ASSERT_TRUE(rsslVectorCheckHasSummaryData(&vector));

					if (isInnerContainer)
						ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, pIter));
					else
						ASSERT_NO_FATAL_FAILURE(decodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, &vector.encSummaryData, RSSL_DT_FIELD_LIST, hasSummaryData, true));
				}
				
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(pIter, &vectorEntry));
				ASSERT_EQ(RSSL_MPEA_ADD_ENTRY, vectorEntry.action);
				ASSERT_EQ(VECTOR_ENTRY_INDEX, vectorEntry.index);
				pInnerEncData = &vectorEntry.encData;
				break;
			}

			case RSSL_DT_FILTER_LIST:
			{
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(pIter, &filterList));
				
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(pIter, &filterEntry));
				ASSERT_TRUE(rsslFilterEntryCheckHasContainerType(&filterEntry));
				ASSERT_EQ(innerContainerType, filterEntry.containerType);
				ASSERT_EQ(RSSL_FTEA_SET_ENTRY, filterEntry.action);
				ASSERT_EQ(FILTER_ENTRY_ID, filterEntry.id);
				pInnerEncData = &filterEntry.encData;
				break;
			}

			case RSSL_DT_OPAQUE:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(rsslBufferIsEqual(&OPAQUE_BUFFER, pInnerBuffer));
				return; /* This doesn't contain anything else */
			}

			case RSSL_DT_XML:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(rsslBufferIsEqual(&XML_BUFFER, pInnerBuffer));
				return; /* This doesn't contain anything else */
			}

			case RSSL_DT_JSON:
			{
				ASSERT_TRUE(isInnerContainer); /* Inner container only; cannot contain other containers */
				ASSERT_TRUE(rsslBufferIsEqual(&JSON_BUFFER, pInnerBuffer));
				return; /* This doesn't contain anything else */
			}

			case RSSL_DT_MSG:
			{
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(pIter, &rsslMsg));
				EXPECT_EQ(msgClass, rsslMsg.msgBase.msgClass);
				EXPECT_EQ(5, rsslMsg.msgBase.streamId);
				EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
				EXPECT_EQ(innerContainerType, rsslMsg.msgBase.containerType);
				pInnerEncData = &rsslMsg.msgBase.encDataBody;
				break;
			}

			default:
				FAIL() << "Unknown container type " << outerContainerType;
				break;
		}

		/* Decode inner container */
		if (isInnerContainer)
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(RSSL_JSON_JPT_JSON2, pIter));
		else
			ASSERT_NO_FATAL_FAILURE(decodeContainer(pIter, innerContainerType, RSSL_MC_UPDATE, pInnerEncData, RSSL_DT_FIELD_LIST, hasSummaryData, true));

		/* Complete decoding outer container */
		switch(outerContainerType)
		{
			case RSSL_DT_MAP:
			{
				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(pIter, &mapEntry, &decodeMapKey));
				break;
			}

			case RSSL_DT_SERIES:
			{
				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(pIter, &seriesEntry));
				break;
			}

			case RSSL_DT_VECTOR:
			{
				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(pIter, &vectorEntry));
				break;
			}

			case RSSL_DT_FILTER_LIST:
			{
				ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(pIter, &filterEntry));
				break;
			}

			case RSSL_DT_MSG:
				break;

			default:
				FAIL() << "Unknown container type " << outerContainerType;
				break;
		}
	}
};

TEST_P(NestedContainerTypesTestFixture, NestedContainerTypesTest)
{
	NestedContainerTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;

	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = params.outerContainerType;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;
	
	rsslClearEncodeIterator(&_eIter);
	rsslSetEncodeIteratorBuffer(&_eIter, &_rsslEncodeBuffer);
	rsslSetEncodeIteratorRWFVersion(&_eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(&_eIter, (RsslMsg*)&updateMsg, 0));

	ASSERT_NO_FATAL_FAILURE(encodeContainer(&_eIter, params.outerContainerType, params.outerMsgClass, params.innerContainerType, params.encodeSummaryData, false));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));
	_rsslEncodeBuffer.length = rsslGetEncodedBufferLength(&_eIter);

	/* Convert to JSON */
	ASSERT_NO_FATAL_FAILURE(convertRsslToJson());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Update", _jsonDocument["Type"].GetString());

	ASSERT_NO_FATAL_FAILURE(checkJsonContainer(&_jsonDocument, params.outerContainerType, params.outerMsgClass, params.innerContainerType, params.encodeSummaryData, false));

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
	EXPECT_EQ(params.outerContainerType, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	ASSERT_NO_FATAL_FAILURE(decodeContainer(&_dIter, params.outerContainerType, params.outerMsgClass, &rsslMsg.msgBase.encDataBody, params.innerContainerType, params.encodeSummaryData, false));
}

INSTANTIATE_TEST_CASE_P(MapNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_VECTOR, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_FILTER_LIST, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_OPAQUE, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_XML, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_JSON, true),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_MSG, false),
		NestedContainerTestParams(RSSL_DT_MAP, RSSL_DT_MSG, true)
));

INSTANTIATE_TEST_CASE_P(VectorNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_VECTOR, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_FILTER_LIST, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_OPAQUE, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_XML, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_JSON, true),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_MSG, false),
		NestedContainerTestParams(RSSL_DT_VECTOR, RSSL_DT_MSG, true)
));

INSTANTIATE_TEST_CASE_P(SeriesNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_VECTOR, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_FILTER_LIST, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_OPAQUE, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_XML, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_JSON, true),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_MSG, false),
		NestedContainerTestParams(RSSL_DT_SERIES, RSSL_DT_MSG, true)
));

INSTANTIATE_TEST_CASE_P(FilterListNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* FilterList cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_FILTER_LIST, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(RequestMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REQUEST, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(RefreshMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_REFRESH, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(StatusMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_STATUS, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(UpdateMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_UPDATE, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(CloseMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_CLOSE, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(AckMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_ACK, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(GenericMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_GENERIC, RSSL_DT_MSG, false)
));

INSTANTIATE_TEST_CASE_P(PostMsgNestedContainerTests, NestedContainerTypesTestFixture, ::testing::Values(
		/* Message cannot contain SummaryData, but the Vector/Series/Map inside it can. */
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_MAP, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_MAP, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_SERIES, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_SERIES, true),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_VECTOR, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_VECTOR, true),

		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_FILTER_LIST, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_OPAQUE, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_XML, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_JSON, false),
		NestedContainerTestParams(RSSL_DT_MSG, RSSL_MC_POST, RSSL_DT_MSG, false)
));

