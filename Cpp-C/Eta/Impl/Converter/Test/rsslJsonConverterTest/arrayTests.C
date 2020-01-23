#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace rapidjson; 

/* Fixture for ArrayTests that has conversion code. */
class ArrayTests : public MsgConversionTestBase
{
};

class ArrayTypesTestParams
{
	public:

	RsslJsonProtocolType protocolType;
	RsslDataTypes	primitiveType;
	int				entryCount;

	ArrayTypesTestParams(RsslJsonProtocolType protocolType, RsslDataTypes primitiveType, RsslUInt32 entryCount)
	{
		this->protocolType = protocolType;
		this->primitiveType = primitiveType;
		this->entryCount = entryCount;
	}
	

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const ArrayTypesTestParams& params)
	{
		out << "["
			"protocolType: " << jsonProtocolTypeToString(params.protocolType) << ","
			<< "type:" << rsslDataTypeToOmmString(params.primitiveType) << ", entryCount:" << params.entryCount << "]";
		return out;
	}
};

class ArrayTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<ArrayTypesTestParams>
{
};

/* Test encoding arrays with zero, one, and two entries of each primitive type. 
 * The array is encoded inside a FieldEntry in an UpdateMsg. */
TEST_P(ArrayTypesTestFixture, ArrayTypesTest)
{
	ArrayTypesTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslArray rsslArray;
	RsslBuffer rsslArrayEntry;

	/* Test only has data to encode at most two entries */
	ASSERT_LE(params.entryCount, 2);

	/* Data to use in array tests. */

	/* Int */
	RsslInt arrayInts[2] = {1, -1};

	/* UInt */
	RsslUInt arrayUInts[2] = {1, 2};

	/* Float */
	RsslFloat arrayFloats[2] = {1.1f, 1.2f};

	/* Double */
	RsslDouble arrayDoubles[2] = {2.1, 2.2};

	/* Real */
	RsslReal arrayReals[2];

	rsslClearReal(&arrayReals[0]);
	arrayReals[0].value = 4501;
	arrayReals[0].hint = RSSL_RH_EXPONENT_2;

	rsslClearReal(&arrayReals[1]);
	arrayReals[1].value = 25;
	arrayReals[1].hint = RSSL_RH_EXPONENT0;

	/* Date/Time/DateTime */
	RsslDateTime arrayDateTimes[2];

	rsslClearDateTime(&arrayDateTimes[0]);
	arrayDateTimes[0].date.month = 1;
	arrayDateTimes[0].date.day = 1;
	arrayDateTimes[0].date.year = 1885;
	arrayDateTimes[0].time.hour = 13;
	arrayDateTimes[0].time.minute = 14;
	arrayDateTimes[0].time.second = 5;

	rsslClearDateTime(&arrayDateTimes[1]);
	arrayDateTimes[1].date.month = 8;
	arrayDateTimes[1].date.day = 13;
	arrayDateTimes[1].date.year = 1893;
	arrayDateTimes[1].time.hour = 13;
	arrayDateTimes[1].time.minute = 13;
	arrayDateTimes[1].time.second = 33;

	/* Qos */
	RsslQos arrayQos[2];

	rsslClearQos(&arrayQos[0]);
	arrayQos[0].timeliness = RSSL_QOS_TIME_REALTIME;
	arrayQos[0].rate = RSSL_QOS_RATE_TICK_BY_TICK;

	rsslClearQos(&arrayQos[1]);
	arrayQos[1].timeliness = RSSL_QOS_TIME_DELAYED;
	arrayQos[1].timeInfo = 60000;
	arrayQos[1].rate = RSSL_QOS_RATE_TIME_CONFLATED;
	arrayQos[1].rateInfo = 5000;

	/* State */
	RsslState arrayStates[2];

	rsslClearState(&arrayStates[0]);
	arrayStates[0].streamState = RSSL_STREAM_CLOSED;
	arrayStates[0].dataState = RSSL_DATA_SUSPECT;
	arrayStates[0].code = RSSL_SC_ALREADY_OPEN;
	arrayStates[0].text.data = (char*)"Already Open";
	arrayStates[0].text.length = 12;

	rsslClearState(&arrayStates[1]);
	arrayStates[1].streamState = RSSL_STREAM_OPEN;
	arrayStates[1].dataState = RSSL_DATA_OK;

	/* Enum */
	RsslEnum arrayEnums[2] = {2, 3};

	/* Buffer */
	RsslBuffer arrayBuffers[2] = {{7, (char*)"Buffer1"}, {7, (char*)"Buffer2"}};

	/* AsciiString */
	RsslBuffer arrayAsciiStrings[2] = {{7, (char*)"String1"}, {7, (char*)"String2"}};

	/* Utf8String */
	char UTF8_UPTICK[] = { 0xe2, 0x87, 0xa7, 0x00 };
	char UTF8_DOWNTICK[] = { 0xe2, 0x87, 0xa9, 0x00 };
	RsslBuffer arrayUtf8Strings[2] = {{3, UTF8_UPTICK}, {3, UTF8_DOWNTICK}};

	/* RmtesString */
	/* The RMTES Strings used in this test match the corresponding UTF8 strings when converted to JSON. */
	char RMTES_UPTICK[] = { 0xde, 0x00 };
	char RMTES_DOWNTICK[] = { 0xfe, 0x00 };
	RsslBuffer arrayRmtesStrings[2] = {{1, RMTES_UPTICK}, {1, RMTES_DOWNTICK}};

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
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

	/* Encode FieldList with one field (the array) */
	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));

	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = ARRAY_FIELD.fieldId;
	fieldEntry.dataType = RSSL_DT_ARRAY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

	/* Encode the array. */
	rsslClearArray(&rsslArray);
	rsslArray.primitiveType = params.primitiveType;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &rsslArray));

	/* Encode array entries of the type specified for the test. */
	for (int i = 0; i < params.entryCount; ++i)
	{
		void *pEntryData;

		switch(params.primitiveType)
		{
			case RSSL_DT_INT: pEntryData = &arrayInts[i]; break;
			case RSSL_DT_UINT: pEntryData = &arrayUInts[i]; break;
			case RSSL_DT_FLOAT: pEntryData = &arrayFloats[i]; break;
			case RSSL_DT_DOUBLE: pEntryData = &arrayDoubles[i]; break;
			case RSSL_DT_REAL: pEntryData = &arrayReals[i]; break;
			case RSSL_DT_DATE: pEntryData = &arrayDateTimes[i].date; break;
			case RSSL_DT_TIME: pEntryData = &arrayDateTimes[i].time; break;
			case RSSL_DT_DATETIME: pEntryData = &arrayDateTimes[i]; break;
			case RSSL_DT_QOS: pEntryData = &arrayQos[i]; break;
			case RSSL_DT_STATE: pEntryData = &arrayStates[i]; break;
			case RSSL_DT_ENUM: pEntryData = &arrayEnums[i]; break;
			case RSSL_DT_BUFFER: pEntryData = &arrayBuffers[i]; break;
			case RSSL_DT_ASCII_STRING: pEntryData = &arrayAsciiStrings[i]; break;
			case RSSL_DT_UTF8_STRING: pEntryData = &arrayUtf8Strings[i]; break;
			case RSSL_DT_RMTES_STRING: pEntryData = &arrayRmtesStrings[i]; break;
			default: FAIL() << "** Attempting to test unknown primitive type " << params.primitiveType;
		}
		
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, pEntryData));
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
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

			/* Check FieldList. */
			ASSERT_TRUE(_jsonDocument.HasMember("Fields"));
			ASSERT_TRUE(_jsonDocument["Fields"].IsObject());

			ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ARRAY_FIELD.fieldName.data));
			ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data].IsObject());

			/* Check Array */
			const Value& jsonArray = _jsonDocument["Fields"][ARRAY_FIELD.fieldName.data];
			ASSERT_TRUE(jsonArray.HasMember("Type"));
			ASSERT_TRUE(jsonArray["Type"].IsString());
			ASSERT_TRUE(jsonArray.HasMember("Data"));
			ASSERT_TRUE(jsonArray["Data"].IsArray());
			ASSERT_EQ(params.entryCount, jsonArray["Data"].Size());
			
			switch(params.primitiveType)
			{
				case RSSL_DT_INT: 
					ASSERT_STREQ("Int", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_EQ(arrayInts[0], jsonArray["Data"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_EQ(arrayInts[1], jsonArray["Data"][1].GetInt());
					}
					break;

				case RSSL_DT_UINT: 
					ASSERT_STREQ("UInt", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_EQ(arrayUInts[0], jsonArray["Data"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_EQ(arrayUInts[1], jsonArray["Data"][1].GetInt());
					}
					break;

				case RSSL_DT_FLOAT: 
					ASSERT_STREQ("Float", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_NEAR(arrayFloats[0], jsonArray["Data"][0].GetDouble(), 0.1);
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_NEAR(arrayFloats[1], jsonArray["Data"][1].GetDouble(), 0.1);
					}
					break;

				case RSSL_DT_DOUBLE: 
					ASSERT_STREQ("Double", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_NEAR(arrayDoubles[0], jsonArray["Data"][0].GetDouble(), 0.1);
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_NEAR(arrayDoubles[1], jsonArray["Data"][1].GetDouble(), 0.1);
					}
					break;

				case RSSL_DT_REAL:
					ASSERT_STREQ("Real", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_NEAR(45.01, jsonArray["Data"][0].GetDouble(), 0.1);
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_NEAR(25, jsonArray["Data"][1].GetDouble(), 0.1);
					}
					break;

				case RSSL_DT_DATE:
					ASSERT_STREQ("Date", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ("1885-01-01", jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ("1893-08-13", jsonArray["Data"][1].GetString());
					}
					break;

				case RSSL_DT_TIME:
					ASSERT_STREQ("Time", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ("13:14:05", jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ("13:13:33", jsonArray["Data"][1].GetString());
					}
					break;

				case RSSL_DT_DATETIME:
					ASSERT_STREQ("DateTime", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ("1885-01-01T13:14:05", jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ("1893-08-13T13:13:33", jsonArray["Data"][1].GetString());
					}
					break;

				case RSSL_DT_QOS:
					ASSERT_STREQ("Qos", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsObject());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Timeliness"));
						ASSERT_TRUE(jsonArray["Data"][0]["Timeliness"].IsString());
						EXPECT_STREQ("Realtime", jsonArray["Data"][0]["Timeliness"].GetString());
						ASSERT_TRUE(jsonArray["Data"][0].IsObject());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Rate"));
						ASSERT_TRUE(jsonArray["Data"][0]["Rate"].IsString());
						EXPECT_STREQ("TickByTick", jsonArray["Data"][0]["Rate"].GetString());

						EXPECT_FALSE(jsonArray["Data"][0].HasMember("TimeInfo"));
						EXPECT_FALSE(jsonArray["Data"][0].HasMember("RateInfo"));
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsObject());
						ASSERT_TRUE(jsonArray["Data"][1].HasMember("Timeliness"));
						ASSERT_TRUE(jsonArray["Data"][1]["Timeliness"].IsString());
						EXPECT_STREQ("Delayed", jsonArray["Data"][1]["Timeliness"].GetString());
						ASSERT_TRUE(jsonArray["Data"][1].IsObject());
						ASSERT_TRUE(jsonArray["Data"][1].HasMember("Rate"));
						ASSERT_TRUE(jsonArray["Data"][1]["Rate"].IsString());
						EXPECT_STREQ("TimeConflated", jsonArray["Data"][1]["Rate"].GetString());

						ASSERT_TRUE(jsonArray["Data"][1].HasMember("TimeInfo"));
						ASSERT_TRUE(jsonArray["Data"][1]["TimeInfo"].IsNumber());
						EXPECT_EQ(arrayQos[1].timeInfo, jsonArray["Data"][1]["TimeInfo"].GetInt());
						ASSERT_TRUE(jsonArray["Data"][1].HasMember("RateInfo"));
						ASSERT_TRUE(jsonArray["Data"][1]["RateInfo"].IsNumber());
						EXPECT_EQ(arrayQos[1].rateInfo, jsonArray["Data"][1]["RateInfo"].GetInt());
					}
					break;

				case RSSL_DT_STATE:
					ASSERT_STREQ("State", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsObject());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Stream"));
						ASSERT_TRUE(jsonArray["Data"][0]["Stream"].IsString());
						EXPECT_STREQ("Closed", jsonArray["Data"][0]["Stream"].GetString());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Data"));
						ASSERT_TRUE(jsonArray["Data"][0]["Data"].IsString());
						EXPECT_STREQ("Suspect", jsonArray["Data"][0]["Data"].GetString());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Code"));
						ASSERT_TRUE(jsonArray["Data"][0]["Code"].IsString());
						EXPECT_STREQ("AlreadyOpen", jsonArray["Data"][0]["Code"].GetString());
						ASSERT_TRUE(jsonArray["Data"][0].HasMember("Text"));
						ASSERT_TRUE(jsonArray["Data"][0]["Text"].IsString());
						EXPECT_STREQ(arrayStates[0].text.data, jsonArray["Data"][0]["Text"].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsObject());
						ASSERT_TRUE(jsonArray["Data"][1].HasMember("Stream"));
						ASSERT_TRUE(jsonArray["Data"][1]["Stream"].IsString());
						EXPECT_STREQ("Open", jsonArray["Data"][1]["Stream"].GetString());
						ASSERT_TRUE(jsonArray["Data"][1].HasMember("Data"));
						ASSERT_TRUE(jsonArray["Data"][1]["Data"].IsString());
						EXPECT_STREQ("Ok", jsonArray["Data"][1]["Data"].GetString());
					}
					break;

				case RSSL_DT_ENUM:
					ASSERT_STREQ("Enum", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsNumber());
						EXPECT_EQ(arrayEnums[0], jsonArray["Data"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsNumber());
						EXPECT_EQ(arrayEnums[1], jsonArray["Data"][1].GetInt());
					}
					break;

				case RSSL_DT_BUFFER:
					ASSERT_STREQ("Buffer", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
						ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&arrayBuffers[0], &jsonArray["Data"][0]));

					if (params.entryCount >= 2)
						ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&arrayBuffers[1], &jsonArray["Data"][1]));
					break;

				case RSSL_DT_ASCII_STRING:
					ASSERT_STREQ("AsciiString", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ(arrayAsciiStrings[0].data, jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ(arrayAsciiStrings[1].data, jsonArray["Data"][1].GetString());
					}
					break;

				case RSSL_DT_UTF8_STRING:
					ASSERT_STREQ("Utf8String", jsonArray["Type"].GetString());

					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ(arrayUtf8Strings[0].data, jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ(arrayUtf8Strings[1].data, jsonArray["Data"][1].GetString());
					}
					break;

				case RSSL_DT_RMTES_STRING:
					ASSERT_STREQ("RmtesString", jsonArray["Type"].GetString());

					/* The RMTES Strings used in this test match the corresponding UTF8 strings when converted to JSON. */
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["Data"][0].IsString());
						EXPECT_STREQ(arrayUtf8Strings[0].data, jsonArray["Data"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["Data"][1].IsString());
						EXPECT_STREQ(arrayUtf8Strings[1].data, jsonArray["Data"][1].GetString());
					}

					break;

				default: FAIL() << "** Attempting to test unknown primitive type " << params.primitiveType;
			}
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
			EXPECT_EQ(RSSL_DT_FIELD_LIST - 128, msgBase["f"].GetInt());

			/* Check FieldList. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsObject());

			/* Check Array. */
			ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ARRAY_FIELD.fieldIdString));
			const Value& jsonArray = _jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString];
			ASSERT_TRUE(jsonArray.HasMember("t"));
			ASSERT_TRUE(jsonArray["t"].IsNumber());
			ASSERT_EQ(params.primitiveType, jsonArray["t"].GetInt());

			if (params.protocolType == RSSL_JSON_JPT_JSON && params.entryCount == 0)
			{
				/* In JSON1, if the array has no entries, no 'd' member is written. */
				EXPECT_FALSE(jsonArray.HasMember("d"));
				break;
			}

			ASSERT_TRUE(jsonArray["d"].IsArray());
			ASSERT_EQ(params.entryCount, jsonArray["d"].Size());

			switch(params.primitiveType)
			{
				case RSSL_DT_INT: 
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsNumber());
						EXPECT_EQ(arrayInts[0], jsonArray["d"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsNumber());
						EXPECT_EQ(arrayInts[1], jsonArray["d"][1].GetInt());
					}
					break;

				case RSSL_DT_UINT: 
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsNumber());
						EXPECT_EQ(arrayUInts[0], jsonArray["d"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsNumber());
						EXPECT_EQ(arrayUInts[1], jsonArray["d"][1].GetInt());
					}
					break;

				case RSSL_DT_FLOAT: 
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsNumber());
						EXPECT_NEAR(arrayFloats[0], jsonArray["d"][0].GetDouble(), 0.1);
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsNumber());
						EXPECT_NEAR(arrayFloats[1], jsonArray["d"][1].GetDouble(), 0.1);
					}
					break;

				case RSSL_DT_DOUBLE: 
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsNumber());
						EXPECT_NEAR(arrayDoubles[0], jsonArray["d"][0].GetDouble(), 0.1);
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsNumber());
						EXPECT_NEAR(arrayDoubles[1], jsonArray["d"][1].GetDouble(), 0.1);
					}
					break;

				case RSSL_DT_REAL:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsObject());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][0]["h"].IsNumber());
						EXPECT_EQ(RSSL_RH_EXPONENT_2, jsonArray["d"][0]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("v"));
						ASSERT_TRUE(jsonArray["d"][0]["v"].IsNumber());
						EXPECT_EQ(4501, jsonArray["d"][0]["v"].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsObject());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][1]["h"].IsNumber());
						EXPECT_EQ(RSSL_RH_EXPONENT0, jsonArray["d"][1]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("v"));
						ASSERT_TRUE(jsonArray["d"][1]["v"].IsNumber());
						EXPECT_EQ(25, jsonArray["d"][1]["v"].GetInt());
					}
					break;

				case RSSL_DT_DATE:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].HasMember("y"));
						ASSERT_TRUE(jsonArray["d"][0]["y"].IsNumber());
						EXPECT_EQ(1885, jsonArray["d"][0]["y"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][0]["m"].IsNumber());
						EXPECT_EQ(1, jsonArray["d"][0]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][0]["d"].IsNumber());
						EXPECT_EQ(1, jsonArray["d"][0]["d"].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].HasMember("y"));
						ASSERT_TRUE(jsonArray["d"][1]["y"].IsNumber());
						EXPECT_EQ(1893, jsonArray["d"][1]["y"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][1]["m"].IsNumber());
						EXPECT_EQ(8, jsonArray["d"][1]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][1]["d"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["d"].GetInt());
					}
					break;

				case RSSL_DT_TIME:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][0]["h"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][0]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][0]["m"].IsNumber());
						EXPECT_EQ(14, jsonArray["d"][0]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][0]["s"].IsNumber());
						EXPECT_EQ(5, jsonArray["d"][0]["s"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0].HasMember("x"));
						ASSERT_TRUE(jsonArray["d"][0]["x"].IsNumber());
						EXPECT_EQ(0, jsonArray["d"][0]["x"].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][1]["h"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][1]["m"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][1]["s"].IsNumber());
						EXPECT_EQ(33, jsonArray["d"][1]["s"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("x"));
						ASSERT_TRUE(jsonArray["d"][1]["x"].IsNumber());
						EXPECT_EQ(0, jsonArray["d"][1]["x"].GetInt());
					}
					break;

				case RSSL_DT_DATETIME:
					if (params.entryCount >= 1)
					{
						/* Date */
						ASSERT_TRUE(jsonArray["d"][0].HasMember("d"));

						ASSERT_TRUE(jsonArray["d"][0]["d"].HasMember("y"));
						ASSERT_TRUE(jsonArray["d"][0]["d"]["y"].IsNumber());
						EXPECT_EQ(1885, jsonArray["d"][0]["d"]["y"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0]["d"].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][0]["d"]["m"].IsNumber());
						EXPECT_EQ(1, jsonArray["d"][0]["d"]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0]["d"].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][0]["d"]["d"].IsNumber());
						EXPECT_EQ(1, jsonArray["d"][0]["d"]["d"].GetInt());


						/* Time */
						ASSERT_TRUE(jsonArray["d"][0].HasMember("t"));

						ASSERT_TRUE(jsonArray["d"][0]["t"].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][0]["t"]["h"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][0]["t"]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0]["t"].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][0]["t"]["m"].IsNumber());
						EXPECT_EQ(14, jsonArray["d"][0]["t"]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0]["t"].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][0]["t"]["s"].IsNumber());
						EXPECT_EQ(5, jsonArray["d"][0]["t"]["s"].GetInt());

						ASSERT_TRUE(jsonArray["d"][0]["t"].HasMember("x"));
						ASSERT_TRUE(jsonArray["d"][0]["t"]["x"].IsNumber());
						EXPECT_EQ(0, jsonArray["d"][0]["t"]["x"].GetInt());

					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].HasMember("d"));

						ASSERT_TRUE(jsonArray["d"][1]["d"].HasMember("y"));
						ASSERT_TRUE(jsonArray["d"][1]["d"]["y"].IsNumber());
						EXPECT_EQ(1893, jsonArray["d"][1]["d"]["y"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1]["d"].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][1]["d"]["m"].IsNumber());
						EXPECT_EQ(8, jsonArray["d"][1]["d"]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1]["d"].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][1]["d"]["d"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["d"]["d"].GetInt());

						/* Time */
						ASSERT_TRUE(jsonArray["d"][1].HasMember("t"));

						ASSERT_TRUE(jsonArray["d"][1]["t"].HasMember("h"));
						ASSERT_TRUE(jsonArray["d"][1]["t"]["h"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["t"]["h"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1]["t"].HasMember("m"));
						ASSERT_TRUE(jsonArray["d"][1]["t"]["m"].IsNumber());
						EXPECT_EQ(13, jsonArray["d"][1]["t"]["m"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1]["t"].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][1]["t"]["s"].IsNumber());
						EXPECT_EQ(33, jsonArray["d"][1]["t"]["s"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1]["t"].HasMember("x"));
						ASSERT_TRUE(jsonArray["d"][1]["t"]["x"].IsNumber());
						EXPECT_EQ(0, jsonArray["d"][1]["t"]["x"].GetInt());
					}
					break;

				case RSSL_DT_QOS:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsObject());
						ASSERT_TRUE(jsonArray["d"][0].HasMember("t"));
						ASSERT_TRUE(jsonArray["d"][0]["t"].IsNumber());
						EXPECT_EQ(RSSL_QOS_TIME_REALTIME, jsonArray["d"][0]["t"].GetInt());
						ASSERT_TRUE(jsonArray["d"][0].IsObject());
						ASSERT_TRUE(jsonArray["d"][0].HasMember("r"));
						ASSERT_TRUE(jsonArray["d"][0]["r"].IsNumber());
						EXPECT_EQ(RSSL_QOS_RATE_TICK_BY_TICK, jsonArray["d"][0]["r"].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsObject());
						ASSERT_TRUE(jsonArray["d"][1].HasMember("t"));
						ASSERT_TRUE(jsonArray["d"][1]["t"].IsNumber());
						EXPECT_EQ(RSSL_QOS_TIME_DELAYED, jsonArray["d"][1]["t"].GetInt());
						ASSERT_TRUE(jsonArray["d"][1].IsObject());
						ASSERT_TRUE(jsonArray["d"][1].HasMember("r"));
						ASSERT_TRUE(jsonArray["d"][1]["r"].IsNumber());
						EXPECT_EQ(RSSL_QOS_RATE_TIME_CONFLATED, jsonArray["d"][1]["r"].GetInt());

						/* Time/Rate Info */
						ASSERT_TRUE(jsonArray["d"][1].HasMember("i"));
						ASSERT_TRUE(jsonArray["d"][1]["i"].IsNumber());
						EXPECT_EQ(arrayQos[1].timeInfo, jsonArray["d"][1]["i"].GetInt());

						ASSERT_TRUE(jsonArray["d"][1].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][1]["s"].IsNumber());
						EXPECT_EQ(arrayQos[1].rateInfo, jsonArray["d"][1]["s"].GetInt());
					}
					break;

				case RSSL_DT_STATE:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsObject());
						ASSERT_TRUE(jsonArray["d"][0].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][0]["s"].IsNumber());
						EXPECT_EQ(RSSL_STREAM_CLOSED, jsonArray["d"][0]["s"].GetInt());
						ASSERT_TRUE(jsonArray["d"][0].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][0]["d"].IsNumber());
						EXPECT_EQ(RSSL_DATA_SUSPECT, jsonArray["d"][0]["d"].GetInt());
						ASSERT_TRUE(jsonArray["d"][0].HasMember("c"));
						ASSERT_TRUE(jsonArray["d"][0]["c"].IsNumber());
						EXPECT_EQ(RSSL_SC_ALREADY_OPEN, jsonArray["d"][0]["c"].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsObject());
						ASSERT_TRUE(jsonArray["d"][1].HasMember("s"));
						ASSERT_TRUE(jsonArray["d"][1]["s"].IsNumber());
						EXPECT_EQ(RSSL_STREAM_OPEN, jsonArray["d"][1]["s"].GetInt());
						ASSERT_TRUE(jsonArray["d"][1].HasMember("d"));
						ASSERT_TRUE(jsonArray["d"][1]["d"].IsNumber());
						EXPECT_EQ(RSSL_DATA_OK, jsonArray["d"][1]["d"].GetInt());
					}
					break;

				case RSSL_DT_ENUM:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsNumber());
						EXPECT_EQ(arrayEnums[0], jsonArray["d"][0].GetInt());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsNumber());
						EXPECT_EQ(arrayEnums[1], jsonArray["d"][1].GetInt());
					}
					break;

				case RSSL_DT_BUFFER:
					if (params.entryCount >= 1)
						ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&arrayBuffers[0], &jsonArray["d"][0]));

					if (params.entryCount >= 2)
						ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&arrayBuffers[1], &jsonArray["d"][1]));
					break;

				case RSSL_DT_ASCII_STRING:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsString());
						EXPECT_STREQ(arrayAsciiStrings[0].data, jsonArray["d"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsString());
						EXPECT_STREQ(arrayAsciiStrings[1].data, jsonArray["d"][1].GetString());
					}
					break;

				case RSSL_DT_UTF8_STRING:
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsString());
						EXPECT_STREQ(arrayUtf8Strings[0].data, jsonArray["d"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsString());
						EXPECT_STREQ(arrayUtf8Strings[1].data, jsonArray["d"][1].GetString());
					}
					break;

				case RSSL_DT_RMTES_STRING:

					/* The RMTES Strings used in this test match the corresponding UTF8 strings when converted to JSON. */
					if (params.entryCount >= 1)
					{
						ASSERT_TRUE(jsonArray["d"][0].IsString());
						EXPECT_STREQ(arrayUtf8Strings[0].data, jsonArray["d"][0].GetString());
					}

					if (params.entryCount >= 2)
					{
						ASSERT_TRUE(jsonArray["d"][1].IsString());
						EXPECT_STREQ(arrayUtf8Strings[1].data, jsonArray["d"][1].GetString());
					}

					break;

				default: FAIL() << "** Attempting to test unknown primitive type " << params.primitiveType;
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

	/* Verify that RsslUpdateMsg is correct. */
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	ASSERT_EQ(ARRAY_FIELD.fieldId, fieldEntry.fieldId);
	ASSERT_EQ(RSSL_DT_UNKNOWN, fieldEntry.dataType);

	/* Check Array. */

	if (params.protocolType == RSSL_JSON_JPT_JSON && params.entryCount == 0)
	{
		/* In JSON1, if the array has no entries, no 'd' member is written.
		 * Conversion to RWF therefore encodes nothing, leaving the field blank. */
		ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeArray(&_dIter, &rsslArray));
		return;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &rsslArray));
	ASSERT_EQ(params.primitiveType, rsslArray.primitiveType);

	for (int i = 0; i < params.entryCount; ++i)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &rsslArrayEntry));

		switch(params.primitiveType)
		{
			case RSSL_DT_INT: 
			{
				RsslInt decodeInt;
				EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &decodeInt));
				EXPECT_EQ(arrayInts[i], decodeInt);
				break;
			}

			case RSSL_DT_UINT: 
			{
				RsslUInt decodeUInt;
				EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeUInt(&_dIter, &decodeUInt));
				EXPECT_EQ(arrayUInts[i], decodeUInt);
				break;
			}

			case RSSL_DT_FLOAT: 
			{
				RsslFloat decodeFloat;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));
				EXPECT_NEAR(arrayFloats[i], decodeFloat, 0.099);
				break;
			}

			case RSSL_DT_DOUBLE: 
			{
				RsslDouble decodeDouble;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDouble(&_dIter, &decodeDouble));
				EXPECT_NEAR(arrayDoubles[i], decodeDouble, 0.099);
				break;
			}

			case RSSL_DT_REAL:
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(&_dIter, &decodeReal));
				EXPECT_EQ(arrayReals[i].isBlank, decodeReal.isBlank);
				EXPECT_EQ(arrayReals[i].hint, decodeReal.hint);
				EXPECT_EQ(arrayReals[i].value, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				break;
			}

			case RSSL_DT_DATE:
			{
				RsslDate decodeDate;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDate(&_dIter, &decodeDate));

				ASSERT_EQ(arrayDateTimes[i].date.day, decodeDate.day);
				ASSERT_EQ(arrayDateTimes[i].date.month, decodeDate.month);
				ASSERT_EQ(arrayDateTimes[i].date.year, decodeDate.year);
				break;
			}

			case RSSL_DT_TIME:
			{
				RsslTime decodeTime;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeTime(&_dIter, &decodeTime));

				ASSERT_EQ(arrayDateTimes[i].time.hour, decodeTime.hour);
				ASSERT_EQ(arrayDateTimes[i].time.minute, decodeTime.minute);
				ASSERT_EQ(arrayDateTimes[i].time.second, decodeTime.second);
				ASSERT_EQ(arrayDateTimes[i].time.millisecond, decodeTime.millisecond);
				ASSERT_EQ(arrayDateTimes[i].time.microsecond, decodeTime.microsecond);
				ASSERT_EQ(arrayDateTimes[i].time.nanosecond, decodeTime.nanosecond);
				break;
			}

			case RSSL_DT_DATETIME:
			{
				RsslDateTime decodeDateTime;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDateTime(&_dIter, &decodeDateTime));

				ASSERT_EQ(arrayDateTimes[i].date.day, decodeDateTime.date.day);
				ASSERT_EQ(arrayDateTimes[i].date.month, decodeDateTime.date.month);
				ASSERT_EQ(arrayDateTimes[i].date.year, decodeDateTime.date.year);
				ASSERT_EQ(arrayDateTimes[i].time.hour, decodeDateTime.time.hour);
				ASSERT_EQ(arrayDateTimes[i].time.minute, decodeDateTime.time.minute);
				ASSERT_EQ(arrayDateTimes[i].time.second, decodeDateTime.time.second);
				ASSERT_EQ(arrayDateTimes[i].time.millisecond, decodeDateTime.time.millisecond);
				ASSERT_EQ(arrayDateTimes[i].time.microsecond, decodeDateTime.time.microsecond);
				ASSERT_EQ(arrayDateTimes[i].time.nanosecond, decodeDateTime.time.nanosecond);
				break;
			}

			case RSSL_DT_QOS:
			{
				RsslQos decodeQos;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeQos(&_dIter, &decodeQos));
				EXPECT_TRUE(rsslQosIsEqual(&arrayQos[i], &decodeQos));
				break;
			}

			case RSSL_DT_STATE:
			{
				RsslState decodeState;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeState(&_dIter, &decodeState));

				ASSERT_EQ(arrayStates[i].streamState, decodeState.streamState);
				ASSERT_EQ(arrayStates[i].dataState, decodeState.dataState);
				ASSERT_EQ(arrayStates[i].code, decodeState.code);
				break;
			}

			case RSSL_DT_ENUM:
			{
				RsslEnum decodeEnum;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeEnum(&_dIter, &decodeEnum));
				EXPECT_EQ(arrayEnums[i], decodeEnum);
				break;
			}

			case RSSL_DT_BUFFER:
			{
				RsslBuffer decodeBuffer;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
				EXPECT_TRUE(rsslBufferIsEqual(&arrayBuffers[i], &decodeBuffer));
				break;
			}

			case RSSL_DT_ASCII_STRING:
			{
				RsslBuffer decodeBuffer;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
				EXPECT_TRUE(rsslBufferIsEqual(&arrayAsciiStrings[i], &decodeBuffer));
				break;
			}

			case RSSL_DT_UTF8_STRING:
			{
				RsslBuffer decodeBuffer;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
				EXPECT_TRUE(rsslBufferIsEqual(&arrayUtf8Strings[i], &decodeBuffer));
				break;
			}

			case RSSL_DT_RMTES_STRING:
			{
				RsslBuffer decodeBuffer;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
				EXPECT_TRUE(rsslBufferIsEqual(&arrayUtf8Strings[i], &decodeBuffer));
				break;
			}

			default: FAIL() << "** Attempting to test unknown primitive type " << params.primitiveType;
		}
	}

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &rsslArrayEntry));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));

}

INSTANTIATE_TEST_CASE_P(ArrayTests, ArrayTypesTestFixture, ::testing::Values(
	/* Test encoding arrays containing entries of the different primitive types, and with different entry counts */

	/* Int */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT, 2),

	/* UInt */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT, 2),

	/* Float */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT, 2),

	/* Double */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE, 2),

	/* Real */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL, 2),

	/* Date */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE, 2),

	/* Time */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME, 2),

	/* DateTime */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME, 2),

	/* QoS */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS, 2),

	/* State */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE, 2),

	/* Enum */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM, 2),

	/* Buffer */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER, 2),

	/* AsciiString */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING, 2),

	/* Utf8String */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING, 2),

	/* RmtesString */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING, 2)
));

INSTANTIATE_TEST_CASE_P(ArrayTestsJson1, ArrayTypesTestFixture, ::testing::Values(
	/* Test encoding arrays containing entries of the different primitive types, and with different entry counts */

	/* Int */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT, 2),

	/* UInt */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT, 2),

	/* Float */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT, 2),

	/* Double */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE, 2),

	/* Real */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL, 2),

	/* Date */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE, 2),

	/* Time */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME, 2),

	/* DateTime */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME, 2),

	/* QoS */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS, 2),

	/* State */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE, 2),

	/* Enum */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM, 2),

	/* Buffer */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER, 2),

	/* AsciiString */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING, 2),

	/* Utf8String */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING, 2),

	/* RmtesString */
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING, 0),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING, 1),
	ArrayTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING, 2)
));

class ArrayMemberOrderTestParams
{
	public:

	const char* jsonString;
	RsslUInt16 expectedItemLength;

	ArrayMemberOrderTestParams(const char *jsonString, RsslUInt8 expectedItemLength)
	{
		this->jsonString = jsonString;
		this->expectedItemLength = expectedItemLength;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const ArrayMemberOrderTestParams& params)
	{
		out << "[string:" << params.jsonString << ", itemLength:" << params.expectedItemLength << "]";
		return out;
	}
};

class ArrayMemberOrderTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<ArrayMemberOrderTestParams> {};

/* Test different permutations of Data, Type, & itemLength in JSON messages. */
TEST_P(ArrayMemberOrderTestFixture, ArrayMemberOrderTest)
{
	RsslMsg rsslMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslArray rsslArray;
	RsslBuffer rsslArrayEntry;
	RsslUInt decodeUInt;
	RsslBuffer elementName = {4, (char*)"TEST"};
	ArrayMemberOrderTestParams const &params = GetParam();

	/* Use a JSON message with an ElementList with an Array in it. Put the 'Data' of the Array before the 'Type'. */
	setJsonBufferToString(params.jsonString);

	/* Convert to RWF. */
	ASSERT_NO_FATAL_FAILURE(convertJsonToRssl());

	/* Decode the message. */
	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	/* Verify that RsslGenericMsg is correct. */
	EXPECT_EQ(RSSL_MC_GENERIC, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(4, rsslMsg.msgBase.streamId);
	EXPECT_EQ(131, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);

	/* Check ElementList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementEntry(&_dIter, &elementEntry));
	ASSERT_TRUE(rsslBufferIsEqual(&elementName, &elementEntry.name));
	ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);

	/* Check Array. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &rsslArray));
	ASSERT_EQ(RSSL_DT_UINT, rsslArray.primitiveType);
	ASSERT_EQ(params.expectedItemLength, rsslArray.itemLength);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &rsslArrayEntry));

	EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeUInt(&_dIter, &decodeUInt));
	EXPECT_EQ(1, decodeUInt);

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &rsslArrayEntry));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeElementEntry(&_dIter, &elementEntry));
}

/* Cases for ArrayMemberOrderTest -- specifies JSON strings that create the array, and the expected 
 * Array.itemLength in RWF.
 * The strings should all convert to the same array in RWF regardless of order (except that the itemLength
 * will be 0 in the case where it is not specified). */
INSTANTIATE_TEST_CASE_P(ArrayTests, ArrayMemberOrderTestFixture, ::testing::Values(
		/* Data, Type , no itemLength */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Data\":[1],\"Type\":\"UInt\"},\"Type\":\"Array\"}}}", 0),

		/* Type, itemLength(4), Data */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Type\":\"UInt\",\"Length\":4,\"Data\":[1]},\"Type\":\"Array\"}}}", 4),

		/* Type, Data, itemLength(4) */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Type\":\"UInt\",\"Data\":[1],\"Length\":4},\"Type\":\"Array\"}}}", 4),

		/* itemLength(4), Type, Data */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Length\":4,\"Type\":\"UInt\",\"Data\":[1]},\"Type\":\"Array\"}}}", 4),

		/* itemLength(4), Data, Type */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Length\":4,\"Data\":[1],\"Type\":\"UInt\"},\"Type\":\"Array\"}}}", 4),

		/* Data, Type, itemLength(4) */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Data\":[1],\"Type\":\"UInt\",\"Length\":4},\"Type\":\"Array\"}}}", 4),

		/* Data, itemLength(4), Type */
		ArrayMemberOrderTestParams("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Data\":[1],\"Length\":4,\"Type\":\"UInt\"},\"Type\":\"Array\"}}}", 4)
));

TEST_F(ArrayTests, InvalidArrayTests)
{
	/* Type and Data missing from Array. */
	setJsonBufferToString("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{},\"Type\":\"Array\"}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Data' for 'Array'"));

	/* Data missing from Array. */
	setJsonBufferToString("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Type\":\"UInt\"},\"Type\":\"Array\"}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Data' for 'Array'"));

	/* Type missing from Array. */
	setJsonBufferToString("{\"ID\":4,\"Type\":\"Generic\",\"Domain\":131,\"Elements\":{\"TEST\":{\"Data\":"
			"{\"Data\":[1]},\"Type\":\"Array\"}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key 'Type' for 'Array'"));
}
