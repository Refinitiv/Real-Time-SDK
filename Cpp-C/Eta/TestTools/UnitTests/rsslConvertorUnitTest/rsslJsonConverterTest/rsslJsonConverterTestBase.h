/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_JSON_CONVERTER_TEST_BASE_H__
#define __RSSL_JSON_CONVERTER_TEST_BASE_H__

#include "gtest/gtest.h"

#include <cjson/cJSON.h>
#include "cJsonCppWrapper/cJsonCppWrapper.h"

#include "rtr/rsslJsonConverter.h"
#ifdef _RSSLJC_SHARED_LIBRARY
#include "rtr/rsslJsonConverterJit.h"
#endif
#include "rtr/rsslMsgDecoders.h"
#include "rtr/jsonToRwfBase.h"
#include "rtr/jsonSimpleDefs.h"

#include "decodeRoutines.h"

using namespace std;
using namespace json;

/* Commandline options that may aid in debugging. */
extern bool cmdlPrintJsonBuffer;	/** Print JSON buffers after conversion from RWF. */
extern bool cmdlPrintRsslBuffer;	/** Print RWF buffers after conversion from JSON. */

/* Sample Data commonly used by tests, and their generally-expected use case. */
extern const RsslBuffer			MSG_KEY_NAME;			/** Msg.Key.Name */
extern const RsslBuffer			MSG_KEY_NAME_ESC_CHAR;	/** Msg.Key.Name */
extern const RsslBuffer			REQ_MSG_KEY_NAME;		/** Req.MsgKey.Name */
extern const RsslBuffer			STATE_TEXT;				/** State.Text */
extern const RsslBuffer			EXTENDED_HEADER;		/** Msg.ExtHdr */
extern const RsslBuffer			PERM_DATA;				/** Msg.PermData */
extern const RsslBuffer			ACK_TEXT;				/** Ack.Text */
extern const RsslUInt16			MSGKEY_SVC_ID;			/** Msg.Key.Service */
extern const RsslInt32			MSGKEY_IDENTIFIER;		/** Msg.Key.Identifier */
extern const RsslUInt16			REQ_MSGKEY_SVC_ID;		/** Msg.ReqKey.Service */
extern const char				*IP_ADDRESS_STR;		/** PostUserInfo.Address. IP_ADDRESS_STR and IP_ADDRESS_UINT represent the same IP address */
extern const RsslUInt32			IP_ADDRESS_UINT;		/** PostUserInfo.Address. IP_ADDRESS_STR and IP_ADDRESS_UINT represent the same IP address */
extern const RsslUInt32			USER_ID;				/** PostUserInfo.UserId */
extern const RsslUInt32			SEQ_NUM;				/** Msg.SeqNum */
extern const RsslUInt32			SECONDARY_SEQ_NUM;		/** GenericMsg.SeqNum */
extern const RsslUInt16			PART_NUM;				/** Msg.SeqNum */
extern const RsslBuffer			GROUP_ID;				/** Msg.GroupId (used to help ensure Group ID is not present in JSON) */
extern const RsslBuffer			JSON_BUFFER;			/** Json Container. */
extern const RsslBuffer			OPAQUE_BUFFER;			/** Opaque container. */
extern const RsslBuffer			ASCII_STRING;			/** AsciiString primtive. */
extern const RsslBuffer			UTF8_STRING;			/** Utf8String primitive. */
extern const RsslBuffer			RMTES_STRING;			/** RMTESString primitive. */
extern const RsslBuffer			RMTES_STRING_AS_UTF8;	/** RMTESString primitive, as it would appear in JSON. */
extern const RsslBuffer			XML_BUFFER;				/** Xml Container */
extern const RsslBuffer			ANSI_PAGE_BUFFER;		/** ANSI PAGE Container */
extern const RsslBuffer			SERVICE_NAME;			/** Service Name corresponding to MSGKEY_SVC_ID. */
extern const RsslReal			&REAL;					/** Real primitive. */
extern const RsslQos			&QOS;					/** Qos primitive. */
extern const RsslDate			&DATE;					/** Date primitive. */
extern const RsslTime			&TIME;					/** Time primitive. */
extern const RsslDateTime		&DATETIME;				/** DateTime primitive. */
extern const RsslState			&STATE;					/** Msg.State, or State primitive. */

/* Fields that the test adds to the RDMFieldDictionary */
typedef struct
{
	RsslFieldId			fieldId;		/** Field ID. */
	const char*			fieldIdString;	/** Field ID as a string. */
	const RsslBuffer	fieldName;		/** Field Name */
} CustomField;

/* Fields that the test adds to the RDMFieldDictionary. The name of each field indicates its type (e.g. INT_FIELD is a field of type RSSL_DT_INT). */
extern const CustomField INT_FIELD;
extern const CustomField UINT_FIELD;
extern const CustomField REAL_FIELD;
extern const CustomField FLOAT_FIELD;
extern const CustomField DOUBLE_FIELD;
extern const CustomField DATE_FIELD;
extern const CustomField TIME_FIELD;
extern const CustomField DATETIME_FIELD;
extern const CustomField QOS_FIELD;
extern const CustomField STATE_FIELD;
extern const CustomField ENUM_FIELD;
extern const CustomField BUFFER_FIELD;
extern const CustomField ASCII_STRING_FIELD;
extern const CustomField UTF8_STRING_FIELD;
extern const CustomField RMTES_STRING_FIELD;
extern const CustomField XML_FIELD;
extern const CustomField FIELDLIST_FIELD;
extern const CustomField ELEMENTLIST_FIELD;
extern const CustomField FILTERLIST_FIELD;
extern const CustomField MAP_FIELD;
extern const CustomField VECTOR_FIELD;
extern const CustomField SERIES_FIELD;
extern const CustomField MSG_FIELD;
extern const CustomField JSON_FIELD;
extern const CustomField OPAQUE_FIELD;
extern const CustomField ARRAY_FIELD;
extern const CustomField ANSI_PAGE_FIELD;

/* Array containing all supported primitive types. */
static RsslUInt8 allPrimitiveTypes[] =
{
	RSSL_DT_INT, RSSL_DT_UINT, RSSL_DT_FLOAT, RSSL_DT_DOUBLE, RSSL_DT_REAL, RSSL_DT_DATE, RSSL_DT_TIME, RSSL_DT_DATETIME, RSSL_DT_QOS, RSSL_DT_STATE,
	RSSL_DT_ENUM, RSSL_DT_ARRAY, RSSL_DT_BUFFER, RSSL_DT_ASCII_STRING, RSSL_DT_UTF8_STRING, RSSL_DT_RMTES_STRING
};

/* Array containing all supported data types. */
static RsslUInt8 allDataTypes[] =
{
	RSSL_DT_INT, RSSL_DT_UINT, RSSL_DT_FLOAT, RSSL_DT_DOUBLE, RSSL_DT_REAL, RSSL_DT_DATE, RSSL_DT_TIME, RSSL_DT_DATETIME, RSSL_DT_QOS, RSSL_DT_STATE,
	RSSL_DT_ENUM, RSSL_DT_ARRAY, RSSL_DT_BUFFER, RSSL_DT_ASCII_STRING, RSSL_DT_UTF8_STRING, RSSL_DT_RMTES_STRING, RSSL_DT_OPAQUE, RSSL_DT_XML,
	RSSL_DT_FIELD_LIST, RSSL_DT_ELEMENT_LIST, RSSL_DT_FILTER_LIST, RSSL_DT_MAP, RSSL_DT_VECTOR, RSSL_DT_SERIES, RSSL_DT_MSG, RSSL_DT_JSON/*, RSSL_DT_ANSI_PAGE*/ // DO NOT REMOVE! RTSDK-5039: This code temporary commented out due to missed implementation of AnsiPage
};

static RsslUInt8 json1AllDataTypes[] =
{
	RSSL_DT_INT, RSSL_DT_UINT, RSSL_DT_FLOAT, RSSL_DT_DOUBLE, RSSL_DT_REAL, RSSL_DT_DATE, RSSL_DT_TIME, RSSL_DT_DATETIME, RSSL_DT_QOS, RSSL_DT_STATE,
	RSSL_DT_ENUM, RSSL_DT_ARRAY, RSSL_DT_BUFFER, RSSL_DT_ASCII_STRING, RSSL_DT_UTF8_STRING, RSSL_DT_RMTES_STRING, RSSL_DT_OPAQUE, RSSL_DT_XML,
	RSSL_DT_FIELD_LIST, RSSL_DT_ELEMENT_LIST, RSSL_DT_FILTER_LIST, RSSL_DT_MAP, RSSL_DT_VECTOR, RSSL_DT_SERIES, RSSL_DT_MSG/*, RSSL_DT_ANSI_PAGE*/ // DO NOT REMOVE! RTSDK-5039: This code temporary commented out due to missed implementation of AnsiPage
};

/* Base class for parameterized message tests.
 * Provides common operations for each test, such as converting from JSON to RWF and back.  */
class MsgConversionTestBase : public ::testing::Test
{

  public:
		MsgConversionTestBase() {};

	protected:

	/* Resuable members. */
	RsslEncodeIterator		_eIter;					/** Encode iterator used when encoding messages that will be converterd to JSON. */
	RsslDecodeIterator		_dIter;					/** Decode iterator used after converting from JSON to RWF. */
	RsslJsonConverter		_rsslJsonConverter;		/** RWF/JSON Converter */
	RsslBuffer				_rsslEncodeBuffer;		/** Buffer for encoding RWF. */
	RsslBuffer				_jsonInputBuffer;		/** Buffer for JSON input strings. */
	RsslBuffer				_jsonBuffer;			/** Buffer containing JSON messages. */
	RsslBuffer				_rsslDecodeBuffer;		/** Buffer for decoding from JSON to RWF. */
    Document  		        _jsonDocument;			/** Document used to parse and test JSON content. */

	/* Callbacks for Service ID vs. Service Name conversion */
	static RsslRet findServiceNameById(RsslBuffer *pServiceName, void *closure, RsslUInt16 serviceId);
	static RsslRet findServiceIdByName(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId);

	public:

		virtual void SetUp();
		
		void SetUpSmallBuffer(int bufferSize);

		virtual void TearDown();

		/** Initialize data for tests, such as dicitonaries and sample data. */
		static void initTestData();

		/** Cleanup resources created in initTestData. */
		static void cleanupTestData();

		/** Convert the RWF in _rsslEncodeBuffer to JSON, point the _jsonBuffer to the JSON content,
		 * and parse it into _jsonDocument. */
		void convertRsslToJson(RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2, bool json1Solicited = true, RsslRet ret = RSSL_RET_SUCCESS);

		/** Set a JSON buffer for converting to RWF. */
		void setJsonBufferToString(const char *jsonBuffer);

		/** Convert the JSON in _jsonBuffer to RWF, and point _rsslDecodeBuffer to the converted buffer. */
		void convertJsonToRssl(RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Convert the JSON in _jsonBuffer to RWF, expecting an error, and parse resulting error json into _jsonDocument. */
		void getJsonToRsslError();

		/** Convert the JSON in _jsonBuffer to RWF, expecting an error. */
		void getJsonToRsslError(RsslJsonProtocolType protocolType, RsslJsonConverterError *pConverterError);

		/** Convert the JSON in _jsonBuffer to RWF, expecting a Parser error. */
		void getJsonToRsslParserError(RsslJsonConverterError *pConverterError);

		/** Check a Base64-encoded buffer **/
		void checkJsonBase64String(const RsslBuffer *pExpectedRsslBuffer, const Value & jsonStringValue);

		/* Sample FieldList encoding/decoding */

		/** Encode a Sample FieldList in RWF. */
		static void encodeSampleRsslFieldList(RsslEncodeIterator *pIter, RsslLocalFieldSetDefDb * setDb = 0);

		/** Check a Sample JSON FieldList (corresponds to the FieldList encoded by encodeSampleRsslFieldList) */
		static void checkSampleJsonFieldList(const json::Value &fieldListObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF FieldList (corresponds to the FieldList encoded by encodeSampleRsslFieldList) */
		static void decodeSampleRsslFieldList(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter, RsslLocalFieldSetDefDb *setDb = NULL);


		/* Large FieldList encoding/decoding */

		/** Encode a Large FieldList in RWF. */
		static void encodeLargeRsslFieldList(RsslEncodeIterator *pIter, RsslLocalFieldSetDefDb * setDb = 0);

		/** Check a Large JSON FieldList (corresponds to the FieldList encoded by encodeLargeRsslFieldList) */
		static void checkLargeJsonFieldList(const json::Value &fieldListObject);

		/** Decode and check the Large RWF FieldList (corresponds to the FieldList encoded by encodeLargeRsslFieldList) */
		static void decodeLargeRsslFieldList(RsslDecodeIterator *pIter);

		/* Sample ElementList encoding/decoding */

		/** Encode a Sample ElementList in RWF. */
		static void encodeSampleRsslElementList(RsslEncodeIterator *pIter, RsslLocalElementSetDefDb * setDb = 0);

		/** Check a Sample JSON ElementList (corresponds to the ElementList encoded by encodeSampleRsslElementList) */
		static void checkSampleJsonElementList(const json::Value &elementListObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF ElementList (corresponds to the ElementList encoded by encodeSampleRsslElementList) */
		static void decodeSampleRsslElementList(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter, RsslLocalElementSetDefDb *setDb = NULL);

		/* Sample FilterList encoding/decoding */

		/** Encode a Sample FilterList in RWF. */
		static void encodeSampleRsslFilterList(RsslEncodeIterator *pIter);

		/** Check a Sample JSON FilterList (corresponds to the FilterList encoded by encodeSampleRsslFilterList) */
		static void checkSampleJsonFilterList(const json::Value &filterListObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF FilterList (corresponds to the FilterList encoded by encodeSampleRsslFilterList) */
		static void decodeSampleRsslFilterList(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter);

		/* Sample Map encoding/decoding */

		/** Encode a Sample Map in RWF. */
		static void encodeSampleRsslMap(RsslEncodeIterator *pIter);

		/** Check a Sample JSON Map (corresponds to the Map encoded by encodeSampleRsslMap) */
		static void checkSampleJsonMap(const json::Value &mapObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF Map (corresponds to the Map encoded by encodeSampleRsslMap) */
		static void decodeSampleRsslMap(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter);

		/* Sample Vector encoding/decoding */

		/** Encode a Sample Vector in RWF. */
		static void encodeSampleRsslVector(RsslEncodeIterator *pIter);

		/** Check a Sample JSON Vector (corresponds to the Vector encoded by encodeSampleRsslVector) */
		static void checkSampleJsonVector(const json::Value &vectorObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF Vector (corresponds to the Vector encoded by encodeSampleRsslVector) */
		static void decodeSampleRsslVector(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter);

		/* Sample Series encoding/decoding */

		/** Encode a Sample Series in RWF. */
		static void encodeSampleRsslSeries(RsslEncodeIterator *pIter);

		/** Check a Sample JSON Series (corresponds to the Series encoded by encodeSampleRsslSeries) */
		static void checkSampleJsonSeries(const json::Value &seriesObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF Series (corresponds to the Series encoded by encodeSampleRsslSeries) */
		static void decodeSampleRsslSeries(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter);

		/* Sample message encoding/decoding */

		/** Encode a Sample Message in RWF. */
		static void encodeSampleRsslUpdateMsg(RsslEncodeIterator *pIter);

		/** Check a Sample JSON Message (corresponds to the Message encoded by encodeSampleRsslUpdateMsg) */
		static void checkSampleJsonUpdateMsg(const json::Value &updateObject, RsslJsonProtocolType protocolType = RSSL_JSON_JPT_JSON2);

		/** Decode and check the Sample RWF Message (corresponds to the Message encoded by encodeSampleRsslUpdateMsg) */
		static void decodeSampleRsslUpdateMsg(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter);

		/* Json Buffer check */

		/** Check the Sample JSON buffer. */
		static void checkSampleJsonObject(const json::Value &jsonObject);
		
		/* Get the static RsslDataDictionary for use in tests */
		static RsslDataDictionary* getRsslDataDictionary();
};

/* For parameterized tests that take RsslDataTypes (it only contains an RsslDataType, but if a test fails this will print the data type's enumeration). */
class RsslDataTypeParam
{
	public:

		RsslDataTypes dataType;

		RsslDataTypeParam(RsslDataTypes dataType);

		/* Overload the << operator -- when tests fail, this will cause the parameter to printed in a readable fashion. */
		friend std::ostream &operator<<(std::ostream &out, const RsslDataTypeParam& param);
};


/* For printing protocol type names */
static const char *jsonProtocolTypeToString(RsslJsonProtocolType protocolType)
{
	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON: return "tr_json";
		case RSSL_JSON_JPT_JSON2: return "tr_json2";
		default: return "Unknown";
	}
}

#endif
