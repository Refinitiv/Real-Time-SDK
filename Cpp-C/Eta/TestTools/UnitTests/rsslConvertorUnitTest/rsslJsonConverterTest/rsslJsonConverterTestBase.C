/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#include "rsslJsonConverterTestBase.h"
#include <fstream>
#include "cpp-base64/base64.h"
#include "q_ansi.h"

using namespace std;
using namespace json;

/** Size of the RSSL encoding buffer. */
static const RsslUInt32 RSSL_BUFFER_SIZE = 65535;

/** RSSL Dictionary */
static RsslDataDictionary	_rsslDictionary;
static RsslDataDictionary	*_pRsslDictionary;

/* Commandline options that may aid in debugging. */
bool cmdlPrintJsonBuffer;	/* Print JSON buffers after conversion from RWF. */
bool cmdlPrintRsslBuffer;	/* Print RWF buffers after conversion from JSON. */


#define ANSI_INIT_STRG             "\033c\033(B\033[0;47;30m\033[1;1H"
#define ANSI_CHG_TEST              "\033[4;10Hwrite on line three\
									\033[8;30Hwrite on line eight"

/* Sample Data commonly used by tests. */

/* Used for the RMTES and UTF8 buffers. The RMTES and UTF8 values below are the same string in their respective formats.
 * (both represent an upward arrow) */
const char RMTES_UPTICK[] = { (const char)0xde, (const char)0x00 };
const char UTF8_UPTICK[] = { (const char)0xe2, (const char)0x87, (const char)0xa7, (const char)0x00 };

static char ENCODED_ANSI_DATA[200] = { 0 };

/* Sample content */
const RsslBuffer		MSG_KEY_NAME			= {4, (char*)"TINY"};
const RsslBuffer		MSG_KEY_NAME_ESC_CHAR   = {7, (char*)"\\GOOG.O"};
const RsslBuffer		REQ_MSG_KEY_NAME		= {4, (char*)"ROLL"};
const RsslBuffer		STATE_TEXT				= {11, (char*)"All is well"};
const RsslBuffer		EXTENDED_HEADER			= {14, (char*)"ExtendedHeader"};
const RsslBuffer		PERM_DATA				= {10, (char*)"PerMission"};
const RsslBuffer		ACK_TEXT				= {12, (char*)"Acknowledged"};
const RsslUInt16		MSGKEY_SVC_ID			= 555;
const RsslInt32		    MSGKEY_IDENTIFIER		= 1205;
const RsslUInt16		REQ_MSGKEY_SVC_ID		= 555;
const char				*IP_ADDRESS_STR			= "127.0.0.1";
const RsslUInt32		IP_ADDRESS_UINT			= 0x7f000001; /* 127.0.0.1, same IP address as IP_ADDRESS_STR */;
const RsslUInt32		USER_ID					= 8;
const RsslUInt32		SEQ_NUM					= 88;
const RsslUInt32		SECONDARY_SEQ_NUM		= 16;
const RsslUInt16		PART_NUM				= 5;
const RsslBuffer		GROUP_ID				= {11, (char*)"SomeGroupId"};
const RsslBuffer	 	JSON_BUFFER				= {16, (char*)"{\"SomeKey\":true}"};
const RsslBuffer	 	OPAQUE_BUFFER			= {10, (char*)"OpaqueData"};
const RsslBuffer	 	ASCII_STRING			= {5, (char*)"Ascii"};
const RsslBuffer	 	UTF8_STRING				= {sizeof(UTF8_UPTICK)/sizeof(char) - 1, (char*)UTF8_UPTICK};
const RsslBuffer		RMTES_STRING			= {sizeof(RMTES_UPTICK)/sizeof(char) - 1, (char*)RMTES_UPTICK};
const RsslBuffer	 	RMTES_STRING_AS_UTF8	= {sizeof(UTF8_UPTICK)/sizeof(char) - 1, (char*)UTF8_UPTICK};
const RsslBuffer		XML_BUFFER				= {38, (char*)"<html><head>Hello World!</head></html>"};
const RsslBuffer		ANSI_PAGE_BUFFER		= {200, (char*)ENCODED_ANSI_DATA };
const RsslBuffer		SERVICE_NAME			= {9, (char*)"DUCK_FEED"};

/* More complex types, initialized in MsgConversionTestBase::initSampleData. Only the constant references are exposed for use in tests. */
static	RsslReal 		sampleReal;
static	RsslQos			sampleQos;
static	RsslDateTime	sampleDateTime;
static 	RsslState		sampleState;

const	RsslReal		&REAL			= sampleReal;
const	RsslQos			&QOS			= sampleQos;
const	RsslDateTime	&DATETIME		= sampleDateTime;
const	RsslState		&STATE			= sampleState;

/* Fields that the test adds to the RDMFieldDictionary. The name of each field indicates its type (e.g. INT_FIELD is a field of type RSSL_DT_INT). */
const CustomField INT_FIELD				= { -1, "-1", {3, (char*)"INT"}};
const CustomField UINT_FIELD			= { -2, "-2", {4, (char*)"UINT"}};
const CustomField REAL_FIELD			= { -3, "-3", {4, (char*)"REAL"}};
const CustomField FLOAT_FIELD			= { -4, "-4", {5, (char*)"FLOAT"}};
const CustomField DOUBLE_FIELD			= { -5, "-5", {6, (char*)"DOUBLE"}};
const CustomField DATE_FIELD			= { -6, "-6", {4, (char*)"DATE"}};
const CustomField TIME_FIELD			= { -7, "-7", {4, (char*)"TIME"}};
const CustomField DATETIME_FIELD		= { -8, "-8", {8, (char*)"DATETIME"}};
const CustomField QOS_FIELD				= { -9, "-9", {9, (char*)"QOS_FIELD"}};
const CustomField STATE_FIELD			= { -10, "-10", {11, (char*)"STATE_FIELD"}};
const CustomField ENUM_FIELD			= { -11, "-11", {10, (char*)"ENUM_FIELD"}};
const CustomField BUFFER_FIELD			= { -12, "-12", {6, (char*)"BUFFER"}};
const CustomField ASCII_STRING_FIELD	= { -13, "-13", {11, (char*)"ASCIISTRING"}};
const CustomField UTF8_STRING_FIELD		= { -14, "-14", {10, (char*)"UTF8STRING"}};
const CustomField RMTES_STRING_FIELD	= { -15, "-15", {11, (char*)"RMTESSTRING"}};
const CustomField XML_FIELD				= { -16, "-16", {3, (char*)"XML"}};
const CustomField FIELDLIST_FIELD		= { -17, "-17", {9, (char*)"FIELDLIST"}};
const CustomField ELEMENTLIST_FIELD		= { -18, "-18", {11, (char*)"ELEMENTLIST"}};
const CustomField FILTERLIST_FIELD		= { -19, "-19", {10, (char*)"FILTERLIST"}};
const CustomField MAP_FIELD				= { -20, "-20", {3, (char*)"MAP"}};
const CustomField VECTOR_FIELD			= { -21, "-21", {12, (char*)"VECTOR_FIELD"}};
const CustomField SERIES_FIELD			= { -22, "-22", {12, (char*)"SERIES_FIELD"}};
const CustomField MSG_FIELD				= { -23, "-23", {3, (char*)"MSG"}};
const CustomField JSON_FIELD			= { -24, "-24", {4, (char*)"JSON"}};
const CustomField OPAQUE_FIELD			= { -25, "-25", {6, (char*)"OPAQUE"}};
const CustomField ARRAY_FIELD			= { -26, "-26", {5, (char*)"ARRAY"}};
const CustomField ANSI_PAGE_FIELD		= { -27, "-27", {200, (char*)ENCODED_ANSI_DATA}};

int decodeAnsi(int x1, int x2, int y1, int y2, char* sText, LISTTYP* u_ptr, PAGETYP* pg_ptr)
{
	long text_len, ch_p;
	int retv, ucnt;
	struct upd_type *l_ptr;
	retv = 0; ch_p = 1;
	text_len = (long)strlen(sText);
	for (; text_len > 0 && ch_p > 0; text_len -= ch_p, sText += ch_p) {
		ch_p = qa_decode(pg_ptr, sText, text_len, u_ptr);
		/*check all updates for range*/
		for (ucnt = (u_ptr->index + 1), l_ptr =
			u_ptr->upd_list; ucnt; ucnt--, l_ptr++) {
			if (!retv && l_ptr->row >= y1 && l_ptr->row <= y2) {
				if ((l_ptr->upd_beg >= x1 &&
					l_ptr->upd_beg <= x2) ||
					(l_ptr->upd_end >= x1 &&
						l_ptr->upd_end <= x2))
					retv = 1;
			}
		}
	}
	return (retv);
}

void encodeAnsi(LISTTYP *u_ptr, PAGETYP* pg_ptr, char* out_buffer)
{
	int retv;
	long len;
	short last_mod;
	last_mod = pg_ptr->last_mod = 0;
	do {
		last_mod = pg_ptr->last_mod;
		retv = qa_encode(pg_ptr, (unsigned char*)out_buffer, 1000, &len, 0, u_ptr);
		out_buffer[len] = '\0';
		/*while not done and progress is being made*/
	} while (last_mod != pg_ptr->last_mod && retv != DONE);
}

void encodeAnsiData()
{
	const int ROWS = 25; /*Standard page size 25 X 80*/
	const int COLS = 80;
	PAGETYP qa_page; /*page structure*/
	CHARTYP pageImage[ROWS*COLS] = { 0 };
	LISTTYP *qa_ulist;/*pointer to update struc*/
	char qa_list_sp[sizeof(LISTTYP) +
		sizeof(struct upd_type) * 200] = { 0 };
	/*space for update*/
	int retv, cnt;

	/* set up the page structure */
	qa_page.page = pageImage;
	qa_set_rows(ROWS);
	qa_set_columns(COLS);
	/* set up update list area*/
	qa_ulist = (LISTTYP *)qa_list_sp;
	qa_ulist->max_updt = 200;

	/* clear data area */
	cnt = qa_decode(&qa_page, ANSI_INIT_STRG, sizeof(ANSI_INIT_STRG), qa_ulist);
	if (!cnt)
	{
		printf("Failed to clear data area for Ansi data \n");
		return;
	}

	retv = decodeAnsi(1, 80, 7, 10, ANSI_CHG_TEST, qa_ulist, &qa_page);
	if (!retv)
	{
		printf("Failed to decode Ansi data \n");
		return;
	}

	encodeAnsi(qa_ulist, &qa_page, ENCODED_ANSI_DATA);
}

/* MsgConversionTestBase function definitions. */

RsslRet MsgConversionTestBase::findServiceNameById(RsslBuffer *pServiceName, void *closure, RsslUInt16 serviceId)
{
	if (serviceId == MSGKEY_SVC_ID)
	{
		*pServiceName = SERVICE_NAME;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

RsslRet MsgConversionTestBase::findServiceIdByName(RsslBuffer *pServiceName, void *closure, RsslUInt16 *pServiceId)
{
	if (rsslBufferIsEqual(pServiceName, &SERVICE_NAME))
	{
		*pServiceId = MSGKEY_SVC_ID;
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

void MsgConversionTestBase::initTestData()
{
	RsslRet ret;
	char errorTextBuf[255];
	RsslBuffer errorText = {255, errorTextBuf};

	ifstream rdmFieldDictionaryFile;
	const char *testDictionaryFileName = "RDMFieldDictionary.converterTest";
	ofstream testDictionaryFile;

	/* Load RSSL/JSON converter functions. */
#ifdef _RSSLJC_SHARED_LIBRARY
	rsslClearJsonConverterFunctions();
	if (rsslLoadRsslJsonConverterFunctions(NULL, &converterError) != RSSL_RET_SUCCESS)
	{
		cout << "Error loading RsslJsonConverter library functions: " << converterError.text << endl;
		goto initTestDataFailed;
	}

	rsslJsonConverterFunctions.rsslJsonInitialize();
#else
	rsslJsonInitialize();
#endif

	/* Initialize data of complex primitive types. */

	/* REAL */
	rsslClearReal(&sampleReal);
	sampleReal.hint = RSSL_RH_EXPONENT_2;
	sampleReal.value = 3905;

	/* QOS */
	rsslClearQos(&sampleQos);
	sampleQos.timeliness = RSSL_QOS_TIME_REALTIME;
	sampleQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;

	/* DATETIME */
	rsslClearDateTime(&sampleDateTime);
	sampleDateTime.date.day = 12;
	sampleDateTime.date.month = 11;
	sampleDateTime.date.year = 1955;
	sampleDateTime.time.hour = 22;
	sampleDateTime.time.minute = 4;

	/* STATE */
	rsslClearState(&sampleState);
	sampleState.streamState = RSSL_STREAM_OPEN;
	sampleState.dataState = RSSL_DATA_OK;

	/* We want to use an official RDMFieldDictionary with this test, but need to add some fields. 
	 * Copy the RDMFieldDictionary into another file, then add the needed fields. */

	rdmFieldDictionaryFile.open("RDMFieldDictionary");
	if (!rdmFieldDictionaryFile.is_open())
	{
		cout << "Failed to open RDMFieldDictionary file (used to create custom field dictionary)." << endl;
		goto initTestDataFailed;
	}

	testDictionaryFile.open("RDMFieldDictionary.converterTest");
	if (!testDictionaryFile.is_open())
	{
		cout << "Failed to create custom field dictionary file" << testDictionaryFileName << endl;
		goto initTestDataFailed;
	}

	/* Copy RDMFieldDictionary */
	testDictionaryFile << rdmFieldDictionaryFile.rdbuf();

	/* Add the custom fields. */
	testDictionaryFile << INT_FIELD.fieldName.data << " \"" << INT_FIELD.fieldName.data << "\" " << INT_FIELD.fieldId << " NULL NONE 0 INT 0" << endl;
	testDictionaryFile << UINT_FIELD.fieldName.data << " \"" << UINT_FIELD.fieldName.data << "\" " << UINT_FIELD.fieldId << " NULL NONE 0 UINT 0" << endl;
	testDictionaryFile << REAL_FIELD.fieldName.data << " \"" << REAL_FIELD.fieldName.data << "\" " << REAL_FIELD.fieldId << " NULL NONE 0 REAL 0" << endl;
	testDictionaryFile << FLOAT_FIELD.fieldName.data << " \"" << FLOAT_FIELD.fieldName.data << "\" " << FLOAT_FIELD.fieldId << " NULL NONE 0 FLOAT 0" << endl;
	testDictionaryFile << DOUBLE_FIELD.fieldName.data << " \"" << DOUBLE_FIELD.fieldName.data << "\" " << DOUBLE_FIELD.fieldId << " NULL NONE 0 DOUBLE 0" << endl;
	testDictionaryFile << DATE_FIELD.fieldName.data << " \"" << DATE_FIELD.fieldName.data << "\" " << DATE_FIELD.fieldId << " NULL NONE 0 DATE 0" << endl;
	testDictionaryFile << TIME_FIELD.fieldName.data << " \"" << TIME_FIELD.fieldName.data << "\" " << TIME_FIELD.fieldId << " NULL NONE 0 TIME 0" << endl;
	testDictionaryFile << DATETIME_FIELD.fieldName.data << " \"" << DATETIME_FIELD.fieldName.data << "\" " << DATETIME_FIELD.fieldId << " NULL NONE 0 DATETIME 0" << endl;
	testDictionaryFile << QOS_FIELD.fieldName.data << " \"" << QOS_FIELD.fieldName.data << "\" " << QOS_FIELD.fieldId << " NULL NONE 0 QOS 0" << endl;
	testDictionaryFile << STATE_FIELD.fieldName.data << " \"" << STATE_FIELD.fieldName.data << "\" " << STATE_FIELD.fieldId << " NULL NONE 0 STATE 0" << endl;
	testDictionaryFile << ENUM_FIELD.fieldName.data << " \"" << ENUM_FIELD.fieldName.data << "\" " << ENUM_FIELD.fieldId << " NULL NONE 0 ENUM 0" << endl;
	testDictionaryFile << BUFFER_FIELD.fieldName.data << " \"" << BUFFER_FIELD.fieldName.data << "\" " << BUFFER_FIELD.fieldId << " NULL NONE 0 BUFFER 0" << endl;
	testDictionaryFile << ASCII_STRING_FIELD.fieldName.data << " \"" << ASCII_STRING_FIELD.fieldName.data << "\" " << ASCII_STRING_FIELD.fieldId << " NULL NONE 0 ASCII_STRING 0" << endl;
	testDictionaryFile << UTF8_STRING_FIELD.fieldName.data << " \"" << UTF8_STRING_FIELD.fieldName.data << "\" " << UTF8_STRING_FIELD.fieldId << " NULL NONE 0 UTF8_STRING 0" << endl;
	testDictionaryFile << RMTES_STRING_FIELD.fieldName.data << " \"" << RMTES_STRING_FIELD.fieldName.data << "\" " << RMTES_STRING_FIELD.fieldId << " NULL NONE 0 RMTES_STRING 0" << endl;
	testDictionaryFile << XML_FIELD.fieldName.data << " \"" << XML_FIELD.fieldName.data << "\" " << XML_FIELD.fieldId << " NULL NONE 0 XML 0" << endl;
	testDictionaryFile << FIELDLIST_FIELD.fieldName.data << " \"" << FIELDLIST_FIELD.fieldName.data << "\" " << FIELDLIST_FIELD.fieldId << " NULL NONE 0 FIELD_LIST 0" << endl;
	testDictionaryFile << ELEMENTLIST_FIELD.fieldName.data << " \"" << ELEMENTLIST_FIELD.fieldName.data << "\" " << ELEMENTLIST_FIELD.fieldId << " NULL NONE 0 ELEMENT_LIST 0" << endl;
	testDictionaryFile << FILTERLIST_FIELD.fieldName.data << " \"" << FILTERLIST_FIELD.fieldName.data << "\" " << FILTERLIST_FIELD.fieldId << " NULL NONE 0 FILTER_LIST 0" << endl;
	testDictionaryFile << MAP_FIELD.fieldName.data << " \"" << MAP_FIELD.fieldName.data << "\" " << MAP_FIELD.fieldId << " NULL NONE 0 MAP 0" << endl;
	testDictionaryFile << VECTOR_FIELD.fieldName.data << " \"" << VECTOR_FIELD.fieldName.data << "\" " << VECTOR_FIELD.fieldId << " NULL NONE 0 VECTOR 0" << endl;
	testDictionaryFile << SERIES_FIELD.fieldName.data << " \"" << SERIES_FIELD.fieldName.data << "\" " << SERIES_FIELD.fieldId << " NULL NONE 0 SERIES 0" << endl;
	testDictionaryFile << MSG_FIELD.fieldName.data << " \"" << MSG_FIELD.fieldName.data << "\" " << MSG_FIELD.fieldId << " NULL NONE 0 MSG 0" << endl;
	testDictionaryFile << JSON_FIELD.fieldName.data << " \"" << JSON_FIELD.fieldName.data << "\" " << JSON_FIELD.fieldId << " NULL NONE 0 JSON 0" << endl;
	testDictionaryFile << OPAQUE_FIELD.fieldName.data << " \"" << OPAQUE_FIELD.fieldName.data << "\" " << OPAQUE_FIELD.fieldId << " NULL NONE 0 OPAQUE 0" << endl;
	testDictionaryFile << ARRAY_FIELD.fieldName.data << " \"" << ARRAY_FIELD.fieldName.data << "\" " << ARRAY_FIELD.fieldId << " NULL NONE 0 ARRAY 0" << endl;

	testDictionaryFile.close();
	rsslClearDataDictionary(&_rsslDictionary);
	if ((ret = rsslLoadFieldDictionary(testDictionaryFileName, &_rsslDictionary, &errorText)) != RSSL_RET_SUCCESS)
	{
		cout << "Failed to load RSSL dictionary file: " << testDictionaryFileName << "(" << errorText.data << ")" << endl;
		goto initTestDataFailed;
	}

	_pRsslDictionary = &_rsslDictionary;
	
	encodeAnsiData();

	return;

	initTestDataFailed:
	cout << "Failed to initialize resources needed for test. NO TESTS WERE RUN." << endl;
	cleanupTestData();
	exit(1);
}

void MsgConversionTestBase::cleanupTestData()
{
	rsslDeleteDataDictionary(&_rsslDictionary);
#ifdef _RSSLJC_SHARED_LIBRARY
	if (rsslJsonConverterFunctions.module)
	{
		rsslJsonConverterFunctions.rsslJsonUninitialize();
		rsslUnloadRsslJsonConverterFunctions();
		rsslJsonUninitialize();
	}
#else
	rsslJsonUninitialize();
#endif
}

/* MsgConversionTestBase definitions */

void MsgConversionTestBase::SetUp()
{
	RsslCreateJsonConverterOptions converterOptions;
	RsslJsonConverterError converterError;
	RsslJsonDictionaryListProperty dictionaryListProperty;
	RsslJsonServiceIdToNameCallbackProperty svcIdToNameCbProperty;
	RsslJsonServiceNameToIdCallbackProperty svcNameToIdCbProperty;

	rsslClearBuffer(&_rsslEncodeBuffer);
	rsslClearBuffer(&_jsonInputBuffer);
	rsslClearBuffer(&_rsslDecodeBuffer);
	rsslClearBuffer(&_jsonBuffer);

	_rsslEncodeBuffer.length = RSSL_BUFFER_SIZE;
	_rsslEncodeBuffer.data = new char[RSSL_BUFFER_SIZE];

	_jsonInputBuffer.length = RSSL_BUFFER_SIZE;
	_jsonInputBuffer.data = new char[RSSL_BUFFER_SIZE];

	/* Create converter. */
	rsslClearCreateRsslJsonConverterOptions(&converterOptions);
#ifdef _RSSLJC_SHARED_LIBRARY
	_rsslJsonConverter = rsslJsonConverterFunctions.rsslCreateRsslJsonConverter(&converterOptions, &converterError);
#else
	_rsslJsonConverter = rsslCreateRsslJsonConverter(&converterOptions, &converterError);
#endif
	ASSERT_TRUE(_rsslJsonConverter != NULL) << "rsslCreateRsslJsonConverter failed: " << converterError.text;

	/* Set service-name/ID callbacks. */
	svcIdToNameCbProperty.callback = MsgConversionTestBase::findServiceNameById;
	svcIdToNameCbProperty.closure = this;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK, &svcIdToNameCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK, &svcIdToNameCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif
	svcNameToIdCbProperty.callback = MsgConversionTestBase::findServiceIdByName;
	svcNameToIdCbProperty.closure = this;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK, &svcNameToIdCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK, &svcNameToIdCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	/* Set dictionary list. */
	rsslClearConverterDictionaryListProperty(&dictionaryListProperty);
	dictionaryListProperty.pDictionaryList = &_pRsslDictionary;
	dictionaryListProperty.dictionaryListLength = 1;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DICTIONARY_LIST, &dictionaryListProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DICTIONARY_LIST, &dictionaryListProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	/* Set default service ID. */
	RsslUInt16 serviceId = 1;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DEFAULT_SERVICE_ID, &serviceId, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DEFAULT_SERVICE_ID, &serviceId, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	RsslBool catchUnknownJsonKeys = RSSL_TRUE;

	RsslBool catchUnknownJsonFids = RSSL_TRUE;

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, &catchUnknownJsonKeys, &converterError))
			<< "rsslJsonCatchUnknownJsonKeys failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, &catchUnknownJsonKeys, &converterError))
			<< "rsslJsonCatchUnknownJsonKeys failed: " << converterError.text;
#endif

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, &catchUnknownJsonFids, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, &catchUnknownJsonFids, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

}

void MsgConversionTestBase::SetUpSmallBuffer(int bufferSize)
{
	RsslCreateJsonConverterOptions converterOptions;
	RsslJsonConverterError converterError;
	RsslJsonDictionaryListProperty dictionaryListProperty;
	RsslJsonServiceIdToNameCallbackProperty svcIdToNameCbProperty;
	RsslJsonServiceNameToIdCallbackProperty svcNameToIdCbProperty;

	rsslClearBuffer(&_rsslEncodeBuffer);
	rsslClearBuffer(&_rsslDecodeBuffer);
	rsslClearBuffer(&_jsonBuffer);

	_rsslEncodeBuffer.length = bufferSize;
	_rsslEncodeBuffer.data = new char[bufferSize];

	/* Create converter. */
	rsslClearCreateRsslJsonConverterOptions(&converterOptions);
	converterOptions.bufferSize = 100;
#ifdef _RSSLJC_SHARED_LIBRARY
	_rsslJsonConverter = rsslJsonConverterFunctions.rsslCreateRsslJsonConverter(&converterOptions, &converterError);
#else
	_rsslJsonConverter = rsslCreateRsslJsonConverter(&converterOptions, &converterError);
#endif
	ASSERT_TRUE(_rsslJsonConverter != NULL) << "rsslCreateRsslJsonConverter failed: " << converterError.text;

	/* Set service-name/ID callbacks. */
	svcIdToNameCbProperty.callback = MsgConversionTestBase::findServiceNameById;
	svcIdToNameCbProperty.closure = this;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK, &svcIdToNameCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_ID_TO_NAME_CALLBACK, &svcIdToNameCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	svcNameToIdCbProperty.callback = MsgConversionTestBase::findServiceIdByName;
	svcNameToIdCbProperty.closure = this;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK, &svcNameToIdCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_SERVICE_NAME_TO_ID_CALLBACK, &svcNameToIdCbProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	/* Set dictionary list. */
	rsslClearConverterDictionaryListProperty(&dictionaryListProperty);
	dictionaryListProperty.pDictionaryList = &_pRsslDictionary;
	dictionaryListProperty.dictionaryListLength = 1;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DICTIONARY_LIST, &dictionaryListProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DICTIONARY_LIST, &dictionaryListProperty, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	/* Set default service ID. */
	RsslUInt16 serviceId = 1;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DEFAULT_SERVICE_ID, &serviceId, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_DEFAULT_SERVICE_ID, &serviceId, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

	RsslBool catchUnknownJsonKeys = RSSL_TRUE;

	RsslBool catchUnknownJsonFids = RSSL_TRUE;

#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, &catchUnknownJsonKeys, &converterError))
			<< "rsslJsonCatchUnknownJsonKeys failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, &catchUnknownJsonKeys, &converterError))
			<< "rsslJsonCatchUnknownJsonKeys failed: " << converterError.text;
#endif


#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, &catchUnknownJsonFids, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterSetProperty(_rsslJsonConverter,
				RSSL_JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, &catchUnknownJsonFids, &converterError))
			<< "rsslJsonConverterSetProperty failed: " << converterError.text;
#endif

}

void MsgConversionTestBase::TearDown()
{
	RsslJsonConverterError converterError;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDestroyRsslJsonConverter(_rsslJsonConverter, &converterError))
		<< "rsslDestroyRsslJsonConverter failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDestroyRsslJsonConverter(_rsslJsonConverter, &converterError))
		<< "rsslDestroyRsslJsonConverter failed: " << converterError.text;
#endif
	delete[] _rsslEncodeBuffer.data;
	delete[] _jsonInputBuffer.data;
}

void MsgConversionTestBase::convertRsslToJson(RsslJsonProtocolType protocolType, bool json1Solicited, RsslRet ret)
{
	RsslConvertRsslMsgToJsonOptions rsslToJsonOptions;
	RsslGetJsonMsgOptions getJsonMsgOptions;
	RsslJsonConverterError converterError;
	RsslMsg rsslMsg;

	rsslClearDecodeIterator(&_dIter);
	rsslSetDecodeIteratorBuffer(&_dIter, &_rsslEncodeBuffer);
	rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

	if (cmdlPrintRsslBuffer)
	{
		RsslDecodeIterator xmlIter;

		rsslClearDecodeIterator(&xmlIter);
		rsslSetDecodeIteratorRWFVersion(&xmlIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		rsslSetDecodeIteratorBuffer(&xmlIter, &rsslMsg.msgBase.encDataBody);

		printf("** RWF message before JSON conversion:\n");
		EXPECT_EQ(RSSL_RET_SUCCESS, decodeMsgToXML(stdout, &rsslMsg, &_rsslDictionary, &xmlIter));
	}

	rsslClearConvertRsslMsgToJsonOptions(&rsslToJsonOptions);
	rsslToJsonOptions.jsonProtocolType = protocolType;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(ret, rsslJsonConverterFunctions.rsslConvertRsslMsgToJson(_rsslJsonConverter, &rsslToJsonOptions, &rsslMsg, &converterError))
		<< "rsslConvertRsslMsgToJson failed: " << converterError.text;
#else
	ASSERT_EQ(ret, rsslConvertRsslMsgToJson(_rsslJsonConverter, &rsslToJsonOptions, &rsslMsg, &converterError))
		<< "rsslConvertRsslMsgToJson failed: " << converterError.text;
#endif

	if (ret != RSSL_RET_SUCCESS)
		return;

	rsslClearGetJsonMsgOptions(&getJsonMsgOptions);
	getJsonMsgOptions.streamId = rsslMsg.msgBase.streamId;
	getJsonMsgOptions.jsonProtocolType = protocolType;
	getJsonMsgOptions.isCloseMsg = (rsslMsg.msgBase.msgClass == RSSL_MC_CLOSE) ? RSSL_TRUE : RSSL_FALSE;
	getJsonMsgOptions.solicited = json1Solicited;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslGetConverterJsonMsg(_rsslJsonConverter, &getJsonMsgOptions, &_jsonBuffer, &converterError))
		<< "rsslGetConverterJsonMsg failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslGetConverterJsonMsg(_rsslJsonConverter, &getJsonMsgOptions, &_jsonBuffer, &converterError))
		<< "rsslGetConverterJsonMsg failed: " << converterError.text;
#endif
	
	if (cmdlPrintJsonBuffer)
		printf("** Converted JSON buffer: %.*s\n", _jsonBuffer.length, _jsonBuffer.data);

	_jsonDocument.Parse(_jsonBuffer.data);
    ASSERT_TRUE(NULL == _jsonDocument.GetParseError()) << "** JSON Parse Error: " << _jsonDocument.GetParseError() << "\n";
	ASSERT_TRUE(_jsonDocument.IsObject());
}

void MsgConversionTestBase::checkJsonBase64String(const RsslBuffer *pExpectedBuffer, const Value & jsonStringValue)
{
	ASSERT_TRUE(jsonStringValue.IsString());
	std::string encodedString(jsonStringValue.GetString(), jsonStringValue.GetStringLength());
	std::string decodedString = base64_decode(encodedString);
	RsslBuffer decodedBuffer = { (RsslUInt32)decodedString.length(), (char*)decodedString.c_str() };
	EXPECT_TRUE(rsslBufferIsEqual(pExpectedBuffer, &decodedBuffer));
}

void MsgConversionTestBase::setJsonBufferToString(const char *jsonString)
{
	_jsonBuffer.length = (RsslUInt32)strlen(jsonString);
	_jsonBuffer.data = _jsonInputBuffer.data;
	ASSERT_LE(_jsonBuffer.length, RSSL_BUFFER_SIZE); /* Make sure we have space. */
	memcpy(_jsonBuffer.data, jsonString, _jsonBuffer.length);
}

void MsgConversionTestBase::convertJsonToRssl(RsslJsonProtocolType protocolType)
{
	RsslDecodeJsonMsgOptions decodeJsonMsgOptions;
	RsslJsonMsg jsonMsg;
	RsslJsonConverterError converterError;
	RsslDataDictionary *pDictionary = &_rsslDictionary;
	RsslParseJsonBufferOptions parseOptions;

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = protocolType;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) 
		<< "rsslParseJsonBuffer failed: " << converterError.text;
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS) 
		<< "rsslParseJsonBuffer failed: " << converterError.text;
#endif

	rsslClearDecodeJsonMsgOptions(&decodeJsonMsgOptions);
	decodeJsonMsgOptions.jsonProtocolType = protocolType;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError)) << "rsslDecodeJsonMsg failed: " << converterError.text;
#else
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeJsonMsg(_rsslJsonConverter, &decodeJsonMsgOptions, &jsonMsg, &_rsslDecodeBuffer,
				&converterError)) << "rsslDecodeJsonMsg failed: " << converterError.text;
#endif

	switch(jsonMsg.msgBase.msgClass)
	{
		case RSSL_JSON_MC_RSSL_MSG:
			/* Message converted to an RsslMsg; continue. */
			break;
		case RSSL_JSON_MC_PING:
			/* Not currently supported in test. */
			FAIL() << "Message was a JSON Ping." << endl;
			break;
		case RSSL_JSON_MC_PONG:
			/* Not currently supported in test. */
			FAIL() << "Message was a JSON Pong." << endl;
			break;
		default:
			FAIL() << "Unknown JSON Message class:" << jsonMsg.msgBase.msgClass << ".\n";
			break;
	}

	if (cmdlPrintRsslBuffer)
	{
		RsslMsg rsslMsg;
		RsslDecodeIterator xmlIter;

		rsslClearMsg(&rsslMsg);

		rsslClearDecodeIterator(&_dIter);
		rsslSetDecodeIteratorBuffer(&_dIter, &_rsslDecodeBuffer);
		rsslSetDecodeIteratorRWFVersion(&_dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(&_dIter, &rsslMsg));

		rsslClearDecodeIterator(&xmlIter);
		rsslSetDecodeIteratorRWFVersion(&xmlIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		rsslSetDecodeIteratorBuffer(&xmlIter, &rsslMsg.msgBase.encDataBody);

		printf("** Converted RWF message:\n");
		EXPECT_EQ(RSSL_RET_SUCCESS, decodeMsgToXML(stdout, &rsslMsg, &_rsslDictionary, &xmlIter));
	}
}

void MsgConversionTestBase::getJsonToRsslError()
{

	RsslRet ret;
	RsslBuffer outBuffer;
	RsslGetJsonErrorParams errorParams;
	RsslJsonConverterError converterError;
	RsslParseJsonBufferOptions parseOptions;
	RsslDecodeJsonMsgOptions decodeOptions;
	RsslJsonMsg jsonMsg;

	if (cmdlPrintJsonBuffer)
		printf("** JSON buffer: %.*s\n", _jsonBuffer.length, _jsonBuffer.data);

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer failed: " << converterError.text;
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, &converterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer failed: " << converterError.text;
#endif

	RsslMsg &rsslMsg = jsonMsg.jsonRsslMsg.rsslMsg;

	rsslClearDecodeJsonMsgOptions(&decodeOptions);
	decodeOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeOptions, &jsonMsg, &outBuffer, &converterError), -1);
#else
	ASSERT_EQ(rsslDecodeJsonMsg(_rsslJsonConverter, &decodeOptions, &jsonMsg, &outBuffer, &converterError), -1);
#endif

	rsslClearRsslGetJsonErrorParams(&errorParams);
#ifdef _RSSLJC_SHARED_LIBRARY
	rsslJsonConverterFunctions.rsslGetJsonSimpleErrorParams(_rsslJsonConverter, &decodeOptions, &converterError, &errorParams, &_jsonBuffer, rsslMsg.msgBase.streamId);

	ret = rsslJsonConverterFunctions.rsslJsonGetErrorMessage(_rsslJsonConverter, &errorParams, &outBuffer);
#else
	rsslGetJsonSimpleErrorParams(_rsslJsonConverter, &decodeOptions, &converterError, &errorParams, &_jsonBuffer, rsslMsg.msgBase.streamId);

	ret = rsslJsonGetErrorMessage(_rsslJsonConverter, &errorParams, &outBuffer);
#endif

	if (ret == RSSL_RET_SUCCESS)
	{
		if (cmdlPrintJsonBuffer)
			printf("** Error message buffer: %.*s\n", outBuffer.length, outBuffer.data);

        _jsonDocument.Parse(outBuffer.data);
        ASSERT_TRUE(NULL == _jsonDocument.GetParseError()) << "** JSON Parse Error: " << _jsonDocument.GetParseError()<<"\n";
		ASSERT_TRUE(_jsonDocument.IsObject());
	}
}

void MsgConversionTestBase::getJsonToRsslError(RsslJsonProtocolType protocolType, RsslJsonConverterError *pConverterError)
{
	RsslBuffer outBuffer;
	RsslParseJsonBufferOptions parseOptions;
	RsslDecodeJsonMsgOptions decodeOptions;
	RsslJsonMsg jsonMsg;

	if (cmdlPrintJsonBuffer)
		printf("** JSON buffer: %.*s\n", _jsonBuffer.length, _jsonBuffer.data);

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = protocolType;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_GE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, pConverterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer failed: " << pConverterError->text;
#else
	ASSERT_GE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, pConverterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer failed: " << pConverterError->text;
#endif

	RsslMsg &rsslMsg = jsonMsg.jsonRsslMsg.rsslMsg;

	rsslClearDecodeJsonMsgOptions(&decodeOptions);
	decodeOptions.jsonProtocolType = protocolType;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_EQ(rsslJsonConverterFunctions.rsslDecodeJsonMsg(_rsslJsonConverter, &decodeOptions, &jsonMsg, &outBuffer, pConverterError), -1);
#else
	ASSERT_EQ(rsslDecodeJsonMsg(_rsslJsonConverter, &decodeOptions, &jsonMsg, &outBuffer, pConverterError), -1);
#endif

	if (cmdlPrintJsonBuffer)
		printf("** Converter Error Text : %s\n", pConverterError->text);
}

void MsgConversionTestBase::getJsonToRsslParserError(RsslJsonConverterError *pConverterError)
{
	RsslParseJsonBufferOptions parseOptions;

	if (cmdlPrintJsonBuffer)
		printf("** JSON buffer: %.*s\n", _jsonBuffer.length, _jsonBuffer.data);

	rsslClearParseJsonBufferOptions(&parseOptions);
	parseOptions.jsonProtocolType = RSSL_JSON_JPT_JSON2;
#ifdef _RSSLJC_SHARED_LIBRARY
	ASSERT_NE(rsslJsonConverterFunctions.rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, pConverterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer NOT failed: ";
#else
	ASSERT_NE(rsslParseJsonBuffer(_rsslJsonConverter, &parseOptions, &_jsonBuffer, pConverterError), RSSL_RET_SUCCESS)
		<< "rsslParseJsonBuffer NOT failed: ";
#endif
}

void MsgConversionTestBase::encodeSampleRsslFieldList(RsslEncodeIterator *pIter, RsslLocalFieldSetDefDb * setDb)
{
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslReal real;

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if (setDb != NULL)
	{
		fieldList.flags = fieldList.flags | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
		fieldList.setId = 0;
	}
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(pIter, &fieldList, setDb, 0));

	/* BID */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 22;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_FRACTION_256;
	real.value = 30396;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));

	/* ASK */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 25;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3906;
	if (setDb != NULL)
		ASSERT_EQ(RSSL_RET_SET_COMPLETE, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	else
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonFieldList(const Value &fieldListObject, RsslJsonProtocolType protocolType)
{

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			ASSERT_TRUE(fieldListObject.IsObject());

			/* BID */
			ASSERT_TRUE(fieldListObject.HasMember("BID"));
			ASSERT_TRUE(fieldListObject["BID"].IsNumber());
			EXPECT_NEAR(fieldListObject["BID"].GetDouble(), 118.734375, 0.0000099);

			/* ASK */
			ASSERT_TRUE(fieldListObject.HasMember("ASK"));
			ASSERT_TRUE(fieldListObject["ASK"].IsNumber());
			EXPECT_NEAR(fieldListObject["ASK"].GetDouble(), 39.06, 0.099);
			break;

		case RSSL_JSON_JPT_JSON:
		{
			ASSERT_TRUE(fieldListObject.IsObject());

			ASSERT_TRUE(fieldListObject.HasMember("d"));
			const Value &fieldListData = fieldListObject["d"];
			ASSERT_TRUE(fieldListData.IsObject());

			/* BID */
			ASSERT_TRUE(fieldListData.HasMember("22"));
			ASSERT_TRUE(fieldListData["22"].IsObject());

			ASSERT_TRUE(fieldListData["22"].HasMember("h"));
			ASSERT_TRUE(fieldListData["22"]["h"].IsNumber());
			EXPECT_EQ(RSSL_RH_FRACTION_256, fieldListData["22"]["h"].GetInt());

			ASSERT_TRUE(fieldListData["22"].HasMember("v"));
			ASSERT_TRUE(fieldListData["22"]["v"].IsNumber());
			EXPECT_EQ(30396, fieldListData["22"]["v"].GetInt());

			/* ASK */
			ASSERT_TRUE(fieldListData.HasMember("25"));
			ASSERT_TRUE(fieldListData["25"].IsObject());

			ASSERT_TRUE(fieldListData["25"].HasMember("h"));
			ASSERT_TRUE(fieldListData["25"]["h"].IsNumber());
			EXPECT_EQ(RSSL_RH_EXPONENT_2, fieldListData["25"]["h"].GetInt());

			ASSERT_TRUE(fieldListData["25"].HasMember("v"));
			ASSERT_TRUE(fieldListData["25"]["v"].IsNumber());
			EXPECT_EQ(3906, fieldListData["25"]["v"].GetInt());
			break;
		}

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}
		
}

void MsgConversionTestBase::checkLargeJsonFieldList(const Value &fieldListObject)
{
	ASSERT_TRUE(fieldListObject.IsObject());

	/* BID */
	ASSERT_TRUE(fieldListObject.HasMember("BID"));
	ASSERT_TRUE(fieldListObject["BID"].IsNumber());
	EXPECT_NEAR(fieldListObject["BID"].GetDouble(), 39.05, 0.099);

	/* ASK */
	ASSERT_TRUE(fieldListObject.HasMember("ASK"));
	ASSERT_TRUE(fieldListObject["ASK"].IsNumber());
	EXPECT_NEAR(fieldListObject["ASK"].GetDouble(), 39.06, 0.099);

	/* BID SIZE */
	ASSERT_TRUE(fieldListObject.HasMember("BIDSIZE"));
	ASSERT_TRUE(fieldListObject["BIDSIZE"].IsNumber());
	EXPECT_NEAR(fieldListObject["BIDSIZE"].GetDouble(), 39.07, 0.099);
	
	/* ASK SIZE */
	ASSERT_TRUE(fieldListObject.HasMember("ASKSIZE"));
	ASSERT_TRUE(fieldListObject["ASKSIZE"].IsNumber());
	EXPECT_NEAR(fieldListObject["ASKSIZE"].GetDouble(), 39.08, 0.099);
	
	/* VOL ACCUMULATED */
	ASSERT_TRUE(fieldListObject.HasMember("ACVOL_1"));
	ASSERT_TRUE(fieldListObject["ACVOL_1"].IsNumber());
	EXPECT_NEAR(fieldListObject["ACVOL_1"].GetDouble(), 39.09, 0.099);
	
	/* EARNINGS */
	ASSERT_TRUE(fieldListObject.HasMember("EARNINGS"));
	ASSERT_TRUE(fieldListObject["EARNINGS"].IsNumber());
	EXPECT_NEAR(fieldListObject["EARNINGS"].GetDouble(), 39.10, 0.099);
	
	/* YIELD */
	ASSERT_TRUE(fieldListObject.HasMember("YIELD"));
	ASSERT_TRUE(fieldListObject["YIELD"].IsNumber());
	EXPECT_NEAR(fieldListObject["YIELD"].GetDouble(), 39.11, 0.099);
	
	/* PERCENT CHANGE */
	ASSERT_TRUE(fieldListObject.HasMember("PCTCHNG"));
	ASSERT_TRUE(fieldListObject["PCTCHNG"].IsNumber());
	EXPECT_NEAR(fieldListObject["PCTCHNG"].GetDouble(), 39.12, 0.099);
	
	/* OPEN BID */
	ASSERT_TRUE(fieldListObject.HasMember("OPEN_BID"));
	ASSERT_TRUE(fieldListObject["OPEN_BID"].IsNumber());
	EXPECT_NEAR(fieldListObject["OPEN_BID"].GetDouble(), 39.13, 0.099);
	
	/* OPEN ASK */
	ASSERT_TRUE(fieldListObject.HasMember("OPEN_ASK"));
	ASSERT_TRUE(fieldListObject["OPEN_ASK"].IsNumber());
	EXPECT_NEAR(fieldListObject["OPEN_ASK"].GetDouble(), 39.14, 0.099);
	
	/* CLOSE BID */
	ASSERT_TRUE(fieldListObject.HasMember("CLOSE_BID"));
	ASSERT_TRUE(fieldListObject["CLOSE_BID"].IsNumber());
	EXPECT_NEAR(fieldListObject["CLOSE_BID"].GetDouble(), 39.15, 0.099);
	
	/* CLOSE ASK */
	ASSERT_TRUE(fieldListObject.HasMember("CLOSE_ASK"));
	ASSERT_TRUE(fieldListObject["CLOSE_ASK"].IsNumber());
	EXPECT_NEAR(fieldListObject["CLOSE_ASK"].GetDouble(), 39.16, 0.099);
	
	/* CNTR LIFE HIGH */
	ASSERT_TRUE(fieldListObject.HasMember("LOCHIGH"));
	ASSERT_TRUE(fieldListObject["LOCHIGH"].IsNumber());
	EXPECT_NEAR(fieldListObject["LOCHIGH"].GetDouble(), 39.17, 0.099);
	
	/* CNTR LIFE LOW */
	ASSERT_TRUE(fieldListObject.HasMember("LOCLOW"));
	ASSERT_TRUE(fieldListObject["LOCLOW"].IsNumber());
	EXPECT_NEAR(fieldListObject["LOCLOW"].GetDouble(), 39.18, 0.099);
	
	/* OPEN INTEREST */
	ASSERT_TRUE(fieldListObject.HasMember("OPINT_1"));
	ASSERT_TRUE(fieldListObject["OPINT_1"].IsNumber());
	EXPECT_NEAR(fieldListObject["OPINT_1"].GetDouble(), 39.19, 0.099);
	
	/* OPN INT NETCHG */
	ASSERT_TRUE(fieldListObject.HasMember("OPINTNC"));
	ASSERT_TRUE(fieldListObject["OPINTNC"].IsNumber());
	EXPECT_NEAR(fieldListObject["OPINTNC"].GetDouble(), 39.20, 0.099);
	
	/* STRIKE PRICE */
	ASSERT_TRUE(fieldListObject.HasMember("STRIKE_PRC"));
	ASSERT_TRUE(fieldListObject["STRIKE_PRC"].IsNumber());
	EXPECT_NEAR(fieldListObject["STRIKE_PRC"].GetDouble(), 39.21, 0.099);
	
	/* COUPON RATE */
	ASSERT_TRUE(fieldListObject.HasMember("COUPN_RATE"));
	ASSERT_TRUE(fieldListObject["COUPN_RATE"].IsNumber());
	EXPECT_NEAR(fieldListObject["COUPN_RATE"].GetDouble(), 39.22, 0.099);
}

void MsgConversionTestBase::decodeSampleRsslFieldList(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter, RsslLocalFieldSetDefDb *setDb)
{
	RsslFieldList decodeFieldList;
	RsslFieldEntry decodeFieldEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(pIter, &decodeFieldList, setDb));

	if (setDb)
		ASSERT_TRUE(rsslFieldListCheckHasSetData(&decodeFieldList));
	else
		ASSERT_TRUE(rsslFieldListCheckHasStandardData(&decodeFieldList));

	/* Decode FieldEntries. Don't assume they are in the same order as they were when originally encoded in RWF. */
	RsslRet ret;
	bool foundBidField = false, foundAskField = false;
	while ((ret = rsslDecodeFieldEntry(pIter, &decodeFieldEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode FieldEntry with ID: " << decodeFieldEntry.fieldId;

		switch(decodeFieldEntry.fieldId)
		{
			case 22: /* BID */
			{
				RsslReal decodeReal;
				if (setDb) ASSERT_EQ(RSSL_DT_REAL, decodeFieldEntry.dataType);
				else ASSERT_EQ(RSSL_DT_UNKNOWN, decodeFieldEntry.dataType);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));

				if (protocol == RSSL_JSON_JPT_JSON)
				{
				EXPECT_EQ(0, decodeReal.isBlank);
					EXPECT_EQ(RSSL_RH_FRACTION_256, decodeReal.hint);
					if (decodeReal.value != 30396)
					EXPECT_EQ(30396, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
					foundBidField = true;
				}
				else
				{
					EXPECT_EQ(0, decodeReal.isBlank);
					EXPECT_EQ(RSSL_RH_EXPONENT_6, decodeReal.hint);
					if (decodeReal.value != 118734375)
						EXPECT_EQ(118734375, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundBidField = true;
				}
				break;
			}

			case 25: /* ASK */
			{
				RsslReal decodeReal;
				if (setDb) ASSERT_EQ(RSSL_DT_REAL, decodeFieldEntry.dataType);
				else ASSERT_EQ(RSSL_DT_UNKNOWN, decodeFieldEntry.dataType);
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3906, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}

			default:
				FAIL() << "** Decoded FieldEntry with unknown ID: " << decodeFieldEntry.fieldId;
		}
	}

	ASSERT_TRUE(foundBidField);
	ASSERT_TRUE(foundAskField);
}

void MsgConversionTestBase::encodeLargeRsslFieldList(RsslEncodeIterator *pIter, RsslLocalFieldSetDefDb * setDb)
{
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;
	RsslReal real;

	rsslClearFieldList(&fieldList);
	fieldList.flags = RSSL_FLF_HAS_STANDARD_DATA;
	if (setDb != NULL)
	{
		fieldList.flags = fieldList.flags | RSSL_FLF_HAS_SET_DATA | RSSL_FLF_HAS_SET_ID;
		fieldList.setId = 0;
	}
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListInit(pIter, &fieldList, setDb, 0));

	/* BID */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 22;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_FRACTION_256;
	real.value = 30396;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));

	/* ASK */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 25;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3906;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* BID SIZE */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 30;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3907;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* ASK SIZE */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 31;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3908;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* VOL ACCUMULATED */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 32;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3909;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* EARNINGS */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 34;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3910;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* YIELD */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 35;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3911;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* PERCENT CHANGE */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 56;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3912;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* OPEN BID */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 57;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3913;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* OPEN ASK */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 59;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3914;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* CLOSE BID */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 60;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3915;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* CLOSE ASK */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 61;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3916;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* CNTR LIFE HIGH */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 62;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3917;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* CNTR LIFE LOW */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 63;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3918;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* OPEN INTEREST */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 64;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3919;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* OPN INT NETCHG */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 65;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3920;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* STRIKE PRICE */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 66;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3921;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	
	/* COUPON RATE */
	rsslClearFieldEntry(&fieldEntry);
	fieldEntry.fieldId = 69;
	fieldEntry.dataType = RSSL_DT_REAL;
	rsslClearReal(&real);
	real.hint = RSSL_RH_EXPONENT_2;
	real.value = 3922;
	if (setDb != NULL)
		ASSERT_EQ(RSSL_RET_SET_COMPLETE, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));
	else
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldEntry(pIter, &fieldEntry, &real));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFieldListComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::decodeLargeRsslFieldList(RsslDecodeIterator *pIter)
{
	RsslFieldList decodeFieldList;
	RsslFieldEntry decodeFieldEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFieldList(pIter, &decodeFieldList, NULL));
	ASSERT_TRUE(rsslFieldListCheckHasStandardData(&decodeFieldList));

	/* Decode FieldEntries. Don't assume they are in the same order as they were when originally encoded in RWF. */
	RsslRet ret;
	bool foundBidField = false, foundAskField = false;
	while ((ret = rsslDecodeFieldEntry(pIter, &decodeFieldEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode FieldEntry with ID: " << decodeFieldEntry.fieldId;
		ASSERT_EQ(RSSL_DT_UNKNOWN, decodeFieldEntry.dataType);

		switch(decodeFieldEntry.fieldId)
		{
			case 22: /* BID */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3905, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundBidField = true;
				break;
			}

			case 25: /* ASK */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3906, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 30: /* BID SIZE */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3907, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 31: /* ASK SIZE */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3908, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 32: /* VOL ACCUMULATED */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3909, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 34: /* EARNINGS */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3910, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 35: /* YIELD */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3911, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 56: /* PERCENT CHANGE */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3912, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 57: /* OPEN BID */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3913, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 59: /* OPEN ASK */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3914, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 60: /* CLOSE BID */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3915, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 61: /* CLOSE ASK */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3916, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 62: /* CNTR LIFE HIGH */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3917, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 63: /* CNTR LIFE LOW */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3918, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 64: /* OPEN INTEREST */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3919, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 65: /* OPN INT NETCHG */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3920, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 66: /* STRIKE PRICE */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3921, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}
			
			case 69: /* COUPON RATE */
			{
				RsslReal decodeReal;
				ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeReal(pIter, &decodeReal));
				EXPECT_EQ(0, decodeReal.isBlank);
				EXPECT_EQ(RSSL_RH_EXPONENT_2, decodeReal.hint);
				EXPECT_EQ(3922, decodeReal.value); /* Despite conversion, should be close enough to get original value back. */
				foundAskField = true;
				break;
			}

			default:
				FAIL() << "** Decoded FieldEntry with unknown ID: " << decodeFieldEntry.fieldId;
		}
	}

	ASSERT_TRUE(foundBidField);
	ASSERT_TRUE(foundAskField);
}

/* ElementEntry names used in the sample ElementList. */
static RsslBuffer sampleElementEntryNames[] = {{3, (char*)"ONE"}, {3, (char*)"TWO"}};

void MsgConversionTestBase::encodeSampleRsslElementList(RsslEncodeIterator *pIter, RsslLocalElementSetDefDb * setDb)
{
	RsslElementList elementList;
	RsslElementEntry elementEntry;
	RsslInt rsslInt;

	rsslClearElementList(&elementList);
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA;
	if (setDb != NULL)
	{
		elementList.flags = elementList.flags | RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID;
		elementList.setId = 0;
	}

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListInit(pIter, &elementList, setDb, 0));
	rsslClearElementEntry(&elementEntry);
	elementEntry.name = sampleElementEntryNames[0];
	elementEntry.dataType = RSSL_DT_INT;
	rsslInt = 5;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(pIter, &elementEntry, &rsslInt));
	rsslClearElementEntry(&elementEntry);
	elementEntry.name = sampleElementEntryNames[1];
	elementEntry.dataType = RSSL_DT_INT;
	rsslInt = 6;
	if (setDb != NULL)
		ASSERT_EQ(RSSL_RET_SET_COMPLETE, rsslEncodeElementEntry(pIter, &elementEntry, &rsslInt));
	else
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementEntry(pIter, &elementEntry, &rsslInt));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeElementListComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonElementList(const Value &elementListObject, RsslJsonProtocolType protocolType)
{
	ASSERT_TRUE(elementListObject.IsObject());

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			/* First Element */
			ASSERT_TRUE(elementListObject.HasMember(sampleElementEntryNames[0].data));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[0].data].IsObject());
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[0].data].HasMember("Type"));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[0].data]["Type"].IsString());
			EXPECT_STREQ("Int", elementListObject[sampleElementEntryNames[0].data]["Type"].GetString());

			ASSERT_TRUE(elementListObject[sampleElementEntryNames[0].data].HasMember("Data"));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[0].data]["Data"].IsNumber());
			EXPECT_EQ(5, elementListObject[sampleElementEntryNames[0].data]["Data"].GetInt());

			/* Second Element */
			ASSERT_TRUE(elementListObject.HasMember(sampleElementEntryNames[1].data));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[1].data].IsObject());
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[1].data].HasMember("Type"));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[1].data]["Type"].IsString());
			EXPECT_STREQ("Int", elementListObject[sampleElementEntryNames[1].data]["Type"].GetString());

			ASSERT_TRUE(elementListObject[sampleElementEntryNames[1].data].HasMember("Data"));
			ASSERT_TRUE(elementListObject[sampleElementEntryNames[1].data]["Data"].IsNumber());
			EXPECT_EQ(6, elementListObject[sampleElementEntryNames[1].data]["Data"].GetInt());
			break;

		case RSSL_JSON_JPT_JSON:
			ASSERT_TRUE(elementListObject.HasMember("d"));
			ASSERT_TRUE(elementListObject["d"].IsArray());
			ASSERT_EQ(2, elementListObject["d"].Size());

			/* First Element */
			ASSERT_TRUE(elementListObject["d"][0].HasMember("n"));
			EXPECT_STREQ(sampleElementEntryNames[0].data, elementListObject["d"][0]["n"].GetString());
			ASSERT_TRUE(elementListObject["d"][0].HasMember("t"));
			ASSERT_TRUE(elementListObject["d"][0]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT, elementListObject["d"][0]["t"].GetInt());
			ASSERT_TRUE(elementListObject["d"][0].HasMember("d"));
			ASSERT_TRUE(elementListObject["d"][0]["d"].IsNumber());
			EXPECT_EQ(5, elementListObject["d"][0]["d"].GetInt());

			/* Second Element */
			ASSERT_TRUE(elementListObject["d"][1].HasMember("n"));
			EXPECT_STREQ(sampleElementEntryNames[1].data, elementListObject["d"][1]["n"].GetString());
			ASSERT_TRUE(elementListObject["d"][1].HasMember("t"));
			ASSERT_TRUE(elementListObject["d"][1]["t"].IsNumber());
			EXPECT_EQ(RSSL_DT_INT, elementListObject["d"][1]["t"].GetInt());
			ASSERT_TRUE(elementListObject["d"][1].HasMember("d"));
			ASSERT_TRUE(elementListObject["d"][1]["d"].IsNumber());
			EXPECT_EQ(6, elementListObject["d"][1]["d"].GetInt());
			break;

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}

}

void MsgConversionTestBase::decodeSampleRsslElementList(RsslJsonProtocolType protocolType, RsslDecodeIterator *pIter, RsslLocalElementSetDefDb *setDb)
{
	RsslElementList decodeElementList;
	RsslElementEntry decodeElementEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeElementList(pIter, &decodeElementList, setDb));

	if (setDb)
		ASSERT_TRUE(rsslElementListCheckHasSetData(&decodeElementList));
	else
		ASSERT_TRUE(rsslElementListCheckHasStandardData(&decodeElementList));

	bool foundFirstElement = false, foundSecondElement = false;

	/* Decode ElementEntries. Don't assume they are in the same order as they were when originally encoded in RWF. */
	RsslRet ret;
	while ((ret = rsslDecodeElementEntry(pIter, &decodeElementEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		std::string elementName(decodeElementEntry.name.data, decodeElementEntry.name.length);
		ASSERT_EQ(RSSL_RET_SUCCESS, ret) << "** Failed to decode ElementEntry with Name: " << elementName;

		if (rsslBufferIsEqual(&sampleElementEntryNames[0], &decodeElementEntry.name))
		{
			RsslInt decodeInt;
			ASSERT_EQ(RSSL_DT_INT, decodeElementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(pIter, &decodeInt));
			EXPECT_EQ(5, decodeInt);
			foundFirstElement = true;
		}
		else if (rsslBufferIsEqual(&sampleElementEntryNames[1], &decodeElementEntry.name))
		{
			RsslInt decodeInt;
			ASSERT_EQ(RSSL_DT_INT, decodeElementEntry.dataType);
			ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeInt(pIter, &decodeInt));
			EXPECT_EQ(6, decodeInt);
			foundSecondElement = true;
		}
	}

	ASSERT_TRUE(foundFirstElement);
	ASSERT_TRUE(foundSecondElement);
}

void MsgConversionTestBase::encodeSampleRsslFilterList(RsslEncodeIterator *pIter)
{
	RsslFilterList filterList;
	RsslFilterEntry filterEntry;

	rsslClearFilterList(&filterList);
	filterList.containerType = RSSL_DT_ELEMENT_LIST;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListInit(pIter, &filterList));

	/* One entry, with an element list. */
	rsslClearFilterEntry(&filterEntry);
	filterEntry.id = 1;
	filterEntry.action = RSSL_FTEA_SET_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryInit(pIter, &filterEntry, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslElementList(pIter, NULL));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterEntryComplete(pIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeFilterListComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonFilterList(const Value &filterListObject, RsslJsonProtocolType protocolType)
{
	ASSERT_TRUE(filterListObject.IsObject());

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			ASSERT_TRUE(filterListObject.HasMember("Entries"));
			ASSERT_TRUE(filterListObject["Entries"].IsArray());
			ASSERT_EQ(1, filterListObject["Entries"].Size());

			ASSERT_TRUE(filterListObject["Entries"][0].HasMember("ID"));
			ASSERT_TRUE(filterListObject["Entries"][0]["ID"].IsNumber());
			EXPECT_EQ(1, filterListObject["Entries"][0]["ID"].GetInt());

			ASSERT_TRUE(filterListObject["Entries"][0].HasMember("Action"));
			ASSERT_TRUE(filterListObject["Entries"][0]["Action"].IsString());
			EXPECT_STREQ("Set", filterListObject["Entries"][0]["Action"].GetString());

			ASSERT_TRUE(filterListObject["Entries"][0].HasMember("Elements"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(filterListObject["Entries"][0]["Elements"], protocolType));
			break;

		case RSSL_JSON_JPT_JSON:

			/* containerType */
			ASSERT_TRUE(filterListObject.HasMember("f"));
			ASSERT_TRUE(filterListObject["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_ELEMENT_LIST - 128, filterListObject["f"].GetInt());

			/* Entries */
			ASSERT_TRUE(filterListObject.HasMember("d"));
			ASSERT_TRUE(filterListObject["d"].IsArray());
			ASSERT_EQ(1, filterListObject["d"].Size());

			/* Entry Filter ID */
			ASSERT_TRUE(filterListObject["d"][0].HasMember("i"));
			ASSERT_TRUE(filterListObject["d"][0]["i"].IsNumber());
			EXPECT_EQ(1, filterListObject["d"][0]["i"].GetInt());

			/* Entry Action */
			ASSERT_TRUE(filterListObject["d"][0].HasMember("a"));
			ASSERT_TRUE(filterListObject["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_FTEA_SET_ENTRY, filterListObject["d"][0]["a"].GetInt());

			/* Entry Data */
			ASSERT_TRUE(filterListObject["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonElementList(filterListObject["d"][0]["d"], protocolType));
			break;

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}

}

void MsgConversionTestBase::decodeSampleRsslFilterList(RsslJsonProtocolType protocolType, RsslDecodeIterator *pIter)
{
	RsslFilterList decodeFilterList;
	RsslFilterEntry decodeFilterEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterList(pIter, &decodeFilterList));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeFilterEntry(pIter, &decodeFilterEntry));
	EXPECT_EQ(1, decodeFilterEntry.id);
	EXPECT_EQ(RSSL_FTEA_SET_ENTRY, decodeFilterEntry.action);
	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslElementList(protocolType, pIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeFilterEntry(pIter, &decodeFilterEntry));
}

void MsgConversionTestBase::encodeSampleRsslMap(RsslEncodeIterator *pIter)
{
	RsslMap map;
	RsslMapEntry mapEntry;
	RsslInt mapEntryKey;

	rsslClearMap(&map);
	map.containerType = RSSL_DT_FIELD_LIST;
	map.keyPrimitiveType = RSSL_DT_INT;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapInit(pIter, &map, 0, 0));

	rsslClearMapEntry(&mapEntry);
	mapEntryKey = 1;
	mapEntry.action = RSSL_MPEA_ADD_ENTRY;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryInit(pIter, &mapEntry, &mapEntryKey, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapEntryComplete(pIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMapComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonMap(const Value &mapObject, RsslJsonProtocolType protocolType)
{
	ASSERT_TRUE(mapObject.IsObject());

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			ASSERT_TRUE(mapObject.HasMember("KeyType"));
			ASSERT_TRUE(mapObject["KeyType"].IsString());
			ASSERT_STREQ("Int", mapObject["KeyType"].GetString());
			ASSERT_TRUE(mapObject.HasMember("Entries"));
			ASSERT_TRUE(mapObject["Entries"].IsArray());
			ASSERT_EQ(1, mapObject["Entries"].Size());

			/* Map Entry */
			ASSERT_TRUE(mapObject["Entries"][0].HasMember("Action"));
			ASSERT_TRUE(mapObject["Entries"][0]["Action"].IsString());
			EXPECT_STREQ("Add", mapObject["Entries"][0]["Action"].GetString());
			ASSERT_TRUE(mapObject["Entries"][0].HasMember("Key"));
			ASSERT_TRUE(mapObject["Entries"][0]["Key"].IsNumber());
			EXPECT_EQ(1, mapObject["Entries"][0]["Key"].GetInt());
			ASSERT_TRUE(mapObject["Entries"][0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(mapObject["Entries"][0]["Fields"], protocolType));
			break;

		case RSSL_JSON_JPT_JSON:
			ASSERT_TRUE(mapObject.HasMember("f"));
			ASSERT_TRUE(mapObject["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_FIELD_LIST - 128, mapObject["f"].GetInt());
			ASSERT_TRUE(mapObject.HasMember("k"));
			ASSERT_TRUE(mapObject["k"].IsNumber());
			ASSERT_EQ(RSSL_DT_INT, mapObject["k"].GetInt());
			ASSERT_TRUE(mapObject.HasMember("d"));
			ASSERT_TRUE(mapObject["d"].IsArray());
			ASSERT_EQ(1, mapObject["d"].Size());

			/* Map Entry */
			ASSERT_TRUE(mapObject["d"][0].HasMember("a"));
			ASSERT_TRUE(mapObject["d"][0]["a"].IsNumber());
			EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, mapObject["d"][0]["a"].GetInt());
			ASSERT_TRUE(mapObject["d"][0].HasMember("k"));
			ASSERT_TRUE(mapObject["d"][0]["k"].IsNumber());
			EXPECT_EQ(1, mapObject["d"][0]["k"].GetInt());
			ASSERT_TRUE(mapObject["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(mapObject["d"][0]["d"], protocolType));
			break;

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}
}

void MsgConversionTestBase::decodeSampleRsslMap(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter)
{
	RsslMap decodeMap;
	RsslMapEntry decodeMapEntry;
	RsslInt decodeMapEntryKey;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMap(pIter, &decodeMap));
	ASSERT_EQ(RSSL_DT_INT, decodeMap.keyPrimitiveType);
	ASSERT_EQ(RSSL_DT_FIELD_LIST, decodeMap.containerType);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMapEntry(pIter, &decodeMapEntry, (void*)&decodeMapEntryKey));
	EXPECT_EQ(1, decodeMapEntryKey);
	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, decodeMapEntry.action);

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(protocol, pIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeMapEntry(pIter, &decodeMapEntry, (void*)&decodeMapEntryKey));
}

void MsgConversionTestBase::encodeSampleRsslVector(RsslEncodeIterator *pIter)
{
	RsslVector vector;
	RsslVectorEntry vectorEntry;

	rsslClearVector(&vector);
	vector.containerType = RSSL_DT_FIELD_LIST;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorInit(pIter, &vector, 0, 0));

	rsslClearVectorEntry(&vectorEntry);
	vectorEntry.action = RSSL_VTEA_SET_ENTRY;
	vectorEntry.index = 1;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryInit(pIter, &vectorEntry, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorEntryComplete(pIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeVectorComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonVector(const Value &vectorObject, RsslJsonProtocolType protocolType)
{
	ASSERT_TRUE(vectorObject.IsObject());

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			ASSERT_TRUE(vectorObject.HasMember("Entries"));
			ASSERT_TRUE(vectorObject["Entries"].IsArray());
			ASSERT_EQ(1, vectorObject["Entries"].Size());

			/* Vector Entry */
			ASSERT_TRUE(vectorObject["Entries"][0].HasMember("Index"));
			ASSERT_TRUE(vectorObject["Entries"][0]["Index"].IsNumber());
			ASSERT_EQ(1, vectorObject["Entries"][0]["Index"].GetInt());
			ASSERT_TRUE(vectorObject["Entries"][0].HasMember("Action"));
			ASSERT_TRUE(vectorObject["Entries"][0]["Action"].IsString());
			ASSERT_STREQ("Set", vectorObject["Entries"][0]["Action"].GetString());
			ASSERT_TRUE(vectorObject["Entries"][0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(vectorObject["Entries"][0]["Fields"], protocolType));
			break;

		case RSSL_JSON_JPT_JSON:
			ASSERT_TRUE(vectorObject.HasMember("f"));
			ASSERT_TRUE(vectorObject["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_FIELD_LIST - 128, vectorObject["f"].GetInt());
			ASSERT_TRUE(vectorObject.HasMember("d"));
			ASSERT_TRUE(vectorObject["d"].IsArray());
			ASSERT_EQ(1, vectorObject["d"].Size());

			/* Vector Entry */
			ASSERT_TRUE(vectorObject["d"][0].HasMember("i"));
			ASSERT_TRUE(vectorObject["d"][0]["i"].IsNumber());
			ASSERT_EQ(1, vectorObject["d"][0]["i"].GetInt());
			ASSERT_TRUE(vectorObject["d"][0].HasMember("a"));
			ASSERT_TRUE(vectorObject["d"][0]["a"].IsNumber());
			ASSERT_EQ(RSSL_VTEA_SET_ENTRY, vectorObject["d"][0]["a"].GetInt());
			ASSERT_TRUE(vectorObject["d"][0].HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(vectorObject["d"][0]["d"], protocolType));
			break;

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}
}

void MsgConversionTestBase::decodeSampleRsslVector(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter)
{
	RsslVector decodeVector;
	RsslVectorEntry decodeVectorEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVector(pIter, &decodeVector));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, decodeVector.containerType);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeVectorEntry(pIter, &decodeVectorEntry));
	EXPECT_EQ(1, decodeVectorEntry.index);
	EXPECT_EQ(RSSL_MPEA_ADD_ENTRY, decodeVectorEntry.action);

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(protocol, pIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeVectorEntry(pIter, &decodeVectorEntry));
}

void MsgConversionTestBase::encodeSampleRsslSeries(RsslEncodeIterator *pIter)
{
	RsslSeries series;
	RsslSeriesEntry seriesEntry;

	rsslClearSeries(&series);
	series.containerType = RSSL_DT_FIELD_LIST;
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesInit(pIter, &series, 0, 0));

	rsslClearSeriesEntry(&seriesEntry);
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryInit(pIter, &seriesEntry, 0));

	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesEntryComplete(pIter, RSSL_TRUE));

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeSeriesComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonSeries(const Value &seriesObject, RsslJsonProtocolType protocolType)
{
	ASSERT_TRUE(seriesObject.IsObject());

	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			ASSERT_TRUE(seriesObject.HasMember("Entries"));
			ASSERT_TRUE(seriesObject["Entries"].IsArray());
			ASSERT_EQ(1, seriesObject["Entries"].Size());

			/* Series Entry */
			ASSERT_TRUE(seriesObject["Entries"][0].HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(seriesObject["Entries"][0]["Fields"], protocolType));
			break;

		case RSSL_JSON_JPT_JSON:
			ASSERT_TRUE(seriesObject.HasMember("f"));
			ASSERT_TRUE(seriesObject["f"].IsNumber());
			ASSERT_EQ(RSSL_DT_FIELD_LIST - 128, seriesObject["f"].GetInt());

			ASSERT_TRUE(seriesObject.HasMember("d"));
			ASSERT_TRUE(seriesObject["d"].IsArray());
			ASSERT_EQ(1, seriesObject["d"].Size());

			/* Series Entry */
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(seriesObject["d"][0], protocolType));
			break;

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}
}

void MsgConversionTestBase::decodeSampleRsslSeries(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter)
{
	RsslSeries decodeSeries;
	RsslSeriesEntry decodeSeriesEntry;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeries(pIter, &decodeSeries));
	ASSERT_EQ(RSSL_DT_FIELD_LIST, decodeSeries.containerType);

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeSeriesEntry(pIter, &decodeSeriesEntry));

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(protocol, pIter));

	ASSERT_EQ(RSSL_RET_END_OF_CONTAINER, rsslDecodeSeriesEntry(pIter, &decodeSeriesEntry));
}


void MsgConversionTestBase::encodeSampleRsslUpdateMsg(RsslEncodeIterator *pIter)
{
	RsslUpdateMsg updateMsg;

	rsslClearUpdateMsg(&updateMsg);
	updateMsg.msgBase.streamId = 5;
	updateMsg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	updateMsg.msgBase.containerType = RSSL_DT_FIELD_LIST;
	updateMsg.updateType = RDM_UPD_EVENT_TYPE_QUOTE;

	ASSERT_EQ(RSSL_RET_ENCODE_CONTAINER, rsslEncodeMsgInit(pIter, (RsslMsg*)&updateMsg, 0));
	ASSERT_NO_FATAL_FAILURE(encodeSampleRsslFieldList(pIter, NULL));
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslEncodeMsgComplete(pIter, RSSL_TRUE));
}

void MsgConversionTestBase::checkSampleJsonUpdateMsg(const Value &updateObject, RsslJsonProtocolType protocolType)
{
	switch(protocolType)
	{
		case RSSL_JSON_JPT_JSON2:
			/* Check message type. */
			ASSERT_TRUE(updateObject.HasMember("Type"));
			ASSERT_TRUE(updateObject["Type"].IsString());
			EXPECT_STREQ("Update", updateObject["Type"].GetString());

			/* Check Stream ID. */
			ASSERT_TRUE(updateObject.HasMember("ID"));
			ASSERT_TRUE(updateObject["ID"].IsNumber());
			EXPECT_EQ(5, updateObject["ID"].GetInt());

			/* Check Domain. */
			ASSERT_FALSE(updateObject.HasMember("Domain")); /* MarketPrice */

			/* Check UpdateType. */
			ASSERT_TRUE(updateObject.HasMember("UpdateType"));
			ASSERT_TRUE(updateObject["UpdateType"].IsString());
			EXPECT_STREQ("Quote", updateObject["UpdateType"].GetString());

			ASSERT_TRUE(updateObject.HasMember("Fields"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(updateObject["Fields"], protocolType));
			break;

		case RSSL_JSON_JPT_JSON:
		{
			/* Get message base. */
			ASSERT_TRUE(updateObject.HasMember("b"));
			ASSERT_TRUE(updateObject["b"].IsObject());
			const Value &msgBase = updateObject["b"];

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
			ASSERT_TRUE(updateObject.HasMember("u"));
			ASSERT_TRUE(updateObject["u"].IsNumber());
			EXPECT_EQ(RDM_UPD_EVENT_TYPE_QUOTE, updateObject["u"].GetInt());

			ASSERT_TRUE(updateObject.HasMember("d"));
			ASSERT_NO_FATAL_FAILURE(checkSampleJsonFieldList(updateObject["d"], protocolType));
			break;
		}

		default:
			FAIL () << "Unknown protocol type " << protocolType;
			break;
	}
}

void MsgConversionTestBase::decodeSampleRsslUpdateMsg(RsslJsonProtocolType protocol, RsslDecodeIterator *pIter)
{
	RsslMsg rsslMsg;

	ASSERT_EQ(RSSL_RET_SUCCESS, rsslDecodeMsg(pIter, &rsslMsg));
	EXPECT_EQ(RSSL_MC_UPDATE, rsslMsg.msgBase.msgClass);
	EXPECT_EQ(5, rsslMsg.msgBase.streamId);
	EXPECT_EQ(RSSL_DMT_MARKET_PRICE, rsslMsg.msgBase.domainType);
	EXPECT_EQ(RSSL_DT_FIELD_LIST, rsslMsg.msgBase.containerType);

	ASSERT_NO_FATAL_FAILURE(decodeSampleRsslFieldList(protocol, pIter));
}

void MsgConversionTestBase::checkSampleJsonObject(const Value &jsonObject)
{
	ASSERT_TRUE(jsonObject.IsObject());
	ASSERT_TRUE(jsonObject.HasMember("SomeKey"));
	ASSERT_TRUE(jsonObject["SomeKey"].IsBool());
	EXPECT_TRUE(jsonObject["SomeKey"].GetBool());
}

RsslDataDictionary* MsgConversionTestBase::getRsslDataDictionary()
{
	return _pRsslDictionary;
}

/* RsslDataTypeParam definitions */

RsslDataTypeParam::RsslDataTypeParam(RsslDataTypes dataType)
{
	this->dataType = dataType;
}

ostream &operator<<(ostream &out, const RsslDataTypeParam& param)
{
	const char *dataTypeString = rsslDataTypeToString(param.dataType);
	out << param.dataType << "(" << (dataTypeString ? dataTypeString : "Unknown") << ")";
	return out;
}
