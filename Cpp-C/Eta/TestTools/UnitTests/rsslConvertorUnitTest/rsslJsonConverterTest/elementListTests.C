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

/* Fixture for ElementListTests that has conversion code. */
class ElementListTests : public MsgConversionTestBase
{
};

/** Most element entries follow a {"Type":"...", "Data": ... } format. Use this function to check correct name, correct Type member, 
 * and that the Data member is present. */
static void checkJsonElementEntryNameAndFormat(const Value &entry, const char *name, const char* type)
{
	ASSERT_TRUE(entry.HasMember(name));
	ASSERT_TRUE(entry[name].IsObject());

	ASSERT_TRUE(entry[name].HasMember("Type"));
	ASSERT_TRUE(entry[name]["Type"].IsString());
	EXPECT_STREQ(type, entry[name]["Type"].GetString());

	ASSERT_TRUE(entry[name].HasMember("Data"));
}

/** Use this function to check correct name, correct Type member, and that the Data member is present for a JSON1 element. */
static void checkJson1ElementEntryNameAndFormat(const Value &entry, const char *name, RsslDataTypes type)
{
	ASSERT_TRUE(entry.HasMember("n"));
	ASSERT_TRUE(entry["n"].IsString());
	EXPECT_STREQ(name, entry["n"].GetString());

	ASSERT_TRUE(entry.HasMember("t"));
	ASSERT_TRUE(entry["t"].IsNumber());
	EXPECT_EQ(type, entry["t"].GetInt());

	ASSERT_TRUE(entry.HasMember("d"));
}

/* Parameters for ElementListTests tests. */
class ElementListTypesTestParams
{
	public:

	RsslJsonProtocolType protocolType;

	/* Boolean parameters; when true, the corresponding datatype will be encoded in a element. */
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

	int memberCount;

	/* For empty test. */
	ElementListTypesTestParams(RsslJsonProtocolType protocolType)
	{
		init(protocolType, 0, NULL);
	}

	/* For tests that encode only one data type. */
	ElementListTypesTestParams(RsslJsonProtocolType protocolType, RsslUInt8 dataType)
	{
		init(protocolType, 1, &dataType);
	}

	/* For tests that encode multiple data types. */
	ElementListTypesTestParams(RsslJsonProtocolType protocolType, int argCount, RsslUInt8 *argList)
	{
		init(protocolType, argCount, argList);
	}
	
	/* Set flags indicating which data types will be encoded in the test. */
	void init(RsslJsonProtocolType protocolType, int argCount, RsslUInt8 *argList)
	{
		/* Initialize all flags to false. */
		memset(this, 0, sizeof(ElementListTypesTestParams));

		/* Set protocol type. */
		this->protocolType = protocolType;

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
				default:					FAIL() << "** Unknown data type specified in element list test parameters.";
			}
		}

		memberCount = argCount;
	}

	/* Overload the << operator -- when tests fail, this will cause the parameters to printed in a readable fashion. */
	friend ostream &operator<<(ostream &out, const ElementListTypesTestParams& params)
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
		out << "]";
		return out;
	}
};

class ElementListTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<ElementListTypesTestParams>
{
};



/* Test a element list that encodes different data types in its element entries. */
TEST_P(ElementListTypesTestFixture, ElementListTypesTest)
{
	ElementListTypesTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;
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
	updateMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
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

	/* Encode an element list whose entries encode different data types. */
	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(&_eIter, &elementList, NULL, 0));
	
	/* Encode Int element. */
	if (params.intValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = INT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &rsslInt));
	}

	/* Encode UInt element. */
	if (params.uintValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = UINT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_UINT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &rsslUInt));
	}

	/* Encode Float element. */
	if (params.floatValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = FLOAT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_FLOAT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &rsslFloat));
	}

	/* Encode Double element. */
	if (params.doubleValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DOUBLE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DOUBLE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &rsslDouble));
	}

	/* Encode Real element. */
	if (params.real)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = REAL_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_REAL;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &REAL));
	}

	/* Encode Date element. */
	if (params.date)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DATE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &DATETIME.date));
	}

	/* Encode Time element. */
	if (params.time)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = TIME_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_TIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &DATETIME.time));
	}

	/* Encode DateTime element. */
	if (params.dateTime)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DATETIME_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DATETIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &DATETIME));
	}

	/* Encode Qos element. */
	if (params.qos)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = QOS_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_QOS;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &QOS));
	}

	/* Encode State element. */
	if (params.state)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = STATE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_STATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &STATE));
	}

	/* Encode Enum element. */
	if (params.enumValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ENUM_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ENUM;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, &rsslEnum));
	}

	/* Encode Array element. */
	if (params.array)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ARRAY_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		rsslClearArray(&rsslArray);
		rsslArray.primitiveType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayInit(&_eIter, &rsslArray));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[0]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[1]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayEntry(&_eIter, NULL, &arrayInts[2]));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeArrayComplete(&_eIter, RSSL_TRUE));
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Buffer element. */
	if (params.buffer)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = BUFFER_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_BUFFER;
		elementEntry.encData = OPAQUE_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode AsciiString element. */
	if (params.asciiString)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ASCII_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ASCII_STRING;
		elementEntry.encData = ASCII_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Utf8String element. */
	if (params.utf8String)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = UTF8_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_UTF8_STRING;
		elementEntry.encData = UTF8_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode RmtesString element. */
	if (params.rmtesString)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = RMTES_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_RMTES_STRING;
		elementEntry.encData = RMTES_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Opaque element. */
	if (params.opaque)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = OPAQUE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_OPAQUE;
		elementEntry.encData = OPAQUE_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Xml element. */
	if (params.xml)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = XML_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_XML;
		elementEntry.encData = XML_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode FieldList element. */
	if (params.fieldList)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = FIELDLIST_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_FIELD_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode ElementList element. */
	if (params.elementList)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ELEMENTLIST_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ELEMENT_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode FilterList element. */
	if (params.filterList)
	{
		RsslFilterList filterList;
		RsslFilterEntry filterEntry;

		rsslClearElementEntry(&elementEntry);
		elementEntry.name = FILTERLIST_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_FILTER_LIST;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		rsslClearFilterList(&filterList);
		filterList.containerType = RSSL_DT_ELEMENT_LIST;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(&_eIter, &filterList));

		rsslClearFilterEntry(&filterEntry);
		filterEntry.id = 1;
		filterEntry.action = RSSL_FTEA_SET_ENTRY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(&_eIter, &filterEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(&_eIter, RSSL_TRUE));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(&_eIter, RSSL_TRUE));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Map element. */
	if (params.map)
	{
		RsslMap map;
		RsslMapEntry mapEntry;
		RsslInt mapEntryKey;

		rsslClearElementEntry(&elementEntry);
		elementEntry.name = MAP_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_MAP;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		rsslClearMap(&map);
		map.containerType = RSSL_DT_FIELD_LIST;
		map.keyPrimitiveType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(&_eIter, &map, 0, 0));

		rsslClearMapEntry(&mapEntry);
		mapEntryKey = 1;
		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(&_eIter, &mapEntry, &mapEntryKey, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(&_eIter, RSSL_TRUE));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(&_eIter, RSSL_TRUE));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Vector element. */
	if (params.vector)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = VECTOR_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_VECTOR;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslVector(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Series element. */
	if (params.series)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = SERIES_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_SERIES;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslSeries(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Message element */
	if (params.msg)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = MSG_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_MSG;

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryInit(&_eIter, &elementEntry, 0));

		ASSERT_NO_FATAL_FAILURE(encodeSampleRsslUpdateMsg(&_eIter));

		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntryComplete(&_eIter, RSSL_TRUE));
	}

	/* Encode Json element. */
	if (params.json)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = JSON_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_JSON;
		elementEntry.encData = JSON_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Complete encoding ElementList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(&_eIter, RSSL_TRUE));
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

			/* Check ElementList. */
			ASSERT_TRUE(_jsonDocument.HasMember("Elements"));
			ASSERT_TRUE(_jsonDocument["Elements"].IsObject());

			/* Check Int element. */
			if (params.intValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], INT_FIELD.fieldName.data, "Int"));
				ASSERT_TRUE(_jsonDocument["Elements"][INT_FIELD.fieldName.data]["Data"].IsNumber());
				EXPECT_EQ(rsslInt, _jsonDocument["Elements"][INT_FIELD.fieldName.data]["Data"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(INT_FIELD.fieldName.data));

			/* Check UInt element. */
			if (params.uintValue)
			{
				/* UInt is only a number (not an object with Type/Data members). */
				ASSERT_TRUE(_jsonDocument["Elements"].HasMember(UINT_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Elements"][UINT_FIELD.fieldName.data].IsNumber());
				EXPECT_EQ(rsslUInt, _jsonDocument["Elements"][UINT_FIELD.fieldName.data].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(UINT_FIELD.fieldName.data));

			/* Check Float element. */
			if (params.floatValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], FLOAT_FIELD.fieldName.data, "Float"));
				ASSERT_TRUE(_jsonDocument["Elements"][FLOAT_FIELD.fieldName.data]["Data"].IsNumber());
				EXPECT_NEAR(rsslFloat, _jsonDocument["Elements"][FLOAT_FIELD.fieldName.data]["Data"].GetFloat(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(FLOAT_FIELD.fieldName.data));

			/* Check Double element. */
			if (params.doubleValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DOUBLE_FIELD.fieldName.data, "Double"));
				ASSERT_TRUE(_jsonDocument["Elements"][DOUBLE_FIELD.fieldName.data]["Data"].IsNumber());
				EXPECT_NEAR(rsslDouble, _jsonDocument["Elements"][DOUBLE_FIELD.fieldName.data]["Data"].GetDouble(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DOUBLE_FIELD.fieldName.data));

			/* Check Real element. */
			if (params.real)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], REAL_FIELD.fieldName.data, "Real"));
				ASSERT_TRUE(_jsonDocument["Elements"][REAL_FIELD.fieldName.data]["Data"].IsNumber());
				EXPECT_NEAR(39.05, _jsonDocument["Elements"][REAL_FIELD.fieldName.data]["Data"].GetDouble(), 0.099);
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(REAL_FIELD.fieldName.data));

			/* Check Date element. */
			if (params.date)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DATE_FIELD.fieldName.data, "Date"));
				ASSERT_TRUE(_jsonDocument["Elements"][DATE_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ("1955-11-12", _jsonDocument["Elements"][DATE_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DATE_FIELD.fieldName.data));

			/* Check Time element. */
			if (params.time)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], TIME_FIELD.fieldName.data, "Time"));
				ASSERT_TRUE(_jsonDocument["Elements"][TIME_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ("22:04:00", _jsonDocument["Elements"][TIME_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(TIME_FIELD.fieldName.data));

			/* Check DateTime element. */
			if (params.dateTime)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DATETIME_FIELD.fieldName.data, "DateTime"));
				ASSERT_TRUE(_jsonDocument["Elements"][DATETIME_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ("1955-11-12T22:04:00", _jsonDocument["Elements"][DATETIME_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DATETIME_FIELD.fieldName.data));

			/* Check Qos element. */
			if (params.qos)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], QOS_FIELD.fieldName.data, "Qos"));

				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"].IsObject());
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"].HasMember("Timeliness"));
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"]["Timeliness"].IsString());
				EXPECT_STREQ("Realtime", _jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"]["Timeliness"].GetString());
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"].IsObject());
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"].HasMember("Rate"));
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"]["Rate"].IsString());
				EXPECT_STREQ("TickByTick", _jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"]["Rate"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(QOS_FIELD.fieldName.data));

			/* Check State element. */
			if (params.state)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], STATE_FIELD.fieldName.data, "State"));

				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"].IsObject());
				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"].HasMember("Stream"));
				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"]["Stream"].IsString());
				EXPECT_STREQ("Open", _jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"]["Stream"].GetString());
				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"].HasMember("Data"));
				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"]["Data"].IsString());
				EXPECT_STREQ("Ok", _jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(STATE_FIELD.fieldName.data));

			/* Check Enum element. */
			if (params.enumValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ENUM_FIELD.fieldName.data, "Enum"));
				ASSERT_TRUE(_jsonDocument["Elements"][ENUM_FIELD.fieldName.data]["Data"].IsNumber());
				EXPECT_EQ(rsslEnum, _jsonDocument["Elements"][ENUM_FIELD.fieldName.data]["Data"].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ENUM_FIELD.fieldName.data));

			/* Check Array element. */
			if (params.array)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ARRAY_FIELD.fieldName.data, "Array"));
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"].IsObject());

				/* Check array type. */
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"].HasMember("Type"));
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"]["Type"].IsString());
				EXPECT_STREQ("Int", _jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"]["Type"].GetString());

				/* Check array data. */
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"].HasMember("Data"));
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"]["Data"].IsArray());
				const Value& intArray = _jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"]["Data"];
				ASSERT_EQ(3, intArray.Size());
				ASSERT_TRUE(intArray[0].IsNumber());
				EXPECT_EQ(arrayInts[0], intArray[0].GetInt());
				ASSERT_TRUE(intArray[1].IsNumber());
				EXPECT_EQ(arrayInts[1], intArray[1].GetInt());
				ASSERT_TRUE(intArray[2].IsNumber());
				EXPECT_EQ(arrayInts[2], intArray[2].GetInt());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ARRAY_FIELD.fieldName.data));

			/* Check Buffer element. */
			if (params.buffer)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], BUFFER_FIELD.fieldName.data, "Buffer"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, _jsonDocument["Elements"][BUFFER_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(BUFFER_FIELD.fieldName.data));

			/* Check AsciiString element. */
			if (params.asciiString)
			{
				/* AsciiString is only a string (not an object with Type/Data members). */
				ASSERT_TRUE(_jsonDocument["Elements"].HasMember(ASCII_STRING_FIELD.fieldName.data));
				ASSERT_TRUE(_jsonDocument["Elements"][ASCII_STRING_FIELD.fieldName.data].IsString());
				EXPECT_STREQ(ASCII_STRING.data, _jsonDocument["Elements"][ASCII_STRING_FIELD.fieldName.data].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ASCII_STRING_FIELD.fieldName.data));

			/* Check Utf8String element. */
			if (params.utf8String)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], UTF8_STRING_FIELD.fieldName.data, "Utf8String"));
				ASSERT_TRUE(_jsonDocument["Elements"][UTF8_STRING_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ(UTF8_STRING.data, _jsonDocument["Elements"][UTF8_STRING_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(UTF8_STRING_FIELD.fieldName.data));

			/* Check RmtesString element. */
			if (params.rmtesString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], RMTES_STRING_FIELD.fieldName.data, "RmtesString"));
				ASSERT_TRUE(_jsonDocument["Elements"][RMTES_STRING_FIELD.fieldName.data]["Data"].IsString());

				/* RMTES content should be converted to UTF8 in JSON (and then back to the RMTES string when converted back to RWF). */
				EXPECT_STREQ(RMTES_STRING_AS_UTF8.data, _jsonDocument["Elements"][RMTES_STRING_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(RMTES_STRING_FIELD.fieldName.data));

			/* Check Opaque element. */
			if (params.opaque)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], OPAQUE_FIELD.fieldName.data, "Opaque"));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, _jsonDocument["Elements"][OPAQUE_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(OPAQUE_FIELD.fieldName.data));

			/* Check Xml element. */
			if (params.xml)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], XML_FIELD.fieldName.data, "Xml"));
				ASSERT_TRUE(_jsonDocument["Elements"][XML_FIELD.fieldName.data]["Data"].IsString());
				EXPECT_STREQ(XML_BUFFER.data, _jsonDocument["Elements"][XML_FIELD.fieldName.data]["Data"].GetString());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(XML_FIELD.fieldName.data));

			/* Check FieldList element. */
			if (params.fieldList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], FIELDLIST_FIELD.fieldName.data, "FieldList"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Elements"][FIELDLIST_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(FIELDLIST_FIELD.fieldName.data));

			/* Check ElementList element. */
			if (params.elementList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ELEMENTLIST_FIELD.fieldName.data, "ElementList"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["Elements"][ELEMENTLIST_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ELEMENTLIST_FIELD.fieldName.data));

			/* Check FilterList element. */
			if (params.filterList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], FILTERLIST_FIELD.fieldName.data, "FilterList"));
				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"].IsObject());

				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"].HasMember("Entries"));
				ASSERT_EQ(1, _jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"].Size());

				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0].HasMember("ID"));
				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0]["ID"].IsNumber());
				EXPECT_EQ(1, _jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0]["ID"].GetInt());

				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0].HasMember("Action"));
				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0]["Action"].IsString());
				EXPECT_STREQ("Set", _jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0]["Action"].GetString());

				ASSERT_TRUE(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0].HasMember("Elements"));

				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["Elements"][FILTERLIST_FIELD.fieldName.data]["Data"]["Entries"][0]["Elements"]));

			}

			/* Check Map element. */
			if (params.map)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], MAP_FIELD.fieldName.data, "Map"));
				ASSERT_TRUE(_jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"].IsObject());
				ASSERT_TRUE(_jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"].HasMember("Entries"));
				ASSERT_TRUE(_jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"]["Entries"].IsArray());
				ASSERT_EQ(1, _jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"]["Entries"].Size());

				/* Map Entry */
				ASSERT_TRUE(_jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"]["Entries"][0].HasMember("Fields"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["Elements"][MAP_FIELD.fieldName.data]["Data"]["Entries"][0]["Fields"]));
			}

			/* Check Vector element. */
			if (params.vector)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], VECTOR_FIELD.fieldName.data, "Vector"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonVector(_jsonDocument["Elements"][VECTOR_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(VECTOR_FIELD.fieldName.data));

			/* Check Series element. */
			if (params.series)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], SERIES_FIELD.fieldName.data, "Series"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonSeries(_jsonDocument["Elements"][SERIES_FIELD.fieldName.data]["Data"]));
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(SERIES_FIELD.fieldName.data));

			/* Check Message element. */
			if (params.msg)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], MSG_FIELD.fieldName.data, "Msg"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(_jsonDocument["Elements"][MSG_FIELD.fieldName.data]["Data"]));
			}

			/* Check Json element. */
			if (params.json)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], JSON_FIELD.fieldName.data, "Json"));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonObject(_jsonDocument["Elements"][JSON_FIELD.fieldName.data]["Data"]));
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
			EXPECT_EQ(RSSL_DT_ELEMENT_LIST - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check ElementList. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(params.memberCount, _jsonDocument["d"]["d"].Size());

			int currentEntry = 0;

			/* Check Int element. */
			if (params.intValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], INT_FIELD.fieldName.data, RSSL_DT_INT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNumber());
				EXPECT_EQ(rsslInt, _jsonDocument["d"]["d"][currentEntry]["d"].GetInt());
				++currentEntry;
			}

			/* Check UInt element. */
			if (params.uintValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], UINT_FIELD.fieldName.data, RSSL_DT_UINT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNumber());
				EXPECT_EQ(rsslUInt, _jsonDocument["d"]["d"][currentEntry]["d"].GetInt());
				++currentEntry;
			}

			/* Check Float element. */
			if (params.floatValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], FLOAT_FIELD.fieldName.data, RSSL_DT_FLOAT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNumber());
				EXPECT_NEAR(rsslFloat, _jsonDocument["d"]["d"][currentEntry]["d"].GetFloat(), 0.099);
				++currentEntry;
			}

			/* Check Double element. */
			if (params.doubleValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DOUBLE_FIELD.fieldName.data, RSSL_DT_DOUBLE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNumber());
				EXPECT_NEAR(rsslDouble, _jsonDocument["d"]["d"][currentEntry]["d"].GetDouble(), 0.099);
				++currentEntry;
			}

			/* Check Real element. */
			if (params.real)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], REAL_FIELD.fieldName.data, RSSL_DT_REAL));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["h"].IsNumber());
				EXPECT_EQ(RSSL_RH_EXPONENT_2, _jsonDocument["d"]["d"][currentEntry]["d"]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("v"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["v"].IsNumber());
				EXPECT_EQ(3905, _jsonDocument["d"]["d"][currentEntry]["d"]["v"].GetInt());
				++currentEntry;
			}

			/* Check Date element. */
			if (params.date)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DATE_FIELD.fieldName.data, RSSL_DT_DATE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("y"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["y"].IsNumber());
				EXPECT_EQ(1955, _jsonDocument["d"]["d"][currentEntry]["d"]["y"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["m"].IsNumber());
				EXPECT_EQ(11, _jsonDocument["d"]["d"][currentEntry]["d"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].IsNumber());
				EXPECT_EQ(12, _jsonDocument["d"]["d"][currentEntry]["d"]["d"].GetInt());
				++currentEntry;
			}

			/* Check Time element. */
			if (params.time)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], TIME_FIELD.fieldName.data, RSSL_DT_TIME));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["h"].IsNumber());
				EXPECT_EQ(22, _jsonDocument["d"]["d"][currentEntry]["d"]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["m"].IsNumber());
				EXPECT_EQ(4, _jsonDocument["d"]["d"][currentEntry]["d"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["s"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][currentEntry]["d"]["s"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("x"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["x"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][currentEntry]["d"]["x"].GetInt());
				++currentEntry;
			}

			/* Check DateTime element. */
			if (params.dateTime)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DATETIME_FIELD.fieldName.data, RSSL_DT_DATETIME));

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());

				/* Date */
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].HasMember("y"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"]["y"].IsNumber());
				EXPECT_EQ(1955, _jsonDocument["d"]["d"][currentEntry]["d"]["d"]["y"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"]["m"].IsNumber());
				EXPECT_EQ(11, _jsonDocument["d"]["d"][currentEntry]["d"]["d"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"]["d"].IsNumber());
				EXPECT_EQ(12, _jsonDocument["d"]["d"][currentEntry]["d"]["d"]["d"].GetInt());

				/* Time */
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].IsObject());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].HasMember("h"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"]["h"].IsNumber());
				EXPECT_EQ(22, _jsonDocument["d"]["d"][currentEntry]["d"]["t"]["h"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].HasMember("m"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"]["m"].IsNumber());
				EXPECT_EQ(4, _jsonDocument["d"]["d"][currentEntry]["d"]["t"]["m"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"]["s"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][currentEntry]["d"]["t"]["s"].GetInt());

				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].HasMember("x"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"]["x"].IsNumber());
				EXPECT_EQ(0, _jsonDocument["d"]["d"][currentEntry]["d"]["t"]["x"].GetInt());
				++currentEntry;
			}

			/* Check Qos element. */
			if (params.qos)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], QOS_FIELD.fieldName.data, RSSL_DT_QOS));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].IsNumber());
				EXPECT_EQ(RSSL_QOS_TIME_REALTIME, _jsonDocument["d"]["d"][currentEntry]["d"]["t"].GetInt());
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("r"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["r"].IsNumber());
				EXPECT_EQ(RSSL_QOS_RATE_TICK_BY_TICK, _jsonDocument["d"]["d"][currentEntry]["d"]["r"].GetInt());
				++currentEntry;
			}

			/* Check State element. */
			if (params.state)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], STATE_FIELD.fieldName.data, RSSL_DT_STATE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("s"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["s"].IsNumber());
				EXPECT_EQ(RSSL_STREAM_OPEN, _jsonDocument["d"]["d"][currentEntry]["d"]["s"].GetInt());
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].IsNumber());
				EXPECT_EQ(RSSL_DATA_OK, _jsonDocument["d"]["d"][currentEntry]["d"]["d"].GetInt());
				++currentEntry;
			}

			/* Check Enum element. */
			if (params.enumValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ENUM_FIELD.fieldName.data, RSSL_DT_ENUM));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNumber());
				EXPECT_EQ(rsslEnum, _jsonDocument["d"]["d"][currentEntry]["d"].GetInt());
				++currentEntry;
			}

			/* Check Array element. */
			if (params.array)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ARRAY_FIELD.fieldName.data, RSSL_DT_ARRAY));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsObject());

				/* Check array type. */
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("t"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["t"].IsNumber());
				EXPECT_EQ(RSSL_DT_INT, _jsonDocument["d"]["d"][currentEntry]["d"]["t"].GetInt());

				/* Check array data. */
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].HasMember("d"));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"]["d"].IsArray());
				const Value& intArray = _jsonDocument["d"]["d"][currentEntry]["d"]["d"];
				ASSERT_EQ(3, intArray.Size());
				ASSERT_TRUE(intArray[0].IsNumber());
				EXPECT_EQ(arrayInts[0], intArray[0].GetInt());
				ASSERT_TRUE(intArray[1].IsNumber());
				EXPECT_EQ(arrayInts[1], intArray[1].GetInt());
				ASSERT_TRUE(intArray[2].IsNumber());
				EXPECT_EQ(arrayInts[2], intArray[2].GetInt());
				++currentEntry;
			}

			/* Check Buffer element. */
			if (params.buffer)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], BUFFER_FIELD.fieldName.data, RSSL_DT_BUFFER));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, _jsonDocument["d"]["d"][currentEntry]["d"]));
				++currentEntry;
			}

			/* Check AsciiString element. */
			if (params.asciiString)
			{
				/* AsciiString is only a string (not an object with Type/Data members). */
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ASCII_STRING_FIELD.fieldName.data, RSSL_DT_ASCII_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsString());
				EXPECT_STREQ(ASCII_STRING.data, _jsonDocument["d"]["d"][currentEntry]["d"].GetString());
				++currentEntry;
			}

			/* Check Utf8String element. */
			if (params.utf8String)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], UTF8_STRING_FIELD.fieldName.data, RSSL_DT_UTF8_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsString());
				EXPECT_STREQ(UTF8_STRING.data, _jsonDocument["d"]["d"][currentEntry]["d"].GetString());
				++currentEntry;
			}

			/* Check RmtesString element. */
			if (params.rmtesString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], RMTES_STRING_FIELD.fieldName.data, RSSL_DT_RMTES_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsString());

				/* RMTES content should be converted to UTF8 in JSON (and then back to the RMTES string when converted back to RWF). */
				EXPECT_STREQ(RMTES_STRING_AS_UTF8.data, _jsonDocument["d"]["d"][currentEntry]["d"].GetString());
				++currentEntry;
			}

			/* Check Opaque element. */
			if (params.opaque)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], OPAQUE_FIELD.fieldName.data, RSSL_DT_OPAQUE));
				ASSERT_NO_FATAL_FAILURE(checkJsonBase64String(&OPAQUE_BUFFER, _jsonDocument["d"]["d"][currentEntry]["d"]));
				++currentEntry;
			}

			/* Check Xml element. */
			if (params.xml)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], XML_FIELD.fieldName.data, RSSL_DT_XML));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsString());
				EXPECT_STREQ(XML_BUFFER.data, _jsonDocument["d"]["d"][currentEntry]["d"].GetString());
				++currentEntry;
			}

			/* Check FieldList element. */
			if (params.fieldList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], FIELDLIST_FIELD.fieldName.data, RSSL_DT_FIELD_LIST));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check ElementList element. */
			if (params.elementList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ELEMENTLIST_FIELD.fieldName.data, RSSL_DT_ELEMENT_LIST));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check FilterList element. */
			if (params.filterList)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], FILTERLIST_FIELD.fieldName.data, RSSL_DT_FILTER_LIST));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonFilterList(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check Map element. */
			if (params.map)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], MAP_FIELD.fieldName.data, RSSL_DT_MAP));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonMap(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check Vector element. */
			if (params.vector)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], VECTOR_FIELD.fieldName.data, RSSL_DT_VECTOR));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonVector(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check Series element. */
			if (params.series)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], SERIES_FIELD.fieldName.data, RSSL_DT_SERIES));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonSeries(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* Check Message element. */
			if (params.msg)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], MSG_FIELD.fieldName.data, RSSL_DT_MSG));
				ASSERT_NO_FATAL_FAILURE(checkSampleJsonUpdateMsg(_jsonDocument["d"]["d"][currentEntry]["d"], params.protocolType));
				++currentEntry;
			}

			/* (Json container type Not supported in JSON1) */
			ASSERT_FALSE(params.json);

			/* Did we check every element? */
			ASSERT_EQ(currentEntry, _jsonDocument["d"]["d"].Size());
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
	EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));


	/* Ensure that the appropriate entries are found (and that elements that shouldn't be there aren't). 
	 * Don't assume that they will appear in order after conversion. */
	RsslRet ret;
	bool foundIntElement = false, foundUintElement = false, foundFloatElement = false, foundDoubleElement = false, foundRealElement = false,
		 foundDateElement = false, foundTimeElement = false, foundDateTimeElement = false, foundQosElement = false, foundStateElement = false,
		 foundEnumElement = false, foundArrayElement = false, foundBufferElement = false, foundAsciiStringElement = false, foundUtf8StringElement = false,
		 foundRmtesStringElement = false, foundOpaqueElement = false, foundXmlElement = false, foundFieldListElement = false,
		 foundElementListElement = false, foundFilterListElement = false, foundMapElement = false, foundVectorElement = false, foundSeriesElement = false,
		 foundMsgElement = false, foundJsonElement = false;

	while ((ret = rsslDecodeElementEntry(&_dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		std::string elementName(elementEntry.name.data, elementEntry.name.length);
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode ElementEntry with name: " << elementName;

		if (rsslBufferIsEqual(&INT_FIELD.fieldName, &elementEntry.name))
		{
			RsslInt decodeInt;
			ASSERT_EQ(RSSL_DT_INT, elementEntry.dataType);
			EXPECT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(&_dIter, &decodeInt));
			EXPECT_EQ(rsslInt, decodeInt);
			foundIntElement = true;
		}
		else if (rsslBufferIsEqual(&UINT_FIELD.fieldName, &elementEntry.name))
		{
			RsslUInt decodeUInt;
			ASSERT_EQ(RSSL_DT_UINT, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeUInt(&_dIter, &decodeUInt));
			EXPECT_EQ(rsslUInt, decodeUInt);
			foundUintElement = true;
		}
		else if (rsslBufferIsEqual(&FLOAT_FIELD.fieldName, &elementEntry.name))
		{
			RsslFloat decodeFloat;
			ASSERT_EQ(RSSL_DT_FLOAT, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFloat(&_dIter, &decodeFloat));
			EXPECT_NEAR(rsslFloat, decodeFloat, 0.099);
			foundFloatElement = true;
		}
		else if (rsslBufferIsEqual(&DOUBLE_FIELD.fieldName, &elementEntry.name))
		{
			RsslDouble decodeDouble;
			ASSERT_EQ(RSSL_DT_DOUBLE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDouble(&_dIter, &decodeDouble));
			EXPECT_NEAR(rsslDouble, decodeDouble, 0.099);
			foundDoubleElement = true;
		}
		else if (rsslBufferIsEqual(&REAL_FIELD.fieldName, &elementEntry.name))
		{
			RsslReal decodeReal;
			ASSERT_EQ(RSSL_DT_REAL, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(&_dIter, &decodeReal));
			EXPECT_EQ(REAL.isBlank, decodeReal.isBlank);
			EXPECT_EQ(REAL.hint, decodeReal.hint);
			EXPECT_EQ(REAL.value, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
			foundRealElement = true;
		}
		else if (rsslBufferIsEqual(&DATE_FIELD.fieldName, &elementEntry.name))
		{
			RsslDate decodeDate;
			ASSERT_EQ(RSSL_DT_DATE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeDate(&_dIter, &decodeDate));

			ASSERT_EQ(DATETIME.date.day, decodeDate.day);
			ASSERT_EQ(DATETIME.date.month, decodeDate.month);
			ASSERT_EQ(DATETIME.date.year, decodeDate.year);
			foundDateElement = true;
		}
		else if (rsslBufferIsEqual(&TIME_FIELD.fieldName, &elementEntry.name))
		{
			RsslTime decodeTime;
			ASSERT_EQ(RSSL_DT_TIME, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeTime(&_dIter, &decodeTime));

			ASSERT_EQ(DATETIME.time.hour, decodeTime.hour);
			ASSERT_EQ(DATETIME.time.minute, decodeTime.minute);
			ASSERT_EQ(DATETIME.time.second, decodeTime.second);
			ASSERT_EQ(DATETIME.time.millisecond, decodeTime.millisecond);
			ASSERT_EQ(DATETIME.time.microsecond, decodeTime.microsecond);
			ASSERT_EQ(DATETIME.time.nanosecond, decodeTime.nanosecond);
			foundTimeElement = true;
		}
		else if (rsslBufferIsEqual(&DATETIME_FIELD.fieldName, &elementEntry.name))
		{
			RsslDateTime decodeDateTime;
			ASSERT_EQ(RSSL_DT_DATETIME, elementEntry.dataType);
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
			foundDateTimeElement = true;
		}
		else if (rsslBufferIsEqual(&QOS_FIELD.fieldName, &elementEntry.name))
		{
			RsslQos decodeQos;
			ASSERT_EQ(RSSL_DT_QOS, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeQos(&_dIter, &decodeQos));
			EXPECT_TRUE(rsslQosIsEqual(&QOS, &decodeQos));
			foundQosElement = true;
		}
		else if (rsslBufferIsEqual(&STATE_FIELD.fieldName, &elementEntry.name))
		{
			RsslState decodeState;
			ASSERT_EQ(RSSL_DT_STATE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeState(&_dIter, &decodeState));

			ASSERT_EQ(STATE.streamState, decodeState.streamState);
			ASSERT_EQ(STATE.dataState, decodeState.dataState);
			ASSERT_EQ(STATE.code, decodeState.code);
			foundStateElement = true;
		}
		else if (rsslBufferIsEqual(&ENUM_FIELD.fieldName, &elementEntry.name))
		{
			RsslEnum decodeEnum;
			ASSERT_EQ(RSSL_DT_ENUM, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeEnum(&_dIter, &decodeEnum));
			EXPECT_EQ(rsslEnum, decodeEnum);
			foundEnumElement = true;
		}
		else if (rsslBufferIsEqual(&ARRAY_FIELD.fieldName, &elementEntry.name))
		{
			RsslArray decodeArray;
			RsslBuffer decodeArrayEntry;
			ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);
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
			foundArrayElement = true;
		}
		else if (rsslBufferIsEqual(&BUFFER_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_BUFFER, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&OPAQUE_BUFFER, &decodeBuffer));
			foundBufferElement = true;
		}
		else if (rsslBufferIsEqual(&ASCII_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_ASCII_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&ASCII_STRING, &decodeBuffer));
			foundAsciiStringElement = true;
		}
		else if (rsslBufferIsEqual(&UTF8_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_UTF8_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&UTF8_STRING, &decodeBuffer));
			foundUtf8StringElement = true;
		}
		else if (rsslBufferIsEqual(&RMTES_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_RMTES_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
            /* RMTES conversion only works from ADS to client. ADS will not convert incoming strings to RMTES */
			EXPECT_TRUE(rsslBufferIsEqual(&RMTES_STRING_AS_UTF8, &decodeBuffer));
			foundRmtesStringElement = true;
		}
		else if (rsslBufferIsEqual(&OPAQUE_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeOpaque;
			ASSERT_EQ(RSSL_DT_OPAQUE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeOpaque));
			EXPECT_TRUE(rsslBufferIsEqual(&OPAQUE_BUFFER, &decodeOpaque));
			foundOpaqueElement = true;
		}
		else if (rsslBufferIsEqual(&XML_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_XML, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			EXPECT_TRUE(rsslBufferIsEqual(&XML_BUFFER, &decodeBuffer));
			foundXmlElement = true;
		}
		else if (rsslBufferIsEqual(&FIELDLIST_FIELD.fieldName, &elementEntry.name))
		{
			ASSERT_EQ(RSSL_DT_FIELD_LIST, elementEntry.dataType);
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));
			foundFieldListElement = true;
		}
		else if (rsslBufferIsEqual(&ELEMENTLIST_FIELD.fieldName, &elementEntry.name))
		{
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST, elementEntry.dataType);
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));
			foundElementListElement = true;
		}
		else if (rsslBufferIsEqual(&FILTERLIST_FIELD.fieldName, &elementEntry.name))
		{
			RsslFilterList decodeFilterList;
			RsslFilterEntry decodeFilterEntry;

			ASSERT_EQ(RSSL_DT_FILTER_LIST, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(&_dIter, &decodeFilterList));

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(&_dIter, &decodeFilterEntry));
			EXPECT_EQ(1, decodeFilterEntry.id);
			EXPECT_EQ(RSSL_FTEA_SET_ENTRY, decodeFilterEntry.action);
			EXPECT_EQ(RSSL_DT_ELEMENT_LIST, decodeFilterEntry.containerType);

			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(params.protocolType, &_dIter));

			ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(&_dIter, &decodeFilterEntry));

			foundFilterListElement = true;
		}
		else if (rsslBufferIsEqual(&MAP_FIELD.fieldName, &elementEntry.name))
		{
			RsslMap decodeMap;
			RsslMapEntry decodeMapEntry;
			RsslInt decodeMapEntryKey;

			ASSERT_EQ(RSSL_DT_MAP, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(&_dIter, &decodeMap));
			ASSERT_EQ(RSSL_DT_INT, decodeMap.keyPrimitiveType);
			ASSERT_EQ(RSSL_DT_FIELD_LIST, decodeMap.containerType);

			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(&_dIter, &decodeMapEntry, (void*)&decodeMapEntryKey));
			EXPECT_EQ(1, decodeMapEntryKey);
			EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, decodeMapEntry.action);

			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(params.protocolType, &_dIter));

			ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(&_dIter, &decodeMapEntry, (void*)&decodeMapEntryKey));
			foundMapElement = true;
		}
		else if (rsslBufferIsEqual(&VECTOR_FIELD.fieldName, &elementEntry.name))
		{
			ASSERT_EQ(RSSL_DT_VECTOR, elementEntry.dataType);
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslVector(params.protocolType, &_dIter));
			foundVectorElement = true;
		}
		else if (rsslBufferIsEqual(&SERIES_FIELD.fieldName, &elementEntry.name))
		{
			ASSERT_EQ(RSSL_DT_SERIES, elementEntry.dataType);
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslSeries(params.protocolType, &_dIter));
			foundSeriesElement = true;
		}
		else if (rsslBufferIsEqual(&MSG_FIELD.fieldName, &elementEntry.name))
		{
			ASSERT_EQ(RSSL_DT_MSG, elementEntry.dataType);
			ASSERT_NO_FATAL_FAILURE(decodeSampleRsslUpdateMsg(params.protocolType, &_dIter));
			foundMsgElement = true;
		}
		else if (rsslBufferIsEqual(&JSON_FIELD.fieldName, &elementEntry.name))
		{
			Document document;
			RsslBuffer decodeJsonBuffer;

			ASSERT_EQ(RSSL_DT_JSON, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeBuffer(&_dIter, &decodeJsonBuffer));

			document.Parse(decodeJsonBuffer.data);
            ASSERT_TRUE(NULL == document.GetParseError()) << "** JSON Parse Error: " << document.GetParseError() << "\n";
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonObject(Value(document)));
			foundJsonElement = true;
		}
		else
			FAIL() << "** Decoded ElementEntry with unknown name: " << elementName;
	}

	ASSERT_EQ(params.intValue, foundIntElement);
	ASSERT_EQ(params.uintValue, foundUintElement);
	ASSERT_EQ(params.floatValue, foundFloatElement);
	ASSERT_EQ(params.doubleValue, foundDoubleElement);
	ASSERT_EQ(params.real, foundRealElement);
	ASSERT_EQ(params.date, foundDateElement);
	ASSERT_EQ(params.time, foundTimeElement);
	ASSERT_EQ(params.dateTime, foundDateTimeElement);
	ASSERT_EQ(params.qos, foundQosElement);
	ASSERT_EQ(params.state, foundStateElement);
	ASSERT_EQ(params.enumValue, foundEnumElement);
	ASSERT_EQ(params.array, foundArrayElement);
	ASSERT_EQ(params.buffer, foundBufferElement);
	ASSERT_EQ(params.asciiString, foundAsciiStringElement);
	ASSERT_EQ(params.utf8String, foundUtf8StringElement);
	ASSERT_EQ(params.rmtesString, foundRmtesStringElement);
	ASSERT_EQ(params.opaque, foundOpaqueElement);
	ASSERT_EQ(params.xml, foundXmlElement);
	ASSERT_EQ(params.fieldList, foundFieldListElement);
	ASSERT_EQ(params.elementList, foundElementListElement);
	ASSERT_EQ(params.filterList, foundFilterListElement);
	ASSERT_EQ(params.map, foundMapElement);
	ASSERT_EQ(params.vector, foundVectorElement);
	ASSERT_EQ(params.series, foundSeriesElement);
	ASSERT_EQ(params.msg, foundMsgElement);
	ASSERT_EQ(params.json, foundJsonElement);
}

INSTANTIATE_TEST_CASE_P(ElementListTests, ElementListTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Nothing */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2),

	/* Int */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT),

	/* UInt */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT),

	/* Float */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT),

	/* Double */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE),

	/* Real */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL),

	/* Date */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE),

	/* Time */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME),

	/* DateTime */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME),

	/* QoS */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS),

	/* State */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE),

	/* Enum */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM),

	/* Array */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ARRAY),

	/* Buffer */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER),

	/* AsciiString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING),

	/* Utf8String */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING),

	/* RmtesString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING),

	/* Opaque */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_OPAQUE),

	/* Xml */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_XML),

	/* FieldList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FIELD_LIST),

	/* ElementList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ELEMENT_LIST),

	/* FilterList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FILTER_LIST),

	/* Map */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_MAP),

	/* Vector */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_VECTOR),

	/* Series */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_SERIES),

	/* Message */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_MSG),

	/* Json */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_JSON),

	/* Everything */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, sizeof(allDataTypes)/sizeof(RsslUInt8), allDataTypes)

));

INSTANTIATE_TEST_CASE_P(ElementListTestsJson1, ElementListTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Nothing */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON),

	/* Int */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT),

	/* UInt */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT),

	/* Float */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT),

	/* Double */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE),

	/* Real */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL),

	/* Date */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE),

	/* Time */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME),

	/* DateTime */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME),

	/* QoS */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS),

	/* State */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE),

	/* Enum */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM),

	/* Array */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ARRAY),

	/* Buffer */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER),

	/* AsciiString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING),

	/* Utf8String */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING),

	/* RmtesString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING),

	/* Opaque */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_OPAQUE),

	/* Xml */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_XML),

	/* FieldList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FIELD_LIST),

	/* ElementList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ELEMENT_LIST),

	/* FilterList */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FILTER_LIST),

	/* Map */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_MAP),

	/* Vector */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_VECTOR),

	/* Series */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_SERIES),

	/* Message */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_MSG),

	/* (Json container type not supported in JSON1) */

	/* Everything */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, sizeof(json1AllDataTypes)/sizeof(RsslUInt8), json1AllDataTypes)

));

class ElementListPrimitiveTypesTestFixture : public MsgConversionTestBase, public ::testing::WithParamInterface<ElementListTypesTestParams>
{
};

/* Test a element list that encodes different data types as blank in its element entries. */
TEST_P(ElementListPrimitiveTypesTestFixture, ElementListBlankTests)
{
	ElementListTypesTestParams const &params = GetParam();
	RsslUpdateMsg updateMsg;
	RsslMsg rsslMsg;
	RsslElementList elementList;
	RsslElementEntry elementEntry;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_ELEMENT_LIST;
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

	/* Encode an element list whose entries encode different data types. */
	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(&_eIter, &elementList, NULL, 0));
	
	/* Encode Int element. */
	if (params.intValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = INT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_INT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode UInt element. */
	if (params.uintValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = UINT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_UINT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Float element. */
	if (params.floatValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = FLOAT_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_FLOAT;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Double element. */
	if (params.doubleValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DOUBLE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DOUBLE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Real element. */
	if (params.real)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = REAL_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_REAL;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Date element. */
	if (params.date)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DATE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Time element. */
	if (params.time)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = TIME_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_TIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode DateTime element. */
	if (params.dateTime)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = DATETIME_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_DATETIME;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Qos element. */
	if (params.qos)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = QOS_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_QOS;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode State element. */
	if (params.state)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = STATE_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_STATE;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Enum element. */
	if (params.enumValue)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ENUM_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ENUM;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Array element. */
	if (params.array)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ARRAY_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ARRAY;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Buffer element. */
	if (params.buffer)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = BUFFER_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_BUFFER;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode AsciiString element. */
	if (params.asciiString)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = ASCII_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_ASCII_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode Utf8String element. */
	if (params.utf8String)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = UTF8_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_UTF8_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Encode RmtesString element. */
	if (params.rmtesString)
	{
		rsslClearElementEntry(&elementEntry);
		elementEntry.name = RMTES_STRING_FIELD.fieldName;
		elementEntry.dataType = RSSL_DT_RMTES_STRING;
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(&_eIter, &elementEntry, NULL));
	}

	/* Complete encoding ElementList and Message. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(&_eIter, RSSL_TRUE));
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

			/* Check ElementList. */
			ASSERT_TRUE(_jsonDocument.HasMember("Elements"));
			ASSERT_TRUE(_jsonDocument["Elements"].IsObject());

			/* Check Int element. */
			if (params.intValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], INT_FIELD.fieldName.data, "Int"));
				ASSERT_TRUE(_jsonDocument["Elements"][INT_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(INT_FIELD.fieldName.data));

			/* Check UInt element. */
			if (params.uintValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], UINT_FIELD.fieldName.data, "UInt"));
				ASSERT_TRUE(_jsonDocument["Elements"][UINT_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(UINT_FIELD.fieldName.data));

			/* Check Float element. */
			if (params.floatValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], FLOAT_FIELD.fieldName.data, "Float"));
				ASSERT_TRUE(_jsonDocument["Elements"][FLOAT_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(FLOAT_FIELD.fieldName.data));

			/* Check Double element. */
			if (params.doubleValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DOUBLE_FIELD.fieldName.data, "Double"));
				ASSERT_TRUE(_jsonDocument["Elements"][DOUBLE_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DOUBLE_FIELD.fieldName.data));

			/* Check Real element. */
			if (params.real)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], REAL_FIELD.fieldName.data, "Real"));
				ASSERT_TRUE(_jsonDocument["Elements"][REAL_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(REAL_FIELD.fieldName.data));

			/* Check Date element. */
			if (params.date)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DATE_FIELD.fieldName.data, "Date"));
				ASSERT_TRUE(_jsonDocument["Elements"][DATE_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DATE_FIELD.fieldName.data));

			/* Check Time element. */
			if (params.time)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], TIME_FIELD.fieldName.data, "Time"));
				ASSERT_TRUE(_jsonDocument["Elements"][TIME_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(TIME_FIELD.fieldName.data));

			/* Check DateTime element. */
			if (params.dateTime)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], DATETIME_FIELD.fieldName.data, "DateTime"));
				ASSERT_TRUE(_jsonDocument["Elements"][DATETIME_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(DATETIME_FIELD.fieldName.data));

			/* Check Qos element. */
			if (params.qos)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], QOS_FIELD.fieldName.data, "Qos"));
				ASSERT_TRUE(_jsonDocument["Elements"][QOS_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(QOS_FIELD.fieldName.data));

			/* Check State element. */
			if (params.state)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], STATE_FIELD.fieldName.data, "State"));
				ASSERT_TRUE(_jsonDocument["Elements"][STATE_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(STATE_FIELD.fieldName.data));

			/* Check Enum element. */
			if (params.enumValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ENUM_FIELD.fieldName.data, "Enum"));
				ASSERT_TRUE(_jsonDocument["Elements"][ENUM_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ENUM_FIELD.fieldName.data));

			/* Check Array element. */
			if (params.array)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ARRAY_FIELD.fieldName.data, "Array"));
				ASSERT_TRUE(_jsonDocument["Elements"][ARRAY_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ARRAY_FIELD.fieldName.data));

			/* Check Buffer element. */
			if (params.buffer)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], BUFFER_FIELD.fieldName.data, "Buffer"));
				ASSERT_TRUE(_jsonDocument["Elements"][BUFFER_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(BUFFER_FIELD.fieldName.data));

			/* Check AsciiString element. */
			if (params.asciiString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], ASCII_STRING_FIELD.fieldName.data, "AsciiString"));
				ASSERT_TRUE(_jsonDocument["Elements"][ASCII_STRING_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(ASCII_STRING_FIELD.fieldName.data));

			/* Check Utf8String element. */
			if (params.utf8String)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], UTF8_STRING_FIELD.fieldName.data, "Utf8String"));
				ASSERT_TRUE(_jsonDocument["Elements"][UTF8_STRING_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(UTF8_STRING_FIELD.fieldName.data));

			/* Check RmtesString element. */
			if (params.rmtesString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJsonElementEntryNameAndFormat(_jsonDocument["Elements"], RMTES_STRING_FIELD.fieldName.data, "RmtesString"));
				ASSERT_TRUE(_jsonDocument["Elements"][RMTES_STRING_FIELD.fieldName.data]["Data"].IsNull());
			}
			else
				EXPECT_FALSE(_jsonDocument["Elements"].HasMember(RMTES_STRING_FIELD.fieldName.data));
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
			EXPECT_EQ(RSSL_DT_ELEMENT_LIST - 128, msgBase["f"].GetInt());

			/* Check UpdateType. */
			ASSERT_TRUE(_jsonDocument.HasMember("u"));
			ASSERT_TRUE(_jsonDocument["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, _jsonDocument["u"].GetInt());

			/* Check ElementList. */
			ASSERT_TRUE(_jsonDocument.HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"].IsObject());

			ASSERT_TRUE(_jsonDocument["d"].HasMember("d"));
			ASSERT_TRUE(_jsonDocument["d"]["d"].IsArray());
			ASSERT_EQ(params.memberCount, _jsonDocument["d"]["d"].Size());

			int currentEntry = 0;

			/* Check Int element. */
			if (params.intValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], INT_FIELD.fieldName.data, RSSL_DT_INT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check UInt element. */
			if (params.uintValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], UINT_FIELD.fieldName.data, RSSL_DT_UINT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Float element. */
			if (params.floatValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], FLOAT_FIELD.fieldName.data, RSSL_DT_FLOAT));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Double element. */
			if (params.doubleValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DOUBLE_FIELD.fieldName.data, RSSL_DT_DOUBLE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Real element. */
			if (params.real)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], REAL_FIELD.fieldName.data, RSSL_DT_REAL));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Date element. */
			if (params.date)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DATE_FIELD.fieldName.data, RSSL_DT_DATE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Time element. */
			if (params.time)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], TIME_FIELD.fieldName.data, RSSL_DT_TIME));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check DateTime element. */
			if (params.dateTime)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], DATETIME_FIELD.fieldName.data, RSSL_DT_DATETIME));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Qos element. */
			if (params.qos)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], QOS_FIELD.fieldName.data, RSSL_DT_QOS));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check State element. */
			if (params.state)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], STATE_FIELD.fieldName.data, RSSL_DT_STATE));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Enum element. */
			if (params.enumValue)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ENUM_FIELD.fieldName.data, RSSL_DT_ENUM));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Array element. */
			if (params.array)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ARRAY_FIELD.fieldName.data, RSSL_DT_ARRAY));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Buffer element. */
			if (params.buffer)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], BUFFER_FIELD.fieldName.data, RSSL_DT_BUFFER));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check AsciiString element. */
			if (params.asciiString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], ASCII_STRING_FIELD.fieldName.data, RSSL_DT_ASCII_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check Utf8String element. */
			if (params.utf8String)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], UTF8_STRING_FIELD.fieldName.data, RSSL_DT_UTF8_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Check RmtesString element. */
			if (params.rmtesString)
			{
				ASSERT_NO_FATAL_FAILURE(checkJson1ElementEntryNameAndFormat(_jsonDocument["d"]["d"][currentEntry], RMTES_STRING_FIELD.fieldName.data, RSSL_DT_RMTES_STRING));
				ASSERT_TRUE(_jsonDocument["d"]["d"][currentEntry]["d"].IsNull());
				++currentEntry;
			}

			/* Did we check every element? */
			ASSERT_EQ(currentEntry, _jsonDocument["d"]["d"].Size());

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
	EXPECT_EQ(RSSL_DT_ELEMENT_LIST, rsslMsg.msgBase.containerType);
	EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateMsg.updateType);

	/* Check FieldList. */
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(&_dIter, &elementList, NULL));


	/* Ensure that the appropriate entries are found (and that elements that shouldn't be there aren't). 
	 * Don't assume that they will appear in order after conversion. */
	RsslRet ret;
	bool foundIntElement = false, foundUintElement = false, foundFloatElement = false, foundDoubleElement = false, foundRealElement = false,
		 foundDateElement = false, foundTimeElement = false, foundDateTimeElement = false, foundQosElement = false, foundStateElement = false,
		 foundEnumElement = false, foundArrayElement = false, foundBufferElement = false, foundAsciiStringElement = false, foundUtf8StringElement = false,
		 foundRmtesStringElement = false;

	while ((ret = rsslDecodeElementEntry(&_dIter, &elementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		std::string elementName(elementEntry.name.data, elementEntry.name.length);
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode ElementEntry with name: " << elementName;

		if (rsslBufferIsEqual(&INT_FIELD.fieldName, &elementEntry.name))
		{
			RsslInt decodeInt;
			ASSERT_EQ(RSSL_DT_INT, elementEntry.dataType);
			EXPECT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeInt(&_dIter, &decodeInt));
			foundIntElement = true;
		}
		else if (rsslBufferIsEqual(&UINT_FIELD.fieldName, &elementEntry.name))
		{
			RsslUInt decodeUInt;
			ASSERT_EQ(RSSL_DT_UINT, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeUInt(&_dIter, &decodeUInt));
			foundUintElement = true;
		}
		else if (rsslBufferIsEqual(&FLOAT_FIELD.fieldName, &elementEntry.name))
		{
			RsslFloat decodeFloat;
			ASSERT_EQ(RSSL_DT_FLOAT, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeFloat(&_dIter, &decodeFloat));
			foundFloatElement = true;
		}
		else if (rsslBufferIsEqual(&DOUBLE_FIELD.fieldName, &elementEntry.name))
		{
			RsslDouble decodeDouble;
			ASSERT_EQ(RSSL_DT_DOUBLE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDouble(&_dIter, &decodeDouble));
			foundDoubleElement = true;
		}
		else if (rsslBufferIsEqual(&REAL_FIELD.fieldName, &elementEntry.name))
		{
			RsslReal decodeReal;
			ASSERT_EQ(RSSL_DT_REAL, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeReal(&_dIter, &decodeReal));
			foundRealElement = true;
		}
		else if (rsslBufferIsEqual(&DATE_FIELD.fieldName, &elementEntry.name))
		{
			RsslDate decodeDate;
			ASSERT_EQ(RSSL_DT_DATE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDate(&_dIter, &decodeDate));
			foundDateElement = true;
		}
		else if (rsslBufferIsEqual(&TIME_FIELD.fieldName, &elementEntry.name))
		{
			RsslTime decodeTime;
			ASSERT_EQ(RSSL_DT_TIME, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeTime(&_dIter, &decodeTime));
			foundTimeElement = true;
		}
		else if (rsslBufferIsEqual(&DATETIME_FIELD.fieldName, &elementEntry.name))
		{
			RsslDateTime decodeDateTime;
			ASSERT_EQ(RSSL_DT_DATETIME, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeDateTime(&_dIter, &decodeDateTime));
			foundDateTimeElement = true;
		}
		else if (rsslBufferIsEqual(&QOS_FIELD.fieldName, &elementEntry.name))
		{
			RsslQos decodeQos;
			ASSERT_EQ(RSSL_DT_QOS, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeQos(&_dIter, &decodeQos));
			foundQosElement = true;
		}
		else if (rsslBufferIsEqual(&STATE_FIELD.fieldName, &elementEntry.name))
		{
			RsslState decodeState;
			ASSERT_EQ(RSSL_DT_STATE, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeState(&_dIter, &decodeState));
			foundStateElement = true;
		}
		else if (rsslBufferIsEqual(&ENUM_FIELD.fieldName, &elementEntry.name))
		{
			RsslEnum decodeEnum;
			ASSERT_EQ(RSSL_DT_ENUM, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeEnum(&_dIter, &decodeEnum));
			foundEnumElement = true;
		}
		else if (rsslBufferIsEqual(&ARRAY_FIELD.fieldName, &elementEntry.name))
		{
			RsslArray decodeArray;
			ASSERT_EQ(RSSL_DT_ARRAY, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeArray(&_dIter, &decodeArray));
			foundArrayElement = true;
		}
		else if (rsslBufferIsEqual(&BUFFER_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_BUFFER, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundBufferElement = true;
		}
		else if (rsslBufferIsEqual(&ASCII_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_ASCII_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundAsciiStringElement = true;
		}
		else if (rsslBufferIsEqual(&UTF8_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_UTF8_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundUtf8StringElement = true;
		}
		else if (rsslBufferIsEqual(&RMTES_STRING_FIELD.fieldName, &elementEntry.name))
		{
			RsslBuffer decodeBuffer;
			ASSERT_EQ(RSSL_DT_RMTES_STRING, elementEntry.dataType);
			ASSERT_EQ(RSSL_RET_BLANK_DATA, rsslDecodeBuffer(&_dIter, &decodeBuffer));
			foundRmtesStringElement = true;
		}
		else
			FAIL() << "** Decoded ElementEntry with unknown name: " << elementName;
	}

	ASSERT_EQ(params.intValue, foundIntElement);
	ASSERT_EQ(params.uintValue, foundUintElement);
	ASSERT_EQ(params.floatValue, foundFloatElement);
	ASSERT_EQ(params.doubleValue, foundDoubleElement);
	ASSERT_EQ(params.real, foundRealElement);
	ASSERT_EQ(params.date, foundDateElement);
	ASSERT_EQ(params.time, foundTimeElement);
	ASSERT_EQ(params.dateTime, foundDateTimeElement);
	ASSERT_EQ(params.qos, foundQosElement);
	ASSERT_EQ(params.state, foundStateElement);
	ASSERT_EQ(params.enumValue, foundEnumElement);
	ASSERT_EQ(params.array, foundArrayElement);
	ASSERT_EQ(params.buffer, foundBufferElement);
	ASSERT_EQ(params.asciiString, foundAsciiStringElement);
	ASSERT_EQ(params.utf8String, foundUtf8StringElement);
	ASSERT_EQ(params.rmtesString, foundRmtesStringElement);
}

INSTANTIATE_TEST_CASE_P(ElementListTests, ElementListPrimitiveTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Int */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_INT),

	/* UInt */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UINT),

	/* Float */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_FLOAT),

	/* Double */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DOUBLE),

	/* Real */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_REAL),

	/* Date */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATE),

	/* Time */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_TIME),

	/* DateTime */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_DATETIME),

	/* QoS */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_QOS),

	/* State */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_STATE),

	/* Enum */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ENUM),

	/* Array */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ARRAY),

	/* Buffer */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_BUFFER),

	/* AsciiString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_ASCII_STRING),

	/* Utf8String */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_UTF8_STRING),

	/* RmtesString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, RSSL_DT_RMTES_STRING),

	/* Everything */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON2, sizeof(allPrimitiveTypes)/sizeof(RsslUInt8), allPrimitiveTypes)

));

INSTANTIATE_TEST_CASE_P(ElementListTestsJson1, ElementListPrimitiveTypesTestFixture, ::testing::Values(
	/* Test encoding any of Int, UInt, Float, Double, Real, Date, Time, DateTime, QoS, State, Enum, Array, Buffer, AsciiString,
	 * Utf8String, RmtesString, Opaque, Xml, FieldList, ElementList, FilterList, Map, Message, and Json. */

	/* Int */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_INT),

	/* UInt */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UINT),

	/* Float */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_FLOAT),

	/* Double */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DOUBLE),

	/* Real */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_REAL),

	/* Date */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATE),

	/* Time */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_TIME),

	/* DateTime */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_DATETIME),

	/* QoS */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_QOS),

	/* State */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_STATE),

	/* Enum */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ENUM),

	/* Array */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ARRAY),

	/* Buffer */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_BUFFER),

	/* AsciiString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_ASCII_STRING),

	/* Utf8String */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_UTF8_STRING),

	/* RmtesString */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, RSSL_DT_RMTES_STRING),

	/* Everything */
	ElementListTypesTestParams(RSSL_JSON_JPT_JSON, sizeof(allPrimitiveTypes)/sizeof(RsslUInt8), allPrimitiveTypes)

));

TEST_F(ElementListTests, InvalidElementListTests)
{
	/* Type and Data missing from ElementEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Elements\":{\"Leeloo\":{}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key "));

	/* Data missing from ElementEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Elements\":{\"Leeloo\":{\"Type\":\"Buffer\"}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key "));

	/* Type missing from ElementEntry. */
	setJsonBufferToString("{\"Type\":\"Generic\",\"ID\":2,\"Domain\":128,\"SeqNumber\":3,\"Elements\":{\"Leeloo\":{\"Data\":{}}}}");
	ASSERT_NO_FATAL_FAILURE(getJsonToRsslError());

	ASSERT_TRUE(_jsonDocument.HasMember("Type"));
	ASSERT_TRUE(_jsonDocument["Type"].IsString());
	EXPECT_STREQ("Error", _jsonDocument["Type"].GetString());
	ASSERT_TRUE(::testing::internal::RE::PartialMatch(_jsonDocument["Text"].GetString(), "JSON Missing required key "));
}
