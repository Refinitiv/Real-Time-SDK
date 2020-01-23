#include "rsslJsonConverterTestBase.h"

using namespace std;
using namespace rapidjson; 

/* Fixture for FieldListTests that has conversion code. */
class FieldListTests : public MsgConversionTestBase
{
};

/* Parameters for FieldList tests. */
class FieldListTypesTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters; when true, the corresponding datatype will be encoded in a field. */
	bool intValue;
	bool uintValue;
	bool floatValue;
	bool doubleValue;
	bool real;
	bool date;
	bool time;
	bool dateTime;
	bool qos;
	bool state;
	bool enumValue;
	bool array;
	bool buffer;
	bool asciiString;
	bool utf8String;
	bool rmtesString;
	bool opaque;
	bool xml;
	bool fieldList;
	bool elementList;
	bool filterList;
	bool map;
	bool vector;
	bool series;
	bool msg;
	bool json;
	bool fieldListInfo;

	int memberCount;

	/* For empty test. */
	FieldListTypesTestParams(RsslJsonProtocolType protocolType, bool fieldListInfo)
	{
		init(protocolType, 0, NULL, fieldListInfo);
	}

	/* For tests that encode only one data type. */
	FieldListTypesTestParams(RsslJsonProtocolType protocolType, RsslUInt8 dataType, bool fieldListInfo)
	{
		init(protocolType, 1, &dataType, fieldListInfo);
	}

	/* For tests that encode multiple data types. */
	FieldListTypesTestParams(RsslJsonProtocolType protocolType, int argCount, RsslUInt8 *argList, bool fieldListInfo)
	{
		init(protocolType, argCount, argList, fieldListInfo);
	}
	
	/* Set flags indicating which data types will be encoded in the test. */
	void init(RsslJsonProtocolType protocolType, int argCount, RsslUInt8 *argList, bool fieldListInfo)
	{
		/* Initialize all flags to false. */
		memset(this, 0, sizeof(FieldListTypesTestParams));

		/* Set protocol type. */
		this->protocolType = protocolType;

		this->fieldListInfo = fieldListInfo;

		/* Go over the passed-in array. Set the corresponding flag for each data type so that
		 * the test will encode it. */
		for (int i = 0; i < argCount; ++i)
		{
			switch (argList[i])
			{
				case RSSL_DT_INT:			this->intValue = true; break;
				case RSSL_DT_UINT:			this->uintValue = true; break;
				case RSSL_DT_FLOAT:			this->floatValue = true; break;
				case RSSL_DT_DOUBLE:		this->doubleValue = true; break;
				case RSSL_DT_REAL:			this->real = true; break;
				case RSSL_DT_DATE:			this->date = true; break;
				case RSSL_DT_TIME:			this->time = true; break;
				case RSSL_DT_DATETIME:		this->dateTime = true; break;
				case RSSL_DT_QOS:			this->qos = true; break;
				case RSSL_DT_STATE:			this->state = true; break;
				case RSSL_DT_ENUM:			this->enumValue = true; break;
				case RSSL_DT_ARRAY:			this->array = true; break;
				case RSSL_DT_BUFFER:		this->buffer = true; break;
				case RSSL_DT_ASCII_STRING:	this->asciiString = true; break;
				case RSSL_DT_UTF8_STRING:	this->utf8String = true; break;
				case RSSL_DT_RMTES_STRING:	this->rmtesString = true; break;
				case RSSL_DT_OPAQUE:		this->opaque = true; break;
				case RSSL_DT_XML:			this->xml = true; break;
				case RSSL_DT_FIELD_LIST:	this->fieldList = true; break;
				case RSSL_DT_ELEMENT_LIST:	this->elementList = true; break;
				case RSSL_DT_FILTER_LIST:	this->filterList = true; break;
				case RSSL_DT_MAP:			this->map = true; break;
				case RSSL_DT_VECTOR:		this->vector = true; break;
				case RSSL_DT_SERIES:		this->series = true; break;
				case RSSL_DT_MSG:			this->msg = true; break;
				case RSSL_DT_JSON:			this->json = true; break;
				default:					FAIL() << "** Unknown data type specified in field list test parameters.";
			}
		}

		memberCount = argCount;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const FieldListTypesTestParams& params)
	{
		out << "["
			<< "protocolType: " << jsonProtocolTypeToString(params.protocolType) << "," 
			<< "Types:";
		if (params.intValue) out << " int";
		if (params.uintValue) out << " uint";
		if (params.floatValue) out << " float";
		if (params.doubleValue) out << " double";
		if (params.real) out << " real";
		if (params.date) out << " date";
		if (params.time) out << " time";
		if (params.dateTime) out << " dateTime";
		if (params.qos) out << " qos";
		if (params.state) out << " state";
		if (params.enumValue) out << " enum";
		if (params.array) out << " array";
		if (params.buffer) out << " buffer";
		if (params.asciiString) out << " asciiString";
		if (params.utf8String) out << " utf8String";
		if (params.rmtesString) out << " rmtesString";
		if (params.opaque) out << " opaque";
		if (params.xml) out << " xml";
		if (params.fieldList) out << " fieldList";
		if (params.elementList) out << " elementList";
		if (params.filterList) out << " filterList";
		if (params.map) out << " map";
		if (params.vector) out << " vector";
		if (params.series) out << " series";
		if (params.msg) out << " msg";
		if (params.json) out << " json";
		if (params.protocolType == RSSL_JSON_JPT_JSON)
			out << "fieldListInfo:" << (params.fieldListInfo ? "true" : "false");
		out << "]";
		return out;
	}
};

class FieldListTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<FieldListTypesTestParams>
{
};

/* Test a field list that encodes different data types in its field entries. */
TEST_P(FieldListTypesTestFixture, FieldListTypesTest)
{
	FieldListTypesTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslArray rsslArray;

	const RsslInt rsslInt = -5;
	const RsslUInt rsslUInt = 65535;
	const RsslFloat rsslFloat = 1.0f;
	const RsslDouble rsslDouble = 2.0;
	const RsslEnum rsslEnum = 4;
	const RsslInt	arrayInts[] = {0, 1, 2};

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

	/* Encode a field list whose entries encode different data types. */
	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	if (params.fieldListInfo)
	{
		ASSERT_EQ(RSSL_JSON_JPT_JSON, params.protocolType); /* fieldListInfo is JSON1 only */
		fieldList.flags |= RSSL_FLF_HAS_FIELD_LIST_INFO;
		fieldList.dictionaryId = 1;
		fieldList.fieldListNum = 555;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));
	
	/* Encode Int field. */
	if (params.intValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = INT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslInt));
	}

	/* Encode UInt field. */
	if (params.uintValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = UINT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_UINT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslUInt));
	}

	/* Encode Float field. */
	if (params.floatValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = FLOAT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_FLOAT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslFloat));
	}

	/* Encode Double field. */
	if (params.doubleValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DOUBLE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DOUBLE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslDouble));
	}

	/* Encode Real field. */
	if (params.real)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = REAL_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_REAL;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &REAL));
	}

	/* Encode Date field. */
	if (params.date)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DATE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &DATETIME.date));
	}

	/* Encode Time field. */
	if (params.time)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = TIME_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_TIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &DATETIME.time));
	}

	/* Encode DateTime field. */
	if (params.dateTime)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DATETIME_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DATETIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &DATETIME));
	}

	/* Encode Qos field. */
	if (params.qos)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = QOS_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_QOS;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &QOS));
	}

	/* Encode State field. */
	if (params.state)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = STATE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_STATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &STATE));
	}

	/* Encode Enum field. */
	if (params.enumValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ENUM_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ENUM;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, &rsslEnum));
	}

	/* Encode Array field. */
	if (params.array)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ARRAY_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		rsslClearArray(&rsslArray);
		rsslArray.primitiveType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &rsslArray));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[0]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[1]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[2]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Buffer field. */
	if (params.buffer)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = BUFFER_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		fieldEntry.encData = OPAQUE_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode AsciiString field. */
	if (params.asciiString)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ASCII_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ASCII_STRING;
		fieldEntry.encData = ASCII_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Utf8String field. */
	if (params.utf8String)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = UTF8_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_UTF8_STRING;
		fieldEntry.encData = UTF8_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode RmtesString field. */
	if (params.rmtesString)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = RMTES_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_RMTES_STRING;
		fieldEntry.encData = RMTES_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Opaque field. */
	if (params.opaque)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = OPAQUE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_OPAQUE;
		fieldEntry.encData = OPAQUE_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Xml field. */
	if (params.xml)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = XML_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_XML;
		fieldEntry.encData = XML_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode FieldList field. */
	if (params.fieldList)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = FIELDLIST_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_FIELD_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode ElementList field. */
	if (params.elementList)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ELEMENTLIST_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ELEMENT_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode FilterList field. */
	if (params.filterList)
	{
		RsslFilterList filterList;
		RsslFilterEntry filterEntry;

		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = FILTERLIST_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_FILTER_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));


		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFilterList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Map field. */
	if (params.map)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = MAP_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_MAP;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslMap(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Vector field. */
	if (params.vector)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = VECTOR_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_VECTOR;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslVector(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Series field. */
	if (params.series)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = SERIES_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_SERIES;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslSeries(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Message field */
	if (params.msg)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = MSG_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_MSG;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryInit(&_eIter, &fieldEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslUpdateMsg(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Json field. */
	if (params.json)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = JSON_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_JSON;
		fieldEntry.encData = JSON_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Complete encoding FieldList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(&_eIter, RSSL_TRUE));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(&_eIter, RSSL_TRUE));

	ASSERT_NO_FATAL_FAILURE(convertRsslToJson(params.protocolType));

	switch (params.protocolType)
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

			/* Check Int field. */
			if (params.intValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(INT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][INT_FIELD.fieldName.data].IsNumber());
				EXPECT_EQ(rsslInt, _jsonDocument["Fields"][INT_FIELD.fieldName.data].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(INT_FIELD.fieldName.data));

			/* Check UInt field. */
			if (params.uintValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(UINT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][UINT_FIELD.fieldName.data].IsNumber());
				EXPECT_EQ(rsslUInt, _jsonDocument["Fields"][UINT_FIELD.fieldName.data].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(UINT_FIELD.fieldName.data));

			/* Check Float field. */
			if (params.floatValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsNumber());
				EXPECT_NEAR(rsslFloat, _jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].GetFloat(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));

			/* Check Double field. */
			if (params.doubleValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DOUBLE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsNumber());
				EXPECT_NEAR(rsslDouble, _jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].GetDouble(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DOUBLE_FIELD.fieldName.data));

			/* Check Real field. */
			if (params.real)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(REAL_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][REAL_FIELD.fieldName.data].IsNumber());
				EXPECT_NEAR(39.05, _jsonDocument["Fields"][REAL_FIELD.fieldName.data].GetDouble(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(REAL_FIELD.fieldName.data));

			/* Check Date field. */
			if (params.date)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DATE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DATE_FIELD.fieldName.data].IsString());
				EXPECT_STREQ("1955-11-12", _jsonDocument["Fields"][DATE_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DATE_FIELD.fieldName.data));

			/* Check Time field. */
			if (params.time)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(TIME_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][TIME_FIELD.fieldName.data].IsString());
				EXPECT_STREQ("22:04:00", _jsonDocument["Fields"][TIME_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(TIME_FIELD.fieldName.data));

			/* Check DateTime field. */
			if (params.dateTime)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DATETIME_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DATETIME_FIELD.fieldName.data].IsString());
				EXPECT_STREQ("1955-11-12T22:04:00", _jsonDocument["Fields"][DATETIME_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DATETIME_FIELD.fieldName.data));

			/* Check Qos field. */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(QOS_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data].IsObject());
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data].HasMember("Timeliness"));
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data]["Timeliness"].IsString());
				EXPECT_STREQ("Realtime", _jsonDocument["Fields"][QOS_FIELD.fieldName.data]["Timeliness"].GetString());
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data].IsObject());
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data].HasMember("Rate"));
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data]["Rate"].IsString());
				EXPECT_STREQ("TickByTick", _jsonDocument["Fields"][QOS_FIELD.fieldName.data]["Rate"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(QOS_FIELD.fieldName.data));

			/* Check State field. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(STATE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data].IsObject());
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data].HasMember("Stream"));
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data]["Stream"].IsString());
				EXPECT_STREQ("Open", _jsonDocument["Fields"][STATE_FIELD.fieldName.data]["Stream"].GetString());
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data].HasMember("Data"));
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ("Ok", _jsonDocument["Fields"][STATE_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(STATE_FIELD.fieldName.data));

			/* Check Enum field. */
			if (params.enumValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ENUM_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ENUM_FIELD.fieldName.data].IsNumber());
				EXPECT_EQ(rsslEnum, _jsonDocument["Fields"][ENUM_FIELD.fieldName.data].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ENUM_FIELD.fieldName.data));

			/* Check Array field. */
			if (params.array)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ARRAY_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data].IsObject());

				/* Check array type. */
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data].HasMember("Type"));
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data]["Type"].IsString());
				EXPECT_STREQ("Int", _jsonDocument["Fields"][ARRAY_FIELD.fieldName.data]["Type"].GetString());

				/* Check array data. */
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data].HasMember("Data"));
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data]["Data"].IsArray());
				const Value& intArray = _jsonDocument["Fields"][ARRAY_FIELD.fieldName.data]["Data"];
				ASSERT_EQ(3, intArray.Size());
				ASSERT_TRUE(intArray[0].IsNumber());
				EXPECT_EQ(arrayInts[0], intArray[0].GetInt());
				ASSERT_TRUE(intArray[1].IsNumber());
				EXPECT_EQ(arrayInts[1], intArray[1].GetInt());
				ASSERT_TRUE(intArray[2].IsNumber());
				EXPECT_EQ(arrayInts[2], intArray[2].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ARRAY_FIELD.fieldName.data));

			/* Check Buffer field. */
			if (params.buffer)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(BUFFER_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][BUFFER_FIELD.fieldName.data].IsString());
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, &_jsonDocument["Fields"][BUFFER_FIELD.fieldName.data]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(BUFFER_FIELD.fieldName.data));

			/* Check AsciiString field. */
			if (params.asciiString)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ASCII_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ASCII_STRING_FIELD.fieldName.data].IsString());
				EXPECT_STREQ(ASCII_STRING.data, _jsonDocument["Fields"][ASCII_STRING_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ASCII_STRING_FIELD.fieldName.data));

			/* Check Utf8String field. */
			if (params.utf8String)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(UTF8_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][UTF8_STRING_FIELD.fieldName.data].IsString());
				EXPECT_STREQ(UTF8_STRING.data, _jsonDocument["Fields"][UTF8_STRING_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(UTF8_STRING_FIELD.fieldName.data));

			/* Check RmtesString field. */
			if (params.rmtesString)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(RMTES_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].IsString());

				/* RMTES content should be converted to UTF8 in JSON (and then back to the RMTES string when converted back to RWF). */
				EXPECT_STREQ(RMTES_STRING_AS_UTF8.data, _jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(RMTES_STRING_FIELD.fieldName.data));

			/* Check Opaque field. */
			if (params.opaque)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(OPAQUE_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, &_jsonDocument["Fields"][OPAQUE_FIELD.fieldName.data]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(OPAQUE_FIELD.fieldName.data));

			/* Check Xml field. */
			if (params.xml)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(XML_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][XML_FIELD.fieldName.data].IsString());
				EXPECT_STREQ(XML_BUFFER.data, _jsonDocument["Fields"][XML_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(XML_FIELD.fieldName.data));

			/* Check FieldList field. */
			if (params.fieldList)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FIELDLIST_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["Fields"][FIELDLIST_FIELD.fieldName.data], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(FIELDLIST_FIELD.fieldName.data));

			/* Check ElementList field. */
			if (params.elementList)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ELEMENTLIST_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["Fields"][ELEMENTLIST_FIELD.fieldName.data], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ELEMENTLIST_FIELD.fieldName.data));

			/* Check FilterList field. */
			if (params.filterList)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FILTERLIST_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(&_jsonDocument["Fields"][FILTERLIST_FIELD.fieldName.data], params.protocolType));
			}

			/* Check Map field. */
			if (params.map)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(MAP_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonMap(&_jsonDocument["Fields"][MAP_FIELD.fieldName.data], params.protocolType));
			}

			/* Check Message field. */
			if (params.msg)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(MSG_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(&_jsonDocument["Fields"][MSG_FIELD.fieldName.data], params.protocolType));
			}

			/* Check Vector field. */
			if (params.vector)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(VECTOR_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonVector(&_jsonDocument["Fields"][VECTOR_FIELD.fieldName.data], params.protocolType));
			}

			/* Check Series field. */
			if (params.series)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(SERIES_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonSeries(&_jsonDocument["Fields"][SERIES_FIELD.fieldName.data], params.protocolType));
			}

			/* Check Json field. */
			if (params.json)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(JSON_FIELD.fieldName.data));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonObject(&_jsonDocument["Fields"][JSON_FIELD.fieldName.data]));
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

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check FieldList. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsObject());

			/* Check FieldListInfo. */
			if (params.fieldListInfo)
			{
				ASSERT_TRUE(_jsonDocument["d"].HasMember("i"));
				ASSERT_TRUE(_jsonDocument["d"]["i"].IsObject());

				/* Dictionary ID */
				ASSERT_TRUE(_jsonDocument["d"]["i"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["i"]["d"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["d"]["i"]["d"].GetInt());

				/* FieldListNum */
				ASSERT_TRUE(_jsonDocument["d"]["i"].HasMember("n"));
				ASSERT_TRUE(_jsonDocument["d"]["i"]["n"].IsNumber());
				EXPECT_EQ(555, _jsonDocument["d"]["i"]["n"].GetInt());
			}

			/* Check Int field. */
			if (params.intValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(INT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][INT_FIELD.fieldIdString].IsNumber());
				EXPECT_EQ(rsslInt, _jsonDocument["d"]["d"][INT_FIELD.fieldIdString].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(INT_FIELD.fieldIdString));

			/* Check UInt field. */
			if (params.uintValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(UINT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][UINT_FIELD.fieldIdString].IsNumber());
				EXPECT_EQ(rsslUInt, _jsonDocument["d"]["d"][UINT_FIELD.fieldIdString].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(UINT_FIELD.fieldIdString));

			/* Check Float field. */
			if (params.floatValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(FLOAT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][FLOAT_FIELD.fieldIdString].IsNumber());
				EXPECT_NEAR(rsslFloat, _jsonDocument["d"]["d"][FLOAT_FIELD.fieldIdString].GetFloat(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(FLOAT_FIELD.fieldIdString));

			/* Check Double field. */
			if (params.doubleValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DOUBLE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DOUBLE_FIELD.fieldIdString].IsNumber());
				EXPECT_NEAR(rsslDouble, _jsonDocument["d"]["d"][DOUBLE_FIELD.fieldIdString].GetDouble(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DOUBLE_FIELD.fieldIdString));

			/* Check Real field. */
			if (params.real)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(REAL_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString]["h"].IsNumber());
				EXPECT_EQ(RSSL_RH_EXPONENT_2, _jsonDocument["d"]["d"][REAL_FIELD.fieldIdString]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString].HasMember("v"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString]["v"].IsNumber());
				EXPECT_EQ(3905, _jsonDocument["d"]["d"][REAL_FIELD.fieldIdString]["v"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(REAL_FIELD.fieldIdString));

			/* Check Date field. */
			if (params.date)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DATE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString].HasMember("y"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["y"].IsNumber());
				EXPECT_EQ(1955, _jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["y"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["m"].IsNumber());
				EXPECT_EQ(11, _jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["d"].IsNumber());
				EXPECT_EQ(12, _jsonDocument["d"]["d"][DATE_FIELD.fieldIdString]["d"].GetInt());

			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DATE_FIELD.fieldIdString));

			/* Check Time field. */
			if (params.time)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(TIME_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["h"].IsNumber());
				EXPECT_EQ(22, _jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["m"].IsNumber());
				EXPECT_EQ(4, _jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["s"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["s"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].HasMember("x"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["x"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][TIME_FIELD.fieldIdString]["x"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(TIME_FIELD.fieldIdString));

			/* Check DateTime field. */
			if (params.dateTime)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DATETIME_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString].IsObject());

				/* Date */
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"].HasMember("y"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["y"].IsNumber());
				EXPECT_EQ(1955, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["y"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["m"].IsNumber());
				EXPECT_EQ(11, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["d"].IsNumber());
				EXPECT_EQ(12, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["d"]["d"].GetInt());

				/* Time */
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["h"].IsNumber());
				EXPECT_EQ(22, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["m"].IsNumber());
				EXPECT_EQ(4, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["s"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["s"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"].HasMember("x"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["x"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString]["t"]["x"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DATETIME_FIELD.fieldIdString));

			/* Check Qos field. */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(QOS_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString]["t"].IsNumber());
				EXPECT_EQ(RSSL_QOS_TIME_REALTIME, _jsonDocument["d"]["d"][QOS_FIELD.fieldIdString]["t"].GetInt());
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString].HasMember("r"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString]["r"].IsNumber());
				EXPECT_EQ(RSSL_QOS_RATE_TICK_BY_TICK, _jsonDocument["d"]["d"][QOS_FIELD.fieldIdString]["r"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(QOS_FIELD.fieldIdString));

			/* Check State field. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(STATE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString]["s"].IsNumber());
				EXPECT_EQ(RSSL_STREAM_OPEN, _jsonDocument["d"]["d"][STATE_FIELD.fieldIdString]["s"].GetInt());
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString]["d"].IsNumber());
				EXPECT_EQ(RSSL_DATA_OK, _jsonDocument["d"]["d"][STATE_FIELD.fieldIdString]["d"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(STATE_FIELD.fieldIdString));

			/* Check Enum field. */
			if (params.enumValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ENUM_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ENUM_FIELD.fieldIdString].IsNumber());
				EXPECT_EQ(rsslEnum, _jsonDocument["d"]["d"][ENUM_FIELD.fieldIdString].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ENUM_FIELD.fieldIdString));

			/* Check Array field. */
			if (params.array)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ARRAY_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString].IsObject());

				/* Check array type. */
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString]["t"].IsNumber());
				EXPECT_EQ(RSSL_DT_INT, _jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString]["t"].GetInt());

				/* Check array data. */
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString]["d"].IsArray());
				const Value& intArray = _jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString]["d"];
				ASSERT_EQ(3, intArray.Size());
				ASSERT_TRUE(intArray[0].IsNumber());
				EXPECT_EQ(arrayInts[0], intArray[0].GetInt());
				ASSERT_TRUE(intArray[1].IsNumber());
				EXPECT_EQ(arrayInts[1], intArray[1].GetInt());
				ASSERT_TRUE(intArray[2].IsNumber());
				EXPECT_EQ(arrayInts[2], intArray[2].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ARRAY_FIELD.fieldIdString));

			/* Check Buffer field. */
			if (params.buffer)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(BUFFER_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][BUFFER_FIELD.fieldIdString].IsString());
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, &_jsonDocument["d"]["d"][BUFFER_FIELD.fieldIdString]));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(BUFFER_FIELD.fieldIdString));

			/* Check AsciiString field. */
			if (params.asciiString)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ASCII_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ASCII_STRING_FIELD.fieldIdString].IsString());
				EXPECT_STREQ(ASCII_STRING.data, _jsonDocument["d"]["d"][ASCII_STRING_FIELD.fieldIdString].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ASCII_STRING_FIELD.fieldIdString));

			/* Check Utf8String field. */
			if (params.utf8String)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(UTF8_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][UTF8_STRING_FIELD.fieldIdString].IsString());
				EXPECT_STREQ(UTF8_STRING.data, _jsonDocument["d"]["d"][UTF8_STRING_FIELD.fieldIdString].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(UTF8_STRING_FIELD.fieldIdString));

			/* Check RmtesString field. */
			if (params.rmtesString)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(RMTES_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][RMTES_STRING_FIELD.fieldIdString].IsString());

				/* RMTES content should be converted to UTF8 in JSON (and then back to the RMTES string when converted back to RWF). */
				EXPECT_STREQ(RMTES_STRING_AS_UTF8.data, _jsonDocument["d"]["d"][RMTES_STRING_FIELD.fieldIdString].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(RMTES_STRING_FIELD.fieldIdString));

			/* Check Opaque field. */
			if (params.opaque)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(OPAQUE_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, &_jsonDocument["d"]["d"][OPAQUE_FIELD.fieldIdString]));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(OPAQUE_FIELD.fieldIdString));

			/* Check Xml field. */
			if (params.xml)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(XML_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][XML_FIELD.fieldIdString].IsString());
				EXPECT_STREQ(XML_BUFFER.data, _jsonDocument["d"]["d"][XML_FIELD.fieldIdString].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(XML_FIELD.fieldIdString));

			/* Check FieldList field. */
			if (params.fieldList)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(FIELDLIST_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(&_jsonDocument["d"]["d"][FIELDLIST_FIELD.fieldIdString], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(FIELDLIST_FIELD.fieldIdString));

			/* Check ElementList field. */
			if (params.elementList)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ELEMENTLIST_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(&_jsonDocument["d"]["d"][ELEMENTLIST_FIELD.fieldIdString], params.protocolType));
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ELEMENTLIST_FIELD.fieldIdString));

			/* Check FilterList field. */
			if (params.filterList)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(FILTERLIST_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(&_jsonDocument["d"]["d"][FILTERLIST_FIELD.fieldIdString], params.protocolType));
			}

			/* Check Map field. */
			if (params.map)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(MAP_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonMap(&_jsonDocument["d"]["d"][MAP_FIELD.fieldIdString], params.protocolType));
			}

			/* Check Message field. */
			if (params.msg)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(MSG_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(&_jsonDocument["d"]["d"][MSG_FIELD.fieldIdString], params.protocolType));
			}

			/* Check Vector field. */
			if (params.vector)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(VECTOR_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonVector(&_jsonDocument["d"]["d"][VECTOR_FIELD.fieldIdString], params.protocolType));
			}

			/* Check Series field. */
			if (params.series)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(SERIES_FIELD.fieldIdString));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonSeries(&_jsonDocument["d"]["d"][SERIES_FIELD.fieldIdString], params.protocolType));
			}

			/* (Json container type Not supported in JSON1) */
			ASSERT_FALSE(params.json);

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
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));


	/* Ensure that the appropriate entries are found (and that fields that shouldn't be there aren't). 
	 * Don't assume that they will appear in order after conversion. */
	RsslRet ret;
	bool foundIntField = false, foundUintField = false, foundFloatField = false, foundDoubleField = false, foundRealField = false,
		 foundDateField = false, foundTimeField = false, foundDateTimeField = false, foundQosField = false, foundStateField = false,
		 foundEnumField = false, foundArrayField = false, foundBufferField = false, foundAsciiStringField = false, foundUtf8StringField = false,
		 foundRmtesStringField = false, foundOpaqueField = false, foundXmlField = false, foundFieldListField = false,
		 foundElementListField = false, foundFilterListField = false, foundMapField = false, foundVectorField = false, foundSeriesField = false,
		 foundMsgField = false, foundJsonField = false;

	while ((ret = rsslDecodeFieldEntry(&_dIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode FieldEntry with ID: " << fieldEntry.fieldId;
		ASSERT_EQ(RSSL_DT_UNKNOWN, fieldEntry.dataType);

		if (fieldEntry.fieldId == INT_FIELD.fieldId)
		{
			RsslInt decodeInt;
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &decodeInt));
			EXPECT_EQ(rsslInt, decodeInt);
			foundIntField = true;
		}
		else if (fieldEntry.fieldId == UINT_FIELD.fieldId)
		{
			RsslUInt decodeUInt;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeUInt(&_dIter, &decodeUInt));
			EXPECT_EQ(rsslUInt, decodeUInt);
			foundUintField = true;
		}
		else if (fieldEntry.fieldId == FLOAT_FIELD.fieldId)
		{
			RsslFloat decodeFloat;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));
			EXPECT_NEAR(rsslFloat, decodeFloat, 0.099);
			foundFloatField = true;
		}
		else if (fieldEntry.fieldId == DOUBLE_FIELD.fieldId)
		{
			RsslDouble decodeDouble;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDouble(&_dIter, &decodeDouble));
			EXPECT_NEAR(rsslDouble, decodeDouble, 0.099);
			foundDoubleField = true;
		}
		else if (fieldEntry.fieldId == REAL_FIELD.fieldId)
		{
			RsslReal decodeReal;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(&_dIter, &decodeReal));
			EXPECT_EQ(REAL.isBlank, decodeReal.isBlank);
			EXPECT_EQ(REAL.hint, decodeReal.hint);
			EXPECT_EQ(REAL.value, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
			foundRealField = true;
		}
		else if (fieldEntry.fieldId == DATE_FIELD.fieldId)
		{
			RsslDate decodeDate;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDate(&_dIter, &decodeDate));

			ASSERT_EQ(DATETIME.date.day, decodeDate.day);
			ASSERT_EQ(DATETIME.date.month, decodeDate.month);
			ASSERT_EQ(DATETIME.date.year, decodeDate.year);
			foundDateField = true;
		}
		else if (fieldEntry.fieldId == TIME_FIELD.fieldId)
		{
			RsslTime decodeTime;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeTime(&_dIter, &decodeTime));

			ASSERT_EQ(DATETIME.time.hour, decodeTime.hour);
			ASSERT_EQ(DATETIME.time.minute, decodeTime.minute);
			ASSERT_EQ(DATETIME.time.second, decodeTime.second);
			ASSERT_EQ(DATETIME.time.millisecond, decodeTime.millisecond);
			ASSERT_EQ(DATETIME.time.microsecond, decodeTime.microsecond);
			ASSERT_EQ(DATETIME.time.nanosecond, decodeTime.nanosecond);
			foundTimeField = true;
		}
		else if (fieldEntry.fieldId == DATETIME_FIELD.fieldId)
		{
			RsslDateTime decodeDateTime;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDateTime(&_dIter, &decodeDateTime));

			ASSERT_EQ(DATETIME.date.day, decodeDateTime.date.day);
			ASSERT_EQ(DATETIME.date.month, decodeDateTime.date.month);
			ASSERT_EQ(DATETIME.date.year, decodeDateTime.date.year);
			ASSERT_EQ(DATETIME.time.hour, decodeDateTime.time.hour);
			ASSERT_EQ(DATETIME.time.minute, decodeDateTime.time.minute);
			ASSERT_EQ(DATETIME.time.second, decodeDateTime.time.second);
			ASSERT_EQ(DATETIME.time.millisecond, decodeDateTime.time.millisecond);
			ASSERT_EQ(DATETIME.time.microsecond, decodeDateTime.time.microsecond);
			ASSERT_EQ(DATETIME.time.nanosecond, decodeDateTime.time.nanosecond);
			foundDateTimeField = true;
		}
		else if (fieldEntry.fieldId == QOS_FIELD.fieldId)
		{
			RsslQos decodeQos;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeQos(&_dIter, &decodeQos));
			EXPECT_TRUE(rsslQosIsEqual(&QOS, &decodeQos));
			foundQosField = true;
		}
		else if (fieldEntry.fieldId == STATE_FIELD.fieldId)
		{
			RsslState decodeState;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeState(&_dIter, &decodeState));

			ASSERT_EQ(STATE.streamState, decodeState.streamState);
			ASSERT_EQ(STATE.dataState, decodeState.dataState);
			ASSERT_EQ(STATE.code, decodeState.code);
			foundStateField = true;
		}
		else if (fieldEntry.fieldId == ENUM_FIELD.fieldId)
		{
			RsslEnum decodeEnum;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeEnum(&_dIter, &decodeEnum));
			EXPECT_EQ(rsslEnum, decodeEnum);
			foundEnumField = true;
		}
		else if (fieldEntry.fieldId == ARRAY_FIELD.fieldId)
		{
			RsslArray decodeArray;
			RsslBuffer decodeArrayEntry;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArray(&_dIter, &decodeArray));
			ASSERT_EQ(RSSL_DT_INT, decodeArray.primitiveType);

			for (int j = 0; j < 3; ++j)
			{
				RsslInt decodeInt;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeArrayEntry(&_dIter, &decodeArrayEntry));
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &decodeInt));
				EXPECT_EQ(arrayInts[j], decodeInt);
			}
			ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeArrayEntry(&_dIter, &decodeArrayEntry));
			foundArrayField = true;
		}
		else if (fieldEntry.fieldId == BUFFER_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&OPAQUE_BUFFER, &decodeBuffer));
			foundBufferField = true;
		}
		else if (fieldEntry.fieldId == ASCII_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&ASCII_STRING, &decodeBuffer));
			foundAsciiStringField = true;
		}
		else if (fieldEntry.fieldId == UTF8_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&UTF8_STRING, &decodeBuffer));
			foundUtf8StringField = true;
		}
		else if (fieldEntry.fieldId == RMTES_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			/* ADS will convert from RMTES to UTF8 when sending data to consumer. UTF8 to RMTES conversion does not happen for consumer to ADS */
            EXPECT_TRUE(rsslBufferIsEqual(&RMTES_STRING_AS_UTF8, &decodeBuffer));
			foundRmtesStringField = true;
		}
		else if (fieldEntry.fieldId == OPAQUE_FIELD.fieldId)
		{
			RsslBuffer decodeOpaque;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeOpaque));
			EXPECT_TRUE(rsslBufferIsEqual(&OPAQUE_BUFFER, &decodeOpaque));
			foundOpaqueField = true;
		}
		else if (fieldEntry.fieldId == XML_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&XML_BUFFER, &decodeBuffer));
			foundXmlField = true;
		}
		else if (fieldEntry.fieldId == FIELDLIST_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
			foundFieldListField = true;
		}
		else if (fieldEntry.fieldId == ELEMENTLIST_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));
			foundElementListField = true;
		}
		else if (fieldEntry.fieldId == FILTERLIST_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFilterList(params.protocolType, &_dIter));
			foundFilterListField = true;
		}
		else if (fieldEntry.fieldId == MAP_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslMap(params.protocolType, &_dIter));
			foundMapField = true;
		}
		else if (fieldEntry.fieldId == VECTOR_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslVector(params.protocolType, &_dIter));
			foundVectorField = true;
		}
		else if (fieldEntry.fieldId == SERIES_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslSeries(params.protocolType, &_dIter));
			foundSeriesField = true;
		}
		else if (fieldEntry.fieldId == MSG_FIELD.fieldId)
		{
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslUpdateMsg(params.protocolType, &_dIter));
			foundMsgField = true;
		}
		else if (fieldEntry.fieldId == JSON_FIELD.fieldId)
		{
			Document document;
			RsslBuffer decodeJsonBuffer;

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeJsonBuffer));

			document.Parse(decodeJsonBuffer.data, decodeJsonBuffer.length);
			ASSERT_FALSE(document.HasParseError()) << "** JSON Parse Error (offset " << document.GetErrorOffset() << "): " 
				<< GetParseError_En(document.GetParseError());

			ASSERT_NO_FATAL_FAILURE(checkSampleJsonObject(&document));
			foundJsonField = true;
		}
		else
			FAIL() << "** Decoded FieldEntry with unknown ID: " << fieldEntry.fieldId;
	}

	ASSERT_EQ(params.intValue, foundIntField);
	ASSERT_EQ(params.uintValue, foundUintField);
	ASSERT_EQ(params.floatValue, foundFloatField);
	ASSERT_EQ(params.doubleValue, foundDoubleField);
	ASSERT_EQ(params.real, foundRealField);
	ASSERT_EQ(params.date, foundDateField);
	ASSERT_EQ(params.time, foundTimeField);
	ASSERT_EQ(params.dateTime, foundDateTimeField);
	ASSERT_EQ(params.qos, foundQosField);
	ASSERT_EQ(params.state, foundStateField);
	ASSERT_EQ(params.enumValue, foundEnumField);
	ASSERT_EQ(params.array, foundArrayField);
	ASSERT_EQ(params.buffer, foundBufferField);
	ASSERT_EQ(params.asciiString, foundAsciiStringField);
	ASSERT_EQ(params.utf8String, foundUtf8StringField);
	ASSERT_EQ(params.rmtesString, foundRmtesStringField);
	ASSERT_EQ(params.opaque, foundOpaqueField);
	ASSERT_EQ(params.xml, foundXmlField);
	ASSERT_EQ(params.fieldList, foundFieldListField);
	ASSERT_EQ(params.elementList, foundElementListField);
	ASSERT_EQ(params.filterList, foundFilterListField);
	ASSERT_EQ(params.map, foundMapField);
	ASSERT_EQ(params.vector, foundVectorField);
	ASSERT_EQ(params.series, foundSeriesField);
	ASSERT_EQ(params.msg, foundMsgField);
	ASSERT_EQ(params.json, foundJsonField);
}

INSTANTIATE_TEST_CASE_P(FieldListTestsJson2, FieldListTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Nothing */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, false),

	/* Int */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT, false),

	/* UInt */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT, false),

	/* Float */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT, false),

	/* Double */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE, false),

	/* Real */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL, false),

	/* Date */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE, false),

	/* Time */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME, false),

	/* DateTime */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME, false),

	/* QoS */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS, false),

	/* State */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE, false),

	/* Enum */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM, false),

	/* Array */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ARRAY, false),

	/* Buffer */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER, false),

	/* AsciiString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING, false),

	/* Utf8String */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING, false),

	/* RmtesString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING, false),

	/* Opaque */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_OPAQUE, false),

	/* Xml */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_XML, false),

	/* FieldList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST, false),

	/* ElementList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ELEMENT_LIST, false),

	/* FilterList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FILTER_LIST, false),

	/* Map */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_MAP, false),

	/* Vector */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_VECTOR, false),

	/* Series */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_SERIES, false),

	/* Message */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_MSG, false),

	/* Json */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_JSON, false),

	/* Everything */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, sizeof(allDataTypes)/sizeof(RsslUInt8), allDataTypes, false)

));

INSTANTIATE_TEST_CASE_P(FieldListTestsJson1, FieldListTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Nothing */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, false),

	/* Int */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT, false),

	/* UInt */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT, false),

	/* Float */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT, false),

	/* Double */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE, false),

	/* Real */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL, false),

	/* Date */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE, false),

	/* Time */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME, false),

	/* DateTime */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME, false),

	/* QoS */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS, false),

	/* State */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE, false),

	/* Enum */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM, false),

	/* Array */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ARRAY, false),

	/* Buffer */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER, false),

	/* AsciiString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING, false),

	/* Utf8String */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING, false),

	/* RmtesString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING, false),

	/* Opaque */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_OPAQUE, false),

	/* Xml */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_XML, false),

	/* FieldList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST, false),

	/* ElementList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ELEMENT_LIST, false),

	/* FilterList */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FILTER_LIST, false),

	/* Map */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_MAP, false),

	/* Vector */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_VECTOR, false),

	/* Series */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_SERIES, false),

	/* Message */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_MSG, false),

	/* (Json container type not supported in JSON1) */

	/* FieldListInfo */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, true),

	/* All types */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, sizeof(json1AllDataTypes)/sizeof(RsslUInt8), json1AllDataTypes, false),

	/* Everything */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, sizeof(json1AllDataTypes)/sizeof(RsslUInt8), json1AllDataTypes, true)

));

class FieldListPrimitiveTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<FieldListTypesTestParams>
{
};

/* Test a field list that encodes different primitive types in its field entries, as blank. */
TEST_P(FieldListPrimitiveTypesTestFixture, FieldListBlankTests)
{
	FieldListTypesTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

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

	/* Encode a field list whose entries encode different primitive types. */
	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(&_eIter, &fieldList, NULL, 0));
	
	/* Encode Int field. */
	if (params.intValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = INT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode UInt field. */
	if (params.uintValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = UINT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_UINT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Float field. */
	if (params.floatValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = FLOAT_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_FLOAT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Double field. */
	if (params.doubleValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DOUBLE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DOUBLE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Real field. */
	if (params.real)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = REAL_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_REAL;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Date field. */
	if (params.date)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DATE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Time field. */
	if (params.time)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = TIME_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_TIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode DateTime field. */
	if (params.dateTime)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = DATETIME_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_DATETIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Qos field. */
	if (params.qos)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = QOS_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_QOS;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode State field. */
	if (params.state)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = STATE_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_STATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Enum field. */
	if (params.enumValue)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ENUM_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ENUM;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Array field. */
	if (params.array)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ARRAY_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Buffer field. */
	if (params.buffer)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = BUFFER_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode AsciiString field. */
	if (params.asciiString)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = ASCII_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_ASCII_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode Utf8String field. */
	if (params.utf8String)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = UTF8_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_UTF8_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Encode RmtesString field. */
	if (params.rmtesString)
	{
		rsslClearFieldEntry(&fieldEntry);
		fieldEntry.fieldId = RMTES_STRING_FIELD.fieldId;
		fieldEntry.dataType = RSSL_DT_RMTES_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(&_eIter, &fieldEntry, NULL));
	}

	/* Complete encoding FieldList and Message. */
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

			/* Check Int field. */
			if (params.intValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(INT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][INT_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(INT_FIELD.fieldName.data));

			/* Check UInt field. */
			if (params.uintValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(UINT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][UINT_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(UINT_FIELD.fieldName.data));

			/* Check Float field. */
			if (params.floatValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][FLOAT_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(FLOAT_FIELD.fieldName.data));

			/* Check Double field. */
			if (params.doubleValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DOUBLE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DOUBLE_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DOUBLE_FIELD.fieldName.data));

			/* Check Real field. */
			if (params.real)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(REAL_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][REAL_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(REAL_FIELD.fieldName.data));

			/* Check Date field. */
			if (params.date)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DATE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DATE_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DATE_FIELD.fieldName.data));

			/* Check Time field. */
			if (params.time)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(TIME_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][TIME_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(TIME_FIELD.fieldName.data));

			/* Check DateTime field. */
			if (params.dateTime)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(DATETIME_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][DATETIME_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(DATETIME_FIELD.fieldName.data));

			/* Check Qos field. */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(QOS_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][QOS_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(QOS_FIELD.fieldName.data));

			/* Check State field. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(STATE_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][STATE_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(STATE_FIELD.fieldName.data));

			/* Check Enum field. */
			if (params.enumValue)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ENUM_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ENUM_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ENUM_FIELD.fieldName.data));

			/* Check Array field. */
			if (params.array)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ARRAY_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ARRAY_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ARRAY_FIELD.fieldName.data));

			/* Check Buffer field. */
			if (params.buffer)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(BUFFER_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][BUFFER_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(BUFFER_FIELD.fieldName.data));

			/* Check AsciiString field. */
			if (params.asciiString)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(ASCII_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][ASCII_STRING_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(ASCII_STRING_FIELD.fieldName.data));

			/* Check Utf8String field. */
			if (params.utf8String)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(UTF8_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][UTF8_STRING_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(UTF8_STRING_FIELD.fieldName.data));

			/* Check RmtesString field. */
			if (params.rmtesString)
			{
				ASSERT_TRUE(_jsonDocument["Fields"].HasMember(RMTES_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Fields"][RMTES_STRING_FIELD.fieldName.data].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Fields"].HasMember(RMTES_STRING_FIELD.fieldName.data));
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
			EXPECT_EQ(RSSL_DT_FIELD_LIST - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check FieldList. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsObject());

			/* Check Int field. */
			if (params.intValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(INT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][INT_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(INT_FIELD.fieldIdString));

			/* Check UInt field. */
			if (params.uintValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(UINT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][UINT_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(UINT_FIELD.fieldIdString));

			/* Check Float field. */
			if (params.floatValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(FLOAT_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][FLOAT_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(FLOAT_FIELD.fieldIdString));

			/* Check Double field. */
			if (params.doubleValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DOUBLE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DOUBLE_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DOUBLE_FIELD.fieldIdString));

			/* Check Real field. */
			if (params.real)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(REAL_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][REAL_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(REAL_FIELD.fieldIdString));

			/* Check Date field. */
			if (params.date)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DATE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATE_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DATE_FIELD.fieldIdString));

			/* Check Time field. */
			if (params.time)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(TIME_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][TIME_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(TIME_FIELD.fieldIdString));

			/* Check DateTime field. */
			if (params.dateTime)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(DATETIME_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][DATETIME_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(DATETIME_FIELD.fieldIdString));

			/* Check Qos field. */
			if (params.qos)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(QOS_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][QOS_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(QOS_FIELD.fieldIdString));

			/* Check State field. */
			if (params.state)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(STATE_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][STATE_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(STATE_FIELD.fieldIdString));

			/* Check Enum field. */
			if (params.enumValue)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ENUM_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ENUM_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ENUM_FIELD.fieldIdString));

			/* Check Array field. */
			if (params.array)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ARRAY_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ARRAY_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ARRAY_FIELD.fieldIdString));

			/* Check Buffer field. */
			if (params.buffer)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(BUFFER_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][BUFFER_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(BUFFER_FIELD.fieldIdString));

			/* Check AsciiString field. */
			if (params.asciiString)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(ASCII_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][ASCII_STRING_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(ASCII_STRING_FIELD.fieldIdString));

			/* Check Utf8String field. */
			if (params.utf8String)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(UTF8_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][UTF8_STRING_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(UTF8_STRING_FIELD.fieldIdString));

			/* Check RmtesString field. */
			if (params.rmtesString)
			{
				ASSERT_TRUE(_jsonDocument["d"]["d"].HasMember(RMTES_STRING_FIELD.fieldIdString));
				ASSERT_TRUE(_jsonDocument["d"]["d"][RMTES_STRING_FIELD.fieldIdString].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["d"]["d"].HasMember(RMTES_STRING_FIELD.fieldIdString));
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

	if (params.fieldListInfo)
	{
		ASSERT_TRUE(rsslFieldListCheckHasInfo(&fieldList));
		EXPECT_EQ(1, fieldList.dictionaryId);
		EXPECT_EQ(555, fieldList.fieldListNum);
	}
	else
		EXPECT_FALSE(rsslFieldListCheckHasInfo(&fieldList));


	/* Ensure that the appropriate entries are found (and that fields that shouldn't be there aren't). 
	 * Don't assume that they will appear in order after conversion. */
	RsslRet ret;
	bool foundIntField = false, foundUintField = false, foundFloatField = false, foundDoubleField = false, foundRealField = false,
		 foundDateField = false, foundTimeField = false, foundDateTimeField = false, foundQosField = false, foundStateField = false,
		 foundEnumField = false, foundArrayField = false, foundBufferField = false, foundAsciiStringField = false, foundUtf8StringField = false,
		 foundRmtesStringField = false;

	while ((ret = rsslDecodeFieldEntry(&_dIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode FieldEntry with ID: " << fieldEntry.fieldId;
		ASSERT_EQ(RSSL_DT_UNKNOWN, fieldEntry.dataType);

		if (fieldEntry.fieldId == INT_FIELD.fieldId)
		{
			RsslInt decodeInt;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeInt(&_dIter, &decodeInt));
			foundIntField = true;
		}
		else if (fieldEntry.fieldId == UINT_FIELD.fieldId)
		{
			RsslUInt decodeUInt;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeUInt(&_dIter, &decodeUInt));
			foundUintField = true;
		}
		else if (fieldEntry.fieldId == FLOAT_FIELD.fieldId)
		{
			RsslFloat decodeFloat;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeFloat(&_dIter, &decodeFloat));
			foundFloatField = true;
		}
		else if (fieldEntry.fieldId == DOUBLE_FIELD.fieldId)
		{
			RsslDouble decodeDouble;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDouble(&_dIter, &decodeDouble));
			foundDoubleField = true;
		}
		else if (fieldEntry.fieldId == REAL_FIELD.fieldId)
		{
			RsslReal decodeReal;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeReal(&_dIter, &decodeReal));
			foundRealField = true;
		}
		else if (fieldEntry.fieldId == DATE_FIELD.fieldId)
		{
			RsslDate decodeDate;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDate(&_dIter, &decodeDate));
			foundDateField = true;
		}
		else if (fieldEntry.fieldId == TIME_FIELD.fieldId)
		{
			RsslTime decodeTime;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeTime(&_dIter, &decodeTime));
			foundTimeField = true;
		}
		else if (fieldEntry.fieldId == DATETIME_FIELD.fieldId)
		{
			RsslDateTime decodeDateTime;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDateTime(&_dIter, &decodeDateTime));
			foundDateTimeField = true;
		}
		else if (fieldEntry.fieldId == QOS_FIELD.fieldId)
		{
			RsslQos decodeQos;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeQos(&_dIter, &decodeQos));
			foundQosField = true;
		}
		else if (fieldEntry.fieldId == STATE_FIELD.fieldId)
		{
			RsslState decodeState;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeState(&_dIter, &decodeState));
			foundStateField = true;
		}
		else if (fieldEntry.fieldId == ENUM_FIELD.fieldId)
		{
			RsslEnum decodeEnum;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeEnum(&_dIter, &decodeEnum));
			foundEnumField = true;
		}
		else if (fieldEntry.fieldId == ARRAY_FIELD.fieldId)
		{
			RsslArray decodeArray;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeArray(&_dIter, &decodeArray));
			foundArrayField = true;
		}
		else if (fieldEntry.fieldId == BUFFER_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundBufferField = true;
		}
		else if (fieldEntry.fieldId == ASCII_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundAsciiStringField = true;
		}
		else if (fieldEntry.fieldId == UTF8_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundUtf8StringField = true;
		}
		else if (fieldEntry.fieldId == RMTES_STRING_FIELD.fieldId)
		{
			RsslBuffer decodeBuffer;
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundRmtesStringField = true;
		}
		else
			FAIL() << "** Decoded FieldEntry with unknown ID: " << fieldEntry.fieldId;
	}

	ASSERT_EQ(params.intValue, foundIntField);
	ASSERT_EQ(params.uintValue, foundUintField);
	ASSERT_EQ(params.floatValue, foundFloatField);
	ASSERT_EQ(params.doubleValue, foundDoubleField);
	ASSERT_EQ(params.real, foundRealField);
	ASSERT_EQ(params.date, foundDateField);
	ASSERT_EQ(params.time, foundTimeField);
	ASSERT_EQ(params.dateTime, foundDateTimeField);
	ASSERT_EQ(params.qos, foundQosField);
	ASSERT_EQ(params.state, foundStateField);
	ASSERT_EQ(params.enumValue, foundEnumField);
	ASSERT_EQ(params.array, foundArrayField);
	ASSERT_EQ(params.buffer, foundBufferField);
	ASSERT_EQ(params.asciiString, foundAsciiStringField);
	ASSERT_EQ(params.utf8String, foundUtf8StringField);
	ASSERT_EQ(params.rmtesString, foundRmtesStringField);
}

INSTANTIATE_TEST_CASE_P(FieldListTestsJson2, FieldListPrimitiveTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. 
	 * Used for both FieldListTypesTest and FieldListBlankTests */

	/* Int */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT, false),

	/* UInt */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT, false),

	/* Float */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT, false),

	/* Double */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE, false),

	/* Real */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL, false),

	/* Date */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE, false),

	/* Time */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME, false),

	/* DateTime */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME, false),

	/* QoS */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS, false),

	/* State */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE, false),

	/* Enum */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM, false),

	/* Array */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ARRAY, false),

	/* Buffer */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER, false),

	/* AsciiString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING, false),

	/* Utf8String */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING, false),

	/* RmtesString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING, false),

	/* Everything */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON2, sizeof(allPrimitiveTypes)/sizeof(RsslUInt8), allPrimitiveTypes, false)

));

INSTANTIATE_TEST_CASE_P(FieldListTestsJson1, FieldListPrimitiveTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. 
	 * Used for both FieldListTypesTest and FieldListBlankTests */

	/* Int */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT, false),

	/* UInt */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT, false),

	/* Float */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT, false),

	/* Double */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE, false),

	/* Real */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL, false),

	/* Date */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE, false),

	/* Time */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME, false),

	/* DateTime */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME, false),

	/* QoS */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS, false),

	/* State */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE, false),

	/* Enum */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM, false),

	/* Array */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ARRAY, false),

	/* Buffer */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER, false),

	/* AsciiString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING, false),

	/* Utf8String */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING, false),

	/* RmtesString */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING, false),

	/* Everything */
	FieldListTypesTestParams(RSSL_JSON_JPT_JSON, sizeof(allPrimitiveTypes)/sizeof(RsslUInt8), allPrimitiveTypes, false)
));

TEST_F(FieldListTests, Json1FieldListNoDataTest)
{
	/* Test a case where no standard or set data is provided in JSON */
	RsslMsg rsslMsg;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	const char *fieldListMsgs [2] = {
		/* No FieldListInfo */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":4},\"u\":1,\"d\":{}}",

		/* Has FieldListInfo */
		"{\"b\":{\"s\":5,\"c\":4,\"t\":6,\"f\":4},\"u\":1,\"d\":{\"i\":{\"d\":1,\"n\":555}}}"
	};

	for (int i = 0; i < 2; ++i)
	{
		setJsonBufferToString(fieldListMsgs[i]);
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
		EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);
		EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, rsslMsg.updateMsg.updateType);

		/* Check FieldList. */
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(&_dIter, &fieldList, NULL));

		if (i == 0)
			ASSERT_FALSE(rsslFieldListCheckHasInfo(&fieldList));
		else
		{
			ASSERT_TRUE(rsslFieldListCheckHasInfo(&fieldList));
			EXPECT_EQ(1, fieldList.dictionaryId);
			EXPECT_EQ(555, fieldList.fieldListNum);
		}

		EXPECT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFieldEntry(&_dIter, &fieldEntry));
	}
}
